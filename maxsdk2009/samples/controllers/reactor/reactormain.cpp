/**********************************************************************
 *<
   FILE: DllMain.cpp

   DESCRIPTION: DllMain is in here

   CREATED BY: Adam Felt

   HISTORY: 

 *>   Copyright (c) 1998-1999 Adam Felt, All Rights Reserved.
 **********************************************************************/

#include "reactionMgr.h"

HINSTANCE hInstance;

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

//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
   return 8;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
   switch(i) {
      case 0: return GetFloatReactorDesc();
      case 1: return GetPositionReactorDesc();
      case 2: return GetPoint3ReactorDesc();
      case 3: return GetRotationReactorDesc();
      case 4: return GetScaleReactorDesc();
      case 5: return GetReactionManagerDesc();
      case 6: return GetReactionSetDesc();
      case 7: return GetReactionMasterDesc();
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

