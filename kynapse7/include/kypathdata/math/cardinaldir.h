/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_CardinalDir_H
#define KyPathData_CardinalDir_H

#include <kypathdata/generic/types.h>

namespace Kaim
{
/*!	Defines a type that refers to one of the cardinal points on the compass: 
	#CardinalDir_EAST, #CardinalDir_NORTH, #CardinalDir_WEST, or #CardinalDir_SOUTH.
	\ingroup kypathdata_math */
typedef KyUInt32 CardinalDir; 

static const CardinalDir CardinalDir_EAST = 0; /*!< Identifies East, defined as the positive direction of the X axis. \ingroup kypathdata_math */
static const CardinalDir CardinalDir_NORTH = 1; /*!< Identifies North, defined as the positive direction of the Y axis. \ingroup kypathdata_math */
static const CardinalDir CardinalDir_WEST = 2; /*!< Identifies West, defined as the negative direction of the X axis. \ingroup kypathdata_math */
static const CardinalDir CardinalDir_SOUTH = 3; /*!< Identifies South, defined as the negative direction of the Y axis. \ingroup kypathdata_math */
static const CardinalDir CardinalDir_INVALID = KyUInt32MAXVAL; /*!< Identifies an invalid cardinal direction. \ingroup kypathdata_math */

/*!	Returns the CardinalDir that lies in the opposite direction from the specified CardinalDir.
	For example, specifying #CardinalDir_EAST returns #CardinalDir_WEST.
	\ingroup kypathdata_math */
KY_FORCE_INLINE CardinalDir GetOppositeCardinalDir(const CardinalDir dir) { return (dir + 2) % 4; }

}

#endif

