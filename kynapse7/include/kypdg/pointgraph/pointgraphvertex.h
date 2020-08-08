/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PointGraphVertex_H
#define KyPathData_PointGraphVertex_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/blob/blobarray.h"
#include "kypathdata/math/vec3f.h"
#include "kypdg/pointgraph/pointgraphtypes.h"


namespace Kaim
{

// type of the vertex - used for stitching PointGraphCells
enum PointGraphVertexType
{
	NORMALVERTEX,
	CELL_BOUNDARY_VERTEX,
	DynamicGraphVertexType_FORCE32 = 0xFFFFFFFF
};

/*
Serializable structure (Blob) containing all basic data of a graph vertex, namely:
-The unique ID of the vertex.
-The position of the vertex.
-The indices of the outgoing edges.
-The indices of the incoming edges.
*/
class PointGraphVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	PointGraphVertex() {}

public:
	Vec3f m_position;                          //< Position of the vertex.

	BlobArray<GraphEdgeIdx> m_outgoingEdges;   //< Indices of outgoing edge.
	BlobArray<GraphEdgeIdx> m_incomingEdges;   //< Indices of incoming egde.

	KyUInt32 m_type;                           //< PointGraphVertexType.
	KyUInt32 m_terrainType;                           //< PointGraphVertexType.

private:
	PointGraphVertex& operator=(const PointGraphVertex& pointGraphVertex);
	PointGraphVertex(const PointGraphVertex& pointGraphVertex);
};

/*
Implements the endianness swap for all members of PointGraphVertex.
*/
inline void SwapEndianness(Endianness::Target e, PointGraphVertex& self)
{
	SwapEndianness(e, self.m_position);

	SwapEndianness(e, self.m_outgoingEdges);
	SwapEndianness(e, self.m_incomingEdges);

	SwapEndianness(e, self.m_type);
	SwapEndianness(e, self.m_terrainType);
}
}

#endif // #ifndef KyPathData_PointGraphVertex_H
