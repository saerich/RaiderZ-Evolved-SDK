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
// AUTHOR: Larry.Minton / David Cunningham - created Oct 2, 2006
//***************************************************************************/
//

#pragma once

#include "CDotNetHostWnd.h"

#pragma managed

namespace MXS_dotNet
{
	class dotNetControl;

	public class mxsCDotNetHostWnd : public CDotNetHostWnd	{

	private:
		typedef CDotNetHostWnd inherited;

		dotNetControl* mDotNetControl;

		int mArgsCountHold;
		Value** mArgsHold;

	public:

		ref class delegate_proxy_type : public delegate_proxy_type_base
		{
		private:
			// the owning MXS dotNetControl
			dotNetControl *m_p_dotNetControl;
			
		public:
			delegate_proxy_type(mxsCDotNetHostWnd* pNativeTarget);
			virtual ~delegate_proxy_type();
			
			dotNetControl* get_dotNetControl();
			void set_dotNetControl(dotNetControl *_dotNetControl);
			static Value* get_MXS_container_for_object(System::Object ^ object);
		};
	
		msclr::delegate_map::internal::delegate_proxy_factory<mxsCDotNetHostWnd> m_delegate_map_proxy;

	public:

		mxsCDotNetHostWnd(dotNetControl *p_dotNetControl);
		virtual ~mxsCDotNetHostWnd();

		Value* GetMXSContainer();

		static Value* GetMXSContainerForObject(System::Object ^ object); //  returns dotNetControl if object already wrapped

		bool CreateControl(
			HWND hParent, 
			HFONT hFont,
			int id, 
			int x, 
			int y, 
			int w, 
			int h, 
			const MCHAR* pControlTypeString, 
			Value **keyparms, 
			int keyparm_count);

		// This method will be called when any event is raised on the user control
		virtual void ProcessEvent(System::String ^eventName, System::Object ^delegateArgs) ;

		dotNetControl* GetDotNetControl();
		void SetDotNetControl(dotNetControl* aControl);

		virtual delegate_proxy_type_base^ GetProxyType();

	protected:
		virtual void DoOnDestroy();
		virtual void DoRegisterControl();
		

		virtual void LoadCreateControlParams();
	};
}