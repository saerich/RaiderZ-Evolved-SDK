/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

#ifndef KYCACHESIZE_H
#define KYCACHESIZE_H

#include "kypathdata/generic/types.h"

namespace Kaim
{
	#if defined(KY_WIN64)
		static const KyUInt32 cache_size = 0x7FFF; // 32Kb 
		static const KyUInt32 cache_line = 0x40; // 64b
	#endif /* defined (KY_WIN64) */

	#if defined (KY_WIN32)
		static const KyUInt32 cache_size = 0x7FFF; // 32Kb 
		static const KyUInt32 cache_line = 0x40; // 64b
	#endif /* defined (KY_WIN32) */

	#if defined(KY_XBOX360)
		static const KyUInt32 cache_size = 0x7FFF; // 32Kb 
		static const KyUInt32 cache_line = 0x80; // 128b
	#endif /* KY_XBOX360 */

	#if defined (KY_WII)
		static const KyUInt32 cache_size = 0x7FFF; // 32Kb 
		static const KyUInt32 cache_line = 0x40; // 64b
	#endif /* KY_WII */

	#if defined(KY_LINUX64)
		static const KyUInt32 cache_size = 0x7FFF; // 32Kb 
		static const KyUInt32 cache_line = 0x40; // 64b
	#endif // defined(KY_LINUX64)

	#if defined (KY_LINUX32)
		static const KyUInt32 cache_size = 0x7FFF; // 32Kb 
		static const KyUInt32 cache_line = 0x40; // 64b
	#endif /* KY_LINUX32 */

	#if defined (KY_PS3)
		static const KyUInt32 cache_size = 0x7FFF; // 32Kb 
		static const KyUInt32 cache_line = 0x80; // 128b
	#endif /* KY_PS3 */
}

#endif // KYCACHESIZE_H
