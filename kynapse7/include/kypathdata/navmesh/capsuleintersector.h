/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_CapsuleIntersector_H
#define KyPathData_CapsuleIntersector_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/math/vec2f.h"
#include "kypathdata/math/boxobstacle.h"
#include "kypathdata/navmesh/intersections.h"
#include "kypathdata/navmesh/closestpoint.h"
#include "kypathdata/navmesh/diskcangoquery.h"
#include "kypathdata/navmesh/diskcastquery.h"

namespace Kaim
{

/*
class CapsuleIntersector
*/
class CapsuleIntersector
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	CapsuleIntersector(const DiskCanGoQuery& query) : m_startCenter(query.m_startPos), m_endCenter(query.m_destPos), m_radius(query.m_radius)
	{
		Vec3f dir(query.m_destPos - query.m_startPos);
		dir.z = 0.f;
		m_dist = dir.Normalize();

		const Vec3f normalizedDir3D_ortho(-dir.y, dir.x, 0.f);
		const Vec3f box_a(m_startCenter - (normalizedDir3D_ortho * m_radius));

		m_orientedBox.Set(box_a, Vec2f(dir.x, dir.y), m_dist, m_radius * 2.f, 0.f);
	}

	CapsuleIntersector(const DiskCastQuery& query) : m_startCenter(query.m_startPos), m_normalizedDir2d(query.m_normalizedDir2d),
		m_dist(query.m_maxDist), m_radius(query.m_radius)
	{
		const Vec3f normalizedDir3D_ortho(-query.m_normalizedDir2d.y, query.m_normalizedDir2d.x, 0.f);
		const Vec3f box_a(m_startCenter - (normalizedDir3D_ortho * m_radius));
		m_orientedBox.Set(box_a, m_normalizedDir2d, m_dist, m_radius * 2.f, 0.f);

		const Vec3f normalizedDir3D(m_normalizedDir2d.x, m_normalizedDir2d.y, 0.f);
		m_endCenter = m_startCenter + (normalizedDir3D * m_dist);
	}

	KY_FORCE_INLINE bool DoesIntersectEdge(const Vec3f& startEdgePos, const Vec3f& endEdgePos)
	{
		return Intersections::SegmentVsCapsule2d(startEdgePos, endEdgePos, m_orientedBox, m_startCenter, m_endCenter, m_radius);
	}

	KY_FORCE_INLINE void ComputeCollisionPosOnEdge(const Vec3f& startEdgePos, const Vec3f& endEdgePos, Vec3f& collisionPos, KyFloat32& squareDistToCollisionPos)
	{
		ClosestPoint::OnSegmentVsDisk2d(startEdgePos, endEdgePos, m_startCenter, m_radius, m_normalizedDir2d, m_dist, collisionPos, squareDistToCollisionPos);
	}

	KY_FORCE_INLINE void ComputeTriangleCost(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, KyFloat32& cost)
	{
		Vec3f unused;
		ClosestPoint::OnTriangleVsDisk2d(v0, v1, v2, m_startCenter, m_radius, m_normalizedDir2d, m_dist, unused, cost);
	}

private:
	Vec3f m_startCenter;
	Vec3f m_endCenter;
	Vec2f m_normalizedDir2d;
	KyFloat32 m_dist;
	KyFloat32 m_radius; //< the radius of the extrimity disks

	BoxObstacle m_orientedBox;
};


}


#endif //KyPathData_CapsuleIntersector_H

