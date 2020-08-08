/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_shapes */

// primary contact: LAPA - secondary contact: LASI
#ifndef KY_EXTRUDEDPOLYLINEVOLUME_H
#define KY_EXTRUDEDPOLYLINEVOLUME_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/shapes/ivolume.h>
#include <kypathdata/math/extrudedpolyline.h>


namespace Kaim
{

class AxisAlignedBoxVolume;
class ExtrudedRectangleVolume;
class VerticalCylinderVolume;


/*!	This implementation of IVolume represents a three-dimensional volume using:
	-	A polyline that indicates the outline of the volume in two dimensions (ignoring altitude).
	-	A minimum altitude value.
	-	A maximum altitude value.
	\pp This class uses an instance of ExtrudedPolyline to store and access its dimensions, and provides a layer of helper methods
	for interacting with the data stored in the ExtrudedPolyline.
	\ingroup kypathdata/shapes */
class ExtrudedPolylineVolume : public IVolume
{
public:
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor You must set up #m_extrudedPolyline before you can use the newly constructed object. */
	ExtrudedPolylineVolume() {}

	/*! \label_virtualdestructor */
	~ExtrudedPolylineVolume() {}

	virtual void Render(KyUInt32 r, KyUInt32 g, KyUInt32 b) const;

	/*! \copydoc IVolume::GetRenderAnchor()
		This implementation of IVolume::GetRenderAnchor() retrieves the first point in the polyline that outlines this volume. */
	virtual KyResult GetRenderAnchor(Vec3f& anchor) const;

	virtual KyResult FillAabb(Vec3f& aabbMin, Vec3f& aabbMax) const;
	virtual KyResult FillOrientedBox(BoxObstacle&) const;

	virtual KyBool IsPositionInside(const Vec3f& point) const;

	virtual KyBool IntersectSegment(const Vec3f& segStart, const Vec3f& segEnd) const;
	virtual KyBool IntersectTriangle(const Vec3f&, const Vec3f&, const Vec3f&) const;

	virtual KyBool Intersect(const IVolume* volume) const { return volume->Intersect(*this); }

	virtual KyBool Intersect(const AxisAlignedBoxVolume& axisAlignedBoxVolume) const;
	virtual KyBool Intersect(const ExtrudedRectangleVolume& extrudedRectangleVolume) const;
	virtual KyBool Intersect(const VerticalCylinderVolume& verticalCylinderVolume) const;
	virtual KyBool Intersect(const ExtrudedPolylineVolume& extrudedPolylineVolume) const;


public:
	ExtrudedPolyline m_extrudedPolyline; /*!< Stores and provides access to the dimensions of this volume. */
};

} // namespace Kaim

#endif // KY_EXTRUDEDPOLYLINEVOLUME_H
