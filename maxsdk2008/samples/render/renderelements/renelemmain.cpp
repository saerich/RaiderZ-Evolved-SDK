/**********************************************************************
 *<
	FILE: renElemMain.cpp

	DESCRIPTION:   	DLL main for render elements

	CREATED BY: 	Kells Elmquist

	HISTORY: 	created 15 apr 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "renElemPch.h"
#include "stdRenElems.h"
#include "stdBakeElem.h"
#include "LuminanceIlluminance.h"

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

__declspec( dllexport ) const TCHAR * LibDescription() { return GetString(IDS_LIBDESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() 
{
	return 29;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
		// Both versions
		case 0: return GetBeautyElementDesc();
		case 1: return GetSpecularElementDesc();
		case 2: return GetDiffuseElementDesc();
		case 3: return GetEmissionElementDesc();
		case 4: return GetReflectionElementDesc();
		case 5: return GetRefractionElementDesc();
		case 6: return GetShadowElementDesc();
		case 7: return GetAtmosphereElementDesc();
		case 8: return GetBlendElementDesc();
		case 9: return GetZElementDesc();
		case 10: return GetAlphaElementDesc();
		case 11: return GetBgndElementDesc();
		case 12: return GetLightingElementDesc();
		case 13: return GetMatteElementDesc();
		case 14: return GetMotionElementDesc();
		case 15: return GetMaterialIDElementDesc();
		case 16: return GetObjectIDElementDesc();
		case 17: return &(GetLuminanceElementClassDesc());
		case 18: return &(GetIlluminanceElementClassDesc());

		case 19: return GetDiffuseBakeElementDesc();
		case 20: return GetSpecularBakeElementDesc();
		case 21: return GetCompleteBakeElementDesc();
//		case 15: return GetReflectRefractBakeElementDesc();
		case 22: return GetAlphaBakeElementDesc();
		case 23: return GetLightBakeElementDesc();
		case 24: return GetShadowBakeElementDesc();
		case 25: return GetNormalsBakeElementDesc();
		case 26: return GetBlendBakeElementDesc();
		case 27: return GetHeightBakeElementDesc();
		case 28: return GetAmbientOcclusionBakeElementDesc();

		

#ifndef DESIGN_VER	// Not Design version

#else  // DESIGN_VER

#endif // DESIGN_VER

		default: return NULL;
	}// end switch
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() {
	return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];
	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}
