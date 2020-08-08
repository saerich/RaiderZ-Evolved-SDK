// MaxApp.cpp : Implementation of CMaxApp
#include "stdafx.h"
#include "Comsrv.h"
#include "MaxApp.h"

#include "MaxDocument.h"
#include "MaxRenderer.h"

#ifdef EXTENDED_OBJECT_MODEL

//These headers are not available in the public sdk 
#include "..\..\..\..\cfgmgr\cfgmgr.h"
#include "..\..\..\..\include\reslib.h"
#pragma comment(lib, "../../../../lib/resmgr.lib")
#pragma comment(lib, "../../../../lib/cfgmgr.lib")



/////////////////////////////////////////////////////////////////////////////
// CMaxApp
CMaxApp::CMaxApp()
{
	mHwnd = 0;//this will hold the max window
	m_ip = 0;
	m_Visible = 0;
	m_dwActiveObjectCookie = 0;
}

CMaxApp::~CMaxApp()
{
	int foo = 42;
}



HRESULT CMaxApp::FinalConstruct()
{
	HRESULT hr = CMaxRenderer::_CreatorClass::CreateInstance(NULL, IID_IMaxRenderer, (void**)&mpRenderer.p);
	if(!SUCCEEDED(hr))
		return hr;

	//add other initialization here
	m_ip = GetCOREInterface();
	mHwnd = m_ip->GetMAXHWnd();
	return hr;
}

void CMaxApp::FinalRelease()
{
	if(mpDoc)
		mpDoc = NULL;//just for debugging this should happen automagically
}

STDMETHODIMP CMaxApp::get_ActiveDocument(IMaxDocument **ppDoc)
{
	if(!ppDoc)
		return E_INVALIDARG;
	
	*ppDoc = NULL;

	if(mpDoc)
	{
		*ppDoc = mpDoc;
		(*ppDoc)->AddRef();
		return S_OK;
	}
	return S_FALSE;
}



STDMETHODIMP CMaxApp::get_LegacyRenderInterface(IMaxRenderer **ppRender)
{
	// This is a stop-gap method to gain access to the older IMaxRenderInterface
	// Once the functionality of this interface is redistributed to interfaces of the OM,
	// this method will be deprecated. 

	if(!ppRender)
		return E_INVALIDARG;
	if(mpRenderer)
	{
		*ppRender = mpRenderer;
		(*ppRender)->AddRef();
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CMaxApp::get_Visible(VARIANT_BOOL* pVal)
{
	*pVal = m_Visible;
	return S_OK;
}

//TODO Refactor into show and hide methods
//probably need to pass an arg to the show method to indicate if
//we should latch user control.
//FOr the moment it's ok that we always latch cause there's no support for
//embedding
STDMETHODIMP CMaxApp::put_Visible(VARIANT_BOOL newVal)
{
	HRESULT hr;
	//managed as a latch
	//once set to visible, application must be closed by teh user
	if(newVal)
	{
		if(!m_Visible)
		{
			m_Visible = true;
			hr = CoLockObjectExternal(this, true, false);
			ATLASSERT(SUCCEEDED(hr));

			// xavier robitaille | 2003.02.28 | set the maximized state of the window
			int isMaximized = 0;
			getCfgMgr().setSection(GetResString(IDS_DB_WINDOWSTATE));
			if(getCfgMgr().keyExists(GetResString(IDS_DB_WINDOW_MAXIMIZED)))
				getCfgMgr().getInt(GetResString(IDS_DB_WINDOW_MAXIMIZED), &isMaximized);

			//if the app is visible make sure we have max/min controls
			DWORD style = GetWindowLong(mHwnd,GWL_STYLE);
			style |= WS_CAPTION|WS_SIZEBOX|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX;
			if(isMaximized) style |= WS_MAXIMIZE;
			SetWindowLong(mHwnd, GWL_STYLE, style);

			//not sure why but when started in server mode the viewport
			//doesn't get drawn, try to force it here
			ShowWindow(mHwnd, SW_SHOW); 
			if(m_ip)
				m_ip->RedrawViews(0);

		}

		//do this first (before setting active object) so that other apps can revoke
		SetActiveWindow(mHwnd);//2/20/04 I'm tempted to remove this, it seems wrong
		//but I'm leaving it because we're in lockdown

		//we also make it the active application object
		/*if(0 == m_dwActiveObjectCookie)
		{
			hr = RegisterActiveObject(this, CLSID_MaxApp, ACTIVEOBJECT_WEAK, &m_dwActiveObjectCookie);
			ATLASSERT(SUCCEEDED(hr));
			ATLASSERT(GetActiveObject()
		}
		*/

#ifndef NO_ACTIVEOBJECT_API
		//2/20/04 Getting more aggressive about this since a stale cookie
		//can be fatal to clients of the active object
		CComPtr<IUnknown> pUnk;
		hr = GetActiveObject(CLSID_MaxApp, 0, &pUnk);
		if(pUnk != this)
		{
			ATLASSERT(m_dwActiveObjectCookie == 0);
			hr = RegisterActiveObject(this, CLSID_MaxApp, ACTIVEOBJECT_WEAK, &m_dwActiveObjectCookie);
			ATLASSERT(SUCCEEDED(hr));
#ifdef _DEBUG
			hr = GetActiveObject(CLSID_MaxApp, 0, &pUnk.p);
			ATLASSERT(pUnk == this);
#endif
		}
#endif
	}
	else
	{
#ifndef NO_ACTIVEOBJECT_API
		if(m_dwActiveObjectCookie)
		{
			hr = RevokeActiveObject(m_dwActiveObjectCookie, 0);
			m_dwActiveObjectCookie = 0;
		}
#endif
		//ShowWindow(mHwnd, SW_HIDE);
	}
	return S_OK;
}

STDMETHODIMP CMaxApp::put_ActiveDocument(IMaxDocument * pDoc)
{
	if(mpDoc != pDoc)
		mpDoc = pDoc;
	return S_OK;
}
#endif //EXTENDED_OBJECT_MODEL