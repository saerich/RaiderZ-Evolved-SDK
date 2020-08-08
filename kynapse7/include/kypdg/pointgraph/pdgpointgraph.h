/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: LASI - secondary contact: JODA

#ifndef KyPdg_PdgPointGraph_H
#define KyPdg_PdgPointGraph_H

#include <kypathdata/generic/memory.h>
#include "kypathdata/math/cardinaldir.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/math/box2f.h"
#include "kypathdata/containers/multiset.h"
#include "kypathdata/containers/array.h"

#include "kypdg/pointgraph/dynamicpointgraphbuilder.h"
#include "kypdg/pointgraph/pdginputgraphvertex.h"

#include <kypdg/common/stlvector.h>


namespace Kaim
{

class PdgGraphEdge;
class PdgGraphVertex;

/*
This class contains an editable pointgraph and provides all methods
to create a pointgraph from scratch. Edges and vertice are referenced
by a pointer which remain valid throughout the graph manipulation.
*/
class PdgPointGraph
{
	friend class PointGraphCellBlobBuilder;
	friend class DynamicPointGraphSimplifier;

	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	typedef Array<PdgGraphVertex*>::ConstIterator VertexPtrConstIterator;
	typedef Array<PdgGraphVertex*>::Iterator VertexPtrIterator;
	typedef Array<PdgGraphEdge*>::ConstIterator EdgePtrConstIterator;
	typedef Array<PdgGraphEdge*>::Iterator EdgePtrIterator;

public:
	explicit PdgPointGraph(KyUInt32 nbEdgesByPoolNode = 8, KyUInt32 nbNodeByChunk = 32);
	~PdgPointGraph();
	void Clear();

private:
	PdgPointGraph(/*const*/ PdgPointGraph& rhs); // no implementaion
	PdgPointGraph& operator = (/*const*/ PdgPointGraph& rhs); // no implementation

//////////////////
// Build
public:
	/* Add a vertex.
		\return index of the vertex. */
	PdgGraphVertex* AddVertex(const Vec3f& position, GraphVertexTerrainType terrainType = GraphVertexTerrainType_Default);
	PdgGraphVertex* AddVertex();
	void RemoveVertex(PdgGraphVertex& vertex);
	void LazyRemoveVertex(PdgGraphVertex& vertex);
	VertexPtrIterator RemoveVertex(VertexPtrIterator it);

	/* After some LazyRemove, remove effectively vertices/edges marked as removed. */
	void Repack();

	void CopyAllButCoverage(PdgPointGraph& rhs);

	enum FindOrAddResult
	{
		Found,
		Added
	};

	struct AsymmetricCandidate
	{
		AsymmetricCandidate(const Vec3f& start, const Vec3f& end)
		{
			m_startPos = start;
			m_endPos = end;
		}
		Vec3f m_startPos; //starting position of the asymmetric connection
		Vec3f m_endPos; //ending position of the asymmetric connection
	};

	/*! Returns whether a vertex was found at provided position, or if not add one. In both cases, returns the vertex in the passed vert param. */
	FindOrAddResult FindOrAddVertex(const Vec3f& position, PdgGraphVertex*& vert, GraphVertexTerrainType terrainType = GraphVertexTerrainType_Default);

	PdgGraphVertex* AddCellConnectionVertex(const Vec3f& position, const GraphVertexTerrainType& terrainType);

	/* Add an edge.
		\return index of the edge. */
private:
	PdgGraphEdge* AddEdge(PdgGraphVertex& startVertex, PdgGraphVertex& endVertex);
public:
	PdgGraphEdge* AddEdgeNoDuplicate(PdgGraphVertex& startVertex, PdgGraphVertex& endVertex);

	/* Remove an edge. */
	void RemoveEdge(PdgGraphEdge& edge);

	/* Remove edges. */
	void RemoveEdges(const Array<PdgGraphEdge*>& edges);

	/* Change start vertex in edges.*/
	void ChangeEdgesStartVertexes(Array<PdgGraphEdge*>::ConstIterator begin, Array<PdgGraphEdge*>::ConstIterator end, PdgGraphVertex& newVertex);
	/* Change end vertex in edges.*/
	void ChangeEdgesEndVertexes(Array<PdgGraphEdge*>::ConstIterator begin, Array<PdgGraphEdge*>::ConstIterator end, PdgGraphVertex& newVertex);

	/* Make sure all edges are double-sided. */
	void DoubleSideAllEdges();

	void CollapseEdge_ToEnd(PdgGraphEdge& edge);
	void CollapseEdge_ToEnd(PdgGraphEdge& edgeToCollapse, Array<PdgGraphEdge*>& toRemove, Array<PdgGraphEdge*>& toUpdate);

	CardinalDir GetBoundaryDirFromPos(const Vec3f& pos) const;
	void BoundPositionFromCardinalDir(Vec3f& pos, CardinalDir cardinalDir) const;

	void ComputeBox3(Box3f& box3) const;

	void SetBox2(const Box2f& box) { m_box2 = box; }
	const Box2f& GetBox2() const { return m_box2; }

	void SortAndComputeIndices();
	void BuildInputVertices();
	// Access
public:
	/* Get number of edges. */
	KyUInt32 GetEdgeCount() const { return m_edges.GetCount(); }
	const PdgGraphEdge* GetEdge(KyUInt32 index) const { return m_edges[index]; }
	EdgePtrConstIterator EdgeBegin() const { return m_edges.Begin(); }
	EdgePtrIterator EdgeBegin() { return m_edges.Begin(); }
	EdgePtrConstIterator EdgeEnd() const { return m_edges.End(); }
	EdgePtrIterator EdgeEnd() { return m_edges.End(); }

	/* Get number of vertices. */
	KyUInt32 GetVertexCount() const { return m_vertices.GetCount(); }
	const PdgGraphVertex* GetVertex(KyUInt32 index) const { return m_vertices[index]; }
	PdgGraphVertex* GetVertex(KyUInt32 index) { return m_vertices[index]; }
	VertexPtrConstIterator VertexBegin() const { return m_vertices.Begin(); }
	VertexPtrIterator VertexBegin() { return m_vertices.Begin(); }
	VertexPtrConstIterator VertexEnd() const { return m_vertices.End(); }
	VertexPtrIterator VertexEnd() { return m_vertices.End(); }

	bool IsValid() const;

	bool HasDuplicatedVertices() const;
	bool HasInvalidEdges() const;
	bool HasDuplicatedEdges() const;

private:
	void DestroyIncomingEdgePair(PdgGraphVertex& vertex);
	void DestroyOutGoingEdgePair(PdgGraphVertex& vertex);
	void LazyDestroyEdgesOf(PdgGraphVertex& vertex);

	MultiSet<PdgGraphEdge*>::PoolType& GetIncomingOutgoingEdgePtrPool() { return m_edgePtrPool; }
private:
	MultiSet<PdgGraphEdge*>::PoolType m_edgePtrPool; // for incoming/outgoing
	Box2f m_box2;
	Array<PdgGraphVertex*> m_vertices; //!< Graph Vertices.
	Array<PdgGraphEdge*> m_edges; //!< Graph Edges.
public:
	Array<PdgInputGraphVertex> m_pdgInputVertices;
	Array<InputGraphVertex>* m_inputVertices;
	StlVector<AsymmetricCandidate> m_asymmetricCandidates;
};

} // namespace Kaim

#endif // #ifndef KyPdg_PdgPointGraph_H
