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

#include "DotNetClassManaged.h"
#include "dotNetControl.h"
#include "utils.h"

using namespace MXS_dotNet;

/* -------------------- DotNetClassManaged  ------------------- */
// 
#pragma managed

DotNetClassManaged::DotNetClassManaged(System::Type ^ p_type, dotNetClass* p_dotNetClass)
{
	m_delegate_map_proxy.get_proxy(this)->set_type(p_type);
	m_delegate_map_proxy.get_proxy(this)->set_dotNetClass(p_dotNetClass);
	if (p_dotNetClass)
		CreateEventHandlerDelegates();
}

DotNetClassManaged::~DotNetClassManaged()
{
}

void DotNetClassManaged::delegate_proxy_type::init()
{
	m_pWrappedClasses = gcnew System::Collections::Hashtable(53);
}

void DotNetClassManaged::Init()
{
	delegate_proxy_type::init();
}

Value* DotNetClassManaged::Create(MCHAR* pTypeString)
{
	try
	{
		// Create the user object.
		System::Type ^l_pType = ResolveTypeFromName(pTypeString);
		if (l_pType)
		{
			// see if already have a wrapper
			one_value_local(result);
			vl.result = GetMXSContainerForObject(l_pType);
			if (!vl.result)
			{
				dotNetClass *l_p_dotNetClass;
				vl.result = l_p_dotNetClass = new dotNetClass();
				l_p_dotNetClass->SetDotNetClass(new DotNetClassManaged(l_pType, l_p_dotNetClass));
			}
			return_value(vl.result);
		}
		return &undefined;
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

Value* DotNetClassManaged::Create(DotNetObjectWrapper* wrapper)
{
	try
	{
		System::Object ^ pObject = wrapper->GetObject();
		System::Type ^ l_pType = wrapper->GetType();

		// if object is a type, create DotNetClassManaged for the type. Otherwise get the type of the object
		// and create DotNetClassManaged for that type
		if (pObject)
		{
			System::Type ^ pType = dynamic_cast<System::Type ^>(pObject);
			if (pType)
				l_pType = pType;
		}

		if (l_pType)
		{
			// see if already have a wrapper
			one_value_local(result);
			vl.result = GetMXSContainerForObject(l_pType);
			if (!vl.result)
			{
				dotNetClass *l_p_dotNetClass;
				vl.result = l_p_dotNetClass = new dotNetClass();
				l_p_dotNetClass->SetDotNetClass(new DotNetClassManaged(l_pType, l_p_dotNetClass));
			}
			return_value(vl.result);
		}
		return &undefined;
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

Value* DotNetClassManaged::Create(System::Type ^ type)
{
	try
	{
		// see if already have a wrapper
		one_value_local(result);
		vl.result = GetMXSContainerForObject(type);
		if (!vl.result)
		{
			dotNetClass *l_p_dotNetClass;
			vl.result = l_p_dotNetClass = new dotNetClass();
			l_p_dotNetClass->SetDotNetClass(new DotNetClassManaged(type, l_p_dotNetClass));
		}
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

void DotNetClassManaged::CreateEventHandlerDelegates()
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

void DotNetClassManaged::delegate_proxy_type::ProcessEvent(System::String ^eventName, System::Object ^delegateArgs)
{
	DbgAssert(m_p_native_target);
	if(m_p_native_target)
		m_p_native_target->ProcessEvent(eventName, delegateArgs);
}

System::Object ^ DotNetClassManaged::GetObject()
{
	return nullptr;
}

System::Type ^ DotNetClassManaged::GetType()
{
	return m_delegate_map_proxy.get_proxy(this)->get_type();
}

Value* DotNetClassManaged::GetMXSContainer()
{
	return m_delegate_map_proxy.get_proxy(this)->get_dotNetClass();
}

void DotNetClassManaged::delegate_proxy_type::store_EventDelegatePair(event_delegate_pair ^ pEventDelegatePair)
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

void DotNetClassManaged::StoreEventDelegatePair(event_delegate_pair ^ pEventDelegatePair)
{
	return m_delegate_map_proxy.get_proxy(this)->store_EventDelegatePair(pEventDelegatePair);
}

Value* DotNetClassManaged::GetEventHandlers(Value* eventName) 
{ 
	dotNetClass *l_p_dotNetClass = m_delegate_map_proxy.get_proxy(this)->get_dotNetClass();
	return l_p_dotNetClass->get_event_handlers(eventName);
}

void DotNetClassManaged::SetLifetimeControlledByDotnet(bool lifetime_controlled_by_dotnet)
{ 
	System::Type ^ pType = GetType();
	if (lifetime_controlled_by_dotnet)
		m_delegate_map_proxy.get_proxy(this)->set_type_as_weakref(pType);
	else
		m_delegate_map_proxy.get_proxy(this)->set_type(pType);
}

bool DotNetClassManaged::GetLifetimeControlledByDotnet()
{
	return m_delegate_map_proxy.get_proxy(this)->has_weakref_object();
}

bool DotNetClassManaged::IsWeakRefObjectAlive()
{
	return m_delegate_map_proxy.get_proxy(this)->is_weakref_object_alive();
}

System::Type ^ DotNetClassManaged::delegate_proxy_type::get_type()
{
	if (m_pType)
		return m_pType;
	if (m_pType_weakref)
		return safe_cast<System::Type ^>(m_pType_weakref->Target);
	return nullptr;
}

void DotNetClassManaged::delegate_proxy_type::set_type(System::Type ^ type)
{
	if (type == m_pType)
		return;
	if (m_pType)
	{
		m_pWrappedClasses->Remove(m_pType);
		m_pType = nullptr;
	}
	if (m_pType_weakref)
	{
		m_pWrappedClasses->Remove(m_pType_weakref);
		m_pType_weakref = nullptr;
	}
	try
	{
		m_pType = type;
		if (m_pType)
			m_pWrappedClasses->Add(m_pType, this);
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

void DotNetClassManaged::delegate_proxy_type::set_type_as_weakref(System::Type ^ type)
{
	if (m_pType_weakref && type == m_pType_weakref->Target)
		return;
	if (m_pType)
	{
		m_pWrappedClasses->Remove(m_pType);
		m_pType = nullptr;
	}
	if (m_pType_weakref)
	{
		m_pWrappedClasses->Remove(m_pType_weakref);
		m_pType_weakref = nullptr;
	}
	try
	{
		m_pType_weakref = gcnew System::WeakReference(type);
		m_pWrappedClasses->Add(m_pType_weakref, this);
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

bool DotNetClassManaged::delegate_proxy_type::has_weakref_object()
{
	return m_pType_weakref != nullptr;
}

bool DotNetClassManaged::delegate_proxy_type::is_weakref_object_alive()
{
	return (m_pType_weakref && m_pType_weakref->IsAlive);
}

MSTR DotNetClassManaged::GetTypeName() 
{
	try
	{
		MSTR name;
		System::Type ^ pType = GetType();
		if (pType) 
		{
			System::String ^ typeName = pType->FullName;
			if (!typeName)
				typeName = pType->Name;
			name = MNETStr::ToMSTR(typeName);
		}
		return name;
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

Value* DotNetClassManaged::delegate_proxy_type::get_MXS_container_for_type(System::Type ^ type)
{
	if (m_pWrappedClasses)
	{
		delegate_proxy_type ^ theProxy = safe_cast<delegate_proxy_type ^>(m_pWrappedClasses[type]);
		if (theProxy)
			return theProxy->get_dotNetClass();
	}
	return NULL;
}

void DotNetClassManaged::delegate_proxy_type::detach()
{
	using namespace System;
	using namespace System::Reflection;

	try
	{
		m_p_native_target = NULL; 
		System::Type ^ pType = get_type();
		if (pType)
		{
			// unhook all the event handlers
			if (m_pEventDelegatePairs)
			{
				BindingFlags flags = (BindingFlags)( BindingFlags::Static | 
					BindingFlags::Public | BindingFlags::FlattenHierarchy);
				System::Collections::IEnumerator^ myEnum = m_pEventDelegatePairs->GetEnumerator();
				while ( myEnum->MoveNext() )
				{
					event_delegate_pair^ pEventDelegatePair = safe_cast<event_delegate_pair^>(myEnum->Current);
					EventInfo ^pEventInfo = pType->GetEvent(pEventDelegatePair->m_pEventName, flags);
					DbgAssert(pEventInfo != nullptr);
					if (pEventInfo)
						pEventInfo->RemoveEventHandler(nullptr, pEventDelegatePair->m_pEventDelegate);
				}
				m_pEventDelegatePairs = nullptr;
			}
			set_type(nullptr);
			set_dotNetClass(nullptr);
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

//  returns dotNetClass if object already wrapped
Value* DotNetClassManaged::GetMXSContainerForObject(System::Object ^ object)
{
	Value* result = NULL;
	System::Type ^ asType = dynamic_cast<System::Type^>(object);
	if (asType)
	{
		result = delegate_proxy_type::get_MXS_container_for_type(asType);
		if (result == NULL && 
			dotNetBase::m_pDotNetLifetimeControlledValues && 
			dotNetBase::m_pDotNetLifetimeControlledValues->m_pExisting_dotNetBaseDotNetLifetimeControlledValues)
		{
			dotNetBaseDotNetLifetimeControlledHashTableMapperObjectFinder htm(nullptr, asType);
			dotNetBase::m_pDotNetLifetimeControlledValues->m_pExisting_dotNetBaseDotNetLifetimeControlledValues->map_keys_and_vals(&htm);
			result = htm.m_result;
		}
	}
	return result;
}