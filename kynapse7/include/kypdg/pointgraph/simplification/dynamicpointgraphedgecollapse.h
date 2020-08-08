/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPdg_DynamicGraphEdgeCollapse_H
#define KyPdg_DynamicGraphEdgeCollapse_H


#include "kypdg/pointgraph/dynamicpointgraphedge.h"
#include "kypathdata/containers/binaryheap.h"

namespace Kaim
{


/*

*/
class PdgGraphEdgeCollapse
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	PdgGraphEdgeCollapse() :
		m_edge(KY_NULL), m_cost(0), m_indexInPriorityQueueEdge(KyUInt32MAXVAL)
	{
	}

	explicit PdgGraphEdgeCollapse(PdgGraphEdge& edge) :
	m_edge(KY_NULL), m_cost(0), m_indexInPriorityQueueEdge(KyUInt32MAXVAL)
	{
		SetEdge(edge);
	}

	void SetEdge(PdgGraphEdge& edge)
	{
		m_edge = &edge;
		m_edge->m_collapse = this;
		m_cost = m_edge->GetSquareLength2d();
	}

	~PdgGraphEdgeCollapse()
	{
		Clear();
	}

	void Clear()
	{
		if (m_edge)
			m_edge->m_collapse = KY_NULL;
		m_edge = KY_NULL;
	}

public:
	PdgGraphEdge* m_edge;
	KyFloat32 m_cost;
	KyUInt32 m_indexInPriorityQueueEdge;
};

class PdgGraphEdgeLengthLess
{
public:
	bool operator () (const PdgGraphEdgeCollapse* lhs, const PdgGraphEdgeCollapse* rhs) const
	{
		return lhs->m_cost < rhs->m_cost;
	}
};


typedef BinaryHeap<PdgGraphEdgeCollapse*, PdgGraphEdgeLengthLess> PriorityQueueEdge;

// Specialize this to track reference on a element of the container
// Called after the element has been added.
template <>
inline void AddReference(PriorityQueueEdge& priorityQueueEdge, KyUInt32 index)
{
	priorityQueueEdge[index]->m_indexInPriorityQueueEdge = index;
}

// Specialize this to track reference on a element of the container
// Called before the element has been removed.
template <>
inline void RemoveReference(PriorityQueueEdge& priorityQueueEdge, KyUInt32 index)
{
	priorityQueueEdge[index]->m_indexInPriorityQueueEdge = PriorityQueueEdge::InvalidReference;
}

// Specialize this to track reference on a element of the container
// Called after the elements has been swapped.
template <>
inline void SwapReference(PriorityQueueEdge& priorityQueueEdge, KyUInt32 left, KyUInt32 right)
{
	Swap(priorityQueueEdge[left]->m_indexInPriorityQueueEdge, priorityQueueEdge[right]->m_indexInPriorityQueueEdge);
}

}

#endif // #ifndef KyPdg_DynamicGraphEdgeCollapse_H
