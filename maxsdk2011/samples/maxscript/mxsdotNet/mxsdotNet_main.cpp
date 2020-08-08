//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: mxsdotNet_main.cpp : Defines the load/unload routines for the DLL.
// AUTHOR: Stephane.Rouleau - created Jan.1.2006
//***************************************************************************/
// 

#include "stdafx.h"
#include "assert1.h"

// 
#pragma unmanaged
#include "MAXScrpt.h"
#include "resource.h"

extern void DotNetControl_init();

HMODULE hInstance = NULL;

__declspec( dllexport ) const MCHAR *
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }

MCHAR *GetString(int id)
{
	static MCHAR buf[256];

	if (hInstance)
		return LoadStringA(hInstance, id, buf, sizeof(buf) / sizeof(buf[0])) ? buf : NULL;
	return NULL;
}

TCHAR *GetTString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf) / sizeof(buf[0])) ? buf : NULL;
	return NULL;
}

// 
#pragma managed

// Doing the Afx stuff here because that will force the static ctors to be run
// LibVersion is called right after the plugin is loaded.
__declspec( dllexport ) ULONG
LibVersion() 
{
#ifdef _AFXDLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	return VERSION_3DSMAX; 
}

__declspec( dllexport ) void
LibInit() { 
#ifdef _AFXDLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// do any setup here
	DotNetControl_init();
}

__declspec( dllexport )
int LibShutdown()
{
#ifdef _AFXDLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// Do it before DllMain is called -- this frees up the temporary CWnd objects.
	AfxLockTempMaps();
	AfxUnlockTempMaps(-1);

	return TRUE;
}


