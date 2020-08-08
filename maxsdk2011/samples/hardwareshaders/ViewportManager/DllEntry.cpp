/**********************************************************************
 *<
   FILE: DllEntry.cpp

   DESCRIPTION: Contains the Dll Entry stuff

   CREATED BY: Neil Hazzard

   HISTORY: 02/15/02

 *>   Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
#include "ViewportManager.h"

extern ClassDesc2* GetViewportManagerControlDesc();
extern ClassDesc2* GetViewportLoaderDesc();

HINSTANCE hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      hInstance = hinstDLL;            // Hang on to this DLL's instance handle.
      DisableThreadLibraryCalls(hInstance);
   }

   return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
   return GetString(IDS_LIBDESCRIPTION);
}

//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
   return 2;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
   switch(i) {
      case 0: return GetViewportManagerControlDesc();
      case 1: return GetViewportLoaderDesc();
      default: return 0;
   }
}

__declspec( dllexport ) ULONG LibVersion()
{
   return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
   static TCHAR buf[256];

   if (hInstance)
      return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
   return NULL;
}

BOOL CheckForDX()
{
	static bool firstcheck	= true;		// ~Wenle: this function get called for the first time?
	static bool checkresult = false;	// ~Wenle: the check result is cached here

	if (firstcheck)
	{
		firstcheck = false;

		ViewExp *vpt = GetCOREInterface()->GetActiveViewport();  

		if (vpt)
		{
			GraphicsWindow *gw = vpt->getGW();

			if (gw && gw->querySupport(GW_SPT_GEOM_ACCEL))
			{
				IHardwareShader * phs = (IHardwareShader*)gw->GetInterface(HARDWARE_SHADER_INTERFACE_ID);
				if (phs)
					checkresult = true;
			}

			GetCOREInterface()->ReleaseViewport( vpt );	// ~Wenle: release the viewport interface
		}
	}

	return checkresult;
}