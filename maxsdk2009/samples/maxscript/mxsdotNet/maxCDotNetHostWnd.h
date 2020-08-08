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

#pragma managed

namespace MXS_dotNet
{

	class maxDotNetControl;

	public class maxCDotNetHostWnd : public CDotNetHostWnd	{

	private:
		typedef CDotNetHostWnd inherited;

		maxDotNetControl* mMaxDotNetControl;
	
	public:
		ref class delegate_proxy_type : public delegate_proxy_type_base
		{
		public:
			delegate_proxy_type(maxCDotNetHostWnd* pNativeTarget);
			virtual ~delegate_proxy_type();

		};

		virtual Value* GetMXSContainer();

		bool CreateControl(
			HWND hParent, 
			int id, 
			int x, 
			int y, 
			int w, 
			int h, 
			System::Windows::Forms::Control^ aControl);

		maxCDotNetHostWnd(maxDotNetControl* aMaxDotNetControl);
		virtual ~maxCDotNetHostWnd();

		virtual void ProcessEvent(System::String ^eventName, System::Object ^delegateArgs);

		msclr::delegate_map::internal::delegate_proxy_factory<maxCDotNetHostWnd> m_delegate_map_proxy;

		virtual delegate_proxy_type_base^ GetProxyType();

	protected:
		virtual void DoOnDestroy();
		virtual void DoRegisterControl();
		
		virtual void LoadCreateControlParams();

	};


}