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
// AUTHOR: Nicolas Desjardins
// DATE: 2007-07-24
//***************************************************************************/

#pragma unmanaged
#include <windows.h>
#include "SceneExplorerExtensiongup.h"

namespace
{
HINSTANCE dll_hInstance = nullptr;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID) 
{
	if( fdwReason == DLL_PROCESS_ATTACH )
	{
		dll_hInstance = hinstDLL;
		DisableThreadLibraryCalls(dll_hInstance);
	}
	return(TRUE);
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

__declspec(dllexport) int LibNumberClasses()
{
	return 1;
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
	if(0 == i)
	{
		// The plug-in singleton instance manages the ClassDesc's instance as
		// a data member
		return SceneExplorerExtension::SceneExplorerExtensionGup::GetInstance()->GetClassDesc();
	}
	else
	{
		return nullptr;
	}
}

__declspec(dllexport) const TCHAR* LibDescription()
{
	static const TCHAR description[] = _T("Scene Explorer Extension");
	return description;
}

// LibShutdown is required to perform any clean-up before the DLL is unloaded.
// This is especially important for C++/CLI DLLs loaded as native plug-ins
// since any statically allocated objects remaining after LibShutdown will 
// cause the DLL to crash during unload.  Managed object clean-up will make a
// call to the CLR, but the CLR is already shut down by the time the DLL unload
// is performed.
__declspec(dllexport) BOOL LibShutdown()
{
	SceneExplorerExtension::SceneExplorerExtensionGup::Destroy();
	return TRUE;
}

__declspec(dllexport) ULONG CanAutoDefer()
{
	return FALSE;
}
