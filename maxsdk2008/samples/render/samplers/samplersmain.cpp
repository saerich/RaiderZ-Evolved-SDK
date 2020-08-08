/**********************************************************************
 *<
	FILE: samplersMain.cpp

	DESCRIPTION:   	DLL main for samplers

	CREATED BY: 	Kells Elmquist

	HISTORY: 	created 1 dec 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "samplersHdr.h"
#include "stdSamplers.h"

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
#ifndef DESIGN_VER
	return 4;
#else
	return 4;
#endif // !DESIGN_VER
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
		// Both versions
		case 0: return GetR25SamplerDesc();
		case 1: return GetHammersleySamplerDesc();
		case 2: return GetAHaltonSamplerDesc();
		case 3: return GetACMJSamplerDesc();
//		case 4: return GetUniformSamplerDesc();
//		case 5: return GetHaltonSamplerDesc();
//		case 6: return GetSingleSamplerDesc();
//		case 7: return GetCMJSamplerDesc();

#ifndef DESIGN_VER	// Not Design version

#else  // DESIGN_VER

#endif // DESIGN_VER

		default: return 0;
	}
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }

TCHAR *GetString(int id)
{
	static TCHAR buf[256];
	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}
