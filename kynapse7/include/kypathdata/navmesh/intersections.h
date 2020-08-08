/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_Intersections_H
#define KyPathData_Intersections_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/math/boxobstacle.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/math/vec2f.h"
#include "kypathdata/math/box2f.h"
#include "kypathdata/math/box3f.h"

namespace Kaim
{


class Intersections // class with static functions only
{
public :

// Itersections
// we test the intersection between convex polylines by using the Separated-Axes Theorem
static bool AABBVsAABB2d(const Box2f& box1, const Box2f& box2);
static bool AABBVsAABB3d(const Box3f& box1, const Box3f& box2);
static bool AABBVsAABB2d(const Box2f& box1, const Box3f& box2);

static bool AABBVsBoxObstacle2d(const BoxObstacle& orientedBox, const Box2f& box);
static bool AABBVsBoxObstacle2d(const BoxObstacle& orientedBox, const Box3f& box)
{
	return AABBVsBoxObstacle2d(orientedBox, Box2f(box.m_min.x, box.m_min.y, box.m_max.x, box.m_max.y));
}

static bool SegmentVsDisk2d(const Vec3f& a, const Vec3f& b, const Vec3f& c, KyFloat32 radius);
static bool SegmentVsDisk2d(const Vec3f& a, const Vec3f& b, const Vec3f& c, KyFloat32 radius, Vec3f& intersection);

static bool SegmentVsBoxObstacle2d(const Vec3f& v1, const Vec3f& v2, const BoxObstacle& orientedBox);

static bool SegmentVsAABB2d(const Vec3f& v1, const Vec3f& v2, const Box2f& aabb2d);

static bool SegmentVsCapsule2d(const Vec3f& v1, const Vec3f& v2, const Vec3f& center, const KyFloat32 radius, const Vec2f& normalizedDir2D, const KyFloat32 distCast);
// this function should be call by a visitor to avoid creating and computing orientedBox, center1 and center2 at each call as in the first function SegmentVsCapsule2d
static bool SegmentVsCapsule2d(const Vec3f& v1, const Vec3f& v2, const BoxObstacle& orientedBox, const Vec3f& center1, const Vec3f& center2, const KyFloat32 radius);

// test intersection between a segment and a Capsule that is truncated on the left and of the right along the middle axis.
static bool SegmentVsCrossSectionCapsule2d(const Vec3f& v1, const Vec3f& v2, const Vec3f& center, const KyFloat32 radius, const Vec2f& normalizedDir2D, const KyFloat32 distCast, const KyFloat32 distOnRight, const KyFloat32 distOnLeft);
// this function should be call by a visitor to avoid creating and computing 2 BoxBostacle and the center of the end Disk of the capsule
static bool SegmentVsCrossSectionCapsule2d(const Vec3f& v1, const Vec3f& v2, const BoxObstacle& box, const Vec3f& centerOfEndDisk, const KyFloat32 radius, const BoxObstacle& boxOfEndDisk);

static bool SegmentVsTriangle2d(const Vec2f& a, const Vec2f& b, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3);

static bool TriangleVsBoxObstacle2d(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const BoxObstacle& orientedBox);

//Not used. Be careful, this function has not been unitTested yet
static bool TriangleVsTriangle2d(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Vec3f& u1, const Vec3f& u2, const Vec3f& u3);


static bool TriangleVsAABB2d(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Box2f& box);
static bool OverlappingBoxObstacleVsTriangle3D(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const BoxObstacle& orientedBox, const KyFloat32 toleranceBelow, const KyFloat32 toleranceAbove);

static bool SegmentVsSegment2d(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d);
static bool SegmentVsSegment2d(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d, Vec3f& intersection);

static bool LineVsLine2d(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d);
static bool LineVsLine2d(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d, Vec3f& intersection);


// these two functions are test of intersection with a vertical line
//
//// compute if vertical line that contains p intersects the plane defined by the triangle
static bool IsPointInsideTriangle2d(const Vec3f& p,const Vec3f& v1, const Vec3f& v2, const Vec3f& v3);

// compute the altitude of point of intersection the vertical line that contains p and the plane defined by the triangle
static KyFloat32 ComputeAltitudeOfPointInTriangle(const Vec3f& p,const Vec3f& v1, const Vec3f& v2, const Vec3f& v3);

// triangle v1, v2, v3 is CounterClockWise
static KyFloat32 ComputeInCircumCircleDeterminant(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Vec3f& p);
static bool IsPointInsideCircumCircle2d(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Vec3f& p);
static bool IsPointStrictlyInsideCircumCircle2d(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Vec3f& p);

};

}

#endif // KyPathData_Intersections_H

