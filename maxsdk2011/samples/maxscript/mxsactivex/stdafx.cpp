// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>

//SS 7/12/2002: While I couldn't find anything in MSDN on this file,
// it's contents in VC7 are gone; all it emits is a pragma message
// saying the file is obsolete, so I've removed it from VC7 builds.
#if _MSC_VER < 1300  // Visual Studio .NET
 #include <statreg.cpp>
#endif

#endif

//#include <atlimpl.cpp>
