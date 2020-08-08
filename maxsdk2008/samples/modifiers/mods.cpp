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

#define MAX_MOD_OBJECTS 52 // LAM - 2/3/03 - bounced up from 51
ClassDesc *classDescArray[MAX_MOD_OBJECTS];
int classDescCount = 0;

// russom - 05/24/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

void initClassDescArray(void)
{
   if( !classDescCount )
   {
#ifndef NO_MODIFIER_BEND   // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetBendModDesc();
#endif 

#ifndef NO_MODIFIER_TAPER // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetTaperModDesc();
#endif 

#ifndef NO_SPACEWARPS
    classDescArray[classDescCount++] = GetSinWaveObjDesc();
    classDescArray[classDescCount++] = GetSinWaveModDesc();
#endif

#ifndef NO_MODIFIER_EDIT_MESH // russom - 10/11/01
    classDescArray[classDescCount++] = GetEditMeshModDesc();
#endif

#ifndef NO_MODIFIER_TWIST // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetTwistModDesc();
#endif 

    classDescArray[classDescCount++] = GetExtrudeModDesc();

#ifndef NO_SPACEWARPS
    classDescArray[classDescCount++] = GetBombObjDesc();
    classDescArray[classDescCount++] = GetBombModDesc();    
#endif

#ifndef NO_MODIFIER_XFORM // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetClustModDesc();
#endif   

    classDescArray[classDescCount++] = GetSkewModDesc();

#ifndef NO_MODIFIER_NOISE // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetNoiseModDesc();
#endif           

#ifndef NO_MODIFIER_RIPPLE_WAVE // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetSinWaveOModDesc();
    classDescArray[classDescCount++] = GetLinWaveOModDesc();
#endif

#ifndef NO_SPACEWARPS
    classDescArray[classDescCount++] = GetLinWaveObjDesc();
    classDescArray[classDescCount++] = GetLinWaveModDesc();
#endif

#ifndef NO_MODIFIER_OPTIMIZE // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetOptModDesc();
#endif 

#ifndef NO_MODIFIER_DISPLACE // JP Morel - July 23th 2002
    classDescArray[classDescCount++] = GetDispModDesc();
#endif

   classDescArray[classDescCount++] = GetClustNodeModDesc();

#ifndef NO_SPACEWARPS
  classDescArray[classDescCount++] = GetGravityObjDesc();
  classDescArray[classDescCount++] = GetGravityModDesc();
  classDescArray[classDescCount++] = GetWindObjDesc();
  classDescArray[classDescCount++] = GetWindModDesc();
  classDescArray[classDescCount++] = GetDispObjDesc();
  classDescArray[classDescCount++] = GetDispWSModDesc();
  classDescArray[classDescCount++] = GetDeflectObjDesc();
  classDescArray[classDescCount++] = GetDeflectModDesc();
#endif

   classDescArray[classDescCount++] = GetUVWMapModDesc();

#ifndef NO_MODIFIER_VOLUME_SELECT // JP Morel - July 25th 2002 
   classDescArray[classDescCount++] = GetSelModDesc();
#endif

   classDescArray[classDescCount++] = GetSmoothModDesc();

#ifndef NO_MODIFIER_MATERIAL
   classDescArray[classDescCount++] = GetMatModDesc();
#endif

   classDescArray[classDescCount++] = GetNormalModDesc();

#ifndef NO_MODIFIER_LATHE // JP Morel - July 23th 2002
   classDescArray[classDescCount++] = GetSurfrevModDesc();
#endif

#ifndef NO_UTILITY_RESETXFORM // JP Morel - July 25th 2002
   classDescArray[classDescCount++] = GetResetXFormDesc();
#endif

#ifndef NO_MODIFIER_AFFECTREGION // JP Morel - June 28th 2002
   classDescArray[classDescCount++] = GetAFRModDesc();         
#endif 

#ifndef NO_MODIFIER_TESSELATE // JP Morel - June 28th 2002
   classDescArray[classDescCount++] = GetTessModDesc();
#endif 

#ifndef NO_MODIFIER_DELETE_MESH     // JP Morel - June 28th 2002
   classDescArray[classDescCount++] = GetDeleteModDesc();
#endif 

  classDescArray[classDescCount++] = GetMeshSelModDesc();

#ifndef NO_MODIFIER_FACE_EXTRUDE // JP Morel - July 23th 2002
   classDescArray[classDescCount++] = GetFaceExtrudeModDesc();
#endif

#ifndef NO_MODIFIER_UVW_XFORM    // russom - 10/11/01
  classDescArray[classDescCount++] = GetUVWXFormModDesc();
  classDescArray[classDescCount++] = GetUVWXFormMod2Desc();
#endif

#ifndef NO_MODIFIER_MIRROR  // JP Morel - June 28th 2002
   classDescArray[classDescCount++] = GetMirrorModDesc();
#endif 

#ifndef NO_SPACEWARPS
  classDescArray[classDescCount++] = GetBendWSMDesc();
  classDescArray[classDescCount++] = GetTwistWSMDesc();
  classDescArray[classDescCount++] = GetTaperWSMDesc();
  classDescArray[classDescCount++] = GetSkewWSMDesc();
  classDescArray[classDescCount++] = GetNoiseWSMDesc();
#endif

#ifndef NO_MODIFIER_DELETE_SPLINE   // JP Morel - July 23th 2002
   classDescArray[classDescCount++] = GetSDeleteModDesc();
#endif

 #if !defined(NO_OUTPUTRENDERER) && !defined(NO_MODIFIER_DISP_APPROX)
   classDescArray[classDescCount++] = GetDispApproxModDesc();
#endif

#if !defined(NO_WSM_DISPLACEMESH)
   classDescArray[classDescCount++] = GetMeshMesherWSMDesc();
#endif

#ifndef NO_MODIFIER_NORMALIZE_SPLINE // JP Morel - July 23th 2002
   classDescArray[classDescCount++] = GetNormalizeSplineDesc();
#endif

#ifndef NO_PATCHES // JP Morel - July 22th 2002
   classDescArray[classDescCount++] = GetDeletePatchModDesc();
#endif

   DbgAssert (classDescCount <= MAX_MOD_OBJECTS);
   }
}


/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;
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


__declspec( dllexport ) int LibNumberClasses() 
{
   initClassDescArray();

   return classDescCount;
}

// russom - 05/07/01 - changed to use classDescArray
__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) 
{
   initClassDescArray();

   if( i < classDescCount )
      return classDescArray[i];
   else
      return NULL;
}


// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
   return 1;
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
