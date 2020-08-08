/**********************************************************************
 *<
	FILE: dllmain.cpp

	DESCRIPTION:   DLL implementation of primitives

	CREATED BY: Charles Thaeler

        BASED on helpers.cpp

	HISTORY: created 12 February 1996

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "polycnt.h"

extern ClassDesc* GetPolyCounterDesc();

HINSTANCE hInstance;

TCHAR
*GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


/** public functions **/
BOOL WINAPI
DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;				// Hang on to this DLL's instance handle.
      DisableThreadLibraryCalls(hInstance);
   }

	return(TRUE);
}

//------------------------------------------------------
// This is the interface to MAX:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() {
	return GetString(IDS_LIBDESCRIPTION);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
#define NUM_CLASSES 1

__declspec( dllexport ) int LibNumberClasses()
{
	return NUM_CLASSES;
}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i)
{
    switch(i) {
    case 0: return GetPolyCounterDesc();
    default: return 0;
    }
    
}

// Return version so can detect obsolete DLLs -- NOTE THIS IS THE API VERSION NUMBER
//                                               NOT THE VERSION OF THE DLL.
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

