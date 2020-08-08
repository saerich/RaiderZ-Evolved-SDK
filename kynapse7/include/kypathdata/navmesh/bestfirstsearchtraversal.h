/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_BestFirstSearchTraversal_H
#define KyPathData_BestFirstSearchTraversal_H

#include "kypathdata/navmesh/nmqsarray.h"
#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/navmeshtraversalcommon.h"

#include "kypathdata/navmesh/navmeshcontainercommon.h"
#include "kypathdata/navmesh/propagationnode.h"
#include "kypathdata/navmesh/nmqsbinaryheap.h"

namespace Kaim
{

class NavMesh;


/*
TVisitor is a class that must have following methods :

	void ComputeTriangleCost(const NavTrianglePtr& trianglePtr, KyFloat32& cost);

	void Visit(const NavTrianglePtr& trianglePtr, KyFloat32 cost);

	bool IsSearchFinished();

	bool ShouldVisitTriangle(const NavTrianglePtr& trianglePtr);

*/

template <class TVisitor>
class BestFirstSearchTraversal
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public :
	BestFirstSearchTraversal(TVisitor& visitor,
	 void* workingMemoryForHeap, KyUInt32 sizeOfWorkingMemoryForHeapInBytes,
	 void* workingMemoryForDeque, KyUInt32 sizeOfWorkingMemoryForDequeInBytes,
	 void* workingMemoryForvisited = KY_NULL, KyUInt32 sizeOfWorkingMemoryForvisited = 0) :
		m_visitor(&visitor), m_terrainTypeMask(NavTerrainTypeMask_All),
		m_openNodes(workingMemoryForHeap, sizeOfWorkingMemoryForHeapInBytes),
		m_closedNodes(workingMemoryForDeque, sizeOfWorkingMemoryForDequeInBytes),
		m_visitedNodes(workingMemoryForvisited, sizeOfWorkingMemoryForvisited) {}

	KY_FORCE_INLINE void Clear()
	{
		m_openNodes.MakeEmpty();
		m_closedNodes.MakeEmpty();
	}

	TraversalResult SetStartNode(const NavTrianglePtr& trianglePtr);
	TraversalResult search();

	void SetMemoryForVisitedNodes(void* WorkingMemoryForvisited = KY_NULL, KyUInt32 sizeOfWorkingMemoryForvisited = 0)
	{
		m_visitedNodes = NmqsArray<NavTrianglePtr>(WorkingMemoryForvisited, sizeOfWorkingMemoryForvisited);
	}

	KY_FORCE_INLINE void SetTerrainTypeMask(NavTerrainTypeMask terrainTypeMask) { m_terrainTypeMask = terrainTypeMask; }

private:
	TraversalResult AddTriangleIfNeverEncountered(const NavTrianglePtr& node);
	PropagationNode createNode(const NavTrianglePtr& node);

public :
	TVisitor* m_visitor;          //< the visitor (= the "node analyser")
	NavTerrainTypeMask m_terrainTypeMask;

	NmqsBinaryHeap<PropagationNode> m_openNodes; //< open nodes = nodes that are about to be analysed

	NmqsArray<NavTrianglePtr> m_closedNodes;      //< closed nodes = nodes that have been analysed
	NmqsArray<NavTrianglePtr> m_visitedNodes;     //< nodes that have been visited
};


template <class TVisitor>
TraversalResult BestFirstSearchTraversal<TVisitor>::SetStartNode(const NavTrianglePtr& trianglePtr)
{
	if (m_visitor->ShouldVisitTriangle(trianglePtr) &&
		Kaim::IsTerrainTypeCompliant(m_terrainTypeMask, trianglePtr.GetNavFloorTerrainType()))
	{
		ContainerResult rc = m_openNodes.Insert(PropagationNode(trianglePtr));

		if (rc == ContainerResult_SUCCESS)
			return TraversalResult_DONE;
		
		if (rc == ContainerResult_FULL_BUFFER)
			return TraversalResult_LACK_OF_MEMORY_FOR_OPEN_NODES;
		else
			return TraversalResult_INVALID_MEMORY_FOR_OPEN_NODES;
	}

	return TraversalResult_DONE;
}

template <class TVisitor>
TraversalResult BestFirstSearchTraversal<TVisitor>::search()
{
	m_closedNodes.MakeEmpty();
	bool doesStoreVisited = m_visitedNodes.m_buffer != KY_NULL;

	//const KyUInt32 numberOfStartsNodes = m_openNodes.m_currentSize;
	//KyUInt32 currentNumberNode = 0;
	PropagationNode currentNode;

	// NavHalfEdgePtr of the currentTrianglePtr
	NavHalfEdgePtr navHalfEdgePtr0, navHalfEdgePtr1, navHalfEdgePtr2;
	// pair NavHalfEdgePtr of the previous NavHalfEdgePtr
	NavHalfEdgePtr pairHalfEdgePtr0, pairHalfEdgePtr1, pairHalfEdgePtr2;
	// NavTrianglePtr of the previous NavHalfEdgePtr
	NavTrianglePtr neighbourTrianglePtr0, neighbourTrianglePtr1, neighbourTrianglePtr2;

	while (!m_openNodes.IsEmpty())
	{
		m_openNodes.DeleteMin(currentNode);

		m_visitor->Visit(currentNode.m_trianglePtr, currentNode.m_cost);

		if (doesStoreVisited)
		{
			ContainerResult rc = m_visitedNodes.PushBack(currentNode.m_trianglePtr);
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

		const NavTrianglePtr& currentTrianglePtr = currentNode.m_trianglePtr;
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

		ContainerResult rc = m_closedNodes.PushBack(currentNode.m_trianglePtr);
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
TraversalResult BestFirstSearchTraversal<TVisitor>::AddTriangleIfNeverEncountered(const NavTrianglePtr& trianglePtr)
{
	const KyUInt32 closedNodesSize = m_closedNodes.GetSize();
	const KyUInt32 openNodesCurrentSize = m_openNodes.m_currentSize;

	for (KyUInt32 k = 0; k != closedNodesSize; ++k)
	{
		if (m_closedNodes.GetItem(k) != trianglePtr)
			continue;

		return TraversalResult_DONE;
	}

	for (KyUInt32 k = 0; k < openNodesCurrentSize; ++k)
	{
		if (m_openNodes.m_heap[k].m_trianglePtr != trianglePtr)
			continue;

		return TraversalResult_DONE;
	}

	ContainerResult rc = m_openNodes.Insert(createNode(trianglePtr));

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

template <class TVisitor>
PropagationNode BestFirstSearchTraversal<TVisitor>::createNode(const NavTrianglePtr& trianglePtr)
{
	PropagationNode node(trianglePtr);
	m_visitor->ComputeTriangleCost(node.m_trianglePtr, node.m_cost);
	return node;
}



}

#endif // KyPathData_BestFirstSearchTraversal_H

