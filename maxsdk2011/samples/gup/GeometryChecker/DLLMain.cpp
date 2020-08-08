/*==============================================================================
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Main GeometryChecker dll
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/

#include "checkerimplementations.h"


extern ClassDesc2* GetGeometryManagerClassDesc();
extern ClassDesc2* GetFlippedFacesClassDesc();
extern ClassDesc2* GetFlippedUVWFacesClassDesc();
extern ClassDesc2* GetIsolatedVertexClassDesc();
extern ClassDesc2* GetMissingUVCoordinatesClassDesc();
extern ClassDesc2* GetMultipleEdgeClassDesc();
extern ClassDesc2* GetOpenEdgesClassDesc();
extern ClassDesc2* GetOverlappedUVWFacesClassDesc();
extern ClassDesc2* GetOverlappingFacesClassDesc();
extern ClassDesc2* GetOverlappingVerticesClassDesc();
HINSTANCE hInstance;

TCHAR
*GetString(int id)
{
	static TCHAR buf[512];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


/** public functions **/
BOOL WINAPI
DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;				// Hang on to this DLL's instance handle.
      DisableThreadLibraryCalls(hInstance);
   }

	return(TRUE);
}

//------------------------------------------------------
// This is the interface to MAX:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() {
	return GetString(IDS_LIBDESCRIPTION);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS


#define NUM_CLASSES 10

__declspec( dllexport ) int LibNumberClasses()
{
	return NUM_CLASSES;
}


__declspec( dllexport ) ClassDesc*
LibClassDesc(int i)
{
    switch(i) {
    case 0: return GetGeometryManagerClassDesc();
	case 1: return GetFlippedFacesClassDesc();
	case 2: return GetFlippedUVWFacesClassDesc();
	case 3: return GetIsolatedVertexClassDesc();
	case 4: return GetMissingUVCoordinatesClassDesc();
	case 5: return GetMultipleEdgeClassDesc();
	case 6: return GetOpenEdgesClassDesc();
	case 7: return GetOverlappedUVWFacesClassDesc();
	case 8: return GetOverlappingFacesClassDesc();
	case 9: return GetOverlappingVerticesClassDesc();

    default: return 0;
    }
    
}


// Return version so can detect obsolete DLLs -- NOTE THIS IS THE API VERSION NUMBER
//                                               NOT THE VERSION OF THE DLL.
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 0;
}

