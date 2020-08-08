/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

	// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_VertexIteratorInWorld_H
#define KyPathData_VertexIteratorInWorld_H

#include <kypathdata/generic/memory.h>
#include <kyruntime/graph/stitchedgraphcell.h>
#include <kyruntime/graph/graphvertexptr.h>
#include <kyruntime/graph/graphstitchdatamanager.h>


namespace Kaim
{

class StitchDataManager;

/*!	This class represents a bi-directional iterator that can be used to iterate through all 
	vertices contained in a GraphManager.
	\pp You can retrieve an instance of this class by calling GraphManager::GetVertexIterator().
	\ingroup kyruntime_graph */
class VertexIteratorInWorld
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:

	/*!	\label_constructor. For internal use. */
	VertexIteratorInWorld(const GraphStitchDataManager& stitchDataManager) : m_stitchDataManager(&stitchDataManager)
	{
		KyUInt32 stitchedGraphIdx = 0;
		while (stitchedGraphIdx < m_stitchDataManager->m_numberOfStitchedGraph)
		{
			StitchedGraph* stitchedGraph = m_stitchDataManager->m_bufferOfStitchedGraph[stitchedGraphIdx];
			StitchedGraphCellIdx stitchedGraphCellIdx = 0;
			while (stitchedGraphCellIdx < stitchedGraph->m_numberOfCells)
			{
				if (FindFirstVertexInCell(stitchedGraph->m_stitchedCells[stitchedGraphCellIdx]))
					return;

				++stitchedGraphCellIdx;
			}

			++stitchedGraphIdx;
		}
	}

	/*!	Sets this iterator to point to the first vertex in the specified cell. For internal use. */
	bool FindFirstVertexInCell(StitchedGraphCell* stitchedGraphCell)
	{
		if (stitchedGraphCell->IsVirtual() == false && stitchedGraphCell->IsStitched())
		{
			const GraphCell& graphCell = stitchedGraphCell->GetGraphCell();
			const KyUInt32 vertexCount = graphCell.GetVertexCount();
			for (GraphVertexIdx vertexIdx = 0; vertexIdx < vertexCount; ++vertexIdx)
			{
				const GraphVertex& vertex = graphCell.GetVertex(vertexIdx);
				const CardinalDir dir = vertex.GetBoundaryDir();
				if (vertex.IsABoundaryVertex() == false || dir % 3 == 0)
				{
					m_vertexPtr.Set(stitchedGraphCell, vertexIdx);
					return true;
				}

				const GraphVertexPtr& twinVertex = stitchedGraphCell->m_links[dir][vertexIdx - graphCell.m_linkVertices[dir].m_first];
				if (!twinVertex.IsValid())
				{
					m_vertexPtr.Set(stitchedGraphCell, vertexIdx);
					return true;
				}
			}
		}
		return false;
	}

	/*!	Returns #KY_TRUE if this iterator points to a valid vertex. */
	KY_FORCE_INLINE bool IsValid() const { return m_vertexPtr.IsValid(); }
	
	void GetGraphVertexPtr(GraphVertexPtr& vertexPtr) const
	{
		vertexPtr = m_vertexPtr;
	}

	/*!	Returns a GraphVertexPtr that corresponds to the edge pointed to by this iterator. */
	KY_FORCE_INLINE GraphVertexPtr operator*() const { return m_vertexPtr; }
	/*!	Returns a GraphVertexPtr that corresponds to the edge pointed to by this iterator. */
	KY_FORCE_INLINE const GraphVertexPtr* operator->() const { return &m_vertexPtr; }

	/*!	Increments this iterator to point to the next vertex in the GraphManager. */
	inline VertexIteratorInWorld& operator++()
	{
		if (!m_vertexPtr.IsValid())
			return *this;


		while (m_vertexPtr.GetGraphVertexIdx() + 1 < m_vertexPtr.GetGraphCell().GetVertexCount())
		{
			++m_vertexPtr.m_vertexIdx;

			const GraphVertex& vertex = m_vertexPtr.GetGraphVertex();
			const CardinalDir dir = vertex.GetBoundaryDir();
			if (vertex.IsABoundaryVertex() == false || dir % 3 == 0)
				return *this;

			const GraphVertexPtr& twinVertex = m_vertexPtr.GetTwinGraphVertexPtr();
			if (!twinVertex.IsValid())
				return *this;
		}

		const StitchedGraph& stitchedGraph = m_vertexPtr.GetStitchedGraph();
		StitchedGraphCellIdx stitchedGraphCellIdx = m_vertexPtr.GetStitchedGraphCell().GetStitchedGraphCellIdx() + 1;
		while (stitchedGraphCellIdx < stitchedGraph.m_numberOfCells)
		{
			if (FindFirstVertexInCell(stitchedGraph.m_stitchedCells[stitchedGraphCellIdx]))
				return *this;

			++stitchedGraphCellIdx;
		}

		KyUInt32 stitchedGraphIdx = stitchedGraph.m_idxInTheBufferOfStitchedGraph + 1;
		while (stitchedGraphIdx < m_stitchDataManager->m_numberOfStitchedGraph)
		{
			StitchedGraph* stitchedGraph = m_stitchDataManager->m_bufferOfStitchedGraph[stitchedGraphIdx];
			stitchedGraphCellIdx = 0;
			while (stitchedGraphCellIdx < stitchedGraph->m_numberOfCells)
			{
				if (FindFirstVertexInCell(stitchedGraph->m_stitchedCells[stitchedGraphCellIdx]))
					return *this;

				++stitchedGraphCellIdx;
			}

			++stitchedGraphIdx;
		}

		m_vertexPtr.Invalidate();
		return *this;
	}
	
	/*!	Increments this iterator to point to the next vertex in the GraphManager. */
	inline VertexIteratorInWorld operator++(int)
	{
		VertexIteratorInWorld res = *this;
		++*this;
		return res;
	}


	/*!	Sets this iterator to point to the last vertex in the specified cell. For internal use. */
	bool FindLastVertexInCell(StitchedGraphCell* stitchedGraphCell)
	{
		if (stitchedGraphCell->IsVirtual() == false && stitchedGraphCell->IsStitched())
		{
			const GraphCell& graphCell = stitchedGraphCell->GetGraphCell();
			const KyUInt32 vertexCount = graphCell.GetVertexCount();
			for (GraphVertexIdx vertexIdx = vertexCount; vertexIdx > 0;)
			{
				--vertexIdx;
				const GraphVertex& vertex = graphCell.GetVertex(vertexIdx);
				const CardinalDir dir = vertex.GetBoundaryDir();
				if (vertex.IsABoundaryVertex() == false || dir % 3 == 0)
				{
					m_vertexPtr.Set(stitchedGraphCell, vertexIdx);
					return true;
				}

				const GraphVertexPtr& twinVertex = stitchedGraphCell->m_links[dir][vertexIdx - graphCell.m_linkVertices[dir].m_first];
				if (!twinVertex.IsValid())
				{
					m_vertexPtr.Set(stitchedGraphCell, vertexIdx);
					return true;
				}
			}
		}
		return false;
	}

	/*!	Decrements this iterator to point to the previous vertex in the GraphManager. */
	inline VertexIteratorInWorld& operator--()
	{
		if (!m_vertexPtr.IsValid())
			return *this;

		while (m_vertexPtr.GetGraphVertexIdx() > 0)
		{
			--m_vertexPtr.m_vertexIdx;
			const GraphVertex& vertex = m_vertexPtr.GetGraphVertex();
			if (vertex.IsABoundaryVertex() == false || vertex.GetBoundaryDir() % 3 == 0)
				return *this;
		}

		const StitchedGraph& stitchedGraph = m_vertexPtr.GetStitchedGraph();
		StitchedGraphCellIdx stitchedGraphCellIdx = m_vertexPtr.GetStitchedGraphCell().GetStitchedGraphCellIdx();
		while (stitchedGraphCellIdx > 0 )
		{
			--stitchedGraphCellIdx;
			if (FindLastVertexInCell(stitchedGraph.m_stitchedCells[stitchedGraphCellIdx ]))
				return *this;
		}

		KyUInt32 stitchedGraphIdx = stitchedGraph.m_idxInTheBufferOfStitchedGraph;
		while (stitchedGraphIdx > 0)
		{
			--stitchedGraphIdx;
			StitchedGraph* stitchedGraph = m_stitchDataManager->m_bufferOfStitchedGraph[stitchedGraphIdx];
			stitchedGraphCellIdx = stitchedGraph->m_numberOfCells;
			while (stitchedGraphCellIdx > 0)
			{
				--stitchedGraphCellIdx;
				if (FindLastVertexInCell(stitchedGraph->m_stitchedCells[stitchedGraphCellIdx]))
					return *this;
			}
		}

		m_vertexPtr.Invalidate();
		return *this;
	}
	
	/*!	Decrements this iterator to point to the previous vertex in the GraphManager. */
	inline VertexIteratorInWorld operator--(int)
	{
		VertexIteratorInWorld res = *this;
		--*this;
		return res;
	}

private:
	const GraphStitchDataManager* m_stitchDataManager;
	GraphVertexPtr m_vertexPtr;

};


}

#endif //KyPathData_VertexIteratorInWorld_H

