/**********************************************************************
 *<
   FILE: BasicOps.cpp

   DESCRIPTION:   DLL implementation of modifiers

   CREATED BY: Rolf Berteig (based on prim.cpp)

   HISTORY: created 30 January 1995

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "BasicOps.h"
#include "buildver.h"

#include "3dsmaxport.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

// russom - 05/24/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

// SCA note - 8/30/01
// Copied to much simpler BasicOps project from Mods project.  Retaining in case
// this module gets complicated.

#define MAX_MOD_OBJECTS 20
ClassDesc *classDescArray[MAX_MOD_OBJECTS];
int classDescCount = 0;

void initClassDescArray(void) {
   if( !classDescCount )
   {
      //classDescArray[classDescCount++] = GetFaceExtrudeModDesc();
      //classDescArray[classDescCount++] = GetVertexChamferModDesc();
      //classDescArray[classDescCount++] = GetEdgeChamferModDesc();
      #ifdef VERTEX_WELD
      classDescArray[classDescCount++] = GetVertexWeldModDesc();
      #endif
      classDescArray[classDescCount++] = GetSymmetryModDesc();
      //classDescArray[classDescCount++] = GetEditPolyModDesc ();
   }
}

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

__declspec( dllexport ) const TCHAR * LibDescription() {
   return GetString(IDS_LIBDESCRIPTION);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {
   initClassDescArray();

   return classDescCount;
}

// russom - 05/07/01 - changed to use classDescArray
__declspec( dllexport ) ClassDesc* LibClassDesc(int i) {
   initClassDescArray();

   if( i < classDescCount )
      return classDescArray[i];
   else
      return NULL;
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer() {
   return 1;
}

INT_PTR CALLBACK DefaultSOTProc (HWND hWnd,
                         UINT msg,WPARAM wParam,LPARAM lParam) {
   IObjParam *ip = DLGetWindowLongPtr<IObjParam*>(hWnd);
   switch (msg) {
   case WM_INITDIALOG:
      DLSetWindowLongPtr(hWnd, lParam);
      break;

   case WM_LBUTTONDOWN:
   case WM_LBUTTONUP:
   case WM_MOUSEMOVE:
      if (ip) ip->RollupMouseMessage(hWnd,msg,wParam,lParam);
      return FALSE;

   default:
      return FALSE;
   }
   return TRUE;
}

TCHAR *GetString(int id) {
   static TCHAR buf[256];
   if (hInstance) return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
   return NULL;
}
