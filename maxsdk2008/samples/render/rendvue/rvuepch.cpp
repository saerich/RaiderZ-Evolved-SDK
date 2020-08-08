/**********************************************************************
 *<
	FILE: rvuepch.cpp

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "rvuepch.h"
#include "resource.h"


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
// This is the interface to Max:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() { return 1;}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetRendVueDesc();
		default: return 0;
		}
	}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() {
	return VERSION_3DSMAX; 
	}

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];
	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}
