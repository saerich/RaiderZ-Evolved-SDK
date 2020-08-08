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
// DESCRIPTION: DotNetMethod.cpp  : MAXScript DotNetMethod code
// AUTHOR: Larry.Minton - created Jan.1.2006
//***************************************************************************/
//

#include "dotNetControl.h"
#include "utils.h"
#include "resource.h"

#include "defextfn.h"
#	include "namedefs.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

using namespace MXS_dotNet;

/* -------------------- DotNetMethod  ------------------- */
// 
#pragma managed

DotNetMethod::DotNetMethod(System::String ^ pMethodName, System::Type ^ pObjectType)
{
	m_delegate_map_proxy.get_proxy(this)->set_MethodName(pMethodName);
	m_delegate_map_proxy.get_proxy(this)->set_type(pObjectType);
}

dotNetMethod* DotNetMethod::intern(System::String ^ pMethodName, DotNetObjectWrapper *pWrapper, System::Type ^ pObjectType)
{
	one_typed_value_local(dotNetMethod *result);
	MSTR methodName(MNETStr::ToMSTR(pMethodName));
	vl.result = new dotNetMethod(methodName, pWrapper);
	vl.result->m_pMethod = new DotNetMethod(pMethodName, pObjectType);
	return_value(vl.result);
}

Value* 
DotNetMethod::apply(DotNetObjectWrapper *pWrapper, Value** arg_list, int count, bool asDotNetObject)
{
	using namespace System::Reflection;
	using namespace System::Collections::Generic;

	try
	{
		BindingFlags flags = (BindingFlags)( BindingFlags::Static | BindingFlags::Instance | BindingFlags::Public | BindingFlags::FlattenHierarchy);

		Value**	evald_args;
		Value**	thunk_args;
		Value	**ap, **eap, **tap;
		int		i;

		one_value_local(result);
		vl.result = &undefined;
		value_local_array(evald_args, count);
		value_local_array(thunk_args, count);

		// stack allocate space for temp eval'd args, run through evaling them all
		for (i = count, ap = arg_list, eap = evald_args, tap = thunk_args; i > 0 ; eap++, ap++, tap++, i--)
		{
			*eap = (*ap)->eval();
			if (is_thunk(*eap))
			{
				*tap = *eap;
				*eap = (*eap)->eval();
			}
		}

		System::String ^ l_pMethodName = m_delegate_map_proxy.get_proxy(this)->get_MethodName();
		System::Type ^ l_pObjectType = m_delegate_map_proxy.get_proxy(this)->get_type();

		array<System::Object^> ^argArray;
		MethodInfo^ l_pMethodInfo;
		int arg_count = count_with_keys();
		if (count != 0)
		{
			// collect the methods that have the right number of parameters
			List<MethodInfo^> ^candidateMInfos = gcnew List<MethodInfo^>();
			List<array<ParameterInfo^>^> ^candidateParamLists = gcnew List<array<ParameterInfo^>^>;

			array<MethodInfo^> ^methods = l_pObjectType->GetMethods(flags);
			for(int m=0; m<methods->Length; m++)
			{
				MethodInfo^ mInfo = methods[m];
				if (l_pMethodName->Equals(mInfo->Name))
				{
					array<ParameterInfo^> ^params = mInfo->GetParameters();
					if (params->Length == arg_count)
					{
						candidateMInfos->Add(mInfo);
						candidateParamLists->Add(params);
					}
				}
			}

			// Find the best match between supplied args and the parameters, convert args to parameter types
			Value* userSpecifiedParamTypes = _get_key_arg(evald_args, count, n_paramTypes);
			Array* userSpecifiedParamTypeArray = NULL;
			if (userSpecifiedParamTypes != &unsupplied)
			{
				if (is_array(userSpecifiedParamTypes))
				{
					userSpecifiedParamTypeArray = (Array*)userSpecifiedParamTypes;
					// make sure correct size. Save check to see if elements are Type wrappers for later
					if (userSpecifiedParamTypeArray->size != arg_count)
						throw RuntimeError(_M("Incorrect paramTypes array size"));
				}
				else
					throw ConversionError(userSpecifiedParamTypes, _M("Array"));

			}

			argArray = gcnew array<System::Object^>(arg_count);
			int bestMatch = MXS_dotNet::FindMethodAndPrepArgList(candidateParamLists, evald_args, argArray, 
				arg_count, userSpecifiedParamTypeArray);
			if (bestMatch >= 0)
				l_pMethodInfo = candidateMInfos[bestMatch];
		}
		else
		{
			l_pMethodInfo = l_pObjectType->GetMethod(l_pMethodName, System::Type::EmptyTypes);
			argArray = System::Type::EmptyTypes;
		}

		System::Object ^ l_pResult;
		if (l_pMethodInfo)
		{
			System::Object ^ l_pObject = pWrapper->GetObject();
			l_pResult = l_pMethodInfo->Invoke(l_pObject, argArray);

			// write back to byRef args
			array<ParameterInfo^> ^params = l_pMethodInfo->GetParameters();
			for (int i = 0; i < params->Length; i++)
			{
				ParameterInfo ^ l_pParam = params[i];
				if (l_pParam->ParameterType->IsByRef && !l_pParam->IsIn && thunk_args[i])
				{
					Value *origValue =  thunk_args[i]->eval();
					if (is_dotNetObject(origValue) || is_dotNetControl(origValue))
						((Thunk*)(thunk_args[i]))->assign(DotNetObjectWrapper::intern(argArray[i]));
					else
						((Thunk*)(thunk_args[i]))->assign(Object_To_MXS_Value(argArray[i]));
				}
			}
		}
		else
			throw RuntimeError(_M("No method found which matched argument list"));
		if (l_pResult)
		{
			if (asDotNetObject)
				vl.result = DotNetObjectWrapper::intern(l_pResult);
			else
				vl.result = Object_To_MXS_Value(l_pResult);
		}

		pop_value_local_array(thunk_args);
		pop_value_local_array(evald_args);
		return_value(vl.result);
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

void DotNetMethod::delegate_proxy_type::detach()
{
	m_p_native_target = NULL; 
}

/* -------------------- dotNetMethod  ------------------- */
// 
#pragma unmanaged

#pragma warning(disable:4835)
visible_class_instance (MXS_dotNet::dotNetMethod, _M("dotNetMethod"))
#pragma warning(default:4835)

dotNetMethod::dotNetMethod(MCHAR* pMethodName, DotNetObjectWrapper* pWrapper) : m_pMethod(NULL)
{
	tag = class_tag(dotNetMethod);
	m_pWrapper = pWrapper;
	name = save_string(pMethodName);
}

dotNetMethod::~dotNetMethod()
{
	// dotNetMethod does not own m_pWrapper. gc_trace keeps Value wrapping the DotNetObjectWrapper alive
	if (m_pMethod)
		delete m_pMethod;
}

void
dotNetMethod::gc_trace()
{
	Function::gc_trace();
	if (m_pWrapper)
	{
		Value *container = m_pWrapper->GetMXSContainer();
		if (container)
			container->gc_trace();
	}
}

Value* 
dotNetMethod::apply(Value** arg_list, int count, CallContext* cc)
{
	DbgAssert (!cc);
	DbgAssert (m_pMethod);
	bool asDotNetObject = key_arg_or_default(asDotNetObject, &false_value)->eval()->to_bool() != FALSE;
	return m_pMethod->apply(m_pWrapper, arg_list, count_with_keys(), asDotNetObject);
}

// index access

Value*
dotNetMethod::get_vf(Value** arg_list, int count)
{
	DbgAssert (m_pMethod);
	bool asDotNetObject = key_arg_or_default(asDotNetObject, &false_value)->eval()->to_bool() != FALSE;
	return m_pMethod->apply(m_pWrapper, arg_list, count_with_keys(), asDotNetObject);
}

Value*
dotNetMethod::put_vf(Value** arg_list, int count)
{
	DbgAssert (m_pMethod);
	m_pMethod->apply(m_pWrapper, arg_list, count, false);
	return arg_list[count-1];
}

