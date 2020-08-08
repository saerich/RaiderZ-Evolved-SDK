/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KY_IPROFILERBRIDGE_H
#define KY_IPROFILERBRIDGE_H

#include "kypathdata/generic/types.h"

namespace Kaim
{
	/*!	Defines an interface used by \SDKName components to publish events in the \gameOrSim engine for
		profiling purposes. You can use an implementation of this interface to send performance counters
		to custom-written or third-party performance analysis tools.
		\pp See :USERGUIDE:"Hooking in External Profilers".
		\pp The Bridge can maintain an instance of a class that implements this interface, making the interface available to other
		\SDKName components. Because the implementation of this interface is specific to the \gameOrSim engine and/or third-party
		tools in use, no implementations of this interface are supplied by default with the SDK. You must write
		your own implementation of IPerfCounterBridge, and assign an instance of your class to the Bridge.
		\pp See :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\pp Note that this interface is only invoked by \SDKName components when you link your project against the Metrics build
		of the \SDKName libraries. See also :USERGUIDE:"Libraries".
		\ingroup kypathdata_bridge */
	class IPerfCounterBridge 
	{
	public:
		/*!	\label_virtualdestructor */
		virtual ~IPerfCounterBridge() {}

	public:
		/*! Adds or updates a performance counter.
			\param name			The name of the counter.
			\param value		The value to add to the counter. */
		virtual void AddCounter(const char* name, KyFloat32 value) = 0;		
		
		/*!	Marks the beginning of a profiling event.
			\param name			The name of the event. */
		virtual void BeginEvent(const char* name) = 0;	

		/*!	Marks the end of the last profiling event registered by a call to BeginEvent(). */
		virtual void EndEvent() = 0;	
	};

} // namespace Kaim


#endif // KY_IPROFILERBRIDGE_H
