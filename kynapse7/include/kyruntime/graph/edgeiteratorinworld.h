/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_EdgeIteratorInWorld_H
#define KyPathData_EdgeIteratorInWorld_H

#include <kypathdata/generic/memory.h>

#include <kyruntime/graph/stitchedgraphcell.h>
#include <kyruntime/graph/graphcellstitcher.h>
#include <kyruntime/graph/graphmanager.h>
#include <kyruntime/graph/graphedgeptr.h>
#include <kyruntime/graph/vertexiteratorinworld.h>
#include <kyruntime/graph/edgeiteratoraroundvertex.h>


namespace Kaim
{

class GraphManager;

/*!	This class represents a bi-directional iterator that can be used to iterate through all 
	edges contained in a GraphManager.
	\pp You can retrieve an instance of this class by calling GraphManager::GetEdgeIterator().
	\ingroup kyruntime_graph */
class EdgeIteratorInWorld
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor. EdgeIteratorInWorld objects constructed using this method cannot handle virtual edges.
		For internal use. */
	EdgeIteratorInWorld(const GraphStitchDataManager& stitchDataManager) :
	m_vertexIteratorInWorld(stitchDataManager), m_outEdgeIteratorAroundVertex(GraphVertexPtr())
	{
		while (m_vertexIteratorInWorld.IsValid())
		{
			m_outEdgeIteratorAroundVertex.GetFirstFromGraphVertexPtr(*m_vertexIteratorInWorld);
			if (m_outEdgeIteratorAroundVertex.IsValid())
			{
				m_edgePtr = *m_outEdgeIteratorAroundVertex;
				return;
			}

			++m_vertexIteratorInWorld;
		}

		m_edgePtr.Invalidate();
	}

	/*!	Returns #KY_TRUE if this iterator points to a valid edge. */
	KY_FORCE_INLINE bool IsValid() const { return m_edgePtr.IsValid(); }
	
	/*!	Returns a GraphEdgePtr that corresponds to the edge pointed to by this iterator. */
	KY_FORCE_INLINE GraphEdgePtr operator*() const { return m_edgePtr; }
	/*!	Returns a GraphEdgePtr that corresponds to the edge pointed to by this iterator. */
	KY_FORCE_INLINE const GraphEdgePtr* operator->() const { return &m_edgePtr; }
	
	/*!	Increments this iterator to point to the next edge in the GraphManager. */
	EdgeIteratorInWorld& operator++()
	{
		if (!m_edgePtr.IsValid())
			return *this;

		++m_outEdgeIteratorAroundVertex;
		if (m_outEdgeIteratorAroundVertex.IsValid())
		{
			m_edgePtr = *m_outEdgeIteratorAroundVertex;
			return *this;
		}

		++m_vertexIteratorInWorld;

		while (m_vertexIteratorInWorld.IsValid())
		{
			m_outEdgeIteratorAroundVertex.GetFirstFromGraphVertexPtr(*m_vertexIteratorInWorld);
			if (m_outEdgeIteratorAroundVertex.IsValid())
			{
				m_edgePtr = *m_outEdgeIteratorAroundVertex;
				return *this;
			}

			++m_vertexIteratorInWorld;
		}

		m_edgePtr.Invalidate();

		return *this;
	}
	
	/*!	Increments this iterator to point to the next edge in the GraphManager. */
	EdgeIteratorInWorld operator++(int)
	{
		EdgeIteratorInWorld res = *this;
		++*this;
		return res;
	}
	
	/*!	Decrements this iterator to point to the previous edge in the GraphManager. */
	EdgeIteratorInWorld& operator--()
	{
		if (!m_edgePtr.IsValid())
			return *this;

		--m_outEdgeIteratorAroundVertex;
		if (m_outEdgeIteratorAroundVertex.IsValid())
		{
			m_edgePtr = *m_outEdgeIteratorAroundVertex;
			return *this;
		}

		--m_vertexIteratorInWorld;

		while (m_vertexIteratorInWorld.IsValid())
		{
			m_outEdgeIteratorAroundVertex.GetLastFromGraphVertexPtr(*m_vertexIteratorInWorld);
			if (m_outEdgeIteratorAroundVertex.IsValid())
			{
				m_edgePtr = *m_outEdgeIteratorAroundVertex;
				return *this;
			}

			--m_vertexIteratorInWorld;
		}

		m_edgePtr.Invalidate();

		return *this;
	}
	
	/*!	Decrements this iterator to point to the previous edge in the GraphManager. */
	EdgeIteratorInWorld operator--(int)
	{
		EdgeIteratorInWorld res = *this;
		--*this;
		return res;
	}

private:
	VertexIteratorInWorld m_vertexIteratorInWorld;
	OutEdgeIteratorAroundVertex m_outEdgeIteratorAroundVertex;

private:
	GraphEdgePtr m_edgePtr;
};


}

#endif //KyPathData_EdgeIteratorInWorld_H

