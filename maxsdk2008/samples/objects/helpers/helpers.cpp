/**********************************************************************
 *<
	FILE: helpers.cpp

	DESCRIPTION:   DLL implementation of primitives

	CREATED BY: Dan Silva

	HISTORY: created 12 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "helpers.h"

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


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }


/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return 2;}

//VIZ will use a different grid helper
__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
//		case 0: return GetVIZGridHelpDesc();
		case 0: return GetGridHelpDesc();
		case 1: return GetPointHelpDesc();
		//case 2: return GetTapeHelpDesc();
		//case 3: return GetLineHelpDesc();
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


TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}
