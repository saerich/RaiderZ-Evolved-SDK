/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: LASI
#ifndef KyPdg_DynamicNavFloor_H
#define KyPdg_DynamicNavFloor_H

#include <kypathdata/generic/memory.h>
#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/math/box2f.h"
#include "kypdg/generator/pdgsystem.h"

#include <kypdg/common/stlvector.h>
#include <assert.h>
#include "kypdg/pdginput/indexedtrianglesoup.h"

namespace Kaim
{

typedef KyUInt32 DynamicNavTerrainType;
struct DynamicFloorLink
{
	KyUInt32 m_edgeIdx;
	KyUInt32 m_floorIdx;
};

// ----------------------------
// ----- DynamicNavVertex -----
// ----------------------------
class DynamicNavVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	DynamicNavVertex() : m_position(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL), m_first(KyUInt32MAXVAL) {}
	DynamicNavVertex(const Vec3f& pos) : m_position(pos), m_first(KyUInt32MAXVAL) {}

public:
	Vec3f m_position;
	KyUInt32 m_first; // first outgoing edge
};

// ------------------------------
// ----- DynamicNavTriangle -----
// ------------------------------
class DynamicNavTriangle
{
public:
	DynamicNavTriangle() : m_edge(KyUInt32MAXVAL) {}
	DynamicNavTriangle(KyUInt32 edge) : m_edge(edge) {}

public:
	KyUInt32 m_edge;	 // one edge around the face
};

// --------------------------
// ----- DynamicNavHalfEdge -----
// --------------------------

enum DynamicNavHalfEdgeType
{
	DynamicNavHalfEdgeType_Normal = 0,
	DynamicNavHalfEdgeType_Obstacle = 1,
	DynamicNavHalfEdgeType_FloorBoundary = 2,
	DynamicNavHalfEdgeType_CellBoundary = 3,

	DynamicNavHalfEdgeType_Invalid = 0xFFFFFFFF
};

class DynamicNavHalfEdge
{
public:
	DynamicNavHalfEdge() : m_idx(KyUInt32MAXVAL), m_start(KyUInt32MAXVAL), m_end(KyUInt32MAXVAL), m_face(KyUInt32MAXVAL),
		m_next(KyUInt32MAXVAL), m_type(DynamicNavHalfEdgeType_Invalid), m_pair(KyUInt32MAXVAL), m_cardinalDir(CardinalDir_INVALID) {}
public:
	KyUInt32 m_idx;
	KyUInt32 m_start; // start vertex
	KyUInt32 m_end; // end vertex (added by LS for dynamicFloorBuilder)
	KyUInt32 m_face; // the adjacent face
	KyUInt32 m_next; // next edge
	DynamicNavHalfEdgeType m_type; // 0 = normal, 1 = obstacle, 2 = Floor boundary, 3 = cell boundary
	KyUInt32 m_pair; // pair edge idx if m_type == 0, floorLink idx if m_type == Floor boundary, if m_type == = cell boundary : the idx this edge among all the halfEdges that are the same boundary of the cell
	CardinalDir m_cardinalDir; // use for edge of type == 3 (cell boundary) only. The cardinal dir of the side of the cell
};

// --------------------------
// ----- DynamicNavFloor -----
// --------------------------
class DynamicNavFloor
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	DynamicNavFloor(PdgSystem* sys = KY_NULL) : m_sys(sys), m_idx(KyUInt32MAXVAL), m_terrainType(KyUInt32MAXVAL), m_originalPixelColor(KyUInt32MAXVAL) {}

	KyUInt32 AddVertex(const Vec3f& position);
	KyUInt32 AddTriangle(KyUInt32 idxV1, KyUInt32 idxV2, KyUInt32 idxV3);
	//added by LS.
	KyUInt32 AddHalfEdge(KyUInt32 start, KyUInt32 end, KyUInt32 face, DynamicNavHalfEdgeType type = DynamicNavHalfEdgeType_Normal);
	KyResult MakeEdgesOpposite(KyUInt32 idxEdge1, KyUInt32 idxEdge2);

	// ---- Edge -----
	KyUInt32 GetNextEdgeIdx(KyUInt32 edgeIdx)     const { assert(edgeIdx < m_edges.size()); return m_edges[edgeIdx].m_next; }
	KyUInt32 GetPrevEdgeIdx(KyUInt32 edgeIdx)     const { assert(edgeIdx < m_edges.size()); return GetNextEdgeIdx(GetNextEdgeIdx(edgeIdx)); }
	KyUInt32 GetOppositeEdgeIdx(KyUInt32 edgeIdx) const { assert(edgeIdx < m_edges.size()); return m_edges[edgeIdx].m_pair; }
	KyUInt32 GetStartVertexIdx(KyUInt32 edgeIdx)  const { assert(edgeIdx < m_edges.size()); return m_edges[edgeIdx].m_start; }
	KyUInt32 GetFaceIdx(KyUInt32 edgeIdx)         const { assert(edgeIdx < m_edges.size()); return m_edges[edgeIdx].m_face; }

	// ---- Face -----
	KyUInt32 GetEdgeIdx(KyUInt32 triangleIdx) const { assert(triangleIdx < m_triangles.size()); return m_triangles[triangleIdx].m_edge; }

	// ---- Vertex -----
	KyUInt32 GetFirstEdgeIdx(KyUInt32 vertexIdx) const { assert(vertexIdx < m_vertices.size()); return m_vertices[vertexIdx].m_first; }
	Vec3f& GetVertexPosition(KyUInt32 vertexIdx)             { assert(vertexIdx < m_vertices.size()); return m_vertices[vertexIdx].m_position; }
	const Vec3f& GetVertexPosition(KyUInt32 vertexIdx) const { assert(vertexIdx < m_vertices.size()); return m_vertices[vertexIdx].m_position; }

	KyUInt32 ComputeNextBorderEdgeIdx(KyUInt32 edgeIdx)  const;

	void Clear()
	{
		m_vertices.clear();
		m_triangles.clear();
		m_edges.clear();
	}

	void BuildAABBox()
	{
		m_aabbox.Clear();

		for (KyUInt32 i = 0; i < m_vertices.size(); ++i)
			m_aabbox.ExpandByVec3(m_vertices[i].m_position);
	}

public:
	PdgSystem* m_sys;

	KyUInt32 m_idx;

	StlVector<DynamicNavTriangle> m_triangles;
	StlVector<DynamicNavVertex> m_vertices;
	StlVector<DynamicNavHalfEdge> m_edges;

	StlVector<DynamicFloorLink> m_links;

	IndexedTriangleSoup m_triangleSoup;

	DynamicNavTerrainType m_terrainType;
	KyUInt32 m_originalPixelColor;

	Box3f m_aabbox;
};

}


#endif //KyPdg_DynamicNavFloor_H

