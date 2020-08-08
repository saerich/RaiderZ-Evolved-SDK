/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERERRORUTILS_H
#define KYDEBUGGERERRORUTILS_H

#include <kypathdata/basesystem/logger.h>
#include <kypathdata/basesystem/basesystem.h>

//#define KAIM_DEBUGGER_PROFILE
//#define KAIM_DEBUGGER_BREAKONERROR

namespace Kaim
{
	namespace Debugger
	{
		struct ProfileTimer
		{
			void Start()
			{
				m_start = Kaim::GetBaseSystem().GetTime();
			}

			void Stop()
			{
				m_end = Kaim::GetBaseSystem().GetTime();
			}

			KyFloat32 GetElapsedTime()
			{
				return m_end - m_start;
			}

			KyFloat32 m_start;
			KyFloat32 m_end;
		};
	}
}

#ifdef KAIM_DEBUGGER_PROFILE

	#define KAIM_DEBUGGER_PROFILE_CREATE(timer) Kaim::Debugger::ProfileTimer timer; const char *s_KY_funcName = "profiling";
	#define KAIM_DEBUGGER_PROFILE_START(timer) timer.Start()
	#define KAIM_DEBUGGER_PROFILE_STOP(timer) timer.Stop()
	#define KAIM_DEBUGGER_PROFILE_PRINT(timer, message) KY_MESSAGE(KY_LOG_MESSAGE, (message##" : (ms) %.2f", (KyFloat32)(timer.GetElapsedTime()*1000)))

#else // KAIM_DEBUGGER_PROFILE

	#define KAIM_DEBUGGER_PROFILE_CREATE(timer) 
	#define KAIM_DEBUGGER_PROFILE_START(timer) 
	#define KAIM_DEBUGGER_PROFILE_STOP(timer) 
	#define KAIM_DEBUGGER_PROFILE_PRINT(timer, message) 

#endif // KAIM_DEBUGGER_PROFILE

#ifdef KAIM_DEBUGGER_BREAKONERROR
	#if (defined(KY_XBOX_ANY) || defined(KY_WIN32))
		#define KAIM_DEBUGGER_BREAK ::__debugbreak()
	#else
		#define KAIM_DEBUGGER_BREAK 
	#endif
#else // KAIM_DEBUGGER_BREAKONERROR
	#define KAIM_DEBUGGER_BREAK
#endif // KAIM_DEBUGGER_BREAKONERROR

// Test expression against KyResult, and forward error if any as returned error code
#ifdef KAIM_DEBUGGER_PROFILE_DETAILS
	#define KAIM_DEBUGGER_FORWARD_ERROR(expression) {KAIM_DEBUGGER_PROFILE_CREATE(_timer); KAIM_DEBUGGER_PROFILE_START(_timer); KyResult error = (expression); KAIM_DEBUGGER_PROFILE_STOP(_timer);KAIM_DEBUGGER_PROFILE_PRINT(_timer, #expression);if (Kaim::Result::Fail(error)) {KAIM_DEBUGGER_BREAK;return error;}}
#else // KAIM_DEBUGGER_PROFILE_DETAILS
	#define KAIM_DEBUGGER_FORWARD_ERROR(expression) {KyResult error = (expression); if (Kaim::Result::Fail(error)) { KY_MESSAGE(Kaim::KY_LOG_ERROR, ("evaluating : "#expression));KAIM_DEBUGGER_BREAK;return error;}}
#endif // KAIM_DEBUGGER_PROFILE_DETAILS

#endif // KYDEBUGGERERRORUTILS_H
