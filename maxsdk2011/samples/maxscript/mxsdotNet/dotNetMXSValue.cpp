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
// DESCRIPTION: DotNetMXSValue.cpp  : MAXScript DotNetMXSValue code
// AUTHOR: Larry.Minton - created Jan.1.2006
//***************************************************************************/
//

#include "dotNetControl.h"
#include "resource.h"

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

using namespace MXS_dotNet;

/* -------------------- DotNetMXSValue  ------------------- */
// 
#pragma managed

DotNetMXSValue::DotNetMXSValue(dotNetMXSValue *p_dotNetMXSValue)
{
	m_delegate_map_proxy.get_proxy(this)->set_dotNetMXSValue(p_dotNetMXSValue);
}

void DotNetMXSValue::delegate_proxy_type::detach()
{
	m_p_native_target = NULL; 
}

Value* 
DotNetMXSValue::DotNetMXSValue_proxy::get_value()
{
	return m_p_delegate_proxy_type->get_dotNetMXSValue(); 
}

dotNetMXSValue* 
DotNetMXSValue::Get_dotNetMXSValue()
{
	return m_delegate_map_proxy.get_proxy(this)->get_dotNetMXSValue();
}

DotNetMXSValue::DotNetMXSValue_proxy ^ 
DotNetMXSValue::delegate_proxy_type::get_weak_proxy()
{
	if (!m_pDotNetMXSValue_proxy_weakRef)
	{
		DotNetMXSValue_proxy ^l_pDotNetMXSValue_proxy = gcnew DotNetMXSValue::DotNetMXSValue_proxy(this);
		m_pDotNetMXSValue_proxy_weakRef = gcnew System::WeakReference(l_pDotNetMXSValue_proxy);
		return l_pDotNetMXSValue_proxy;
	}
	if (!m_pDotNetMXSValue_proxy_weakRef->IsAlive)
		m_pDotNetMXSValue_proxy_weakRef->Target = gcnew DotNetMXSValue::DotNetMXSValue_proxy(this);
	return (DotNetMXSValue::DotNetMXSValue_proxy ^)m_pDotNetMXSValue_proxy_weakRef->Target;
}

DotNetMXSValue::DotNetMXSValue_proxy ^ 
DotNetMXSValue::GetWeakProxy()
{
	return m_delegate_map_proxy.get_proxy(this)->get_weak_proxy();
}

bool
DotNetMXSValue::delegate_proxy_type::is_alive()
{
	return ( m_pDotNetMXSValue_proxy_weakRef && m_pDotNetMXSValue_proxy_weakRef->IsAlive );
}

bool
DotNetMXSValue::IsAlive()
{
	return m_delegate_map_proxy.get_proxy(this)->is_alive();
}

/* -------------------- dotNetMXSValue  ------------------- */
// 
#pragma unmanaged

#pragma warning(disable:4835)
visible_class_instance (MXS_dotNet::dotNetMXSValue, _M("dotNetMXSValue"))
#pragma warning(default:4835)

Value*
dotNetMXSValueClass::apply(Value** arg_list, int count, CallContext* cc)
{
	check_arg_count(dotNetMXSValue, 1, count);
	return dotNetMXSValue::intern(arg_list[0]->eval());
}

dotNetMXSValueHashTable* dotNetMXSValue::m_pExisting_dotNetMXSValues = NULL;

dotNetMXSValue::dotNetMXSValue(Value *pMXSValue) : m_pDotNetMXSValue(NULL)
{
	tag = class_tag(dotNetMXSValue);
	m_pMXSValue = pMXSValue;
	m_pDotNetMXSValue = new DotNetMXSValue(this);
	m_pExisting_dotNetMXSValues->put(pMXSValue, this);
}

dotNetMXSValue::~dotNetMXSValue()
{
	m_pExisting_dotNetMXSValues->remove(m_pMXSValue);
	if (m_pDotNetMXSValue)
		delete m_pDotNetMXSValue;
}

dotNetMXSValue* 
dotNetMXSValue::intern(Value *pMXSValue)
{
	one_typed_value_local(dotNetMXSValue* result);
	if (!m_pExisting_dotNetMXSValues)
		m_pExisting_dotNetMXSValues = new (GC_STATIC) dotNetMXSValueHashTable();
	vl.result = (dotNetMXSValue*)m_pExisting_dotNetMXSValues->get(pMXSValue);
	if (!vl.result)
		vl.result = new dotNetMXSValue(pMXSValue);
	return_value(vl.result);
}

void
dotNetMXSValue::gc_trace()
{
	Value::gc_trace();
	if (m_pMXSValue && m_pMXSValue->is_not_marked())
		m_pMXSValue->gc_trace();
}

void
dotNetMXSValue::sprin1(CharStream* s)
{
	s->printf(_M("dotNetMXSValue:["));
	m_pMXSValue->sprin1(s);
	s->printf(_M("]"));
}

Value*
dotNetMXSValue::get_property(Value** arg_list, int count)
{
	// getProperty <dotNetMXSValue> <prop name>
	Value* prop = arg_list[0];

	if (prop == n_value)
		return m_pMXSValue;
	else
		return Value::get_property(arg_list, count);
}

Value*
dotNetMXSValue::set_property(Value** arg_list, int count)
{
	// setProperty <dotNetMXSValue> <prop name> <value>

	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if (prop == n_value)
	{
		m_pMXSValue = val;
		return val;
	}
	else
		return Value::set_property(arg_list, count);
}

Value*
dotNetMXSValue::get_props_vf(Value** arg_list, int count)
{
	// getPropNames <dotNetMXSValue>
	one_typed_value_local(Array* result);
	vl.result = new Array (1);
	vl.result->append(n_value);
	return_value(vl.result);
}

Value*
dotNetMXSValue::show_props_vf(Value** arg_list, int count)
{
	// showProperties <dotNetMXSValue> ["prop_pat"] [to:<file>]
	CharStream*	out;
	MCHAR*		prop_pat = _M("*");

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

	Value *res = &false_value;
	if (max_name_match(_M("value"), prop_pat))
	{
		res = &true_value;
		if (out == NULL)
			return res;
		out->puts(_M("  .value\n"));
	}
	return res;
}

