/*==============================================================================

  file:     DllEntry.cpp

  author:   Daniel Levesque

  created:  3mar2003

  description:

     Entry point for this plugin.

  modified: 


© 2003 Autodesk
==============================================================================*/
#include "mrShaderListShader.h"
#include "resource.h"

HINSTANCE hInstance;

TCHAR *GetString(int id);

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

__declspec( dllexport ) int LibNumberClasses()
{
   return mrShaderListShader::GetNumClassDescs();
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
   if(i < mrShaderListShader::GetNumClassDescs()) {
      ClassDesc2& classDesc = mrShaderListShader::GetClassDesc(i);
      return &classDesc;
   }
   else {
      return NULL;
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

