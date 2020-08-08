/*===========================================================================*\
 | 
 |  FILE:   Plugin.cpp
 |       A MAXScript Plugin that adds Morpher modifier access
 |       Created for 3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |       Developer Consulting Group
 |       Copyright(c) Autodesk 2006
 |
 |  HIST:   Started 5-4-99
 | 
\*===========================================================================*/

#include "MAXScrpt.h"
#include "resource.h"

TCHAR *GetString(int id);

HMODULE hInstance = NULL;

BOOL APIENTRY
DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   switch (ul_reason_for_call)
   {
      case DLL_PROCESS_ATTACH:
         // Hang on to this DLL's instance handle.
         hInstance = hModule;
         DisableThreadLibraryCalls(hInstance);
         break;
   }
      
   return(TRUE);
}

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

__declspec( dllexport ) ULONG
LibVersion() {  return VERSION_3DSMAX; }

