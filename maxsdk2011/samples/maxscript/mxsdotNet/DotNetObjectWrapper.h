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
// DESCRIPTION: CWinFormsControl.h 
// AUTHOR: Larry.Minton / David Cunningham - created Oct 2, 2006
//***************************************************************************/
//

#pragma once

#include "MaxIncludes.h"

/* -------------------- DotNetObjectWrapper  ------------------- */
// 
#pragma managed

namespace MXS_dotNet
{
	public class DotNetObjectWrapper
	{
	protected:
#ifdef _DEBUG
		static int instanceCounter;
#endif
	public:
		static Value* intern(System::Object ^ object); // wraps existing System::Object, returns dotNetObject or dotNetControl
		static Value* WrapType(System::Type ^ pType); // wraps type, returns dotNetObject
		static Value* WrapType(MCHAR* pTypeString); // Finds and wraps type, returns dotNetObject
		static Value* WrapType(DotNetObjectWrapper* pDotNetObjectWrapper); // Wraps type of DotNetObjectWrapper, returns dotNetObject
		virtual System::Object ^ GetObject() = 0;
		virtual System::Type ^ GetType();
		virtual Value* GetMXSContainer() = 0; //  returns dotNetObject, dotNetControl, or dotNetClass
		// derived classes must implement:
		// returns dotNetObject or dotNetControl if object already wrapped
		// static Value* GetMXSContainerForObject(System::Object ^ object);
		virtual void StoreEventDelegatePair(event_delegate_pair ^ pEventDelegatePair) = 0;
		virtual MSTR GetTypeName();
		virtual Value* GetEventHandlers(Value* eventName) { return &undefined; }
		Value* get_property(Value** arg_list, int count);
		Value* set_property(Value** arg_list, int count);
		void get_property_names(Array* propNames, bool showStaticOnly, bool declaredOnTypeOnly);
		bool show_properties(MCHAR* pattern, CharStream* out, bool showStaticOnly, bool showMethods, bool showAttributes, bool declaredOnTypeOnly);
		bool show_methods(MCHAR* pattern, CharStream* out, bool showStaticOnly, bool showSpecial, bool showAttributes, bool declaredOnTypeOnly);
		bool show_events(MCHAR* pattern, CharStream* out, bool showStaticOnly, bool declaredOnTypeOnly);
		bool show_fields(MCHAR* pattern, CharStream* out, bool showStaticOnly, bool showAttributes, bool declaredOnTypeOnly);
		/*
		// see comments in utils.cpp
		bool show_interfaces(MCHAR* pattern, CharStream* out);
		void get_interfaces(Array* result);
		bool show_attributes(MCHAR* pattern, CharStream* out, bool declaredOnTypeOnly);
		void get_attributes(Array* result);
		*/
		bool show_constructors(CharStream* out);
		void CreateEventHandlerDelegates(System::Object ^ targetWrapper, System::Reflection::MethodInfo ^mi);
		void RunEventHandlers(Value* target, Value* eventName, Array* handlers, array<System::Object^> ^delegateArgsArray);
		bool HasEvent(MCHAR *eventName);
		virtual void SetLifetimeControlledByDotnet(bool lifetime_controlled_by_dotnet) {}
		virtual bool GetLifetimeControlledByDotnet() { return false; }
		virtual bool IsWeakRefObjectAlive() { return false; }
		virtual void ProcessEvent(System::String ^eventName, System::Object ^delegateArgs);
		DotNetObjectWrapper();
		virtual ~DotNetObjectWrapper();
	};
}
