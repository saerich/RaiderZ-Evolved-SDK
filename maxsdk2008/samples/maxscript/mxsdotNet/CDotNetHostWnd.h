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

#include "DotNetObjectWrapper.h"

/* -------------------- CDotNetHostWnd  ------------------- */
// 
#pragma managed



namespace MXS_dotNet
{

	public class CDotNetHostWnd : public CWnd, public DotNetObjectWrapper
	{

	private:
		typedef CWnd inherited;		

	public:

		ref class delegate_proxy_type_base abstract
		{
		protected:
			CDotNetHostWnd* m_p_native_target;
			// the control 
			System::Windows::Forms::Control ^ m_pControl;
			// existing wrapped controls
			static System::Collections::Hashtable ^ m_pWrappedControls = nullptr;
			// event/delegate pairs for removing the delegates from the events when done with the object
			System::Collections::Generic::List<event_delegate_pair^> ^ m_pEventDelegatePairs;
			// flag that we are in the process of running the ControlResizeMethod method
			bool m_in_ResizeMethod;

		public:
			delegate_proxy_type_base(CDotNetHostWnd* pNativeTarget);
			virtual ~delegate_proxy_type_base();
			void detach();

			// this method is called by a dynamic method attached to a delegate, which is then attached to an event
			// eventName is the event name, delegateArgs is an array containing the parameters passed to the delegate method
			// (sender, EventArgs)
			void ProcessEvent(System::String ^eventName, System::Object ^delegateArgs);
			void ControlSetFocusMethod(System::Object ^ sender, System::EventArgs ^e);
			void ControlResizeMethod(System::Object ^ sender, System::EventArgs ^e);

			System::Windows::Forms::Control ^ get_control();
			void set_control(System::Windows::Forms::Control ^ control);
			void store_EventDelegatePair(event_delegate_pair ^ pEventDelegatePair);

			void SetFocusHandler(System::EventHandler^ aHandler);
			System::EventHandler^ GetFocusHandler();

			void SetResizeHandler(System::EventHandler^ aHandler);
			System::EventHandler^ GetResizeHandler();

		private:
			System::EventHandler^ m_pSetFocusHandler;
			System::EventHandler^ m_pResizeHandler;
		};


		CWinFormsControl<System::Windows::Forms::Control> m_CWinFormsControl;

		CDotNetHostWnd();

		virtual ~CDotNetHostWnd();

		

		void CreateEventHandlerDelegates();
		
		void CreateSetFocusEventHandler();
		void CreateResizeEventHandler();

		// DotNetObjectWrapper
		System::Object ^ GetObject();
		void StoreEventDelegatePair(event_delegate_pair ^ pEventDelegatePair);

		afx_msg void OnDestroy();

		// This method will be called when any event is raised on the user control
		virtual void ProcessEvent(System::String ^eventName, System::Object ^delegateArgs) = 0;

		bool SetFocus();

		static void Init();

		static System::Windows::Forms::Control^ CreateGenericControl(const MCHAR* aControlTypeString);

		virtual delegate_proxy_type_base^ GetProxyType() = 0;

	protected:
		virtual void DoOnDestroy() = 0;
		virtual void DoRegisterControl() = 0;
		

		bool DoCreateControl(
			HWND hParent, 
			int id, 
			int x, 
			int y, 
			int w, 
			int h, 
			System::Windows::Forms::Control^ l_pControl);

		

		virtual void LoadCreateControlParams() = 0;

	private:
		DECLARE_MESSAGE_MAP()
	};
}
