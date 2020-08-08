/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: JODA - secondary contact: NOBODY
#ifndef KyPdg_ObstaclesInfo_H
#define KyPdg_ObstaclesInfo_H

#include "kypdg/pointgraph/dynamicpointgraphedge.h"

namespace Kaim
{

class DynamicNavFloor;

class Edge2d
{
public:
	Edge2d() {} // To be compatible with Array
	Edge2d(const Vec2f& v1, const Vec2f& v2) : m_v1(v1), m_v2(v2) {}

	const Vec2f& GetV1() const { return m_v1; }
	const Vec2f& GetV2() const { return m_v2; }

private:
	Vec2f m_v1;
	Vec2f m_v2;
};

class ObstaclesInfo
{
public:
	ObstaclesInfo() {}
	explicit ObstaclesInfo(const DynamicNavFloor& navFloor) { Init(navFloor); }

	void Init(const DynamicNavFloor& navFloor);

	bool CanGo(const Vec2f& v1, const Vec2f& v2) const;
	bool CanGo_Strict(const Vec2f& v1, const Vec2f& v2) const;
	KyFloat32 DistToObstacle(const Vec2f& pos) const;
	bool IsPointCanSeeSegment(const Vec2f& v, const Vec2f& seg1, const Vec2f& seg2) const;
	bool IsTriangleFree(const Vec2f& a, const Vec2f& b, const Vec2f& c) const;

	void AddObstacle(const Vec2f& v1, const Vec2f& v2);
	void AddCellBoundaries(const Vec2f& v1, const Vec2f& v2);
	void AddFloorBoundaries(const Vec2f& v1, const Vec2f& v2);

private:
	typedef const Edge2d* Edge2dConstIterator;
	typedef Edge2d* Edge2dIterator;
private:
	Array<Edge2d> m_obstacles;
	Array<Edge2d> m_cellBoundaries;
	Array<Edge2d> m_floorBoundaries;
};

} // namespace Kaim

#endif // KyPdg_ObstaclesInfo_H
