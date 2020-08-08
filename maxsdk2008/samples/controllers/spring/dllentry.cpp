/**********************************************************************
 *<
	FILE: DllEntry.cpp

	DESCRIPTION: Contains the Dll Entry stuff for the Spring controller

	CREATED BY:  Adam Felt

	HISTORY: 

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "jiggle.h"

extern ClassDesc* GetPosJiggleDesc();
extern ClassDesc* GetPoint3JiggleDesc();

HINSTANCE hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
		hInstance = hinstDLL;				// Hang on to this DLL's instance handle.
      DisableThreadLibraryCalls(hInstance);
	}
			
	return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}

//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
	return 2;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetPosJiggleDesc();
		case 1: return GetPoint3JiggleDesc();
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

