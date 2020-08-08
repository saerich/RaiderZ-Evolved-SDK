/**********************************************************************
 *<
	FILE: relaxmod.cpp

	DESCRIPTION:   DLL front end for Relax modifier

	CREATED BY: Steve Anderson

	HISTORY: created March 4 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "relaxmod.h"

HINSTANCE hInstance;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;
      DisableThreadLibraryCalls(hInstance);
   }

	return(TRUE);
}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString (IDS_LIBDESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return 1;}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
	case 0: return GetRelaxModDesc();
	default: return 0;
	}
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

TCHAR *GetString (int id) {
	static TCHAR buf[256];
	if (!hInstance) return NULL;
	return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
}
