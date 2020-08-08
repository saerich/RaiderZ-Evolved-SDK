/**********************************************************************
 *<
   FILE:       DllEntry.cpp

   DESCRIPTION:   Contains the Dll Entry stuff

   CREATED BY:    Ravi Karra

   HISTORY:    Created on 5/12/99

 *>   Copyright © 1997, All Rights Reserved.
 **********************************************************************/
#include "MXSCurveCtl.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

extern void cc_init();

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;            // Hang on to this DLL's instance handle.
      DisableThreadLibraryCalls(hInstance);
   }
         
   return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
   return GetString(IDS_LIBDESCRIPTION);
}

__declspec( dllexport ) void LibInit() { 
   // do any setup here
   cc_init();
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

