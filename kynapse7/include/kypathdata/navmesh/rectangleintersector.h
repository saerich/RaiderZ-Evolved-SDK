/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_RectangleIntersector_H
#define KyPathData_RectangleIntersector_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/math/boxobstacle.h"
#include "kypathdata/navmesh/intersections.h"
#include "kypathdata/navmesh/closestpoint.h"
#include "kypathdata/navmesh/segmentcangoquery.h"
#include "kypathdata/navmesh/segmentcastquery.h"

namespace Kaim
{

/*
class RectangleIntersector
*/
class RectangleIntersector
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	RectangleIntersector(SegmentCastQuery& query) : m_startA(query.m_startA), m_startB(query.m_startB), m_maxDist(query.m_maxDist)
	{
		m_rectangle = BoxObstacle(query.m_startB, query.m_normalizedDir2d, m_maxDist, 2.f * query.m_radius, 0.f);
	}

	RectangleIntersector(SegmentCanGoQuery& query)
	{
		Vec2f dir(query.m_destPos.x - query.m_startPos.x, query.m_destPos.y - query.m_startPos.y);
		KyFloat32 length = dir.Normalize();
		m_rectangle = BoxObstacle(query.m_startB, dir, length, 2.f * query.m_radius, 0.f);
	}

	KY_FORCE_INLINE bool DoesIntersectEdge(const Vec3f& startEdgePos, const Vec3f& endEdgePos)
	{
		return Intersections::SegmentVsBoxObstacle2d(startEdgePos, endEdgePos, m_rectangle);
	}

	KY_FORCE_INLINE void ComputeCollisionPosOnEdge(const Vec3f& startEdgePos, const Vec3f& endEdgePos, Vec3f& collisionPos, KyFloat32& squareDistToCollisionPos)
	{
		ClosestPoint::OnSegmentVsSegment2d(startEdgePos, endEdgePos, m_startA, m_startB, m_maxDist, collisionPos, squareDistToCollisionPos);
	}

	KY_FORCE_INLINE void ComputeTriangleCost(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, KyFloat32& cost)
	{
		Vec3f unused;
		ClosestPoint::OnTriangleVsSegment2d(v0, v1, v2, m_startA, m_startB, m_maxDist, unused, cost);
	}

private:
	BoxObstacle m_rectangle;
	Vec3f m_startA;
	Vec3f m_startB;
	KyFloat32 m_maxDist;
};


}


#endif //KyPathData_RectangleIntersector_H

