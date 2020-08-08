/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_shapes */

// primary contact: MAMU - secondary contact: LAPA
#ifndef KyPathData_AxisAlignedBoxVolume_H
#define KyPathData_AxisAlignedBoxVolume_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/shapes/ivolume.h>
#include <kypathdata/math/axisalignedbox.h>


namespace Kaim
{

class ExtrudedPolylineVolume;
class ExtrudedRectangleVolume;
class VerticalCylinderVolume;


/*! This implementation of IVolume represents a bounding box whose orientations match the orientation of the axes in the 
	\SDKName coordinate system.
	\pp This class uses an instance of AxisAlignedBox to store and access its dimensions, and provides a layer of helper methods
	for interacting with the data stored in the AxisAlignedBox.
	\ingroup kypathdata_shapes */
class AxisAlignedBoxVolume : public IVolume
{
public:
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor You must set up #m_axisAlignedBox before you can use the newly constructed object. */
	AxisAlignedBoxVolume() : m_axisAlignedBox() {}

	/*! \label_virtualdestructor */
	~AxisAlignedBoxVolume() {}

	virtual void Render(KyUInt32 r, KyUInt32 g, KyUInt32 b) const;

	/*! \copydoc IVolume::GetRenderAnchor()
		This implementation of IVolume::GetRenderAnchor() retrieves the center of the bottom face of the box. */
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
	AxisAlignedBox m_axisAlignedBox; /*!< Stores and provides access to the dimensions of this volume. */
};

} // namespace Kaim

#endif // KyPathData_AxisAlignedBoxVolume_H
