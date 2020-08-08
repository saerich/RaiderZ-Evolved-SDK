/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: LASI
#ifndef KyPdg_DynamicTriangulation_H
#define KyPdg_DynamicTriangulation_H

#include <kypathdata/generic/memory.h>
#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/containers/multiset.h"
#include "kypdg/common/stlvector.h"
#include "kypathdata/containers/chunkbasedpool.h"

namespace Kaim
{

class DynamicHalfEdge;
class IndexedTriangleSoup;
template <typename T>
class Array;

// ----------------------------
// ----- DynamicVertex --------
// ----------------------------
class DynamicVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	DynamicVertex() : m_idx(KyUInt32MAXVAL), m_position(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL), m_userData(KY_NULL) {}


	void InsertInOutGoingEdges(DynamicHalfEdge* dynamicHalfEdge)
	{
		KY_FUNCTION("DynamicVertex::InsertInOutGoingEdges");
		KY_ASSERTK(m_outGoingEdges.DoesContain(dynamicHalfEdge) == false);
		m_outGoingEdges.Insert(dynamicHalfEdge);
		KY_ASSERTK(m_outGoingEdges.DoesContain(dynamicHalfEdge));
	}

	void RemoveFromOutGoingEdges(DynamicHalfEdge* dynamicHalfEdge)
	{
		KY_FUNCTION("DynamicVertex::RemoveFromOutGoingEdges");
		KY_ASSERTK(m_outGoingEdges.DoesContain(dynamicHalfEdge));
		m_outGoingEdges.RemoveFirst(dynamicHalfEdge);
		KY_ASSERTK(m_outGoingEdges.DoesContain(dynamicHalfEdge) == false);
	}

public:
	KyUInt32 m_idx;
	Vec3f m_position;
	MultiSet<DynamicHalfEdge*> m_outGoingEdges;

	void* m_userData;
};

// ------------------------------
// ----- DynamicTriangle --------
// ------------------------------
class DynamicTriangle
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	DynamicTriangle() : m_idx(KyUInt32MAXVAL), m_edge(KY_NULL), m_userData(KY_NULL) {}
public:
	KyUInt32 m_idx;
	DynamicHalfEdge* m_edge; // one edge around the face

	void* m_userData;
};

// ---------------------------
// ----- DynamicHalfEdge -----
// ---------------------------
class DynamicHalfEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	DynamicHalfEdge() : m_idx(KyUInt32MAXVAL), m_start(KY_NULL), m_triangle(KY_NULL),
		m_next(KY_NULL), m_pair(KY_NULL), m_status(HalfEdgeStatus_NotOpen), m_userData(KY_NULL) {}

	enum HalfEdgeStatus
	{
		HalfEdgeStatus_Open = 0,
		HalfEdgeStatus_NotOpen = 1
	};

	KY_FORCE_INLINE bool IsABorder() const { return m_pair == KY_NULL; }

public:
	KyUInt32 m_idx;
	DynamicVertex* m_start;
	DynamicTriangle* m_triangle;
	DynamicHalfEdge* m_next;
	DynamicHalfEdge* m_pair;
	HalfEdgeStatus m_status;

	void* m_userData;
};

/*	Instance of this class are returned by DynamicTriangulation::InsertANewVertexInMeshAndUpdateTriangulation
	if m_result == RESULT_NEW_VERTEX_CREATED => m_vertex refers to the new created vertex
	if m_result == RESULT_CLOSE_TO_EXISTING_VERTEX => m_vertex refers to the vertex we are close to
	if m_result == RESULT_CLOSE_TO_BORDER_EDGE => m_ halfEdge refers to the border HalfEdge we are close to
	if m_result == RESULT_NOT_IN_TRIANGLE => no triangle contains input position. No usefull datan, m_vertex and m_halfEdge remains to KY_NULL
	if m_result == RESULT_REFUSED => position refused becaus of floating precision issue (we don't want to create reverse face). No usefull data,  m_vertex and m_halfEdge remains to KY_NULL
*/

class DynTriangulationInsertionResult
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:

	DynTriangulationInsertionResult() : m_result(RESULT_FORCE32), m_vertex(KY_NULL), m_halfEdge(KY_NULL) {}

	enum Result
	{
		RESULT_NEW_VERTEX_CREATED = 0,
		RESULT_CLOSE_TO_EXISTING_VERTEX = 1,
		RESULT_CLOSE_TO_BORDER_EDGE = 2,
		RESULT_NOT_IN_TRIANGLE = 3,
		RESULT_REFUSED = 4,
		RESULT_FORCE32 = 0xFFFFFFFF
	};

	Result m_result;
	DynamicVertex* m_vertex;
	DynamicHalfEdge* m_halfEdge;
};


// --------------------------------
// ----- DynamicTriangulation -----
// --------------------------------
class DynamicTriangulation
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	DynamicTriangulation() : m_squareMinDistFromExistingPoint(0.0001f), m_squareMinDistFromBorder(0.0001f), m_maxSquareDistToConsiderAPointOnAnEdge(0.0001f),
		m_maxNormalEdgeLength(0.3f), m_maxBorderEdgeLength(0.3f)
	{
		m_poolForOutGoingEdges.AddNodePool(8, 128);
	}

	~DynamicTriangulation();

	void Clear();


	KY_FORCE_INLINE void SetMinDistFromBorder(KyFloat32 minDistFromBorder) { m_squareMinDistFromBorder = minDistFromBorder * minDistFromBorder; }
	KY_FORCE_INLINE void SetMinDistFromExistingPoint(KyFloat32 minDistFromExistingPoint) { m_squareMinDistFromExistingPoint = minDistFromExistingPoint * minDistFromExistingPoint; }
	KY_FORCE_INLINE void SetMaxDistToConsiderAPointOnAnEdge(KyFloat32 maxDistToConsiderAPointOnAnEdge) { m_maxSquareDistToConsiderAPointOnAnEdge = maxDistToConsiderAPointOnAnEdge * maxDistToConsiderAPointOnAnEdge; }


	// mesh Creation
	DynamicVertex* AddVertex(const Vec3f& position);
	DynamicTriangle* AddTriangle(DynamicVertex& v0, DynamicVertex& v1, DynamicVertex& v2);
	KyResult BuildFromIndexedTriangleSoup(const IndexedTriangleSoup* triangulation);
	KyResult BuildIndexedTriangleSoup(IndexedTriangleSoup* triangulation);

	// Process the triangulation to get a "Delauney Like" triangulation
	// This function should be called before inserting new vertices and Updating Triangulation
	void GetABetterTriangulation();

	/*-------------- Functions used for better altitude approximation -------------- */

	// Add a new vertex in the Mesh and triangulate around him
	// return KY_NULL if
	//     - the position isn't in any triangle
	//     - the position is too close to one of the three vertices of the triangle (according to m_squareMinDistFromExistingPoint, distances are in 2D)
	//     - the position is too close to a border edge (according to m_squareMinDistFromBorder, distances are in 2D)
	// If the posisition is close to an edge (according to m_maxSquareDistToConsiderAPointOnAnEdge, distances are in 2D), the vertex is snapped on the edge
	DynamicVertex* InsertANewVertexInMesh(const Vec3f& position, DynTriangulationInsertionResult& result);

	// perform an InsertANewVertexInMesh and update the triangulation to get a "Delauney Like" triangulation
	// if impactedTrianglesIndices is not null, fill it with the indices of modified triangles
	DynTriangulationInsertionResult InsertANewVertexInMeshAndUpdateTriangulation(const Vec3f& position, Array<KyUInt32>* impactedTrianglesIndices = KY_NULL);
	DynamicVertex* InsertANewVertexOnBorderAndUpdateTriangulation(const Vec3f& position, Array<KyUInt32>* impactedTrianglesIndices = KY_NULL);

	/*-------------- Functions used for better triangulation to create the Graph -------------- */

	void SetMaxNormalEdgeLength(KyFloat32 maxNormalEdgeLength ) { m_maxNormalEdgeLength = maxNormalEdgeLength; }
	void SetMaxBorderEdgeLength(KyFloat32 maxBorderEdgeLength ) { m_maxBorderEdgeLength = maxBorderEdgeLength; }

	void SplitBorderEdges();
	void SplitNormalEdges();


private:
	KyResult FlipEdge(DynamicHalfEdge* halfEdge);


	void UpdateTriangulationAfterVertexInsertion(DynamicVertex* newVertex);

	bool IsEdgeLegal(DynamicHalfEdge* halfEdge);
	bool IsEdgeLegalAfterPointInsertion(DynamicHalfEdge* halfEdge, DynamicVertex* newVertex);

	// dynamic mesh modification
	DynamicTriangle* FindTriangleThatContainsAPosition(const Vec3f& position, DynTriangulationInsertionResult& result);

	void InsertANewVertexInsideATriangle(DynamicVertex* newVertex, DynamicTriangle* triangle);
	void InsertANewVertexInTheMiddleOfAnEdge(DynamicVertex* newVertex, DynamicHalfEdge* halfEdge);
	void InsertANewVertexInTheMiddleOfABorderEdge(DynamicVertex* newVertex, DynamicHalfEdge* halfEdge);


	void AddToEdgeToProcessIfNotOpen(DynamicHalfEdge* halfEdge);
	void AddToEdgeToProcessIfIllegalAndNotOpen(DynamicHalfEdge* halfEdge);

private :
	DynamicVertex* GetNewDynamicVertex();
	DynamicHalfEdge* GetNewDynamicHalfEdge();
	DynamicTriangle* GetNewDynamicTriangle();

	void ReleaseDynamicVertex(DynamicVertex* vertex)
	{
		m_poolOfVertices.Delete(ChunkBasedPool<DynamicVertex>::Handle(vertex));
	}

	void ReleaseDynamicHalfEdge(DynamicHalfEdge* halfEdge)
	{
		m_poolOfHalfEdges.Delete(ChunkBasedPool<DynamicHalfEdge>::Handle(halfEdge));
	}
	
	void ReleaseDynamicTriangle(DynamicTriangle* triangle)
	{
		m_poolOfTriangles.Delete(ChunkBasedPool<DynamicTriangle>::Handle(triangle));
	}


public:
	StlVector<DynamicTriangle*> m_triangles;
	StlVector<DynamicVertex*> m_vertices;
	StlVector<DynamicHalfEdge*> m_edges;

	StlVector<DynamicHalfEdge*> m_edgeToProcess;

	KyFloat32 m_squareMinDistFromExistingPoint;

	// we should have m_maxSquareDistToConsiderAPointOnAnEdge <= m_squareMinDistFromBorder
	KyFloat32 m_squareMinDistFromBorder;
	KyFloat32 m_maxSquareDistToConsiderAPointOnAnEdge;

	KyFloat32 m_maxNormalEdgeLength;
	KyFloat32 m_maxBorderEdgeLength;

private:
	MultiSet<DynamicHalfEdge*>::PoolType m_poolForOutGoingEdges;

	ChunkBasedPool<DynamicVertex> m_poolOfVertices;
	ChunkBasedPool<DynamicHalfEdge> m_poolOfHalfEdges;
	ChunkBasedPool<DynamicTriangle> m_poolOfTriangles;
};

}


#endif //KyPdg_DynamicTriangulation_H

