/**********************************************************************
 *<
	FILE:			   color_correction.h
	DESCRIPTION:	Color Correction map Class Descriptor (declaration)
	CREATED BY:		Hans Larsen
	HISTORY:		   Created 2.oct.07
 *>	Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/
#ifndef __COLORCORRECTION_H__
#define __COLORCORRECTION_H__

#include "iparamm2.h"

////////////////////////////////////////////////////////////////////////////////
// Disabling color correction.
#ifndef NO_MAPTYPE_COLORCORRECTION

namespace ColorCorrection {
   // Even though the instance has a common name, it is situated in an anonymous
   // namespace so it's only available inside this file.
   struct Desc : public ClassDesc2 {
            int         IsPublic();
            void      * Create(BOOL);
            SClass_ID   SuperClassID();
            Class_ID    ClassID();
            HINSTANCE   HInstance();

      const TCHAR     * ClassName();
      const TCHAR     * Category();
      const TCHAR     * InternalName();
   };

}

ClassDesc2* GetColorCorrectionDesc();

#endif // NO_MAPTYPE_COLORCORRECTION

////////////////////////////////////////////////////////////////////////////////
#endif // __COLORCORRECTION_H__
