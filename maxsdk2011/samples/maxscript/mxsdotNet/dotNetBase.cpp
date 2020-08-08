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
// DESCRIPTION: dotNetBase.cpp  : MAXScript dotNetBase code
// AUTHOR: Larry.Minton - created Jan.1.2006
//***************************************************************************/
//

#include "dotNetControl.h"
#include "DotNetHashTables.h"
#include "resource.h"

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

using namespace MXS_dotNet;

/* -------------------- dotNetBase  ------------------- */
// 
#pragma unmanaged

dotNetBaseDotNetLifetimeControlledHashTable* dotNetBase::m_pDotNetLifetimeControlledValues = NULL;

#ifdef _DEBUG
int dotNetBase::instanceCounter = 0;
#endif

dotNetBase::dotNetBase() : m_pEventHandlers(NULL)
{
#ifdef _DEBUG
	instanceCounter++;
#endif
}

dotNetBase::~dotNetBase()
{
	if (m_pDotNetLifetimeControlledValues && get_lifetime_controlled_by_dotnet())
	{
		m_pDotNetLifetimeControlledValues->remove(this);
		if (m_pDotNetLifetimeControlledValues->num_entries() == 0)
		{
			m_pDotNetLifetimeControlledValues->make_collectable();
			m_pDotNetLifetimeControlledValues = NULL;
		}
	}
#ifdef _DEBUG
	instanceCounter--;
#endif
}

void
dotNetBase::gc_trace()
{
	Value::gc_trace();
	if (m_pEventHandlers && m_pEventHandlers->is_not_marked())
		m_pEventHandlers->gc_trace();
}

Value*
dotNetBase::get_property(Value** arg_list, int count)
{
	// getProperty <dotNetControl> <prop name> [asDotNetObject:<bool>]
	// getProperty <dotNetObject> <prop name> [asDotNetObject:<bool>]
	// getProperty <dotNetClass> <prop name> [asDotNetObject:<bool>]
	Value* prop = arg_list[0];
	Value* result = NULL;

	if (GetDotNetObjectWrapper() && (result = GetDotNetObjectWrapper()->get_property(arg_list, count)) != NULL)
		return_protected(result)
	else
	return Value::get_property(arg_list, count);
}

Value*
dotNetBase::set_property(Value** arg_list, int count)
{
	// setProperty <dotNetControl> <prop name> <value>
	// setProperty <dotNetObject> <prop name> <value>
	// setProperty <dotNetClass> <prop name> <value>

	//	Value* val = arg_list[0];
	//	Value* prop = arg_list[1];

	Value* result = NULL;
	if (GetDotNetObjectWrapper() && (result = GetDotNetObjectWrapper()->set_property(arg_list, count)) != NULL)
		return_protected(result)
	else
	return Value::set_property(arg_list, count);
}

Value*
dotNetBase::get_props_vf(Value** arg_list, int count)
{
	// getPropNames <dotNetControl> [showStaticOnly:<bool>] [declaredOnTypeOnly:<bool>] -- returns array of prop names of wrapped obj
	// getPropNames <dotNetObject> [showStaticOnly:<bool>] [declaredOnTypeOnly:<bool>] -- returns array of prop names of wrapped obj
	// getPropNames <dotNetClass> [showStaticOnly:<bool>] [declaredOnTypeOnly:<bool>] -- returns array of prop names of wrapped obj

	bool showStaticOnly = key_arg_or_default(showStaticOnly,DefaultShowStaticOnly())->to_bool() != FALSE;
	bool declaredOnTypeOnly = key_arg_or_default(declaredOnTypeOnly,&false_value)->to_bool() != FALSE;

	one_typed_value_local(Array* result);
	vl.result = new Array (0);
	if (GetDotNetObjectWrapper())
		GetDotNetObjectWrapper()->get_property_names(vl.result, showStaticOnly, declaredOnTypeOnly);
	return_value(vl.result);
}

Value*
dotNetBase::show_props_vf(Value** arg_list, int count)
{
	// showProperties <dotNetControl> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [showMethods:<bool>] [showAttributes:<bool>] [declaredOnTypeOnly:<bool>]
	// showProperties <dotNetObject> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [showMethods:<bool>] [showAttributes:<bool>] [declaredOnTypeOnly:<bool>]
	// showProperties <dotNetClass> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [showMethods:<bool>] [showAttributes:<bool>] [declaredOnTypeOnly:<bool>]
	CharStream*	out;
	MCHAR*		prop_pat = _M("*");

	if (!GetDotNetObjectWrapper())
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

	bool showStaticOnly = key_arg_or_default(showStaticOnly,DefaultShowStaticOnly())->to_bool() != FALSE;
	bool showMethods = key_arg_or_default(showMethods,&false_value)->to_bool() != FALSE;
	bool showAttributes = key_arg_or_default(showAttributes,&false_value)->to_bool() != FALSE;
	bool declaredOnTypeOnly = key_arg_or_default(declaredOnTypeOnly,&false_value)->to_bool() != FALSE;

	// look in properties, then fields
	bool res = GetDotNetObjectWrapper()->show_properties(prop_pat, out, showStaticOnly, showMethods, showAttributes, declaredOnTypeOnly);
	if (out || !res) // not searching for prop match, or prop match failed
		res |= GetDotNetObjectWrapper()->show_fields(prop_pat, out, showStaticOnly, showAttributes, declaredOnTypeOnly);
	return bool_result(res);
}

Value*
dotNetBase::show_methods_vf(Value** arg_list, int count)
{
	// showMethods <dotNetControl> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [showSpecial:<bool>] [showAttributes:<bool>] [declaredOnTypeOnly:<bool>]
	// showMethods <dotNetObject> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [showSpecial:<bool>] [showAttributes:<bool>] [declaredOnTypeOnly:<bool>]
	// showMethods <dotNetClass> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [showSpecial:<bool>] [showAttributes:<bool>] [declaredOnTypeOnly:<bool>]
	CharStream*	out;
	MCHAR*		prop_pat = _M("*");

	if (!GetDotNetObjectWrapper())
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

	bool showStaticOnly = key_arg_or_default(showStaticOnly,DefaultShowStaticOnly())->to_bool() != FALSE;
	bool showSpecial = key_arg_or_default(showSpecial,&false_value)->to_bool() != FALSE;
	bool showAttributes = key_arg_or_default(showAttributes,&false_value)->to_bool() != FALSE;
	bool declaredOnTypeOnly = key_arg_or_default(declaredOnTypeOnly,&false_value)->to_bool() != FALSE;

	bool res = GetDotNetObjectWrapper()->show_methods(prop_pat, out, showStaticOnly, showSpecial, showAttributes, declaredOnTypeOnly);
	return bool_result(res);
}

Value*
dotNetBase::show_events_vf(Value** arg_list, int count)
{
	// showEvents <dotNetControl> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [declaredOnTypeOnly:<bool>]
	// showEvents <dotNetObject> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [declaredOnTypeOnly:<bool>]
	// showEvents <dotNetClass> ["prop_pat"] [to:<stream>] [showStaticOnly:<bool>] [declaredOnTypeOnly:<bool>]
	CharStream*	out;
	MCHAR*		prop_pat = _M("*");

	if (!GetDotNetObjectWrapper())
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

	bool showStaticOnly = key_arg_or_default(showStaticOnly,DefaultShowStaticOnly())->to_bool() != FALSE;
	bool declaredOnTypeOnly = key_arg_or_default(declaredOnTypeOnly,&false_value)->to_bool() != FALSE;

	bool res = GetDotNetObjectWrapper()->show_events(prop_pat, out, showStaticOnly, declaredOnTypeOnly);
	return bool_result(res);
}

/*
// see comments in utils.cpp
Value*
dotNetBase::show_interfaces_vf(Value** arg_list, int count)
{
	// showInterfaces <dotNetControl> ["prop_pat"] [to:<stream>]
	// showInterfaces <dotNetObject> ["prop_pat"] [to:<stream>]
	// showInterfaces <dotNetClass> ["prop_pat"] [to:<stream>]
	CharStream*	out;
	MCHAR*		prop_pat = _M("*");

	if (!GetDotNetObjectWrapper())
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

	bool res = GetDotNetObjectWrapper()->show_interfaces(prop_pat, out);
	return bool_result(res);
}

Value*	
dotNetBase::get_interfaces_vf(Value** arg_list, int count)
{
	// getInterfaces <dotNetControl> ["prop_pat"] [to:<stream>]
	// getInterfaces <dotNetObject> ["prop_pat"] [to:<stream>]
	// getInterfaces <dotNetClass> ["prop_pat"] [to:<stream>]
	check_arg_count(getInterfaces, 1, count+1);
	one_typed_value_local(Array* result);
	vl.result = new Array (0);
	if (GetDotNetObjectWrapper())
		GetDotNetObjectWrapper()->get_interfaces(vl.result);
	return_value(vl.result);
}
*/

Value*
dotNetBase::get_event_handlers(Value* eventName)
{
	Value *handlerFns = &undefined;
	if (m_pEventHandlers)
	{
		handlerFns = m_pEventHandlers->get(eventName);
		if (handlerFns == NULL)
			handlerFns = &undefined;
	}
	return handlerFns;
}

void
dotNetBase::add_event_handler(Value* eventName, Value* handler)
{
	DotNetObjectWrapper *wrapper =  GetDotNetObjectWrapper();
	if (!wrapper->HasEvent(eventName->to_string()))
		throw RuntimeError(_M("Specified event does not exist for dotNet object: "), eventName);
	if (!m_pEventHandlers)
		m_pEventHandlers = new HashTable();
	Array *handlerFns = (Array*)m_pEventHandlers->get(eventName);
	if (handlerFns)
		handlerFns->append(handler);
	else
	{
		handlerFns = new Array(1);
		handlerFns->append(handler);
		m_pEventHandlers->put(eventName, handlerFns);
	}
}

void
dotNetBase::remove_event_handler(Value* eventName, Value* handler)
{
	if (m_pEventHandlers)
	{
		Array *handlerFns = (Array*)m_pEventHandlers->get(eventName);
		if (handlerFns)
		{
			for (int i = 0; i < handlerFns->size; i++)
			{
				if (((*handlerFns)[i]) == handler)
				{
					for (int j = i+1; j < handlerFns->size; j++, i++)
						(*handlerFns)[i] = (*handlerFns)[j];
					handlerFns->size--;
					break;
				}
			}
			if (handlerFns->size == 0)
				m_pEventHandlers->remove(eventName);
			if (m_pEventHandlers->num_entries() == 0)
				remove_all_event_handlers();
		}
	}
}

void
dotNetBase::remove_all_event_handlers(Value* eventName)
{
	if (m_pEventHandlers)
	{
		Array *handlerFns = (Array*)m_pEventHandlers->get(eventName);
		if (handlerFns)
			m_pEventHandlers->remove(eventName);
		if (m_pEventHandlers->num_entries() == 0)
			remove_all_event_handlers();
	}
}

void
dotNetBase::remove_all_event_handlers()
{
	m_pEventHandlers = NULL;
}

void
dotNetBase::set_lifetime_controlled_by_dotnet(bool lifetime_controlled_by_dotnet)
{
	DotNetObjectWrapper *wrapper =  GetDotNetObjectWrapper();
	wrapper->SetLifetimeControlledByDotnet(lifetime_controlled_by_dotnet);
	if (lifetime_controlled_by_dotnet)
	{
		if (!m_pDotNetLifetimeControlledValues)
			m_pDotNetLifetimeControlledValues = new (GC_STATIC) dotNetBaseDotNetLifetimeControlledHashTable();
		m_pDotNetLifetimeControlledValues->put(this,NULL);
		flags3 |= LIFETIME_CONTROLLED_BY_DOTNET_FLAG;
	}
	else
	{
		if (m_pDotNetLifetimeControlledValues)
		{
			m_pDotNetLifetimeControlledValues->remove(this);
			if (m_pDotNetLifetimeControlledValues->num_entries() == 0)
			{
				m_pDotNetLifetimeControlledValues->make_collectable();
				m_pDotNetLifetimeControlledValues = NULL;
			}
		}
		flags3 &= ~LIFETIME_CONTROLLED_BY_DOTNET_FLAG;
	}
}

