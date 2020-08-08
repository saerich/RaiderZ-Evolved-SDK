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

#pragma once

#include "CDotNetHostWnd.h"
#include "utils.h"

#pragma managed

const TCHAR* const DOT_NET_CLASS = _T("dotNetClassTest1");
const TCHAR* const DOT_NET_CLASS2 = _T("dotNetClassTest2");
const TCHAR* const DOT_NET_BUTTON = _T("dotNetButton");

namespace MXS_dotNet
{

	class maxCDotNetHostWnd;

	class maxDotNetControl	{
	public:

		ref class delegate_proxy_type 
		{
		public:
			delegate_proxy_type(maxDotNetControl* pNativeTarget);
			virtual ~delegate_proxy_type();

			void NeedFocusHandler(System::Object^ sender, System::EventArgs^ args);
			void ReleaseFocusHandler(System::Object^ sender, System::EventArgs^ args);

			void detach();

		};

		maxDotNetControl(HWND aHandle, int id);

		maxDotNetControl(HWND aHandle, int id, const MCHAR* aControlClass);
		virtual ~maxDotNetControl();

		void AddControlGeneric(const MCHAR* aControlClass);
		void AddControl(System::Windows::Forms::Control^ aControl);

		System::Windows::Forms::Control^ GetControl();

		

	protected:
		virtual void InitControl();
		virtual void RegisterOnCallbackEvents(System::Windows::Forms::Control^ aControl);

		msclr::delegate_map::internal::delegate_proxy_factory<maxDotNetControl> m_delegate_map_proxy;

	private:
		HWND mHwnd;
		int mID;

		MString mControlClass;


		maxCDotNetHostWnd* mDotNetHostWnd;
	};

	


	LRESULT CALLBACK dotNetControlWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
}