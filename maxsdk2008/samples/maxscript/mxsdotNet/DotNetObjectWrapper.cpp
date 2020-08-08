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
// DESCRIPTION: DotNetObjectWrapper.cpp  : MAXScript DotNetObjectWrapper code
// AUTHOR: Larry.Minton - created Jan.1.2006
//***************************************************************************/
//

#include "DotNetObjectWrapper.h"
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

/* -------------------- DotNetObjectWrapper  ------------------- */
// 
#pragma managed

// wraps existing System::Object, returns dotNetObject or dotNetControl if present, otherwise
// returns new dotNetObject
Value* DotNetObjectWrapper::intern(System::Object ^ object)
{
	try
	{
		if (!object)
			return &undefined;
		one_value_local(result);
		// look to see if already have a wrapper for the object
		vl.result = DotNetObjectManaged::GetMXSContainerForObject(object);
		if (!vl.result)
			vl.result = DotNetObjectManaged::GetMXSContainerForObject(object);
		if (!vl.result)
		{
			dotNetObject *l_p_dotNetObject;
			vl.result = l_p_dotNetObject = new dotNetObject();
			l_p_dotNetObject->m_pDotNetObject = new DotNetObjectManaged(object, l_p_dotNetObject);
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

Value* DotNetObjectWrapper::WrapType(System::Type ^ pType)
{
	return intern(pType);
}


Value* DotNetObjectWrapper::WrapType(MCHAR* pTypeString)
{
	System::Type ^l_pType = ResolveTypeFromName(pTypeString);
	return WrapType(l_pType);
}

Value* DotNetObjectWrapper::WrapType(DotNetObjectWrapper* pDotNetObjectWrapper)
{
	System::Type ^l_pType = pDotNetObjectWrapper->GetType();
	return WrapType(l_pType);
}

System::Type ^ DotNetObjectWrapper::GetType()
{ 
	System::Object ^ obj = GetObject();
	if (obj)
		return obj->GetType();
	return nullptr;
}

MSTR DotNetObjectWrapper::GetTypeName() 
{
	try
	{
		MSTR name;
		System::Object ^ pObject = GetObject();
		System::Type ^ pType;
		if (pObject) 
		{
			pType = pObject->GetType();
			System::String ^ typeName = pType->FullName;
			if (!typeName)
				typeName = pType->Name;
			name = MNETStr::ToMSTR(typeName);
		}
		pType = dynamic_cast<System::Type ^>(pObject);
		if (pType)
		{
			name.Append(_M("["));
			System::String ^ pTypeName = pType->FullName;
			if (!pTypeName)
				pTypeName = pType->Name;
			name.Append(MNETStr::ToMSTR(pTypeName));
			name.Append(_M("]"));
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

// TODO: Need to set up a wrapper based on the System::Type that has a hash
// table of prop names vs accessors
Value* DotNetObjectWrapper::get_property(Value** arg_list, int count)
{
	using namespace System;
	using namespace System::Reflection;

	try
	{
		System::Object ^ l_pObject = GetObject();
		Type ^ l_pType = GetType();
		if (!l_pType) 
			return NULL;

		System::Object ^ l_pResult;
		bool found = false;

		BindingFlags flags = (BindingFlags)( BindingFlags::Static | BindingFlags::Instance | BindingFlags::Public | BindingFlags::FlattenHierarchy);

		CString cPropString (arg_list[0]->to_string());
		System::String ^ l_pPropString = gcnew System::String(cPropString);

		array<FieldInfo^> ^l_pFields = l_pType->GetFields(flags);
		FieldInfo^ l_pFieldInfo;
		for(int f=0; f< l_pFields->Length; f++)
		{
			l_pFieldInfo = l_pFields[f];
			System::String ^ l_pFieldName = l_pFieldInfo->Name;
			if (System::String::Compare(l_pPropString, l_pFieldName, StringComparison::OrdinalIgnoreCase) == 0) 
			{
				l_pResult = l_pFieldInfo->GetValue(l_pObject);
				found = true;
				break;
			}
		}
		if (!found)
		{
			array<PropertyInfo^>^ l_pProps = l_pType->GetProperties(flags);
			PropertyInfo^ l_pPropertyInfo;
			for(int m=0; m < l_pProps->Length; m++)
			{
				l_pPropertyInfo = l_pProps[m];
				System::String ^ l_pPropName = l_pPropertyInfo->Name;
				if (System::String::Compare(l_pPropString, l_pPropName, StringComparison::OrdinalIgnoreCase) == 0) 
					break;
				l_pPropertyInfo = nullptr;
			}
			if (l_pPropertyInfo) 
			{
				found = true;
				MethodInfo ^l_pAccessor = l_pPropertyInfo->GetGetMethod();
				if (l_pAccessor) 
				{
					// if accessor has arguments, pass back a dotNetMethod
					array<ParameterInfo^> ^l_pParams = l_pAccessor->GetParameters();
					if (l_pParams->Length != 0)
						return DotNetMethod::intern(l_pAccessor->Name, this, l_pType);
					l_pResult = l_pAccessor->Invoke(l_pObject, nullptr);
				}
			}
		}
		if (!found)
		{
			array<MethodInfo^> ^l_pMethods = l_pType->GetMethods(flags);
			MethodInfo^ l_pMethodInfo;
			System::String ^ l_pMethodName;
			for(int m=0; m < l_pMethods->Length; m++)
			{
				l_pMethodInfo = l_pMethods[m];
				l_pMethodName = l_pMethodInfo->Name;
				if (l_pPropString->Equals(l_pMethodName, System::StringComparison::OrdinalIgnoreCase)) 
					break;
				l_pMethodInfo = nullptr;
			}
			if (l_pMethodInfo) 
				return DotNetMethod::intern(l_pMethodName, this, l_pType);
		}
		if (!found)
			return NULL;
		one_value_local(result);
		if (key_arg_or_default(asDotNetObject, &false_value)->to_bool())
			vl.result = DotNetObjectWrapper::intern(l_pResult);
		else
			vl.result = Object_To_MXS_Value(l_pResult);
		return_value (vl.result);
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

Value* DotNetObjectWrapper::set_property(Value** arg_list, int count)
{
	using namespace System;
	using namespace System::Reflection;

	try
	{
		Value* val = arg_list[0];
		Value* prop = arg_list[1];

		bool found = false;

		System::Object ^ pObject = GetObject();
		Type ^ type = GetType();
		if (!type) 
			return NULL;

		BindingFlags flags = (BindingFlags)( BindingFlags::Static | BindingFlags::Instance | BindingFlags::Public | BindingFlags::FlattenHierarchy);

		CString cPropString (prop->to_string());
		System::String ^ pPropString = gcnew System::String(cPropString);

		array<FieldInfo^> ^fields = type->GetFields(flags);
		FieldInfo^ pFieldInfo;
		for(int f=0; f<fields->Length; f++)
		{
			pFieldInfo = fields[f];
			System::String ^ pFieldName = pFieldInfo->Name;
			if (System::String::Compare(pPropString, pFieldName, StringComparison::OrdinalIgnoreCase) == 0) 
			{
				if (pFieldInfo->IsInitOnly || pFieldInfo->IsLiteral)
					throw RuntimeError(_M("Attempt to set read-only property: "), prop);
				System::Object ^ pVal = MXS_dotNet::MXS_Value_To_Object(val, pFieldInfo->FieldType);
				pFieldInfo->SetValue(pObject, pVal);
				found = true;
				break;
			}
		}
		if (!found)
		{
			array<PropertyInfo^>^ props = type->GetProperties(flags);
			PropertyInfo^ pPropertyInfo;
			for(int m=0; m<props->Length; m++)
			{
				pPropertyInfo = props[m];
				System::String ^ pPropName = pPropertyInfo->Name;
				if (System::String::Compare(pPropString, pPropName, StringComparison::OrdinalIgnoreCase) == 0) 
					break;
				pPropertyInfo = nullptr;
			}
			if (pPropertyInfo) 
			{
				found = true;
				MethodInfo ^accessor = pPropertyInfo->GetSetMethod();
				if (accessor) 
				{
					// if accessor has more than 1 argument, pass back a dotNetMethod
					array<ParameterInfo^> ^params = accessor->GetParameters();
					if (params->Length != 1)
						return DotNetMethod::intern(accessor->Name, this, type);

					System::Object ^ pVal = MXS_dotNet::MXS_Value_To_Object(val, pPropertyInfo->PropertyType);
					array<System::Object^>^arguments = gcnew array<System::Object^>{pVal};
					accessor->Invoke(pObject,arguments);
				}
				else
					throw RuntimeError(_M("Attempt to set read-only property: "), prop);
			}
		}

		return found ? val : NULL;
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

void DotNetObjectWrapper::get_property_names(Array* propNames, bool showStaticOnly, bool declaredOnTypeOnly)
{
	try
	{
		System::Type ^ type = GetType();
		if (!type) 
			return;
		MXS_dotNet::GetPropertyAndFieldNames(type, propNames, showStaticOnly, declaredOnTypeOnly);
		return;
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

bool DotNetObjectWrapper::show_properties(MCHAR* pattern, CharStream* out, 
										  bool showStaticOnly, bool showMethods, bool showAttributes, bool declaredOnTypeOnly)
{
	try
	{
		System::Type ^ type = GetType();
		if (!type) 
			return false;

		System::Text::StringBuilder ^sb;
		if (out)
		{
			sb = gcnew System::Text::StringBuilder();
			bool res = ShowPropertyInfo(type, pattern, sb, showStaticOnly, showMethods, showAttributes, declaredOnTypeOnly);
			out->puts(MNETStr::ToMSTR(sb->ToString()));
			return res;
		}
		else
			return ShowPropertyInfo(type, pattern, sb, showStaticOnly, showMethods, showAttributes, declaredOnTypeOnly);
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

bool DotNetObjectWrapper::show_methods(MCHAR* pattern, CharStream* out, 
									   bool showStaticOnly, bool showSpecial, bool showAttributes, bool declaredOnTypeOnly)
{
	try
	{
		System::Type ^ type = GetType();
		if (!type) 
			return false;

		System::Text::StringBuilder ^sb;
		if (out)
		{
			sb = gcnew System::Text::StringBuilder();
			bool res = ShowMethodInfo(type, pattern, sb, showStaticOnly, showSpecial, showAttributes, declaredOnTypeOnly);
			out->puts(MNETStr::ToMSTR(sb->ToString()));
			return res;
		}
		return ShowMethodInfo(type, pattern, sb, showStaticOnly, showSpecial, showAttributes, declaredOnTypeOnly);
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

bool DotNetObjectWrapper::show_events(MCHAR* pattern, CharStream* out, bool showStaticOnly, bool declaredOnTypeOnly)
{
	try
	{
		System::Type ^ type = GetType();
		if (!type) 
			return false;

		System::Text::StringBuilder ^sb;
		if (out)
		{
			sb = gcnew System::Text::StringBuilder();
			bool res = ShowEventInfo(type, pattern, sb, showStaticOnly, declaredOnTypeOnly);
			out->puts(MNETStr::ToMSTR(sb->ToString()));
			return res;
		}
		return ShowEventInfo(type, pattern, sb, showStaticOnly, declaredOnTypeOnly);
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

bool DotNetObjectWrapper::show_fields(MCHAR* pattern, CharStream* out, bool showStaticOnly, bool showAttributes, bool declaredOnTypeOnly)
{
	try
	{
		System::Type ^ type = GetType();
		if (!type) 
			return false;

		System::Text::StringBuilder ^sb;
		if (out)
		{
			sb = gcnew System::Text::StringBuilder();
			bool res = ShowFieldInfo(type, pattern, sb, showStaticOnly, showAttributes, declaredOnTypeOnly);
			out->puts(MNETStr::ToMSTR(sb->ToString()));
			return res;
		}
		return ShowFieldInfo(type, pattern, sb, showStaticOnly, showAttributes, declaredOnTypeOnly);
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

/*
// see comments in utils.cpp
bool DotNetObjectWrapper::show_interfaces(MCHAR* pattern, CharStream* out)
{
	try
	{
		System::Type ^ type = GetType();
		if (!type) 
			return false;

		System::Text::StringBuilder ^sb;
		if (out)
		{
			sb = gcnew System::Text::StringBuilder();
			bool res = ShowInterfaceInfo(type, pattern, sb);
			out->puts(MNETStr::ToMSTR(sb->ToString()));
			return res;
		}
		return ShowInterfaceInfo(type, pattern, sb);
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

void DotNetObjectWrapper::get_interfaces(Array* result)
{
	try
	{
		System::Type ^ type = GetType();
		if (!type) 
			return;

		GetInterfaces(type, result);
		return;
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

bool DotNetObjectWrapper::show_attributes(MCHAR* pattern, CharStream* out, bool declaredOnTypeOnly)
{
	try
	{
		System::Type ^ type = GetType();
		if (!type) 
			return false;

		System::Text::StringBuilder ^sb;
		if (out)
		{
			sb = gcnew System::Text::StringBuilder();
			bool res = ShowAttributeInfo(type, pattern, sb, declaredOnTypeOnly);
			out->puts(MNETStr::ToMSTR(sb->ToString()));
			return res;
		}
		return ShowAttributeInfo(type, pattern, sb, declaredOnTypeOnly);
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

void DotNetObjectWrapper::get_attributes(Array* result)
{
	try
	{
		System::Type ^ type = GetType();
		if (!type) 
			return;

		GetAttributes(type, result);
		return;
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
*/

bool DotNetObjectWrapper::show_constructors(CharStream* out)
{
	try
	{
		System::Object ^ pObject = GetObject();
		System::Type ^ type = GetType();
		if (!type) 
			return false;

		System::Text::StringBuilder ^sb;
		// if object is a type, show constructors for the type. Otherwise get the type of the object
		// and get the constructors for  that type
		if (pObject)
		{
			System::Type ^ pType = dynamic_cast<System::Type ^>(pObject);
			if (pType)
				type = pType;
		}

		if (out)
		{
			sb = gcnew System::Text::StringBuilder();
			bool res = ShowConstructorInfo(type, sb);
			out->puts(MNETStr::ToMSTR(sb->ToString()));
			return res;
		}
		return ShowConstructorInfo(type, sb);
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

void DotNetObjectWrapper::CreateEventHandlerDelegates(System::Object ^ targetWrapper, 
													  System::Reflection::MethodInfo ^mi)
{
	using namespace System;
	using namespace System::Reflection;
	using namespace System::Reflection::Emit;

	try
	{
		System::Object ^ target = GetObject();
		Type ^ targetType = GetType();

		if (!targetWrapper || !mi || !targetType) 
		{
			DbgAssert(false);
			return;
		}

		Type ^ targetWrapperType = targetWrapper->GetType();
		Type ^ methodReturnType = mi->ReturnType;

		BindingFlags flags = (BindingFlags)( BindingFlags::Static | 
			BindingFlags::Public | BindingFlags::FlattenHierarchy);
		if (target)
			flags = (BindingFlags)(flags | BindingFlags::Instance );

		array<EventInfo^> ^events = targetType->GetEvents(flags);
		for (int i = 0; i < events->Length; i++)
		{
			EventInfo ^pEventInfo = events[i];
			Type ^tDelegate = pEventInfo->EventHandlerType;

			// To construct an event handler, you need the return type and parameter types of the delegate. 
			// These are obtained by examining the delegate's Invoke method. 
			// When building the typeParameters array, we add 1 additional member as element 0 which contains
			// delegate_proxy_type::typeid. This is because we are binding the delegate to
			// m_delegate_map_proxy.get_proxy(this) (see the handler->CreateDelegate call)
			Type ^ delegateReturnType = tDelegate->GetMethod("Invoke")->ReturnType;
			array<ParameterInfo^> ^parameters = tDelegate->GetMethod("Invoke")->GetParameters();
			array<Type^> ^typeParameters = gcnew array<Type^>(parameters->Length+1);
			typeParameters[0] = targetWrapperType;
			for (int i = 0; i < parameters->Length; i++)
				typeParameters[i+1] = parameters[i]->ParameterType;

			// Create the DynamicMethod instance. Doesn't really need a name, using event name 
			// From the docs on DynamicMethod:
			//		The last argument associates the dynamic method with the proxy type, giving the delegate
			//		access to all the public and private members of the proxy, as if it were an instance method.
			// I really don't have a clue as to exactly what that means.
			DynamicMethod ^ handler = 
				gcnew DynamicMethod(pEventInfo->Name, 
				delegateReturnType,
				typeParameters,
				targetWrapperType);

			ILGenerator ^ eventIL  = handler->GetILGenerator();

			// Generate a method body. This method calls a non-static method on the proxy. The proxy instance
			// is the first arg to the dynamic method (since the delegate is bound to it).  Load this as the first 
			// item on the stack so OpCodes::Callvirt calls the method on the instance.
			eventIL->Emit(OpCodes::Ldarg_0);

			// the ..., pops the 
			// return value off the stack (because the handler has no
			// return type), and returns.

			// first arg to the proxy method is the event name.
			eventIL->Emit(OpCodes::Ldstr, pEventInfo->Name);

			// second arg to the proxy method is an array containing the delegate's invoke parameters 
			LocalBuilder ^ local = eventIL->DeclareLocal(array<System::Object^>::typeid);
			eventIL->Emit(OpCodes::Ldc_I4, parameters->Length);
			eventIL->Emit(OpCodes::Newarr, System::Object::typeid);
			eventIL->Emit(OpCodes::Stloc, local);

			for (int i = 0; i < parameters->Length; i++)
			{
				eventIL->Emit(OpCodes::Ldloc, local);
				eventIL->Emit(OpCodes::Ldc_I4, i);
				eventIL->Emit(OpCodes::Ldarg, i+1); // start at 2nd arg since first is the proxy
				eventIL->Emit(OpCodes::Stelem_Ref);
			}
			eventIL->Emit(OpCodes::Ldloc, local);

			// call the virtual method
			eventIL->Emit(OpCodes::Callvirt, mi);

			// if proxy method returns a void (which it always will here), 
			if (methodReturnType == System::Void::typeid)
			{
				// if the delegate expects the method to return void, we 
				// don't need to do anything. Otherwise, pass back a null reference
				if (delegateReturnType != System::Void::typeid)
					eventIL->Emit(OpCodes::Ldnull);
			}
			else
			{
				// if the delegate expects the method to return void, pop 
				// the return value from the proxy method. 
				if (delegateReturnType == System::Void::typeid)
					eventIL->Emit(OpCodes::Pop);
				else
					eventIL->Emit(OpCodes::Castclass, delegateReturnType);
			}
			eventIL->Emit(OpCodes::Ret);

			// Complete the dynamic method by calling its CreateDelegate method. Bind the delegate to the proxy.
			Delegate ^ dEmitted = handler->CreateDelegate(tDelegate, targetWrapper);

			try
			{
				// And add the delegate to the invocation list for the event.
				pEventInfo->AddEventHandler(target, dEmitted);

				// and store the event/delegate so we can remove them later
				StoreEventDelegatePair(gcnew event_delegate_pair(pEventInfo->Name, dEmitted));
			}
			catch (System::Exception ^ e)
			{
				// currently don't know of a way to detect events that are defined on a base class,
				// but are not supported by the derived class we are working with.
				// So we just catch this exception, and ignore it. All other exceptions we pass along
				if (e->InnerException->GetType() != System::NotSupportedException::typeid)
					throw;
			}
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

void DotNetObjectWrapper::RunEventHandlers(Value* target, Value* eventName, Array* handlers, array<System::Object^> ^delegateArgsArray)
{
	Value**		arg_list;
	// make the event handler argument list
	int dArgs = delegateArgsArray->Length;
	DbgAssert (dArgs == 2); // should always be sender, EventArgs
	value_local_array(arg_list, dArgs);
	for (int j = 0; j < dArgs; j++)
		arg_list[j] = DotNetObjectWrapper::intern(delegateArgsArray[j]);
	for (int i = 0; i < handlers->size; i++)
	{
		// call the event handler
		Value *handler = (*handlers)[i]->eval_no_wrapper();
		if (handler->tag == class_tag(MAXScriptFunction))
		{
			save_current_frames();
			trace_back_active = FALSE;
//			macroRecorder->Disable();
			try 
			{
				int cArgs = ((MAXScriptFunction*)handler)->parameter_count;
				// if only 1 arg is specified for the event handler, we want to pass the
				// EventArgs as the arg. This matches the behavior of dotNetControl event
				// handlers, where typically we only send the eventArgs, but if 2 args are
				// specified we pass the sender as the first arg.
				if (cArgs == 1 && dArgs == 2)
					handler->apply(&arg_list[1], 1);
				else
					handler->apply(arg_list, min(cArgs,dArgs));
				if (thread_local(redraw_mode) && thread_local(needs_redraw))
				{
					if (thread_local(needs_redraw) == 2)
						MAXScript_interface->ForceCompleteRedraw(FALSE);
					else
						MAXScript_interface->RedrawViews(MAXScript_interface->GetTime());
					needs_redraw_clear();
				}
			}
			catch (MAXScriptException& e)
			{
				if (progress_bar_up)
					MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
				restore_current_frames();
				MAXScript_signals = 0;
				if (!thread_local(try_mode))
				{
					show_source_pos();
					error_message_box(e, _M("dotNet event handler"));
				}
				else
				{
					clear_error_source_data();
				}
				if (thread_local(try_mode))
				{
					macroRecorder->Enable();
					pop_alloc_frame();
					throw;
				}
			}
			catch (...)
			{
				if (progress_bar_up)
					MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
				restore_current_frames();
				if ( !thread_local(try_mode))
				{
					show_source_pos();
					error_message_box(UnknownSystemException (), _M("dotNet event handler"));
				}
				else
				{
					clear_error_source_data();
				}
				if (thread_local(try_mode))
				{
					macroRecorder->Enable();
					pop_alloc_frame();
					throw;
				}
			}
			macroRecorder->Enable();
		}
	}
	pop_value_local_array(arg_list);
}

bool DotNetObjectWrapper::HasEvent(MCHAR *eventName)
{
	using namespace System;
	using namespace System::Reflection;
	CString cEventNameString (eventName);
	System::String ^ l_pEventNameString = gcnew System::String(cEventNameString);

	BindingFlags flags = (BindingFlags)( BindingFlags::Static | BindingFlags::Instance | 
		BindingFlags::Public | BindingFlags::FlattenHierarchy);
	array<EventInfo^> ^l_pEvents = GetType()->GetEvents(flags);
	for(int e=0; e < l_pEvents->Length; e++)
	{
		EventInfo^ l_pEvent = l_pEvents[e];
		System::String ^ l_pEventName = l_pEvent->Name;
		if (System::String::Compare(l_pEventNameString,l_pEventName, StringComparison::OrdinalIgnoreCase))
			return true;
	}
	return false;
}

// This method will be called when any event is raised on the object
void DotNetObjectWrapper::ProcessEvent(System::String ^eventName, System::Object ^delegateArgs)
{
	try
	{
		array<System::Object^> ^ delegateArgsArray = safe_cast<array<System::Object^> ^>(delegateArgs) ;

		init_thread_locals();
		push_alloc_frame();
		two_value_locals(eventName, handlers);
		vl.eventName = Name::intern(MNETStr::ToMSTR(eventName));
		vl.handlers = GetEventHandlers(vl.eventName);
		// see if an handler exists for this event
		if ( vl.handlers != &undefined)
		{
			Array *handlers = (Array*)vl.handlers;
			RunEventHandlers(GetMXSContainer(), vl.eventName, handlers, delegateArgsArray);
		}
		pop_value_locals();
		pop_alloc_frame();
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

