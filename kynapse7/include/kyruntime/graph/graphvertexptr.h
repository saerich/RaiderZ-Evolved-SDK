/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphVertexPtr_H
#define KyPathData_GraphVertexPtr_H

#include <kyruntime/graph/stitchedgraphcell.h>
#include <kyruntime/graph/graphcellptr.h>
#include <kyruntime/graph/graphvertexsafeptr.h>

#include <kypathdata/containers/list.h>
#include <kypathdata/graph/graphcell.h>
#include <kypathdata/navmesh/navmeshtypes.h>
#include <kypathdata/pathdata/additionaldata.h>

namespace Kaim
{

class GraphEdgePtr;
class OutEdgeIteratorAroundVertex;
class InEdgeIteratorAroundVertex;


///////////////////////////////////////////////////////////////////////////////////////////
// GraphVertexPtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! This class uniquely identifies a single vertex in a Graph.
	\pp This unique ID is guaranteed to be valid only in the frame in which it was retrieved.
	\pp Never store a GraphVertexPtr for use in subsequent frames, because it has no protection against data
	streaming. Use GraphVertexSafePtr instead.
	\ingroup kyruntime_graph */
class GraphVertexPtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:

	/*!	\label_constructor Creates an invalid GraphVertexPtr. */
	KY_FORCE_INLINE GraphVertexPtr() : m_graphCellPtr(), m_vertexIdx(GraphVertexIdx_Invalid) {}

	/*!	\label_constructor
		\param stitchedGraphCell		A pointer to the StitchedGraphCell that contains this vertex.
		\param vertexIdx				The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE GraphVertexPtr(StitchedGraphCell* stitchedGraphCell, GraphVertexIdx vertexIdx) : m_graphCellPtr(stitchedGraphCell), m_vertexIdx(vertexIdx) {}

	/*!	\label_constructor
		\param stitchedGraphCell		A reference to the StitchedGraphCell that contains this vertex.
		\param vertexIdx				The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE GraphVertexPtr(StitchedGraphCell& stitchedGraphCell, GraphVertexIdx vertexIdx) : m_graphCellPtr(stitchedGraphCell), m_vertexIdx(vertexIdx) {}

	/*!	\label_constructor
		\param cellPtr					A GraphCellPtr that identifies the cell that contains this vertex.
		\param vertexIdx				The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE GraphVertexPtr(const GraphCellPtr& cellPtr, GraphVertexIdx vertexIdx) : m_graphCellPtr(cellPtr), m_vertexIdx(vertexIdx) {}

	/*! \label_constructor Copies the state of the specified GraphVertexPtr to the newly created object. */
	KY_FORCE_INLINE GraphVertexPtr(const GraphVertexPtr& vertexPtr) : m_graphCellPtr(vertexPtr.GetGraphCellPtr()), m_vertexIdx(vertexPtr.GetGraphVertexIdx()) {}


	/*!	\label_reinit
		\param stitchedGraphCell		A pointer to the StitchedGraphCell that contains this vertex.
		\param vertexIdx				The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE void Set(StitchedGraphCell* stitchedGraphCell, GraphVertexIdx vertexIdx) { m_graphCellPtr.Set(stitchedGraphCell); m_vertexIdx = vertexIdx; }

	/*!	\label_reinit
		\param stitchedGraphCell		A reference to the StitchedGraphCell that contains this vertex.
		\param vertexIdx				The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE void Set(StitchedGraphCell& stitchedGraphCell, GraphVertexIdx vertexIdx) { m_graphCellPtr.Set(stitchedGraphCell); m_vertexIdx = vertexIdx; }

	/*!	\label_reinit
		\param cellPtr					A GraphCellPtr that identifies the cell that contains this vertex.
		\param vertexIdx				The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE void Set(const GraphCellPtr& cellPtr, GraphVertexIdx vertexIdx) { m_graphCellPtr = cellPtr; m_vertexIdx = vertexIdx; }

	/*! Returns \c true if this object refers to a valid vertex: i.e. a vertex in a Graph that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_graphCellPtr.IsValid() && m_vertexIdx < m_graphCellPtr.GetGraphCell().GetVertexCount(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_graphCellPtr.Invalidate(); m_vertexIdx = GraphVertexIdx_Invalid; }

	/*!	Returns \c true if this object identifies the same vertex as \c vertexPtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const GraphVertexPtr& vertexPtr) const { return m_vertexIdx == vertexPtr.m_vertexIdx && m_graphCellPtr == vertexPtr.m_graphCellPtr; }

	/*!	Returns \c true if this object identifies a vertex different from the one identified by \c vertexPtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const GraphVertexPtr& vertexPtr) const { return m_vertexIdx != vertexPtr.m_vertexIdx || m_graphCellPtr != vertexPtr.m_graphCellPtr; }

	KY_FORCE_INLINE bool operator < (const GraphVertexPtr& vertexPtr) const { return m_graphCellPtr < vertexPtr.m_graphCellPtr || (m_graphCellPtr == vertexPtr.m_graphCellPtr && m_vertexIdx < vertexPtr.m_vertexIdx); }
	KY_FORCE_INLINE bool operator <=(const GraphVertexPtr& vertexPtr) const { return !(vertexPtr < *this); }

	/*!	Returns a reference to the GraphCellSafePtr that identifies the GraphCell that contains the vertex identified by this object. */
	KY_FORCE_INLINE GraphCellPtr& GetGraphCellPtr() { return m_graphCellPtr; }

	/*!	Returns a const reference to the GraphCellSafePtr that identifies the GraphCell that contains the vertex identified by this object. */
	KY_FORCE_INLINE const GraphCellPtr& GetGraphCellPtr() const { return m_graphCellPtr; }

	/*!	Returns the GraphVertexIdx that identifies this vertex within its GraphCell. */
	KY_FORCE_INLINE GraphVertexIdx GetGraphVertexIdx() const { return m_vertexIdx; }


	/*!	Constructs a new instance of GraphVertexSafePtr that identifies the same vertex as this object.
		If the current instance is invalid, it returns an invalid GraphVertexSafePtr. */
	inline GraphVertexSafePtr ConvertToGraphVertexSafePtr() const
	{
		if (IsValid())
		{
			const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
			return GraphVertexSafePtr(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx(), m_vertexIdx);
		}

		return GraphVertexSafePtr();
	}

	/*!	Updates \c vertexSafePtr to identify the same edge as this object.
		If the current instance is invalid, \c vertexSafePtr is invalidated. */
	inline void ConvertToGraphVertexSafePtr(GraphVertexSafePtr& vertexSafePtr) const
	{
		if (IsValid())
		{
			const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
			vertexSafePtr.Set(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx(), m_vertexIdx);
		}
		else
			vertexSafePtr.Invalidate();
	}

	/*!	Constructs a new instance of GraphVertexSafePtr that identifies the same vertex as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline GraphVertexSafePtr ConvertToGraphVertexSafePtr_UnSafe() const
	{
		const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
		return GraphVertexSafePtr(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx(), m_vertexIdx);
	}

	/*!	Updates \c vertexSafePtr to identify the same vertex as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline void ConvertToGraphVertexSafePtr_UnSafe(GraphVertexSafePtr& vertexSafePtr) const
	{
		const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
		vertexSafePtr.Set(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx(), m_vertexIdx);
	}


	/*!	Returns a const reference to the Graph that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const Graph& GetGraph() const { return m_graphCellPtr.GetGraph(); }

	/*!	Returns a const reference to the StitchedGraphCell that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedGraph& GetStitchedGraph() const { return m_graphCellPtr.GetStitchedGraph(); }

	/*!	Returns a const reference to the StitchedGraphCell that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedGraphCell& GetStitchedGraphCell() const { return m_graphCellPtr.GetStitchedGraphCell(); }
	
	/*!	Returns a const reference to the GraphCell that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphCell& GetGraphCell() const { return m_graphCellPtr.GetGraphCell(); }

	/*!	Returns a const reference to the GraphVertex that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphVertex& GetGraphVertex() const { return GetGraphCell().GetVertex(m_vertexIdx); }



	/*!	Returns a GraphEdgePtr that identifies the edge that goes from the current vertex to the vertex identified by \c end.
		Returns an invalid GraphEdgePtr if no such edge exists, if this object is invalid, or if \c end is invalid. */
	GraphEdgePtr GetOutEdgePtr(const GraphVertexPtr& end) const;

	/*!	Updates \c edgePtr to identify the edge that goes from the current vertex to the vertex identified by \c end.
		Invalidates \c edgePtr if no such edge exists, if this object is invalid, or if \c end is invalid. */
	void GetOutEdgePtr(const GraphVertexPtr& end, GraphEdgePtr& edgePtr) const;


	/*!	Returns a GraphEdgePtr that identifies the edge that goes from the current vertex to the vertex identified by \c endVertexAstarId.
		Returns an invalid GraphEdgePtr if no such edge exists, if this object is invalid, or if \c endVertexAstarId is invalid. */
	GraphEdgePtr GetOutEdgePtr(const AstarId endVertexAstarId) const;

	/*!	Updates \c outputEdgePtr to identify the edge that goes from the current vertex to the vertex identified by \c endVertexAstarId.
		Invalidates \c outputEdgePtr if no such edge exists, if this object is invalid, or if \c endVertexAstarId is invalid. */
	void GetOutEdgePtr(const AstarId endVertexAstarId, GraphEdgePtr& outputEdgePtr) const;

	/*!	Returns a new instance of OutEdgeIteratorAroundVertex that can be used to iterate through the edges that lead away from this vertex.
		\pre	This object must be valid. Its validity is not checked. */
	OutEdgeIteratorAroundVertex GetOutEdgeIterator() const;

	/*!	Returns a new instance of InEdgeIteratorAroundVertex that can be used to iterate through the edges that lead to this vertex.
		\pre	This object must be valid. Its validity is not checked. */
	InEdgeIteratorAroundVertex GetInEdgeIterator() const;


	/*!	Returns a const reference to the GraphManager that maintains the Graph that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphManager& GetGraphManager() const { return m_graphCellPtr.GetGraphManager(); }

	/*!	Returns a const pointer to the GraphManager that maintains the Graph that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphReference* GetGraphRefence() const { return m_graphCellPtr.GetGraphReference(); }


	/*!	Returns the ID of the vertex identified by this object during the last or current A* calculation. For internal use.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE KyUInt32 GetAstarId() const
	{
		KY_FUNCTION("GraphVertexPtr::GetAstarId");
		KY_ASSERT(IsValid(), ("You cannot call GetAstarId on an invalid GraphVertexPtr"));

		return GetStitchedGraphCell().GetAstarId(m_vertexIdx);
	}

	/*!	Sets an ID for the vertex identified by this object, used during A* calculations. For internal use.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE void SetAstarId(AstarId astarId)
	{
		KY_FUNCTION("GraphVertexPtr::GetAstarId");
		KY_ASSERT(IsValid(), ("You cannot call GetAstarId on an invalid GraphVertexPtr"));

		GetStitchedGraphCell().SetAstarId(m_vertexIdx, astarId);
	}

	/*!	Returns a reference to the GraphVertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphVertex& GetVertex() const
	{
		KY_FUNCTION("GraphVertexPtr::GetVertex");
		KY_ASSERT(IsValid(), ("You cannot call GetVertex on an invalid GraphVertexPtr"));

		return GetGraphCell().GetVertex(m_vertexIdx);
	}

	/*!	Returns a const reference to a Vec3f that contains the position of the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const Vec3f& GetPosition() const
	{
		KY_FUNCTION("GraphVertexPtr::GetPosition");
		KY_ASSERT(IsValid(), ("You cannot call GetPosition on an invalid GraphVertexPtr"));

		return GetGraphCell().GetVertexPosition(m_vertexIdx);
	}

	/*!	Returns the NavTerrainTypeMask that indicates the terrain types assigned to the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	NavTerrainType GetVertexTerrainType() const
	{
		KY_FUNCTION("GraphVertexPtr::GetVertexTerrainType");
		KY_ASSERT(IsValid(), ("You cannot call GetVertexTerrainType on an invalid GraphVertexPtr"));

		const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
		const GraphCell& graphCell = *(stitchedGraphCell.m_graphCell);
		const GraphVertex& vertex = graphCell.GetVertex(m_vertexIdx);
		GraphVertexTerrainType terrainType =  graphCell.GetVertexTerrainType(m_vertexIdx);

		if (vertex.IsABoundaryVertex() == false)
			return terrainType;

		const CardinalDir dir = vertex.GetBoundaryDir();
		GraphVertexPtr& twinVertexPtr = stitchedGraphCell.m_links[dir][m_vertexIdx - graphCell.m_linkVertices[dir].m_first];
		if (!twinVertexPtr.IsValid())
			return terrainType;
		return terrainType | twinVertexPtr.GetGraphCell().GetVertexTerrainType(twinVertexPtr.GetGraphVertexIdx());
	}
	
	/*!	Returns a reference to the VertexData associated with the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE VertexData& GetVertexData() const 
	{
		KY_FUNCTION("GraphVertexPtr::GetVertexData");
		KY_ASSERT(IsValid(), ("You cannot call GetVertexData on an invalid GraphVertexPtr"));

		return GetStitchedGraphCell().GetVertexData(m_vertexIdx);
	}

	/*!	Returns true if the vertex identified by this object is a boundary vertex. For internal use.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE bool IsABoundaryVertex() const
	{
		return GetGraphVertex().IsABoundaryVertex();
	}

	/*!	Returns a const reference to the GraphVertexPtr that is the twin of the vertex identified by this object. For internal use.
		\pre	This object must be valid. Its validity is not checked. */
	inline const GraphVertexPtr& GetTwinGraphVertexPtr() const
	{
		KY_FUNCTION("GraphVertexPtr::GetTwinGraphVertexPtr");
		KY_ASSERT(IsValid() && IsABoundaryVertex(), ("You cannot call GetVertexData on an invalid GraphVertexPtr"));

		const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
		const GraphCell& graphCell = *(stitchedGraphCell.m_graphCell);
		const CardinalDir dir = graphCell.GetVertex(m_vertexIdx).GetBoundaryDir();
		return stitchedGraphCell.m_links[dir][m_vertexIdx - graphCell.m_linkVertices[dir].m_first];
	}

	/*!	Retrieves the additional data associated with this vertex, if any.
		\pp See also :USERGUIDE:"Using Additional Data".
		\tparam T	The type of additional data to retrieve.
		\pre		This object must be valid. Its validity is not checked. 
		\pre		The vertex must have had additional data of the specified type created for it when the sector was generated, 
					and that data must have been loaded when the sector was added to the Database.
		\return An instance of the specified type that contains the data stored for this vertex. */
	template <class T>
	T* GetAdditionalData() const
	{
		KY_FUNCTION("GraphVertexPtr::GetAdditionalData");
		KY_ASSERT(GetStitchedGraph().HasVertexAdditionalData(), ("There is no Vertex Additional Data for this graph"));

		VertexAdditionalData<T>* vertexAdditionalData = (VertexAdditionalData<T>*)GetStitchedGraph().m_vertexAdditionalDatas;
		KY_ASSERTK(GetStitchedGraph().GetGuidCompound() == vertexAdditionalData->m_guidCompound);

		StitchedGraphCellIdx cellIndex = m_graphCellPtr.GetStitchedGraphCell().GetStitchedGraphCellIdx();
		return &(vertexAdditionalData->m_dataCells.Values()[cellIndex].Values()[m_vertexIdx]);
	}

private:
	friend class VertexIteratorInWorld;
	GraphCellPtr m_graphCellPtr;
	GraphVertexIdx m_vertexIdx;
};

}

#endif //KyPathData_GraphVertexPtr_H

