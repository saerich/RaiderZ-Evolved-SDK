/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_graph */

// primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphCell_H
#define KyPathData_GraphCell_H


#include <kypathdata/navmesh/navmeshtypes.h>
#include <kypathdata/graph/graphtypes.h>
#include <kypathdata/blob/blobarray.h>
#include <kypathdata/graph/graphedge.h>
#include <kypathdata/graph/graphvertex.h>
#include <kypathdata/math/box3f.h>
#include <kypathdata/generic/guidcompound.h>


namespace Kaim
{

class InputGraphVertexCell;

/*! Defines a range of GraphVertex indices within a GraphCell. */
class GraphCellVertexRange
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	GraphCellVertexRange() : m_first(KyUInt32MAXVAL), m_count(0) {}

	KyUInt32 m_first; /*!< The first GraphVertex index in the range. */
	KyUInt32 m_count; /*!< The number of vertices in the range. */
};


/*! \label_swapendianness
	\ingroup kypathdata_graph */
inline void SwapEndianness(Endianness::Target e, GraphCellVertexRange& self)
{
	SwapEndianness(e, self.m_first);
	SwapEndianness(e, self.m_count);
}


/*!	A GraphCell represents a localized, square area within a Graph. It contains a list of GraphVertex and
	GraphEdge objects that represent the topology of the area: i.e. the connections between different 
	reachable points.
	\ingroup kypathdata_graph */
class GraphCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_blobtype */
	static const char* GetBlobType() { return "GraphCell"; }
	//version history: 
	//-----------------
	//0- The graph becomes a blob !
	//1- Added GraphVertexTerrainType array
	//2- remove AstarId from Vertex
	//3- the uid becomes a GraphGUID
	//4- the GraphGUID becomes a GuidCompound
	//5- the SubGraph becomes a GraphCell
	/*! \label_blobversion */
	static KyUInt32 GetBlobVersion() { return 6; }

public:
	/*!	\label_constructor For internal use. */
	GraphCell() {}

	/*!	Retrieves the number of GraphVertex objects maintained by the GraphCell. */
	KY_FORCE_INLINE KyUInt32 GetVertexCount() const { return m_vertices.m_count; }

	/*!	Retrieves the number of GraphEdge objects maintained by the GraphCell. */
	KY_FORCE_INLINE KyUInt32 GetEdgeCount() const { return m_edges.m_count; }

	/*!	Returns \c true if the GraphCell contains no vertices at all. */
	KY_FORCE_INLINE bool IsEmpty() const { return GetVertexCount() == 0; }
	
	/*!	Retrieves the GraphVertex with the specified index value. */
	const GraphVertex& GetVertex(GraphVertexIdx idx) const { return m_vertices.Values()[idx]; }

	/*!	Retrieves the GraphEdge with the specified index value. */
	const GraphEdge& GetEdge(GraphEdgeIdx idx) const { return m_edges.Values()[idx]; }

	/*!	Retrieves the coordinates of the GraphVertex with the specified index value. */
	const Vec3f& GetVertexPosition(GraphVertexIdx vertexIdx) const { return GetVertex(vertexIdx).GetPosition(); }

	/*!	Retrieves the terrain type of the GraphVertex with the specified index value. */
	GraphVertexTerrainType GetVertexTerrainType(GraphVertexIdx vertexIdx) const { return vertexIdx < m_terrainTypes.m_count ?  m_terrainTypes.Values()[vertexIdx] : GraphVertexTerrainType_Default; }

	/*!	Retrieves the index of the GraphVertex at the start of the GraphEdge with the specified index value. */
	GraphVertexIdx GetStartVertexIdx(GraphEdgeIdx edgeIdx) const { return GetEdge(edgeIdx).GetStartVertexIdx(); }

	/*!	Retrieves the index of the GraphVertex at the end of the GraphEdge with the specified index value. */
	GraphVertexIdx GetEndVertexIdx(GraphEdgeIdx edgeIdx) const { return GetEdge(edgeIdx).GetEndVertexIdx(); }

	/*!	Retrieves the terrain type of the GraphVertex at the start of the edge with the specified index value. */
	GraphVertexTerrainType GetStartVertexTerrainType(GraphEdgeIdx edgeIdx) const { return GetVertexTerrainType(GetStartVertexIdx(edgeIdx)); }

	/*!	Retrieves the terrain type of the GraphVertex at the end of the edge with the specified index value. */
	GraphVertexTerrainType GetEndVertexTerrainType(GraphEdgeIdx edgeIdx) const { return GetVertexTerrainType(GetEndVertexIdx(edgeIdx)); }

	/*!	Retrieves the coordinates of the GraphVertex at the start of the GraphEdge with the specified index value. */
	const Vec3f& GetStartVertexPosition(GraphEdgeIdx edgeIdx) const { return GetVertexPosition(GetStartVertexIdx(edgeIdx)); }

	/*!	Retrieves the coordinates of the GraphVertex at the end of the GraphEdge with the specified index value. */
	const Vec3f& GetEndVertexPosition(GraphEdgeIdx edgeIdx) const { return GetVertexPosition(GetEndVertexIdx(edgeIdx)); }

	/*!	Retrieves the position of this GraphCell within the partitioning scheme of its Graph. */
	const CellPos& GetCellPos() const { return m_cellPos; }

	/*!	Indicates whether or not the GraphVertex with the specified index lies on the boundary of the GraphCell. */
	bool IsBoundaryVertex(GraphVertexIdx idx) const { return idx < m_interiorVertices.m_first; }

	/*!	For internal use.*/
	KY_FORCE_INLINE const GraphVertex& GetBoundaryVertex(CardinalDir dir, KyUInt32 idx) const
	{
		KY_FUNCTION("GraphCell::GetBoundaryVertex");
		KY_ASSERT(dir < 4, ("Invalid input direction : allowed paramaters are CardinalDir_EAST, NORTH, WEST and SOUTH"));
		KY_ASSERT(idx < m_linkVertices[dir].m_count, ("Invalid Boundary edge index"));

		return GetVertex(m_linkVertices[dir].m_first + idx);
	}

	/*!	For internal use.*/
	KY_FORCE_INLINE GraphVertexIdx GetBoundaryVertexIdx(CardinalDir dir, KyUInt32 idx) const
	{
		KY_FUNCTION("GraphCell::GetBoundaryVertex");
		KY_ASSERT(dir < 4, ("Invalid input direction : allowed paramaters are CardinalDir_EAST, NORTH, WEST and SOUTH"));
		KY_ASSERT(idx < m_linkVertices[dir].m_count, ("Invalid Boundary edge index"));

		return m_linkVertices[dir].m_first + idx;
	}

	/*! For internal use. */
	GraphVertexIdx GetGraphVertexIdxFromInputGraphVertexPos(const Vec3f& kynapsePos, const InputGraphVertexCell& inputGraphVertexCell) const;

public:
	CellPos m_cellPos; /*!< The position of this GraphCell within the partitioning scheme of its Graph. For internal use. Do not modify. */

	Box3f m_box3; /*!< Bounding box of all \GraphVertices. For internal use. Do not modify. */

	BlobArray<GraphVertex> m_vertices; /*!< The list of GraphVertex objects contained in the GraphCell. For internal use. Do not modify. */
	BlobArray<GraphEdge> m_edges;      /*!< The list of GraphEdge objects contained in the GraphCell. For internal use. Do not modify. */

	BlobArray<GraphVertexTerrainType> m_terrainTypes; /*!< The list of terrain types associated to each GraphVertex, if any. Terrain types are optional. If m_terrainTypes.m_count == 0 then no terrain types exist for any vertices. */

	GraphCellVertexRange m_linkVertices[4]; /*! For internal use. Do not modify. */
	GraphCellVertexRange m_interiorVertices; /*! For internal use. Do not modify. */

private:
	GraphCell& operator=(const GraphCell& navMesh);
	GraphCell(const GraphCell& navMesh);
};


/*! \label_swapendianness
	\ingroup kypathdata_graph */
inline void SwapEndianness(Endianness::Target e, GraphCell& self)
{
	SwapEndianness(e, self.m_cellPos);

	SwapEndianness(e, self.m_box3);

	SwapEndianness(e, self.m_vertices);
	SwapEndianness(e, self.m_edges);

	SwapEndianness(e, self.m_terrainTypes);

	SwapEndianness(e, self.m_linkVertices[0]);
	SwapEndianness(e, self.m_linkVertices[1]);
	SwapEndianness(e, self.m_linkVertices[2]);
	SwapEndianness(e, self.m_linkVertices[3]);
	SwapEndianness(e, self.m_interiorVertices);
}


} // namespace Kaim


#endif // KyPathData_GraphCell_H

