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

#pragma once

#include "DotNetObjectWrapper.h"

/* -------------------- DotNetObjectManaged  ------------------- */
// 
#pragma managed
namespace MXS_dotNet
{
	class dotNetObject;
	class dotNetClass;

	public class DotNetObjectManaged : public DotNetObjectWrapper
	{
	public:
		DotNetObjectManaged(dotNetObject* p_dotNetObject);
		DotNetObjectManaged(System::Object ^ object, dotNetObject* p_dotNetObject);
		~DotNetObjectManaged();

		ref class delegate_proxy_type
		{
			DotNetObjectManaged* m_p_native_target;
			// the object 
			System::Object ^ m_pObject;
			// the owning MXS dotNetObject
			dotNetObject *m_p_dotNetObject;
			// existing wrapped objects
			static System::Collections::Hashtable ^ m_pWrappedObjects;
			// event/delegate pairs for removing the delegates from the events when done with the object
			System::Collections::Generic::List<event_delegate_pair^> ^ m_pEventDelegatePairs;

		public:
			delegate_proxy_type(DotNetObjectManaged* pNativeTarget) : m_p_native_target(pNativeTarget), m_p_dotNetObject(NULL) {}
			void detach();

			// this method is called by a dynamic method attached to a delegate, which is then attached to an event
			// eventName is the event name, delegateArgs is an array containing the parameters passed to the delegate method
			// (sender, EventArgs)
			void ProcessEvent(System::String ^eventName, System::Object ^delegateArgs);

			System::Object ^ get_object();
			void set_object(System::Object ^ object);
			dotNetObject* get_dotNetObject();
			void set_dotNetObject(dotNetObject *_dotNetObject);
			void store_EventDelegatePair(event_delegate_pair ^ pEventDelegatePair);
			static void init();
			static Value* get_MXS_container_for_object(System::Object ^ object);
		};

		msclr::delegate_map::internal::delegate_proxy_factory<DotNetObjectManaged> m_delegate_map_proxy;

		static Value* Create(MCHAR* pTypeString, dotNetClass* p_dotNetClass, Value** arg_list, int count);
		void CreateEventHandlerDelegates();
		static void Init();
		static Value* GetMXSContainerForObject(System::Object ^ object); //  returns dotNetObject if object already wrapped
		bool Compare(DotNetObjectManaged * pCompareVal);
		Value* Copy();

		// DotNetObjectWrapper
		System::Object ^ GetObject();
		Value* GetMXSContainer();
		void StoreEventDelegatePair(event_delegate_pair ^ pEventDelegatePair);
		Value* GetEventHandlers(Value* eventName);
	};
}