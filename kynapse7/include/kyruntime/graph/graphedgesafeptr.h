/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// Primary contact: JUBA - secondary contact: MAMU
#ifndef KyPathData_GraphEdgeSafePtr_H
#define KyPathData_GraphEdgeSafePtr_H

#include "kypathdata/generic/memory.h"
#include "kypathdata/containers/array.h"
#include "kypathdata/graph/graphtypes.h"
#include "kyruntime/graph/graphcellsafeptr.h"


namespace Kaim
{

class GraphEdgePtr;

///////////////////////////////////////////////////////////////////////////////////////////
// GraphEdgeSafePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! This class uniquely identifies a single edge in a Graph.
	\pp Instances of this class can be safely stored and used in subsequent frames, unlike GraphEdgePtr.
	\pp When you are finished with an instance of this class, you must call Invalidate() to free its allocated resources. This
	must be done before you destroy the World.
	\ingroup kyruntime_graph */
class GraphEdgeSafePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid GraphEdgeSafePtr. */
	KY_FORCE_INLINE GraphEdgeSafePtr() : m_graphCellSafePtr(), m_edgeIdx(GraphEdgeIdx_Invalid) {}
	
	/*!	\label_constructor
		\param navMeshReference		A pointer to the GraphReference of the Graph that contains this edge.
		\param stitchedGraphCellIdx	The StitchedGraphCellIdx that identifies the GraphCell that contains this edge.
		\param edgeIdx				The index of the edge within its GraphCell. */
	KY_FORCE_INLINE GraphEdgeSafePtr(GraphReference* navMeshReference, StitchedGraphCellIdx stitchedGraphCellIdx, GraphEdgeIdx edgeIdx) :
		m_graphCellSafePtr(navMeshReference, stitchedGraphCellIdx), m_edgeIdx(edgeIdx) {}

	/*!	\label_constructor
		\param navMeshReference		A reference to the GraphReference of the Graph that contains this edge.
		\param stitchedGraphCellIdx	The StitchedGraphCellIdx that identifies the GraphCell that contains this edge.
		\param edgeIdx				The index of the edge within its GraphCell. */
	KY_FORCE_INLINE GraphEdgeSafePtr(GraphReference& navMeshReference, StitchedGraphCellIdx stitchedGraphCellIdx, GraphEdgeIdx edgeIdx) :
		m_graphCellSafePtr(navMeshReference, stitchedGraphCellIdx), m_edgeIdx(edgeIdx) {}
	
	/*!	\label_constructor
		\param cellSafePtr			A GraphCellSafePtr that identifies the GraphCell that contains this edge.
		\param edgeIdx				The index of the edge within its GraphCell. */
	KY_FORCE_INLINE GraphEdgeSafePtr(const GraphCellSafePtr& cellSafePtr, GraphEdgeIdx edgeIdx) : m_graphCellSafePtr(cellSafePtr), m_edgeIdx(edgeIdx) {}

	/*! \label_constructor Copies the state of the specified GraphEdgeSafePtr to this object. */
	KY_FORCE_INLINE GraphEdgeSafePtr(const GraphEdgeSafePtr& graphEdgeSafePtr) : m_graphCellSafePtr(graphEdgeSafePtr.GetGraphCellSafePtr()), m_edgeIdx(graphEdgeSafePtr.GetGraphEdgeIdx()) {}

	/*!	\label_reinit
		\param navMeshReference			A pointer to the GraphReference of the Graph that contains this edge.
		\param stitchedGraphCellSafePtr	The StitchedGraphCellIdx that identifies the GraphCell that contains this edge.
		\param edgeIdx					The index of the edge within its GraphCell. */
	KY_FORCE_INLINE void Set(GraphReference* navMeshReference, StitchedGraphCellIdx stitchedGraphCellSafePtr, GraphEdgeIdx edgeIdx)
	{
		m_graphCellSafePtr.Set(navMeshReference, stitchedGraphCellSafePtr);
		m_edgeIdx = edgeIdx;
	}

	/*!	\label_reinit
		\param navMeshReference			A reference to the GraphReference of the Graph that contains this edge.
		\param stitchedGraphCellSafePtr	The StitchedGraphCellIdx that identifies the GraphCell that contains this edge.
		\param edgeIdx					The index of the edge within its GraphCell. */
	KY_FORCE_INLINE void Set(GraphReference& navMeshReference, StitchedGraphCellIdx stitchedGraphCellSafePtr, GraphEdgeIdx edgeIdx)
	{
		m_graphCellSafePtr.Set(navMeshReference, stitchedGraphCellSafePtr);
		m_edgeIdx = edgeIdx;
	}

	/*!	\label_reinit
		\param cellSafePtr			A GraphCellSafePtr that identifies the GraphCell that contains this edge.
		\param edgeIdx				The index of the edge within its GraphCell. */
	KY_FORCE_INLINE void Set(const GraphCellSafePtr& cellSafePtr, GraphEdgeIdx edgeIdx)
	{
		m_graphCellSafePtr = cellSafePtr;
		m_edgeIdx = edgeIdx;
	}

	/*! Returns \c true if this object refers to a valid edge: i.e. an edge in a Graph that is currently loaded in memory. */
	bool IsValid() const;

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_graphCellSafePtr.Invalidate(); m_edgeIdx = GraphEdgeIdx_Invalid; }

	/*!	Returns \c true if this object identifies the same edge as \c graphEdgeSafePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const GraphEdgeSafePtr& graphEdgeSafePtr) const { return m_edgeIdx == graphEdgeSafePtr.m_edgeIdx && m_graphCellSafePtr == graphEdgeSafePtr.m_graphCellSafePtr; }

	/*!	Returns \c true if this object identifies an edge different from the one identified by \c graphEdgeSafePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const GraphEdgeSafePtr& graphEdgeSafePtr) const { return m_edgeIdx != graphEdgeSafePtr.m_edgeIdx || m_graphCellSafePtr != graphEdgeSafePtr.m_graphCellSafePtr; }

	KY_FORCE_INLINE bool operator < (const GraphEdgeSafePtr& edgeSafePtr) const { return m_graphCellSafePtr < edgeSafePtr.m_graphCellSafePtr || (m_graphCellSafePtr == edgeSafePtr.m_graphCellSafePtr && m_edgeIdx < edgeSafePtr.m_edgeIdx); }
	KY_FORCE_INLINE bool operator <=(const GraphEdgeSafePtr& edgeSafePtr) const { return !(edgeSafePtr < *this); }

	/*!	Returns a GraphEdgeIdx that identifies this edge within its GraphCell. */
	KY_FORCE_INLINE GraphEdgeIdx GetGraphEdgeIdx() const { return m_edgeIdx; }

	/*!	Returns a reference to the GraphCellSafePtr that identifies the GraphCell that contains the edge identified by this object.
		If this object is invalid, returns an invalid GraphCellSafePtr. */
	KY_FORCE_INLINE GraphCellSafePtr& GetGraphCellSafePtr() { return m_graphCellSafePtr; }

	/*!	Returns aconst reference to the GraphCellSafePtr that identifies the GraphCell that contains the edge identified by this object.
		If this object is invalid, returns an invalid GraphCellSafePtr. */
	KY_FORCE_INLINE const GraphCellSafePtr& GetGraphCellSafePtr() const { return m_graphCellSafePtr; }


	/*!	Returns \c true if this object identifies an edge in a VirtualGraph: i.e. an edge that has been dynamically created to linked two static spatial graphs.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE bool IsVirtualEdge() const { return m_graphCellSafePtr.IsVirtualGraph(); }

	/*!	Constructs a new instance of GraphEdgePtr that identifies the same edge as this object.
		If this object is invalid, it returns an invalid GraphEdgeSafePtr. */
	GraphEdgePtr ConvertToEdgePtr() const;
	
	/*!	Updates \c edgePtr to identify the same edge as this object.
		If this object is invalid, \c edgePtr is invalidated. */
	void ConvertToEdgePtr(GraphEdgePtr& edgePtr) const;

	/*!	Constructs a new instance of GraphEdgePtr that identifies the same edge as this object.
		\pre	This object must be valid. Its validity is not checked. */
	GraphEdgePtr ConvertToEdgePtr_UnSafe() const;

	/*!	Updates \c edgePtr to identify the same edge as this object.
		\pre	This object must be valid. Its validity is not checked. */
	void ConvertToEdgePtr_UnSafe(GraphEdgePtr& edgePtr) const;


	/*!	Returns a const pointer to the StitchedGraphCell that contains the edge identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE StitchedGraphCell* GetStitchedGraphCell() const { return IsValid() ? &m_graphCellSafePtr.GetStitchedGraphCell_UnSafe() : KY_NULL; }

	/*!	Returns a const reference to the StitchedGraphCell that contains the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedGraphCell& GetStitchedGraphCell_UnSafe() const { return m_graphCellSafePtr.GetStitchedGraphCell_UnSafe(); }


	/*!	Returns a const pointer to the Graph that contains the edge identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const Graph* GetGraph() const { return IsValid() ? &(GetGraph_UnSafe()) : KY_NULL; }

	/*!	Returns a const reference to the Graph that contains the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. In addition, the Graph must be static. */
	KY_FORCE_INLINE const Graph& GetGraph_UnSafe() const { return m_graphCellSafePtr.GetGraph_UnSafe(); }


	/*!	Returns a const pointer to the GraphCell that contains the edge identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const GraphCell* GetGraphCell() const { return IsValid() ? &(GetGraphCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const reference to the GraphCell that contains the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphCell& GetGraphCell_UnSafe() const { return GetStitchedGraphCell_UnSafe().GetGraphCell(); }


	/*!	Returns a const pointer to the StitchedGraph that handles the Graph that contains the edge identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE StitchedGraph* GetStitchedGraph() const { return IsValid() ? &m_graphCellSafePtr.GetStitchedGraph_UnSafe() : KY_NULL; }

	/*!	Returns a const reference to the StitchedGraph that handles the Graph that contains the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedGraph& GetStitchedGraph_UnSafe() const { return m_graphCellSafePtr.GetStitchedGraph_UnSafe(); }

	/*!	Returns a pointer to the GraphReference that identifies the Graph that contains the edge identified by this object. */
	KY_FORCE_INLINE GraphReference* GetGraphReference() const { return m_graphCellSafePtr.GetGraphReference(); }



private:
	GraphCellSafePtr m_graphCellSafePtr;
	GraphEdgeIdx m_edgeIdx;
};

}

#endif // #ifndef KyPathData_GraphEdgeSafePtr_H
