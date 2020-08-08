/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_BestFirstSearchEdgeCollisionVisitor_H
#define KyPathData_BestFirstSearchEdgeCollisionVisitor_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navtriangleptr.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

class NavMeshManager;

/*
class BestFirstSearchEdgeCollisionVisitor

TEdgeIntersector must have the following functions :

	bool DoesIntersectEdge(const Vec3f& startEdgePos, const Vec3f& endEdgePos);

	void ComputeCollisionPosOnEdge(const Vec3f& startEdgePos, const Vec3f& endEdgePos, Vec3f& collisionPos, KyFloat32& squareDistToCollisionPos)

	void ComputeTriangleCost(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, KyFloat32& cost)

*/
template<class TEdgeIntersector>
class BestFirstSearchEdgeCollisionVisitor
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	BestFirstSearchEdgeCollisionVisitor(TEdgeIntersector& edgeIntersector) : 
		m_edgeIntersector(&edgeIntersector), m_terrainTypeMask(NavTerrainTypeMask_All), m_collisionFound(false),
		m_squareDistToCollisionPos(KyFloat32MAXVAL), m_lastCost(KyFloat32MAXVAL),
		m_currentNeighBourTriangleIdx(0)
	{
		m_shouldVisitNeighbourTriangle[0] = true;
		m_shouldVisitNeighbourTriangle[1] = true;
		m_shouldVisitNeighbourTriangle[2] = true;
	}

	KY_FORCE_INLINE bool IsSearchFinished() { return m_lastCost > m_squareDistToCollisionPos; }

	KY_FORCE_INLINE bool ShouldVisitTriangle(const NavTrianglePtr& /*trianglePtr*/)
	{
		return m_shouldVisitNeighbourTriangle[m_currentNeighBourTriangleIdx++];
	}

	void ComputeTriangleCost(const NavTrianglePtr& trianglePtr, KyFloat32& cost);

	KY_FORCE_INLINE void SetTerrainTypeMask(NavTerrainTypeMask terrainTypeMask) { m_terrainTypeMask = terrainTypeMask; }

	void Visit(const NavTrianglePtr& trianglePtr, KyFloat32 cost);

public:
	TEdgeIntersector* m_edgeIntersector;
	NavTerrainTypeMask m_terrainTypeMask;
	bool m_collisionFound;

	KyFloat32 m_squareDistToCollisionPos;
	Vec3f m_collisionPos;
	NavHalfEdgePtr m_intersectedHalfEdgePtr;

private:
	KyFloat32 m_lastCost;

	bool m_shouldVisitNeighbourTriangle[3];
	KyUInt32 m_currentNeighBourTriangleIdx;
};

template<class TEdgeIntersector>
void BestFirstSearchEdgeCollisionVisitor<TEdgeIntersector>::ComputeTriangleCost(const NavTrianglePtr& trianglePtr, KyFloat32& cost)
{
	NavVertexIdx v0Idx, v1Idx, v2Idx;
	const NavFloor& navFloor = trianglePtr.GetNavFloor();
	navFloor.TriangleIdxToVertexIndices(trianglePtr.GetTriangleIdx(), v0Idx, v1Idx, v2Idx);

	const NavVertex& v0 = navFloor.GetVertex(v0Idx);
	const NavVertex& v1 = navFloor.GetVertex(v1Idx);
	const NavVertex& v2 = navFloor.GetVertex(v2Idx);

	m_edgeIntersector->ComputeTriangleCost(v0, v1, v2, cost);
}

template<class TEdgeIntersector>
void BestFirstSearchEdgeCollisionVisitor<TEdgeIntersector>::Visit(const NavTrianglePtr& trianglePtr, const KyFloat32 triangleCost)
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

	m_lastCost = triangleCost;

	if (!isACrossableEdges[0] || !isACrossableEdges[1] || !isACrossableEdges[2])
	{
		NavVertex collisionPos;
		KyFloat32 squareDistToCollisionPos;

		if (!isACrossableEdges[0] && doesIntersectsEdges[0])
		{
			m_edgeIntersector->ComputeCollisionPosOnEdge(v0, v1, collisionPos, squareDistToCollisionPos);
			if (squareDistToCollisionPos < m_squareDistToCollisionPos)
			{
				m_squareDistToCollisionPos = squareDistToCollisionPos;
				m_collisionPos = collisionPos;
				m_intersectedHalfEdgePtr.Set(trianglePtr.GetNavFloorPtr(), NavFloor::TriangleIdxToHalfEdgeIdx(trianglePtr.GetTriangleIdx(), 0));
			}
		}

		if (!isACrossableEdges[1] && doesIntersectsEdges[1])
		{
			m_edgeIntersector->ComputeCollisionPosOnEdge(v1, v2, collisionPos, squareDistToCollisionPos);
			if (squareDistToCollisionPos < m_squareDistToCollisionPos)
			{
				m_squareDistToCollisionPos = squareDistToCollisionPos;
				m_collisionPos = collisionPos;
				m_intersectedHalfEdgePtr.Set(trianglePtr.GetNavFloorPtr(), NavFloor::TriangleIdxToHalfEdgeIdx(trianglePtr.GetTriangleIdx(), 1));
			}
		}

		if (!isACrossableEdges[2] && doesIntersectsEdges[2])
		{
			m_edgeIntersector->ComputeCollisionPosOnEdge(v2, v0, collisionPos, squareDistToCollisionPos);
			if (squareDistToCollisionPos < m_squareDistToCollisionPos)
			{
				m_squareDistToCollisionPos = squareDistToCollisionPos;
				m_collisionPos = collisionPos;
				m_intersectedHalfEdgePtr.Set(trianglePtr.GetNavFloorPtr(), NavFloor::TriangleIdxToHalfEdgeIdx(trianglePtr.GetTriangleIdx(), 2));
			}
		}

		m_collisionFound = m_collisionFound || m_squareDistToCollisionPos != KyFloat32MAXVAL;
	}


	KyUInt32 currentNeighBourTriangleIdx = 0;
	if (isACrossableEdges[0])
	{
		m_shouldVisitNeighbourTriangle[0] = doesIntersectsEdges[0];
		currentNeighBourTriangleIdx = 1;
	}

	if (isACrossableEdges[1])
	{
		m_shouldVisitNeighbourTriangle[currentNeighBourTriangleIdx] = doesIntersectsEdges[1];
		++currentNeighBourTriangleIdx;
	}

	if (isACrossableEdges[2])
		m_shouldVisitNeighbourTriangle[currentNeighBourTriangleIdx] = doesIntersectsEdges[2];

	m_currentNeighBourTriangleIdx = 0;
}

}


#endif //KyPathData_BestFirstSearchEdgeCollisionVisitor_H

