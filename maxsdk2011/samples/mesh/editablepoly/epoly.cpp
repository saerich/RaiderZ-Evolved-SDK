/**********************************************************************
 *<
   FILE: EPoly.cpp

   DESCRIPTION: Editable PolyMesh

   CREATED BY: Steve Anderson

   HISTORY: created 4 March 1996

 *>   Copyright (c) Discreet 1999, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "..\PolyPaint\PolyPaint.h"

HINSTANCE hInstance;
int enabled = FALSE;

#define MAX_MOD_OBJECTS 51
ClassDesc *classDescArray[MAX_MOD_OBJECTS];
int classDescCount = 0;

static BOOL InitEPolyDLL(void)
{
	if( !classDescCount )
	{
#ifndef NO_MODIFIER_POLY_SELECT // JP Morel - July 23rd 2002
		classDescArray[classDescCount++] = GetPolySelectDesc  ();
#endif
		classDescArray[classDescCount++] = GetEditablePolyDesc ();
#ifndef NO_MODIFIER_EDIT_NORMAL
		classDescArray[classDescCount++] = GetEditNormalsDesc ();
#endif

		// Register us as the editable poly object?
		RegisterEditPolyObjDesc (GetEditablePolyDesc());
	}
	
	return TRUE;
}

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
// This is the interface to MAX
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIB_DESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() 
{
   InitEPolyDLL();

   return classDescCount;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) {
   InitEPolyDLL();

   if( i < classDescCount )
      return classDescArray[i];
   else
      return NULL;
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }

__declspec( dllexport ) int LibInitialize()
{
	MeshPaintMgr::GetInstance();
	return InitEPolyDLL();
}
__declspec( dllexport ) void LibShutdown()
{
	MeshPaintMgr::DestroyInstance();
}
TCHAR *GetString(int id) {
   static TCHAR buf[256];
   if (hInstance)
      return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
   return NULL;
}

