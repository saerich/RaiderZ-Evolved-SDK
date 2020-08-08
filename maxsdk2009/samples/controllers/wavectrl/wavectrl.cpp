/**********************************************************************
 *<
	FILE: wavectrl.cpp

	DESCRIPTION:   Generic waveform controller

	CREATED BY: Tom Hudson

	HISTORY: created 25 Sept. 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "wavectrl.h"

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
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetFloatWaveDesc();
		default: return 0;
		}
	}


// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

