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
// DESCRIPTION: dotNetObject.cpp  : MAXScript dotNetObject code
// AUTHOR: Larry.Minton - created Jan.1.2006
//***************************************************************************/
//

#include "dotNetControl.h"
#include "DotNetObjectManaged.h"
#include "resource.h"

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

using namespace MXS_dotNet;

/* -------------------- dotNetObject  ------------------- */
// 
#pragma unmanaged

#pragma warning(disable:4835)
visible_class_instance(MXS_dotNet::dotNetObject, _M("dotNetObject"));
#pragma warning(default:4835)

Value*
dotNetObjectClass::apply(Value** arg_list, int count, CallContext* cc)
{
	// dotNetObject { <dotNetClass> | <class_type_string> } ... additional args as needed ...
	if (count == 0)
		check_arg_count(dotNetObject, 1, count);
	two_value_locals(arg, result);
	vl.arg = arg_list[0]->eval();
	if (is_dotNetClass(vl.arg))
	{
		vl.result = DotNetObjectManaged::Create(NULL, (dotNetClass*)vl.arg, &arg_list[1], count-1);
	}
	else
	{
		MCHAR *type = vl.arg->to_string();
		vl.result = DotNetObjectManaged::Create(type, NULL, &arg_list[1], count-1);
	}
	return_value(vl.result);
}

dotNetObject::dotNetObject() : m_pDotNetObject(NULL)
{
	tag = class_tag(dotNetObject); 
}

dotNetObject::~dotNetObject()
{
	if (m_pDotNetObject)
		delete m_pDotNetObject;
}

void
dotNetObject::SetObject(DotNetObjectManaged* pDotNetObject)
{
	m_pDotNetObject = pDotNetObject;
	if (m_typeName.isNull() && m_pDotNetObject)
		m_typeName = m_pDotNetObject->GetTypeName();
}

void
dotNetObject::collect()
{
	delete this;
}

void
dotNetObject::sprin1(CharStream* s)
{
	if (m_typeName.isNull() && m_pDotNetObject)
		m_typeName = m_pDotNetObject->GetTypeName();
	s->printf(_M("dotNetObject:%s"), m_typeName);
}

Value*
dotNetObject::eq_vf(Value** arg_list, int count)
{
	// dotNetObject operator == 
	Value* cmpnd = arg_list[0];
	if (cmpnd == this)
		return &true_value;
	if (m_pDotNetObject && comparable(cmpnd))
	{
		dotNetObject *l_p_dotNetObject = (dotNetObject*)cmpnd;
		return bool_result(m_pDotNetObject->Compare(l_p_dotNetObject->m_pDotNetObject));
	}
	else
		return &false_value;
}

Value*
dotNetObject::ne_vf(Value** arg_list, int count)
{
	// dotNetObject operator != 
	Value* cmpnd = arg_list[0];
	if (cmpnd == this)
		return &false_value;
	if (m_pDotNetObject && comparable(cmpnd))
	{
		dotNetObject *l_p_dotNetObject = (dotNetObject*)cmpnd;
		return bool_result(!m_pDotNetObject->Compare(l_p_dotNetObject->m_pDotNetObject));
	}
	else
		return &true_value;
}

Value*
dotNetObject::copy_vf(Value** arg_list, int count) 
{ 
	if (m_pDotNetObject)
		return m_pDotNetObject->Copy();
	return this;
}
