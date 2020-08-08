/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_graph */

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPathData_AdditionalGraphTypes_H
#define KyPathData_AdditionalGraphTypes_H

#include <kypathdata/blob/blobarray.h>
#include <kypathdata/math/vec3f.h>
#include <kypathdata/graph/graphtypes.h>

namespace Kaim
{


///////////////////////////////////////////////////////////////////////////////////////////
// AdditionalGraphVertexConnectionType
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Enumerates the possible ways that a vertex in an additional graph can be stitched to the surrounding Graph
	at runtime. This is set at the time the vertex is added to the DynamicAdditionalGraph. 
	\ingroup kypathdata_graph */
enum AdditionalGraphVertexConnectionType
{
	AdditionalGraph_No_Connection,							 /*!< Indicates that this vertex should not be stitched to the surrounding Graph at all. */
	AdditionalGraph_OneToOne_Connection_InputVertex,		 /*!< Fastest stitching method. Creates a single bi-directional link to a vertex at the same position that was passed to the ClientInputConsumer as input for the sector. See :USERGUIDE:"Specifying Vertex Positions". */
	AdditionalGraph_OneToOne_Connection_GenericGraphVertex,  /*!< Slower stitching method. Creates a single bi-directional link to a vertex at the same position in the Graph, but that was not passed as input to the ClientInputConsumer. */
	AdditionalGraph_Multiple_Connections_NearbyVertices		 /*!< Slowest but most flexible stitching method. Finds nearby reachable vertices at runtime and creates one or multiple bi-directional links to those vertices. */
};

///////////////////////////////////////////////////////////////////////////////////////////
// AdditionalGraphVertexInfo
///////////////////////////////////////////////////////////////////////////////////////////

/*	This class represents additional information associated with a single vertex in an AdditionalGraph. 
	\ingroup kypathdata_graph */
class AdditionalGraphVertexInfo
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	KyUInt32 m_tag; /*< Can be used to store a custom tag to identify this vertex. */
	KyUInt32 m_data; /*< Can be used to store custom data about this vertex. */
	KyUInt32 m_connectionType; /*< An element from the #AdditionalGraphVertexConnectionType enumeration that indicates the way this vertex should be stitched to the surrounding Graph at runtime. */
};

/* \label_swapendianness 
	\ingroup kypathdata_graph */
inline void SwapEndianness(Endianness::Target e, AdditionalGraphVertexInfo& self)
{
	SwapEndianness(e, self.m_tag);
	SwapEndianness(e, self.m_data);
	SwapEndianness(e, self.m_connectionType);
}

///////////////////////////////////////////////////////////////////////////////////////////
// AdditionalGraphVertex
///////////////////////////////////////////////////////////////////////////////////////////

/*	This class represents a single vertex within an additionalGraph. 
	\ingroup kypathdata_graph */
class AdditionalGraphVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:

	AdditionalGraphVertex() {};
	/*	Retrieves the coordinates of the AdditionalGraphVertex. */
	const Vec3f& GetPosition() const { return m_position; }

	const AdditionalGraphVertexInfo& GetInfo() const { return m_info; }

public:
	Vec3f m_position;                   /*< Maintains the three-dimensional coordinates of this AdditionalGraphVertex. Do not modify. */
	BlobArray<GraphEdgeIdx> m_outEdges; /*< Used internally to iterate through outgoing \AdditionalGraphEdges. Do not modify. */
	BlobArray<GraphEdgeIdx> m_inEdges;  /*< Used internally to iterate through incoming \AdditionalGraphEdges. Do not modify. */
	AdditionalGraphVertexInfo m_info;

private:
	AdditionalGraphVertex& operator=(const AdditionalGraphVertex& navMesh);
	AdditionalGraphVertex(const AdditionalGraphVertex& navMesh);
};

/* \label_swapendianness 
	\ingroup kypathdata_graph */
inline void SwapEndianness(Endianness::Target e, AdditionalGraphVertex& self)
{
	SwapEndianness(e, self.m_position);
	SwapEndianness(e, self.m_outEdges);
	SwapEndianness(e, self.m_inEdges);
	SwapEndianness(e, self.m_info);
}

///////////////////////////////////////////////////////////////////////////////////////////
// AdditionalAdditionalGraphEdgeInfo
///////////////////////////////////////////////////////////////////////////////////////////

/*	This class represents additional information associated with a single vertex in an AdditionalGraph. 
	\ingroup kypathdata_graph */
class AdditionalGraphEdgeInfo
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	KyUInt32 m_tag; /*< Can be used to store a custom tag to identify this edge. */
	KyUInt32 m_data; /*< Can be used to store custom data about this edge. */
};

/* \label_swapendianness 
	\ingroup kypathdata_graph */
inline void SwapEndianness(Endianness::Target e, AdditionalGraphEdgeInfo& self)
{
	SwapEndianness(e, self.m_tag);
	SwapEndianness(e, self.m_data);
}


///////////////////////////////////////////////////////////////////////////////////////////
// AdditionalGraphEdge
///////////////////////////////////////////////////////////////////////////////////////////

/*	This class represents a single edge within a GraphCell. Each AdditionalGraphEdge is a one-way connection between
	a starting AdditionalGraphVertex and an ending AdditionalGraphVertex of the same additionalGraph.
	\ingroup kypathdata_graph */
class AdditionalGraphEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*	\label_constructor For internal use. */
	AdditionalGraphEdge() : m_startIdx(GraphVertexIdx_Invalid), m_endIdx(GraphVertexIdx_Invalid) {}

	/*	Retrieves the index of the starting GraphVertex within the GraphCell. */
	GraphVertexIdx GetStartVertexIdx() const { return m_startIdx; }

	/*	Retrieves the index of the ending GraphVertex within the GraphCell. */
	GraphVertexIdx GetEndVertexIdx() const { return m_endIdx; }

	const AdditionalGraphEdgeInfo& GetInfo() const { return m_info; }

public:
	GraphVertexIdx m_startIdx;     /*< The index of the starting GraphVertex. Do not modify. */
	GraphVertexIdx m_endIdx;       /*< The index of the ending GraphVertex. Do not modify. */
	AdditionalGraphEdgeInfo m_info;

private:
	AdditionalGraphEdge& operator=(const AdditionalGraphEdge&);
	AdditionalGraphEdge(const AdditionalGraphEdge&);
};

/* \label_swapendianness 
	\ingroup kypathdata_graph */
inline void SwapEndianness(Endianness::Target e, AdditionalGraphEdge& self)
{
	SwapEndianness(e, self.m_startIdx);
	SwapEndianness(e, self.m_endIdx);
	SwapEndianness(e, self.m_info);
}

} // namespace Kaim

#endif // KyPathData_AdditionalGraphTypes_H
