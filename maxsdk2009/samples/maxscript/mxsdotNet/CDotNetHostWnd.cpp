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
//***********

#include "CDotNetHostWnd.h"
#include "utils.h"


using namespace MXS_dotNet;

/* -------------------- CDotNetHostWnd  ------------------- */
// 
#pragma managed

// first some MFC stuff...
BEGIN_MESSAGE_MAP(CDotNetHostWnd, CWnd)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


CDotNetHostWnd::CDotNetHostWnd() : inherited()
{
	
}

CDotNetHostWnd::~CDotNetHostWnd()
{
	// Safety net -- if we delete the C++ object and there's still an HWND on it,
	// we need to call this so our own OnDestroy() is called.
	try
	{
		DestroyWindow(); 
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}



void CDotNetHostWnd::Init()
{
	// force a load of the Windows::Form and System::Drawing Assemblies
	System::Reflection::Assembly ^windowsFormAssembly = System::Windows::Forms::MonthCalendar::typeid->Assembly;
	System::Reflection::Assembly ^drawingAssembly = System::Drawing::Color::typeid->Assembly;
}

afx_msg void CDotNetHostWnd::OnDestroy()
{
	// Do any cleanup we need to here, if any.
	try
	{
		DoOnDestroy();
		m_CWinFormsControl.DestroyWindow();
		CWnd::OnDestroy();
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}

void CDotNetHostWnd::CreateResizeEventHandler()
{
	try
	{
		delegate_proxy_type_base ^l_pProxy = GetProxyType();
		System::Windows::Forms::Control ^ l_pControl = l_pProxy->get_control();
		System::Type ^ l_pControlType = l_pControl->GetType();
		System::Reflection::EventInfo ^l_pResizeEvent = l_pControlType->GetEvent("Resize");
		if (l_pResizeEvent)
		{
			l_pProxy->SetResizeHandler(gcnew System::EventHandler( l_pProxy, &delegate_proxy_type_base::ControlResizeMethod ));
			l_pResizeEvent->AddEventHandler(l_pControl, l_pProxy->GetResizeHandler());
		}
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}

void CDotNetHostWnd::CreateSetFocusEventHandler()
{
	try
	{
		delegate_proxy_type_base ^l_pProxy = GetProxyType();
		System::Windows::Forms::Control ^ l_pControl = l_pProxy->get_control();
		System::Type ^ l_pControlType = l_pControl->GetType();
		System::Reflection::EventInfo ^l_pGotFocusEvent = l_pControlType->GetEvent("GotFocus");
		if (l_pGotFocusEvent)
		{
			l_pProxy->SetFocusHandler(gcnew System::EventHandler( l_pProxy, &delegate_proxy_type_base::ControlSetFocusMethod ));
			l_pGotFocusEvent->AddEventHandler(l_pControl, l_pProxy->GetFocusHandler());
		}
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}

void CDotNetHostWnd::CreateEventHandlerDelegates()
{
	using namespace System;
	using namespace System::Reflection;
	using namespace System::Reflection::Emit;
	// Event handler methods are generated at run time using lightweight dynamic methods and Reflection.Emit. 
	// get the method that the delegate's dynamic method will be calling

	try
	{
		Type ^ proxyType = GetProxyType()->GetType();
		MethodInfo ^mi = proxyType->GetMethod("ProcessEvent");
		DotNetObjectWrapper::CreateEventHandlerDelegates(GetProxyType(), mi);
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}

System::Object ^ CDotNetHostWnd::GetObject()
{
	return GetProxyType()->get_control();
}

void CDotNetHostWnd::StoreEventDelegatePair(event_delegate_pair ^ pEventDelegatePair)
{
	return GetProxyType()->store_EventDelegatePair(pEventDelegatePair);
}

bool CDotNetHostWnd::SetFocus()
{
	System::Windows::Forms::Control ^ pControl = GetProxyType()->get_control();
	if (pControl)
		return pControl->Focus();
	return FALSE;
}

System::Windows::Forms::Control^ CDotNetHostWnd::CreateGenericControl(const MCHAR* aControlTypeString)	{

	using namespace System;
	using namespace System::Reflection;
	using namespace System::Reflection::Emit;

	System::Windows::Forms::Control ^l_pControl = nullptr;

	// look for control type using fully qualified class name (includes assembly)
	Type ^controlType = ResolveTypeFromName(aControlTypeString);
	if (controlType)
	{
		ConstructorInfo ^ l_pConstructorInfo = controlType->GetConstructor(Type::EmptyTypes);
		if (l_pConstructorInfo)	{
			l_pControl = (System::Windows::Forms::Control^)l_pConstructorInfo->Invoke(Type::EmptyTypes);
		}
	}
	return l_pControl;
}

bool CDotNetHostWnd::DoCreateControl( HWND hParent, int id, int x, int y, int w, int h, 
									 System::Windows::Forms::Control^ l_pControl)
{
	// Create and add the control to the user control.
	using namespace System;
	using namespace System::Reflection;
	using namespace System::Reflection::Emit;

	try
	{
		// Create a dummy AFX class name (doesn't really have to be static here, MFC will
		// return the same class name for the same parameters, but it does make it explicit
		// this way)
		static CString lcsClassName = AfxRegisterWndClass(NULL);

		CWnd* pCWnd = CWnd::FromHandle(hParent);

		if( !Create(lcsClassName, _T(""), WS_CHILD|WS_VISIBLE, CRect(x, y, x+w, y+h), 
			pCWnd, id) )
		{
			return FALSE;
		}

		if (l_pControl)
		{
			DoRegisterControl();
			GetProxyType()->set_control(l_pControl);
			CreateEventHandlerDelegates();
			CreateSetFocusEventHandler();

			LoadCreateControlParams();

			RECT rect; 
			rect.left = 0; rect.top = 0; rect.right = w; rect.bottom = h;
			bool res = m_CWinFormsControl.CreateManagedControl(
				l_pControl, 
				WS_VISIBLE | WS_CHILD | WS_TABSTOP, 
				rect, 
				this, 
				id) == TRUE;

			if (res)	{
				CreateResizeEventHandler();
			}
			return res;
		}

		return FALSE;
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}


CDotNetHostWnd::delegate_proxy_type_base::delegate_proxy_type_base(CDotNetHostWnd* pNativeTarget) :
	m_p_native_target(pNativeTarget),
	m_in_ResizeMethod(false) 
{	
	if(m_pWrappedControls == nullptr)	{
		m_pWrappedControls = gcnew System::Collections::Hashtable(53);
	}
}

CDotNetHostWnd::delegate_proxy_type_base::~delegate_proxy_type_base()	{

}


System::Windows::Forms::Control^ CDotNetHostWnd::delegate_proxy_type_base::get_control() { 
	return m_pControl; 
}

void CDotNetHostWnd::delegate_proxy_type_base::set_control(System::Windows::Forms::Control ^ control)
{
	try
	{
		m_pControl = control;
		m_pWrappedControls->Add(control, this);
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}

void CDotNetHostWnd::delegate_proxy_type_base::store_EventDelegatePair(event_delegate_pair ^ pEventDelegatePair)
{
	try
	{
		if (!m_pEventDelegatePairs)
			m_pEventDelegatePairs = gcnew System::Collections::Generic::List<event_delegate_pair^>();
		m_pEventDelegatePairs->Add(pEventDelegatePair);
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}

void CDotNetHostWnd::delegate_proxy_type_base::SetFocusHandler(System::EventHandler^ aHandler)	{
	m_pSetFocusHandler = aHandler;
}

System::EventHandler^ CDotNetHostWnd::delegate_proxy_type_base::GetFocusHandler()	{	
	return m_pSetFocusHandler;
}

void CDotNetHostWnd::delegate_proxy_type_base::SetResizeHandler(System::EventHandler^ aHandler)	{
	m_pResizeHandler = aHandler;
}
System::EventHandler^ CDotNetHostWnd::delegate_proxy_type_base::GetResizeHandler() 	{
	return m_pResizeHandler;
}

void CDotNetHostWnd::delegate_proxy_type_base::ProcessEvent(System::String ^eventName, System::Object ^delegateArgs)
{
	// hack! If m_in_ResizeMethod is true, we are processing the ControlResizeMethod method, that 
	// does 2 resizes of the control in order for it to update to the new CWnd size. We don't want to
	// call the scripted event handlers due to events fired by those resizings....
	if (m_in_ResizeMethod)
		return;

	//	System::Diagnostics::Debug::WriteLine( "Event: " + eventName );
	if (!m_p_native_target && eventName == "HandleDestroyed") 
		return; // happens after detach

	//	MSTR xx = MXS_dotNet::MNETStr::ToMSTR(eventName);
	//	xx.Append(_M("\n"));
	//	CString xxx(xx);
	//	OutputDebugString(xxx);

	DbgAssert(m_p_native_target);
	if(m_p_native_target)
		m_p_native_target->ProcessEvent(eventName, delegateArgs);
}

void CDotNetHostWnd::delegate_proxy_type_base::ControlSetFocusMethod(System::Object ^ sender, System::EventArgs ^e)
{
	//	System::Diagnostics::Debug::WriteLine( "ControlSetFocusMethod" );
	DisableAccelerators();
}

void CDotNetHostWnd::delegate_proxy_type_base::ControlResizeMethod(System::Object ^ sender, System::EventArgs ^e)
{
	//	System::Diagnostics::Debug::WriteLine( "ControlResizeMethod" );
	if (!m_in_ResizeMethod && m_p_native_target && m_p_native_target->GetParent())
	{
		m_in_ResizeMethod = true;
		CWnd *hCWndParent = m_p_native_target->GetParent();
		CRect rect_host;
		m_p_native_target->GetWindowRect(&rect_host);
		hCWndParent->ScreenToClient(&rect_host);
		System::Drawing::Size control_size = m_pControl->Size;
		rect_host.bottom = rect_host.top + control_size.Height;
		rect_host.right = rect_host.left + control_size.Width;
		m_p_native_target->MoveWindow(rect_host);

		// hack alert! The HWND of the control appears take its size when the size of
		// the control changes. The HWNDs size is a combination of the control's size and
		// the parent's window's size. But we are changing the parent's window size here,
		// and we need to tell the control to update it's HWND size. The only way I've found
		// so far is to change and reset the size. We use the m_in_ResizeMethod variable as an
		// interlock so we don't go recursive. We also check m_in_ResizeMethod before calling 
		// scripted event handlers so that events fired from these resizings don't show.
		int width_orig = m_pControl->Width;
		m_pControl->Width = width_orig + 1;
		m_pControl->Width = width_orig;
		m_in_ResizeMethod = false;
	}
}

void CDotNetHostWnd::delegate_proxy_type_base::detach() 
{ 
	using namespace System;
	using namespace System::Reflection;

	try
	{
		// HandleDestroyed event happens after detach. Do a fake one now before detaching.
		ProcessEvent("HandleDestroyed", gcnew array<System::Object^>{m_pControl, System::EventArgs::Empty});
		m_p_native_target = NULL; 
		if (m_pControl)
		{
			Type ^ targetType = m_pControl->GetType();

			// unhook all the event handlers
			if (m_pEventDelegatePairs)
			{
				System::Collections::IEnumerator^ myEnum = m_pEventDelegatePairs->GetEnumerator();
				while ( myEnum->MoveNext() )
				{
					event_delegate_pair^ pEventDelegatePair = safe_cast<event_delegate_pair^>(myEnum->Current);
					EventInfo ^l_pEvent = targetType->GetEvent(pEventDelegatePair->m_pEventName);
					if (l_pEvent)
						l_pEvent->RemoveEventHandler(m_pControl, pEventDelegatePair->m_pEventDelegate);
				}
				m_pEventDelegatePairs = nullptr;
			}
			// unhook GotFocus handler
			System::Reflection::EventInfo ^l_pGotFocusEvent = targetType->GetEvent("GotFocus");
			if (l_pGotFocusEvent && GetFocusHandler())	{
				l_pGotFocusEvent->RemoveEventHandler(m_pControl, GetFocusHandler());
			}
			SetFocusHandler(nullptr);
			// unhook Resize handler
			System::Reflection::EventInfo ^l_pResizeEvent = targetType->GetEvent("Resize");
			if (l_pResizeEvent && GetResizeHandler())
				l_pResizeEvent->RemoveEventHandler(m_pControl, GetResizeHandler());
			SetResizeHandler(nullptr);

			m_pWrappedControls->Remove(m_pControl);
			m_pControl = nullptr;
		}
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}