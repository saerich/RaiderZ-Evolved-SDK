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
// DESCRIPTION: dotNetClass.cpp  : MAXScript dotNetClass code
// AUTHOR: Larry.Minton - created Jan.1.2006
//***************************************************************************/
//

#include "dotNetControl.h"
#include "DotNetClassManaged.h"
#include "resource.h"

#include "defextfn.h"
#	include "namedefs.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

using namespace MXS_dotNet;

/* -------------------- dotNetClass  ------------------- */
// 
#pragma unmanaged

#pragma warning(disable:4835)
visible_class_instance(MXS_dotNet::dotNetClass, _M("dotNetClass"));
#pragma warning(default:4835)

Value*
dotNetClassClass::apply(Value** arg_list, int count, CallContext* cc)
{
	// dotNetClass {<dotNetObject> | <dotNetControl> | <class_type_string>}
	check_arg_count(dotNetClass, 1, count);
	two_value_locals(val, result);
	vl.val = arg_list[0]->eval();
	if (is_dotNetObject(vl.val) || is_dotNetControl(vl.val))
	{
		DotNetObjectWrapper *wrapper = NULL;
		if (is_dotNetObject(vl.val)) wrapper = ((dotNetObject*)vl.val)->GetDotNetObjectWrapper();
		else wrapper = ((dotNetControl*)vl.val)->GetDotNetObjectWrapper();
		if (wrapper)
			vl.result = DotNetClassManaged::Create(wrapper);
	}
	else
	{
		MCHAR *type = vl.val->to_string();
		vl.result = DotNetClassManaged::Create(type);
	}
	return_value(vl.result);
}

dotNetClass::dotNetClass() : m_pDotNetClass(NULL)
{
	tag = class_tag(dotNetClass); 
}

dotNetClass::~dotNetClass()
{
	if (m_pDotNetClass)
		delete m_pDotNetClass;
}

void
dotNetClass::sprin1(CharStream* s)
{
	if (m_typeName.isNull() && m_pDotNetClass)
		m_typeName = m_pDotNetClass->GetTypeName();
	s->printf(_M("dotNetClass:%s"), (const MCHAR*) m_typeName);
}


