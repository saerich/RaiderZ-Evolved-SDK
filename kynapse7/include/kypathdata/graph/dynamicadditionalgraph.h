/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*! \file
	\ingroup kypathdata_graph */

// primary contact: LASI - secondary contact: NONE

#ifndef KyPathData_DynamicAdditionalGraph_H
#define KyPathData_DynamicAdditionalGraph_H

#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/math/box2f.h"
#include "kypathdata/containers/array.h"
#include "kypathdata/graph/dynamicadditionalgraphvertex.h"
#include "kypathdata/generic/kyguid.h"

namespace Kaim
{

class DynamicAdditionalGraphVertex;
class DynamicAdditionalGraphEdge;
struct DynamicAdditionalGraphEdgeInfo;

/*! The DynamicAdditionalGraph class represents an editable spatial graph that you can create
	and add to the PathData for a sector. When the sector is loaded at runtime, the
	DynamicAdditionalGraph is also loaded transparently and stitched into the Graph.
	\pp Edges and vertices in the DynamicAdditionalGraph are referenced by pointers that remain
	valid throughout the graph manipulation.
	\pp For more information, see also :USERGUIDE:"Using Custom Additional Graphs".
	\ingroup kypathdata_graph */
class DynamicAdditionalGraph
{

	KY_DEFINE_NEW_DELETE_OPERATORS
public: 
	/*! Represents a vertex in a DynamicAdditionalGraph. */
	typedef DynamicAdditionalGraphVertex Vertex;
	/*! Represents an edge in a DynamicAdditionalGraph. */
	typedef DynamicAdditionalGraphEdge Edge;
	typedef Array<Vertex*> VertexList;
	typedef Array<Edge*> EdgeList;

public:
	/*! \label_constructor */
	DynamicAdditionalGraph();

	/*! \label_destructor */
	~DynamicAdditionalGraph();

	/*! \label_clear */
	void Clear();

//////////////////
// Build
public:
	/*! Adds a new vertex to the DynamicAdditionalGraph.
		\param position				The position of the vertex in 3D space, in the \SDKName coordinate system.
		\param info					An instance of DynamicAdditionalGraphVertexInfo that provides some additional
									information about the new vertex, most notably the way the vertex will be 
									stitched to the Graph at runtime.
		\return A pointer to the newly created vertex. */
	Vertex* AddVertex(const Vec3f& position, const DynamicAdditionalGraphVertexInfo& info = DynamicAdditionalGraphVertexInfo());

	/*! Removes the specified vertex from the DynamicAdditionalGraph, and removes all of its incoming edges and
		outgoing edges. */
	void RemoveVertex(Vertex& vertex);

	/*! Adds a new edge that connects the specified vertices. Note that the edge is mono-directional, from
		the \c startVertex to the \c endVertex. If you want your characters to be able to plan paths that go
		between the two vertices in either direction, you must add a second edge from the \c endVertex to the
		\c startVertex.
		\param startVertex			The starting vertex for the edge.
		\param endVertex			The ending vertex for the edge.
		\param info					An instance of DynamicAdditionalGraphEdgeInfo that provides some additional information
									to be stored for the new edge.
		\return A pointer to the newly created edge. */
	Edge* AddEdge(Vertex& startVertex, Vertex& endVertex, const DynamicAdditionalGraphEdgeInfo& info);

	/*! Removes the specified edge from the DynamicAdditionalGraph, and updates the lists of incoming and outgoing
		edges maintained by the two vertices connected by the edge. */
	void RemoveEdge(Edge& edge);

	// Access
public:
	/*! Retrieves the number of edges in the DynamicAdditionalGraph. */
	KyUInt32 GetEdgeCount() const { return m_edges.GetCount(); }

	/*! Retrieves the first edge in the DynamicAdditionalGraph. */
	EdgeList::ConstIterator EdgeBegin() const { return m_edges.Begin(); }

	/*! Retrieves the first edge in the DynamicAdditionalGraph. */
	EdgeList::Iterator EdgeBegin() { return m_edges.Begin(); }

	/*! Retrieves the last edge in the DynamicAdditionalGraph. */
	EdgeList::ConstIterator EdgeEnd() const { return m_edges.End(); }

	/*! Retrieves the last edge in the DynamicAdditionalGraph. */
	EdgeList::Iterator EdgeEnd() { return m_edges.End(); }

	/*! Retrieves the number of vertices in the DynamicAdditionalGraph. */
	KyUInt32 GetVertexCount() const { return m_vertices.GetCount(); }

	/*! Retrieves the first vertex in the DynamicAdditionalGraph. */
	VertexList::ConstIterator VertexBegin() const { return m_vertices.Begin(); }

	/*! Retrieves the first vertex in the DynamicAdditionalGraph. */
	VertexList::Iterator VertexBegin() { return m_vertices.Begin(); }

	/*! Retrieves the last vertex in the DynamicAdditionalGraph. */
	VertexList::ConstIterator VertexEnd() const { return m_vertices.End(); }

	/*! Retrieves the last vertex in the DynamicAdditionalGraph. */
	VertexList::Iterator VertexEnd() { return m_vertices.End(); }

	/*! Retrieves the GUID assigned to this DynamicAdditionalGraph. */ 
	const KyGuid& GetGuid() const { return m_guid; }

	/*! Sets the GUID for this DynamicAdditionalGraph. */ 
	void SetGuid(const KyGuid& guid) { m_guid = guid; }

	/*! Indicates whether or not a specific GUID has been set for this DynamicAdditionalGraph. */ 
	bool IsGuidSet() const { return m_guid != KyGuid::s_defaultGuid; }

private:
	void DestroyIncomingEdgePair(Vertex& vertex);
	void DestroyOutGoingEdgePair(Vertex& vertex);
private:
	VertexList m_vertices; //< Graph Vertices.
	EdgeList m_edges; //< Graph Edges.
	KyGuid m_guid; //< Graph guid.
};

} // namespace Kaim

#endif // #ifndef KyPathData_DynamicAdditionalGraph_H
