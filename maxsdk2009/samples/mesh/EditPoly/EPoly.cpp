/**********************************************************************
 *<
   FILE: EPoly.cpp

   DESCRIPTION: Editable PolyMesh

   CREATED BY: Steve Anderson

   HISTORY: created 4 March 1996

 *>   Copyright (c) Discreet 1999, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"

HINSTANCE hInstance;
static int controlsInit = FALSE;
int enabled = FALSE;

#define MAX_MOD_OBJECTS 51
ClassDesc *classDescArray[MAX_MOD_OBJECTS];
int classDescCount = 0;

void initClassDescArray(void)
{
   if( !classDescCount )
   {
      classDescArray[classDescCount++] = GetEditPolyDesc ();
   }
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
   initClassDescArray();

   return classDescCount;
}

__declspec( dllexport ) ClassDesc*LibClassDesc(int i) {
   initClassDescArray();

   if( i < classDescCount )
      return classDescArray[i];
   else
      return NULL;
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }

// Functions and methods declared in EPoly.h:

TCHAR *GetString(int id) {
   static TCHAR buf[256];
   if (hInstance)
      return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
   return NULL;
}

static PolySelImageHandler thePolySelImageHandler;
PolySelImageHandler *GetPolySelImageHandler () {
   return &thePolySelImageHandler;
}

HIMAGELIST PolySelImageHandler::LoadImages() {
   if (images ) return images;

   HBITMAP hBitmap, hMask;
   images = ImageList_Create(24, 23, ILC_COLOR|ILC_MASK, 10, 0);
   hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE (IDB_SELTYPES));
   hMask = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_SELMASK));
   ImageList_Add (images, hBitmap, hMask);
   DeleteObject (hBitmap);
   DeleteObject (hMask);
   return images;
}

HIMAGELIST PolySelImageHandler::LoadPlusMinus () {
   if (hPlusMinus) return hPlusMinus;

   HBITMAP hBitmap, hMask;
   hPlusMinus = ImageList_Create(12, 12, ILC_MASK, 6, 0);
   hBitmap     = LoadBitmap (hInstance,MAKEINTRESOURCE(IDB_PLUSMINUS));
   hMask       = LoadBitmap (hInstance,MAKEINTRESOURCE(IDB_PLUSMINUSMASK));
   ImageList_Add (hPlusMinus, hBitmap, hMask);
   DeleteObject (hBitmap);
   DeleteObject (hMask);
   return hPlusMinus;
}

bool CheckNodeSelection (Interface *ip, INode *inode) {
   if (!ip) return FALSE;
   if (!inode) return FALSE;
   int i, nct = ip->GetSelNodeCount();
   for (i=0; i<nct; i++) if (ip->GetSelNode (i) == inode) return TRUE;
   return FALSE;
}

ToggleShadedRestore::ToggleShadedRestore (INode *pNode, bool newShow) : mpNode(pNode), mNewShowVertCol(newShow) {
   mOldShowVertCol = mpNode->GetCVertMode() ? true : false;
   mOldShadedVertCol = mpNode->GetShadeCVerts() ? true : false;
   mOldVertexColorType = mpNode->GetVertexColorType ();
}

void ToggleShadedRestore::Restore (int isUndo) {
   mpNode->SetCVertMode (mOldShowVertCol);
   mpNode->SetShadeCVerts (mOldShadedVertCol);
   mpNode->SetVertexColorType (mOldVertexColorType);
   mpNode->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

void ToggleShadedRestore::Redo () {
   mpNode->SetCVertMode (mNewShowVertCol);
   mpNode->SetShadeCVerts (true);
   mpNode->SetVertexColorType (nvct_soft_select);
   mpNode->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}
