/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\brief Contains the declaration of the default ITimerBridge implementation used on
	32-bit Windows platforms.
	\ingroup kypathdata_bridge */

#ifndef KYTIMERBRIDGE_WIN32_H
#define KYTIMERBRIDGE_WIN32_H

#include <kypathdata/generic/types.h>
#if defined(KY_WIN32)

#include <kypathdata/bridge/itimerbridge.h>

namespace Kaim
{
	/*! This class is the default implementation of the DefaultTimerBridge created by the Bridge at its
		initialization.
		\pp This class is declared and implemented separately for each platform supported by \SDKName. You can use
		these implementations as-is, or write your own implementation of DefaultTimerBridge and set your own 
		implementation in your Bridge.
		\pp See also :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class DefaultTimerBridge : public Kaim::ITimerBridge
	{
	public:
		/*! \label_baseconstructor */
		DefaultTimerBridge();
		/*! \label_virtualdestructor */
		virtual ~DefaultTimerBridge();

		virtual void Reset();
		virtual KyFloat32 GetTime();

	protected:
		bool HasPerformanceCounter;
		double Resolution;
		__int32 t0_32;
		__int64 t0_64;
	};

} // namespace Kaim


#endif // KY_WIN32
#endif // KYTIMERBRIDGE_WIN32_H
