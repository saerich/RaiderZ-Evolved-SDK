/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyCrt_Math_H
#define KyCrt_Math_H

#include <kypathdata/generic/macros.h>
#include <kypathdata/generic/basetypes.h>
#include <math.h>

namespace Kaim
{
/*
Macros used do detect platforms
KY_WIN64
KY_WIN32
KY_XBOX360
KY_GCN
KY_WII
KY_LINUX64
KY_LINUX32
KY_PS3 (__GCC__ or __SNC__)
*/

/* -------------- floorf --------------*/

#if defined(KY_XBOX360)
	// we face with problems using floorf directly on XBOX360
	// To solve this issue, we encapsulate it inside a function in a .cpp file

	KyFloat32 NonInlinedFloorF(KyFloat32 floatValue);
	KY_FORCE_INLINE KyFloat32 Floorf(KyFloat32 floatValue) { return NonInlinedFloorF(floatValue); }
#else

	KY_FORCE_INLINE KyFloat32 Floorf(KyFloat32 floatValue) { return floorf(floatValue); }

#endif


/* -------------- ceilf --------------*/

	KY_FORCE_INLINE KyFloat32 Ceilf(KyFloat32 floatValue) { return ceilf(floatValue); }

}
#endif //#define KyCrt_Math_H

