/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KY_ITIMERBRIDGE_H
#define KY_ITIMERBRIDGE_H

#include "kypathdata/generic/types.h"

namespace Kaim
{
	/*!	Defines an interface used by \SDKName components to retrieve the current time. 
		\pp The Bridge maintains an instance of a class that implements this interface, making the interface available to other
		\SDKName components. By default, the Bridge uses an instance of the DefaultTimerBridge class. You can also write
		your own implementation of ITimerBridge, and assign an instance of your class to the Bridge.
		\pp See :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class ITimerBridge
	{
	public:
		/*! \label_virtualdestructor */
		virtual ~ITimerBridge() {}

	public:
		/*! Resets the current time to zero. */
		virtual void Reset() = 0;

		/*! Retrieves the elapsed time, in seconds, since the last call to Reset(). */
		virtual KyFloat32 GetTime() = 0;
	};

} // namespace Kaim


#endif // KY_ITIMERBRIDGE_H
