// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__4E380CA3_5DB1_11D2_91CB_0060081C257E__INCLUDED_)
#define AFX_STDAFX_H__4E380CA3_5DB1_11D2_91CB_0060081C257E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#undef WINVER
#undef _WIN32_WINNT
#undef _WIN32_IE

#define _WIN32_WINNT 0x0500
#define WINVER _WIN32_WINNT
#define _WIN32_IE    0x0500

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <atlbase.h>
extern CComModule _Module;
#pragma warning(push)
#pragma warning(disable : 4265)
#include <atlcom.h>
#pragma warning(pop)


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4E380CA3_5DB1_11D2_91CB_0060081C257E__INCLUDED_)
