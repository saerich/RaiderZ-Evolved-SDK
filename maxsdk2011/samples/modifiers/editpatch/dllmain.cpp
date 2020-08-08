/**********************************************************************
 *<
	FILE:			dllmain.cpp

	DESCRIPTION:	Edit Patch OSM

	CREATED BY:		Christer Janson

	HISTORY:		Created August 15, 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#include "dllmain.h"

HINSTANCE hInstance;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;				// Hang on to this DLL's instance handle.
      DisableThreadLibraryCalls(hInstance);
   }
	return(TRUE);
	}

__declspec( dllexport ) const TCHAR* LibDescription()
	{
	return GetString(IDS_LIBDESC);
	}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() 
{
#ifdef NO_PATCHES
	return 1;
#else
	return 2;
#endif
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
	{
	switch(i) {
#ifdef NO_PATCHES		
		case 0: return GetEditSplineModDesc();
#else
		case 0: return GetEditSplineModDesc();
		case 1: return GetEditPatchModDesc();
#endif
		default: return 0;
		}
	}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 0;
}

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}
