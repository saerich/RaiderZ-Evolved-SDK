//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// AUTHOR: David Cunningham - created Oct 16, 2006
//***************************************************************************/
//

#include "maxDotNetControl.h"
#include "maxDotNetButton.h"
#include "3dsmaxport.h"
#include "maxCDotNetHostWnd.h"
#include "custcont.h"


using namespace MXS_dotNet;

#pragma managed

maxDotNetControl::delegate_proxy_type::delegate_proxy_type(maxDotNetControl* pNativeTarget)	{

}
maxDotNetControl::delegate_proxy_type::~delegate_proxy_type()	{

}

void maxDotNetControl::delegate_proxy_type::NeedFocusHandler(System::Object^ sender, System::EventArgs^ args)	{
	DisableAccelerators();
}

void maxDotNetControl::delegate_proxy_type::ReleaseFocusHandler(System::Object^ sender, System::EventArgs^ args)	{
	EnableAccelerators();
}

void maxDotNetControl::delegate_proxy_type::detach()	{

}

maxDotNetControl::maxDotNetControl(HWND aHandle, int id) : 
	mHwnd(aHandle), 
	mID(id), 
	mDotNetHostWnd(NULL)	
{
	DLSetWindowLongPtr( mHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this) );

	InitControl();
}

maxDotNetControl::maxDotNetControl(HWND aHandle, int id, const MCHAR* aControlClass) : 
	mHwnd(aHandle), 
	mID(id), 
	mControlClass(aControlClass != NULL ? aControlClass : ""),
	mDotNetHostWnd(NULL)	
{
	DLSetWindowLongPtr( mHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this) );

	InitControl();
}

maxDotNetControl::~maxDotNetControl()	{
	if(mDotNetHostWnd)	{
		delete mDotNetHostWnd;
	}
}

void maxDotNetControl::InitControl()	{

	if(!mControlClass.empty())	{
		AddControlGeneric(mControlClass.c_str());
	}
}

void maxDotNetControl::AddControlGeneric(const MCHAR* aControlClass)	{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	try
	{

		RECT pos = {0};
		GetClientRect(mHwnd, &pos);

		int width = pos.right - pos.left;
		int height = pos.bottom - pos.top;

		System::Windows::Forms::Control^ control = CDotNetHostWnd::CreateGenericControl(aControlClass);


		if(control != nullptr)	{
			mDotNetHostWnd = new maxCDotNetHostWnd(this);   // To host .NET controls
			BOOL res = mDotNetHostWnd->CreateControl(
				mHwnd, 
				mID, 
				pos.left, 
				pos.top, 
				width, 
				height, 
				control);

			RegisterOnCallbackEvents(control);
		}
	}
	catch (...) 
	{

		throw;
	}
}

void maxDotNetControl::RegisterOnCallbackEvents(System::Windows::Forms::Control^ aControl)	{
	//if(MaxCustomControls::MaxUserControl^ maxControl = dynamic_cast<MaxCustomControls::MaxUserControl^>(aControl) )	{
	//	maxControl->NeedKeyboardFocus += gcnew System::EventHandler(m_delegate_map_proxy.get_proxy(this), &delegate_proxy_type::NeedFocusHandler);
	//	maxControl->ReleasingKeyboardFocus += gcnew System::EventHandler(m_delegate_map_proxy.get_proxy(this)	, &delegate_proxy_type::ReleaseFocusHandler);
	//}
}

void maxDotNetControl::AddControl(System::Windows::Forms::Control^ aControl)	{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	try
	{

		RECT pos = {0};
		GetClientRect(mHwnd, &pos);

		int width = pos.right - pos.left;
		int height = pos.bottom - pos.top;


		if(aControl != nullptr)	{
			mDotNetHostWnd = new maxCDotNetHostWnd(this);   // To host .NET controls
			BOOL res = mDotNetHostWnd->CreateControl(
				mHwnd, 
				mID, 
				pos.left, 
				pos.top, 
				width, 
				height, 
				aControl);

			RegisterOnCallbackEvents(aControl);
		}
	}
	catch (...) 
	{

		throw;
	}
}

System::Windows::Forms::Control^ maxDotNetControl::GetControl()	{
	if(mDotNetHostWnd != NULL)	{
		return mDotNetHostWnd->GetProxyType()->get_control();
	}
	return nullptr;
}

LRESULT CALLBACK MXS_dotNet::dotNetControlWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )	{

	static bool doOnce = true;
	//if(doOnce)	{
	//	// toggle the max custom control lib
	//	MaxCustomControls::UIOptions::LaunchedFromMax = true;
	//	doOnce = false;
	//}
	
	maxDotNetControl *c = DLGetWindowLongPtr<maxDotNetControl *>(hwnd);
	
	switch ( message ) {

		
		case WM_CREATE:
			{

				//LPCREATESTRUCT createStruct = (LPCREATESTRUCT) lParam;

				//if(createStruct->lpszClass[0] != _T('\0'))	{
				//	if(_tcscmp(createStruct->lpszClass, DOT_NET_CLASS) == 0)	{
				//		// create here
				//		c = new MXS_dotNet::maxDotNetControl(hwnd, GetDlgCtrlID(hwnd));
				//		c->AddControl(gcnew MaxCustomControls::FileBrowseControl());
				//	}
				//}
				//else if (_tcscmp(buf, DOT_NET_CLASS) == 0)	{
				//	// create here
				//	c = new MXS_dotNet::maxDotNetButton(hwnd, 0);
				//}
			}
			return 0;
		case WM_DESTROY:
			if ( c )	{
				delete c;
			}
			c = NULL;
			DLSetWindowLongPtr( hwnd, 0 );
			break;
		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return TRUE;

}
