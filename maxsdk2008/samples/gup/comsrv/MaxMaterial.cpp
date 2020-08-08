// MaxMaterial.cpp : Implementation of CMaxMaterial
#include "stdafx.h"
#include "Comsrv.h"
#include "MaxMaterial.h"
#include "systemutilities.h"

#include <set>
#include "modstack.h"

#define DID485967
#define USE_PNG
#define DROPFROMSCRATCH

#define NO_WRAPPERS_IN_HOLD
/* 2/22/05 Apparently we've been allowing our reference to the material to be 
recorded by a MakeRefRestore if some enclosing operation turns on the hold.(e.g. Filelink). 
This is bad since wrappers can't live in the hold: their lifetimes are managed as 
COM objects rather than as ReferenceMakers. In fact undoing such a makerefrestore will trigger 
destruction of the wrapper and a subsequent redo will crash on the destroyed RefMaker
We now suspend the hold around making and removing our reference to the material.
*/

#ifdef EXTENDED_OBJECT_MODEL

#include "MaxMaterialCollection.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include <comdef.h>
#include <comutil.h>

//#ifndef RENDER_VER
#include "imtledit.h"
//#endif

//we create an XMLMAterial importer, need its interface and guids
#include "xmlmtl.h"

#ifdef MTLEDITOR_CONVERT_TO_ARCHMAT
// We need MaxScript to convert to Architectural materials
// This sample cannot be built with just the SDK when
// MTLEDITOR_CONVERT_TO_ARCHMAT is defined.
#include "maxscrpt/maxscrpt.h"
#include "maxscrpt/thunks.h"
#include "maxscrpt/maxmats.h"

#include "../../../../cfgmgr/cfgmgr.h"
#endif	// MTLEDITOR_CONVERT_TO_ARCHMAT

extern TCHAR *GetString(int id);

#define COMSRV_INI_FILENAME    _T("Comsrv.ini")
#define COMSRV_INI_SECTION     _T("Dont't show this again")
#define COMSRV_INI_PROMPT_PUTMTL   _T("Prompt on PutMaterial")

#ifdef DID485967
class IsSuperMtlProc : public DependentEnumProc
{
public:
	IsSuperMtlProc(ReferenceTarget* t):rtarg(t), found(false){};
	ReferenceTarget *rtarg;
	bool found;
	virtual int proc(ReferenceMaker *rmaker) {
		if(rmaker->SuperClassID() != MATERIAL_CLASS_ID)
			return DEP_ENUM_SKIP;
		else if(rmaker == rtarg)
		{
			found = true;
			return DEP_ENUM_HALT;
		}
		else
			return DEP_ENUM_CONTINUE;
	}

};
#endif //DID


//we want to keep track of when references to our inner material are made and removed
//The normal refernce system notification is disabled in IMtl.cpp
//We'll use a notifaction approach to restore this, I added the broadcast calls to Imtl.cpp
//Declare two callbacks which route notifications back through the NotifyDependents method
//of the correct wrapper.

static void MtlRefAddedProc(void *param, NotifyInfo *info) {
	CMaxMaterial *rMaker = (CMaxMaterial *) param;//this points to one of us
	ReferenceTarget *rTarg = (ReferenceTarget*) info->callParam;//this should be a Mtl*
	if(rTarg ==  rMaker->GetReference(REFIDX_MTL))
	{
		PartID PartID = PART_ALL;
		rMaker->NotifyRefChanged(FOREVER, rTarg, PartID, REFMSG_REF_ADDED);
	}
#ifdef DID485967
	else if(rMaker->IsSubMtl() && rMaker->GetReference(REFIDX_MTL))
	{
		IsSuperMtlProc proc(rTarg);
		rMaker->mpMtl->DoEnumDependents(&proc);
		if(proc.found)
		{
			PartID PartID = PART_ALL;
			rMaker->NotifyRefChanged(FOREVER, rTarg, PartID, REFMSG_REF_ADDED);
		}
	}
#endif
}

static void MtlRefDeletedProc(void *param, NotifyInfo *info) {
	CMaxMaterial *rMaker = (CMaxMaterial *) param;//this points to one of us
	ReferenceTarget *rTarg = (ReferenceTarget*) info->callParam;//this should be a Mtl*
	if(rTarg ==  rMaker->GetReference(REFIDX_MTL))
	{
		PartID PartID = PART_ALL;
		rMaker->NotifyRefChanged(FOREVER, rTarg, PartID, REFMSG_REF_DELETED);
	}
#ifdef DID485967
	else if(rMaker->IsSubMtl() && rMaker->GetReference(REFIDX_MTL))
	{
		IsSuperMtlProc proc(rTarg);
		rMaker->mpMtl->DoEnumDependents(&proc);
		if(proc.found)
		{
			PartID PartID = PART_ALL;
			rMaker->NotifyRefChanged(FOREVER, rTarg, PartID, REFMSG_REF_DELETED);
		}
	}
#endif
}


#define TESTCOLOR Color(1.0f, 0.0f, 1.0f)
//#define _ANIMATE_SAMPLE
/////////////////////////////////////////////////////////////////////////////
// CMaxMaterial
// a COM wrapper on a max material.

bool CMaxMaterial::sbClassInitialized = false;
DWORD CMaxMaterial::sdwPutMtlNoPrompt = 0;


CMaxMaterial::CMaxMaterial()
: m_currentFrame(0)
{
	LoadClassDefaults();
	mpMtl = NULL;
	mpCollection = NULL;
	CoCreateGuid(&mID);
	mbSuspended = true;//cleared when fully initialized in SetMtl
	mbEditing = false;//set in begin edit cleared in end edit
	mcNodeRefs = 0;
	mcMaxRefs = 0;
	mdwKey = 0;
	mcPersistCt = 0;
}

CMaxMaterial::~CMaxMaterial()
{
	UnRegisterNotification(MtlRefAddedProc, (void *)this, NOTIFY_MTL_REFADDED);
	UnRegisterNotification(MtlRefDeletedProc, (void *)this, NOTIFY_MTL_REFDELETED);
	mbSuspended = true;

#ifdef NO_WRAPPERS_IN_HOLD
		theHold.Suspend();
#endif

	DeleteAllRefsFromMe();

#ifdef NO_WRAPPERS_IN_HOLD
		theHold.Resume();
#endif
}

//class initializer
void CMaxMaterial::LoadClassDefaults()
{
	if(!sbClassInitialized)
	{
		//initialze class statics once
		
		//stuff initialized from the ini
		ReadINI();

		sbClassInitialized = true;
	}
}

void
CMaxMaterial::GetINIFilename(TCHAR* filename)
{
    if (!filename) {
        assert(FALSE);
    }

    _tcscpy(filename, GetCOREInterface()->GetDir(APP_PLUGCFG_DIR));

    if (filename)
        if (_tcscmp(&filename[_tcslen(filename)-1], _T("\\")))
            _tcscat(filename, _T("\\"));
    _tcscat(filename, COMSRV_INI_FILENAME);   
}

void
CMaxMaterial::ReadINI()
{
    TCHAR filename[MAX_PATH];
    TCHAR buffer[256];
    GetINIFilename(filename);

    GetPrivateProfileString(COMSRV_INI_SECTION, COMSRV_INI_PROMPT_PUTMTL, buffer, buffer, sizeof(buffer), filename);
    sdwPutMtlNoPrompt = (DWORD) atoi(buffer);
}


void
CMaxMaterial::WriteINI()
{
    TCHAR filename[MAX_PATH];
    TCHAR buffer[256];
    GetINIFilename(filename);
    
    wsprintf(buffer,"%d", sdwPutMtlNoPrompt);
    WritePrivateProfileString(COMSRV_INI_SECTION, COMSRV_INI_PROMPT_PUTMTL, buffer, filename);

}

/////////////////////////////////////////////////////////////////////////////////////////////
//IMaxMaterial methods

STDMETHODIMP CMaxMaterial::assignSelected()
{
	Interface *ip = GetCOREInterface();
	int count = ip->GetSelNodeCount();
	if(mpMtl && count)
	{
	    TSTR name = mpMtl->GetName();
	
		updateRefCounts();

		mpCollection->Suspend();

		Tab<Mtl*> mtls;

		mtls.Resize(count);
		for(int i = 0; i< count; i++)
		{
			INode *node = ip->GetSelNode(i);
/*			mtls[i] = node->GetMtl();
				if(mtls[i])
					mtls[i]->SetAFlag(A_LOCK_TARGET);
*/
			Mtl* m = node->GetMtl();
			if(m)
				RecursiveAddMtl(m, mtls);
		}
					
		BOOL ok = ip->OkMtlForScene(mpMtl);

		if(ok)
		{

			theHold.Begin();
#ifdef DROPFROMSCRATCH
			if (mcMaxRefs != 0) {
				// If we are droping from the scratch palette,
				// remove it from the scratch lib
#ifdef DID486021
				theHold.Put(new PersistMtlRestore(mpCollection->mpScratchMtlLib, mpMtl));
#endif
				mpCollection->RemovePersistence(this);
			}
#endif //DROPFROMSCRATCH

			for(int i = 0; i< count; i++)
			{
				INode *node = ip->GetSelNode(i);
				node->SetMtl(mpMtl);
			}
			theHold.Accept(GetString(IDS_SET_MATERIAL));
		}
		else
		{
			mpCollection->Resume();
			return S_FALSE;
		}
		if (mpCollection->Resume()) {
			// We need to add the new material to the scene palette,
			// and we need to check whether the original materials
			// need to be deleted.
			CheckReference(true, false);

			//DID638932
			//user may have changed name through dialog
			TSTR newname = mpMtl->GetName();
			if(newname != name)
				mpCollection->OnMtlChanged(this);


			//3/6/03 Fixing DID489010 we we're looping with the wrong limits
			for (int i = 0; i < mtls.Count(); ++i) {
				Mtl* mtl = mtls[i];
				if (mtl != NULL) {
					CheckReference(mtl);
				}
			}
		}

		//TODO JH 8/18/02
		//need a way to clean up the scene matlib here

		//TODO (JH 7/30/02) Done By FC
		//can this be done differently?
		//ip->RedrawViews(ip->GetTime());
		ip->ForceCompleteRedraw();
	}

	return S_OK;
}

void CMaxMaterial::RecursiveAddMtl(Mtl *pMtl, Tab<Mtl*>& mtls)
{
	if(pMtl)
	{
		mtls.Append(1, &pMtl, 8);
		pMtl->SetAFlag(A_LOCK_TARGET);

#ifdef TP_SUPPRESS_MULTI_EXPANSION
	return;
#endif
		if(pMtl->IsMultiMtl())
		{
			int numsubs = pMtl->NumSubMtls();
			for(int i = 0;i<numsubs; i++)
				RecursiveAddMtl(pMtl->GetSubMtl(i), mtls);
		}
	}

}

void CMaxMaterial::CheckReference(Mtl *pMtl)
{
	if(pMtl == NULL)
		return;

	CMaxMaterial* mtlWrap = mpCollection->findItemKey(
		reinterpret_cast<DWORD_PTR>(pMtl));
	if (mtlWrap != NULL)
		mtlWrap->CheckReference(false, true);
	pMtl->ClearAFlag(A_LOCK_TARGET);
}

STDMETHODIMP CMaxMaterial::loadXML(IUnknown *pXMLNode)
{

	//validate input
	if(!pXMLNode)
		return E_INVALIDARG;
	mpXML = pXMLNode;
	if(!mpXML)
		return E_NOINTERFACE;

#if 0
	USES_CONVERSION;
	CComBSTR temp;
	mpXML->get_xml(&temp);
	MessageBox(NULL, OLE2T(temp.m_str), NULL, MB_OK);
#endif

	Mtl *pMtl = NULL;
	CComPtr<IVIZPointerClient> pPointerClient;
	HRESULT hr = CMaxMaterialCollection::GetXMLImpExp(&pPointerClient);
	if(!SUCCEEDED(hr))
	{
		return E_FAIL;
	}

	CComQIPtr<IXmlMaterial> pIMtl = pPointerClient;
	ATLASSERT(pIMtl);
	if(!pIMtl)
		return E_FAIL;

	VARIANT vResult;
	hr = pIMtl->ImportMaterial(mpXML, &vResult);
	if(SUCCEEDED(hr) && vResult.vt == VT_BYREF)
	{
		pMtl = (Mtl *) vResult.byref;
	}
	else
		return E_FAIL;
	//End temporary


	if(pMtl)
	{
		setMtl(pMtl);
		if(GetReference(REFIDX_MTL))
			return S_OK;

#if 0
		//an optimization. If we're loading xml, it might contain the location
		//of a usable thumbnail. If so, initialize the thumbnail cache with this location
		CComBSTR query = "./@thumbnail";
		CComPtr<IXMLDOMAttribute> pThumbAtt;
		hr = mpXML->selectSingleNode(query, &pThumbAtt);
		if (hr = S_OK && pThumbAtt)
		{
			hr = pThumbAtt->get_value(&val);
		}
#endif
	}
	return E_FAIL;
}

static BOOL RemoveChildren(IXMLDOMNode* pNode)
{
	BOOL ret = FALSE;
    try {
        ATLASSERT(pNode != NULL);
        if (pNode != NULL) {
			CComPtr<IXMLDOMNodeList> pChildren; 
			HRESULT hr = pNode->get_childNodes(&pChildren);
			ATLASSERT( SUCCEEDED(hr) && (pChildren != NULL));
			if ( SUCCEEDED(hr) && (pChildren != NULL)) {
				CComPtr<IXMLDOMNode> pChildNode;
				HRESULT hr;
				for (hr = pChildren->nextNode(&pChildNode); hr == S_OK && pChildNode != NULL;
						pChildNode = NULL, hr = pChildren->nextNode(&pChildNode)){
					CComPtr<IXMLDOMNode> pRemove;
					pNode->removeChild(pChildNode, &pRemove);
					// Release it
					pRemove = NULL;
				}
				ret = TRUE;
			}
		}
    } catch (_com_error  &e) {
        ATLASSERT(FALSE);
        SetLastError(e.Error());
    } catch (...) {
    }

    return ret;
}

STDMETHODIMP CMaxMaterial::saveXML(IUnknown* pXMLNode, VARIANT_BOOL noPrompts)
{
	//validate input
	if(!pXMLNode || !mpMtl)
		return E_INVALIDARG;
	CComQIPtr<IXMLDOMNode> pXML = pXMLNode;
	if(!pXML)
		return E_NOINTERFACE;

	CComPtr<IVIZPointerClient> pPointerClient;
	HRESULT hr = CMaxMaterialCollection::GetXMLImpExp(&pPointerClient);
	if(!SUCCEEDED(hr))
	{
		return E_FAIL;
	}

	CComQIPtr<IMaterialXmlExport2> pIMtl = pPointerClient;
	ATLASSERT(pIMtl);
	if(!pIMtl)
		return E_FAIL;

	CComPtr<IXMLDOMDocument> doc;
	_bstr_t empty("");
	pXML->get_ownerDocument(&doc);
	hr = pIMtl->InitMtlExport(doc, empty.GetBSTR());
	VARIANT_BOOL f = 0;
	hr = pIMtl->InitThumbnailPrefs(f, empty.GetBSTR(), empty.GetBSTR());

	RemoveChildren(pXML);

	VARIANT vMtl;
	vMtl.vt = VT_BYREF;
	vMtl.byref = mpMtl;
	hr = pIMtl->ExportMaterial(pXML, vMtl, noPrompts);
	//if (FAILED(hr))
	if(hr != S_OK)
		return hr;

	//THE NODE PASSED IN IS THE DATA NODE.
	//what we really want is the child
	CComPtr<IXMLDOMNode> materials, material, child, parent;
	CComBSTR query = "./Materials";
	hr = pXML->selectSingleNode(query, &materials.p);
	ATLASSERT(SUCCEEDED(hr));
	if (hr != S_OK) {
		RemoveChildren(pXML);
		return hr;
	}

	if(mpMtl->IsMultiMtl())
	{
		//for multi, save the materials node itself
		hr = materials->get_parentNode(&parent.p);
		ATLASSERT(SUCCEEDED(hr));
		hr = parent->removeChild(materials, &child);
		ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr)) {
			RemoveChildren(pXML);
			return hr;
		}
	}
	else
	{ //just save a single material node
		query = "./Material";
		hr = materials->selectSingleNode(query, &material.p);
		//ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr)) {
			RemoveChildren(pXML);
			return hr;
		}
		
		hr = materials->removeChild(material, &child);
		ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr)) {
			RemoveChildren(pXML);
			return hr;
		}
	}

	//our data is now in child
	material = NULL;
	materials = NULL;
	RemoveChildren(pXML);
	hr = pXML->appendChild(child, &material.p);
	ATLASSERT(SUCCEEDED(hr));
	return hr;
}

///////////////////////////////////////////////////////////////////////////////////
//ReferenceMaker methods
RefResult CMaxMaterial::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message)
{
	if (message != REFMSG_TARGET_DELETED && (mpCollection == NULL || mpCollection->IsSuspended()))
		return REF_SUCCEED;

	//TODO (JH 6/9/02 Done by CC)Handle ref mssgs
	//what to do?
	if(hTarget == mpMtl && !mbSuspended)
	{
	switch(message)
	{
		case REFMSG_CHANGE:
			mPSCache.Invalidate();
		// Fall into...
		case REFMSG_NODE_NAMECHANGE:
			mpCollection->OnMtlChanged(this);

			break;
		case REFMSG_TARGET_DELETED:
			mpMtl = NULL;
			//mignotc : fix for : 770626
			//Its too restrictive to only check for remove when ref deleted.
			CheckReference(true, true);
			return REF_STOP;
			break;
		case REFMSG_REF_DELETED:
			if(mbEditing)
			{
				if(!confirmEditing())
					EndEdit();
			}
			//mignotc : fix for : 770626
			//Its too restrictive to only check for remove when ref deleted.
			CheckReference(true, true);
			break;

		case REFMSG_REF_ADDED:
			//mignotc : fix for : 770626
			//Its too restrictive to only check for add when ref added.
			CheckReference(true, true);
			break;
	}
	}
#ifdef DID485967 //process message coming from containing materials
	else if(!mbSuspended)
	{
		switch(message)
		{
		case REFMSG_REF_DELETED:
			if(mbEditing)
			{
				if(!confirmEditing())
					EndEdit();
			}
			//mignotc : fix for : 770626
			//Its too restrictive to only check for remove when ref deleted.
			CheckReference(true, true);
			break;

		case REFMSG_REF_ADDED:
			//mignotc : fix for : 770626
			//Its too restrictive to only check for add when ref added.
			CheckReference(true, true);
			break;
		}
	}
#endif //DID485967 //process message coming from containing materials


	return REF_SUCCEED;
}

void CMaxMaterial::CheckReference(bool added, bool removed)
{
	USHORT cNodeRefsLast = mcNodeRefs;
	USHORT cMaxRefsLast = mcMaxRefs;

	if(mbSuspended)//not fully initialized
		return;
	updateRefCounts();

	//mignotc : fix for : 770626
	//This is more to notify of more bugs in the future.
	//If removed is false but we actually lost a reference, this means that we are
	//losing a message and that the Scene Inused Unused palette have a risk of not being up to date.
	//And same for added.
	DbgAssert(!(!removed && (mcNodeRefs < cNodeRefsLast)));
	DbgAssert(!(!added && (mcNodeRefs > cNodeRefsLast)));


	if(mpCollection)
	{

		if(removed)
		{
			if(mbSuspended)//bottom-up death
			{
				mpCollection->OnMtlDeleted(this, mdwKey);

				//we just got destroyed, get outta here
				return;
			}
			// If the node refcount or max refcount transition to zero
			// then this material may need to be moved to another dynamic palette.

#ifdef TP_DYNAMIC_TOOL_OVERLAYS
			//11/07/04 I'm sending additional messages whenever noderefs changes rather
			//than just when they transition to/from 0. This allows use count overlays
			//to be updated
			else if((mcNodeRefs < cNodeRefsLast)|| (mcMaxRefs == 0 && cMaxRefsLast))
#else
			else if((mcNodeRefs == 0 && cNodeRefsLast)|| (mcMaxRefs == 0 && cMaxRefsLast))
#endif //TP_DYNAMIC_TOOL_OVERLAYS
				mpCollection->OnMtlRemovedFromScene(this);
		}
		if (added)
		{
			// If the node refcount or max refcount transition to non-zero
			// then this material may need to be moved to another dynamic palette.

#ifdef TP_DYNAMIC_TOOL_OVERLAYS
			//11/07/04 I'm sending additional messages whenever noderefs changes
			if((mcNodeRefs > cNodeRefsLast)|| (mcMaxRefs && cMaxRefsLast == 0))
#else
			if((mcNodeRefs  && cNodeRefsLast == 0)|| (mcMaxRefs && cMaxRefsLast == 0))
#endif //TP_DYNAMIC_TOOL_OVERLAYS
				mpCollection->OnMtlAddedToScene(this);
		}
	}
}
	
int CMaxMaterial::NumRefs()
{
	return NUMREFS;
}

RefTargetHandle CMaxMaterial::GetReference(int i)
{
	if(i<0 || i>= NUMREFS)
	{
		ATLASSERT(0);//bad index
		return NULL;
	}
	switch(i)
	{
	case REFIDX_MTL:
		return (RefTargetHandle) mpMtl;
		break;
	default:
		ATLASSERT(0);
		return NULL;
	}
}
	
void CMaxMaterial::SetReference(int i, RefTargetHandle rtarg)
{
	if(i<0 || i>= NUMREFS)
	{
		ATLASSERT(0);//bad index
		return;
	}
	switch(i)
	{
	case REFIDX_MTL:
		{
#ifdef _DEBUG
			if(rtarg)
				ATLASSERT(rtarg->SuperClassID() == MATERIAL_CLASS_ID);
#endif
			mpMtl = (Mtl*) rtarg;
		}
		break;
	default:
		ATLASSERT(0);
		break;
	}
}



STDMETHODIMP CMaxMaterial::getDynPropManagers(VARIANT *pObjUnkArray, VARIANT *pDynUnkArray)
{
	//TO DO
	//note the first arg is unused and should be removed
	//creates a safe array of IPropertyManager2 interfaces and return thems in pDynUnkArray

	ATLASSERT(pDynUnkArray != NULL);

	HRESULT hr;

	//Create an inner safe array to hold the PropertyManager(s)
	SAFEARRAYBOUND saBound;
	saBound.lLbound = 0;
	saBound.cElements = 1;//TODO should correspond to the number of PB2s


    SAFEARRAY *psaPMArray = SafeArrayCreate(VT_VARIANT, 1, &saBound);
	if(!psaPMArray)
		return E_FAIL;
	for (long i=0; i<saBound.cElements; i++)
	{
		//TODO wrap a variant around the unknown of the IPopertyManager2
        hr = SafeArrayPutElement(psaPMArray, &i, &CComVariant((IUnknown*) NULL));
		if(!SUCCEEDED(hr))
			return hr;
	}

	pDynUnkArray->vt = VT_ARRAY | VT_VARIANT;
	pDynUnkArray->parray = psaPMArray;

	return S_OK;
}

//DEL void CMaxMaterial::showData()
//DEL {
//DEL }

void CMaxMaterial::setCollection(CMaxMaterialCollection *pCol)
{
	ATLASSERT(pCol);
	ATLASSERT(mpCollection == NULL);//don't allow changing
	mpCollection = pCol;
}

//final initialization, sets the object we wrap
//side effect of adding us to our collection
void CMaxMaterial::setMtl(Mtl *pMat)
{

	ATLASSERT(pMat);
	RefResult res;

#ifdef NO_WRAPPERS_IN_HOLD
		theHold.Suspend();
#endif

	ATLASSERT(GetReference(REFIDX_MTL) == NULL);//Don't allow this
	res = ReplaceReference(REFIDX_MTL, pMat);

	ATLASSERT(res == REF_SUCCEED);

#ifdef NO_WRAPPERS_IN_HOLD
		theHold.Resume();
#endif

	updateRefCounts();

	mdwKey = (DWORD_PTR) pMat;
	if(mpCollection)
		mpCollection->AddMtl(static_cast<CMaxMaterialCollection::MtlWrapper *>(this), mdwKey);

	mbSuspended = false;//enable message processing

	RegisterNotification(MtlRefAddedProc, (void *)this, NOTIFY_MTL_REFADDED);
	RegisterNotification(MtlRefDeletedProc, (void *)this, NOTIFY_MTL_REFDELETED);

}

STDMETHODIMP CMaxMaterial::get_Name(BSTR *pVal)
{
	USES_CONVERSION;
	if(mpMtl)
	{
		TSTR name = mpMtl->GetName();
		CComBSTR bName;
		bName = name;
		bName.CopyTo(pVal);
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CMaxMaterial::put_Name(BSTR newVal)
{
	USES_CONVERSION;
	mpMtl->SetName(OLE2T(newVal));
	return S_OK;
}

STDMETHODIMP CMaxMaterial::get_ThumbnailFile(BSTR *pVal)
{
#ifdef _ANIMATE_SAMPLE
		m_currentFrame = 0;
		mpMtl->meditRotate.Identity();
#endif
	USES_CONVERSION;
	if(mpMtl)
	{
		TSTR basePath;
		basePath.Resize(MAX_PATH);
#ifdef RENDER_VER
        GetSpecDir(APP_CATALOGS_DIR, _T("Catalogs\\vizRWorkspace"), basePath.data());
#else
        GetSpecDir(APP_CATALOGS_DIR, _T("Catalogs\\Workspace"), basePath.data());
#endif //RENDER_VER
		mpCollection->Suspend();
		HRESULT hr = mPSCache.updateCache(basePath, mpMtl, mID);
		mpCollection->Resume();
		ATLASSERT(SUCCEEDED(hr));
		if(SUCCEEDED(hr))
		{
			bool got = mPSCache.getPath(pVal);

			return got?S_OK:E_UNEXPECTED;
		}
	}
	return S_FALSE;
}

UINT Hash(char *key)
{
	UINT hash = 0;
	while (*key) 
	{
		hash = (hash<<5)^(hash>>27)^(*key++);		
	}
	return hash;
}

TCHAR* GetNameInCache(TCHAR* fname, unsigned int frame)
{
    static TCHAR buffer[MAX_PATH+1];
    ZeroMemory(buffer, sizeof(buffer));

#ifndef USE_PNG
    _stprintf(buffer, "%s.%d.bmp", fname, frame);
#else
    _stprintf(buffer, "%s.%d.png", fname, frame);
#endif

    return buffer;
}

PBITMAPINFO CreateBitmapInfoStruct( HBITMAP hBmp)
{ 
    BITMAP bmp; 
    PBITMAPINFO pbmi; 
    WORD    cClrBits; 

    // Retrieve the bitmap color format, width, and height. 
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
       ATLASSERT(0); 

    // Convert the color format to a count of bits. 
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

    // Allocate memory for the BITMAPINFO structure. (This structure 
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD 
    // data structures.) 

     if (cClrBits != 24) 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER) + 
                    sizeof(RGBQUAD) * (1<< cClrBits)); 

     // There is no RGBQUAD array for the 24-bit-per-pixel format. 

     else 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER)); 

    // Initialize the fields in the BITMAPINFO structure. 

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = bmp.bmWidth; 
    pbmi->bmiHeader.biHeight = bmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
    if (cClrBits < 24) 
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

    // If the bitmap is not compressed, set the BI_RGB flag. 
    pbmi->bmiHeader.biCompression = BI_RGB; 

    // Compute the number of bytes in the array of color 
    // indices and store the result in biSizeImage. 
    // For Windows NT, the width must be DWORD aligned unless 
    // the bitmap is RLE compressed. This example shows this. 
    // For Windows 95/98/Me, the width must be WORD aligned unless the 
    // bitmap is RLE compressed.
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                  * pbmi->bmiHeader.biHeight; 
    // Set biClrImportant to 0, indicating that all of the 
    // device colors are important. 
     pbmi->bmiHeader.biClrImportant = 0; 
     return pbmi; 
 } 

#ifdef USE_PNG
#include <gdiplus.h>
//using namespace Gdiplus;
#pragma comment(lib , "gdiplus")
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

   Gdiplus::GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

bool CreatePNGFile(LPTSTR pszFile, int s, 
                  LPBYTE lpBits)
{

   // Create a Bitmap object.
   Gdiplus::Bitmap bitmap(s, s, ((3*s) +3 )/4 * 4, PixelFormat24bppRGB, lpBits);

   //I don't know why but this image is upside down now
	bitmap.RotateFlip(Gdiplus::RotateNoneFlipY);

   // Save the altered image.
   CLSID pngClsid;
   int res = GetEncoderClsid(L"image/png", &pngClsid);
   if(res<0)
	   return false;
   USES_CONVERSION;
   bitmap.Save(T2CW(pszFile), &pngClsid, NULL);
   return true;

}
#endif

void CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, 
                  LPBYTE lpBits) 
 { 
     HANDLE hf;                 // file handle 
    BITMAPFILEHEADER hdr;       // bitmap file-header 
    PBITMAPINFOHEADER pbih;     // bitmap info-header 
 //   LPBYTE lpBits;              // memory pointer 
    DWORD dwTotal;              // total count of bytes 
    DWORD cb;                   // incremental count of bytes 
    BYTE *hp;                   // byte pointer 
    DWORD dwTmp; 

    pbih = (PBITMAPINFOHEADER) pbi; 

/* now passed in
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);
*/
    if (!lpBits) 
         ATLASSERT(0); 

	/*
    // Retrieve the color table (RGBQUAD array) and the bits 
    // (array of palette indices) from the DIB. 
    if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, 
        DIB_RGB_COLORS)) 
    {
         ATLASSERT(0); 
    }
*/
    // Create the .BMP file. 
    hf = CreateFile(pszFile, 
                   GENERIC_READ | GENERIC_WRITE, 
                   (DWORD) FILE_SHARE_WRITE | FILE_SHARE_READ, 
                    NULL, 
                   OPEN_ALWAYS, 
                   FILE_ATTRIBUTE_ARCHIVE, 
                   (HANDLE) NULL); 
    if (hf == INVALID_HANDLE_VALUE) 
	{
         ATLASSERT(0); 
		 DWORD errcode = GetLastError();
	}

    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
    // Compute the size of the entire file. 
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
                 pbih->biSize + pbih->biClrUsed 
                 * sizeof(RGBQUAD) + pbih->biSizeImage); 
    hdr.bfReserved1 = 0; 
    hdr.bfReserved2 = 0; 

    // Compute the offset to the array of color indices. 
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
                    pbih->biSize + pbih->biClrUsed 
                    * sizeof (RGBQUAD); 

    // Copy the BITMAPFILEHEADER into the .BMP file. 
    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), 
        (LPDWORD) &dwTmp,  NULL)) 
    {
         ATLASSERT(0); 
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file. 
    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) 
                  + pbih->biClrUsed * sizeof (RGBQUAD), 
                  (LPDWORD) &dwTmp, ( NULL))) 
        ATLASSERT(0); 

    // Copy the array of color indices into the .BMP file. 
    dwTotal = cb = pbih->biSizeImage; 
    hp = lpBits; 
    if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)) 
         ATLASSERT(0); 

    // Close the .BMP file. 
     if (!CloseHandle(hf)) 
         ATLASSERT(0); 

    // Free memory. 
   // GlobalFree((HGLOBAL)lpBits);
}




STDMETHODIMP CMaxMaterial::get_ID(GUID *pVal)
{
	if(!pVal)
		return E_INVALIDARG;
	*pVal = mID;

	return S_OK;
}

//TO DO
//Is this used?
STDMETHODIMP CMaxMaterial::getThumbnailBytes(int sz, SAFEARRAY **psa)
{
	*psa = NULL;
	//cache the Postage stamp if not already there
	mpCollection->Suspend();
	mPSCache.updatePSByteCache(sz, mpMtl);
	mpCollection->Resume();
	int size = mPSCache.getByteCount();

/*
	//TO do 
	// do we really need to copy the bytes again?
	*psa = SafeArrayCreateVector(VT_UI1,0,size);
	ATLASSERT(*psa);
	BYTE *data;
	SafeArrayAccessData(*psa,(void**)&data);
	ATLASSERT(data);

	//copy from the cache to the safearray
	UBYTE *p = mPSCache.getBytes();
	memcpy(data, p, size);
	SafeArrayUnaccessData(*psa);
	*/
	*psa = mPSCache.getBytes();
	return (*psa)?S_OK:E_FAIL;
}


CMaxMaterial::ThumbnailCache::~ThumbnailCache()
{
	USES_CONVERSION;
//clean out the cache
	Invalidate();//side effect of cleaning cache

	if(mPSByteCache)
	{
		//delete[] mPSByteCache;
		SafeArrayUnlock(mPSByteCache);
		SafeArrayDestroy(mPSByteCache);
		mPSByteCache = NULL;
	}

}

void CMaxMaterial::ThumbnailCache::Invalidate()
{
	USES_CONVERSION;
	framePathType::iterator it = mframePaths.begin();
	while(it != mframePaths.end())
	{
		ATLASSERT((*it).second);
		if(PathFileExists(OLE2A((*it).second)))
		{
			BOOL res = DeleteFile(OLE2A((*it).second));
			if(!res)
			{
				DWORD foo = GetLastError();
				foo = 0;
			}
		}
		it++;
	}
	mframePaths.clear();

	if(mPSByteCache)
	{
		//delete[] mPSByteCache;
		//mPSByteCache = NULL;
		mPSByteCount = 0;
	}

}

class FreeAllMapsRefEnum: public RefEnumProc {
	ReferenceMaker *mSkipRef; 
public:
	FreeAllMapsRefEnum()
	{
		//2/18/05 We now skip the actively edited material ( see DID 602250)
		mSkipRef = NULL;
		IMtlEditInterface *pMedit = GetMtlEditInterface();
		if(pMedit)
			mSkipRef = (ReferenceMaker *)(pMedit->GetCurMtl());
	}
	int proc(ReferenceMaker *rm) 
	{
		if(rm != mSkipRef)
			rm->FreeAllBitmaps(); 
		return REF_ENUM_CONTINUE;
	}
};

void RecursiveFreeAllBitmaps(MtlBase *mb)
{
	FreeAllMapsRefEnum freeEnum;
	if (mb) 
		mb->EnumRefHierarchy(freeEnum, 
							true,	// include custom attribs
							true,	// include indirect refs
							true,	// include non-persistent refs
							false); // prevent duplicates via flag
}

HRESULT CMaxMaterial::ThumbnailCache::updateCache(TSTR& basePath, Mtl *pMtl, const GUID& id, unsigned int frame)
//function to write a thumbnail image file using the Postage Stamp bytes of the material
{
	USES_CONVERSION;
	HRESULT hr;

	if(!pMtl)
		return E_INVALIDARG;

	framePathType::iterator it = mframePaths.find(frame);
	if(it != mframePaths.end())
		return S_OK;
//	if(mValid)
//		return S_OK;

//	hr = updatePSByteCache(PS_LARGE, pMtl);
//	if(!SUCCEEDED(hr))
//		return hr;

	PStamp *ps = pMtl->CreatePStamp(PS_LARGE,TRUE);  // create and render a large pstamp
	ATLASSERT(ps);
	RecursiveFreeAllBitmaps(pMtl);
	if(!ps)
		return E_FAIL;

	int d = PSDIM(PS_LARGE);
	int scanw = ByteWidth(d);
	int bytecount = scanw*d;
	UBYTE *work = new UBYTE[bytecount];

	ps->GetImage(work);

#ifndef USE_PNG
	HBITMAP Hmap = CreateBitmap(PSDIM(PS_LARGE), PSDIM(PS_LARGE), 1, 24, (void *)work);
	if(!Hmap)
		ATLASSERT(0);

	PBITMAPINFO pbi = CreateBitmapInfoStruct(Hmap);
#endif
    if(!basePath.length())
	#ifdef RENDER_VER
        GetSpecDir(APP_CATALOGS_DIR, _T("Catalogs\\vizRWorkspace"), basePath.data());
	#else
        GetSpecDir(APP_CATALOGS_DIR, _T("Catalogs\\Workspace"), basePath.data());
	#endif //RENDER_VER

	DbgAssert(PathFileExists(basePath));
	if(!PathFileExists(basePath))
	{
		return E_FAIL;
	}

		
    BSTR bstrGuid;
    hr = StringFromCLSID(id, &bstrGuid);
    ATLASSERT(SUCCEEDED(hr));
    TSTR sId = OLE2A(bstrGuid);
    CoTaskMemFree((LPVOID) bstrGuid);

    TCHAR	path[MAX_PATH];
    _tcscpy(path, basePath.data());
	_tcscat(path, "\\Temp");
	if(!PathFileExists(path))
	{
		int res = CreateDirectory(path, NULL);
		if(!res)
		{
			DWORD errcode = GetLastError();
			return E_FAIL;
		}
	}
	_tcscat(path, "\\");
    _tcscat(path, GetNameInCache(sId, frame));

	//TODO this is temporary
	//it may be that the file is locked and we've been unable to clear
	//the cache, in that case make up yet another name
    HANDLE hf = CreateFile(path, 
                   GENERIC_READ | GENERIC_WRITE, 
                   (DWORD) FILE_SHARE_WRITE | FILE_SHARE_READ, 
                    NULL, 
                   OPEN_ALWAYS, 
                   FILE_ATTRIBUTE_ARCHIVE, 
                   (HANDLE) NULL); 
    while(hf == INVALID_HANDLE_VALUE) 
	{
		GUID newid;
		CoCreateGuid(&newid);
		BSTR bstrGuid;
		hr = StringFromCLSID(newid, &bstrGuid);
		ATLASSERT(SUCCEEDED(hr));
		TSTR sId = OLE2A(bstrGuid);
		CoTaskMemFree((LPVOID) bstrGuid);

		_tcscpy(path, basePath.data());
		_tcscat(path, "\\Catalogs\\Temp");
		_tcscat(path, "\\");
		_tcscat(path, GetNameInCache(sId, frame));

		hf = CreateFile(path, 
					GENERIC_READ | GENERIC_WRITE, 
					(DWORD) FILE_SHARE_WRITE | FILE_SHARE_READ, 
						NULL, 
					OPEN_ALWAYS, 
					FILE_ATTRIBUTE_ARCHIVE, 
					(HANDLE) NULL); 
	}

	 // Close the file. 
     if (!CloseHandle(hf)) 
         ATLASSERT(0); 




#ifndef USE_PNG
	CreateBMPFile(path, pbi, work);
	DeleteObject( Hmap);
	LocalFree( pbi);
#else
	CreatePNGFile(path, d, work);
#endif

    

	mframePaths.insert(framePathType::value_type(frame, CComBSTR(path)));
	//mBSTRPSpath = path;
	delete[] work;
	return S_OK;

}

bool CMaxMaterial::ThumbnailCache::getPath(BSTR *path, unsigned int frame)
{
	framePathType::iterator it = mframePaths.find(frame);
	if(it != mframePaths.end())
	{
		(*it).second.CopyTo(path);
		return true;
	}
	else
		return false;
}

void CMaxMaterial::ThumbnailCache::Init(BSTR path)
{
	Invalidate();
	mframePaths.insert(framePathType::value_type(0, CComBSTR(path)));
}
	

#define BIG_HUNK 524288

HRESULT CMaxMaterial::ThumbnailCache::updatePSByteCache(int sz, Mtl *pMtl)
{
	int req = (sz == 3)?BIG_HUNK:(PSDIM(sz)*ByteWidth(PSDIM(sz)));
	if(mPSByteCount == req)
		return S_OK;


	PStamp *ps = pMtl->CreatePStamp(sz,TRUE);  // create and render a large pstamp
	ATLASSERT(ps);
	RecursiveFreeAllBitmaps(pMtl);
	if(!ps)
		return E_FAIL;

	int d = PSDIM(sz);
	int scanw = ByteWidth(d);
	mPSByteCount = scanw*d;

	/*
	mPSByteCache = new UBYTE[mPSByteCount];
	ATLASSERT(mPSByteCache);
	if (mPSByteCache==NULL)
	{
		return E_OUTOFMEMORY;
		mPSByteCount = 0;
	}
	*/

	if(mPSByteCache)
	{
//		delete[] mPSByteCache;
//		mPSByteCache = NULL;
		mPSByteCount = 0;
		long count;
		SafeArrayGetUBound(mPSByteCache, 1, &count); 
		if(req != count +1)
		{
			SafeArrayUnlock(mPSByteCache);
			SafeArrayDestroy(mPSByteCache);
			mPSByteCache = NULL;
		}
	}

	if(!mPSByteCache)
	{
		mPSByteCache = SafeArrayCreateVector(VT_UI1,0,mPSByteCount);
		if(mPSByteCache)
			SafeArrayLock(mPSByteCache);
		else
			return E_OUTOFMEMORY;
	}

    //ps->GetImage(mPSByteCache);
	UBYTE *temp;
	HRESULT hr = SafeArrayAccessData(mPSByteCache, (void **) &temp);
	if(SUCCEEDED(hr))
		ps->GetImage(temp);
	else return hr;
	hr = SafeArrayUnaccessData(mPSByteCache);

	pMtl->DiscardPStamp(sz);

	return S_OK;

}

int CMaxMaterial::ThumbnailCache::getByteCount()
{
	return mPSByteCount;

}

#define CMD_ASSIGN_SINGLE 1
INode *DropNode(DWORD_PTR hWnd, POINT point)
{
	POINT screen = point;
	// map a point back to screen for calling WindowFromPoint
	// MapWindowPoints((HWND)hWnd, NULL, &screen, 1); //should already be a screen point
	HWND vpwin = WindowFromPoint(screen);
	ViewExp* vp = GetCOREInterface()->GetViewport(vpwin);
	ATLASSERT(vp);
	vpwin = vp->GetHWnd();
	if(vp)
	{
		if (vp->IsEnabled())
		{
			//map from screen to viewport window
			MapWindowPoints(NULL, vpwin, &point, 1); 
			return GetCOREInterface()->PickNode(vpwin, IPoint2(point.x, point.y), NULL);
		}
	}
	return NULL;
}

STDMETHODIMP CMaxMaterial::OnDrop(SHORT nFlag, DWORD_PTR hWnd, DWORD point, DWORD dwKeyState)
{
	ATLASSERT(nFlag);
	int command = CMD_ASSIGN_SINGLE;
	if(nFlag > 1)//left mouse drop
		bool showMenu = true;

	switch(command)
	{
	case CMD_ASSIGN_SINGLE:
		{
			//TODO (JH 7/30/02)Done by FC
			//refactor code in app so that ImportContextNOde works
			//currently this mechanism only works for the default DnD handler
			POINT pt;
			pt.x =  LOWORD(point);
			pt.y =  HIWORD(point);
//			GetCursorPos(&pt);
			Interface *ip = GetCOREInterface();
			INode* node = DropNode(hWnd, pt);
			//if(node)
			if(1)//1/30/03 allow updating the scene with a drop in space
			{
				// Paste a copy of the material, if we are dropping
				// from the scratch palette.
				updateRefCounts();


				Mtl* oldMtl = NULL;
                Mtl* existMtl = NULL;
                Tab<Mtl*> mtls;
				mpCollection->Suspend();

                // MSW 636403
                // does a material exist of the same name..?
                // If so, we will need to check it's references later.
                Interface7 *pInt = GetCOREInterface7();
                TSTR name = mpMtl->GetName();
				existMtl = pInt->FindMtlNameInScene(name);
                if (existMtl && (existMtl != mpMtl)) {
                    RecursiveAddMtl(existMtl, mtls);
                }

				if(node)
				{
					mtls.Resize(1);

                    oldMtl = node->GetMtl();
                    if(oldMtl && (oldMtl != existMtl)) { // check against the existing.. no point adding it twice
						RecursiveAddMtl(oldMtl, mtls);
                    }

					BOOL ok = ip->OkMtlForScene(mpMtl);

					if(ok)
					{

						theHold.Begin();
#ifdef DROPFROMSCRATCH
						if (mcMaxRefs != 0) {
							// If we are droping from the scratch palette,
							// remove it from the scratch lib
#ifdef DID486021
							theHold.Put(new PersistMtlRestore(mpCollection->mpScratchMtlLib, mpMtl));
#endif
							mpCollection->RemovePersistence(this);
						}
#endif //DROPFROMSCRATCH

						node->SetMtl(mpMtl);
						theHold.Accept(GetString(IDS_SET_MATERIAL));
					}
					else
					{
						mpCollection->Resume();
						return S_FALSE;
					}

				}
				else //no node, possibly put the material
				{
                    if(existMtl && existMtl!=mpMtl)
					{                        
						if (!sdwPutMtlNoPrompt) 
						{
							TCHAR buf[MAX_PATH];
							_tcscpy(buf, name);
							_tcscat(buf, GetString(IDS_REPLACE));

							DWORD dontshow = 0;
							if(IDOK == MaxMsgBox(GetCOREInterface()->GetMAXHWnd(), buf,
									GetString(IDS_REPLACE_CAP), MB_ICONQUESTION | MB_APPLMODAL | MB_OKCANCEL, MAX_MB_DONTSHOWAGAIN,
									&dontshow)) 
							{
										theHold.SuperBegin();
#ifdef DROPFROMSCRATCH
										if (mcMaxRefs != 0) {
											// If we are droping from the scratch palette,
											// remove it from the scratch lib
#ifdef DID486021
											theHold.Put(new PersistMtlRestore(mpCollection->mpScratchMtlLib, mpMtl));
#endif
											mpCollection->RemovePersistence(this);
										}
#endif //DROPFROMSCRATCH
										pInt->PutMaterial(mpMtl, existMtl, 0, this);//has built-in undo support
										//also protect oldMtl against delettion

										theHold.SuperAccept(GetString(IDS_PUT_MTL));

							}
							else //cancelled
							{
								mpCollection->Resume();
								return S_FALSE;
							}


							if(dontshow & MAX_MB_DONTSHOWAGAIN)
							{
								sdwPutMtlNoPrompt = dontshow;
								WriteINI();
							}
						}
					}
					else
					{
						TCHAR buf[256];
						_tcscpy(buf, GetString(IDS_HIT_FAILED));
						MessageBox((HWND)hWnd, buf, GetString(IDS_HIT_FAILED_CAPTION), MB_OK);
						mpCollection->Resume();
						return S_FALSE;
					}
				}
				if (mpCollection->Resume()) {
					// We need to add the new material to the scene palette,
					// and we need to check whether the original materials
					// need to be deleted.
					CheckReference(true, false);
					
					//DID638932
					//user may have changed name through dialog
					TSTR newname = mpMtl->GetName();
					if(newname != name)
						mpCollection->OnMtlChanged(this);
					
                    // 636403 (MSW).. material references may need to be checked regardless
                    //   of whether the object being assigned has a previous material.
                    // A case in point is a drawing containing more than one AutoCAD entity.
                    // If a material is assigned to 1 entity, there is no old material, but
                    // also no material reference to remove
                    // If the same material is assigned to a different entity, the original
                    // named material may be replaced and reassigned. As the AutoCAD entity 
                    // does not have an old material though we previously did not recheck the
                    // the material references.
                    if ((NULL != oldMtl) || (NULL != existMtl))
                    {
						for (int i = 0; i < mtls.Count(); ++i) {
							Mtl* mtl = mtls[i];
							if (mtl != NULL) {
								CheckReference(mtl);
							}
						}
					}
				}


				//TODO (JH 7/30/02) Done By FC
				//can this be done differently?
				//ip->RedrawViews(ip->GetTime());
				ip->ForceCompleteRedraw();
			}
			else
			{
				
				MessageBox((HWND)hWnd, GetString(IDS_HIT_FAILED), GetString(IDS_HIT_FAILED_CAPTION), MB_OK);
				return S_FALSE;
			}

		}
		break;
	default:
		ATLASSERT(0);
		break;
	}

	return S_OK;
}

#ifdef MTLEDITOR_CONVERT_TO_ARCHMAT
// CallMaxscript
// Send the string to maxscript 
//
static Mtl* convertMtl(Mtl* mtl, bool replaceRefs = false)
{
	// If the material is NULL just return it.
	if (mtl == NULL)
		return NULL;

	// Magic initialization stuff for maxscript.
	static bool script_initialized = false;
	if (!script_initialized) {
		init_MAXScript();
		script_initialized = TRUE;
	}
	init_thread_locals();
	push_alloc_frame();
	five_value_locals(name, fn, mtl, result, replace);
	save_current_frames();
	trace_back_active = FALSE;

	Mtl* converted = NULL;

	try	{
		// Create the name of the maxscript function we want.
		// and look it up in the global names
		vl.name = Name::intern(_T("convertToArchMat"));
		vl.fn = globals->get(vl.name);

		// For some reason we get a global thunk back, so lets
		// check the cell which should point to the function.
		// Just in case if it points to another global thunk
		// try it again.
		while (vl.fn != NULL && is_globalthunk(vl.fn))
			vl.fn = static_cast<GlobalThunk*>(vl.fn)->cell;

		// Now we should have a MAXScriptFunction, which we can
		// call to do the actual conversion. If we didn't
		// get a MAXScriptFunction, we can't convert.
		if (vl.fn != NULL && vl.fn->tag == class_tag(MAXScriptFunction)) {
			Value* args[4];

			// Ok. convertToArchMat takes one parameter, the material
			// and an optional keyword paramter, replace, which tells
			// convertToArchMat whether to replace all reference to
			// the old material by the new one.
			args[0] = vl.mtl = MAXMaterial::intern(mtl);	// The original material
			args[1] = &keyarg_marker;						// Separates keyword params from mandatory

			// Keyword paramters are given in pairs, first the name
			// followed by the value. Missing keyword parameters
			// are assigned defaults from the function definition.
			args[2] = vl.replace = Name::intern(_T("replace"));	// Keyword "replace"
			args[3] = replaceRefs ? &true_value : &false_value;	// Value true or false based on argument

			// Call the funtion and save the result.
			vl.result = static_cast<MAXScriptFunction*>(vl.fn)->apply(args, 4);

			// If the result isn't NULL, try to convert it to a material.
			// If the convesion fails, an exception will be thrown.
			if (vl.result != NULL)
				converted = vl.result->to_mtl();
		}
	} catch (...) {
		clear_error_source_data();
		restore_current_frames();
		MAXScript_signals = 0;
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
	}

	// Magic Max Script stuff to clear the frame and locals.
	pop_value_locals();
	pop_alloc_frame();

	// converted will be NULL if the conversion failed.
	return converted;
}

static BOOL isMtlPublic(ReferenceTarget* m)
{
	if (m == NULL)
		return FALSE;
	SubClassList *subList = GetCOREInterface()->GetDllDir().ClassDir().GetClassList(m->SuperClassID());
	if (subList) {
		int entry = subList->FindClass(m->ClassID());
		if (entry>=0)
			return (*subList)[entry].IsPublic();
	}

	return FALSE;
}

static TCHAR dontShowArchMatPromp[] = _T("DoNotShowPromptForArchMatConversion");

static bool convertMaterial(Mtl*& m, bool replaceRefs, bool prompt)
{
	if (!isMtlPublic(m)) {
		TCHAR caption[128];

		if (prompt) {
			int skipPrompt = 0;
			DWORD dontshow = 0;
			_tcscpy(caption, GetString(IDS_MTLEDITOR));
			ConfigManager& cfg = getCfgMgr();
			cfg.setSection(caption);
			cfg.getInt(dontShowArchMatPromp, &skipPrompt);
			if (!skipPrompt) {
				switch (MaxMsgBox(GetCOREInterface()->GetMAXHWnd(), GetString(IDS_CONVERT_TO_ARCHMAT),
					caption, MB_ICONQUESTION | MB_APPLMODAL | MB_OKCANCEL, MAX_MB_DONTSHOWAGAIN,
					&dontshow)) {
				case IDOK:
					break;
				default:
#ifdef MTLEDITOR_EDIT_PUBLIC_ONLY
					return false;
#else	// !MTLEDITOR_EDIT_PUBLIC_ONLY
					return true;
#endif	// !MTLEDITOR_EDIT_PUBLIC_ONLY
				}

				if (dontshow & MAX_MB_DONTSHOWAGAIN) {
					cfg.putInt(dontShowArchMatPromp, 1);
				}
			}
			else
				prompt = false;
		}

		// Convert material
		Mtl* arch = convertMtl(m, replaceRefs);
		if (arch != NULL) {
			m = arch;
			return true;
		}

		if (prompt) {
			TCHAR caption[128];
			_tcscpy(caption, GetString(IDS_MTLEDITOR));
			MessageBox(NULL, GetString(IDS_CONVERT_TO_ARCHMAT_FAILED), caption,
				MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
		}

		return false;
	}

	return true;
}

static bool convertMaterial(MtlBase*& m, bool replaceRefs, bool prompt)
{
	if (m->SuperClassID() == MATERIAL_CLASS_ID) {
		Mtl* mm = static_cast<Mtl*>(m);
		if (convertMaterial(mm, replaceRefs, prompt)) {
			m = mm;
			return true;
		}
		return false;
	}

	return true;
}

#define CONVERT_MATERIAL(m, r, p)	convertMaterial((m), (r), (p))
#else	// MTLEDITOR_CONVERT_TO_ARCHMAT
#define CONVERT_MATERIAL(m, r, p)	true
#endif	// MTLEDITOR_CONVERT_TO_ARCHMAT

STDMETHODIMP CMaxMaterial::BeginEdit(VARIANT_BOOL bForceShow)
{
	Interface7 *pInt = GetCOREInterface7();
	ATLASSERT(pInt);
	BOOL bShowing = pInt->IsMtlDlgShowing();

	if(mpMtl && (bShowing || bForceShow == VARIANT_TRUE))
	{
#if 0
		int n = mpMtl->NumParamBlocks();
		for(int i=0; i<n; i++)
		{
			IParamBlock2 *pBlock = mpMtl->GetParamBlock(i);
			if(pBlock)
			{
				IAutoMParamDlg *pdlg = pBlock->GetMParamDlg();
				pdlg->SetThing(mpMtl);
				pdlg->ActivateDlg(1);
			}
		}
#endif
		// Convert the material if needed
		Mtl* newMtl = mpMtl;
		mpCollection->Suspend();
		bool converted = CONVERT_MATERIAL(newMtl, true, true);
		mpCollection->Resume();
		if (!converted)
			return E_FAIL;
		if (mpMtl != newMtl) {

#ifdef NO_WRAPPERS_IN_HOLD
				theHold.Suspend();
#endif

			// The material editor converted the material
			mpCollection->switchMaterial(this, newMtl);

#ifdef NO_WRAPPERS_IN_HOLD
				theHold.Resume();
#endif
		}

		Interface *ip = GetCOREInterface();
#ifdef RENDER_VER
		//VIZRneder has single slot in Medit
		ip->PutMtlToMtlEditor(mpMtl, 0);
#endif
        if(bForceShow == VARIANT_TRUE && !bShowing)
			ip->ExecuteMAXCommand(MAXCOM_MTLEDIT);
#ifndef RENDER_VER
		int slot;
		bool needPut;
		slot = WhichSlot(mpMtl, &needPut);
		if(needPut)
			ip->PutMtlToMtlEditor(mpMtl, slot);
		GetMtlEditInterface()->SetActiveMtlSlot(slot);
		GetMtlEditInterface()->UpdateMtlEditorBrackets();		
#endif
		mbEditing = true;
		return S_OK;
	}
	else
		return S_FALSE;
}



STDMETHODIMP CMaxMaterial::EndEdit(void)
{
	mpCollection->OnMtlEndEdit( this );	
	mbEditing = false;
	//GetCOREInterface()->ForceCompleteRedraw();
	return S_OK;
}

const unsigned int CMaxMaterial::sFrameCount = 16;



STDMETHODIMP CMaxMaterial::NextThumbnailFrame(BSTR* filename)
{
	USES_CONVERSION;
	HRESULT hr = S_FALSE;
	if(mpMtl)
	{
#ifdef _ANIMATE_SAMPLE
		m_currentFrame = (m_currentFrame +1 )%sFrameCount;
		//new method
		mpMtl->meditRotate.Identity();
		float ang[3];
		//TO DO (JH 8/22/02) correction specific to Sphere sample type
		Matrix3 tm(TRUE);
		tm.RotateX(DegToRad(15.0f));
		tm.RotateY(m_currentFrame * (TWOPI/(float)sFrameCount));
		tm.RotateX(DegToRad(-15.0f));
		MatrixToEuler(tm, ang, EULERTYPE_XYZ);
		
//		ang[0] = DegToRad(15.0f); 
//		ang[1] = m_currentFrame * (TWOPI/(float)sFrameCount);
//		ang[2] = 0.0f;
		EulerToQuat(ang , mpMtl->meditRotate, EULERTYPE_XYZ);

		//old method
		//AngAxis aa_increment(0.0f, 1.0f, 0.0f, TWOPI/(float)sFrameCount); 
		//mpMtl->meditRotate += aa_increment;
#endif
		TSTR basePath;
		basePath.Resize(MAX_PATH);
	#ifdef RENDER_VER
        GetSpecDir(APP_CATALOGS_DIR, _T("Catalogs\\vizRWorkspace"), basePath.data());
	#else
        GetSpecDir(APP_CATALOGS_DIR, _T("Catalogs\\Workspace"), basePath.data());
	#endif //RENDER_VER
		mpCollection->Suspend();
		HRESULT hr = mPSCache.updateCache(basePath, mpMtl, mID, m_currentFrame);
		mpCollection->Resume();
		ATLASSERT(SUCCEEDED(hr));
		if(SUCCEEDED(hr))
		{
			bool got = mPSCache.getPath(filename, m_currentFrame);

			hr = got?S_OK:E_UNEXPECTED;
		}
#ifdef _ANIMATE_SAMPLE
		mpMtl->meditRotate.Identity();
#endif
	}
	return hr;
}

STDMETHODIMP CMaxMaterial::get_DiffuseColor(OLE_COLOR* pVal)
{
	Color c = mpMtl->GetDiffuse();
	*pVal = RGB(c.r,c.g,c.b);

	return S_OK;
}

STDMETHODIMP CMaxMaterial::put_DiffuseColor(OLE_COLOR newVal)
{
	Color c = Color(GetRValue(newVal),GetGValue(newVal),GetBValue(newVal));
	
	mpMtl->SetDiffuse(c, GetCOREInterface()->GetTime());

	//TO DO Review
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	return S_OK;
}

STDMETHODIMP CMaxMaterial::get_DiffuseMapAmount(FLOAT* pVal)
{
	if(mpMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
	{
		StdMat *pStdMtl = (StdMat *) mpMtl; 
		*pVal = pStdMtl->GetTexmapAmt(1, GetCOREInterface()->GetTime());
		return S_OK;
	}
	else
		return S_FALSE;
}

STDMETHODIMP CMaxMaterial::put_DiffuseMapAmount(FLOAT newVal)
{
	if(mpMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
	{
		StdMat *pStdMtl = (StdMat *) mpMtl; 
		pStdMtl->SetTexmapAmt(1, newVal, GetCOREInterface()->GetTime());
		return S_OK;
		//TO DO Review
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
	else
		return S_FALSE;
}

STDMETHODIMP CMaxMaterial::get_Shininess(FLOAT* pVal)
{
	*pVal = mpMtl->GetShininess();

	return S_OK;
}

STDMETHODIMP CMaxMaterial::put_Shininess(FLOAT newVal)
{
	mpMtl->SetShininess(newVal, GetCOREInterface()->GetTime());

	//TO DO Review
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	return S_OK;
}

STDMETHODIMP CMaxMaterial::get_Transparency(FLOAT* pVal)
{
	//note that max thinks in terms of opacity, the additive inverse
	if(mpMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
	{
		StdMat *pStdMtl = (StdMat *) mpMtl; 
		*pVal = 1.0f - pStdMtl->GetOpacity(GetCOREInterface()->GetTime());
		return S_OK;
	}
	else
		return S_FALSE;
}

STDMETHODIMP CMaxMaterial::put_Transparency(FLOAT newVal)
{
	//note that max thinks in terms of opacity, the additive inverse
	if(mpMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
	{
		StdMat *pStdMtl = (StdMat *) mpMtl; 
		pStdMtl->SetOpacity(1.0f - newVal, GetCOREInterface()->GetTime());

		//TO DO Review
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

		return S_OK;
	}
	else
		return S_FALSE;
}

STDMETHODIMP CMaxMaterial::get_IdxRefraction(FLOAT* pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CMaxMaterial::put_IdxRefraction(FLOAT newVal)
{
	// TODO: Add your implementation code here

	//TO DO Review
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	return S_OK;
}

STDMETHODIMP CMaxMaterial::get_Luminance(FLOAT* pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CMaxMaterial::put_Luminance(FLOAT newVal)
{
	// TODO: Add your implementation code here

	//TO DO Review
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	return S_OK;
}

STDMETHODIMP CMaxMaterial::get_TwoSided(VARIANT_BOOL* pVal)
{
	if(mpMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
	{
		StdMat *pStdMtl = (StdMat *) mpMtl; 
		*pVal = pStdMtl->GetTwoSided();
		return S_OK;
	}
	else
		return S_FALSE;
}

STDMETHODIMP CMaxMaterial::put_TwoSided(VARIANT_BOOL newVal)
{
	if(mpMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
	{
		StdMat *pStdMtl = (StdMat *) mpMtl; 
		pStdMtl->SetTwoSided(newVal);

		//TO DO Review
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

		return S_OK;
	}
	else
		return S_FALSE;
}

STDMETHODIMP CMaxMaterial::DoColorPicker(void)
{
	Interface *ip = GetCOREInterface();
	DWORD color;
	BOOL picked = ip->NodeColorPicker(ip->GetMAXHWnd(), color);
	if(picked)
	{
		Color c = Color(GetRValue(color),GetGValue(color),GetBValue(color));
		
		mpMtl->SetDiffuse(c, ip->GetTime());

		return S_OK;
	}
	else
		return S_FALSE;
}

class MtlInSceneEnum: public DependentEnumProc {
	public:
		INode *depNode;
		short count;
		Mtl *mpMtl;
		MtlInSceneEnum(Mtl *m) {  mpMtl = m;depNode = NULL; count = 0;}
		// proc should return 1 when it wants enumeration to halt.
		virtual	int proc(ReferenceMaker *rmaker);
	};

int MtlInSceneEnum::proc(ReferenceMaker *rmaker) {
#ifdef TP_SUPPRESS_MULTI_EXPANSION	
	//only  count direct refs from nodes
	if (rmaker == (ReferenceMaker *)mpMtl) 
		return DEP_ENUM_CONTINUE;
	if (rmaker->SuperClassID()!=BASENODE_CLASS_ID) 
		return DEP_ENUM_SKIP;
#else
	if (rmaker->SuperClassID()!=BASENODE_CLASS_ID) 
		return DEP_ENUM_CONTINUE;
#endif
	INode* node = (INode *)rmaker;
//	if(!node->IsRootNode())//not just any node 
//		return 0;
	if(node->TestAFlag(A_IS_DELETED))
		return DEP_ENUM_CONTINUE;
	depNode = node;
	count ++;
	return DEP_ENUM_SKIP;
	}


STDMETHODIMP CMaxMaterial::InScene(VARIANT_BOOL* res)
{
	/*
	*res = VARIANT_FALSE;
	if(mcNodeRefs)
		*res = VARIANT_TRUE;
	*/
	*res = mcNodeRefs;
	return mcNodeRefs?S_OK:S_FALSE;
}


class MtlIsReferencedEnum: public DependentEnumProc {
	public:
		ReferenceTarget *rTarg;
		ReferenceTarget *me;
		bool mbIsSubMtl;
		MtlIsReferencedEnum(ReferenceTarget *t) : mbIsSubMtl(false) {  rTarg = NULL; me = t; }
		// proc should return 1 when it wants enumeration to halt.
		virtual	int proc(ReferenceMaker *rmaker);
		int IgnoreRef(RefMakerHandle rm);

	};

int MtlIsReferencedEnum::proc(ReferenceMaker *rmaker) {
	if(IgnoreRef(rmaker))
		return DEP_ENUM_CONTINUE;
	rTarg = (ReferenceTarget *)rmaker;//cast ok because IgnoreRef excludes pure makers
	return DEP_ENUM_HALT;
	}
static Class_ID meditClassID(MEDIT_CLASS_ID,0);
static Class_ID mtlLibClassID(MTL_LIB_CLASS_ID,0);
static Class_ID mtlBaseLibClassID(MTLBASE_LIB_CLASS_ID,0);
static Class_ID mtlScratchLibClassID(SCRATCH_LIB_CLASS_ID);
#define VZMATERIALTABLE_CLASSID Class_ID(0x5640052e, 0x31d7012e)

int MtlIsReferencedEnum::IgnoreRef(RefMakerHandle rm) {
//11/15/04, I'm inverting the logic of this. We can ignore all references except for those
//from the scratch lib and the filelink material table. I'd even like to get rid of the latter one
//by requiring Filelink to explicit create the material with persistence. TODO
	if(rm->ClassID() == mtlScratchLibClassID)
		return 0;

	if(rm->ClassID() == VZMATERIALTABLE_CLASSID)
		return 0;

	return 1;

/*	
	//here's the code prior to 11/15/04
	if(rm == me)
		return 1;

	if (!rm->IsRefTarget()) //this will exclude the COM wrapper
		return 1;  
	if (rm->SuperClassID()==REF_MAKER_CLASS_ID) 
	{
		if(rm->ClassID() == mtlScratchLibClassID)
			return 0;
		//ignore all of these
		return 1;

		if(rm->ClassID() == meditClassID)
			return 1;
		if(rm->ClassID() == mtlLibClassID)
			return 1;
		if(rm->ClassID() == mtlBaseLibClassID)//TODO check this
			return 1;
	}

	if (rm->SuperClassID()==TREE_VIEW_CLASS_ID)
		return 1;
	if (rm->SuperClassID()==MAKEREF_REST_CLASS_ID)
		return 1;
	if (rm->SuperClassID()==ASSIGNREF_REST_CLASS_ID)
		return 1;
	if (rm->SuperClassID()==MAXSCRIPT_WRAPPER_CLASS_ID)
		return 1;
  	if (rm->SuperClassID()==MATERIAL_CLASS_ID)
	{
		mbIsSubMtl = true;
		return 1;
	}
  	if (rm->SuperClassID()==TEXMAP_CLASS_ID)
		return 1;


	return 0;
*/
}

class MtlIsReferencedOnlyByScratchLibEnum: public DependentEnumProc {
	MtlIsReferencedOnlyByScratchLibEnum();//hide this constructor
	public:
		ReferenceMaker *me;//the material itself
		ReferenceMaker *ignore;//the wrapper on the material
		bool mbResult;//set to false
		MtlIsReferencedOnlyByScratchLibEnum(ReferenceMaker *m, ReferenceMaker *wrapper) : mbResult(false) {  me = m; ignore = wrapper;}
		// proc should return 1 when it wants enumeration to halt.
		virtual	int proc(ReferenceMaker *rmaker){
			if(rmaker->ClassID() == Class_ID(0,0))//restore objects and others?
				return DEP_ENUM_CONTINUE;
			if(rmaker == ignore || rmaker == me)
				return DEP_ENUM_CONTINUE;
			if(	rmaker->ClassID() == mtlScratchLibClassID)
			{
				mbResult = true;//found the expected matlib reference
				return DEP_ENUM_CONTINUE;
			}
			
			//found some other reference 
			mbResult = false;
			return DEP_ENUM_HALT;
		}

	};



STDMETHODIMP CMaxMaterial::IsReferenced(VARIANT_BOOL* res)
{
	/*
	*res = VARIANT_FALSE;
	if(mcMaxRefs)
		*res = VARIANT_TRUE;
	*/
	*res = mcMaxRefs;
	return mcMaxRefs?S_OK:S_FALSE;
}

bool CMaxMaterial::confirmEditing()
{
	MtlBase *pm = GetCOREInterface()->GetMtlSlot(0);
	return(pm == mpMtl)?true:false;
}

void CMaxMaterial::updateRefCounts()
{
	if(mpMtl == NULL)
	{
		mcMaxRefs = mcNodeRefs = 0;
		mbSuspended = true;
	}
	else
	{

	//count all "significant refs"
	MtlIsReferencedEnum e1(mpMtl);
	mpMtl->DoEnumDependents(&e1);
	mcMaxRefs = e1.rTarg?1:0;
	mbSubMtl = e1.mbIsSubMtl;

	//is material in scene, node refs
	MtlInSceneEnum e2(mpMtl);
	mpMtl->DoEnumDependents(&e2);
//	mcNodeRefs = e2.depNode?1:0;
	mcNodeRefs = e2.count;
	}
}

//This is a general device for temporary behaviors
STDMETHODIMP CMaxMaterial::utilExecute(SHORT command, VARIANT arg1)
{
	HRESULT hr;
	// TODO: disable done by FCS
	switch(command)
	{
	case 1:
		{
			GUID id;
			CComBSTR idString;
			if(arg1.vt == VT_BSTR)
				idString = arg1.bstrVal;
			hr = CLSIDFromString(idString, &id);
			if(SUCCEEDED(hr))
				mID = id;
			return hr;
		}
		break;
	case 2:
		{
			mPSCache.Invalidate();
				mpCollection->OnMtlChanged(this);
		}
		break;
	case 3:
		{ //return S_OK if the material is referenced only by the scratchlib
			if(mpMtl)
			{
				MtlIsReferencedOnlyByScratchLibEnum e1(mpMtl, this);
				mpMtl->DoEnumDependents(&e1);
				return e1.mbResult?S_OK:S_FALSE;
			}
		}
		break;	
	}

	return S_OK;
}

STDMETHODIMP CMaxMaterial::QueryXmlSupport( BSTR *  pbstrLog,  VARIANT_BOOL  fullReport)
{
	//validate input
	if(!mpMtl)
		return E_FAIL;

	CComPtr<IVIZPointerClient> pPointerClient;
	HRESULT hr = CMaxMaterialCollection::GetXMLImpExp(&pPointerClient);
	if(!SUCCEEDED(hr))
	{
		return E_FAIL;
	}

	CComQIPtr<IMaterialXmlExport3> pIMtl = pPointerClient;
	ATLASSERT(pIMtl);
	if(!pIMtl)
		return E_FAIL;

/*
	CComPtr<IXMLDOMDocument> doc;
	_bstr_t empty("");
	pXML->get_ownerDocument(&doc);
	hr = pIMtl->InitMtlExport(doc, empty.GetBSTR());
	VARIANT_BOOL f = 0;
	hr = pIMtl->InitThumbnailPrefs(f, empty.GetBSTR(), empty.GetBSTR());

*/
	VARIANT vMtl;
	vMtl.vt = VT_BYREF;
	vMtl.byref = mpMtl;
	hr = pIMtl->QueryXmlSupport(vMtl, pbstrLog, fullReport);

	return hr;
}

STDMETHODIMP CMaxMaterial::SerializeXml( IUnknown *  pUnk)
{
	// Add your function implementation here.
	return E_NOTIMPL;
}

STDMETHODIMP CMaxMaterial::LoadXml( IUnknown *  pUnk)
{
	// Add your function implementation here.
	return E_NOTIMPL;
}

class MtlSelectEnum: public DependentEnumProc {
	public:
		INodeTab depNodes;
		BitArray multiBits;
		Mtl* mpMtl;
		MtlSelectEnum(Mtl* pMtl) {mpMtl = pMtl;}
		// proc should return 1 when it wants enumeration to halt.
		virtual	int proc(ReferenceMaker *rmaker);
	};

int MtlSelectEnum::proc(ReferenceMaker *rmaker) {
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID) 
	{
		INode* node = (INode *)rmaker;
		depNodes.Append(1, &node);
		
		Mtl *m = node->GetMtl();
		BOOL bMulti = m != mpMtl;
		int n = multiBits.GetSize();
		multiBits.SetSize(n+1, 1);
		multiBits.Set(n, bMulti);
		//Don't continue up through the node
		return DEP_ENUM_SKIP;
	}
	return DEP_ENUM_CONTINUE;
	}

typedef std::set<MtlID> FaceIDS;
//recursive function to get the ids for a specific material
//from within another material
//multi arg should be multi
//mtl arg can be anything
static void GetIds(Mtl *multi, Mtl *mtl, FaceIDS& ids)
{
	ATLASSERT(multi);
	if(multi == NULL)
		return;
	ATLASSERT(multi->IsMultiMtl());
	for(int j=0; j<multi->NumSubMtls(); j++)
	{
		Mtl* m = multi->GetSubMtl(j);
		if(m==NULL)
			continue;
		if(mtl == m)
		{
			ids.insert(j);
		}
		else if(m->IsMultiMtl())//recurse
			GetIds(m, mtl, ids);
	}
}

static void selFacesWithMaterial( INode* node, Object *obj, Mtl* mtl, bool bAdd, bool bSub)
{
	FaceIDS ids;
	Mtl *pMtl = node->GetMtl();
	if (pMtl != mtl && pMtl != NULL && pMtl->IsMultiMtl()) {

		GetIds(pMtl, mtl, ids);
		ATLASSERT(ids.size());
		if(ids.size() == 0)
			return;

		TimeValue t = GetCOREInterface()->GetTime();
/*
		ObjectState os = node->EvalWorldState(t);
		if (os.obj != NULL && os.obj->CanConvertToType(triObjectClassID)) {
			TriObject* tri = static_cast<TriObject*>(
				os.obj->ConvertToType(t, triObjectClassID));
				*/
		if (obj != NULL && obj->CanConvertToType(triObjectClassID)) {
			TriObject* tri = static_cast<TriObject*>(
				obj->ConvertToType(t, triObjectClassID));
			if (tri != NULL) {
				BitArray& sel = tri->mesh.FaceSel();
				if(!(bAdd || bSub))
					sel.ClearAll();//start with empty 
				int i, count =  tri->mesh.getNumFaces();
				for (i = 0; i < count; ++i) {
					int id = tri->mesh.faces[i].getMatID();
					if(ids.find(id)!= ids.end())
						sel.Set(i, bSub?0:1);
				}
//				if (tri != os.obj)
				if (tri != obj)
					tri->DeleteThis();
			}
		}
	}
}

static BOOL IsInPipeline(INode* node, BaseObject* pObj, ModContext*& pModContext)
{
	pModContext = NULL;
	Object* obj = node->GetObjectRef();

	if(obj == pObj)
		return TRUE;

	while (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
	    IDerivedObject* dobj = (IDerivedObject*)obj;
	    int m;
	    int numMods = dobj->NumModifiers();

	    for (m=0; m<numMods; m++) {
	        BaseObject* mod = dobj->GetModifier(m);
	        if (mod == pObj) {
				pModContext = dobj->GetModContext(m);
	            return TRUE;
			}
		}
		obj = dobj->GetObjRef();
	}
	// At this point there is no ModContext and we just have to
	// compare the base object.
	return (obj == pObj);
}

STDMETHODIMP CMaxMaterial::selectNodes(void)
{
	//is material in scene, node refs
	if(mpMtl)
	{
		Interface *ip = GetCOREInterface();
		ip->ClearNodeSelection();
		MtlSelectEnum e(mpMtl);
		mpMtl->DoEnumDependents(&e);
		ip->SelectNodeTab(e.depNodes, 1);

		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CMaxMaterial::clone(IMaxMaterial ** ppNew)
{
	if(mpMtl)
	{
		ATLASSERT(mpCollection);
		//Create the wrapper
		CComObject<CMaxMaterial> *pWrapperObject;
		HRESULT hr = CComObject<CMaxMaterial>::CreateInstance(&pWrapperObject);
		assert(SUCCEEDED(hr));
		if(!SUCCEEDED(hr))
			return hr;

		//Initalize the new wrapper
		pWrapperObject->setCollection(mpCollection);
		
		Mtl* pMtl = (Mtl*) mpMtl->Clone();
		if(!pMtl)
			return E_FAIL;
		pWrapperObject->setMtl(pMtl);

		hr = pWrapperObject->QueryInterface(IID_IMaxMaterial, (void **)ppNew);

		return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP CMaxMaterial::CanAssignSelected(VARIANT_BOOL* vResult)
{
	Interface* ip = GetCOREInterface();

	*vResult = VARIANT_FALSE;
	if(ip->GetSelNodeCount())
		*vResult = VARIANT_TRUE;
	return S_OK;
}


#define _FACE_SELS
STDMETHODIMP CMaxMaterial::selectFace(USHORT idxSelection)
{
#ifdef _FACE_SELS
	//is material in scene, node refs
	bool bAdd = GetAsyncKeyState(VK_CONTROL)?true:false;
	bool bSub = GetAsyncKeyState(VK_MENU)?true:false;
	if(mpMtl)
	{
		Interface *ip = GetCOREInterface();
		MtlSelectEnum e(mpMtl);
		mpMtl->DoEnumDependents(&e);
		if(e.depNodes.Count())
		{
			int numMultiUsers = e.multiBits.NumberSet();
			if( numMultiUsers == 0)
			{
				if(!(bAdd || bSub))
					ip->ClearNodeSelection();

				ip->SelectNodeTab(e.depNodes, (bSub)?false:true, 1);
			}
			else
			{

				Tab<INodeTab*> nodeSets;//each InodeTab holds a set of nodes with common objef
				//compute all common histories and put a dialog to choose 
				//which common history to use
				for (int i = 0;i<e.multiBits.GetSize(); i++)
				{
					if(e.multiBits[i])
					{
						INodeTab *newSet = new INodeTab;
						nodeSets.Append(1, &newSet);

						INode *pNode1 = e.depNodes[i];
						newSet->Append(1, &pNode1);
						e.multiBits.Clear(i);
						BaseObject *pObj = pNode1->GetObjectRef();
						FaceIDS ids1;
						GetIds(pNode1->GetMtl(), mpMtl, ids1);
						ATLASSERT(ids1.size());

						for(int j=0; j<e.multiBits.GetSize(); j++)
						{
							if(e.multiBits[j])
							{
								INode* pNode2 = e.depNodes[j];
								assert(pNode2);
								FaceIDS ids2;
								GetIds(pNode2->GetMtl(), mpMtl, ids2);

								ModContext* pModContext = NULL;
								if (IsInPipeline(pNode2, pObj, pModContext) && ids1==ids2) 
								{
									newSet->Append(1, &pNode2);
									e.multiBits.Clear(j);
								}
							}
						}
					}
				}

				int whichSet;
				//temporary
				whichSet = idxSelection;
				//whichSet = 0;

				ip->ClearNodeSelection(FALSE);
				ip->SelectNodeTab(*nodeSets[whichSet%nodeSets.Count()], 1);
				BaseObject *pObj = ip->GetCurEditObject();
//				INode *hintNode = nodeSets[whichSet%nodeSets.Count()]->operator[](0);
//				BaseObject *pObj = hintNode->GetObjectRef();
				if(pObj)
				{
					BaseObject *obj = pObj;
					ISubMtlAPI* api;
					Interface7 *pInt = GetCOREInterface7();

					while(obj && NULL == (api = (ISubMtlAPI*)obj->GetInterface(I_SUBMTLAPI)))
					{
						/*
						obj = nodeSets[whichSet%nodeSets.Count()]->operator[](0)->GetObjectRef();
						while(obj && obj->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
							IDerivedObject* dobj = (IDerivedObject*)obj;
							int m;
							int numMods = dobj->NumModifiers();

							for (m=0; m<numMods; m++) {
								BaseObject* mod = dobj->GetModifier(m);
								if (api = (ISubMtlAPI*)mod->GetInterface(I_SUBMTLAPI)) 
									break;
							}
							obj = dobj->GetObjRef();
						}
						*/
						BaseObject *lastobj = obj;

						/*
						//walk to top of history
						while(1)
						{
						pInt->ChangeHistory(1);//down
						obj = pInt->GetCurEditObject();
						if(obj == lastobj)
							break;
						}
						*/

						pInt->ChangeHistory(0);//down
						obj = pInt->GetCurEditObject();
						if(obj == lastobj)
							break;
					}

					if(api)
					{
						if(obj != pObj )
						{
							//pInt->SelectedHistoryChanged();
							//pInt->SetCurEditObject(obj);
						}
						ip->SetSubObjectLevel(3);
						
/*
						int num = ip->GetSelNodeCount();
						for ( i = 0; i < num ; i++) {
							INode* pNode = ip->GetSelNode(i);
							assert(pNode);
							//is the downcast justified by the presence of api?
							selFacesWithMaterial( pNode, (Object*)obj, mpMtl, bAdd, bSub);
						}
						*/
						
						INode *pNode = nodeSets[whichSet%nodeSets.Count()]->operator[](0);
						ObjectState os = pNode->EvalWorldState(ip->GetTime());
						selFacesWithMaterial( pNode, os.obj, mpMtl, bAdd, bSub);
					}
				}
				
				//clean up locally allocated INodeTabs
				for(int i=0; i<nodeSets.Count();i++)
					if(nodeSets[i])
						delete nodeSets[i];

//				ip->ViewportZoomExtents(FALSE);
				ip->RedrawViews(ip->GetTime());

			}
			}
			}
#endif
	return S_OK;
}
#define MAXNAME 128
STDMETHODIMP CMaxMaterial::getFaceSelectionNames(VARIANT* pvFaceSelNames)
{
#ifdef _FACE_SELS
	//is material in scene, node refs
	bool bAdd = GetAsyncKeyState(VK_CONTROL)?true:false;
	bool bSub = GetAsyncKeyState(VK_MENU)?true:false;
	if(mpMtl)
	{
		Interface *ip = GetCOREInterface();
		MtlSelectEnum e(mpMtl);
		mpMtl->DoEnumDependents(&e);
		if(e.depNodes.Count())
		{
			int numMultiUsers = e.multiBits.NumberSet();
			if( numMultiUsers != 0)
			{

				Tab<INodeTab*> nodeSets;//each InodeTab holds a set of nodes with common objef
				//compute all common histories and put a dialog to choose 
				//which common history to use
				for (int i = 0;i<e.multiBits.GetSize(); i++)
				{
					if(e.multiBits[i])
					{
						INodeTab *newSet = new INodeTab;
						nodeSets.Append(1, &newSet);

						INode *pNode1 = e.depNodes[i];
						newSet->Append(1, &pNode1);
						e.multiBits.Clear(i);
						BaseObject *pObj = pNode1->GetObjectRef();
						FaceIDS ids1;
						GetIds(pNode1->GetMtl(), mpMtl, ids1);
						ATLASSERT(ids1.size());

						for(int j=0; j<e.multiBits.GetSize(); j++)
						{
							if(e.multiBits[j])
							{
								INode* pNode2 = e.depNodes[j];
								assert(pNode2);
								FaceIDS ids2;
								GetIds(pNode2->GetMtl(), mpMtl, ids2);

								ModContext* pModContext = NULL;
								if (IsInPipeline(pNode2, pObj, pModContext) && ids1==ids2) 
								{
									newSet->Append(1, &pNode2);
									e.multiBits.Clear(j);
								}
							}
						}
					}
				}

				ATLASSERT(pvFaceSelNames != NULL);


				//Create an safe array to hold the descriptions
				SAFEARRAYBOUND saBound;
				saBound.lLbound = 0;
				saBound.cElements = nodeSets.Count();


				SAFEARRAY *psaPMArray = SafeArrayCreate(VT_VARIANT, 1, &saBound);
				if(!psaPMArray)
					return E_FAIL;
				long index[1];
				CComVariant var;
				for (int i=0; i<saBound.cElements; i++)
				{
					index[0] = i;
					//Build up string
					CComBSTR desc, temp;
					INodeTab& nodes = *nodeSets[i];
					for(int j=0; j<nodes.Count(); j++)
					{
						int roomleft = MAXNAME - desc.Length()-3;
						if(j!=0)
						{

							desc.Append(", ");
							roomleft -= 2;
						}
						temp = nodes[j]->GetName();
						int len = min(roomleft, temp.Length());
						desc.Append(temp, len);
						roomleft-=len;
						if(roomleft == 0)
						{
							desc.Append("...");
							break;
						}
					}
					var = desc;
					HRESULT hr = SafeArrayPutElement(psaPMArray, index, &var);
					if(!SUCCEEDED(hr))
						return hr;
					
					//clean up locally allocated INodeTabs
					if(nodeSets[i])
						delete nodeSets[i];

				}

				pvFaceSelNames->vt = VT_ARRAY | VT_VARIANT;
				pvFaceSelNames->parray = psaPMArray;
				return S_OK;

			}
		}
	}
#endif
	return S_FALSE;
}

int CMaxMaterial::WhichSlot(MtlBase *pMtl, bool * needPut)
{
		/*
			find the right slot
			1) search all slots for the material itself, if found select the slot
			2) search all slots and use the first containing a default material which isn't used in the scene
			3) as last result use current slot
		*/

		Interface *ip = GetCOREInterface();
		for(int i=0; i< 24; i++)
		{
			MtlBase *pSlotMtl = ip->GetMtlSlot(i);
			if(pSlotMtl == pMtl)
			{ 
				*needPut = false;
				return i;
			}
		}

		*needPut = true;
		for(int i=0; i< 24; i++)
		{
			MtlBase *pSlotMtl = ip->GetMtlSlot(i);

			if(mpCollection->findItemKey((DWORD_PTR)pSlotMtl))
				continue; //don't use this slot

			//check if the name contains Default starting at position 6, if so, we use this
			TSTR name = pSlotMtl->GetName();
			TCHAR *pdest;
			int  result;
			pdest = _tcsstr( name.data(), _T("Default") );
			result = (int)(pdest - name.data() + 1);
			if ( pdest != NULL && ((result == 6) || (result == 5)))// "17 -Default" has one less leading char
			{ 
				return i;
			}
		}

		//no good candidate, return the active slot
#ifdef RENDER_VER // viz render does not have the medit dialog TODO should this override all this function?
      return 0;
#else
		return GetMtlEditInterface()->GetActiveMtlSlot();
#endif
}

#endif	// EXTENDED_OBJECT_MODEL
