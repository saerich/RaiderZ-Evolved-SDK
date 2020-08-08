/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavMeshUtils_H
#define KyPathData_NavMeshUtils_H


#include "kypathdata/navmesh/navmeshtypes.h"


namespace Kaim
{
/*! Retrieves the minimum and maximum values from the specified inputs.
	\param a			The first value.
	\param b			The second value.
	\param[out] min		The lowest of the supplied values.
	\param[out] max		The largest of the supplied values.
	\ingroup kypathdata_navmesh */
KY_FORCE_INLINE void GetMinMax(const KyFloat32 a, const KyFloat32 b, KyFloat32& min, KyFloat32& max)
{
	min = Kaim::Min(a, b);
	max = Kaim::Max(a, b);
}

/*! Retrieves the minimum and maximum values from the specified inputs.
	\param a			The first value.
	\param b			The second value.
	\param c			The third value.
	\param[out] min		The lowest of the supplied values.
	\param[out] max		The largest of the supplied values. 
	\ingroup kypathdata_navmesh */
KY_FORCE_INLINE void GetMinMax(const KyFloat32 a, const KyFloat32 b, const KyFloat32 c, KyFloat32& min, KyFloat32& max)
{
	min = Kaim::Min(Kaim::Min(a, b), c);
	max = Kaim::Max(Kaim::Max(a, b), c);
}

/*! Retrieves the minimum and maximum values from the specified inputs.
	\param a			The first value.
	\param b			The second value.
	\param c			The third value.
	\param d			The third value.
	\param[out] min		The lowest of the supplied values.
	\param[out] max		The largest of the supplied values. 
	\ingroup kypathdata_navmesh */
KY_FORCE_INLINE void GetMinMax(const KyFloat32 a, const KyFloat32 b, const KyFloat32 c, const KyFloat32 d, KyFloat32& min, KyFloat32& max)
{
	min = Kaim::Min(Kaim::Min(a, b), Kaim::Min(c, d));
	max = Kaim::Max(Kaim::Max(a, b), Kaim::Max(c, d));
}

}

#endif

