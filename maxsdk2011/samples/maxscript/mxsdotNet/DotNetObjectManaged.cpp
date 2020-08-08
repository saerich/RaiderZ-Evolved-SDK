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

#include "MaxIncludes.h"
#include "DotNetObjectManaged.h"
#include "dotNetControl.h"
#include "utils.h"
#include "DotNetHashTables.h"

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

using namespace MXS_dotNet;

/* -------------------- DotNetObjectManaged  ------------------- */
// 
#pragma managed

DotNetObjectManaged::DotNetObjectManaged(dotNetObject* p_dotNetObject)
{
	m_delegate_map_proxy.get_proxy(this)->set_dotNetObject(p_dotNetObject);
}

DotNetObjectManaged::DotNetObjectManaged(System::Object ^ pObject, dotNetObject* p_dotNetObject)
{
	m_delegate_map_proxy.get_proxy(this)->set_object(pObject);
	m_delegate_map_proxy.get_proxy(this)->set_dotNetObject(p_dotNetObject);
	if (pObject)
		CreateEventHandlerDelegates();
}

DotNetObjectManaged::~DotNetObjectManaged()
{
}

Value* DotNetObjectManaged::Create(MCHAR* pTypeString, dotNetClass* p_dotNetClass, Value** arg_list, int count)
{
	using namespace System::Reflection;
	using namespace System::Collections::Generic;

	try
	{

		Value**	evald_args;
		Value**	thunk_args;
		Value	**ap, **eap, **tap;
		int		i;

		// stack allocate space for temp eval'd args, run through evaling them all 
		value_local_array(evald_args, count);
		value_local_array(thunk_args, count);
		for (i = count, ap = arg_list, eap = evald_args, tap = thunk_args; i > 0 ; eap++, ap++, tap++, i--)
		{
			*eap = (*ap)->eval();
			if (is_thunk(*eap))
			{
				*tap = *eap;
				*eap = (*eap)->eval();
			}
		}

		// Create the user object.
		System::Type ^l_pType;
		if (p_dotNetClass)
			l_pType = p_dotNetClass->GetDotNetObjectWrapper()->GetType();
		else if (pTypeString)
			l_pType = ResolveTypeFromName(pTypeString);
		if (l_pType)
		{
			ConstructorInfo ^ l_pConstructorInfo;
			array<System::Object^> ^argArray;
			int arg_count = count_with_keys();
			if (count != 0)
			{
				array<ConstructorInfo^> ^cInfos = l_pType->GetConstructors();

				// if no constructors, we may have a basic data type. Try going through 
				// MXS_Value_To_Object
				if (cInfos->Length == 0 && count == 1)
				{
					System::Object ^l_pObject = MXS_dotNet::MXS_Value_To_Object(evald_args[0],l_pType);
					pop_value_local_array(thunk_args);
					pop_value_local_array(evald_args);
					return intern(l_pObject);
				}

				// collect the constructors that have the right number of parameters
				List<ConstructorInfo^> ^candidateCInfos = gcnew List<ConstructorInfo^>();
				List<array<ParameterInfo^>^> ^candidateParamLists = gcnew List<array<ParameterInfo^>^>;

				for(int c=0; c<cInfos->Length; c++)
				{
					ConstructorInfo ^ cInfo = cInfos[c];
					array<ParameterInfo^> ^params = cInfo->GetParameters();
					if (params->Length == arg_count)
					{
						candidateCInfos->Add(cInfo);
						candidateParamLists->Add(params);
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
				int bestMatch = MXS_dotNet::FindMethodAndPrepArgList(candidateParamLists, evald_args, 
					argArray, arg_count, userSpecifiedParamTypeArray);
				if (bestMatch >= 0)
					l_pConstructorInfo = candidateCInfos[bestMatch];
			}
			else
			{
				l_pConstructorInfo = l_pType->GetConstructor(System::Type::EmptyTypes);
				argArray = System::Type::EmptyTypes;
			}

			System::Object ^ l_pObject;
			if (l_pConstructorInfo)
				l_pObject = l_pConstructorInfo->Invoke(argArray);
			else
				throw RuntimeError(_M("No constructor found which matched argument list: "), pTypeString);
			if (l_pObject)
			{
				// write back to byRef args
				array<ParameterInfo^> ^params = l_pConstructorInfo->GetParameters();
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
				pop_value_local_array(thunk_args);
				pop_value_local_array(evald_args);
				return intern(l_pObject);
			}
			else
				throw RuntimeError(_M("Construction of object failed: "), pTypeString);
		}
		else
			throw RuntimeError(_M("Cannot resolve type: "), pTypeString);
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

void DotNetObjectManaged::CreateEventHandlerDelegates()
{
	using namespace System;
	using namespace System::Reflection;
	using namespace System::Reflection::Emit;
	// Event handler methods are generated at run time using lightweight dynamic methods and Reflection.Emit. 
	// get the method that the delegate's dynamic method will be calling

	try
	{
		Type ^ proxyType = m_delegate_map_proxy.get_proxy(this)->GetType();
		MethodInfo ^mi = proxyType->GetMethod("ProcessEvent");
		DotNetObjectWrapper::CreateEventHandlerDelegates(m_delegate_map_proxy.get_proxy(this), mi);
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

void DotNetObjectManaged::delegate_proxy_type::init()
{
	m_pWrappedObjects = gcnew System::Collections::Hashtable(53);
}

void DotNetObjectManaged::Init()
{
	delegate_proxy_type::init();
}

System::Object ^ DotNetObjectManaged::GetObject()
{
	return m_delegate_map_proxy.get_proxy(this)->get_object();
}

Value* DotNetObjectManaged::GetMXSContainer()
{
	return m_delegate_map_proxy.get_proxy(this)->get_dotNetObject();
}

void DotNetObjectManaged::delegate_proxy_type::store_EventDelegatePair(event_delegate_pair ^ pEventDelegatePair)
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

void DotNetObjectManaged::StoreEventDelegatePair(event_delegate_pair ^ pEventDelegatePair)
{
	return m_delegate_map_proxy.get_proxy(this)->store_EventDelegatePair(pEventDelegatePair);
}

Value* DotNetObjectManaged::GetEventHandlers(Value* eventName) 
{ 
	dotNetObject *l_p_dotNetObject = m_delegate_map_proxy.get_proxy(this)->get_dotNetObject();
	return l_p_dotNetObject->get_event_handlers(eventName);
}

void DotNetObjectManaged::SetLifetimeControlledByDotnet(bool lifetime_controlled_by_dotnet)
{ 
	System::Object ^ target = GetObject();
	if (lifetime_controlled_by_dotnet)
		m_delegate_map_proxy.get_proxy(this)->set_object_as_weakref(target);
	else
		m_delegate_map_proxy.get_proxy(this)->set_object(target);
}

bool DotNetObjectManaged::GetLifetimeControlledByDotnet()
{
	return m_delegate_map_proxy.get_proxy(this)->has_weakref_object();
}

bool DotNetObjectManaged::IsWeakRefObjectAlive()
{
	return m_delegate_map_proxy.get_proxy(this)->is_weakref_object_alive();
}

System::Object ^ DotNetObjectManaged::delegate_proxy_type::get_object()	
{
	if (m_pObject)
		return m_pObject; 
	if (m_pObject_weakref)
		return m_pObject_weakref->Target;
	return nullptr;
}

void DotNetObjectManaged::delegate_proxy_type::set_object(System::Object ^ object)
{
	if (object == m_pObject)
		return;
	if (m_pObject)
	{
		m_pWrappedObjects->Remove(m_pObject);
		m_pObject = nullptr;
	}
	if (m_pObject_weakref)
	{
		m_pWrappedObjects->Remove(m_pObject_weakref);
		m_pObject_weakref = nullptr;
	}

	try
	{
		m_pObject = object;
		// we can't cache structure instances. Hashcode is same for 2 different
		// instances with the same values. 
		if (m_pObject)
		{
			System::Type ^ type = m_pObject->GetType();
			if (!type->IsValueType || type->IsPrimitive || type->IsEnum)
				m_pWrappedObjects->Add(m_pObject, this);
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

void DotNetObjectManaged::delegate_proxy_type::set_object_as_weakref(System::Object ^ object)
{
	if (m_pObject_weakref && object == m_pObject_weakref->Target)
		return;
	if (m_pObject)
	{
		m_pWrappedObjects->Remove(m_pObject);
		m_pObject = nullptr;
	}
	if (m_pObject_weakref)
	{
		m_pWrappedObjects->Remove(m_pObject_weakref);
		m_pObject_weakref = nullptr;
	}

	try
	{
		if (object)
		{
			m_pObject_weakref = gcnew System::WeakReference(object);
			m_pWrappedObjects->Add(m_pObject_weakref, this);
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

bool DotNetObjectManaged::delegate_proxy_type::has_weakref_object()
{
	return m_pObject_weakref != nullptr;
}

bool DotNetObjectManaged::delegate_proxy_type::is_weakref_object_alive()
{
	return (m_pObject_weakref && m_pObject_weakref->IsAlive);
}

dotNetObject* DotNetObjectManaged::delegate_proxy_type::get_dotNetObject() { 
	return m_p_dotNetObject; 
}

void DotNetObjectManaged::delegate_proxy_type::set_dotNetObject(dotNetObject *_dotNetObject) { 
	m_p_dotNetObject = _dotNetObject; 
}

Value* DotNetObjectManaged::delegate_proxy_type::get_MXS_container_for_object(System::Object ^ object)
{
	if (m_pWrappedObjects)
	{
		delegate_proxy_type ^ theProxy = safe_cast<delegate_proxy_type ^>(m_pWrappedObjects[object]);
		if (theProxy)
			return theProxy->get_dotNetObject();
	}
	return NULL;
}

void DotNetObjectManaged::delegate_proxy_type::detach()
{
	using namespace System;
	using namespace System::Reflection;

	try
	{
		m_p_native_target = NULL; 
		Object ^ target = get_object();
		if (target)
		{
			// unhook all the event handlers
			if (m_pEventDelegatePairs)
			{
				Type ^ targetType = target->GetType();
				BindingFlags flags = (BindingFlags)( BindingFlags::Static | 
					BindingFlags::Public | BindingFlags::FlattenHierarchy | BindingFlags::Instance);
				System::Collections::IEnumerator^ myEnum = m_pEventDelegatePairs->GetEnumerator();
				while ( myEnum->MoveNext() )
				{
					event_delegate_pair^ pEventDelegatePair = safe_cast<event_delegate_pair^>(myEnum->Current);
					EventInfo ^pEventInfo = targetType->GetEvent(pEventDelegatePair->m_pEventName, flags);
					DbgAssert(pEventInfo != nullptr);
					if (pEventInfo)
						pEventInfo->RemoveEventHandler(target, pEventDelegatePair->m_pEventDelegate);
				}
				m_pEventDelegatePairs = nullptr;
			}
			set_object(nullptr);
			set_dotNetObject(nullptr);
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

void DotNetObjectManaged::delegate_proxy_type::ProcessEvent(System::String ^eventName, System::Object ^delegateArgs)
{
	DbgAssert(m_p_native_target);
	if(m_p_native_target)
		m_p_native_target->ProcessEvent(eventName, delegateArgs);
}

//  returns dotNetObject if object already wrapped
Value* DotNetObjectManaged::GetMXSContainerForObject(System::Object ^ object)
{
	Value* result = delegate_proxy_type::get_MXS_container_for_object(object);
	if (result == NULL && 
		dotNetBase::m_pDotNetLifetimeControlledValues && 
		dotNetBase::m_pDotNetLifetimeControlledValues->m_pExisting_dotNetBaseDotNetLifetimeControlledValues)
	{
		dotNetBaseDotNetLifetimeControlledHashTableMapperObjectFinder htm(object, nullptr);
		dotNetBase::m_pDotNetLifetimeControlledValues->m_pExisting_dotNetBaseDotNetLifetimeControlledValues->map_keys_and_vals(&htm);
		result = htm.m_result;
	}
	return result;
}

//  returns dotNetObject if object already wrapped
bool DotNetObjectManaged::Compare(DotNetObjectManaged * pCompareVal)
{
	System::Object ^ l_pObject = GetObject();
	System::Object ^ l_pCompareObject;
	if (pCompareVal)
		l_pCompareObject = pCompareVal->GetObject();

	if (!l_pObject || !l_pCompareObject)
		return false;
	if (l_pObject == l_pCompareObject)
		return true;
	return l_pObject->Equals(l_pCompareObject);
}

//  returns dotNetObject containing a copy of the wrapped object 
Value* DotNetObjectManaged::Copy()
{
	using namespace System;
	using namespace System::Reflection;

	try
	{
		System::Object ^ l_pObject = GetObject();
		System::ICloneable ^ l_pICloneable = dynamic_cast<System::ICloneable^>(l_pObject);
		if (l_pICloneable) 
			return intern(l_pICloneable->Clone());
		// see if a structure. 
		System::Type ^ l_pType = GetType();
		if (l_pType->IsValueType && !l_pType->IsPrimitive)
		{
			System::Object ^ l_pNewObject;
			try
			{
				TypeDelegator ^ td = gcnew TypeDelegator(l_pType);
				BindingFlags flags = (BindingFlags)( BindingFlags::FlattenHierarchy | BindingFlags::Public | BindingFlags::Instance | BindingFlags::InvokeMethod);
				l_pNewObject = td->InvokeMember( "MemberwiseClone", flags, nullptr, l_pObject, nullptr );
			}
			catch (System::Exception ^) { }
			if (l_pNewObject)
				return intern(l_pNewObject);
		}
		return GetMXSContainer();
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
