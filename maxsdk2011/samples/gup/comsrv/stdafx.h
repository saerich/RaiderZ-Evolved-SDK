// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__4AD72E64_5A4B_11D2_91CB_0060081C257E__INCLUDED_)
#define AFX_STDAFX_H__4AD72E64_5A4B_11D2_91CB_0060081C257E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN    // Exclude rarely-used stuff from Windows headers

#undef WINVER
#undef _WIN32_WINNT
#undef _WIN32_IE

#define _WIN32_WINNT 0x0500
#define WINVER _WIN32_WINNT
#define _WIN32_IE    0x0500


//Although this project is a dll, our class objects should always treat us like a local server
//i.e. they shouldn't have any effect on the lock count
//atlcom.h uses _USRDLL and _WINDLL to make this distinction
//here we make sure that we get CComObjectNoLock based class factories
#if defined(_WINDLL)
   #if _MSC_VER < 1400  // With VS2005, _WINDLL is defined...
	#  pragma message("Why is _WINDLL defined?")
   #endif
	#undef _WINDLL
#endif
#if defined(_USRDLL)
	#pragma message("Why is _USRDLL defined?")
	#undef _USRDLL
#endif
#define _ATL_APARTMENT_THREADED

#include "..\..\..\include\buildver.h"
#pragma warning( push )
#pragma warning( disable : 4265 )
#include <atlbase.h>
#pragma warning( pop )

//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
class CExeModule : public CComModule
{
public:
	LONG Lock();
	LONG Unlock();
	DWORD dwThreadID;
	HANDLE hEventShutdown;
    HANDLE hMonitorThread;
	void MonitorShutdown();
	bool StartMonitor();
	bool bActivity;
	bool m_bOLEStart;
};
extern CExeModule _Module;
#pragma warning( push )
#pragma warning( disable : 4265 )
#include <atlcom.h>
#pragma warning( pop )




//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4AD72E64_5A4B_11D2_91CB_0060081C257E__INCLUDED)
