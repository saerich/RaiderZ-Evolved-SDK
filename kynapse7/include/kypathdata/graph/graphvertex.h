/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_graph */

// primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphVertex_H
#define KyPathData_GraphVertex_H

#include <kypathdata/graph/graphtypes.h>
#include <kypathdata/blob/blobarray.h>
#include <kypathdata/math/vec3f.h>
#include <kypathdata/math/cardinaldir.h>

namespace Kaim
{

///////////////////////////////////////////////////////////////////////////////////////////
// GraphVertex
///////////////////////////////////////////////////////////////////////////////////////////

/*!	This class represents a single vertex within a GraphCell. 
	\ingroup kypathdata_graph */
class GraphVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor For internal use. */
	GraphVertex() : m_boundaryCardinalDir(CardinalDir_INVALID) {}

	/*!	Retrieves the coordinates of the GraphVertex. */
	const Vec3f& GetPosition() const { return m_position; }

	/*!	Indicates whether or not this GraphVertex lies on the boundary of its GraphCell. */
	bool IsABoundaryVertex() const { return m_boundaryCardinalDir != CardinalDir_INVALID; }

	/*!	Indicates the cardinal direction of the boundary that this GraphVertex lies on. */
	KY_FORCE_INLINE CardinalDir GetBoundaryDir() const { return m_boundaryCardinalDir; }

public:
	Vec3f m_position;                   /*!< Maintains the three-dimensional coordinates of this GraphVertex. Do not modify. */
	BlobArray<GraphEdgeIdx> m_outEdges; /*!< Used internally to iterate through outgoing \GraphEdges. Do not modify. */
	BlobArray<GraphEdgeIdx> m_inEdges;  /*!< Used internally to iterate through incoming \GraphEdges. Do not modify. */
	CardinalDir m_boundaryCardinalDir; // CardinalDir_INVALID if in interior

private:
	GraphVertex& operator=(const GraphVertex& navMesh);
	GraphVertex(const GraphVertex& navMesh);
};

/*! \label_swapendianness 
	\ingroup kypathdata_graph */
inline void SwapEndianness(Endianness::Target e, GraphVertex& self)
{
	SwapEndianness(e, self.m_position);
	SwapEndianness(e, self.m_outEdges);
	SwapEndianness(e, self.m_inEdges);
	SwapEndianness(e, self.m_boundaryCardinalDir);
}


} // namespace Kaim

#endif // KyPathData_GraphVertex_H
