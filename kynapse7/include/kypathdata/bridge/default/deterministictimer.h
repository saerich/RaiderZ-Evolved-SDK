/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KY_KYDETERMINISTICTIMER_H
#define KY_KYDETERMINISTICTIMER_H

#include <kypathdata/generic/types.h>
#include <kypathdata/bridge/itimerbridge.h>


namespace Kaim
{
	/*!	This implementation of the ITimerBridge is created by the Bridge at its
		initialization. It is transparently set as the ITimerBridge implementation when the
		BaseSystem is configured to use deterministic time measurement mode.
		\pp This implementation must be incremented manually through calls to the Increase() method. 
		It can be used to replicate behaviors reliably across different platforms and different
		hardware.
		\pp For details, see :USERGUIDE:"Using Deterministic Time Measurement".
		\ingroup kypathdata_bridge */
	class DeterministicTimer : public ITimerBridge
	{
	public:
		/*!	\label_baseconstructor */
		DeterministicTimer(): m_currentTime(0.f) {}

		/*!	Increases the current time by the specified amount. */
		void Increase(KyFloat32 dt) { m_currentTime += dt; }

	public:
		virtual void Reset() { m_currentTime = 0.f; }
		virtual KyFloat32 GetTime() { return m_currentTime; }

	private:
		KyFloat32 m_currentTime;
	};
}

#endif //KY_KYDETERMINISTICTIMER_H
