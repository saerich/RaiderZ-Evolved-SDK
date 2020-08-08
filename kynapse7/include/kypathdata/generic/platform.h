/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\brief Contains macros that detect the target platform.
	\ingroup kypathdata_generic */

#ifndef KYPLATFORM_H
#define KYPLATFORM_H

// To Check incoherance with platform's flag.
#undef KY_PLATFORM_FOUND

#if (!defined(KY_WIN32) && !defined(KY_WIN64) && !defined (KY_XBOX) && !defined(KY_XBOX360) && !defined(KY_PS2) && !defined(KY_GCN) && !defined(KY_WII) && !defined(KY_LINUX32) && !defined(KY_LINUX64) && !defined(KY_PSP) && !defined(KY_PS3))

#if (defined (_WIN32) && !defined(_XBOX) && !defined(_WIN64))
#define KY_WIN32
#define KY_PLATFORM_FOUND "WIN32"
#endif

#if defined (_WIN64)
#define KY_WIN64
#define KY_PLATFORM_FOUND "WIN64"
#endif

#if (defined (_XBOX) && _XBOX_VER < 200) 
#define KY_XBOX // first Xbox
#define KY_PLATFORM_FOUND "XBOX"
#endif

#if (defined(_XBOX) && _XBOX_VER >= 200)
#define KY_XBOX360
#ifndef KY_PLATFORM_FOUND
#define KY_PLATFORM_FOUND "Xbox360"
#else
#pragma message (__FILE__ " : Preprocessor definitions conflict detected between platform " KY_PLATFORM_FOUND " and XBox360")
#error "Invalid settings, please check your preprocessor definitions"
#endif
#endif

#if (defined(PS2) || defined(SN_TARGET_PS2))
#define KY_PS2
#ifndef KY_PLATFORM_FOUND
#define KY_PLATFORM_FOUND "PS2"
#else
#error "Invalid settings, please check your preprocessor definitions"
#endif
#endif

#if (defined(GCN) || defined(HW2))
#define KY_GCN
#ifndef KY_PLATFORM_FOUND
#define KY_PLATFORM_FOUND "GCN"
#else
#error "Invalid settings, please check your preprocessor definitions"
#endif
#endif

#if (defined(RVL) || defined(WII))
#define KY_WII
#ifndef KY_PLATFORM_FOUND
#define KY_PLATFORM_FOUND "Wii"
#else
#error "Invalid settings, please check your preprocessor definitions"
#endif
#endif

#if (defined(LINUX))
#define KY_LINUX32
#ifndef KY_PLATFORM_FOUND
#define KY_PLATFORM_FOUND "Linux"
#else
#error "Invalid settings, please check your preprocessor definitions"
#endif
#endif

#if (defined(LINUX64))
#define KY_LINUX64
#ifndef KY_PLATFORM_FOUND
#define KY_PLATFORM_FOUND "Linux64"
#else
#error "Invalid settings, please check your preprocessor definitions"
#endif
#endif

#if (defined(PSP) || defined(SN_TARGET_PSP_HW) || defined(SN_TARGET_PSP_PRX))
#define KY_PSP
#ifndef KY_PLATFORM_FOUND
#define KY_PLATFORM_FOUND "PSP"
#else
#error "Invalid settings, please check your preprocessor definitions"
#endif
#endif

#if (defined(PS3) || defined(SN_TARGET_PS3))
#define KY_PS3
#ifndef KY_PLATFORM_FOUND
#define KY_PLATFORM_FOUND "PS3"
#else
#error "Invalid settings, please check your preprocessor definitions"
#endif
#endif

#else // One or more Platforms defined externally.

#if defined (KY_WIN32)
#if (defined (KY_WIN64) || defined (KY_XBOX) || defined(KY_XBOX360) || defined(KY_PS2) || defined(KY_GCN) || defined(KY_WII) || defined(KY_LINUX32) || defined(KY_LINUX64) || defined(KY_PSP) || defined(KY_PS3))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "WIN32"
#endif

#if defined (KY_WIN64)
#if (defined (KY_WIN32) || defined (KY_XBOX) || defined(KY_XBOX360) || defined(KY_PS2) || defined(KY_GCN) || defined(KY_WII) || defined(KY_LINUX32) || defined(KY_LINUX64) || defined(KY_PSP) || defined(KY_PS3))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "WIN64"
#endif

#if defined (KY_XBOX)
#if (defined (KY_WIN32) || defined(KY_WIN64) || defined(KY_XBOX360) || defined(KY_PS2) || defined(KY_GCN) || defined(KY_WII) || defined(KY_LINUX32) || defined(KY_LINUX64) || defined(KY_PSP) || defined(KY_PS3))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "XBOX"
#endif

#if defined (KY_XBOX360)
#if (defined (KY_WIN32) || defined(KY_WIN64) || defined(KY_XBOX) || defined(KY_PS2) || defined(KY_GCN) || defined(KY_WII) || defined(KY_LINUX32) || defined(KY_LINUX64) || defined(KY_PSP) || defined(KY_PS3))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "Xbox360"
#endif

#if defined (KY_PS2)
#if (defined (KY_WIN32) || defined(KY_WIN64) || defined(KY_XBOX) || defined(KY_XBOX360) || defined(KY_GCN) || defined(KY_WII) || defined(KY_LINUX32) || defined(KY_LINUX64) || defined(KY_PSP) || defined(KY_PS3))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "PS2"
#endif

#if defined (KY_GCN)
#if (defined (KY_WIN32) || defined(KY_WIN64) || defined(KY_XBOX) || defined(KY_XBOX360) || defined(KY_PS2) || defined(KY_WII) || defined(KY_LINUX32) || defined(KY_LINUX64) || defined(KY_PSP) || defined(KY_PS3))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "GCN"
#endif

#if defined (KY_WII)
#if (defined (KY_WIN32) || defined(KY_WIN64) || defined(KY_XBOX) || defined(KY_XBOX360) || defined(KY_PS2) || defined(KY_GCN) || defined(KY_LINUX32) || defined(KY_LINUX64) || defined(KY_PSP) || defined(KY_PS3))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "Wii"
#endif

#if defined (KY_LINUX32)
#if (defined (KY_WIN32) || defined(KY_WIN64) || defined(KY_XBOX) || defined(KY_XBOX360) || defined(KY_PS2) || defined(KY_GCN) || defined(KY_LINUX64) || defined(KY_WII) || defined(KY_PSP) || defined(KY_PS3))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "Linux"
#endif

#if defined (KY_LINUX64)
#if (defined (KY_WIN32) || defined(KY_WIN64) || defined(KY_XBOX) || defined(KY_XBOX360) || defined(KY_PS2) || defined(KY_GCN) || defined(KY_LINUX32) || defined(KY_WII) || defined(KY_PSP) || defined(KY_PS3))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "Linux64"
#endif

#if defined (KY_PSP)
#if (defined (KY_WIN32) || defined(KY_WIN64) || defined(KY_XBOX) || defined(KY_XBOX360) || defined(KY_PS2) || defined(KY_GCN) || defined(KY_WII) || defined(KY_LINUX32) || defined(KY_LINUX64) || defined(KY_PS3))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "PSP"
#endif

#if defined (KY_PS3)
#if (defined (KY_WIN32) || defined(KY_WIN64) || defined(KY_XBOX) || defined(KY_XBOX360) || defined(KY_PS2) || defined(KY_GCN) || defined(KY_WII) || defined(KY_LINUX32) || defined(KY_LINUX64) || defined(KY_PSP))
#error "More than one Platform defined, please check your preprocessor definitions"
#endif
#define KY_PLATFORM_FOUND "PS3"
#endif

#endif

#if !defined(KY_PLATFORM_FOUND)
#error "No platform has been detected : Unknown Target !!"
#else
#undef KY_PLATFORM_FOUND
#endif

// define KY_XBOX_ANY if any version of the Xbox is the current platform
#if defined (KY_XBOX) || defined (KY_XBOX360)
#define KY_XBOX_ANY
#endif

// define KY_WINDOWS_ANY  if any version of Windows is the current platform
#if defined (KY_WIN32) || defined (KY_WIN64)
#define KY_WINDOWS_ANY
#endif

// define KY_LINUX_ANY  if any version of Linux is the current platform
#if defined (KY_LINUX32) || defined (KY_LINUX64)
#define KY_LINUX_ANY
#endif

// define KY_64BIT_PLATFORM  if the current platform is 64 bits
#if defined (KY_WIN64) || defined (KY_LINUX64)
#define KY_64BIT_PLATFORM 
#endif


// Variants

#if defined (KYDEBUG) && !defined(__SPU__)
#endif // KYDEBUG

#if (defined(KYMETRICS) && !defined(__SPU__))
	#define KY_MEMORY_LOG_ENABLED
	#define KY_PROFILE_ENABLED	// low level profiling
	#define KY_PERF_COUNTERS_ENABLED // high level profiling
#endif // KYMETRICS || KYDEBUG

#if defined (KY_MEMORY_LOG_ENABLED)
	//#define KY_CALLSTACK_AUTO
#endif // KY_MEMORY_LOG_ENABLED

#endif /* KYPLATFORM_H */
