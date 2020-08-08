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
// DESCRIPTION: mxsdotNet.cpp  : MAXScript mxsdotNet scripted methods code
// AUTHOR: Larry.Minton - created Jan.1.2006
//***************************************************************************/
//

#include "dotNetControl.h"
#include "maxDotNetControl.h"
#include "bitmap.h"
#include "utils.h"
#include "resource.h"
#include <icolorman.h>

#include "assetmanagement\AssetUser.h"
#include "AssetManagement\iassetmanager.h"
using namespace MaxSDK::AssetManagement;

#include <maxscript\macros\define_instantiation_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
#	include "dotnet_wraps.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

using namespace MXS_dotNet;

/* -------------------- dotNet struct methods  ------------------- */

Value*
dotNet_getType_cf(Value** arg_list, int count)
{
	// getType {<dotNetObject> | <dotNetControl> | <dotNetClass> | <type string>}
	check_arg_count(dotNet.getType, 1, count);
	Value* val = arg_list[0];
	if (is_dotNetBase(val) || is_dotNetControl(val))
	{
		DotNetObjectWrapper *wrapper = NULL;
		if (is_dotNetBase(val)) 
			wrapper = ((dotNetBase*)val)->GetDotNetObjectWrapper();
		else 
			wrapper = ((dotNetControl*)val)->GetDotNetObjectWrapper();
		if (wrapper)
			return DotNetObjectWrapper::WrapType(wrapper);
		return &undefined;
	}
	else
	{
		MCHAR *type = val->to_string();
		return DotNetObjectWrapper::WrapType(type);
	}
}

Value*
showConstructors_cf(Value** arg_list, int count)
{
	// showConstructors {<dotNetObject> | <dotNetControl> | <dotNetClass> | <type string>} [to:<stream>]
	check_arg_count_with_keys(dotNet.showConstructors, 1, count);
	Value* val = arg_list[0];
	DotNetObjectWrapper *wrapper = NULL;
	if (is_dotNetBase(val)) 
		wrapper = ((dotNetBase*)val)->GetDotNetObjectWrapper();
	else if (is_dotNetControl(val))
		wrapper = ((dotNetControl*)val)->GetDotNetObjectWrapper();
	else
	{
		MCHAR *type = val->to_string();
		val = DotNetObjectWrapper::WrapType(type);
		if (val == &undefined)
			return &false_value;
		wrapper = ((dotNetObject*)val)->GetDotNetObjectWrapper();
	}
	if (!wrapper)
		return &false_value;

	CharStream*	out;
	if ((out = (CharStream*)key_arg(to)) != (CharStream*)&unsupplied)
	{
		if (out == (CharStream*)&undefined)   // to:undefined -> no output, return true or false if prop found
			out = NULL;
		else if (!is_charstream(out))
			throw TypeError (GetString(IDS_SHOWPROPERTIES_NEED_STREAM), out);
	}
	else
		out = thread_local(current_stdout);
	bool res = wrapper->show_constructors(out);
	return bool_result(res);
}

/* 
// see comments in utils.cpp
Value*
showAttributes_cf(Value** arg_list, int count)
{
	// showAttributes {<dotNetObject> | <dotNetControl> | <dotNetClass> | <type string>} ["prop_pat"] [to:<stream>] [declaredOnTypeOnly:<bool>]
	int countNoKeys = count_with_keys();
	if (countNoKeys < 1 || countNoKeys > 2)
		check_arg_count_with_keys(dotNet.showAttributes, 1, count);
	Value* val = arg_list[0];
	DotNetObjectWrapper *wrapper = NULL;
	if (is_dotNetBase(val)) 
		wrapper = ((dotNetBase*)val)->GetDotNetObjectWrapper();
	else if (is_dotNetControl(val))
		wrapper = ((dotNetControl*)val)->GetDotNetObjectWrapper();
	else
	{
		MCHAR *type = val->to_string();
		val = DotNetObjectWrapper::WrapType(type);
		if (val == &undefined)
			return &false_value;
		wrapper = ((dotNetObject*)val)->m_pDotNetObject;
	}
	if (!wrapper)
		return &false_value;

	MCHAR*		prop_pat = _M("*");
	if (countNoKeys == 2)	
		prop_pat = arg_list[1]->to_string();	

	CharStream*	out;
	if ((out = (CharStream*)key_arg(to)) != (CharStream*)&unsupplied)
	{
		if (out == (CharStream*)&undefined)   // to:undefined -> no output, return true or false if prop found
			out = NULL;
		else if (!is_charstream(out))
			throw TypeError (GetString(IDS_SHOWPROPERTIES_NEED_STREAM), out);
	}
	else
		out = thread_local(current_stdout);

	bool declaredOnTypeOnly = key_arg_or_default(declaredOnTypeOnly,&false_value)->to_bool() != FALSE;

	bool res = wrapper->show_attributes(prop_pat, out, declaredOnTypeOnly);
	return bool_result(res);
}

Value*
getAttributes_cf(Value** arg_list, int count)
{
	// getAttributes {<dotNetObject> | <dotNetControl> | <dotNetClass> | <type string>}
	check_arg_count_with_keys(dotNet.getAttributes, 1, count);
	Value* val = arg_list[0];
	one_typed_value_local(Array* result);
	vl.result = new Array (0);

	DotNetObjectWrapper *wrapper = NULL;
	if (is_dotNetBase(val)) 
		wrapper = ((dotNetBase*)val)->GetDotNetObjectWrapper();
	else if (is_dotNetControl(val))
		wrapper = ((dotNetControl*)val)->GetDotNetObjectWrapper();
	else
	{
		MCHAR *type = val->to_string();
		val = DotNetObjectWrapper::WrapType(type);
		if (val == &undefined)
			return_value(vl.result);
		wrapper = ((dotNetObject*)val)->m_pDotNetObject;
	}
	if (!wrapper)
		return_value(vl.result);

	wrapper->get_attributes(vl.result);
	return_value(vl.result);
}
*/

Value*
dotNet_add_event_handler_cf(Value** arg_list, int count)
{
	// addEventHandler {<dotNetObject> | <dotNetClass>} <event> <fn>
	check_arg_count(dotNet.addEventHandler, 3, count);
	Value* val = arg_list[0];
	two_value_locals(fn,eventName);
	MCHAR *eventName = arg_list[1]->to_string();
	vl.eventName = Name::intern(eventName);
	vl.fn = arg_list[2];
	if (!is_function(vl.fn))
		throw RuntimeError (_M("dotNet event handler must be a functions, got: "), vl.fn);
	MSPlugin *currentPlugin = thread_local(current_plugin); // LAM - 10/19/02 - defect 291499
	if (currentPlugin)
		vl.fn = new PluginMethod(currentPlugin, vl.fn);
	Struct *currentStruct = thread_local(current_struct);  // LAM - defect 468259
	if (currentStruct)
		vl.fn = new StructMethod(currentStruct, vl.fn);
	vl.fn = heap_ptr(vl.fn);

	if (is_dotNetBase(val)) 
		((dotNetBase*)val)->add_event_handler(vl.eventName, vl.fn);
	else
		throw ConversionError(val, _M("dotNetObject"));
	pop_value_locals();
	return &ok;
}

Value*
dotNet_remove_event_handler_cf(Value** arg_list, int count)
{
	// removeEventHandler {<dotNetObject> | <dotNetClass>} <event> <fn>
	check_arg_count(dotNet.removeEventHandler, 3, count);
	Value* val = arg_list[0];
	two_value_locals(fn,eventName);
	MCHAR *eventName = arg_list[1]->to_string();
	vl.eventName = Name::intern(eventName);
	vl.fn = arg_list[2];
	if (!is_function(vl.fn))
		throw RuntimeError (_M("dotNet event handler must be a functions, got: "), vl.fn);
	MSPlugin *currentPlugin = thread_local(current_plugin); // LAM - 10/19/02 - defect 291499
	if (currentPlugin)
		vl.fn = new PluginMethod(currentPlugin, vl.fn);
	Struct *currentStruct = thread_local(current_struct);  // LAM - defect 468259
	if (currentStruct)
		vl.fn = new StructMethod(currentStruct, vl.fn);
	vl.fn = heap_ptr(vl.fn);

	if (is_dotNetBase(val)) 
		((dotNetBase*)val)->remove_event_handler(vl.eventName, vl.fn);
	else
		throw ConversionError(val, _M("dotNetObject"));
	pop_value_locals();
	return &ok;
}

Value*
dotNet_remove_event_handlers_cf(Value** arg_list, int count)
{
	// removeEventHandlers {<dotNetObject> | <dotNetClass>} <event>
	check_arg_count(dotNet.removeEventHandlers, 2, count);
	Value* val = arg_list[0];
	one_value_local(eventName);
	MCHAR *eventName = arg_list[1]->to_string();
	vl.eventName = Name::intern(eventName);

	if (is_dotNetBase(val)) 
		((dotNetBase*)val)->remove_all_event_handlers(vl.eventName);
	else
		throw ConversionError(val, _M("dotNetObject"));
	pop_value_locals();
	return &ok;
}

Value*
dotNet_remove_all_event_handlers_cf(Value** arg_list, int count)
{
	// removeAllEventHandlers {<dotNetObject> | <dotNetClass>}
	check_arg_count(dotNet.removeAllEventHandlers, 1, count);
	Value* val = arg_list[0];

	if (is_dotNetBase(val)) 
		((dotNetBase*)val)->remove_all_event_handlers();
	else
		throw ConversionError(val, _M("dotNetObject"));
	return &ok;
}


Value*
dotNet_set_lifetime_control_cf(Value** arg_list, int count)
{
	// setLifetimeControl {<dotNetObject> | <dotNetClass>} {#mxs | #dotnet} 
	check_arg_count(dotNet.setLifetimeControl, 2, count);
	Value* val = arg_list[0];

	bool lifetime_controlled_by_dotnet;
	if (arg_list[1] == n_mxs)
		lifetime_controlled_by_dotnet = false;
	else if (arg_list[1] == n_dotnet)
		lifetime_controlled_by_dotnet = true;
	else
		throw RuntimeError(_M("Expected #mxs or #dotnet, got:"), arg_list[1]);

	if (is_dotNetBase(val)) 
		((dotNetBase*)val)->set_lifetime_controlled_by_dotnet(lifetime_controlled_by_dotnet);
	else
		throw ConversionError(val, _M("dotNetObject"));
	return &ok;
}

Value*
dotNet_combineEnums_cf(Value** arg_list, int count)
{
	// combineEnums {<dotNetObject> | <number>}+
	return CombineEnums(arg_list,count);
}

Value*
dotNet_compareEnums_cf(Value** arg_list, int count)
{
	// CompareEnums {<dotNetObject> | <number>} {<dotNetObject> | <number>} 
	check_arg_count(dotNet.compareEnums, 2, count);
	return bool_result(CompareEnums(arg_list,count));
}

Value*
dotNet_loadAssembly_cf(Value** arg_list, int count)
{
	// loadAssembly <assembly_dll_string> 
	check_arg_count(dotNet.loadAssembly, 1, count);
	MSTR  fileName = arg_list[0]->to_filename();
	AssetId assetId;
	if (IAssetManager::GetInstance()->StringToAssetId(fileName, assetId))
		fileName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();
	return LoadAssembly(fileName);
}

Value*
dotNet_Value_To_DotNetObject_cf(Value** arg_list, int count)
{
	// ValueToDotNetObject <value> { <dotNetClass> | <dotNetObject> }
	check_arg_count(dotNet.ValueToDotNetObject, 2, count);
	dotNetBase *type = (dotNetBase*)arg_list[1];
	if (is_dotNetBase(type)) 
	{
		return_protected(MXSValueToDotNetObjectValue(arg_list[0], type));
	}
	else
		throw ConversionError(type, _M("dotNetClass"));
}



/* --------------------- plug-in init --------------------------------- */
// this is called by the dlx initializer, register the global var here

void DotNetControl_init()
{
	install_rollout_control(Name::intern(_M("dotNetControl")), dotNetControl::create);
#include <maxscript\macros\define_implementations.h>
#	include "namedefs.h"
	CDotNetHostWnd::Init();
	DotNetObjectManaged::Init();
	DotNetClassManaged::Init();

	WNDCLASS  wc;

	wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.hInstance     = TheManager->AppInst();
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	//	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.hbrBackground = GetCustSysColorBrush(COLOR_BTNFACE);
	wc.lpszMenuName  = NULL;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.lpfnWndProc   = dotNetControlWndProc;
	wc.lpszClassName = DOT_NET_CLASS;

	RegisterClass(&wc);
}


