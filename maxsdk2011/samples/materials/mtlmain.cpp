/**********************************************************************
 *<
   FILE: mtl.cpp

   DESCRIPTION:   DLL implementation of material and textures

   CREATED BY: Dan Silva

   HISTORY: created 12 December 1994

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "stdmat.h"
#include "mtlres.h"
#include "mtlresOverride.h"

#include <vector>

HINSTANCE hInstance;

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
// This is the interface to Max:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }

// orb - 01/03/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

// Made classDescArray dynamic to avoid problems with MAX_MTLTEX_OBJECTS
// being less than the number of maps.
std::vector<ClassDesc *> classDescArray;

static BOOL InitMtlDLL(void)
{
   if( classDescArray.empty() )
   {

      classDescArray.push_back(   GetStdMtl2Desc() );
      classDescArray.push_back(  GetMultiDesc() );

#ifndef NO_MTL_TOPBOTTOM      
      classDescArray.push_back(  GetCMtlDesc() );
#endif // NO_MTL_TOPBOTTOM

      classDescArray.push_back(  GetBMTexDesc() );
      classDescArray.push_back(  GetMaskDesc() );
      
#ifndef NO_MAPTYPE_RGBTINT
      classDescArray.push_back(  GetTintDesc() );
#endif // NO_MAPTYPE_RGBTINT
      
      
      classDescArray.push_back(  GetCheckerDesc() );
      classDescArray.push_back(  GetMixDesc() );
      classDescArray.push_back(  GetMarbleDesc() );
      classDescArray.push_back(  GetNoiseDesc() );
      classDescArray.push_back(  GetTexmapsDesc() );
      classDescArray.push_back(  GetDoubleSidedDesc() );
      classDescArray.push_back(  GetMixMatDesc() );

#ifndef NO_MAPTYPE_REFLECTREFRACT
      classDescArray.push_back(  GetACubicDesc() );
#endif // NO_MAPTYPE_REFLECTREFRACT

#ifndef NO_MAPTYPE_FLATMIRROR 
      classDescArray.push_back(  GetMirrorDesc() );
#endif // NO_MAPTYPE_FLATMIRROR

#ifndef NO_MAPTYPE_GRADIENT      
      classDescArray.push_back(  GetGradientDesc() );
#endif // NO_MAPTYPE_GRADIENT

      classDescArray.push_back(  GetCompositeDesc() );

#ifndef NO_MTL_MATTESHADOW
      classDescArray.push_back(  GetMatteDesc() );
#endif // NO_MTL_MATTESHADOW

#ifndef NO_MAPTYPE_RGBMULT
      classDescArray.push_back(  GetRGBMultDesc() );
#endif // NO_MAPTYPE_RGBMULT
      
#ifndef NO_MAPTYPE_OUTPUT
      classDescArray.push_back(  GetOutputDesc() );
#endif // NO_MAPTYPE_OUTPUT
      
      classDescArray.push_back(  GetFalloffDesc() );

#ifndef  NO_MAPTYPE_VERTCOLOR
      classDescArray.push_back(  GetVColDesc() );
#endif // NO_MAPTYPE_VERTCOLOR

#ifndef USE_LIMITED_STDMTL 
      classDescArray.push_back(  GetPhongShaderCD() );
      classDescArray.push_back(  GetMetalShaderCD() );
#endif
      classDescArray.push_back(  GetBlinnShaderCD() );
      classDescArray.push_back(  GetOldBlinnShaderCD() );

#ifndef NO_MAPTYPE_THINWALL
      classDescArray.push_back(  GetPlateDesc() );
#endif // NO_MAPTYPE_THINWALL

      classDescArray.push_back(  GetOldTexmapsDesc() );

#ifndef NO_MTL_COMPOSITE
      classDescArray.push_back(  GetCompositeMatDesc() );
#endif // NO_MTL_COMPOSITE

#if !defined( NO_PARTICLES ) // orb 07-11-01

#ifndef NO_MAPTYPE_PARTICLEMBLUR
      classDescArray.push_back(  GetPartBlurDesc() );
#endif  // NO_MAPTYPE_PARTICLEMBLUR

#ifndef NO_MAPTYPE_PARTICLEAGE
      classDescArray.push_back(  GetPartAgeDesc() );
#endif // NO_MAPTYPE_PARTICLEAGE

#endif 
      classDescArray.push_back(  GetBakeShellDesc() );

#ifndef NO_MAPTYPE_COLORCORRECTION
      classDescArray.push_back(  GetColorCorrectionDesc() );
#endif

      // register SXP readers
      RegisterSXPReader(_T("MARBLE_I.SXP"), Class_ID(MARBLE_CLASS_ID,0));
      RegisterSXPReader(_T("NOISE_I.SXP"),  Class_ID(NOISE_CLASS_ID,0));
      RegisterSXPReader(_T("NOISE2_I.SXP"), Class_ID(NOISE_CLASS_ID,0));
   }

   return TRUE;
}

__declspec( dllexport ) int LibNumberClasses() 
{ 
   InitMtlDLL();

   return int(classDescArray.size());
}

// This function return the ith class descriptor.
__declspec( dllexport ) ClassDesc* 
LibClassDesc(int i) {
   InitMtlDLL();

   if( i < classDescArray.size() )
      return classDescArray[i];
   else
      return NULL;

   }



// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

__declspec( dllexport ) int LibInitialize() 
{
   return InitMtlDLL();
}

TCHAR *GetString(int id)
{
   static TCHAR buf[256];
   if(hInstance)
      return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
   return NULL;
}
