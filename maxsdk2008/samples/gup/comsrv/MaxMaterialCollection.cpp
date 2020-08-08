// MaxMaterialCollection.cpp : Implementation of CMaxMaterialCollection
#include "stdafx.h"
#include "Comsrv.h"
#include "MaxMaterialCollection.h"
#include "imtledit.h"

#ifdef EXTENDED_OBJECT_MODEL
#include "..\..\..\..\include\iappinternal.h" //this is needed for NOTIFY_MEDIT_SHOW
#include "MaxMaterial.h"
#include <comutil.h>

//we create an XMLMAterial importer, need its interface and guids
#include "xmlmtl.h"
#include "xmlmtl_i.c"

//don't use max's assert
#include <assert.h>


//Refmaker array access
#define NUM_MMC_REFS 2
#define SCENE_MTLLIB_REFNUM 0
#define SCRATCH_MTLLIB_REFNUM 1
#define RECYCLE_MTLLIB_REFNUM 2
// add new refs here

#define RECYCLE_MATS
#define PERSIST_SCRATCH
#define SUSPEND_UNDO //2/23/05 realize we could get pre-undo notification so why not sleep through it


#ifdef PERSIST_SCRATCH
static ScratchLib theScratchMtlLib;
#endif

extern TCHAR *GetString(int id);
bool CMaxMaterialCollection::s_InMakeWrapper = false;
bool CMaxMaterialCollection::s_PreventRecursion = false;
HMODULE CMaxMaterialCollection::shXmlMtlDllModule = NULL;



/////////////////////////////////////////////////////////////////////////////
// CMaxMaterialCollection
STDMETHODIMP CMaxMaterialCollection::FinalConstruct()
{
	HRESULT hr = E_FAIL, hr2 = E_FAIL;
	//simply make a reference to the scene material collection
	MtlBaseLib* pMtlLib = GetCOREInterface()->GetSceneMtls();
	mLastSceneMtlLibSize = 0;

#ifdef PERSIST_SCRATCH
	mpScratchMtlLib = NULL;
#endif

	assert(pMtlLib);

	//RK: 10/21/04 -- this forces ACT toolkit to initialize at startup and avoids delays during material assignment.
	CComDispatchDriver act;
	/*
		FIXME: Use the .NET wrappers directly instead of this;
		act.CoCreateInstance( L"Autodesk.Act.Core.ContentSerializer" );

		For some reason, the prog ID above is not being registered with the version
		we now install with Max 9, and I've had to resort to using the CLSID directly.

		Either we have the wrong version, or the new DLL just doesn't register its
		prog ID.  (more likely it's the former)
	*/
	CLSID clsidCore;
	hr = CLSIDFromString(L"{978C551B-5919-42F7-81AB-690D66AB7B78}", &clsidCore);

	if( SUCCEEDED(hr) )
		hr = act.CoCreateInstance( clsidCore );

	if (SUCCEEDED(hr))
	{
		CComVariant xml(L"<Material><StandardMtl name=\"Test\" /></Material>");
		CComPtr<IUnknown> mtl;
		hr2 = mtl.CoCreateInstance( L"Autodesk.Act.Content.Material");
		hr2 = act.Invoke1( L"DeserializeString", &xml );
	}
	if ( FAILED(hr) || FAILED(hr2) )
	{
		if (GetCOREInterface()->GetQuietMode())
			GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG, GetString(IDS_PROJNAME), GetString(IDS_ACT_REGISTRATION_FAILED));
		else
			MaxMsgBox( GetCOREInterface()->GetMAXHWnd(), GetString(IDS_ACT_REGISTRATION_FAILED), GetString(IDS_PROJNAME), MB_OK );
	}
	assert(SUCCEEDED(hr) && SUCCEEDED(hr2));

	//RK: This forces XMlmtl to initialize at startup.
	CComPtr<IVIZPointerClient> pPointerClient;
	hr = GetXMLImpExp(&pPointerClient);
	assert(SUCCEEDED(hr));

#ifdef RECYCLE_MATS
#endif

	RegisterNotification(NotifyProc, this, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(NotifyProc, this, NOTIFY_FILE_POST_OPEN);
    RegisterNotification(NotifyProc, this, NOTIFY_FILE_OPEN_FAILED);
	RegisterNotification(NotifyProc, this, NOTIFY_FILE_PRE_MERGE);
	RegisterNotification(NotifyProc, this, NOTIFY_FILE_POST_MERGE);
	RegisterNotification(NotifyProc, this, NOTIFY_FILE_PRE_SAVE);
	RegisterNotification(NotifyProc, this, NOTIFY_FILE_POST_SAVE);
	RegisterNotification(NotifyProc, this, NOTIFY_PRE_IMPORT);
	RegisterNotification(NotifyProc, this, NOTIFY_POST_IMPORT);
	RegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_PRE_NEW);
	RegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_POST_NEW);
	RegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_POST_RESET);
	RegisterNotification(NotifyProc, this, NOTIFY_SCENE_UNDO);
	RegisterNotification(NotifyProc, this, NOTIFY_SCENE_REDO);
#ifdef SUSPEND_UNDO
	RegisterNotification(NotifyProc, this, NOTIFY_SCENE_PRE_UNDO);
	RegisterNotification(NotifyProc, this, NOTIFY_SCENE_PRE_REDO);
#endif

	RegisterNotification(NotifyProc, (void *)this, NOTIFY_MEDIT_SHOW);

#ifdef TP_SUSPEND_FOR_FILELINK
	RegisterNotification(NotifyProc, this, NOTIFY_FILELINK_PRE_BIND		);
	RegisterNotification(NotifyProc, this, NOTIFY_FILELINK_POST_BIND	);
	RegisterNotification(NotifyProc, this, NOTIFY_FILELINK_PRE_DETACH	);
	RegisterNotification(NotifyProc, this, NOTIFY_FILELINK_POST_DETACH	);
	RegisterNotification(NotifyProc, this, NOTIFY_FILELINK_PRE_RELOAD	);
	RegisterNotification(NotifyProc, this, NOTIFY_FILELINK_POST_RELOAD	);
	RegisterNotification(NotifyProc, this, NOTIFY_FILELINK_PRE_ATTACH	);
	RegisterNotification(NotifyProc, this, NOTIFY_FILELINK_POST_ATTACH	);
#endif

	//and finally a mechanism for other parts of the system to actively suspend TP
	RegisterNotification(NotifyProc, this, NOTIFY_TOOLPALETTE_MTL_SUSPEND	);
	RegisterNotification(NotifyProc, this, NOTIFY_TOOLPALETTE_MTL_RESUME	);

	//for DID 642266, pre and post cloning 
	RegisterNotification(NotifyProc, this, NOTIFY_PRE_NODES_CLONED	);
	RegisterNotification(NotifyProc, this, NOTIFY_POST_NODES_CLONED	);
	


	RefResult res = ReplaceReference(SCENE_MTLLIB_REFNUM, (RefTargetHandle) pMtlLib);
#ifdef PERSIST_SCRATCH
	res = ReplaceReference(SCRATCH_MTLLIB_REFNUM, (RefTargetHandle) &theScratchMtlLib);
#endif

	Resume();
	if(res == REF_SUCCEED)
		hr = S_OK;
	return hr;
}

void CMaxMaterialCollection::FinalRelease()
{
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILE_PRE_OPEN);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILE_POST_OPEN);
    UnRegisterNotification(NotifyProc, this, NOTIFY_FILE_OPEN_FAILED);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILE_PRE_MERGE);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILE_POST_MERGE);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILE_PRE_SAVE);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILE_POST_SAVE);
	UnRegisterNotification(NotifyProc, this, NOTIFY_PRE_IMPORT);
	UnRegisterNotification(NotifyProc, this, NOTIFY_POST_IMPORT);
	UnRegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_PRE_NEW);
	UnRegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_POST_NEW);
	UnRegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_PRE_RESET);
	UnRegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_POST_RESET);
	UnRegisterNotification(NotifyProc, this, NOTIFY_SCENE_UNDO);
	UnRegisterNotification(NotifyProc, this, NOTIFY_SCENE_REDO);
#ifdef SUSPEND_UNDO
	UnRegisterNotification(NotifyProc, this, NOTIFY_SCENE_PRE_UNDO);
	UnRegisterNotification(NotifyProc, this, NOTIFY_SCENE_PRE_REDO);
#endif

	UnRegisterNotification(NotifyProc, (void *)this, NOTIFY_MEDIT_SHOW);

#ifdef TP_SUSPEND_FOR_FILELINK
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILELINK_PRE_BIND		);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILELINK_POST_BIND	);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILELINK_PRE_DETACH	);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILELINK_POST_DETACH	);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILELINK_PRE_RELOAD	);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILELINK_POST_RELOAD	);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILELINK_PRE_ATTACH	);
	UnRegisterNotification(NotifyProc, this, NOTIFY_FILELINK_POST_ATTACH	);
#endif

	//and finally a mechanism for other parts of the system to actively suspend TP
	UnRegisterNotification(NotifyProc, this, NOTIFY_TOOLPALETTE_MTL_SUSPEND	);
	UnRegisterNotification(NotifyProc, this, NOTIFY_TOOLPALETTE_MTL_RESUME	);

	//for DID 642266, pre and post cloning 
	UnRegisterNotification(NotifyProc, this, NOTIFY_PRE_NODES_CLONED	);
	UnRegisterNotification(NotifyProc, this, NOTIFY_POST_NODES_CLONED	);

	DeleteAllRefsFromMe();

		
	if(shXmlMtlDllModule)
	{ 
		FreeLibrary(shXmlMtlDllModule);
		shXmlMtlDllModule = NULL;
	}

}


STDMETHODIMP CMaxMaterialCollection::createNew(IMaxMaterial **ppMtl)
{
	if(!ppMtl)
		return E_INVALIDARG;
	CComPtr<IMaxMaterial> pMtlWrapper;

	//Create the wrapper
	MtlWrapper *pWrapperObject;
	HRESULT hr = MtlWrapper::CreateInstance(&pWrapperObject);
	assert(SUCCEEDED(hr));
	if(!SUCCEEDED(hr))
		return hr;

	//Initalize the new wrapper
	pWrapperObject->setCollection(this);

	hr = pWrapperObject->QueryInterface(IID_IMaxMaterial, (void **)ppMtl);
	if(!SUCCEEDED(hr))
		return hr;

	return hr;
}

static bool getUL(const VARIANT& v, unsigned long& val)
{
	switch (v.vt) {
		case VT_BSTR: {
			unsigned long ul;
			char c;
			if (swscanf(_bstr_t(v.bstrVal), L" %lu %c", &ul, &c) != 1)
				return false;
			val = ul;
		} break;

		default:
			{
				val = _variant_t(v).operator unsigned long();
				break;
			}
	}
	return true;
}

STDMETHODIMP CMaxMaterialCollection::createFromMaxID(VARIANT partA, VARIANT partB, IMaxMaterial ** ppMtl)
{
	unsigned long a, b;
	if(!ppMtl || !getUL(partA, a) || !getUL(partB, b))
		return E_INVALIDARG;

	//Create the wrapper
	MtlWrapper *pWrapperObject;
	HRESULT hr = MtlWrapper::CreateInstance(&pWrapperObject);
	assert(SUCCEEDED(hr));
	if(!SUCCEEDED(hr))
		return hr;

	hr = pWrapperObject->QueryInterface(IID_IMaxMaterial, (void **)ppMtl);
	if(!SUCCEEDED(hr))
		return hr;

	Mtl* mtl = static_cast<Mtl*>(::CreateInstance(MATERIAL_CLASS_ID, Class_ID(a, b)));
	if (mtl == NULL)
		return E_INVALIDARG;

	//Initalize the new wrapper
	pWrapperObject->setCollection(this);
	pWrapperObject->setMtl(mtl);

	return hr;
}

//Implement reference maker methods

RefResult CMaxMaterialCollection::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message)
{
	if(m_suspendCount > 0)
		return REF_SUCCEED;
	if(hTarget == mpSceneMtlLib)
	{
		switch(message)
		{
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			OnSceneLibChanged();
			break;
		default:
			break;
		}
	}
	else if(hTarget == mpScratchMtlLib)
	{
		switch(message)
		{
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			OnScratchLibChanged();
			break;
		default:
			break;
		}
	}
	return REF_SUCCEED;
}

int CMaxMaterialCollection::NumRefs()
{
	return NUM_MMC_REFS;
}

RefTargetHandle CMaxMaterialCollection::GetReference(int i)
{
	switch(i)
	{
	case SCENE_MTLLIB_REFNUM:
		return (RefTargetHandle) mpSceneMtlLib;
		break;
#ifdef PERSIST_SCRATCH
	case SCRATCH_MTLLIB_REFNUM:
		return (RefTargetHandle) mpScratchMtlLib;
		break;
#endif
	default:
		assert(0);
	}
	return NULL;
}

void CMaxMaterialCollection::SetReference(int i, RefTargetHandle rtarg)
{
	switch(i)
	{
	case SCENE_MTLLIB_REFNUM:
//		assert(rtarg->ClassID() == mtlBaseLibClassID); //not visible in the sdk
		mpSceneMtlLib = (MtlBaseLib *) rtarg;
		break;
#ifdef PERSIST_SCRATCH
	case SCRATCH_MTLLIB_REFNUM:
		mpScratchMtlLib = (ScratchLib *) rtarg;
		break;
#endif
	default:
		assert(0);
	}

}

static const TSTR sXmlMaterialModuleName("stdplugs\\XmlMtl.dll");

// DllGetClassObject function pointer
typedef HRESULT (STDAPICALLTYPE *DLLGETCLASSOBJECT_PROC)(REFCLSID, REFIID, LPVOID*);

HRESULT CMaxMaterialCollection::CreatePluglet(REFCLSID rclsid, REFIID riid, void** ppPluglet, LPCTSTR sModule)
{
	HRESULT hr = E_FAIL;

	if (sModule && _tcslen(sModule) > 0)
	{

		if(shXmlMtlDllModule == NULL)//cache of handle not set
		{
			shXmlMtlDllModule = LoadLibrary(sModule);
			if (shXmlMtlDllModule == NULL)
			{
				// Failed to load DLL, the last error will be set by the system
				return E_FAIL;
			}
		}

		// Get the proc addres for function DllGetClassObject
		DLLGETCLASSOBJECT_PROC pFunc = 
				(DLLGETCLASSOBJECT_PROC)GetProcAddress(shXmlMtlDllModule, "DllGetClassObject");
		assert(pFunc != NULL);
		if (pFunc == NULL)
		{
			// DllGetClassObject function is not exported
			// The specified procedure could not be found.
			SetLastError(ERROR_PROC_NOT_FOUND);
			return E_FAIL;
		}

		// Get the class factory interface using DllGetClassObject
		CComPtr<IClassFactory> pCF;
		hr = pFunc(rclsid, IID_IClassFactory, (void**) &pCF);
		assert(!FAILED(hr) && pCF != NULL);
		if (FAILED(hr) || pCF == NULL)
		{
			SetLastError(hr); // Failed to get interface
			return E_FAIL;
		}

		// Create the COM object using the class factory
		hr = pCF->CreateInstance(NULL, riid, (void**)ppPluglet);
		assert(SUCCEEDED(hr) && ppPluglet != NULL);
		if (FAILED(hr) || ppPluglet == NULL)
		{
			pCF = NULL; // will release the COM object
			SetLastError(hr); // Failed to create COM object
			return E_FAIL;
		}
	}
	return hr;
}

HRESULT CMaxMaterialCollection::GetXMLImpExp(IVIZPointerClient** ppClient)
{
#ifdef XXX
	//Begin temporary
	// temporary impl: Create an XMLMaterial COM object and ask it to import
	//TODO (JH 7/25/02 Done By CC) implement this differently, CoCreate is slow
	HRESULT hr = CoCreateInstance(CLSID_XmlMaterial, NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IVIZPointerClient), (void**)ppClient);
	if(!SUCCEEDED(hr) || *ppClient == NULL)
	{
		//TODO (JH 7/25/02 Done By CC)Remove the message box 
		MessageBox(NULL, "Failed to create XML Material importer, is it registered?", NULL, MB_OK | MB_TASKMODAL);
		return E_FAIL;
	}
	VARIANT vIp, vImpIp;
	vIp.vt = vImpIp.vt = VT_BYREF;
	vIp.byref = GetCOREInterface();
	vImpIp.byref = GetCOREInterface();//this is a HACK
	return (*ppClient)->InitPointers(vIp, vImpIp);
#else
	*ppClient = NULL;
	Interface *ip = GetCOREInterface();
	TSTR plugletModuleName(ip->GetDir(APP_MAX_SYS_ROOT_DIR));
	plugletModuleName.Append(sXmlMaterialModuleName);
	HRESULT hr = CreatePluglet(CLSID_XmlMaterial, __uuidof(IVIZPointerClient), (void**)ppClient, plugletModuleName.data());
	if(SUCCEEDED(hr) && *ppClient)
	{
		VARIANT vIp, vImpIp;
		VariantInit(&vIp);
		VariantInit(&vImpIp);
		V_VT(&vIp) = VT_BYREF;
		V_BYREF(&vIp) = ip;
		// We don't have access to an import interface ptr here.
		HRESULT hr = (*ppClient)->InitPointers(vIp, vImpIp);
	}
	return hr;
#endif
}

static const USHORT kScratchLibChunk = 0xe430;
static _bstr_t sMaterialsTag(_T("Materials"));

// These are ReferenceMaker methods but they are called when
// the GUP is saved, which is not in the reference hierarchy.
// These methods cannot use the reference hierarchy for
// saving and loading data.
IOResult CMaxMaterialCollection::Save(ISave *isave)
{
	// Since the library can't be saved using the reference
	// hierarchy, we will use XML. This is imperfect, but
	// for the tool palette it should be enough, since the basis of
	// materials in the tool palette is also XML.

	// Don't save anything we don't need to.
	if (mpScratchMtlLib == NULL || mpScratchMtlLib->Count() <= 0)
		return IO_OK;

	// Get the VIZ Importer/Exporter
	CComPtr<IVIZPointerClient> pPointerClient;
	HRESULT hr = GetXMLImpExp(&pPointerClient);
	if(hr != S_OK)
		return IO_ERROR;

	// Get the export interface
	CComQIPtr<IMaterialXmlExport> pILib = pPointerClient;
	ATLASSERT(pILib);
	if(!pILib)
		return IO_ERROR;

	// Create an XML document
	CComPtr<IXMLDOMDocument> doc;
	hr = doc.CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER);
	if (hr != S_OK || doc == NULL)
		return IO_ERROR;

	CComPtr<IXMLDOMElement> materials;
	hr = doc->createElement(sMaterialsTag.GetBSTR(), &materials);
	if (FAILED(hr) || materials == NULL)
		return IO_ERROR;
	hr = doc->putref_documentElement(materials);
	if (FAILED(hr))
		return IO_ERROR;

#ifdef TP_SINGLE_DYNAMIC_MTL_PALETTE
	//WIP The pesistence of scratch materials needs to be rethought. I'd like
	//to save the dynamic catalog from ToolPaletteGUP. Dynamic tools should be coupled
	//to their scene item through perssited monikers. The persistence of the "items" should 
	//be completely independent. I think this means the scratchlib should be referenced by 
	//the scene. 

	//The immediate issue is that when using a single palette for all the dynamic materials,
	//inscene materials are being saved twice. We'll try to fix this by deleting them first. 
	//At this point the refhierarchy is already saved so we should be ok.
	deleteMaterials(true, true, false);

#endif

	//if the scratch palette contains materials with real references to them (e.g. Filelink)
	//then they could potentiall be saved twice: once by the ref hierarchy and once in xml. 
	//Therefore we remove them now
	//deleteReferencedScratchMaterials();

    hr = Fire_OnSaveScratchMaterials(this, materials);
	if (FAILED(hr))
		return IO_ERROR;
	if (hr != S_OK)
		return IO_OK;		// Nothing to save

	// Now read the data from the XML document and write it
	// to the MAX data stream
	_bstr_t xml;
	hr = doc->get_xml(xml.GetAddress());
	if (hr != S_OK)
		return IO_ERROR;

	isave->BeginChunk(kScratchLibChunk);
	IOResult res = isave->WriteCString(LPCTSTR(xml));
	isave->EndChunk();
	return res;
}

static IOResult ImportLibrary(ILoad* iload, MtlBaseLib& lib)
{
	// Get the VIZ Importer/Exporter
	CComPtr<IVIZPointerClient> pPointerClient;
	HRESULT hr = CMaxMaterialCollection::GetXMLImpExp(&pPointerClient);
	if(hr != S_OK)
		return IO_ERROR;

	// Get the export interface
	CComQIPtr<IXmlMaterial> pIMtl = pPointerClient;
	ATLASSERT(pIMtl);
	if(!pIMtl)
		return IO_ERROR;

	// Create an XML document
	CComPtr<IXMLDOMDocument> doc;
	hr = doc.CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER);
	if (hr != S_OK || doc == NULL)
		return IO_ERROR;

	char* xmlStr = NULL;
	IOResult res = iload->ReadCStringChunk(&xmlStr);
	if (res != IO_OK)
		return res;
	if (xmlStr == NULL)
		return IO_OK;
	_bstr_t xml(xmlStr);

	VARIANT_BOOL result;
	hr = doc->loadXML(xml.GetBSTR(), &result);
	if (hr != S_OK || result == 0)
		return IO_ERROR;

	CComBSTR query = "./Materials/Material";
	CComPtr<IXMLDOMNodeList> list;
	hr = doc->selectNodes(query, &list);
	if (hr != S_OK || list == NULL)
		return IO_ERROR;

	long i, len = 0;
	hr = list->get_length(&len);
	if (hr != S_OK)
		return IO_ERROR;

	long failed = 0;
	for (i = 0; i < len ; ++i) {
		CComPtr<IXMLDOMNode> node;
		hr = list->get_item(i, &node);
		if (hr == S_OK && node != NULL) {
			VARIANT vResult;
			vResult.vt = VT_BYREF;
			vResult.byref = NULL;
			hr = pIMtl->ImportMaterial(node, &vResult);
			if(SUCCEEDED(hr) && vResult.vt == VT_BYREF && vResult.byref != NULL)
			{
				lib.Add(static_cast<Mtl*>(vResult.byref));
			}
			else
				++failed;
		}
	}
	
	return failed == 0 ? IO_OK : IO_ERROR;
}

// These are ReferenceMaker methods but they are called when
// the GUP is saved, which is not in the reference hierarchy.
// These methods cannot use the reference hierarchy for
// saving and loading data.
IOResult CMaxMaterialCollection::Load(ILoad *iload)
{
	// Since the library can't be loaded using the reference
	// hierarchy, we will use XML. This is imperfect, but
	// for the tool palette it should be enough, since the basis of
	// materials in the tool palette is also XML.
	IOResult res;

	//JH 3/1/05 if we're getting loaded we can safely toss the scratch materials
	//we need to do this always now, because they won't be deleted in the PRE_OPEN
	//processing.
	deleteMaterials(false, false, true); 

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case kScratchLibChunk: {
				// Ignore errors
				//deleteMaterials(false, false, true); //JH 3/1/05 moving this above
				ImportLibrary(iload, theScratchMtlLib);
			} break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

HRESULT CMaxMaterialCollection::MakeWrapperObject(Mtl* pMtl)
{
	if (pMtl == NULL )
		return E_INVALIDARG;

	if(s_InMakeWrapper)
	{
		//prevent recursion
		ATLASSERT(0);
		return S_FALSE;
	}

	s_InMakeWrapper = true;

	HRESULT hr = RecursiveMakeWrapperObject(pMtl);

	s_InMakeWrapper = false;
	return hr;
}

HRESULT CMaxMaterialCollection::RecursiveMakeWrapperObject(Mtl* pMtl){

	HRESULT hr = S_OK;
	if(pMtl)
	{
		if (pMtl->IsMultiMtl()) {
#ifdef TP_SUPPRESS_MULTI_EXPANSION
			//just make a wrapper on the multi itself.
			hr = MakeWrapperObjectLeaf(pMtl);
#else 
			int count = pMtl->NumSubMtls();
			for (int i = 0;
				i < count && SUCCEEDED(hr = RecursiveMakeWrapperObject(pMtl->GetSubMtl(i)));
				++i) {}
#endif
		}
		else 
		{
			hr = MakeWrapperObjectLeaf(pMtl);
		}
	}
	return hr;
}

HRESULT CMaxMaterialCollection::MakeWrapperObjectLeaf(Mtl *pMtl)
{
	HRESULT hr;
	DWORD_PTR key = (DWORD_PTR) pMtl;

	if(mMtlMap.find(key) == mMtlMap.end()) {
		//Create the wrapper
		CComObject<CMaxMaterial> *pWrapperObject;
		hr = CComObject<CMaxMaterial>::CreateInstance(&pWrapperObject);
		if(SUCCEEDED(hr)) {
			//Initalize the new wrapper
			pWrapperObject->setCollection(this);
			pWrapperObject->setMtl(pMtl);
		}
	}
	else 
		hr = S_FALSE;

	return hr;
}


void CMaxMaterialCollection::OnSceneLibChanged()
{
	//figure out which materials were added or deleted and fire appropriate events.
	//this is currently pretty inefficient since we hav to walk the entire list of materials to 
	//figure out which ones to add or remove. Could improve this by adding better messagin from 
	//mpSceneLib

	if(s_PreventRecursion)
		return;
	s_PreventRecursion = true;

	int numLibMtls = mpSceneMtlLib->Count();
	//int numMtlsWrapped = mMtlMap.size();

	//adding material
	if(numLibMtls > mLastSceneMtlLibSize )
	{

		for(int i=0; i< numLibMtls; i++)
		{
			//get the max material to be wrapped
			MtlBase *pMtl = mpSceneMtlLib->operator[](i);
			//assert(pMtl); //can this be empty?
			//the assertion is no good because a makerefrestore may be setting the ref from
			//lib to material to null. The lib notifies us.
			if(pMtl == NULL || pMtl->SuperClassID() != MATERIAL_CLASS_ID)
				continue;

			//TODO validate the downcast
			HRESULT hr = MakeWrapperObject(static_cast<Mtl*>(pMtl));
			assert(SUCCEEDED(hr));
			if (!SUCCEEDED(hr))
			{
				s_PreventRecursion = false;
				return;
			}

		}
	}
	//removing material
	/*
	We no longer track removals other than to update our record of the new size (mLastSceneMtlLibSize)
	The idea is that we keep all of our wrappers alive unless explicitly destroyed. 
	We assume that all new materials will pass through the scenematerials lib via OkMtlForScene or some equivalent method.

	else if(numLibMtls < mLastSceneMtlLibSize )
	{
		ifxClxnType::iterator it=mMtlMap.begin();
		while(it!=mMtlMap.end())
		{
			Mtl *pMtlToCheck = (Mtl *) (*it).first;
			if(mpSceneMtlLib->FindMtl(pMtlToCheck) == -1)//no longer in scene
			{
				pMtlWrapper.Attach((*it).second);
#ifdef XXXRECYCLE_MATS
			//add the wrapper to our data structure
				mRecycledMtlMap.insert(*it);
				pMtlWrapper.p->AddRef();
#endif
				Fire_OnMaterialRemoved(this, pMtlWrapper);
				it = mMtlMap.erase(it);
				pMtlWrapper.Release();
			}
			else
				it++;
		}
	}
	*/
	//update our record of the size of sceneMtlLib
	mLastSceneMtlLibSize = numLibMtls;
	s_PreventRecursion = false;
/* TO DO implement an integrity checker
#ifndef NDEBUG
	else
		CheckIntegrity();
#endif
		*/
}

void CMaxMaterialCollection::OnScratchLibChanged()
{
#ifdef PERSIST_SCRATCH

	//figure out which materials were added or deleted and fire appropriate events.
	//this is currently pretty inefficient since we hav to walk the entire list of materials to 
	//figure out which ones to add or remove. Could improve this by adding better messagin from 
	//mpSceneLib

	if(s_PreventRecursion)
		return;
	s_PreventRecursion = true;

	int numLibMtls = mpScratchMtlLib->Count();
	//int numMtlsWrapped = mMtlMap.size();

	//adding material
	if(numLibMtls > mLastScratchMtlLibSize )
	{

		for(int i=0; i< numLibMtls; i++)
		{
			//get the max material to be wrapped
			MtlBase *pMtl = mpScratchMtlLib->operator[](i);
			assert(pMtl); //can this be empty?
			if(pMtl == NULL || pMtl->SuperClassID() != MATERIAL_CLASS_ID)
				continue;

			DWORD_PTR key = (DWORD_PTR) pMtl;

			if(mMtlMap.find(key)!= mMtlMap.end())
				continue;//found

			//Create the wrapper
			CComObject<CMaxMaterial> *pWrapperObject;
			HRESULT hr = CComObject<CMaxMaterial>::CreateInstance(&pWrapperObject);
			assert(SUCCEEDED(hr));
			if(!SUCCEEDED(hr))
			{
				s_PreventRecursion = false;
				return;
			}

			//Initalize the new wrapper
			pWrapperObject->setCollection(this);
			//TODO validate the downcast
			pWrapperObject->setMtl((Mtl *)pMtl);

		}
	}
	s_PreventRecursion = false;
	mLastScratchMtlLibSize = numLibMtls;
#endif
}

#include "..\..\..\..\include\Filelinkapi.h"
#include "..\..\..\..\include\Filelinkapi\LinkTableRecord.h"


//from DwgTableRecord.h
#define DTR_MTLTABLE_REF 3
#define DWGTABLERECORD_CLASSID Class_ID(0x2a080200, 0x496b6ca2)

//from VzMaterialTable.h
#define VZMATERIALTABLE_CLASSID Class_ID(0x5640052e, 0x31d7012e)


void CMaxMaterialCollection::OnFileLinkMtlsChanged()
{


	//look for any new materials (i.e unwrapped) and add them
	//existing ones should be updated

	IVizLinkTable* linktbl = 
    static_cast<IVizLinkTable*>(GetCOREInterface(FILELINKMGR_INTERFACE_ID));
	if(!linktbl)
		return;

	//Iterate through the linked materials. For any new material, create a COM wrapper.
	//Creating the wrapper will also fire and event to the palette system.
	int numlinks = linktbl->NumLinkedFiles();
	IVizLinkTable::Iterator it;

	for(int j =0; j<numlinks; j++)
	{
		if(!linktbl->GetLinkID(j, it))
			continue;
		LinkTableRecord *pLTR = linktbl->RecordAt(it);
		if((pLTR == NULL) || (pLTR->ClassID() != DWGTABLERECORD_CLASSID))
			continue; 
		ReferenceMaker* pMtls = pLTR->GetReference(DTR_MTLTABLE_REF);
		if((pMtls == NULL) || (pMtls->ClassID() != VZMATERIALTABLE_CLASSID))
			continue;


		int numMtls = pMtls->NumRefs();
		for(int i=0; i< numMtls; i++)
		{
			//get the max material to be wrapped
			ReferenceTarget *rtarg = pMtls->GetReference(i);
			if((rtarg == NULL) || (rtarg->SuperClassID() != MATERIAL_CLASS_ID))
				continue;
			Mtl* pMtl = (Mtl*)rtarg;
			assert(pMtl);

			DWORD_PTR key = (DWORD_PTR) pMtl;

			CMaxMaterial* pWrapper = findItemKey(key);
			if (pWrapper != NULL)
			{
				pWrapper->CheckReference(false, true);
#ifdef TP_SUSPEND_FOR_FILELINK
				//processed so remove it from the cache
				mCacheLinkedMtls.erase(key);
#endif
				continue;
			}

			//Create the wrapper
			CComObject<CMaxMaterial> *pWrapperObject;
			HRESULT hr = CComObject<CMaxMaterial>::CreateInstance(&pWrapperObject);
			assert(SUCCEEDED(hr));
			if(!SUCCEEDED(hr))
			{
				s_PreventRecursion = false;
				return;
			}

			//Initalize the new wrapper
			pWrapperObject->setCollection(this);
			//TODO validate the downcast
			pWrapperObject->setMtl((Mtl *)pMtl);

		}//next mtl
	}//next link
	
#ifdef TP_SUSPEND_FOR_FILELINK

	//Note that if a material is removed by filelink, we should really remove entirely
	//anything left in mCacheLinkedMtls at this point is a candidate for destructo
	//or at least an update

	ifxClxnType::iterator iter=mCacheLinkedMtls.begin();
	while(iter!=mCacheLinkedMtls.end())
	{
		MtlWrapper *pWrapper = (*iter).second;
		assert(pWrapper);

		OnMtlDeleted(pWrapper, (*iter).first);
//			pMtlWrapper->CheckReference(false, true);
		iter++;
	}
	mCacheLinkedMtls.clear();
#endif
}


#ifdef TP_SUSPEND_FOR_FILELINK
void CMaxMaterialCollection::OnPreFileLinkEvent()
{
	//stash all FileLinked materials int a tab. When the event is complete,
	//we need to update refcounts and notify any clients or removals

	//assert(mCacheLinkedMtls.size() == 0);
	if(mCacheLinkedMtls.size() != 0)
		return; //this can happen on PRE_NEW which spawns a PRE_BIND, no need to collect again

	IVizLinkTable* linktbl = 
    static_cast<IVizLinkTable*>(GetCOREInterface(FILELINKMGR_INTERFACE_ID));
	if(!linktbl)
		return;

	//Iterate through the linked materials. For any new material, create a COM wrapper.
	//Creating the wrapper will also fire and event to the palette system.
	int numlinks = linktbl->NumLinkedFiles();
	IVizLinkTable::Iterator it;

	for(int i =0; i<numlinks; i++)
	{
		if(!linktbl->GetLinkID(i, it))
			continue;
		LinkTableRecord *pLTR = linktbl->RecordAt(it);
		if((pLTR == NULL) || (pLTR->ClassID() != DWGTABLERECORD_CLASSID))
			continue; 
		ReferenceMaker* pMtls = pLTR->GetReference(DTR_MTLTABLE_REF);
		if((pMtls == NULL) || (pMtls->ClassID() != VZMATERIALTABLE_CLASSID))
			continue;


		int numMtls = pMtls->NumRefs();

		//enlarge our tab
//		mCacheLinkedMtls.Resize(mCacheLinkedMtls.Count() + numMtls);
		
		for(int i=0; i< numMtls; i++)
		{
			//get the max material to be wrapped
			ReferenceTarget *rtarg = pMtls->GetReference(i);
			if((rtarg == NULL) || (rtarg->SuperClassID() != MATERIAL_CLASS_ID))
				continue;
			Mtl* pMtl = (Mtl*)rtarg;
			assert(pMtl);

			//find the wrapper, which we should have already
			ifxClxnType::iterator it = mMtlMap.find((DWORD_PTR)pMtl);
			if(it != mMtlMap.end())
				mCacheLinkedMtls.insert(*it);
			else 
				assert(0);

			//mCacheLinkedMtls.Append(1, &pMtl);
		}//next mtl
	}//next link
}
#endif


//stop gap method
//if the material editor becomes visible and teh material in it is not
//already wrapped we better add it. 
void CMaxMaterialCollection::OnMeditVisible()
{
	if(!m_suspendCount)
	{
		MtlBase *m = GetMtlEditInterface()->GetCurMtl();
		if(m->SuperClassID()!=MATERIAL_CLASS_ID)
			return;
		CComPtr<IMaxMaterial> pWrapper;
		FindItemKey((DWORD_PTR)(m), &pWrapper.p);
		if(!pWrapper)
		{
			MakeWrapperObject((Mtl*)m);
			FindItemKey((DWORD_PTR)(m), &pWrapper.p);
			if(pWrapper)
				AddPersistence(pWrapper);
		}
	}
}

STDMETHODIMP CMaxMaterialCollection::get_count(short *pVal)
{

	*pVal = static_cast<short>(mMtlMap.size());
	return S_OK;
}

STDMETHODIMP CMaxMaterialCollection::getMaterial(int which, IMaxMaterial **ppMtl)
{
	// TODO: This is O(n), make it constant time by addin a different collection type or
	//fiure out hoe to get random access from a map
	if(*ppMtl != NULL)
		return E_INVALIDARG;

	ifxClxnType::iterator it=mMtlMap.begin();
	int i= 0;
	while(it!=mMtlMap.end() && i<which)
	{
		if(i == which)
		{
			//11/13/02*ppMtl = (*it).second;
			(*it).second->QueryInterface(ppMtl);
			assert(*ppMtl);
			//11/13/02(*ppMtl)->AddRef();//copying an interface pointer
			return S_OK;
		}
		it++;//just advance the iterator to the requested entry
		i++;//and our counter
	}

	return E_INVALIDARG;
}

STDMETHODIMP CMaxMaterialCollection::FindItem(/*[in]*/ GUID id, /*[out, retval]*/ IMaxMaterial **ppMtl)
{
	if(*ppMtl != NULL)
		return E_INVALIDARG;

	CMaxMaterial* pMtl = findItem(id);
	if (pMtl == NULL)
		return S_FALSE;
	return pMtl->QueryInterface(__uuidof(IMaxMaterial), reinterpret_cast<void**>(ppMtl));
}

CMaxMaterial* CMaxMaterialCollection::findItem(const GUID& id)
{
	// TODO: This is O(n)
	// Can we gain anything by maintanig maps indexed by guid
	ifxClxnType::iterator it = mMtlMap.begin();
	GUID MtlId;
	for ( ; it != mMtlMap.end(); ++it)
	{
		it->second->get_ID(&MtlId);
		if(MtlId == id)
			return it->second;
	}

	return NULL;
}

//static which forwards to instance
void CMaxMaterialCollection::NotifyProc(void *param, NotifyInfo *info)
{
		CMaxMaterialCollection *pinst = (CMaxMaterialCollection *) param;
		pinst->OnMaxNotify(info);
}

static bool sbDelSceneMtls;

void CMaxMaterialCollection::OnMaxNotify(NotifyInfo *info)
{
	switch(info->intcode)
	{
	case NOTIFY_SCENE_UNDO:
	case NOTIFY_SCENE_REDO:
		{
#ifdef SUSPEND_UNDO
			Resume();
#endif

			ifxClxnType::iterator p = mMtlMap.begin();
			while (p != mMtlMap.end()) {
				ifxClxnType::iterator del = p;		// Just in case p is removed from the map
				++p;
				del->second->CheckReference(true, true);
			}

			//2/21/05 also check for changes to source libs. DID627361

			// do this first, it may destroy some wrappers which will need to be recreated.
			// Consider the case of a Filelink bind, at the end of which, Filelink has no more
			// link-created materials. These wrappers get explicitly removed (consider revising with
			// a ref-check). By doing this first, we get a chance to recreate them in OnSceneLibChanged
			OnFileLinkMtlsChanged(); 

			OnSceneLibChanged();
			OnScratchLibChanged();
		} break;

	case NOTIFY_FILE_PRE_OPEN:
		if (info->callParam)
		{
			DWORD iotype = *(DWORD*)(info->callParam);
			if(iotype == IOTYPE_RENDER_PRESETS)
				return;
		}
//	case NOTIFY_SYSTEM_PRE_NEW: //moved below
	case NOTIFY_SYSTEM_PRE_RESET:
		{
			sbDelSceneMtls = true;
		Suspend();
		}

		break;
	//next come all the PRE events


	// if the user chooses to keep geometry, filelink may perform a bind
        // so we want this here so that we fall into the FileLink cases
	case NOTIFY_SYSTEM_PRE_NEW: 
		{
			// The scene materials are usually tossed
			// The scratch materials are not preserved between different files. 
			// The recycled materials are always tossed
			sbDelSceneMtls = true;
			if (info->intcode == NOTIFY_SYSTEM_PRE_NEW )
			{
				if (info->callParam)
				{
					DWORD flags = *(DWORD*)(info->callParam);
					if(flags == PRE_NEW_KEEP_OBJECTS || flags == PRE_NEW_KEEP_OBJECTS_AND_HIERARCHY)
						sbDelSceneMtls = false;
				}

			}
		}
		//fall into


#ifdef SUSPEND_UNDO
	//these must go before the Filelink pre-notifications because we want to
	//fall into OnPreFileLinkEvent. There's no way to know if the undo/redo will
	//be filelink relevant, so we have to assume it will be and get a snapshot of 
	//the filelink materials
	case NOTIFY_SCENE_PRE_UNDO:
	case NOTIFY_SCENE_PRE_REDO:
		//fall into
#endif



#ifdef TP_SUSPEND_FOR_FILELINK
	case NOTIFY_FILELINK_PRE_ATTACH:
	case NOTIFY_FILELINK_PRE_BIND:		
	case NOTIFY_FILELINK_PRE_DETACH:
	case NOTIFY_FILELINK_PRE_RELOAD:
		OnPreFileLinkEvent();
#endif
	case NOTIFY_FILE_PRE_MERGE:
	case NOTIFY_PRE_IMPORT:
	case NOTIFY_FILE_PRE_SAVE:
	case NOTIFY_TOOLPALETTE_MTL_SUSPEND:
	case NOTIFY_PRE_NODES_CLONED:
		Suspend();
		break;

	case NOTIFY_SYSTEM_POST_NEW:
	case NOTIFY_FILE_POST_OPEN:
	if (info->callParam)
	{
		DWORD iotype = *(DWORD*)(info->callParam);
		if(iotype == IOTYPE_RENDER_PRESETS)
			return;
	}
	case NOTIFY_SYSTEM_POST_RESET:
		{
			//JH 3/1/05 destroy most materials. In the case of POST_OPEN, we skip
			//the scratch materials because they will be tossed in the ::Load method
			//and if we delete them now, we are deleting the new ones and they will be lost.
			deleteMaterials(true, sbDelSceneMtls, (info->intcode != NOTIFY_FILE_POST_OPEN));
		}
		//fall into


    case NOTIFY_FILE_OPEN_FAILED:
	case NOTIFY_FILE_POST_MERGE:
	case NOTIFY_FILE_POST_SAVE:
	case NOTIFY_POST_IMPORT:
	case NOTIFY_TOOLPALETTE_MTL_RESUME:
#ifdef TP_SUSPEND_FOR_FILELINK
	case NOTIFY_FILELINK_POST_ATTACH:
	case NOTIFY_FILELINK_POST_BIND:	
	case NOTIFY_FILELINK_POST_DETACH:
	case NOTIFY_FILELINK_POST_RELOAD:
#endif
		{

		if(Resume())
		{
			// do this first, it may destroy some wrappers which will need to be recreated.
			// Consider the case of a Filelink bind, at the end of which, Filelink has no more
			// link-created materials. These wrappers get explicitly removed (consider revising with
			// a ref-check). By doing this first, we get a chance to recreate them in OnSceneLibChanged
			OnFileLinkMtlsChanged(); 

			OnSceneLibChanged();
			OnScratchLibChanged();
		}
		}
		break;
	case NOTIFY_POST_NODES_CLONED:
		{
			Resume();
			
			ifxClxnType::iterator p = mMtlMap.begin();

			while (p != mMtlMap.end()) {
				ifxClxnType::iterator del = p;		// Just in case p is removed from the map
				++p;
				del->second->CheckReference(true, true);
			}
		}
		break;

	case NOTIFY_MEDIT_SHOW:
		{
			OnMeditVisible();
		}
		break;
	default:
		break;
	}
}

//called by a wrapper when node count leaves 0 
void CMaxMaterialCollection::OnMtlAddedToScene(IMaxMaterial * pMtl)
{
	Fire_OnMaterialAssigned(this, pMtl);
}

//called by a wrapper when node count reaches 0 
void CMaxMaterialCollection::OnMtlRemovedFromScene(IMaxMaterial * pMtl)
{
	Fire_OnMaterialUnAssigned(this, pMtl);
}

//called by wrapper when release by editor
void CMaxMaterialCollection::OnMtlEndEdit(IMaxMaterial *pMtl)
{
	Fire_OnMaterialChanged(this, pMtl);
}

//called by wrapper when release by editor
void CMaxMaterialCollection::OnMtlChanged(IMaxMaterial *pMtl)
{
	Fire_OnMaterialChanged(this, pMtl);
}

//called by a wrapper when its material self-destructs
void CMaxMaterialCollection::OnMtlDeleted(IMaxMaterial * pMtl, DWORD_PTR key)
{
	ifxClxnType::iterator it = mMtlMap.find(key);
	if(it != mMtlMap.end())
	{
		Fire_OnMaterialRemoved(this, pMtl);
		it = mMtlMap.erase(it);
		pMtl->Release();//should destroy both the material and the wrapper
	}
}

bool CMaxMaterialCollection::AddMtl(MtlWrapper* pMtl, DWORD_PTR key)
{
	CComPtr<IMaxMaterial> pMtlWrapper = pMtl;
	assert(pMtlWrapper);

	//add the wrapper to our data structure
	mMtlMap.insert(ifxClxnType::value_type(key, pMtl));

	//if this material already has clients dependents
	//go ahead and notify any clients
	//for instance this happens after a load
	//it only doesn't happen when a new material is being added to the scene from scratch
	short vScene, vRefs;
	HRESULT hr = pMtlWrapper->IsReferenced(&vRefs);
	hr = pMtlWrapper->InScene(&vScene);
	if(vScene != 0)
		Fire_OnMaterialAssigned(this, pMtlWrapper);
	else if(vRefs != 0)//TODO invent another message?
		Fire_OnMaterialAssigned(this, pMtlWrapper);

	pMtlWrapper.Detach(); //don't release the pointer
	return true;
}

STDMETHODIMP CMaxMaterialCollection::emptyTrash(void)
{
	return deleteMaterials(true, false, false);
}

HRESULT CMaxMaterialCollection::deleteMaterials(bool recycle, bool scene, bool scratch)
{
	HRESULT hr = S_OK;
	//walk our map and remove any wrappers on unreferenced materials
	CComPtr<IMaxMaterial> pMtlWrapper;

	ifxClxnType::iterator it=mMtlMap.begin();
	short numRefs, numNodeRefs;
	bool hasDependents, inScene;
	while(it!=mMtlMap.end())
	{
		pMtlWrapper.Attach((*it).second);
		hr = pMtlWrapper->IsReferenced(&numRefs);
		ATLASSERT(SUCCEEDED(hr));
		hasDependents = numRefs > 0;
		if (1) {//compute this always now
			hr = pMtlWrapper->InScene(&numNodeRefs);
			ATLASSERT(SUCCEEDED(hr));
			inScene = numNodeRefs > 0;
		}

		if ((scene && inScene)
				|| (scratch && !inScene && hasDependents)
				|| (recycle && !inScene && !hasDependents))
		{
			Fire_OnMaterialRemoved(this, pMtlWrapper);
			it = mMtlMap.erase(it);
			pMtlWrapper.Release();//should destroy both the material and the wrapper
		}
		else
		{
			pMtlWrapper.Detach();
			it++;
		}
	}

	if (scratch) {
		theScratchMtlLib.DeleteAll();
		mLastScratchMtlLibSize = 0;
	}
	if (scene) {
		mLastSceneMtlLibSize = 0;
	}

	return hr;
}

void CMaxMaterialCollection::deleteReferencedScratchMaterials()
{
	//loop through the scratch lib and delete any with non-trivial references
	MtlBase *m = NULL;
	for(int i=0; i<theScratchMtlLib.NumChildren();i++)
	{
		m = theScratchMtlLib[i];
		if(m)
		{
			//look up the wrapper in our map
			CComPtr<IMaxMaterial> pMtlWrapper = NULL;
			HRESULT hr = FindItemKey((DWORD_PTR)m, &pMtlWrapper.p);
			if(SUCCEEDED(hr))
			{
				ATLASSERT(pMtlWrapper);
				short numRefs;
				hr = pMtlWrapper->IsReferenced(&numRefs);

				//if the refcount is greater than 1, remove it
				if(SUCCEEDED(hr) && numRefs>1)
				{
					Fire_OnMaterialRemoved(this, pMtlWrapper);
					//all that we require is that the tool be removed from palette, this should be done now
					//we could also remove from the scratchlib and from our map
				}
			}
		}
	}
}


STDMETHODIMP CMaxMaterialCollection::FindItemKey(DWORD_PTR key, IMaxMaterial ** ppMtl)
{
	if(*ppMtl != NULL)
		return E_INVALIDARG;

	CMaxMaterial* pMtl = findItemKey(key);
	if (pMtl == NULL)
		return S_FALSE;
	return pMtl->QueryInterface(__uuidof(IMaxMaterial), reinterpret_cast<void**>(ppMtl));
}

CMaxMaterial* CMaxMaterialCollection::findItemKey(DWORD_PTR key)
{
	ifxClxnType::iterator it = mMtlMap.find(key);
	if(it != mMtlMap.end())
		return it->second;

	return NULL;
}

void CMaxMaterialCollection::switchMaterial(CMaxMaterial* wrapper, Mtl* mtl)
{
	mMtlMap.erase((DWORD_PTR)(wrapper->GetReference(REFIDX_MTL)));
	wrapper->ReplaceReference(REFIDX_MTL, mtl);
	mMtlMap.insert(ifxClxnType::value_type((DWORD_PTR)(wrapper->GetReference(REFIDX_MTL)),
		static_cast<MtlWrapper*>(wrapper)));
	OnMtlChanged(wrapper);
}



STDMETHODIMP CMaxMaterialCollection::AddPersistence(IMaxMaterial * pMtl)
{
#ifdef PERSIST_SCRATCH

	if(mpScratchMtlLib)
	{
		ifxClxnType::iterator it=mMtlMap.begin();
		GUID MtlId, id;
		pMtl->get_ID(&id);
		while(it!=mMtlMap.end())
		{
			MtlWrapper *pm = (*it).second;
			pm->get_ID(&MtlId);
			if(MtlId == id)
			{
				USHORT ct = pm->AddPersistence();
				if(ct ==1)
				{
#ifndef DID486021 //removing this, we don't want this inside this method (which now gets called often)
				  //rather we create the undo object at some specific instances where this method is called
					theHold.Begin();
					theHold.Put(new PersistMtlRestore(mpScratchMtlLib, pm->getMtl()));
					theHold.Accept(GetString(IDS_TOGGLE_PERSIST));
#endif
					mpScratchMtlLib->Add(pm->getMtl());

				}
				return S_OK;
			}
			it++;
		}
	}
#endif
	return S_FALSE;
}

STDMETHODIMP CMaxMaterialCollection::RemovePersistence(IMaxMaterial * pMtl)
{
#ifdef PERSIST_SCRATCH

	if(mpScratchMtlLib)
	{
		ifxClxnType::iterator it=mMtlMap.begin();
		GUID MtlId, id;
		pMtl->get_ID(&id);
		while(it!=mMtlMap.end())
		{
			MtlWrapper *pm = (*it).second;
			pm->get_ID(&MtlId);
			if(MtlId == id)
			{
				USHORT ct = pm->RemovePersistence();
				if(ct ==0)
				{
#ifndef DID486021 //removing this, we don't want this inside this method (which now gets called often)
				  //rather we create the undo object at some specific instances where this method is called
					theHold.Begin();
					theHold.Put(new PersistMtlRestore(mpScratchMtlLib, pm->getMtl()));
					theHold.Accept(GetString(IDS_TOGGLE_PERSIST));
#endif
					//2/23/05 don't all DeleteRefRestores for this reference removal
					//They are unreliable on tabs of references
					theHold.Suspend();

					mpScratchMtlLib->Remove(pm->getMtl());

					theHold.Resume();//2/23/05
				}
				return S_OK;
			}
			it++;
		}
	}
#endif
	return S_FALSE;
}



static SClass_ID GetNodeType(INode *node)
	{
	TimeValue t = GetCOREInterface()->GetTime();
	ObjectState os = node->EvalWorldState( t,  FALSE);
	if (os.obj) {
		SClass_ID id  = os.obj->SuperClassID();
//		if (node->IsTarget()) {
		if (node->IsTarget() && 
			node->GetLookatNode() && 
			node->GetLookatNode()->Selected()) {
			
			INode* depNode = node->GetLookatNode();
			if (depNode) {
				ObjectState osc = depNode->EvalWorldState(t,FALSE);
				return osc.obj->SuperClassID();		
				}
			else return SClass_ID(0);
			}
		else  
			return id;
		}
	else 
		return SClass_ID(0);
	}

STDMETHODIMP CMaxMaterialCollection::IsSelectionValid(VARIANT_BOOL* vResult)
{
	Interface* ip = GetCOREInterface();

	*vResult = VARIANT_FALSE;

	/*
	if(ip->GetSelNodeCount())
		*vResult = VARIANT_TRUE;
		*/

    int n = 0;
	int count = ip->GetSelNodeCount();
	INode *pNode;
    for (int i=0; i<count; i++) {
		pNode = ip->GetSelNode(i);
		if(pNode == NULL)
			continue;
        SClass_ID id = GetNodeType(pNode);
        if (id==SHAPE_CLASS_ID) n++;
        else if (id==GEOMOBJECT_CLASS_ID) {
            if (!pNode->IsTarget()) n++;
            }
        }
 
	if(n>0)
		*vResult = VARIANT_TRUE;


	return S_OK;
}

#endif	// EXTENDED_OBJECT_MODEL
