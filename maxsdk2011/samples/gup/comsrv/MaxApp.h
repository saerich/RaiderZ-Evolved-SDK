// MaxApp.h : Declaration of the CMaxApp

#ifndef __MAXAPP_H_
#define __MAXAPP_H_

#include "resource.h"       // main symbols
#include "resOverride.h"    // alias some symbols for specific versions

class Interface;//forward declare the max interface class

/////////////////////////////////////////////////////////////////////////////
// CMaxApp
#ifdef EXTENDED_OBJECT_MODEL
class ATL_NO_VTABLE CMaxApp : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMaxApp, &CLSID_MaxApp>,
	public IDispatchImpl<IMaxApp, &IID_IMaxApp, &LIBID_COMSRVLib>
{
public:
	CMaxApp();
	~CMaxApp();
	HRESULT FinalConstruct();
	void FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_MAXAPP)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMaxApp)
	COM_INTERFACE_ENTRY(IMaxApp)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IMaxApp
public:
	STDMETHOD(get_LegacyRenderInterface)(/*[out, retval]*/ IMaxRenderer** ppRender);
	STDMETHOD(get_ActiveDocument)(/*[out, retval]*/ IMaxDocument **ppDoc);
	STDMETHOD(get_Visible)(VARIANT_BOOL* pVal);
	STDMETHOD(put_Visible)(VARIANT_BOOL newVal);
	STDMETHOD(put_ActiveDocument)(IMaxDocument * pDoc);

private:
	CComPtr<IMaxDocument> mpDoc; //the visible document
	CComPtr<IMaxRenderer> mpRenderer;
	bool m_Visible;
	DWORD m_dwActiveObjectCookie;
	HWND mHwnd;
	Interface *m_ip;
};
#endif //EXTENDED_OBJECT_MODEL

#endif //__MAXAPP_H_
