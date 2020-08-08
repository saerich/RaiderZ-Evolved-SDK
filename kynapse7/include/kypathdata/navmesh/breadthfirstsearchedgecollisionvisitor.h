/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_BreadthFirstSearchEdgeCollisionVisitor_H
#define KyPathData_BreadthFirstSearchEdgeCollisionVisitor_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

class NavMeshManager;

/*
class BreadthFirstSearchEdgeCollisionVisitor

TEdgeIntersector must have the following function :

	bool DoesIntersectEdge(const Vec3f& startEdgePos, const Vec3f& endEdgePos)
*/
template<class TEdgeIntersector>
class BreadthFirstSearchEdgeCollisionVisitor
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	BreadthFirstSearchEdgeCollisionVisitor(TEdgeIntersector& edgeIntersector) :
		m_edgeIntersector(&edgeIntersector), m_terrainTypeMask(NavTerrainTypeMask_All), m_collisionFound(false),
		m_currentNeighBourTriangleIdx(0)
	{
		m_shouldVisitNeighbourTriangle[0] = true;
		m_shouldVisitNeighbourTriangle[1] = true;
		m_shouldVisitNeighbourTriangle[2] = true;
	}

	KY_FORCE_INLINE bool IsSearchFinished() { return m_collisionFound; }

	KY_FORCE_INLINE bool ShouldVisitTriangle(const NavTrianglePtr& /*trianglePtr*/)
	{
		return m_shouldVisitNeighbourTriangle[m_currentNeighBourTriangleIdx++];
	}

	KY_FORCE_INLINE void SetTerrainTypeMask(NavTerrainTypeMask terrainTypeMask) { m_terrainTypeMask = terrainTypeMask; }

	void Visit(const NavTrianglePtr& trianglePtr);

public:
	TEdgeIntersector* m_edgeIntersector;
	NavTerrainTypeMask m_terrainTypeMask;
	bool m_collisionFound;

private:
	bool m_shouldVisitNeighbourTriangle[3];
	KyUInt32 m_currentNeighBourTriangleIdx;
};

template<class TEdgeIntersector>
void BreadthFirstSearchEdgeCollisionVisitor<TEdgeIntersector>::Visit(const NavTrianglePtr& trianglePtr)
{
	bool isACrossableEdges[3];
	bool doesIntersectsEdges[3];

	NavHalfEdgePtr navHalfEdgePtr0, navHalfEdgePtr1, navHalfEdgePtr2;
	NavVertexIdx v0Idx, v1Idx, v2Idx;

	const NavTriangleIdx triangleIdx = trianglePtr.GetTriangleIdx();
	const NavFloorPtr& navFloorPtr = trianglePtr.GetNavFloorPtr();
	const NavFloor& navFloor = navFloorPtr.GetNavFloor();

	navFloor.TriangleIdxToVertexIndices(triangleIdx, v0Idx, v1Idx, v2Idx);

	navHalfEdgePtr0.Set(navFloorPtr, NavFloor::TriangleIdxToHalfEdgeIdx(triangleIdx, 0));
	navHalfEdgePtr1.Set(navFloorPtr, NavFloor::TriangleIdxToHalfEdgeIdx(triangleIdx, 1));
	navHalfEdgePtr2.Set(navFloorPtr, NavFloor::TriangleIdxToHalfEdgeIdx(triangleIdx, 2));

	const NavVertex& v0 = navFloor.GetVertex(v0Idx);
	const NavVertex& v1 = navFloor.GetVertex(v1Idx);
	const NavVertex& v2 = navFloor.GetVertex(v2Idx);

	isACrossableEdges[0] = navHalfEdgePtr0.IsHalfEdgeCrossable(m_terrainTypeMask);
	isACrossableEdges[1] = navHalfEdgePtr1.IsHalfEdgeCrossable(m_terrainTypeMask);
	isACrossableEdges[2] = navHalfEdgePtr2.IsHalfEdgeCrossable(m_terrainTypeMask);

	doesIntersectsEdges[0] = m_edgeIntersector->DoesIntersectEdge(v0, v1);
	doesIntersectsEdges[1] = m_edgeIntersector->DoesIntersectEdge(v1, v2);
	doesIntersectsEdges[2] = m_edgeIntersector->DoesIntersectEdge(v2, v0);

	m_collisionFound =	(!isACrossableEdges[0] && doesIntersectsEdges[0]) ||
						(!isACrossableEdges[1] && doesIntersectsEdges[1]) ||
						(!isACrossableEdges[2] && doesIntersectsEdges[2]);

	m_currentNeighBourTriangleIdx = 0;

	for (KyUInt32 idx = 0; idx < 3; ++idx)
	{
		if (isACrossableEdges[idx])
		{
			m_shouldVisitNeighbourTriangle[m_currentNeighBourTriangleIdx] = doesIntersectsEdges[idx];
			++m_currentNeighBourTriangleIdx;
		}
	}

	m_currentNeighBourTriangleIdx = 0;
}
}


#endif //KyPathData_BreadthFirstSearchEdgeCollisionVisitor_H

