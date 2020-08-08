/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef KY_MACROS_H
#define KY_MACROS_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// KY_CONCATENATE
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define KY_CONCATENATE(arg1, arg2)   KY_CONCATENATE1(arg1, arg2)
#define KY_CONCATENATE1(arg1, arg2)  KY_CONCATENATE2(arg1, arg2)
#define KY_CONCATENATE2(arg1, arg2)  arg1##arg2

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Pragma messages
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Pragma TODO macros
#ifndef	KY_QUOTE
/*	For internal use. */
#define	KY__QUOTE(x) #x
/*	For internal use. */
#define	KY_QUOTE(x) KY__QUOTE(x)
#endif //QUOTE

#ifndef KY___FILE_LINE__
/*	For internal use. */
# define	KY___FILE_LINE__ __FILE__ "(" KY_QUOTE(__LINE__) ") : "
#endif //KY___FILE_LINE__

#ifndef KY_PRAGMA
# if !defined(__GNUC__) && !defined(__SNC__) && !defined(__GCCXML__) && !defined(KY_WII)
#  define KY_PRAGMA __pragma 
# elif defined(__SNC__) || defined (KY_WII) || (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ > 40400) // gcc version > 4.4.0 required to support pragma message
#  define KY_PRAGMA(x) _Pragma(#x)
# else
#  define KY_PRAGMA(x) 
# endif
#endif

#ifndef KY_TODO
/*	For internal use. */
# define KY_TODO(x) KY_PRAGMA(message(KY___FILE_LINE__"[KY_TODO] : " #x))
#endif //KY_TODO

#ifndef KY_COMPILE_TIME_MESSAGE
/*	For internal use. */
# define KY_COMPILE_TIME_MESSAGE( x ) KY_PRAGMA(message( KY___FILE_LINE__"[KY_COMPILE_TIME_MESSAGE]: " #x))
#endif //KY_COMPILE_TIME_MESSAGE

#ifndef KY_TEMPORARY_CODE
/*	For internal use. */
# define KY_TEMPORARY_CODE KY_PRAGMA(message( KY___FILE_LINE__"[KY_TEMPORARY_CODE] ########### !! TEMPORARY CODE TO BE REMOVED !! ###########"))
#endif //KY_TEMPORARY_CODE





///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	force inline
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(KYDEBUG)
	# define KY_FORCE_INLINE inline
#else
	#if defined(KY_XBOX360) || defined(KY_WIN32) || defined(KY_WIN64)
		# define KY_FORCE_INLINE __forceinline
	#elif defined(KY_PS3) || defined(KY_LINUX32) || defined(KY_LINUX64)// PS3 and Linux
		# ifndef _DEBUG
			#  define KY_FORCE_INLINE inline __attribute__((__always_inline__))
		# else
			#  define KY_FORCE_INLINE inline
		# endif
	# else
		# define KY_FORCE_INLINE inline
	#endif
#endif

#endif //KY_MACROS_H

