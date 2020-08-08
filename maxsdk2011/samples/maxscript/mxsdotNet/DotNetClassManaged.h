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
//***************************************************************************/
//

#pragma once

#include "DotNetObjectWrapper.h"

/* -------------------- DotNetClassManaged  ------------------- */
// 
#pragma managed

namespace MXS_dotNet	{

	class dotNetClass;

	public class DotNetClassManaged : public DotNetObjectWrapper
	{
	public:
		DotNetClassManaged(System::Type ^ p_type, dotNetClass* p_dotNetClass);
		~DotNetClassManaged();

		ref class delegate_proxy_type
		{
			DotNetClassManaged* m_p_native_target;
			// the object when held as a strong reference
			System::Type ^ m_pType;
			// the object when held as a weak reference
			System::WeakReference ^ m_pType_weakref;
			// the owning MXS dotNetClass
			dotNetClass *m_p_dotNetClass;
			// existing wrapped objects
			static System::Collections::Hashtable ^ m_pWrappedClasses;
			// event/delegate pairs for removing the delegates from the events when done with the object
			System::Collections::Generic::List<event_delegate_pair^> ^ m_pEventDelegatePairs;

		public:
			delegate_proxy_type(DotNetClassManaged* pNativeTarget) : m_p_native_target(pNativeTarget), m_p_dotNetClass(NULL) {}
			void detach();

			// this method is called by a dynamic method attached to a delegate, which is then attached to an event
			// eventName is the event name, delegateArgs is an array containing the parameters passed to the delegate method
			// (sender, EventArgs)
			void ProcessEvent(System::String ^eventName, System::Object ^delegateArgs);

			System::Type ^ get_type();
			void set_type(System::Type ^ type);
			void set_type_as_weakref(System::Type ^ type);
			bool has_weakref_object();
			bool is_weakref_object_alive();
			dotNetClass* get_dotNetClass() { return m_p_dotNetClass; }
			void set_dotNetClass(dotNetClass *_dotNetClass) { m_p_dotNetClass = _dotNetClass; }
			void store_EventDelegatePair(event_delegate_pair ^ pEventDelegatePair);
			static void init();
			static Value* get_MXS_container_for_type(System::Type ^ type);
		};

		msclr::delegate_map::internal::delegate_proxy_factory<DotNetClassManaged> m_delegate_map_proxy;

		static void Init();
		static Value* Create(MCHAR* pTypeString);
		static Value* Create(DotNetObjectWrapper* wrapper);
		static Value* Create(System::Type ^ type);
		void CreateEventHandlerDelegates();
		static Value* GetMXSContainerForObject(System::Object ^ object); //  returns dotNetClass if object already wrapped

		// DotNetObjectWrapper
		System::Object ^ GetObject();
		System::Type ^ GetType();
		MSTR GetTypeName();
		Value* GetMXSContainer();
		void StoreEventDelegatePair(event_delegate_pair ^ pEventDelegatePair);
		Value* GetEventHandlers(Value* eventName);
		void SetLifetimeControlledByDotnet(bool lifetime_controlled_by_dotnet);
		bool GetLifetimeControlledByDotnet();
		bool IsWeakRefObjectAlive();
	};
}
