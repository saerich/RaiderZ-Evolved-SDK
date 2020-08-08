/**********************************************************************
 *<
   FILE: DllEntry.cpp

   DESCRIPTION: Contains the Dll Entry stuff

   CREATED BY: 

   HISTORY: 

 *>   Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#include "boolcntrl.h"

extern ClassDesc2* GetBoolCntrlDesc(); // 1431 rjc:  I took this out

HINSTANCE hInstance;

static BOOL InitBoolCntrlDLL()
{
   static BOOL InitBool=TRUE;

   if( InitBool )
   {
      // CAL-9/17/2002: set default bool control to boolean controller
      SetDefaultBoolController(GetBoolCntrlDesc()); 
     InitBool=FALSE;
    }

   return TRUE;
}

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;            // Hang on to this DLL's instance handle.
      DisableThreadLibraryCalls(hInstance);
   }

   return (TRUE);
}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR* LibDescription()
{
   return GetString(IDS_LIBDESCRIPTION);
}

// This function returns the number of plug-in classes this DLL
//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
   return 1;
}

// This function returns the number of plug-in classes this DLL
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
   InitBoolCntrlDLL();

   switch(i) {
      case 0: return GetBoolCntrlDesc();
      default: return 0;
   }
}

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion()
{
   return VERSION_3DSMAX;
}

// Perform initializations that shouldn't be done in DllMain(), as mentionned in
// the Windows API's DllMain().
__declspec( dllexport ) int LibInitialize(void)
{
   return InitBoolCntrlDLL();
}

TCHAR *GetString(int id)
{
   static TCHAR buf[256];

   if (hInstance)
      return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
   return NULL;
}

