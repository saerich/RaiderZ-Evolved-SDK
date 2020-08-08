/**********************************************************************
 *<
   FILE: NURBS.cpp

   DESCRIPTION: A Test harness utility plugin for the NURBS API

   CREATED BY: Charlie Thaeler

   HISTORY: created 8/13/97

 *>   Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "nutil.h"


HINSTANCE hInstance;

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

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved) 
{
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
LibDescription() { return GetString(IDS_LIB_DESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN NEW CLASS ARE ADD
__declspec( dllexport ) int LibNumberClasses()
{
#ifdef _DEBUG
    return 2;
#else
    return 1;
#endif
}





__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) 
{
    switch(i) {
    case 0: return GetSurfApproxUtilDesc();  // Batch Surface Approximation changer
#ifdef _DEBUG
   case 1: return GetAPITestUtilDesc();   // API Test Class
#endif
    default: return 0;
    }
    
}
