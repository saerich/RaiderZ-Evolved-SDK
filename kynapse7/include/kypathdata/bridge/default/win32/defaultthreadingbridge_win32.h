/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\brief Contains the declaration of the default IThreadingBridge implementation used on 
	32-bit Windows platforms.
	\ingroup kypathdata_bridge */

#ifndef KYTHREADINGBRIDGE_WIN32_H
#define KYTHREADINGBRIDGE_WIN32_H

#include <kypathdata/generic/types.h>
#if defined(KY_WIN32)

#include <kypathdata/bridge/ithreadingbridge.h>

namespace Kaim
{
	/*!	This class is the default implementation of the IThreadingBridge created by the Bridge at its
		initialization.
		\pp This class is declared and implemented separately for each platform supported by \SDKName. You can use
		these implementations as-is, or write your own implementation of IThreadingBridge and set your own 
		implementation in your Bridge.
		\pp See also :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class DefaultThreadingBridge : public Kaim::IThreadingBridge
	{
	public:
		/*!	\label_baseconstructor */
		DefaultThreadingBridge() {};
		/*!	\label_virtualdestructor */
		virtual ~DefaultThreadingBridge() {};

		virtual Kaim::IThread* InitThread(Kaim::threadfunc func, void* argList);
		virtual void DestroyThread(Kaim::IThread* thread);

		virtual Kaim::IMutex* InitMutex(bool thisThreadHasOwnership);
		virtual void DestroyMutex(Kaim::IMutex* mutex);
	};

} // namespace Kaim

#endif // KY_WIN32
#endif // KYTHREADINGBRIDGE_WIN32_H
