/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_EdgeIteratorAroundVertex_H
#define KyPathData_EdgeIteratorAroundVertex_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/blob/blobarray.h>
#include <kyruntime/graph/stitchedgraphcell.h>
#include <kyruntime/graph/graphedgeptr.h>
#include <kyruntime/graph/graphmanager.h>
#include <kyruntime/graph/edgedata.h>


namespace Kaim
{

/*!	This class represents a bi-directional iterator that can be used to iterate through all edges that
	lead out of a vertex.
	\pp You can obtain an instance of this class by calling GraphVertexPtr::GetOutEdgeIterator().
	\ingroup kyruntime_graph */
class OutEdgeIteratorAroundVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	
protected:
	enum IteratorStatus
	{
		Invalid,
		NormalVertex,
		FirstVertex,
		TwinVertex
	};

	friend class GraphVertexPtr;
	
	/*! \label_constructor For internal use. */
	OutEdgeIteratorAroundVertex(const GraphVertexPtr& graphVertexPtr) : m_vertexPtr(graphVertexPtr), m_currentIdxInArray(0),
		m_IsThereMoreVirtualEdges(KY_TRUE)
	{
		if (!graphVertexPtr.IsValid())
		{
			m_status = Invalid;
			return;
		}

		if (graphVertexPtr.IsABoundaryVertex())
		{
			const GraphVertexPtr& twinVertexPtr = graphVertexPtr.GetTwinGraphVertexPtr();
			if (twinVertexPtr.IsValid())
			{
				const GraphVertex& graphVertex = graphVertexPtr.GetGraphVertex();
				if (graphVertex.GetBoundaryDir() % 3 == 0)
					m_vertexPtr = twinVertexPtr;

				m_status = FirstVertex;
			}
			else
				m_status = NormalVertex;
		}
		else
			m_status = NormalVertex;
		
		m_edgesArray = &m_vertexPtr.GetGraphVertex().m_outEdges;
		m_sharedPoolListOfVirtualEdgeIndices = m_vertexPtr.GetVertexData().GetListOfOutVirtualEdgeIndices();

		if (FindFirstStaticEdgeOfVertex())
			return;

		if (FindFirstVirtualEdgeOfVertex())
			return;

		m_status = Invalid;
	}

	bool FindFirstStaticEdgeOfVertex()
	{
		if (m_edgesArray->m_count != 0)
		{
			// Get first not locked edge
			for (m_currentIdxInArray = 0; m_currentIdxInArray < m_edgesArray->m_count; ++m_currentIdxInArray)
			{
				const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
				if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
					return true;
			}
		}

		if (m_status == FirstVertex)
		{
			const GraphVertexPtr& twinVertex = m_vertexPtr.GetTwinGraphVertexPtr();
			m_edgesArray = &twinVertex.GetGraphVertex().m_outEdges;
			m_sharedPoolListOfVirtualEdgeIndices = twinVertex.GetVertexData().GetListOfOutVirtualEdgeIndices();
			m_status = TwinVertex;
			m_vertexPtr = twinVertex;
			m_currentIdxInArray = 0;
			if (m_edgesArray->m_count != 0)
			{
				// Get first not locked edge
				for (; m_currentIdxInArray < m_edgesArray->m_count; ++m_currentIdxInArray)
				{
					const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
					if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
						return true;
				}
			}
		}

		return false;
	}

	bool FindFirstVirtualEdgeOfVertex()
	{
		// no unlocked static edges around this vertex, check for unlocked virtual edges
		if (m_sharedPoolListOfVirtualEdgeIndices != KY_NULL)
		{
			for(SharedPoolList<VirtualEdgeIdx>::ConstIterator it = m_sharedPoolListOfVirtualEdgeIndices->GetFirst(); it.IsValid(); ++it)
			{
				const VirtualEdge& currentVirtualEdge = m_vertexPtr.GetGraphManager().GetVirtualEdge(*it);
				if (currentVirtualEdge.m_data.IsNotLocked())
				{
					m_linkEdgeIt = it;
					return true;;
				}
			}
		}

		return false;
	}


public:

	/*!	Returns #KY_TRUE if this iterator points to a valid edge. */
	KY_FORCE_INLINE bool IsValid() const { return m_status != Invalid; }

	/*!	Retrieves the GraphEdgePtr that corresponds to the edge currently represented by this iterator. */
	GraphEdgePtr GetGraphEdgePtr() const
	{
		KY_FUNCTION("OutEdgeIteratorAroundVertex::GetGraphEdgePtr");
		KY_ASSERTK(IsValid());

		if (m_currentIdxInArray < m_edgesArray->m_count)
			// static edge
			return GraphEdgePtr(m_vertexPtr.GetStitchedGraphCell(), (*m_edgesArray).Values()[m_currentIdxInArray]);
		else
			return GraphEdgePtr(m_vertexPtr.GetGraphManager().GetVirtualEdge(*m_linkEdgeIt).GetVirtualStitchedGraph()->m_stitchedCells[0], *m_linkEdgeIt);
	}

	/*!	Retrieves the GraphEdgePtr that corresponds to the edge currently represented by this iterator. */
	KY_FORCE_INLINE GraphEdgePtr operator*() const { return GetGraphEdgePtr(); }

	/*!	Increments this iterator to point to the next edge in the Graph. */
	OutEdgeIteratorAroundVertex& operator++()
	{
		KY_FUNCTION("OutEdgeIteratorAroundVertex::operator++");
		KY_ASSERTK(IsValid());

		if (m_currentIdxInArray < m_edgesArray->m_count)
		{
			++m_currentIdxInArray;

			// Get first not locked edge
			while(m_currentIdxInArray < m_edgesArray->m_count)
			{
				const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
				if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
					return *this;

				++m_currentIdxInArray;
			}

			if (m_status == FirstVertex)
			{
				const GraphVertexPtr& twinVertex = m_vertexPtr.GetTwinGraphVertexPtr();
				m_edgesArray = &twinVertex.GetGraphVertex().m_outEdges;
				m_sharedPoolListOfVirtualEdgeIndices = twinVertex.GetVertexData().GetListOfOutVirtualEdgeIndices();
				m_status = TwinVertex;
				m_vertexPtr = twinVertex;

				m_currentIdxInArray = 0;
				// Get first not locked edge
				for (; m_currentIdxInArray < m_edgesArray->m_count; ++m_currentIdxInArray)
				{
						const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
					if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
						return *this;
				}
			}
		}

		if (m_currentIdxInArray == m_edgesArray->m_count && m_sharedPoolListOfVirtualEdgeIndices != KY_NULL)
		{
			if (m_IsThereMoreVirtualEdges == KY_TRUE)
			{
				if (!m_linkEdgeIt.IsValid())
					m_linkEdgeIt = m_sharedPoolListOfVirtualEdgeIndices->GetFirst();
				else
					++m_linkEdgeIt;
				
				for(; m_linkEdgeIt.IsValid(); ++m_linkEdgeIt)
				{
					const VirtualEdge& currentVirtualEdge = m_vertexPtr.GetGraphManager().GetVirtualEdge(*m_linkEdgeIt);
					if (currentVirtualEdge.m_data.IsNotLocked())
						return *this;
				}

				m_IsThereMoreVirtualEdges = KY_FALSE;
			}
		}

		m_status = Invalid;
		return *this;
	}

	/*!	Increments this iterator to point to the next edge in the Graph. */
	OutEdgeIteratorAroundVertex operator++(int)
	{
		OutEdgeIteratorAroundVertex res = *this;
		++*this;
		return res;
	}

	/*!	Decrements this iterator to point to the previous edge in the Graph. */
	OutEdgeIteratorAroundVertex& operator--()
	{
		KY_FUNCTION("EdgeIteratorAroundVertex::operator--");
		KY_ASSERTK(IsValid());

		if (m_currentIdxInArray == m_edgesArray->m_count && m_sharedPoolListOfVirtualEdgeIndices != KY_NULL)
		{
			--m_linkEdgeIt;
			
			for(; m_linkEdgeIt.IsValid(); --m_linkEdgeIt)
			{
				const VirtualEdge& currentVirtualEdge = m_vertexPtr.GetGraphManager().GetVirtualEdge(*m_linkEdgeIt);

				if (currentVirtualEdge.m_data.IsNotLocked())
					return *this;
			}

			m_IsThereMoreVirtualEdges = KY_TRUE;
		}

		if (m_edgesArray->m_count != 0)
		{
			// Get first not locked edge
			while(m_currentIdxInArray > 0)
			{
				--m_currentIdxInArray;
				const GraphEdgeIdx& idx = m_edgesArray->Values()[m_currentIdxInArray];

				if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
					return *this;
			}

			if (m_status == TwinVertex)
			{
				const GraphVertexPtr& twinVertex = m_vertexPtr.GetTwinGraphVertexPtr();
				m_edgesArray = &twinVertex.GetGraphVertex().m_outEdges;
				m_sharedPoolListOfVirtualEdgeIndices = twinVertex.GetVertexData().GetListOfOutVirtualEdgeIndices();
				m_status = FirstVertex;
				m_vertexPtr = twinVertex;
				m_currentIdxInArray = m_edgesArray->m_count;
					// Get first not locked edge
				while (m_currentIdxInArray > 0)
				{
					--m_currentIdxInArray;
					const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
					if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
						return *this;
				}
			}
		}
		
		m_status = Invalid;
		return *this;
	}

	/*!	Decrements this iterator to point to the previous edge in the Graph. */
	OutEdgeIteratorAroundVertex operator--(int)
	{
		OutEdgeIteratorAroundVertex res = *this;
		--*this;
		return res;
	}

private:
	friend class EdgeIteratorInWorld;


	bool FindLastStaticEdgeOfVertex()
	{
		m_currentIdxInArray = m_edgesArray->m_count;
		while (m_currentIdxInArray > 0)
		{
			--m_currentIdxInArray;
			const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
			if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
				return true;
		}

		if (m_status == TwinVertex)
		{
			const GraphVertexPtr& twinVertex = m_vertexPtr.GetTwinGraphVertexPtr();
			m_edgesArray = &twinVertex.GetGraphVertex().m_outEdges;
			m_sharedPoolListOfVirtualEdgeIndices = twinVertex.GetVertexData().GetListOfOutVirtualEdgeIndices();
			m_status = FirstVertex;
			m_vertexPtr = twinVertex;
			m_currentIdxInArray = m_edgesArray->m_count;

			while (m_currentIdxInArray > 0)
			{
				--m_currentIdxInArray;
				const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
				if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
					return true;
			}

		}

		return false;
	}

	bool FindLastVirtualEdgeOfVertex()
	{
		// no unlocked static edges around this vertex, check for unlocked virtual edges
		if (m_sharedPoolListOfVirtualEdgeIndices != KY_NULL)
		{
			for(SharedPoolList<VirtualEdgeIdx>::ConstIterator it = m_sharedPoolListOfVirtualEdgeIndices->GetLast(); it.IsValid(); --it)
			{
				const VirtualEdge& currentVirtualEdge = m_vertexPtr.GetGraphManager().GetVirtualEdge(*it);
				if (currentVirtualEdge.m_data.IsNotLocked())
				{
					m_linkEdgeIt = it;
					return true;
				}
			}
		}

		return false;
	}

	void GetFirstFromGraphVertexPtr(const GraphVertexPtr& graphVertexPtr)
	{
		m_vertexPtr = graphVertexPtr;
		m_currentIdxInArray = 0;
		m_IsThereMoreVirtualEdges = KY_TRUE;
		
		if (graphVertexPtr.IsABoundaryVertex())
		{
			const GraphVertexPtr& twinVertex = graphVertexPtr.GetTwinGraphVertexPtr();
			if (twinVertex.IsValid())
			{
				m_vertexPtr = twinVertex;
				m_status = FirstVertex;
			}
			else
				m_status = NormalVertex;
		}
		else
			m_status = NormalVertex;

		m_edgesArray = &m_vertexPtr.GetGraphVertex().m_outEdges;
		m_sharedPoolListOfVirtualEdgeIndices = m_vertexPtr.GetVertexData().GetListOfOutVirtualEdgeIndices();

		if (FindFirstStaticEdgeOfVertex())
			return;

		if (FindFirstVirtualEdgeOfVertex())
			return;

		m_status = Invalid;
	}

	void GetLastFromGraphVertexPtr(const GraphVertexPtr& graphVertexPtr)
	{
		m_vertexPtr = graphVertexPtr;
		m_currentIdxInArray = 0;
		m_IsThereMoreVirtualEdges = KY_TRUE;

		if (graphVertexPtr.IsABoundaryVertex())
		{
			const GraphVertexPtr& twinVertex = graphVertexPtr.GetTwinGraphVertexPtr();
			if (twinVertex.IsValid())
				m_status = TwinVertex;
			else
				m_status = NormalVertex;
		}
		else
			m_status = NormalVertex;

		m_edgesArray = &m_vertexPtr.GetGraphVertex().m_outEdges;
		m_sharedPoolListOfVirtualEdgeIndices = m_vertexPtr.GetVertexData().GetListOfOutVirtualEdgeIndices();

		if (FindLastVirtualEdgeOfVertex())
			return;

		if (FindLastStaticEdgeOfVertex())
			return;

		m_status = Invalid;
	}

private:
	IteratorStatus m_status;

	GraphVertexPtr m_vertexPtr;
	
	const BlobArray<GraphEdgeIdx>* m_edgesArray;
	const SharedPoolList<VirtualEdgeIdx>* m_sharedPoolListOfVirtualEdgeIndices;

	KyUInt32 m_currentIdxInArray;
	SharedPoolList<VirtualEdgeIdx>::ConstIterator m_linkEdgeIt;

	bool m_IsThereMoreVirtualEdges;
};



/*!	This class represents a bi-directional iterator that can be used to iterate through all edges that
	lead into a vertex.
	\pp You can obtain an instance of this class by calling GraphVertexPtr::GetInEdgeIterator().
	\ingroup kyruntime_graph */
class InEdgeIteratorAroundVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS


protected:
	enum IteratorStatus
	{
		Invalid,
		NormalVertex,
		FirstVertex,
		TwinVertex
	};

	friend class GraphVertexPtr;

	/*! \label_constructor For internal use. */
	InEdgeIteratorAroundVertex(const GraphVertexPtr& graphVertexPtr) : m_vertexPtr(graphVertexPtr), m_currentIdxInArray(0),
		m_IsThereMoreVirtualEdges(KY_TRUE)
	{
		if (!graphVertexPtr.IsValid())
		{
			m_status = Invalid;
			return;
		}

		if (graphVertexPtr.IsABoundaryVertex())
		{
			const GraphVertexPtr& twinVertexPtr = graphVertexPtr.GetTwinGraphVertexPtr();
			if (twinVertexPtr.IsValid())
			{
				const GraphVertex& graphVertex = graphVertexPtr.GetGraphVertex();
				if (graphVertex.GetBoundaryDir() % 3 == 0)
					m_vertexPtr = twinVertexPtr;

				m_status = FirstVertex;
			}
			else
				m_status = NormalVertex;
		}
		else
			m_status = NormalVertex;

		m_edgesArray = &m_vertexPtr.GetGraphVertex().m_inEdges;
		m_sharedPoolListOfVirtualEdgeIndices = m_vertexPtr.GetVertexData().GetListOfInVirtualEdgeIndices();

		if (FindFirstStaticEdgeOfVertex())
			return;

		if (FindFirstVirtualEdgeOfVertex())
			return;

		m_status = Invalid;
	}

	/*! \label_constructor Sets this iterator to point to the first edge in the static PathData that leads into the
		vertex.
		\return true if a valid static edge was found, or false otherwise. */
	bool FindFirstStaticEdgeOfVertex()
	{
		if (m_edgesArray->m_count != 0)
		{
			// Get first not locked edge
			for (m_currentIdxInArray = 0; m_currentIdxInArray < m_edgesArray->m_count; ++m_currentIdxInArray)
			{
				const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
				if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
					return true;
			}
		}

		if (m_status == FirstVertex)
		{
			const GraphVertexPtr& twinVertex = m_vertexPtr.GetTwinGraphVertexPtr();
			m_edgesArray = &twinVertex.GetGraphVertex().m_inEdges;
			m_sharedPoolListOfVirtualEdgeIndices = twinVertex.GetVertexData().GetListOfInVirtualEdgeIndices();
			m_status = TwinVertex;
			m_vertexPtr = twinVertex;
			m_currentIdxInArray = 0;
			if (m_edgesArray->m_count != 0)
			{
				// Get first not locked edge
				for (; m_currentIdxInArray < m_edgesArray->m_count; ++m_currentIdxInArray)
				{
					const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
					if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
						return true;
				}
			}
		}

		return false;
	}

	/*! \label_constructor Sets this iterator to point to the first virtual edge that leads into the vertex.
		\return true if a valid virtual edge was found, or false otherwise. */
	bool FindFirstVirtualEdgeOfVertex()
	{
		// no unlocked static edges around this vertex, check for unlocked virtual edges
		if (m_sharedPoolListOfVirtualEdgeIndices != KY_NULL)
		{
			for(SharedPoolList<VirtualEdgeIdx>::ConstIterator it = m_sharedPoolListOfVirtualEdgeIndices->GetFirst(); it.IsValid(); ++it)
			{
				const VirtualEdge& currentVirtualEdge = m_vertexPtr.GetGraphManager().GetVirtualEdge(*it);
				if (currentVirtualEdge.m_data.IsNotLocked())
				{
					m_linkEdgeIt = it;
					return true;;
				}
			}
		}

		return false;
	}


public:

	/*!	Returns #KY_TRUE if this iterator points to a valid edge. */
	KY_FORCE_INLINE bool IsValid() const { return m_status != Invalid; }

	/*!	Retrieves a GraphEdgePtr that corresponds to the edge currently pointed to by this iterator. */
	GraphEdgePtr GetGraphEdgePtr() const
	{
		KY_FUNCTION("InEdgeIteratorAroundVertex::GetGraphEdgePtr");
		KY_ASSERTK(IsValid());

		if (m_currentIdxInArray < m_edgesArray->m_count)
			// static edge
			return GraphEdgePtr(m_vertexPtr.GetStitchedGraphCell(), (*m_edgesArray).Values()[m_currentIdxInArray]);
		else
			return GraphEdgePtr(m_vertexPtr.GetGraphManager().GetVirtualEdge(*m_linkEdgeIt).GetVirtualStitchedGraph()->m_stitchedCells[0], *m_linkEdgeIt);
	}

	/*!	Retrieves a GraphEdgePtr that corresponds to the edge currently pointed to by this iterator. */
	KY_FORCE_INLINE GraphEdgePtr operator*() const { return GetGraphEdgePtr(); }

	/*!	Increments this iterator to point to the next edge in the Graph. */
	InEdgeIteratorAroundVertex& operator++()
	{
		KY_FUNCTION("InEdgeIteratorAroundVertex::operator++");
		KY_ASSERTK(IsValid());

		if (m_currentIdxInArray < m_edgesArray->m_count)
		{
			++m_currentIdxInArray;

			// Get first not locked edge
			while(m_currentIdxInArray < m_edgesArray->m_count)
			{
				const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
				if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
					return *this;

				++m_currentIdxInArray;
			}

			if (m_status == FirstVertex)
			{
				const GraphVertexPtr& twinVertex = m_vertexPtr.GetTwinGraphVertexPtr();
				m_edgesArray = &twinVertex.GetGraphVertex().m_inEdges;
				m_sharedPoolListOfVirtualEdgeIndices = twinVertex.GetVertexData().GetListOfInVirtualEdgeIndices();
				m_status = TwinVertex;
				m_vertexPtr = twinVertex;

				m_currentIdxInArray = 0;
				// Get first not locked edge
				for (; m_currentIdxInArray < m_edgesArray->m_count; ++m_currentIdxInArray)
				{
					const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
					if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
						return *this;
				}
			}
		}

		if (m_currentIdxInArray == m_edgesArray->m_count && m_sharedPoolListOfVirtualEdgeIndices != KY_NULL)
		{
			if (m_IsThereMoreVirtualEdges == KY_TRUE)
			{
				if (!m_linkEdgeIt.IsValid())
					m_linkEdgeIt = m_sharedPoolListOfVirtualEdgeIndices->GetFirst();
				else
					++m_linkEdgeIt;

				for(; m_linkEdgeIt.IsValid(); ++m_linkEdgeIt)
				{
					const VirtualEdge& currentVirtualEdge = m_vertexPtr.GetGraphManager().GetVirtualEdge(*m_linkEdgeIt);
					if (currentVirtualEdge.m_data.IsNotLocked())
						return *this;
				}

				m_IsThereMoreVirtualEdges = KY_FALSE;
			}
		}

		m_status = Invalid;
		return *this;
	}

	/*!	Increments this iterator to point to the next edge in the Graph. */
	InEdgeIteratorAroundVertex operator++(int)
	{
		InEdgeIteratorAroundVertex res = *this;
		++*this;
		return res;
	}

	/*!	Decrements this iterator to point to the previous edge in the Graph. */
	InEdgeIteratorAroundVertex& operator--()
	{
		KY_FUNCTION("EdgeIteratorAroundVertex::operator--");
		KY_ASSERTK(IsValid());

		if (m_currentIdxInArray == m_edgesArray->m_count && m_sharedPoolListOfVirtualEdgeIndices != KY_NULL)
		{
			--m_linkEdgeIt;

			for(; m_linkEdgeIt.IsValid(); --m_linkEdgeIt)
			{
				const VirtualEdge& currentVirtualEdge = m_vertexPtr.GetGraphManager().GetVirtualEdge(*m_linkEdgeIt);

				if (currentVirtualEdge.m_data.IsNotLocked())
					return *this;
			}

			m_IsThereMoreVirtualEdges = KY_TRUE;
		}

		if (m_edgesArray->m_count != 0)
		{
			// Get first not locked edge
			while(m_currentIdxInArray > 0)
			{
				--m_currentIdxInArray;
				const GraphEdgeIdx& idx = m_edgesArray->Values()[m_currentIdxInArray];

				if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
					return *this;
			}

			if (m_status == TwinVertex)
			{
				const GraphVertexPtr& twinVertex = m_vertexPtr.GetTwinGraphVertexPtr();
				m_edgesArray = &twinVertex.GetGraphVertex().m_inEdges;
				m_sharedPoolListOfVirtualEdgeIndices = twinVertex.GetVertexData().GetListOfInVirtualEdgeIndices();
				m_status = FirstVertex;
				m_vertexPtr = twinVertex;
				m_currentIdxInArray = m_edgesArray->m_count;
				// Get first not locked edge
				while (m_currentIdxInArray > 0)
				{
					--m_currentIdxInArray;
					const GraphEdgeIdx idx = m_edgesArray->Values()[m_currentIdxInArray];
					if (m_vertexPtr.GetStitchedGraphCell().GetEdgeData(idx).IsNotLocked())
						return *this;
				}
			}
		}

		m_status = Invalid;
		return *this;
	}

	/*!	Decrements this iterator to point to the previous edge in the Graph. */
	InEdgeIteratorAroundVertex operator--(int)
	{
		InEdgeIteratorAroundVertex res = *this;
		--*this;
		return res;
	}

private:
	IteratorStatus m_status;

	GraphVertexPtr m_vertexPtr;

	const BlobArray<GraphEdgeIdx>* m_edgesArray;
	const SharedPoolList<VirtualEdgeIdx>* m_sharedPoolListOfVirtualEdgeIndices;

	KyUInt32 m_currentIdxInArray;
	SharedPoolList<VirtualEdgeIdx>::ConstIterator m_linkEdgeIt;

	bool m_IsThereMoreVirtualEdges;
};


}

#endif //KyPathData_EdgeIteratorAroundVertex_H

