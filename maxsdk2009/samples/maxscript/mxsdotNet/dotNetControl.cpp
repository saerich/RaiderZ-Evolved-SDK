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
// DESCRIPTION: dotNetControl.cpp  : MAXScript dotNetControl code
// AUTHOR: Larry.Minton - created Jan.1.2006
//***************************************************************************/
//

#include "DotNetObjectWrapper.h"
#include "mxsCDotNetHostWnd.h"
#include "dotNetControl.h"

#include "resource.h"

#include "defextfn.h"
#	include "namedefs.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

using namespace MXS_dotNet;

/* -------------------- dotNetControl  ------------------- */
// 
#pragma unmanaged

#pragma warning(disable:4835)
visible_class_instance (MXS_dotNet::dotNetControl, "dotNetControl")
#pragma warning(default:4835)

dotNetControl::dotNetControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
: RolloutControl(name, caption, keyparms, keyparm_count), m_pDotNetHostWnd(NULL)
{
	// dotNetControl <name> <class_type_string> [{<property_name>:<value>}]
	tag = class_tag(dotNetControl); 
}

dotNetControl::~dotNetControl()
{
	if (m_pDotNetHostWnd)
		delete m_pDotNetHostWnd;
}

void
dotNetControl::gc_trace()
{
	RolloutControl::gc_trace();
}

void
dotNetControl::sprin1(CharStream* s)
{
	if (m_typeName.isNull() && m_pDotNetHostWnd)
		m_typeName = m_pDotNetHostWnd->GetTypeName();
	s->printf(_M("dotNetControl:%s:%s"), name->to_string(), m_typeName);
}

void
dotNetControl::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	caption = caption->eval();

	MCHAR *controlTypeName = caption->to_string();

	parent_rollout = ro;
	control_ID = next_id();

	// compute bounding box, apply layout params
	layout_data pos;
	compute_layout(ro, &pos, current_y);

	ASSERT( !m_pDotNetHostWnd );
	
	Value**  evaled_keyparms = NULL;
	value_temp_array(evaled_keyparms, keyparm_count*2);
	for (int i = 0; i < keyparm_count*2; i++)
		evaled_keyparms[i] = keyparms[i]->eval();

	try
	{
		m_pDotNetHostWnd = new mxsCDotNetHostWnd(this);   // To host .NET controls
		BOOL res = m_pDotNetHostWnd->CreateControl(
			parent, 
			control_ID, 
			pos.left, 
			pos.top, 
			pos.width, 
			pos.height, 
			controlTypeName, 
			evaled_keyparms, 
			keyparm_count);
	}
	catch (...) 
	{
		pop_value_temp_array(evaled_keyparms);
		throw;
	}
	pop_value_temp_array(evaled_keyparms);
}

BOOL
dotNetControl::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (message == WM_CLOSE && m_pDotNetHostWnd)
	{
		CDotNetHostWnd *l_pDotNetHostWnd = m_pDotNetHostWnd;
		m_pDotNetHostWnd->DestroyWindow();
		m_pDotNetHostWnd = NULL;
		delete l_pDotNetHostWnd;

	}
	return FALSE;
}

void
dotNetControl::set_enable()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (parent_rollout != NULL && parent_rollout->page != NULL && m_pDotNetHostWnd)
	{
		// set ActiveX control enable
		m_pDotNetHostWnd->EnableWindow(enabled);
		::InvalidateRect(parent_rollout->page, NULL, TRUE);
	}
}

BOOL
dotNetControl::set_focus()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (parent_rollout && parent_rollout->page && m_pDotNetHostWnd)
	{
		// set ActiveX control enable
		return m_pDotNetHostWnd->SetFocus() ? TRUE : FALSE;
	}
	return FALSE;
}

Value*
dotNetControl::get_property(Value** arg_list, int count)
{
	// getProperty <dotNetControl> <prop name> [asDotNetObject:<bool>]
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	Value* prop = arg_list[0];
	Value* result = NULL;

	if (prop == n_enabled)
		return bool_result(enabled);
	else if (m_pDotNetHostWnd && (result = m_pDotNetHostWnd->get_property(arg_list, count)) != NULL)
		return_protected(result)
	else
	return RolloutControl::get_property(arg_list, count);
}

Value*
dotNetControl::set_property(Value** arg_list, int count)
{
	// setProperty <dotNetControl> <prop name> <value>
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Value* val = arg_list[0];
	// Value* prop = arg_list[1];
	one_value_local(result);
	if (m_pDotNetHostWnd && (vl.result = m_pDotNetHostWnd->set_property(arg_list, count)) != NULL)
		return_value(vl.result)
	else
		return RolloutControl::set_property(arg_list, count);
}

Value*
dotNetControl::get_props_vf(Value** arg_list, int count)
{
	// getPropNames <dotNetControl> [showStaticOnly:<bool>] [declaredOnTypeOnly:<bool>] -- returns array of prop names of wrapped obj

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	bool showStaticOnly = key_arg_or_default(showStaticOnly,&false_value)->to_bool() != FALSE;
	bool declaredOnTypeOnly = key_arg_or_default(declaredOnTypeOnly,&false_value)->to_bool() != FALSE;

	one_typed_value_local(Array* result);
	vl.result = new Array (0);
	if (m_pDotNetHostWnd)
		m_pDotNetHostWnd->get_property_names(vl.result, showStaticOnly, declaredOnTypeOnly);
	return_value(vl.result);
}

Value*
dotNetControl::show_props_vf(Value** arg_list, int count)
{
	// showProperties <dotNetControl> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [showMethods:<bool>] [showAttributes:<bool>] [declaredOnTypeOnly:<bool>]
	CharStream*	out;
	MCHAR*		prop_pat = _M("*");

	if (!m_pDotNetHostWnd)
		return &false_value;

	if (count >= 1 && arg_list[0] != &keyarg_marker)	
		prop_pat = arg_list[0]->to_string();	

	if ((out = (CharStream*)key_arg(to)) != (CharStream*)&unsupplied)
	{
		if (out == (CharStream*)&undefined)   // to:undefined -> no output, return true or false if prop found
			out = NULL;
		else if (!is_charstream(out))
			throw TypeError (GetString(IDS_SHOWPROPERTIES_NEED_STREAM), out);
	}
	else
		out = thread_local(current_stdout);

	bool showStaticOnly = key_arg_or_default(showStaticOnly,&false_value)->to_bool() != FALSE;
	bool showMethods = key_arg_or_default(showMethods,&false_value)->to_bool() != FALSE;
	bool showAttributes = key_arg_or_default(showAttributes,&false_value)->to_bool() != FALSE;
	bool declaredOnTypeOnly = key_arg_or_default(declaredOnTypeOnly,&false_value)->to_bool() != FALSE;

	// look in properties, then fields
	bool res = m_pDotNetHostWnd->show_properties(prop_pat, out, showStaticOnly, showMethods, showAttributes, declaredOnTypeOnly);
	if (out || !res) // not searching for prop match, or prop match failed
		res |= m_pDotNetHostWnd->show_fields(prop_pat, out, showStaticOnly, showAttributes, declaredOnTypeOnly);
	return bool_result(res);
}

Value*
dotNetControl::show_methods_vf(Value** arg_list, int count)
{
	// showMethods <dotNetControl> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [showSpecial:<bool>] [showAttributes:<bool>] [declaredOnTypeOnly:<bool>]
	CharStream*	out;
	MCHAR*		prop_pat = _M("*");

	if (!m_pDotNetHostWnd)
		return &false_value;

	if (count >= 1 && arg_list[0] != &keyarg_marker)	
		prop_pat = arg_list[0]->to_string();	

	if ((out = (CharStream*)key_arg(to)) != (CharStream*)&unsupplied)
	{
		if (out == (CharStream*)&undefined)   // to:undefined -> no output, return true or false if prop found
			out = NULL;
		else if (!is_charstream(out))
			throw TypeError (GetString(IDS_SHOWMETHODS_NEED_STREAM), out);
	}
	else
		out = thread_local(current_stdout);

	bool showStaticOnly = key_arg_or_default(showStaticOnly,&false_value)->to_bool() != FALSE;
	bool showSpecial = key_arg_or_default(showSpecial,&false_value)->to_bool() != FALSE;
	bool showAttributes = key_arg_or_default(showAttributes,&false_value)->to_bool() != FALSE;
	bool declaredOnTypeOnly = key_arg_or_default(declaredOnTypeOnly,&false_value)->to_bool() != FALSE;

	bool res = m_pDotNetHostWnd->show_methods(prop_pat, out, showStaticOnly, showSpecial, showAttributes, declaredOnTypeOnly);
	return bool_result(res);
}

Value*
dotNetControl::show_events_vf(Value** arg_list, int count)
{
	// showEvents <dotNetControl> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [declaredOnTypeOnly:<bool>]
	CharStream*	out;
	MCHAR*		prop_pat = _M("*");

	if (!m_pDotNetHostWnd)
		return &false_value;

	if (count >= 1 && arg_list[0] != &keyarg_marker)	
		prop_pat = arg_list[0]->to_string();	

	if ((out = (CharStream*)key_arg(to)) != (CharStream*)&unsupplied)
	{
		if (out == (CharStream*)&undefined)   // to:undefined -> no output, return true or false if prop found
			out = NULL;
		else if (!is_charstream(out))
			throw TypeError (GetString(IDS_SHOWEVENTS_NEED_STREAM), out);
	}
	else
		out = thread_local(current_stdout);

	bool showStaticOnly = key_arg_or_default(showStaticOnly,&false_value)->to_bool() != FALSE;
	bool declaredOnTypeOnly = key_arg_or_default(declaredOnTypeOnly,&false_value)->to_bool() != FALSE;

	bool res = m_pDotNetHostWnd->show_events(prop_pat, out, showStaticOnly, declaredOnTypeOnly);
	return bool_result(res);
}

/*
// see comments in utils.cpp
Value*
dotNetControl::show_interfaces_vf(Value** arg_list, int count)
{
	// showInterfaces <dotNetControl> ["prop_pat"] [to:<stream>]
	CharStream*	out;
	MCHAR*		prop_pat = _M("*");

	if (!m_pDotNetHostWnd)
		return &false_value;

	if (count >= 1 && arg_list[0] != &keyarg_marker)	
		prop_pat = arg_list[0]->to_string();	

	if ((out = (CharStream*)key_arg(to)) != (CharStream*)&unsupplied)
	{
		if (out == (CharStream*)&undefined)   // to:undefined -> no output, return true or false if prop found
			out = NULL;
		else if (!is_charstream(out))
			throw TypeError (GetString(IDS_SHOWMETHODS_NEED_STREAM), out);
	}
	else
		out = thread_local(current_stdout);

	bool res = m_pDotNetHostWnd->show_interfaces(prop_pat, out);
	return bool_result(res);
}

Value*	
dotNetControl::get_interfaces_vf(Value** arg_list, int count)
{
	// getInterfaces <dotNetControl> ["prop_pat"] [to:<stream>]
	check_arg_count(getInterfaces, 1, count+1);
	one_typed_value_local(Array* result);
	vl.result = new Array (0);
	if (m_pDotNetHostWnd)
		m_pDotNetHostWnd->get_interfaces(vl.result);
	return_value(vl.result);
}
*/
