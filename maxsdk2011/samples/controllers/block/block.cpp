/**********************************************************************
 *<
    FILE: ctrl.cpp

    DESCRIPTION:   DLL implementation of some controllers

    CREATED BY: Rolf Berteig

    HISTORY: created 13 June 1995

	         added independent scale controller (ScaleXYZ)
			 see file "./indescale.cpp"
			   mjm 9.15.98

 *> Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "block.h"
#include "buildver.h"

#define MAX_CONTROLLERS	8
ClassDesc *classDescArray[MAX_CONTROLLERS];
int classDescCount = 0;

void initClassDescArray(void)
{
   if( !classDescCount )
   {
      classDescArray[classDescCount++] = GetMasterBlockDesc();
      classDescArray[classDescCount++] = GetBlockControlDesc();
      classDescArray[classDescCount++] = GetSlaveFloatDesc();
   #ifndef NO_CONTROLLER_SLAVE_POSITION
      classDescArray[classDescCount++] = GetSlavePosDesc();
   #endif
      classDescArray[classDescCount++] = GetControlContainerDesc();
   #ifndef NO_CONTROLLER_SLAVE_ROTATION
      classDescArray[classDescCount++] = GetSlaveRotationDesc();
   #endif
   #ifndef NO_CONTROLLER_SLAVE_SCALE
      classDescArray[classDescCount++] = GetSlaveScaleDesc();
   #endif
      classDescArray[classDescCount++] = GetSlavePoint3Desc();
   }
}

HINSTANCE hInstance;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
    switch(fdwReason) {
       case DLL_PROCESS_ATTACH:
          hInstance = hinstDLL;
          DisableThreadLibraryCalls(hInstance);
          break;
        }
    return(TRUE);
    }


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }


/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int 
LibNumberClasses() { 
   initClassDescArray();

   return classDescCount; 
}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
   initClassDescArray();

	if( i < classDescCount )
		return classDescArray[i];
	else
		return NULL;
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

