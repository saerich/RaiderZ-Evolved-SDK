/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: LAPA
#ifndef KyPathData_GraphVertexSafePtr_H
#define KyPathData_GraphVertexSafePtr_H

#include "kypathdata/containers/array.h"
#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/navmesh/navmeshtypes.h"
#include "kyruntime/graph/stitchedgraphcell.h"
#include "kyruntime/graph/graphcellsafeptr.h"


namespace Kaim
{

class GraphVertexPtr;
class GraphCell;
class GraphEdgeSafePtr;
class GraphVertex;
class VertexData;

///////////////////////////////////////////////////////////////////////////////////////////
// GraphVertexSafePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! This class uniquely identifies a single vertex in a Graph.
	\pp Instances of this class can be safely stored and used in subsequent frames, unlike GraphVertexPtr.
	\pp When you are finished with an instance of this class, you must call Invalidate() to free its allocated resources. This
	must be done before you destroy the World.
	\ingroup kyruntime_graph */
class GraphVertexSafePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid GraphVertexSafePtr. */
	KY_FORCE_INLINE GraphVertexSafePtr() : m_graphCellSafePtr(), m_vertexIdx(GraphVertexIdx_Invalid) {}
	
	/*!	\label_constructor
		\param navMeshReference		A pointer to the GraphReference of the Graph that contains this vertex.
		\param stitchedGraphCellIdx	The StitchedGraphCellIdx that identifies the GraphCell that contains this vertex.
		\param vertexIdx			The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE GraphVertexSafePtr(GraphReference* navMeshReference, StitchedGraphCellIdx stitchedGraphCellIdx, GraphVertexIdx vertexIdx) :
		m_graphCellSafePtr(navMeshReference, stitchedGraphCellIdx), m_vertexIdx(vertexIdx) {}

	/*!	\label_constructor
		\param navMeshReference		A reference to the GraphReference of the Graph that contains this vertex.
		\param stitchedGraphCellIdx	The StitchedGraphCellIdx that identifies the GraphCell that contains this vertex.
		\param vertexIdx			The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE GraphVertexSafePtr(GraphReference& navMeshReference, StitchedGraphCellIdx stitchedGraphCellIdx, GraphVertexIdx vertexIdx) :
		m_graphCellSafePtr(navMeshReference, stitchedGraphCellIdx), m_vertexIdx(vertexIdx) {}
	
	/*!	\label_constructor
		\param cellSafePtr			A GraphCellSafePtr that identifies the GraphCell that contains this vertex.
		\param vertexIdx			The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE GraphVertexSafePtr(const GraphCellSafePtr& cellSafePtr, GraphVertexIdx vertexIdx) : m_graphCellSafePtr(cellSafePtr), m_vertexIdx(vertexIdx) {}

	/*! \label_constructor Copies the state of the specified GraphVertexSafePtr to this object. */
	KY_FORCE_INLINE GraphVertexSafePtr(const GraphVertexSafePtr& graphVertexSafePtr) : m_graphCellSafePtr(graphVertexSafePtr.GetGraphCellSafePtr()), m_vertexIdx(graphVertexSafePtr.GetGraphVertexIdx()) {}

	/*!	\label_reinit
		\param navMeshReference			A pointer to the GraphReference of the Graph that contains this vertex.
		\param stitchedGraphCellSafePtr	The StitchedGraphCellIdx that identifies the GraphCell that contains this vertex.
		\param vertexIdx				The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE void Set(GraphReference* navMeshReference, StitchedGraphCellIdx stitchedGraphCellSafePtr, GraphVertexIdx vertexIdx)
	{
		m_graphCellSafePtr.Set(navMeshReference, stitchedGraphCellSafePtr);
		m_vertexIdx = vertexIdx;
	}

	/*!	\label_reinit
		\param navMeshReference			A reference to the GraphReference of the Graph that contains this vertex.
		\param stitchedGraphCellSafePtr	The StitchedGraphCellIdx that identifies the GraphCell that contains this vertex.
		\param vertexIdx				The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE void Set(GraphReference& navMeshReference, StitchedGraphCellIdx stitchedGraphCellSafePtr, GraphVertexIdx vertexIdx)
	{
		m_graphCellSafePtr.Set(navMeshReference, stitchedGraphCellSafePtr);
		m_vertexIdx = vertexIdx;
	}

	/*!	\label_reinit
		\param cellSafePtr			A GraphCellSafePtr that identifies the GraphCell that contains this vertex.
		\param vertexIdx			The index of the vertex within its GraphCell. */
	KY_FORCE_INLINE void Set(const GraphCellSafePtr& cellSafePtr, GraphVertexIdx vertexIdx)
	{
		m_graphCellSafePtr = cellSafePtr;
		m_vertexIdx = vertexIdx;
	}

	/*! Returns \c true if this object refers to a valid edge: i.e. an edge in a Graph that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_graphCellSafePtr.IsValid() && m_vertexIdx < m_graphCellSafePtr.GetGraphCell_UnSafe().GetVertexCount(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_graphCellSafePtr.Invalidate(); m_vertexIdx = GraphVertexIdx_Invalid; }

	/*!	Returns \c true if this object identifies the same vertex as \c graphVertexSafePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const GraphVertexSafePtr& graphVertexSafePtr) const { return m_vertexIdx == graphVertexSafePtr.m_vertexIdx && m_graphCellSafePtr == graphVertexSafePtr.m_graphCellSafePtr; }

	/*!	Returns \c true if this object identifies a vertex different from the one identified by \c graphVertexSafePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const GraphVertexSafePtr& graphVertexSafePtr) const { return m_vertexIdx != graphVertexSafePtr.m_vertexIdx || m_graphCellSafePtr != graphVertexSafePtr.m_graphCellSafePtr; }

	KY_FORCE_INLINE bool operator < (const GraphVertexSafePtr& vertexSafePtr) const { return m_graphCellSafePtr < vertexSafePtr.m_graphCellSafePtr || (m_graphCellSafePtr == vertexSafePtr.m_graphCellSafePtr && m_vertexIdx < vertexSafePtr.m_vertexIdx); }
	KY_FORCE_INLINE bool operator <=(const GraphVertexSafePtr& vertexSafePtr) const { return !(vertexSafePtr < *this); }

	/*!	Returns a GraphVertexIdx that identifies this vertex within its GraphCell. */
	KY_FORCE_INLINE GraphVertexIdx GetGraphVertexIdx() const { return m_vertexIdx; }

	/*!	Returns a reference to the GraphCellSafePtr that identifies the GraphCell that contains the vertex identified by this object.
		If this object is invalid, returns an invalid GraphCellSafePtr. */
	KY_FORCE_INLINE GraphCellSafePtr& GetGraphCellSafePtr() { return m_graphCellSafePtr; }

	/*!	Returns aconst reference to the GraphCellSafePtr that identifies the GraphCell that contains the vertex identified by this object.
		If this object is invalid, returns an invalid GraphCellSafePtr. */
	KY_FORCE_INLINE const GraphCellSafePtr& GetGraphCellSafePtr() const { return m_graphCellSafePtr; }

	/*!	Constructs a new instance of GraphVertexPtr that identifies the same vertex as this object.
		If this object is invalid, returns an invalid GraphVertexPtr. */
	GraphVertexPtr ConvertToGraphVertexPtr() const;

	/*!	Updates \c vertexPtr to identify the same vertex as this object.
		If this object is invalid, \c vertexPtr is invalidated. */
	void ConvertToGraphVertexPtr(GraphVertexPtr& vertexPtr) const;

	/*!	Constructs a new instance of GraphVertexPtr that identifies the same vertex as this object.
		\pre	This object must be valid. Its validity is not checked. */
	GraphVertexPtr ConvertToGraphVertexPtr_UnSafe() const;

	/*!	Updates \c vertexPtr to identify the same vertex as this object.
		\pre	This object must be valid. Its validity is not checked. */
	void ConvertToGraphVertexPtr_UnSafe(GraphVertexPtr& vertexPtr) const;

	
	/*!	Returns a const pointer to the StitchedGraphCell that contains the vertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE StitchedGraphCell* GetStitchedGraphCell() const { return IsValid() ? &m_graphCellSafePtr.GetStitchedGraphCell_UnSafe() : KY_NULL; }

	/*!	Returns a const reference to the StitchedGraphCell that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedGraphCell& GetStitchedGraphCell_UnSafe() const { return m_graphCellSafePtr.GetStitchedGraphCell_UnSafe(); }


	/*!	Returns a const pointer to the Graph that contains the vertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const Graph* GetGraph() const { return IsValid() ? &(GetGraph_UnSafe()) : KY_NULL; }

	/*!	Returns a const reference to the Graph that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. In addition, the Graph must be static. */
	KY_FORCE_INLINE const Graph& GetGraph_UnSafe() const { return m_graphCellSafePtr.GetGraph_UnSafe(); }


	/*!	Returns a const pointer to the GraphCell that contains the vertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const GraphCell* GetGraphCell() const { return IsValid() ? &(GetGraphCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const reference to the GraphCell that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphCell& GetGraphCell_UnSafe() const { return GetStitchedGraphCell_UnSafe().GetGraphCell(); }


	/*!	Returns a const pointer to the StitchedGraph that handles the Graph that contains the vertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE StitchedGraph* GetStitchedGraph() const { return IsValid() ? &m_graphCellSafePtr.GetStitchedGraph_UnSafe() : KY_NULL; }

	/*!	Returns a const reference to the StitchedGraph that handles the Graph that contains the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedGraph& GetStitchedGraph_UnSafe() const { return m_graphCellSafePtr.GetStitchedGraph_UnSafe(); }

	/*!	Returns a pointer to the GraphReference that identifies the Graph that contains the vertex identified by this object. */
	KY_FORCE_INLINE GraphReference* GetGraphReference() const { return m_graphCellSafePtr.GetGraphReference(); }




	/*!	Returns a const pointer to the GraphVertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const GraphVertex* GetGraphVertex() const { return IsValid() ? &(m_graphCellSafePtr.GetGraphCell_UnSafe().GetVertex(m_vertexIdx)) : KY_NULL; }

	/*!	Returns a const reference to the GraphVertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const GraphVertex& GetGraphVertex_UnSafe() const { return m_graphCellSafePtr.GetGraphCell_UnSafe().GetVertex(m_vertexIdx); }


	/*!	Returns the ID of the vertex identified by this object during the last or current A* calculation. For internal use.
		If this object is invalid, returns #KyUInt32MAXVAL. */
	KY_FORCE_INLINE KyUInt32 GetAstarId() const { return IsValid() ? m_graphCellSafePtr.GetStitchedGraphCell_UnSafe().GetAstarId(m_vertexIdx) : KyUInt32MAXVAL; }

	/*!	Returns the ID of the vertex identified by this object during the last or current A* calculation. For internal use.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE KyUInt32 GetAstarId_Unsafe() const { return m_graphCellSafePtr.GetStitchedGraphCell_UnSafe().GetAstarId(m_vertexIdx); }

	/*!	Returns a const pointer to the GraphVertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const GraphVertex* GetVertex() const { return IsValid() ? &m_graphCellSafePtr.GetGraphCell_UnSafe().GetVertex(m_vertexIdx) : KY_NULL; }

	/*!	Returns a const reference to a Vec3f that contains the position of the vertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const Vec3f* GetPosition() const { return IsValid() ? &m_graphCellSafePtr.GetGraphCell_UnSafe().GetVertexPosition(m_vertexIdx) : KY_NULL; }

	/*!	Returns the NavTerrainTypeMask that indicates the terrain types assigned to the vertex identified by this object.
		If this object is invalid, returns #NavTerrainType_None.
	*/
	KY_FORCE_INLINE NavTerrainType GetVertexTerrainType() const { return IsValid() ? m_graphCellSafePtr.GetGraphCell_UnSafe().GetVertexTerrainType(m_vertexIdx) : NavTerrainType_None; }

	/*!	Returns the NavTerrainTypeMask that indicates the terrain types assigned to the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavTerrainType GetVertexTerrainType_Unsafe() const { return m_graphCellSafePtr.GetGraphCell_UnSafe().GetVertexTerrainType(m_vertexIdx); }

	/*!	Returns a pointer to the VertexData associated with the vertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE VertexData* GetVertexData() { return IsValid() ? &m_graphCellSafePtr.GetStitchedGraphCell_UnSafe().GetVertexData(m_vertexIdx) : KY_NULL; }

	/*!	Returns a const pointer to the VertexData associated with the vertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const VertexData* GetVertexData() const { return IsValid() ? &m_graphCellSafePtr.GetStitchedGraphCell_UnSafe().GetVertexData(m_vertexIdx) : KY_NULL; }

private :
	GraphCellSafePtr m_graphCellSafePtr;
	GraphVertexIdx m_vertexIdx;
};


}

#endif // #ifndef KyPathData_GraphVertexSafePtr_H
