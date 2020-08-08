/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_shapes */

// primary contact: MAMU - secondary contact: LAPA
#ifndef KyPathData_iVolume_H
#define KyPathData_iVolume_H

#include <kypathdata/generic/basetypes.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{
class Vec3f;
class AxisAlignedBoxVolume;
class ExtrudedRectangleVolume;
class ExtrudedPolylineVolume;
class VerticalCylinderVolume;
class BoxObstacle;

/*!	IVolume is an abstract base class that provides an interface for interacting with three-dimensional volumes.
	You cannot use the IVolume class as-is. You must use one of the derived classes supplied with the \SDKName
	SDK, or write your own custom class that derives from IVolume. 
	\ingroup kypathdata_shapes */
class IVolume
{
public:
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_virtualdestructor */
	virtual ~IVolume() {}

	/*! Draws the outline of the volume in the specified color.
		\param r				\label_redval
		\param g				\label_greenval
		\param b				\label_blueval */
	virtual void Render(KyUInt32 r, KyUInt32 g, KyUInt32 b) const;

	/*! Retrieves a point in the volume that may be suitable for use as an anchor point
		or the position of a manipulator in a 3D rendering of the volume. */
	virtual KyResult GetRenderAnchor(Vec3f& anchor) const = 0;

	/*! Updates the specified coordinates to reflect the minima and maxima of the smallest axis-aligned
		bounding box that completely encloses this volume. */
	virtual KyResult FillAabb(Vec3f& aabbMin, Vec3f& aabbMax) const = 0;

	/*! Updates the specified BoxObstacle to reflect the dimensions of the smallest oriented
		bounding box that completely encloses this volume. */
	virtual KyResult FillOrientedBox(BoxObstacle&) const = 0;

	/*! Indicates whether or not the specified point lies inside this three-dimensional volume. */
	virtual KyBool IsPositionInside(const Vec3f&) const = 0;

	/*! Indicates whether or not the specified line segment intersects with this volume. */
	virtual KyBool IntersectSegment(const Vec3f&, const Vec3f&) const = 0;

	/*! Indicates whether or not the specifed triangle intersects with this volume. */
	virtual KyBool IntersectTriangle(const Vec3f&, const Vec3f&, const Vec3f&) const = 0;

	/*! Indicates whether or not the specifed IVolume intersects with this volume. */
	virtual KyBool Intersect(const IVolume*) const = 0;

	/*! Indicates whether or not the specifed AxisAlignedBoxVolume intersects with this box. */
	virtual KyBool Intersect(const AxisAlignedBoxVolume&) const = 0;

	/*! Indicates whether or not the specifed ExtrudedRectangleVolume intersects with this box. */
	virtual KyBool Intersect(const ExtrudedRectangleVolume&) const = 0;

	/*! Indicates whether or not the specifed VerticalCylinderVolume intersects with this box. */
	virtual KyBool Intersect(const VerticalCylinderVolume&) const = 0;

	/*! Indicates whether or not the specifed ExtrudedPolylineVolume intersects with this box. */
	virtual KyBool Intersect(const ExtrudedPolylineVolume&) const = 0;
};

} // namespace Kaim

#endif // KyPathData_iVolume_H
