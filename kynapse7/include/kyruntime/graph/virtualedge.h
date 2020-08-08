/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: JUBA - secondary contact: NOBODY
#ifndef KY_VIRTUALEDGE_H
#define KY_VIRTUALEDGE_H

#include <kypathdata/graph/graphtypes.h>
#include <kyruntime/graph/graphvertexptr.h>
#include <kyruntime/graph/edgedata.h>


namespace Kaim
{

class StitchedGraph;

/*	Used internally to link \GraphCells added to the GraphManager.
	\ingroup kyruntime_graph */
class VirtualEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	VirtualEdge() : m_virtualStitchedGraph(KY_NULL) {}

	~VirtualEdge()
	{
		Invalidate();
	}

	bool IsValid() const { return m_startVertexPtr.IsValid() && m_endVertexPtr.IsValid(); }

	void Invalidate()
	{
		KY_FUNCTION("VirtualEdge::Invalidate")
		m_startVertexPtr.Invalidate();
		m_endVertexPtr.Invalidate();
		KY_ASSERTK(m_data.IsNotLocked());
		m_data.Clear();
		m_virtualStitchedGraph = KY_NULL;
	}

	EdgeData& GetEdgeData() const { return m_data; }

	StitchedGraph* GetVirtualStitchedGraph() const { return m_virtualStitchedGraph; }

public:
	GraphVertexPtr m_startVertexPtr;
	GraphVertexPtr m_endVertexPtr;

	mutable EdgeData m_data;

	StitchedGraph* m_virtualStitchedGraph;
};

} // namespace Kaim

#endif // KY_VIRTUALEDGE_H
