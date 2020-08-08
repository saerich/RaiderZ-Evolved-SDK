// MaxDocument.cpp : Implementation of CMaxDocument
#include "stdafx.h"
#include "Comsrv.h"
#include "MaxDocument.h"
#include "mscom.h"

#include "MaxMaterialCollection.h"
#include "MaxApp.h"

//don't use max's assert
#include <assert.h>

#ifdef EXTENDED_OBJECT_MODEL

//static which forwards to instance
void NotifyProc(void *param, NotifyInfo *info)
{
		CMaxDocument *pinst = (CMaxDocument *) param;
		pinst->OnMaxNotify(info);
}

/////////////////////////////////////////////////////////////////////////////
// CMaxDocument
CMaxDocument::CMaxDocument():bUserLocked(false)
{
	//notifications which cause us to revoke our file moniker
	RegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_PRE_NEW);
	RegisterNotification(NotifyProc, this, NOTIFY_SYSTEM_SHUTDOWN);


	//notifications which cause us to register a moniker
	RegisterNotification(NotifyProc, this, NOTIFY_FILE_POST_OPEN);
	//when saving, it could be a save as so need to update anyway
	RegisterNotification(NotifyProc, this, NOTIFY_FILE_POST_SAVE);
}

void CMaxDocument::OnMaxNotify(NotifyInfo *info)
{
	HRESULT hr;
	switch(info->intcode)
	{
	case NOTIFY_SYSTEM_SHUTDOWN:
		//break the reference cycle with the application
		mpApp.p->Release();
		mpApp.Detach();
		StrongLock(false);

	case NOTIFY_SYSTEM_PRE_RESET:
	case NOTIFY_SYSTEM_PRE_NEW:
		{
			RevokeMoniker();
		}
		break;
	case NOTIFY_FILE_POST_OPEN:
	case NOTIFY_FILE_POST_SAVE:
		{
			hr = RegisterMoniker();
			ATLASSERT(SUCCEEDED(hr));
		}
		break;
	default:
		{
			ATLASSERT(0);
		}
		break;
	}
}

HRESULT CMaxDocument::FinalConstruct()
{
		dwRotCookie = 0;

/*
	HRESULT hr = CMaxApp::_CreatorClass::CreateInstance(NULL, IID_IMaxApp, (void**)&mpApp.p);
	if(!SUCCEEDED(hr))
		return hr;
*/
	//create a reference to the cached application object.
	mpApp = GUP_MSCOM::GetAppObject();
	ATLASSERT(mpApp);
//	long test = mpApp.p->AddRef();
//	test = mpApp.p->Release();

	HRESULT hr = S_OK;
#ifdef EXTENDED_OBJECT_MODEL
	hr = CMaxMaterialCollection::_CreatorClass::CreateInstance(NULL, IID_IMaxMaterialCollection, (void**)&mpMtls.p);
	ATLASSERT(hr == S_OK);
#endif	// EXTENDED_OBJECT_MODEL
	return hr;

}

void CMaxDocument::FinalRelease()
{
	UnRegisterNotification(NotifyProc, this);

	if(dwRotCookie)
	{
		CComPtr<IRunningObjectTable> pRot;
		HRESULT hr = GetRunningObjectTable(0, &pRot.p);
		ATLASSERT(SUCCEEDED(hr));

		pRot->Revoke(dwRotCookie);
	}
}

STDMETHODIMP CMaxDocument::get_Materials(IMaxMaterialCollection **ppMtls)
{
	if(!ppMtls)
		return E_INVALIDARG;
	if(mpMtls)
	{
		*ppMtls = mpMtls;
		(*ppMtls)->AddRef();
		return S_OK;
	}
	return E_FAIL;
}

static const TSTR sFilelinkModuleName("stdplugs\\FileLink.dlu");

// DllGetClassObject function pointer
typedef HRESULT (STDAPICALLTYPE *DLLGETCLASSOBJECT_PROC)(REFCLSID, REFIID, LPVOID*);

//JH 1/28/04
/*
I've been thrashing on this... but the latest thinking is to code against 
#included clsids. Although this tightly couples the projects, it makes versioning
easier. The bottom line is that we want to load the in-proc server that 
matches (from an install perspective) this version.
*/

#ifndef _WIN64
#	import "..\..\..\..\dll\filelink\midl\win32\filelink.tlb" raw_interfaces_only, named_guids
#else
#	import "..\..\..\..\dll\filelink\midl\x64\filelink.tlb" raw_interfaces_only, named_guids
#endif

STDMETHODIMP CMaxDocument::get_FileLinkDocument( IUnknown **ppLinkDocUnk)
{
	HRESULT hr = S_OK;
	*ppLinkDocUnk = NULL;

/*	CComBSTR bstrProgID = "FileLink.FileLinkDocument";
	CLSID rclsid;
	hr = CLSIDFromProgID(bstrProgID.m_str, &rclsid);
	if(hr != S_OK)
		return hr;
*/
	if(!mpLinkDocUnk)
	{
//		CComBSTR bstrProgID = "FileLink.FileLinkDocument.2";
//		hr = mpLinkDocUnk.CoCreateInstance(bstrProgID.m_str);
		hr = mpLinkDocUnk.CoCreateInstance(FILELINKLib::CLSID_FileLinkDocument);

		if(hr != S_OK)
			return hr;
	}
/*
	if(!mpLinkDocUnk)
	{

		HMODULE hModule = LoadLibrary(sFilelinkModuleName.data());
		if (hModule == NULL) {
			// Failed to load DLL, the last error will be set by the system
			//
			return E_FAIL;
		}

		// Get the proc addres for function DllGetClassObject
		DLLGETCLASSOBJECT_PROC pFunc = (DLLGETCLASSOBJECT_PROC) 
			GetProcAddress(hModule, "DllGetClassObject");
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
//		hr = pFunc(rclsid, IID_IClassFactory, (void**) &pCF);
		hr = pFunc(FILELINKLib::CLSID_FileLinkDocument, IID_IClassFactory, (void**) &pCF);
		if (FAILED(hr) || pCF == NULL) {
			FreeLibrary(hModule);
			SetLastError(hr); // Failed to get interface
			return E_FAIL;
		}

		// Create the COM object using the class factory
		IUnknown* pUnk = NULL;
		hr = pCF->CreateInstance(NULL, IID_IUnknown, (void**) &pUnk);
		assert(SUCCEEDED(hr) && pUnk != NULL);
		mpLinkDocUnk = pUnk;//addref
		if (FAILED(hr) || mpLinkDocUnk == NULL) {
			SetLastError(hr); // Failed to create COM object
			return E_FAIL;
		}

	}
*/

	ATLASSERT(mpLinkDocUnk.p);
	(*ppLinkDocUnk = mpLinkDocUnk.p)->AddRef();
	
	return hr;
}

// IPersist
STDMETHODIMP CMaxDocument::GetClassID(CLSID *pClassID //Pointer to CLSID of object
)
{
	*pClassID = CLSID_MaxDocument;
	return S_OK;
}

// IPersistFile
STDMETHODIMP CMaxDocument::IsDirty(void)
{
	//TODO we may well need to implement this 
	//When ADT shuts down, it shut at least try to save us if we report being dirty.
	if (IsSaveRequired()) {
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CMaxDocument::Load(
	LPCOLESTR pszFileName, //Pointer to absolute path of the file to open
	DWORD dwMode //Specifies the access mode from the STGM enumeration
	)
{
	mpApp->put_ActiveDocument(this);
	LPTSTR filename;
	USES_CONVERSION;
	filename = OLE2T(pszFileName);
	Interface *max = GetCOREInterface();
	if (!max->LoadFromFile(filename)) {
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP CMaxDocument::Save(
	LPCOLESTR pszFileName, //Pointer to absolute path of the file 
	//where the object is saved
	BOOL fRemember //Specifies whether the file is to be the 
	//current working file or not
	)
{
	LPTSTR filename;
	USES_CONVERSION;
	filename = OLE2T(pszFileName);
	Interface *max = GetCOREInterface();
	if (!max->SaveToFile(filename)) {
		return E_FAIL;
	}

	return S_OK;
}

STDMETHODIMP CMaxDocument::SaveCompleted(
	LPCOLESTR pszFileName //Pointer to absolute path of the file 
	//where the object was saved
	)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMaxDocument::GetCurFile(
	LPOLESTR *ppszFileName //Pointer to the path for the current file 
	//or the default save prompt
	)
{
	USES_CONVERSION;
	Interface *max = GetCOREInterface();
	TSTR filename = max->GetCurFilePath();

    (*ppszFileName) = (LPOLESTR)CoTaskMemAlloc(sizeof(OLECHAR)*(filename.length() +1));
    if (NULL != (*ppszFileName))
    {
        wcscpy(*ppszFileName,T2OLE((LPCTSTR)filename));
        return S_OK;
    }
	return E_POINTER;
}

STDMETHODIMP CMaxDocument::GetApplication(IMaxApp ** ppApp)
{
	if(*ppApp)
		return E_INVALIDARG;

	(*ppApp = mpApp)->AddRef();
	return S_OK;
}

STDMETHODIMP CMaxDocument::Show(void)
{
	//this is managed as a latch. Once shown only the app can close 
	HRESULT hr = mpApp->put_ActiveDocument(this);
	ATLASSERT(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	StrongLock(true);

	//if we're going to show the doc, we better make sure the app is visible
	mpApp->put_Visible(VARIANT_TRUE);

	//now the app is logically visible, make the window physically visible
#ifdef FOREGROUND_FIX
	//The following should probably be factored out into a new InsureVisible method on the application
	//object.
	//BEGIN_INSURE_VISIBLE
	Interface *max = GetCOREInterface();
	HWND hMAxWnd = max->GetMAXHWnd();
	if(hMAxWnd != GetForegroundWindow())
	{
		BOOL bFG = SetForegroundWindow(hMAxWnd);
		if(!bFG)
		{
			FLASHWINFO finfo;
			ZeroMemory(&finfo, sizeof(FLASHWINFO));
			finfo.cbSize = sizeof(FLASHWINFO);
			finfo.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
			finfo.uCount = 999;
			finfo.hwnd = hMAxWnd;
			bFG = FlashWindowEx(&finfo);
		}
	}

	//what if the window is minimized
    WINDOWPLACEMENT wp;
    ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	GetWindowPlacement(hMAxWnd, &wp);
	if (wp.showCmd == SW_SHOWMINIMIZED || wp.showCmd == SW_MINIMIZE)
	    ShowWindow(hMAxWnd, SW_RESTORE);
	//END_INSURE_VISIBLE
#endif //FOREGROUND_FIX

	return hr;

}

HRESULT CMaxDocument::RevokeMoniker()
{
	if(dwRotCookie)
	{
		CComPtr<IRunningObjectTable> pRot;
		HRESULT hr = GetRunningObjectTable(0/*reserved*/, &pRot.p);
		ATLASSERT(SUCCEEDED(hr));
		if(FAILED(hr))
			return hr;

		DWORD temp = dwRotCookie;
		dwRotCookie = 0;
		return pRot->Revoke(temp);
	}
	return S_OK;
}

HRESULT CMaxDocument::RegisterMoniker()
{
	USES_CONVERSION;
	TSTR& pszFileName = GetCOREInterface()->GetCurFilePath();
	
	//revoke the old moniker if any
	RevokeMoniker();

	//register in the ROT
	CComPtr<IMoniker> pMk;
	HRESULT hr = CreateFileMoniker(T2W(pszFileName), &pMk.p);
	ATLASSERT(SUCCEEDED(hr));
	if(SUCCEEDED(hr))
	{
		CComPtr<IRunningObjectTable> pRot;
		hr = GetRunningObjectTable(0/*reserved*/, &pRot.p);
		ATLASSERT(SUCCEEDED(hr));
		if(FAILED(hr))
			return hr;

		CComPtr<IUnknown> punk;
		hr = this->QueryInterface(IID_IUnknown, (void**)&punk.p);
		ATLASSERT(SUCCEEDED(hr));
		hr = pRot->Register(0/*flags*/, punk.p, pMk, &dwRotCookie);
//		ATLASSERT(hr != MK_S_MONIKERALREADYREGISTERED ); // LAM - 2/28/03 - commented out pending fix from JohnH
	}
	return hr;
}

void CMaxDocument::StrongLock(bool bLocking)
{
	if(bLocking != bUserLocked)
	{
		CComPtr<IUnknown> punk;
		this->QueryInterface(IID_IUnknown, (void **) &punk.p);
		ATLASSERT(punk);

		HRESULT hr = CoLockObjectExternal(punk.p, bLocking, TRUE);
		ATLASSERT(SUCCEEDED(hr));

		if(bLocking)
			bUserLocked = bLocking;//never clear this member

	}
}

#endif //EXTENDED_OBJECT_MODEL
