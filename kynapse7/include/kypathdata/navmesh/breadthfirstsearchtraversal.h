/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_BreadthFirstSearchTraversal_H
#define KyPathData_BreadthFirstSearchTraversal_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/navmeshtraversalcommon.h"
#include "kypathdata/navmesh/navmeshcontainercommon.h"
#include "kypathdata/navmesh/nmqsdeque.h"
#include "kypathdata/navmesh/nmqsarray.h"

namespace Kaim
{

class NavMesh;

/*
TVisitor is a class that must have following methods :

	void Visit(const NavTrianglePtr& trianglePtr);

	bool IsSearchFinished();

	bool ShouldVisitTriangle(const NavTrianglePtr& trianglePtr);


*/
template <class TVisitor>
class BreadthFirstSearchTraversal
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public :
	BreadthFirstSearchTraversal(TVisitor& visitor,
	 void* WorkingMemoryForDeque1, KyUInt32 sizeOfWorkingMemoryForDeque1InBytes,
	 void* WorkingMemoryForDeque2, KyUInt32 sizeOfWorkingMemoryForDeque2InBytes,
	 void* WorkingMemoryForvisited = KY_NULL, KyUInt32 sizeOfWorkingMemoryForvisited = 0) :
	 m_visitor(&visitor), m_terrainTypeMask(NavTerrainTypeMask_All),
	 m_openNodes(WorkingMemoryForDeque1, sizeOfWorkingMemoryForDeque1InBytes),
	 m_closedNodes(WorkingMemoryForDeque2, sizeOfWorkingMemoryForDeque2InBytes),
	 m_visitedNodes(WorkingMemoryForvisited, sizeOfWorkingMemoryForvisited) {}

	void Clear()
	{
		m_openNodes.MakeEmpty();
		m_closedNodes.MakeEmpty();
	}

	inline TraversalResult SetStartTriangle(const NavTrianglePtr& trianglePtr)
	{
		if (m_visitor->ShouldVisitTriangle(trianglePtr)&&
			Kaim::IsTerrainTypeCompliant(m_terrainTypeMask, trianglePtr.GetNavFloorTerrainType()))
		{
			ContainerResult rc = m_openNodes.PushBack(trianglePtr);

			if (rc == ContainerResult_SUCCESS)
				return TraversalResult_DONE;

			if (rc == ContainerResult_FULL_BUFFER)
				return TraversalResult_LACK_OF_MEMORY_FOR_OPEN_NODES;
			else
				return TraversalResult_INVALID_MEMORY_FOR_OPEN_NODES;
		}

		return TraversalResult_DONE;
	}

	TraversalResult AddTriangleIfNeverEncountered(const NavTrianglePtr& trianglePtr);

	TraversalResult search();

	inline void SetMemoryForVisitedNodes(void* WorkingMemoryForvisited = KY_NULL, KyUInt32 sizeOfWorkingMemoryForvisited = 0)
	{
		m_visitedNodes = NmqsArray<NavTrianglePtr>(WorkingMemoryForvisited, sizeOfWorkingMemoryForvisited);
	}

	KY_FORCE_INLINE void SetTerrainTypeMask(NavTerrainTypeMask terrainTypeMask) { m_terrainTypeMask = terrainTypeMask; }

public :
	TVisitor* m_visitor;                     //< the visitor (= the "node analyser")
	NavTerrainTypeMask m_terrainTypeMask;

	NmqsDeque<NavTrianglePtr> m_openNodes;    //< open nodes = nodes that are about to be analysed

	NmqsArray<NavTrianglePtr> m_closedNodes;  //< closed nodes = nodes that have been analysed
	NmqsArray<NavTrianglePtr> m_visitedNodes; //< nodes that have been visited
};

template <class TVisitor>
TraversalResult BreadthFirstSearchTraversal<TVisitor>::search()
{
	m_closedNodes.MakeEmpty();
	bool doesStoreVisited = m_visitedNodes.m_buffer != KY_NULL;

	NavTrianglePtr currentTrianglePtr;
	// NavHalfEdgePtr of the currentTrianglePtr
	NavHalfEdgePtr navHalfEdgePtr0, navHalfEdgePtr1, navHalfEdgePtr2;
	// pair NavHalfEdgePtr of the previous NavHalfEdgePtr
	NavHalfEdgePtr pairHalfEdgePtr0, pairHalfEdgePtr1, pairHalfEdgePtr2;
	// NavTrianglePtr of the previous NavHalfEdgePtr
	NavTrianglePtr neighbourTrianglePtr0, neighbourTrianglePtr1, neighbourTrianglePtr2;

	while (!m_openNodes.IsEmpty())
	{
		m_openNodes.Front(currentTrianglePtr);
		m_openNodes.PopFront();

		m_visitor->Visit(currentTrianglePtr);

		if (doesStoreVisited)
		{
			ContainerResult rc = m_visitedNodes.PushBack(currentTrianglePtr);
			if (rc != ContainerResult_SUCCESS)
			{
				if (rc == ContainerResult_FULL_BUFFER)
					return TraversalResult_LACK_OF_MEMORY_FOR_VISITED_NODES;
				else
					return TraversalResult_INVALID_MEMORY_FOR_VISITED_NODES;
			}
		}

		if (m_visitor->IsSearchFinished())
			return TraversalResult_DONE;

		const NavFloorPtr& currentFloorPtr = currentTrianglePtr.GetNavFloorPtr();
		const NavTriangleIdx currentTriangleIdx = currentTrianglePtr.GetTriangleIdx();

		navHalfEdgePtr0.Set(currentFloorPtr, NavFloor::TriangleIdxToHalfEdgeIdx(currentTriangleIdx, 0));
		navHalfEdgePtr1.Set(currentFloorPtr, NavFloor::TriangleIdxToHalfEdgeIdx(currentTriangleIdx, 1));
		navHalfEdgePtr2.Set(currentFloorPtr, NavFloor::TriangleIdxToHalfEdgeIdx(currentTriangleIdx, 2));

		bool halfEdge0IsCrossable = navHalfEdgePtr0.IsHalfEdgeCrossable(pairHalfEdgePtr0, m_terrainTypeMask);
		bool halfEdge1IsCrossable = navHalfEdgePtr1.IsHalfEdgeCrossable(pairHalfEdgePtr1, m_terrainTypeMask);
		bool halfEdge2IsCrossable = navHalfEdgePtr2.IsHalfEdgeCrossable(pairHalfEdgePtr2, m_terrainTypeMask);

		if (halfEdge0IsCrossable)
		{
			neighbourTrianglePtr0.Set(pairHalfEdgePtr0.GetNavFloorPtr(), NavFloor::HalfEdgeIdxToTriangleIdx(pairHalfEdgePtr0.GetHalfEdgeIdx()));
			if (m_visitor->ShouldVisitTriangle(neighbourTrianglePtr0))
			{
				TraversalResult rc = AddTriangleIfNeverEncountered(neighbourTrianglePtr0);
				if (rc != TraversalResult_DONE)
					return rc;
			}
		}

		if (halfEdge1IsCrossable)
		{
			neighbourTrianglePtr1.Set(pairHalfEdgePtr1.GetNavFloorPtr(), NavFloor::HalfEdgeIdxToTriangleIdx(pairHalfEdgePtr1.GetHalfEdgeIdx()));
			if (m_visitor->ShouldVisitTriangle(neighbourTrianglePtr1))
			{
				TraversalResult rc = AddTriangleIfNeverEncountered(neighbourTrianglePtr1);
				if (rc != TraversalResult_DONE)
					return rc;
			}
		}

		if (halfEdge2IsCrossable)
		{
			neighbourTrianglePtr2.Set(pairHalfEdgePtr2.GetNavFloorPtr(), NavFloor::HalfEdgeIdxToTriangleIdx(pairHalfEdgePtr2.GetHalfEdgeIdx()));
			if (m_visitor->ShouldVisitTriangle(neighbourTrianglePtr2))
			{
				TraversalResult rc = AddTriangleIfNeverEncountered(neighbourTrianglePtr2);
				if (rc != TraversalResult_DONE)
					return rc;
			}
		}

		ContainerResult rc = m_closedNodes.PushBack(currentTrianglePtr);

		if (rc != ContainerResult_SUCCESS)
		{
			if (rc == ContainerResult_FULL_BUFFER)
				return TraversalResult_LACK_OF_MEMORY_FOR_CLOSED_NODES;
			else
				return TraversalResult_INVALID_MEMORY_FOR_CLOSED_NODES;
		}
	}

	return TraversalResult_DONE;
}

template <class TVisitor>
TraversalResult BreadthFirstSearchTraversal<TVisitor>::AddTriangleIfNeverEncountered(const NavTrianglePtr& trianglePtr)
{
	NavTrianglePtr currentNode;
	const KyUInt32 closedNodesSize = m_closedNodes.GetSize();
	for (KyUInt32 k = 0; k != closedNodesSize; ++k)
	{
		if (m_closedNodes.GetItem(k) != trianglePtr)
			continue;

		return TraversalResult_DONE;
	}

	const KyUInt32 openNodesTailEndIdx = m_openNodes.GetTailEndIdx();
	for (KyUInt32 k = m_openNodes.GetHeadIdx(); k != openNodesTailEndIdx; k = m_openNodes.GetNextItemIdx(k))
	{
		if (m_openNodes.GetItem(k) != trianglePtr)
			continue;

		return TraversalResult_DONE;
	}

	ContainerResult rc = m_openNodes.PushBack(trianglePtr);
	if (rc == ContainerResult_SUCCESS)
		return TraversalResult_DONE;
	else
	{
		if (rc == ContainerResult_FULL_BUFFER)
			return TraversalResult_LACK_OF_MEMORY_FOR_OPEN_NODES;
		else
			return TraversalResult_INVALID_MEMORY_FOR_OPEN_NODES;
	}
}

}

#endif // KyPathData_BreadthFirstSearchTraversal_H

