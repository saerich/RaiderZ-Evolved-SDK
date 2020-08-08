/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_DiskIntersector_H
#define KyPathData_DiskIntersector_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/navmesh/intersections.h"
#include "kypathdata/navmesh/closestpoint.h"
#include "kypathdata/navmesh/diskcollisionquery.h"
#include "kypathdata/navmesh/diskexpansionquery.h"

namespace Kaim
{

/*
class DiskIntersector
*/
class DiskIntersector
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	DiskIntersector(const DiskExpansionQuery& query) : m_center(query.m_center), m_radius(query.m_radiusMax) {}

	DiskIntersector(const DiskCollisionQuery& query) : m_center(query.m_center), m_radius(query.m_radius) {}

	KY_FORCE_INLINE bool DoesIntersectEdge(const Vec3f& startEdgePos, const Vec3f& endEdgePos)
	{
		return Intersections::SegmentVsDisk2d(startEdgePos, endEdgePos, m_center, m_radius);
	}

	KY_FORCE_INLINE void ComputeCollisionPosOnEdge(const Vec3f& startEdgePos, const Vec3f& endEdgePos, Vec3f& collisionPos, KyFloat32& squareDistToCollisionPos)
	{
		ClosestPoint::OnSegmentVsPoint2d(startEdgePos, endEdgePos, m_center, collisionPos, squareDistToCollisionPos);
	}

	KY_FORCE_INLINE void ComputeTriangleCost(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, KyFloat32& cost)
	{
		Vec3f unused;
		ClosestPoint::OnTriangleVsPoint2d(v0, v1, v2, m_center, unused, cost);
	}

private:
	Vec3f m_center;
	KyFloat32 m_radius;
};


}


#endif //KyPathData_DiskIntersector_H

