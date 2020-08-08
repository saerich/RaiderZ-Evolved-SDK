/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: JUBA
#ifndef KY_POINTINFO_VERTEXHANDLE_H
#define KY_POINTINFO_VERTEXHANDLE_H

#include <kyruntime/core/pointinfo.h>
#include <kyruntime/graph/graphvertexptr.h>


namespace Kaim
{

/*! This class represents an item of additional information that can be attached
	to a PointWrapper object, which provides access to the vertex related to
	the position saved in the PointWrapper.
	It is used internally by the PathFinder to optimize CPU consumption.
	\ingroup kyruntime_graph */
class PointInfo_VertexPtr: public PointInfo
{
public:
	KY_DECLARE_POINTINFO(PointInfo_VertexPtr)

public:
	/*!	\label_constructor */
	PointInfo_VertexPtr() { Reset(); }

	/*!	Constructs an object of this class from the data provided.
		\param vertex			The nearest vertex to the point contained in the PointWrapper. */
	PointInfo_VertexPtr(const GraphVertexPtr& vertex): m_vertex(vertex) {}

	virtual void Reset() { m_vertex.Invalidate(); }

	/*!	Sets the nearest reachable vertex to the point contained in the PointWrapper. */
	void SetVertexPtr(const GraphVertexPtr& vertex) { m_vertex = vertex; }

	/*!	Retrieves the nearest reachable vertex to the point contained in the PointWrapper. */
	const GraphVertexPtr& GetVertexPtr() const { return m_vertex; }


protected:
	GraphVertexPtr m_vertex;
};

} // namespace Kaim

#endif // KY_POINTINFO_VERTEXHANDLE_H
