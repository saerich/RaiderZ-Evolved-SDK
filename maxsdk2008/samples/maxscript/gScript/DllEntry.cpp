// ============================================================================
// DllEntry.cpp
//
// History:
//  09.12.00 - Created by Simon Feltman
//
// Copyright ©2006, Autodesk
// ----------------------------------------------------------------------------
#include "MAXScrpt.h"
#include "resource.h"

HINSTANCE hInstance;

// ============================================================================
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   switch(fdwReason)
   {
   case DLL_PROCESS_ATTACH:
      hInstance = hinstDLL;
      DisableThreadLibraryCalls(hInstance);
      break;

   case DLL_PROCESS_DETACH:
      break;
   }
   return TRUE;
}

// ============================================================================
__declspec(dllexport) void LibInit()
{
}
__declspec(dllexport) const TCHAR* LibDescription() { return GetString(IDS_LIBDESCRIPTION); }
__declspec(dllexport) ULONG LibVersion() {  return VERSION_3DSMAX; }

TCHAR*
GetString(int id)
{
   static TCHAR buf[256];

   if(hInstance)
      return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
   return NULL;
}
