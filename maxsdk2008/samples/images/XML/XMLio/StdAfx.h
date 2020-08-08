// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__EF9B800D_0F08_11D4_A9E7_0060B0F1894E__INCLUDED_)
#define AFX_STDAFX_H__EF9B800D_0F08_11D4_A9E7_0060B0F1894E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

// TODO: reference additional headers your program requires here
#include "max.h"
#include "iparamb2.h" //not in max.h?
#include "resource.h"
extern TCHAR *GetString(int id);
extern ClassDesc2* GetXMLImpDesc();
extern ClassDesc2* GetNewXMLImpDesc();
#include <assert.h>
#include <atlcomcli.h> //we're a client of at the very least the xmldom

//We're a client of several COM components, assume their header are in a public location
#ifdef KAHN_OR_EARLIER //this is old scheme deprecated at Granite
	#include "vizpointer.h" //we're a client of IVIZPointerClient 
	#include "domloader.h"
	#include "xslregistry.h"
	#include "xmldocutil.h"
	//#include "xmlcoercion.h" JH 10/16/01 redundant?
#else
	//the following includes contain inclusion of domloader.h and vizclient.h
	//These files are generated fom the respective .idls which import domload.idl and vizclient.idl
	#include "xmlmtl.h"
	#include "xmlmapmods.h"
#endif



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__EF9B800D_0F08_11D4_A9E7_0060B0F1894E__INCLUDED_)
