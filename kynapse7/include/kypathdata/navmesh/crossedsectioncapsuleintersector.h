/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_CrossedSectionCapsuleIntersector_H
#define KyPathData_CrossedSectionCapsuleIntersector_H

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
class CrossedSectionCapsuleIntersector
*/
class CrossedSectionCapsuleIntersector
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	CrossedSectionCapsuleIntersector(const DiskCastQuery& query, KyFloat32 distOnRight, KyFloat32 distOnLeft) :
		m_startCenter(query.m_startPos), m_normalizedDir2d(query.m_normalizedDir2d), m_dist(query.m_maxDist), m_radius(query.m_radius)
	{
		const Vec2f normalizedDir_ortho(m_normalizedDir2d.PerpCCW());
		const Vec3f normalizedDir3D(m_normalizedDir2d.x, m_normalizedDir2d.y, 0.f);
		const Vec3f normalizedDir3D_ortho(normalizedDir_ortho.x, normalizedDir_ortho.y, 0.f);

		m_centerOfTheEndHalfDisk = m_startCenter + (normalizedDir3D * m_dist);

		const Vec3f orientedBox_a(m_startCenter - (normalizedDir3D_ortho * distOnRight));
		const Vec3f m_BBoxOfTheEndHalfDisk_a(m_centerOfTheEndHalfDisk - (normalizedDir3D_ortho * distOnRight));

		m_orientedBox.Set(orientedBox_a, m_normalizedDir2d, m_dist, distOnRight + distOnLeft, 0.f);
		m_BBoxOfTheEndHalfDisk.Set(m_BBoxOfTheEndHalfDisk_a, m_normalizedDir2d, m_radius, distOnRight + distOnLeft, 0.f);
	}

	CrossedSectionCapsuleIntersector(const DiskCanGoQuery& query, KyFloat32 distOnRight, KyFloat32 distOnLeft) :
		m_startCenter(query.m_startPos), m_radius(query.m_radius)
	{
		Vec3f dir(query.m_destPos - query.m_startPos);
		dir.z = 0.f;
		m_dist = dir.Normalize();
		m_normalizedDir2d.Set(dir.x, dir.y);

		const Vec2f normalizedDir_ortho(m_normalizedDir2d.PerpCCW());
		const Vec3f normalizedDir3D(m_normalizedDir2d.x, m_normalizedDir2d.y, 0.f);
		const Vec3f normalizedDir3D_ortho(normalizedDir_ortho.x, normalizedDir_ortho.y, 0.f);

		m_centerOfTheEndHalfDisk = m_startCenter + (normalizedDir3D * m_dist);

		const Vec3f orientedBox_a(m_startCenter - (normalizedDir3D_ortho * distOnRight));
		const Vec3f m_BBoxOfTheEndHalfDisk_a(m_centerOfTheEndHalfDisk - (normalizedDir3D_ortho * distOnRight));

		m_orientedBox.Set(orientedBox_a, m_normalizedDir2d, m_dist, distOnRight + distOnLeft, 0.f);
		m_BBoxOfTheEndHalfDisk.Set(m_BBoxOfTheEndHalfDisk_a, m_normalizedDir2d, m_radius, distOnRight + distOnLeft, 0.f);

	}

	KY_FORCE_INLINE bool DoesIntersectEdge(const Vec3f& startEdgePos, const Vec3f& endEdgePos)
	{
		return Intersections::SegmentVsCrossSectionCapsule2d(startEdgePos, endEdgePos, m_orientedBox, m_centerOfTheEndHalfDisk, m_radius, m_BBoxOfTheEndHalfDisk);
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
	Vec2f m_normalizedDir2d;
	KyFloat32 m_dist;
	KyFloat32 m_radius;

	BoxObstacle m_orientedBox;
	BoxObstacle m_BBoxOfTheEndHalfDisk;
	Vec3f m_centerOfTheEndHalfDisk;
};


}


#endif //KyPathData_CrossedSectionCapsuleIntersector_H

