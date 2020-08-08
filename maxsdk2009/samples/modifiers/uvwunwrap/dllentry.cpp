/**********************************************************************
 *<
   FILE: mods.cpp

   DESCRIPTION:   DLL implementation of modifiers

   CREATED BY: Rolf Berteig (based on prim.cpp)

   HISTORY: created 30 January 1995

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "buildver.h"

#include "3dsmaxport.h"

HINSTANCE hInstance;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;            // Hang on to this DLL's instance handle.
      DisableThreadLibraryCalls(hInstance);
   }

   return(TRUE);
   }


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return
 GetString(IDS_LIBDESCRIPTION); }


// CA - Changed DESIGN_VER to NO_MODIFIER_UVW_UNWRAP to allow
// uvwunrap into VIZ.
#ifndef NO_MODIFIER_UVW_UNWRAP

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return 1;}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
   switch(i) {
      case 0: return GetUnwrapModDesc();
      default: return 0;
      }

   }

#else

//
// DESIGN VERSION EXCLUDES SOME PLUG_INS
//

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return 0;}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
   switch(i) {

      default: return 0;
      }

   }

#endif



// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
   return 0;
}

INT_PTR CALLBACK DefaultSOTProc(
      HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
   {
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

TCHAR *GetString(int id)
   {
   static TCHAR buf[256];

   if (hInstance)
      return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
   return NULL;
   }
