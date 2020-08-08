/**********************************************************************
 *<
	FILE: XMLImp.cpp

	DESCRIPTION: Implementation of the XMLImp class. 
	The main class for importing XML data.
	written to be independent of the parser choice

	CREATED BY: John Hutchinson, Discreet Development

	HISTORY: 1/09/00
			6/09/03

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "stdafx.h"
#include <atlbase.h>


#ifdef KAHN_OR_EARLIER // 06/10/03 this class is only relevant to VIZ now
#include "xmlimp.h"
#include "logclient.h"
#include <exdisp.h>
#include <shellapi.h>

#include "Comstyle.h"
//put my preferred utilities into scope
using namespace jh;

#define XMLIO_INI_FILENAME    _T("XMLIO.ini")
#define XMLIO_LOG_FILENAME    _T("XMLIO.xml")
#define XMLIO_REGISTRY_FILENAME _T("SchemaRegistry.xml")
#define XMLIO_SECTION         _T("Default Settings")
#define XMLIO_VERSION         _T("Current Version")
#define XMLIO_REGISTRY_URL     _T("XML Transform Registry location")
#define XMLIO_PROGID_REGISTRY     _T("XML Transform Registry component")
#define XMLIO_PROGID_COERCER     _T("XSL Coercion component")
#define XMLIO_VERBOSITY			_T("Verbose")
#define XMLIO_DEFAULTSTRING			_T("Default")


static const GUID nullGUID = 
{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

class XMLImpClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new XMLImp();}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return SCENE_IMPORT_CLASS_ID;}
	Class_ID		ClassID() {return XMLIMP_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	const TCHAR*	InternalName() { return _T("XMLImp"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

static XMLImpClassDesc XMLImpDesc;
ClassDesc2* GetXMLImpDesc() {return &XMLImpDesc;}



//Implementation of XMLImp class

XMLImp::XMLImp():m_ClassActivator(0)
{
	VariantInit(&m_ifx);
	m_ifx.vt = VT_BYREF;

	VariantInit(&m_impifx);
	m_impifx.vt = VT_BYREF;

	mpLog = NULL;
}

XMLImp::~XMLImp()
{
	RELEASE(m_ClassActivator);
}

const TCHAR *	XMLImp::Ext(int n)
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


INT_PTR CALLBACK XMLImpOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static XMLImp *imp = NULL;

	switch(message) {
		case WM_INITDIALOG:
			imp = (XMLImp *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}






//   Function name	: XMLImp::DoImport
//   Description	    : do the import
//   Return type		: int						1 success, 0 failure
//   Argument         : const TCHAR *name			The name of the file to import
//   Argument         : ImpInterface *ii			The import interface
//   Argument         : Interface *i				The main MAX interface
//   Argument         : BOOL suppressPrompts		
int	XMLImp::DoImport(const TCHAR *name,ImpInterface *ii,Interface *i, BOOL suppressPrompts)
{
//	
//	strategy:
//	Uses the concept of coercion and the ICoercion Interface 
//	
//	traverse the DOM and process the known nodes
//
	//do some COM initialization
	HRESULT hr = S_OK;
	hr = CoInitialize(NULL); // check the return value, hr...
	if(hr == RPC_E_CHANGED_MODE )
	{
		DbgMsgBox(GetString(IDS_COM_ERROR));
		return 0;
	}
	assert(hr != RPC_E_CHANGED_MODE);

	//SS 10/3/2001: No cast needed in R4 SDK.(copied from ..\pluglets\vizrootloader.cpp(62))
	//salt away the VIZ interface pointers in variants
	//we use variants cause we'll be passing these to in-proc COM objects
	//VizInterface* vip = reinterpret_cast<VizInterface*>(i);
	assert(i);
	INode *test = i->GetImportCtxNode();

	m_ifx.byref = i;
	m_impifx.byref = ii;

	//load configuration data from private ini file
	ReadINI();

	//The following pointers need to get freed prior to returning
	BSTR URL = NULL;
	IXMLDOMDocument *pXMLDoc = NULL;

	//Convert the filename into BSTR
	URL = AsciiToBSTR(name);

	try
	{
		start_log();

		//Transforn the document until it's fully consumed
		CHECKHR(CoerceDocument(/*[in]*/URL, /*[out]*/&pXMLDoc, NULL , /*[in]*/suppressPrompts));

	}

	catch(...)
	{
	}
	
	if(hr != S_OK )
	{
		LogEntry(mpLog, hr, L"XML Import had errors:%d", hr);
		stop_log();
		TCHAR caption[256];
		_tcscpy( caption, GetString(IDS_SHORT_DESC));
		int response = MessageBox(NULL, GetString(IDS_HAD_ERRORS), caption, MB_OK | MB_TOPMOST);
		ShowLog();
	}
	else
	{
		LogEntry(mpLog, LOG_INFO, L"Import succeeded:%d", hr);
		stop_log();
	}


	SysFreeString(URL);
	RELEASE(pXMLDoc);
    CoUninitialize();
	i->RedrawViews(0);

	return 1;

}






// Function name	: XMLImp::NonDelegatingCoerceDocument
// Description	    : Once a document is determined to be conformant we load it into VIZ
// Return type		: HRESULT 
// Argument         : ImpInterface *ii
// Argument         : IXMLDOMDocument * pXMLDoc
// Argument         : BSTR RootHandlerDesc
// Argument         : BOOL suppressPrompts
// 3/27/00 My thinking now is that this is just a continuation of the coercion process
HRESULT XMLImp::NonDelegatingCoerceDocument(IXMLDOMDocument * pXMLDoc, IMoniker *pXfrmMk, BOOL suppressPrompts)
{
	USES_LOG_SECTION(L"Loading XML", LOG_INFO)

/*
Old Strategy:
1. Get the root Node
2. create the indicated bootstrap pluglet, 
4. Call IXMLElementLoader::LoadElement( CurrentNode)
5. Release pXMLElementLoader
7. Release the root node;

New Strategy:
1. We may be passed a transfrom moniker, if not we look one up
2. Bind the moniker
3. Call transform
*/	
	//Be sure to release the following stuff
	IXMLDOMElement *pElem = NULL;
	IDOMElementLoader *pLoader = NULL;
	HRESULT hr = S_OK;
	IMoniker *pmk = NULL; //I own this one
	IBindCtx *pbc = NULL;
	ICoercionTransform *pXfrm = NULL;
	IVIZPointerClient *pVpc = NULL;
	ISupportXMLErrorLog *pXMLLog = NULL;
	IMoniker *pPending = NULL; //not used 

	VARIANT vDocInfo;
	VariantInit(&vDocInfo);
	vDocInfo.vt = VT_UNKNOWN;
	pXMLDoc->AddRef();
	vDocInfo.punkVal = (IUnknown *)pXMLDoc;

	try
	{

		CreateBindCtx(0, &pbc);
		CHECKHR(StashClassActivator(pbc));

		if(pXfrmMk)
			( pmk = pXfrmMk )->AddRef();
		else
			CHECKHR(LookupTransform(pbc, vDocInfo, &pmk));
		
		if(pmk)
		{
			CHECKHR(BindTransformMoniker(pbc, pmk, &pXfrm, &pPending));
			assert(pPending == NULL);
			assert(pXfrm);
			
			CHECKHR(pXfrm->QueryInterface(IID_PPV(IVIZPointerClient, &pVpc)));
			assert(pVpc);
			CHECKHR(pVpc->InitPointers(m_ifx, m_impifx));

			CHECKHR(pXfrm->QueryInterface(IID_PPV(ISupportXMLErrorLog, &pXMLLog)));
			assert(pXMLLog);
			int detail;
			CHECKHR(mpLog->get_detail(&detail));
			CHECKHR(pXMLLog->set_logDetailLimit(detail));

			CHECKHR(pXfrm->put_input(vDocInfo));
			VARIANT_BOOL done = VARIANT_FALSE;
			try
			{
			while(done != VARIANT_TRUE)
				CHECKHR(pXfrm->transform(&done));
			assert(done == VARIANT_TRUE);
			}
			catch(...)
			{
//				LOG_ERROR(L"Load Failed");
			}

			try
			{
				MergeLog(mpLog);
			}
			catch(...)
			{
				LOG_ERROR(L"Exception while merging transform log");
			}

			if(hr == S_OK)
				LOG_ENTRY(L"Load was successful")
			else
				LogEntry(mpLog, hr, L"Load had errors:%d", hr);
		}
	}

	catch(...)
	{
		LogEntry(mpLog, hr, L"Load had errors:%d", hr);
	}

	VariantClear(&vDocInfo);
	RELEASE(pbc);
	RELEASE(pmk);
	RELEASE(pVpc);
	RELEASE(pXMLLog);
	RELEASE(pXfrm);
	DONE_LOG_SECTION
	return hr;
}



// Function name	: XMLImp::CoerceDocument
// Description	    : The guts of the importer. Strategy, give any external coercer a chance, then
//						do our own coercion using pluglets which load elements into VIZ.
// Return type		: HRESULT 
// Argument         : /*[in]*/BSTR BURL
// Argument         : /*[out]*/IXMLDOMDocument **pRoot
// Argument         : /*[out]*/IMoniker **pPendingMk
// Argument         : /*[in]*/BOOL suppressPrompts
HRESULT XMLImp::CoerceDocument(/*[in]*/BSTR BURL, /*[out]*/IXMLDOMDocument **pRoot,  /*[out]*/IMoniker **pPendingMk, /*[in]*/BOOL suppressPrompts)
{
	USES_LOG(LOG_INFO);
	HRESULT hr = S_OK;
	IMoniker *pXfrmMk = NULL;
	IXMLUtil2 *util = NULL;
	VARIANT vState;
	vState.vt = VT_BSTR;

	try
	{
		//Possibly perform some coercion in 
		CHECKHR(DelegatingCoerceDocument(BURL, pRoot, pXfrmMk, suppressPrompts));

		if(SUCCEEDED(hr))
		{
			if(!*pRoot)
			{
				//Load the document
				CHECKHR(CoCreateInstance(CLSID_XMLUtil, NULL, CLSCTX_INPROC_SERVER, IID_IXMLUtil2, (void**) &util));
				assert(util);
				LogEntry(mpLog, LOG_INFO,  L"Loading XML document");
				vState.bstrVal = BURL;
				LOG_STATE(L"URL", vState);

				CHECKHR(util->LoadDocument(pRoot, BURL, NULL, 1));
				if(hr != S_OK)
				{
					hr = E_FAIL;
					IErrorInfo *pError = NULL;
					LPOLESTR pszDesc = NULL;
					GetErrorInfo(0, &pError);
					if(pError)
					{
						pError->GetDescription(&pszDesc);
					}
					LogEntry(mpLog, hr,  L"Failed to load XML document");
					vState.bstrVal = pszDesc;
					LOG_STATE(L"Error", vState);
					SysFreeString(pszDesc);
					throw hr;
				}
			}
			CHECKHR(NonDelegatingCoerceDocument(*pRoot, pXfrmMk, suppressPrompts));
		}
	}
	catch(...)
	{
		hr = E_FAIL;
	}
	return hr;
}



// Function name	: XMLImp::BuildCompositeMoniker
// Description	    : From an array of XML fragments representing individual transfromers
//						build a composite moniker
// Return type		: HRESULT 
// Argument         : IBindCtx *pbc
// Argument         : short nummk
// Argument         : BSTR *pBNames
// Argument         : IMoniker **ppCompositeMk
HRESULT XMLImp::BuildCompositeMoniker(IBindCtx *pbc, short nummk, BSTR *pBNames, IMoniker **ppCompositeMk)
{
	USES_LOG_SECTION(L"Building tranformation object", LOG_PROC);
	HRESULT hr = E_FAIL;
	//Now we should build up a compound moniker from the individual monikers
	LPMONIKER pmk = NULL;
	LPMONIKER mktemp = NULL;
	LPMONIKER mkcomp = NULL;//FIXME JH can this be optimized out by using pmk in the call to compose?

	try
	{
		for (int iter = 0; iter<nummk;iter++)
		{

			CHECKHR(BuildAtomicXfrmMoniker(pbc, pBNames[iter], &mktemp));
			assert(mktemp);


		#ifdef _DEBUG					
			DWORD dwMksys;
			CHECKHR(mktemp->IsSystemMoniker(&dwMksys));
		#endif
			//TODO
			//actually check that the monker we got back is one which we expect
			//and handle cases where it isn't, like when it comes back as an
			//urlmon


			if(pmk)
			{
				CHECKHR(pmk->ComposeWith(mktemp,0/*generic OK*/,&mkcomp));
				RELEASE(pmk);
				RELEASE(mktemp);
				mktemp = NULL;
				(pmk = mkcomp)->AddRef();
				RELEASE(mkcomp);
			}
			else //first time through the loop
			{
				(pmk = mktemp)->AddRef();
				RELEASE(mktemp);
				mktemp = NULL;
			}


		}

		((*ppCompositeMk)=pmk)->AddRef();
	}
	catch(...)
	{
		RELEASE(mktemp);
		RELEASE(mkcomp);
	}
	RELEASE(pmk);
	DONE_LOG_SECTION
	return hr;	
}

HRESULT XMLImp::LookupTransform(IBindCtx *pbc, VARIANT vDoc, IMoniker **ppCompositeMk)
{
	USES_LOG_SECTION(L"Locate XML Loader", LOG_PROC)
	HRESULT hr = S_OK;
	IXMLTransformRegistry *pXreg = NULL;
	IXMLUtil2 *pUtil = NULL;

	VARIANT vDocInfo;
	vDocInfo.vt = VT_UNKNOWN;
	(vDocInfo.punkVal = vDoc.punkVal)->AddRef();

	try
	{
		hr = OpenRegistry(&pXreg);
		CHECKHR(hr);

		CHECKHR(CoCreateInstance(CLSID_XMLUtil, NULL, CLSCTX_INPROC_SERVER,	IID_PPV(IXMLUtil2, &pUtil )));
		assert(pUtil);
		//IDOMDocument-containing Variant into a IXMLDOMSchemaCollection-containing variant 
		hr = pUtil->NormalizeDocInfo(&vDocInfo);
		if(hr == S_FALSE)
		{
			LOG_ERROR(L"Document Contains no namespace info, import aborted")
			return hr;
		}
		else if (hr != S_OK)
		{
			LOG_ERROR(L"Problem extracting namespace info, import aborted")
			throw hr;
		}
		
		short namecount;
		BSTR *pBmknames = NULL; //the call allocates and returns an array of BSTRs
		//remember to call CoTaskMemFree
		hr = pXreg->GetRecords(vDocInfo, &namecount, &pBmknames );
		try
		{
			MergeLog(mpLog);
		}
		catch(...)
		{
			LOG_ERROR(L"Error merging registry log")
		}
		CHECKHR(hr);


		if(hr == S_OK)
		{
			assert(pBmknames);
			CHECKHR(BuildCompositeMoniker(pbc, namecount, pBmknames, ppCompositeMk));

			//This buffer is full of pointers also allocated by CoTaskMemAlloc
			//Must free them each. 

			for(int i= 0; i<namecount; i++)
			{
				CoTaskMemFree(pBmknames[i]);
			}
			CoTaskMemFree(pBmknames);

		}
		else
		{
			LOG_ERROR(L"No recognized elements in the document")
		}
	}
	catch(...)
	{
		RELEASE(*ppCompositeMk);
		hr = E_FAIL;
	}
	VariantClear(&vDocInfo);
	DONE_LOG_SECTION
	return hr;
}


HRESULT XMLImp::BindTransformMoniker(IBindCtx *pbc, IMoniker *pmk, ICoercionTransform **xfrm, IMoniker **ppMkNotBound)
{
	USES_LOG_SECTION(L"Binding Transformation object", LOG_PROC)
	HRESULT hr = S_OK;
	try
	{
//		CreateBindCtx( 0, &pbc ); 
//		unsigned long chEaten;
//		CHECKHR(MkParseDisplayNameEx(pbc, pBmk, &chEaten, &pmk));
		if(pmk)
		{
#ifdef _DEBUG
				IEnumMoniker *penumMoniker = NULL;
				IMoniker* submk = NULL;
				CHECKHR((pmk)->Enum(1, &penumMoniker));
				unsigned long celtFetched;
				unsigned long dwMksys;
				CHECKHR(pmk->IsSystemMoniker(&dwMksys));

				while(1)
				{
					CHECKHR(penumMoniker->Next(1, &submk, &celtFetched));
					if(!celtFetched)
						break;
					CHECKHR(submk->IsSystemMoniker(&dwMksys));
					hr = 0;
					IXfrmMk *pxmk = NULL;
					CHECKHR(submk->QueryInterface(IID_PPV(IXfrmMk, &pxmk)));
					RELEASE(pxmk);
					RELEASE(submk);
				}
				RELEASE(penumMoniker);
#endif
			hr = pmk->BindToObject(pbc, 0, IID_ICoercionTransform, (void**)xfrm);
			switch(hr)
			{
				//an alternativbe to using a message would be to stash some 
				//info in the BindCOntext at the point where teh decision is made to
				//skip the binding. 
			case S_OK:
				break;
			case MK_E_NOTBOUND:
				{
					(*ppMkNotBound = pmk)->AddRef();
				}
				break;
			case MK_E_NOOBJECT:
			default:
				throw hr;
				break;
			}
		}
	}
	catch(...)
	{
		LOG_ERROR(L"Moniker Binding Failed");
		hr = E_FAIL;
	}
	DONE_LOG_SECTION
	return hr;
}



HRESULT XMLImp::OpenRegistry(IXMLTransformRegistry **ppReg)
{
	USES_LOG_SECTION(L"Locating Registry", LOG_PROC)
	HRESULT hr = S_OK;
	RELEASE(*ppReg);
	ISupportXMLErrorLog *pXMLLog = NULL;

	try
	{
/*		TODO make this work?
		MULTI_QI multiQI[1];
		multiQI[0].pIID = IID_IXMLTransformRegistry;
		multiQI[0].pItf = NULL ;
		multiQI[0].hr = 0 ;
		CoCreateInstanceEx(m_configdata.clsidRegistry, NULL, CLSCTX_SERVER , NULL,  1, multiQI);
		CHECKHR(multiQI[0].hr);
		if(hr == S_OK)
		{
			*ppReg = multiQI[0].pItf; //no need to addref this?
		}
*/
		CHECKHR(CoCreateInstance(m_configdata.clsidRegistry, NULL, CLSCTX_SERVER, IID_PPV(IXMLTransformRegistry, ppReg)));
		assert(*ppReg);

		CHECKHR((*ppReg)->QueryInterface(IID_PPV(ISupportXMLErrorLog, &pXMLLog)));
		assert(pXMLLog);
		int detail;
		CHECKHR(mpLog->get_detail(&detail));
		CHECKHR(pXMLLog->set_logDetailLimit(detail));

		assert(m_configdata.pszRegistryURL);
		LOG_ENTRY(m_configdata.pszRegistryURL);
		hr = (*ppReg)->dbOpen(m_configdata.pszRegistryURL);
		try
		{
			MergeLog(mpLog);
		}
		catch(...)
		{
			LOG_ERROR(L"Error merging registry contact log")
		}
		CHECKHR(hr);

	}
	catch(...)
	{
		LOG_ERROR(L"Failed to contact transform registry.")
		RELEASE(*ppReg); //return NULL on failure
	}
	RELEASE(pXMLLog);
	DONE_LOG_SECTION
	return hr;
};






HRESULT XMLImp::CreateExternalCoercer(IXMLDocCoercion2 **ppCoercion)
{
	HRESULT hr = S_OK;

	//TODO possibly replace with a call to CoCreateInstanceEx for remote coercion
	try
	{
		CHECKHR(CoCreateInstance(m_configdata.clsidXSLCoercer, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDocCoercion2, (void**)ppCoercion));
	}
	catch(...)
	{
		RELEASE(*ppCoercion);//return NULL
		hr = S_FALSE;//no biggee
	}

	return hr;
}

HRESULT XMLImp::BuildAtomicXfrmMoniker(IBindCtx *pbc, LPOLESTR pszDispName, IMoniker **pMk)
{
	HRESULT hr = S_OK;
	//What's being passed in here is an xml fragments. IN order to make this into 
	//Moniker Display Name, we need to prepend the progid string 
	assert(*pMk == NULL);

//	LPOLESTR pszMkDisplayNameBase = SysAllocString(L"XMLDocUtil.XfrmMk:");
/*
until we figure out how to fix up the version-indepenedent progID at uninstall time, we're faced
with the problem that uninstalling one versoin will remove the VIPID for all others making the
above string unresolvable. For the moment we must use a version specifc progID and forgo writing
the VIPID. see XfrmMk.rgs and diff against R3.
*/
	LPOLESTR pszMkDisplayNameBase = SysAllocString(L"XMLDocUtil.XfrmMk.2:");
	LPOLESTR pszMkDisplayNameComplete = NULL;
	try
	{
		//Build up a displayname
		if(pszMkDisplayNameComplete)
			SysFreeString(pszMkDisplayNameComplete);
		int len = wcslen(pszMkDisplayNameBase) + wcslen(pszDispName);
		pszMkDisplayNameComplete = SysAllocStringLen(NULL, len);
		assert(pszMkDisplayNameComplete);
		if(!pszMkDisplayNameComplete)
			throw E_OUTOFMEMORY;
		wcscpy(pszMkDisplayNameComplete, pszMkDisplayNameBase);
		wcscat(pszMkDisplayNameComplete, pszDispName);

		unsigned long chEaten;
		CHECKHR(MkParseDisplayNameEx(pbc, pszMkDisplayNameComplete, &chEaten, pMk));
		if(hr != S_OK)
			throw E_FAIL;
	}
	catch(...)
	{
		RELEASE(*pMk);
		hr = E_FAIL;
	}
	return hr;
}

HRESULT XMLImp::StashClassActivator(IBindCtx *pbc)
{
	HRESULT hr = S_OK;
	LPOLESTR pszkey = NULL;
	try
	{
		if(!m_ClassActivator)
		{
			StringFromCLSID(CLSID_XfrmMk, &pszkey);
			CHECKHR(CoCreateInstance(CLSID_LoadXfrmActivator, NULL, CLSCTX_INPROC_SERVER, IID_PPV(IUnknown, &m_ClassActivator)));
			CHECKHR(pbc->RegisterObjectParam(pszkey, m_ClassActivator));
		}
	}
	catch(...)
	{
	}
	CoTaskMemFree(pszkey);
	return hr;

}

//Some ini file code borrowed from BitmapIO
void XMLImp::GetCfgFilename(TCHAR* filename, TCHAR* filenamebase)
{
    if (!filename || !filenamebase) {
        assert(FALSE);
    }

	assert(m_ifx.byref);
    _tcscpy(filename, ((Interface*)(m_ifx.byref))->GetDir(APP_PLUGCFG_DIR));

    if (filename)
        if (_tcscmp(&filename[_tcslen(filename)-1], _T("\\")))
            _tcscat(filename, _T("\\"));
    _tcscat(filename, filenamebase);   
}

void XMLImp::ReadINI()
{
	USES_CONVERSION;
	HROK;
    TCHAR filename[MAX_PATH];
	TCHAR logfilename[MAX_PATH];
	TCHAR regfilename[MAX_PATH];
    TCHAR buffer[MAX_PATH];
	TCHAR pszDefault[MAX_PATH];
	LPOLESTR pszClsID = NULL;

    GetCfgFilename(filename, XMLIO_INI_FILENAME);
    GetCfgFilename(logfilename, XMLIO_LOG_FILENAME);
	GetCfgFilename(regfilename, XMLIO_REGISTRY_FILENAME);
	LPOLESTR pszTemp = T2OLE(regfilename);
	LPOLESTR pszTemp2 = T2OLE(logfilename);

    wsprintf(pszDefault,"%d", XMLIO_CONFIGDATA_VERSION);    // set it to the current version
    GetPrivateProfileString(XMLIO_SECTION, XMLIO_VERSION, pszDefault, buffer,  sizeof(buffer), filename);
		m_configdata.version   = (DWORD) atoi(buffer);
	try
	{
		if (!m_configdata.version || m_configdata.version != XMLIO_CONFIGDATA_VERSION)
		{//set the defaults and write them
			throw E_FAIL;
		}
		else
		{//read the data
//		    wsprintf(pszDefault,"http://vizdevel/people/john.hutchinson/SchemaRegistry/SchemaRegistry.xml");
		    wsprintf(pszDefault, "%s", regfilename);
			int test = GetPrivateProfileString(XMLIO_SECTION, XMLIO_REGISTRY_URL, pszDefault, buffer, sizeof(buffer), filename);
			m_configdata.pszRegistryURL     = SysAllocString(T2OLE(buffer));

			hr = ProgIDFromCLSID(CLSID_XSLReg, &pszClsID);
			if (SUCCEEDED(hr)) 
			{
				GetPrivateProfileString(XMLIO_SECTION, XMLIO_PROGID_REGISTRY,   OLE2T(pszClsID), buffer, sizeof(buffer), filename);
				CHECKHR(CLSIDFromProgID(T2OLE(buffer), &m_configdata.clsidRegistry));
			}
			else
				m_configdata.clsidRegistry = CLSID_XSLReg;

			if(pszClsID)
			{
				CoTaskMemFree(pszClsID);
				pszClsID = NULL;
			}

			//JH 10/16/01
			//The client code (this plugin) is not compatible with the current version of the document coercer (version 2)
			//we want to insure that we only use version 1.
			TSTR ver1progID = TSTR("XMLDocUtil.DocCoercion.1");
			GetPrivateProfileString(XMLIO_SECTION, XMLIO_PROGID_COERCER, ver1progID.data(), buffer, sizeof(buffer), filename);
			hr = CLSIDFromProgID(T2OLE(buffer), &m_configdata.clsidXSLCoercer);
			if(FAILED(hr))
				m_configdata.clsidXSLCoercer = nullGUID;

			/*
			hr = ProgIDFromCLSID(CLSID_DocCoercion, &pszClsID);
			if (SUCCEEDED(hr)) 
			{
				GetPrivateProfileString(XMLIO_SECTION, XMLIO_PROGID_COERCER,   OLE2T(pszClsID), buffer, sizeof(buffer), filename);
				CHECKHR(CLSIDFromProgID(T2OLE(buffer), &m_configdata.clsidXSLCoercer));
			}
			else
				m_configdata.clsidXSLCoercer = CLSID_DocCoercion;
				*/

			if(pszClsID)
			{
				CoTaskMemFree(pszClsID);
				pszClsID = NULL;
			}

		    wsprintf(pszDefault,"%d", 1);    // set it to the current version
			GetPrivateProfileString(XMLIO_SECTION, XMLIO_VERBOSITY,   pszDefault, buffer, sizeof(buffer), filename);
			m_configdata.verbose = atoi(buffer);
#ifdef _DEBUG
			m_configdata.verbose = 4;
#endif
			m_configdata.pszLogFile = SysAllocString(pszTemp2);
		}
	}
	catch(...)
	{
		m_configdata.version     = XMLIO_CONFIGDATA_VERSION;

		//		m_configdata.pszRegistryURL = SysAllocString(L"http://vizdevel/people/john.hutchinson/SchemaRegistry/SchemaRegistry.xml");

		//we've decided to use a static copy of the registry by default.
		//users can change this to point to the dynamic version on the web
		m_configdata.pszRegistryURL = SysAllocString(pszTemp);
		m_configdata.clsidRegistry = CLSID_XSLReg;
		m_configdata.clsidXSLCoercer = CLSID_DocCoercion;
		m_configdata.verbose = 1;
#ifdef _DEBUG
			m_configdata.verbose = 4;
#endif
		m_configdata.pszLogFile = SysAllocString(pszTemp2);
	}
	WriteINI();
}

//done
void XMLImp::WriteINI()
{	HROK
    TCHAR filename[MAX_PATH];
 	TCHAR buffer[MAX_PATH];

    GetCfgFilename(filename, XMLIO_INI_FILENAME);
	LPOLESTR pszProgID = NULL;
    
    wsprintf(buffer,"%d", XMLIO_CONFIGDATA_VERSION);    // set it to the current version
    WritePrivateProfileString(XMLIO_SECTION, XMLIO_VERSION, buffer, filename);

    wsprintf(buffer,"%S", m_configdata.pszRegistryURL);
    WritePrivateProfileString(XMLIO_SECTION, XMLIO_REGISTRY_URL, buffer, filename);

	try
	{
		CHECKHR(ProgIDFromCLSID(m_configdata.clsidRegistry, &pszProgID));
	    wsprintf(buffer,"%S", pszProgID);
		CoTaskMemFree(pszProgID);
		WritePrivateProfileString(XMLIO_SECTION, XMLIO_PROGID_REGISTRY,   buffer,  filename);

		CHECKHR(ProgIDFromCLSID(m_configdata.clsidXSLCoercer, &pszProgID));
	    wsprintf(buffer,"%S", pszProgID);
		CoTaskMemFree(pszProgID);
		WritePrivateProfileString(XMLIO_SECTION, XMLIO_PROGID_COERCER,   buffer,  filename);
	}
	catch(...)
	{
		if(pszProgID)
			CoTaskMemFree(pszProgID);
	}

	if(m_configdata.verbose)
	{
		wsprintf(buffer,"%d", m_configdata.verbose);
		WritePrivateProfileString(XMLIO_SECTION, XMLIO_VERBOSITY, buffer, filename);
	}
}

void XMLImp::start_log()
{
	HROK;
	IErrorInfo *pError = NULL;
	LPOLESTR pszHeading = SysAllocString(L"XML Import");
	try
	{
		CHECKHR(CoCreateInstance(CLSID_XMLLog, NULL, CLSCTX_INPROC_SERVER, IID_PPV(IXMLLog, &mpLog)));
		if(mpLog)
		{
			CHECKHR(mpLog->set_detail(m_configdata.verbose));

//			CHECKHR(mpLog->set_Heading(L"XML Import"));
			// The above causes an access violation in Release builds
			CHECKHR(mpLog->set_Heading(pszHeading));

			CHECKHR(mpLog->QueryInterface(IID_PPV(IErrorInfo, &pError)));
			assert(pError);
			::SetErrorInfo(0, pError);
		}
	}
	catch(...)
	{
	}
	SysFreeString(pszHeading);
	HRESULT logcreated = hr;
	MaxAssert(logcreated == S_OK);
	RELEASE(pError);
}

void XMLImp::stop_log()
{
	HROK;
	IPersistFile *pPersistFile = NULL;
	try
	{
		if(mpLog)
		{
			CHECKHR(mpLog->QueryInterface(IID_PPV(IPersistFile, &pPersistFile)));
			CHECKHR(pPersistFile->Save(m_configdata.pszLogFile, 1));
			::SetErrorInfo(0, 0);
		}
	}
	catch(...)
	{
	}
	HRESULT logsaved = hr;
	MaxAssert(logsaved == S_OK);
	RELEASE(pPersistFile);
	RELEASE(mpLog);

}

HRESULT XMLImp::ShowLog()
{
	HROK;
	USES_CONVERSION;
	IMoniker *pmk = NULL;
	IBindCtx *pbc = NULL;
	IWebBrowser2 *pbrows = NULL;


	IPersist *pPersist = NULL;
//	CLSID cid;

	try
	{
		//We expect the following call to fail because we specify CREATE_NEW
		//and the file should already exist.
		HANDLE dwHandle = CreateFile(OLE2T(m_configdata.pszLogFile), 
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		
		DWORD logExists = (dwHandle == INVALID_HANDLE_VALUE);
		MaxAssert(logExists);//this should never happen in real life

		if(logExists)
			ShellExecute(GetDesktopWindow(), _T("open"), OLE2T(m_configdata.pszLogFile), NULL, NULL, SW_MAXIMIZE);
		else
		{
			CloseHandle(dwHandle);
			dwHandle = INVALID_HANDLE_VALUE;
		}

	}
	catch(...)
	{
	}
	RELEASE(pmk);
	RELEASE(pbc);
	RELEASE(pbrows);
	return hr;
}

HRESULT XMLImp::DelegatingCoerceDocument(LPOLESTR pszURL, IXMLDOMDocument **pRoot, IMoniker *pXfrmMk, int suppressPrompts)
{
	HROK;
	USES_LOG_SECTION(L"Transforming XML", LOG_INFO)

	IXMLDocCoercion2 *pCoercion = NULL;

	//TODO initalize a variant to contain application-specific coercion goals
	//Whatever goes into this variant needs to represent the set of transformations.
	//alternatively this data could be part of the of the coercion instance.
	VARIANT vtarg;
	VariantInit(&vtarg);

	try
	{
		CHECKHR(CreateExternalCoercer(&pCoercion));
		if(pCoercion)
		{
			//TODO Configure logging in this component

			//6/22/01
			//signature has changed
			CComVariant vEmpty;

			CHECKHR(pCoercion->SpecifyCoercionObjective(vtarg, vEmpty));
			CComBSTR bstrURL(pszURL);
			CComVariant varBstrSrc(bstrURL);
			CHECKHR(pCoercion->CoerceDocument(/*[in]*/varBstrSrc, /*[out]*/pRoot, /*[out]*/&pXfrmMk, /*[in]*/suppressPrompts));
			//TODO Merge any log returned by this component
		}
		RELEASE(pCoercion);
	}
	catch(...)
	{
	}
	RELEASE(pCoercion);
	DONE_LOG_SECTION;
	return hr;
}
#endif //KAHN_OR_EARLIER