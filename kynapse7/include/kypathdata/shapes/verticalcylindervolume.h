/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_shapes */

// primary contact: MAMU - secondary contact: LAPA
#ifndef KyPathData_VerticalCylinderVolume_H
#define KyPathData_VerticalCylinderVolume_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/shapes/ivolume.h>
#include <kypathdata/math/verticalcylinder.h>


namespace Kaim
{

class AxisAlignedBoxVolume;
class ExtrudedPolylineVolume;
class ExtrudedRectangleVolume;


/*!	This implementation of IVolume represents a cylinder whose central axis extends along the vertical Z axis.
	\pp This class uses an instance of VerticalCylinder to store and access its dimensions, and provides a layer of helper methods
	for interacting with the data stored in the VerticalCylinder.
	\ingroup kypathdata_shapes */
class VerticalCylinderVolume : public IVolume
{
public:
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor You must set up #m_verticalCylinder before you can use the newly constructed object. */
	VerticalCylinderVolume() {}

	/*! \label_virtualdestructor */
	~VerticalCylinderVolume() {}

	virtual void Render(KyUInt32 r, KyUInt32 g, KyUInt32 b) const;

	/*! \copydoc IVolume::GetRenderAnchor()
		This implementation of IVolume::GetRenderAnchor() retrieves the center of the bottom face of the cylinder. */
	virtual KyResult GetRenderAnchor(Vec3f& anchor) const;

	virtual KyResult FillAabb(Vec3f& aabbMin, Vec3f& aabbMax) const;
	virtual KyResult FillOrientedBox(BoxObstacle& orientedBox) const;

	virtual KyBool IsPositionInside(const Vec3f& point) const;

	virtual KyBool IntersectSegment(const Vec3f& segStart, const Vec3f& segEnd) const;
	virtual KyBool IntersectTriangle(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3) const;

	virtual KyBool Intersect(const IVolume* volume) const { return volume->Intersect(*this); }

	virtual KyBool Intersect(const AxisAlignedBoxVolume& axisAlignedBoxVolume) const;
	virtual KyBool Intersect(const ExtrudedRectangleVolume& extrudedRectangleVolume) const;
	virtual KyBool Intersect(const VerticalCylinderVolume& verticalCylinderVolume) const;
	virtual KyBool Intersect(const ExtrudedPolylineVolume& extrudedPolylineVolume) const;


public:
	VerticalCylinder m_verticalCylinder; /*!< Stores and provides access to the dimensions of this volume. */
};

} // namespace Kaim

#endif // KyPathData_VerticalCylinderVolume_H
