/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_graph */

// primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphEdge_H
#define KyPathData_GraphEdge_H

#include <kypathdata/graph/graphtypes.h>

namespace Kaim
{

///////////////////////////////////////////////////////////////////////////////////////////
// GraphEdge
///////////////////////////////////////////////////////////////////////////////////////////

/*!	This class represents a single edge within a GraphCell. Each GraphEdge is a one-way connection between
	a starting GraphVertex and an ending GraphVertex of the same GraphCell.
	\ingroup kypathdata_graph */
class GraphEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor For internal use. */
	GraphEdge() : m_startIdx(GraphVertexIdx_Invalid), m_endIdx(GraphVertexIdx_Invalid) {}

	/*!	Retrieves the index of the starting GraphVertex within the GraphCell. */
	GraphVertexIdx GetStartVertexIdx() const { return m_startIdx; }

	/*!	Retrieves the index of the ending GraphVertex within the GraphCell. */
	GraphVertexIdx GetEndVertexIdx() const { return m_endIdx; }

public:
	GraphVertexIdx m_startIdx;     /*!< The index of the starting GraphVertex. Do not modify. */
	GraphVertexIdx m_endIdx;       /*!< The index of the ending GraphVertex. Do not modify. */

private:
	GraphEdge& operator=(const GraphEdge& navMesh);
	GraphEdge(const GraphEdge& navMesh);
};

/*! \label_swapendianness 
	\ingroup kypathdata_graph */
inline void SwapEndianness(Endianness::Target e, GraphEdge& self)
{
	SwapEndianness(e, self.m_startIdx);
	SwapEndianness(e, self.m_endIdx);
}



} // namespace Kaim

#endif // KyPathData_GraphEdge_H

