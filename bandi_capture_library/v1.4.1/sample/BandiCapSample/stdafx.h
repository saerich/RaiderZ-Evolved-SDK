// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"


#pragma warning(disable: 4819)	// The file contains a character that cannot be represented in the current code page (949). Save the file in Unicode format to prevent data loss

#define _CRT_SECURE_NO_WARNINGS			
#define _CRT_NON_CONFORMING_SWPRINTFS

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include <atlhost.h>
#include <atlcoll.h>

#include <assert.h>
#define ASSERT(x) assert(x)

#pragma warning(disable: 4819) // he file contains a character that cannot be represented in the current code page (949). Save the file in Unicode format to prevent data loss
