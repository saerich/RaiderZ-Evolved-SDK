// MaxDocument.h : Declaration of the CMaxDocument

#ifndef __MAXDOCUMENT_H_
#define __MAXDOCUMENT_H_

#include "resource.h"       // main symbols
#include "resOverride.h"    // alias some symbols for specific versions
#include "max.h" //for referencemaker
#include "notify.h"

//this define controls some code in the document show method 
//which attemps to bring the app to the foreground
#define FOREGROUND_FIX

interface IMaxMaterialCollection;
//interface IFileLinkDocument;
/////////////////////////////////////////////////////////////////////////////
// CMaxDocument
#ifdef EXTENDED_OBJECT_MODEL
class ATL_NO_VTABLE CMaxDocument : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMaxDocument, &CLSID_MaxDocument>,
	public IDispatchImpl<IMaxDocument, &IID_IMaxDocument, &LIBID_COMSRVLib>,
	public IPersistFile
{
public:
	CMaxDocument();
	HRESULT FinalConstruct();
	void FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_MAXDOCUMENT)
DECLARE_PROTECT_FINAL_CONSTRUCT()
//DECLARE_CLASSFACTORY_SINGLETON(CMaxDocument)

BEGIN_COM_MAP(CMaxDocument)
	COM_INTERFACE_ENTRY(IMaxDocument)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IPersistFile)
END_COM_MAP()

// IMaxDocument
public:
	STDMETHOD(get_FileLinkDocument)(/*[out, retval*/ IUnknown **ppLinkDocUnk);
	STDMETHOD(get_Materials)(/*[out, retval]*/ IMaxMaterialCollection **ppMtls);

// IPersist
	STDMETHOD(GetClassID)(CLSID *pClassID);

// IPersistFile
	STDMETHOD(IsDirty)(void);
	STDMETHOD(Load)(LPCOLESTR pszFileName, DWORD dwMode );
	STDMETHOD(Save)(LPCOLESTR pszFileName,BOOL fRemember); 
	STDMETHOD(SaveCompleted)(LPCOLESTR pszFileName);
	STDMETHOD(GetCurFile)(LPOLESTR *ppszFileName ); 

private:
	CComPtr<IMaxMaterialCollection> mpMtls;
	CComPtr<IUnknown> mpLinkDocUnk;
	CComQIPtr<IMaxApp> mpApp;// hold a ref on the containing app
	DWORD dwRotCookie;//cookie to our file moniker
	bool bUserLocked;//externally locked on behalf of the user

	//manage our file moniker
	HRESULT RevokeMoniker();
	HRESULT RegisterMoniker();

	//manage strong lock
	void StrongLock(bool bLocking);

public:
	STDMETHOD(GetApplication)(IMaxApp ** ppApp);
	STDMETHOD(Show)(void);
	void OnMaxNotify(NotifyInfo *info);

};
#endif //EXTENDED_OBJECT_MODEL
#endif //__MAXDOCUMENT_H_
