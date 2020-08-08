/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_BoxObstacle_H
#define KyPathData_BoxObstacle_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/math/vec2f.h"
#include "kypathdata/basesystem/logger.h"

namespace Kaim
{

/*
     ^       +--------------------+
     |       |                    |
     |width  |                    |
     |       |                    |
     |       |                    |        orientation
     v      A+--------------------+------------->

             <-------------------->
                   length
*/

/*!	This class represents a three-dimensional bounding box with a free orientation on the (X,Y) plane.
	\pp It is defined by:
	-	The position of a single reference corner (#m_a).
	-	An orientation vector that determines the facing direction of the box from the reference corner (#m_normalizedOrientation).
	-	A length value, which determines the extents of the box along its orientation vector (#m_length).
	-	A width value, which determines the extents of the box perpendicular to the orientation vector (#m_width).
		The width of the box always extends to the left when facing the orientation.
	-	A thickness value, which determines the extents of the box along the vertical Z axis (#m_thickness).
	\pp For example:
	\code
     ^       +--------------------+
     |       |                    |
     |width  |                    |
     |       |                    |
     |       |                    |        orientation
     v      A+--------------------+------------->

             <-------------------->
                   length
	\endcode
	\ingroup kypathdata_math */
class BoxObstacle
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*!	\label_constructor Creates a box with 0 length, width and thickness. If you use this constructor, you must call Set() to set the dimensions of the rectangle
		before it can be used.*/
	BoxObstacle() : m_length(0.f), m_width(0.f), m_thickness(0.f) {}

	/*!	\label_constructor
		\param a				The position of the reference corner.
		\param orientation		The facing orientation of the box.
		\param length			The extents of the box along its orientation vector.
		\param width			The extents of the box perpendicular to its orientation vector on the (X,Y) plane.
		\param thickness		The extents of the box along the vertical Z axis. */
	BoxObstacle(const Vec3f& a, const Vec2f& orientation, KyFloat32 length, KyFloat32 width, KyFloat32 thickness) :
	 m_a(a), m_length(length), m_width(width), m_thickness(thickness)
	{
		orientation.GetNormalized(m_normalizedOrientation);
	}

	/*!	Sets the dimensions of the box.
		\param a				The position of the reference corner.
		\param orientation		The facing orientation of the box.
		\param length			The extents of the box along its orientation vector.
		\param width			The extents of the box perpendicular to its orientation vector on the (X,Y) plane.
		\param thickness		The extents of the box along the vertical Z axis. */
	void Set(const Vec3f& a, const Vec2f& orientation, KyFloat32 length, KyFloat32 width, KyFloat32 thickness)
	{
		KY_FUNCTION("BoxObstacle::Set");
		KY_ASSERT(orientation.GetLength() == Vec2f(orientation).Normalize(), ("Wrong BoxObtacle parameter, the orientation should be normalized"));

		m_a.Set(a.x, a.y, a.z);
		m_normalizedOrientation.Set(orientation.x, orientation.y);
		m_length = length;
		m_width = width;
		m_thickness = thickness;
	}

public:
	Vec3f m_a; /*!< The position of the reference corner. */
	Vec2f m_normalizedOrientation; /*!< The facing orientation of the box (unit vector). */
	KyFloat32 m_length; /*!< The extents of the box along its orientation vector. */
	KyFloat32 m_width; /*!< The extents of the box perpendicular to its orientation vector on the (X,Y) plane. */
	KyFloat32 m_thickness; /*!< The extents of the box along the vertical Z axis. */
};

}

#endif

