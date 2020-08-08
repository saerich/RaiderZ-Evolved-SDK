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

#include "mxsCDotNetHostWnd.h"
#include "dotNetControl.h"

#pragma once
#pragma managed

using namespace MXS_dotNet;


mxsCDotNetHostWnd::mxsCDotNetHostWnd(dotNetControl *p_dotNetControl) : 
	CDotNetHostWnd(), 
	mDotNetControl(p_dotNetControl),
	mArgsCountHold(0),
	mArgsHold(NULL)
{
	m_delegate_map_proxy.get_proxy(this)->set_dotNetControl(p_dotNetControl);
}

mxsCDotNetHostWnd::~mxsCDotNetHostWnd()	{

	
}

Value* mxsCDotNetHostWnd::GetMXSContainer()
{
	return GetDotNetControl();
}

//  returns dotNetObject if object already wrapped
Value* mxsCDotNetHostWnd::GetMXSContainerForObject(System::Object ^ object)
{
	return delegate_proxy_type::get_MXS_container_for_object(object);
}


// This method will be called when any event is raised on the user control
void mxsCDotNetHostWnd::ProcessEvent(System::String ^eventName, System::Object ^delegateArgs)
{
	init_thread_locals();
	push_alloc_frame();
	Value**		arg_list;
	three_value_locals(eventName, handler, ehandler);

	try
	{
		array<System::Object^> ^ delegateArgsArray = safe_cast<array<System::Object^> ^>(delegateArgs);
		System::Windows::Forms::Control ^ sender = safe_cast<System::Windows::Forms::Control ^>(delegateArgsArray[0]);
		System::EventArgs ^ eventArgs = safe_cast<System::EventArgs ^>(delegateArgsArray[1]);

		vl.eventName = Name::intern(MNETStr::ToMSTR(eventName));
		
		dotNetControl *l_p_dotNetControl = GetDotNetControl();
		vl.handler = l_p_dotNetControl->get_event_handler(vl.eventName);
		// see if an handler exists for this event
		if ( vl.handler != &undefined)
		{
			// make the event handler argument list
			int cArgs = 0;
			vl.ehandler = vl.handler->eval_no_wrapper();
			if (is_maxscriptfunction(vl.ehandler))
				cArgs = ((MAXScriptFunction*)vl.ehandler)->parameter_count;
			value_local_array(arg_list, cArgs);
			for (int i = 0; i < cArgs; i++)
				arg_list[i] = &undefined;
			if (cArgs == 1)
				arg_list[0] = DotNetObjectWrapper::intern(eventArgs);
			if (cArgs > 1)
			{
				arg_list[0] = l_p_dotNetControl;
				arg_list[1] = DotNetObjectWrapper::intern(eventArgs);
			}
			// call the event handler
			l_p_dotNetControl->run_event_handler(l_p_dotNetControl->parent_rollout, vl.eventName, arg_list, cArgs);
			pop_value_local_array(arg_list);
		}
	}
	catch (MAXScriptException&)
	{
		pop_value_locals();
		pop_alloc_frame();
		throw;
	}
	catch (System::Exception ^ e)
	{
		pop_value_locals();
		pop_alloc_frame();
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
	pop_value_locals();
	pop_alloc_frame();
}

void mxsCDotNetHostWnd::LoadCreateControlParams()	{

	if(mArgsHold != NULL && mArgsCountHold > 0)	{
		Value* args[2];
		for (int i = 0; i < mArgsCountHold; i+=2)
		{
			args[0] = mArgsHold[i+1];
			args[1] = mArgsHold[i];
			set_property(args,2);
		}
	}	
}

bool mxsCDotNetHostWnd::CreateControl( 
	HWND hParent, 
	HFONT hFont,
	int id, 
	int x, 
	int y, 
	int w, 
	int h, 
	const MCHAR* pControlTypeString, 
	Value **keyparms, 
	int keyparm_count)
{

	
	mArgsCountHold = keyparm_count;
	mArgsHold = keyparms;

	System::Windows::Forms::Control^ control = CreateGenericControl(pControlTypeString);

	bool res = false;

	if(control != nullptr)	{
		res = DoCreateControl(
			hParent,
			hFont,
			id,
			x,
			y,
			w,
			h,
			control);
	}

	mArgsCountHold = 0;
	mArgsHold = NULL;

	return res;

}



void mxsCDotNetHostWnd::DoRegisterControl()	{
	// hook up to the max message pump. Really needed?
	MAXScript_interface->RegisterDlgWnd(GetSafeHwnd());
}

mxsCDotNetHostWnd::delegate_proxy_type::delegate_proxy_type(mxsCDotNetHostWnd* pNativeTarget) : 
		inherited::delegate_proxy_type_base(pNativeTarget), 
		m_p_dotNetControl(NULL)
		
{
	
}

mxsCDotNetHostWnd::delegate_proxy_type::~delegate_proxy_type()	{

}


dotNetControl* mxsCDotNetHostWnd::delegate_proxy_type::get_dotNetControl() { 
	return m_p_dotNetControl; 
}
void mxsCDotNetHostWnd::delegate_proxy_type::set_dotNetControl(dotNetControl *_dotNetControl) { 
	m_p_dotNetControl = _dotNetControl; 
}

Value* mxsCDotNetHostWnd::delegate_proxy_type::get_MXS_container_for_object(System::Object ^ object)
{
	if (m_pWrappedControls)
	{
		delegate_proxy_type ^ theProxy = safe_cast<delegate_proxy_type ^>(m_pWrappedControls[object]);
		if (theProxy)
			return theProxy->get_dotNetControl();
	}
	return NULL;
}



dotNetControl* mxsCDotNetHostWnd::GetDotNetControl()	{
	return mDotNetControl;
}

void mxsCDotNetHostWnd::SetDotNetControl(dotNetControl* aControl)	{
	mDotNetControl = aControl;
}

void mxsCDotNetHostWnd::DoOnDestroy()	{
	MAXScript_interface->UnRegisterDlgWnd(GetSafeHwnd());
}

CDotNetHostWnd::delegate_proxy_type_base^ mxsCDotNetHostWnd::GetProxyType()	{
	return m_delegate_map_proxy.get_proxy(this);	
}