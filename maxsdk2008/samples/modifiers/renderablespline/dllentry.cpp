/**********************************************************************
 *<
   FILE: DllEntry.cpp

   DESCRIPTION: Contains the Dll Entry stuff

   CREATED BY: 

   HISTORY: 

 *>   Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/
#include "RendSpline.h"

extern ClassDesc2* GetRendSplineDesc();

HINSTANCE hInstance;

 //ADD_COMMENTS

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;            // Hang on to this DLL's instance handle.
      DisableThreadLibraryCalls(hInstance);
   }

   return (TRUE);
}

 //ADD_COMMENTS 
__declspec( dllexport ) const TCHAR* LibDescription()
{
   return GetString(IDS_LIBDESCRIPTION);
}

 //ADD_COMMENTS 
//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
   return 1;
}

 //ADD_COMMENTS 
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
   switch(i) {
      case 0: return GetRendSplineDesc();
      default: return 0;
   }
}

 //ADD_COMMENTS 
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

