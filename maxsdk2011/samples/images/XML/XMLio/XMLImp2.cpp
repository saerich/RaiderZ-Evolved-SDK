#include "stdafx.h"
#include "xmlimp2.h"

#include "XmlMtl.h"
#include "XmlMapMods.h"

#include "comstyle.h"

class XMLImp2ClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new XMLImp2();}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME_GRANITE);}
	SClass_ID		SuperClassID() {return SCENE_IMPORT_CLASS_ID;}
	Class_ID		ClassID() {return XMLIMP2_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	const TCHAR*	InternalName() { return _T("XMLImp2"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

//for Granite we hardcode to the modules that we know we need
//this reduces a registration dependency
static const TSTR sXmlMaterialModuleName("stdplugs\\XmlMtl.dll");
static const TSTR sMappingModsModuleName("stdplugs\\XmlMapMods.dll");

static XMLImp2ClassDesc XMLImp2Desc;
ClassDesc2* GetNewXMLImpDesc() {return &XMLImp2Desc;}

const TCHAR *	XMLImp2::Ext(int n)
{
	// Extension #n (i.e. "3DS")
	switch(n)
	{
	case 0:
		    return _T("xml");
		break;
	case 1:
		    return _T("XML");
		break;
	}					
    return _T("");
}

int XMLImp2::DoImport(const TCHAR *name,ImpInterface *ii,Interface *i, BOOL suppressPrompts)
{
	HRESULT hr;
	int numElementsProcessed = 0;

	//Initialize COM
	hr = CoInitialize(NULL); // check the return value, hr...
	assert(hr != RPC_E_CHANGED_MODE);
	if(hr == RPC_E_CHANGED_MODE )
	{
//		DbgMsgBox(GetString(IDS_COM_ERROR));
		return 0;
	}

	CComPtr<IXMLDOMDocument> pDoc;
	try
	{
		hr = pDoc.CoCreateInstance(CLSID_DOMDocument);
		CHECKHR(hr);
		VARIANT_BOOL bLoaded;
		hr = pDoc->load(CComVariant(name), &bLoaded);
		CHECKHR(hr);
		if(!bLoaded)//load failed
			return 0;

		CComPtr<IXMLDOMNode> pNode;

		hr = pDoc->selectSingleNode(CComBSTR("Scene/ObjectModifiers"), &pNode);
		//TODO other file validation
		CHECKHR(hr);
		if(pNode)
		{
			//we'll try to process a know selection of child nodes
			//if a known type is found we load the appropriate pluglet and call it
			//these plugets get passed teh max interfaces through a com call
			//set up the variants for that
			CComVariant v_ifx;
			v_ifx.vt = VT_BYREF;
			v_ifx.byref = i;
			CComVariant v_impifx;
			v_impifx.vt = VT_BYREF;
			v_impifx.byref = ii;

			CComPtr<IXMLDOMNode> pChild;

			//process the Materials element, there should only be one
			hr = pNode->selectSingleNode(CComBSTR("Materials"), &pChild.p);
			CHECKHR(hr);
			if(pChild)
			{
				CComPtr<IDOMElementLoader> pLoader;
				TSTR plugletModuleName(i->GetDir(APP_MAX_SYS_ROOT_DIR));
				plugletModuleName.Append(sXmlMaterialModuleName);
				hr = CreatePluglet(CLSID_XmlMaterial, IID_IDOMElementLoader, (void**)&pLoader.p, plugletModuleName.data());
				CHECKHR(hr);
				if (pLoader)
				{
					CComQIPtr<IVIZPointerClient> pInterfaceClient = pLoader;
					assert(pInterfaceClient);
					if(pInterfaceClient)
					{
						CHECKHR(pInterfaceClient->InitPointers(v_ifx, v_impifx));
						CHECKHR(pLoader->LoadElement(pChild, NULL /*LoadCtx not needed*/, suppressPrompts));
					}
					numElementsProcessed++;
					pChild = NULL;
				}
			}//Materials Element

			//process the Modifiers element, there should only be one
			hr = pNode->selectSingleNode(CComBSTR("Modifiers"), &pChild.p);
			CHECKHR(hr);
			if(pChild)
			{
				CComPtr<IDOMElementLoader> pLoader;
				TSTR plugletModuleName(i->GetDir(APP_MAX_SYS_ROOT_DIR));
				plugletModuleName.Append(sMappingModsModuleName);
				hr = CreatePluglet(CLSID_XmlMappingMods, IID_IDOMElementLoader, (void**)&pLoader.p, plugletModuleName.data());
				CHECKHR(hr);
				if (pLoader)
				{
					CComQIPtr<IVIZPointerClient> pInterfaceClient = pLoader;
					assert(pInterfaceClient);
					if(pInterfaceClient)
					{
						CHECKHR(pInterfaceClient->InitPointers(v_ifx, v_impifx));
						CHECKHR(pLoader->LoadElement(pChild, NULL /*LoadCtx not needed*/, suppressPrompts));
					}
					numElementsProcessed++;
					pLoader = NULL;
				}
			}//Modifiers Element

		}//pNode
	}
	catch(HRESULT)
	{
		return 0;
	}
    CoUninitialize();
	if(numElementsProcessed)
		i->RedrawViews(0);
	return numElementsProcessed?IMPEXP_SUCCESS:0;
}

// DllGetClassObject function pointer
typedef HRESULT (STDAPICALLTYPE *DLLGETCLASSOBJECT_PROC)(REFCLSID, REFIID, LPVOID*);

HRESULT XMLImp2::CreatePluglet(REFCLSID rclsid, REFIID riid, void **ppPluglet, LPCTSTR sModule)
{
	HRESULT hr;

    if (sModule && sModule[0] != 0) {
		//TODO should we be managing the module handles somehow?
		HMODULE hModule = LoadLibrary(sModule);
		if (hModule == NULL) {
			// Failed to load DLL, the last error will be set by the system
			//
			return E_FAIL;
		}

		// Get the proc addres for function DllGetClassObject
		DLLGETCLASSOBJECT_PROC pFunc = (DLLGETCLASSOBJECT_PROC) 
			GetProcAddress(hModule, /*MSG0*/"DllGetClassObject");
		assert(pFunc != NULL);
		if (pFunc == NULL) {
			// DllGetClassObject function is not exported
			// The specified procedure could not be found.
			FreeLibrary(hModule);
			SetLastError(ERROR_PROC_NOT_FOUND);
			return E_FAIL;
		}

		// Get the class factory interface using DllGetClassObject
		CComPtr<IClassFactory> pCF;
		hr = pFunc(rclsid, IID_IClassFactory, (void**) &pCF);
		if (FAILED(hr) || pCF == NULL) {
			FreeLibrary(hModule);
			SetLastError(hr); // Failed to get interface
			return E_FAIL;
		}

		// Create the COM object using the class factory
		hr = pCF->CreateInstance(NULL, riid, (void**) ppPluglet);
		assert(SUCCEEDED(hr) && ppPluglet != NULL);
		if (FAILED(hr) || ppPluglet == NULL) {
			SetLastError(hr); // Failed to create COM object
			return E_FAIL;
		}
	}
	else
	{
		// Co-create using IXmlMaterial's CLSID, but ask for the IMaterialXmlExport
		// interface.
#ifndef NO_COCREATE
		hr = CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER,
									riid, (void**)ppPluglet);
#else
		hr = E_FAIL;
#endif
	}

	return hr;
} 
