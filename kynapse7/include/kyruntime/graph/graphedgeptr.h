/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphEdgePtr_H
#define KyPathData_GraphEdgePtr_H

#include <kypathdata/graph/graphcell.h>
#include <kypathdata/containers/list.h>
#include <kyruntime/graph/graphvertexptr.h>
#include <kyruntime/graph/graphedgesafeptr.h>
#include <kyruntime/graph/virtualedge.h>
#include <kypathdata/pathdata/additionaldata.h>

namespace Kaim
{

class GraphVertexPtr;
class GraphManager;
class GraphEdgeSafePtr;
class EdgeData;

///////////////////////////////////////////////////////////////////////////////////////////
// GraphEdgePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! This class uniquely identifies a single edge in a Graph.
	\pp This unique ID is guaranteed to be valid only in the frame in which it was retrieved.
	\pp Never store a GraphEdgePtr for use in subsequent frames, because it has no protection against data
	streaming. Use GraphEdgeSafePtr instead.
	\ingroup kyruntime_graph */
class GraphEdgePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:

	/*!	\label_constructor Creates an invalid GraphEdgePtr. */
	KY_FORCE_INLINE GraphEdgePtr() : m_graphCellPtr(), m_edgeIdx(GraphEdgeIdx_Invalid) {}

	/*!	\label_constructor
		\param stitchedGraphCell	A pointer to the StitchedGraphCell that contains this edge.
		\param edgeIdx				The index of this edge in its GraphCell. */
	KY_FORCE_INLINE GraphEdgePtr(StitchedGraphCell* stitchedGraphCell, GraphEdgeIdx edgeIdx) : m_graphCellPtr(stitchedGraphCell), m_edgeIdx(edgeIdx) {}

	/*!	\label_constructor
		\param stitchedGraphCell	A reference to the StitchedGraphCell that contains this edge.
		\param edgeIdx				The index of this edge in its GraphCell. */
	KY_FORCE_INLINE GraphEdgePtr(StitchedGraphCell& stitchedGraphCell, GraphEdgeIdx edgeIdx) : m_graphCellPtr(stitchedGraphCell), m_edgeIdx(edgeIdx) {}

	/*!	\label_constructor
		\param cellPtr				A GraphCellPtr that identifies the cell that contains this edge.
		\param edgeIdx				The index of the edge in its GraphCell. */
	KY_FORCE_INLINE GraphEdgePtr(const GraphCellPtr& cellPtr, GraphEdgeIdx edgeIdx) : m_graphCellPtr(cellPtr), m_edgeIdx(edgeIdx) {}

	/*! \label_constructor Copies the state of the specified GraphEdgePtr to the newly created object. */
	KY_FORCE_INLINE GraphEdgePtr(const GraphEdgePtr& edgePtr) : m_graphCellPtr(edgePtr.GetGraphCellPtr()), m_edgeIdx(edgePtr.GetGraphEdgeIdx()) {}


	/*!	\label_reinit
		\param stitchedGraphCell	A pointer to the StitchedGraphCell that contains this edge.
		\param edgeIdx				The index of this edge in its GraphCell. */
	KY_FORCE_INLINE void Set(StitchedGraphCell* stitchedGraphCell, GraphEdgeIdx edgeIdx) { m_graphCellPtr.Set(stitchedGraphCell); m_edgeIdx = edgeIdx; }

	/*!	\label_reinit
		\param stitchedGraphCell	A reference to the StitchedGraphCell that contains this edge.
		\param edgeIdx				The index of this edge in its GraphCell. */
	KY_FORCE_INLINE void Set(StitchedGraphCell& stitchedGraphCell, GraphEdgeIdx edgeIdx) { m_graphCellPtr.Set(stitchedGraphCell); m_edgeIdx = edgeIdx; }

	/*!	\label_reinit
		\param cellPtr				A GraphCellPtr that identifies the cell that contains this edge.
		\param edgeIdx				The index of the edge in its GraphCell. */
	KY_FORCE_INLINE void Set(const GraphCellPtr& cellPtr, GraphEdgeIdx edgeIdx) { m_graphCellPtr = cellPtr; m_edgeIdx = edgeIdx; }

	/*! Returns \c true if this object refers to a valid edge: i.e. an edge in a Graph that is currently loaded in memory. */
	bool IsValid() const;

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_graphCellPtr.Invalidate(); m_edgeIdx = GraphEdgeIdx_Invalid; }

	/*!	Returns \c true if this object identifies the same edge as \c edgePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const GraphEdgePtr& edgePtr) const { return m_edgeIdx == edgePtr.m_edgeIdx && m_graphCellPtr == edgePtr.m_graphCellPtr; }

	/*!	Returns \c true if this object identifies an edge different from the one identified by \c edgePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const GraphEdgePtr& edgePtr) const { return m_edgeIdx != edgePtr.m_edgeIdx || m_graphCellPtr != edgePtr.m_graphCellPtr; }

	KY_FORCE_INLINE bool operator < (const GraphEdgePtr& edgePtr) const { return m_graphCellPtr < edgePtr.m_graphCellPtr || (m_graphCellPtr == edgePtr.m_graphCellPtr && m_edgeIdx < edgePtr.m_edgeIdx); }
	KY_FORCE_INLINE bool operator <=(const GraphEdgePtr& edgePtr) const { return !(edgePtr < *this); }

	/*!	Returns a reference to the GraphCellSafePtr that identifies the GraphCell that contains the edge identified by this object. */
	KY_FORCE_INLINE GraphCellPtr& GetGraphCellPtr() { return m_graphCellPtr; }

	/*!	Returns a const reference to the GraphCellSafePtr that identifies the GraphCell that contains the edge identified by this object. */
	KY_FORCE_INLINE const GraphCellPtr& GetGraphCellPtr() const { return m_graphCellPtr; }

	/*!	Returns the GraphEdgeIdx that identifies this edge within its GraphCell. */
	KY_FORCE_INLINE GraphEdgeIdx GetGraphEdgeIdx() const { return m_edgeIdx; }

	/*!	Returns a const reference to the GraphManager that maintains the Graph that contains the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphManager& GetGraphManager() const { return m_graphCellPtr.GetGraphManager(); }

	/*!	Returns a const pointer to the GraphReference that refers to the Graph that contains the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphReference* GetGraphRefence() const { return m_graphCellPtr.GetGraphReference(); }

	
	/*!	Returns a const reference to the Graph that contains the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const Graph& GetGraph() const { return m_graphCellPtr.GetGraph(); }

	/*!	Returns a const reference to the StitchedGraph that contains the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedGraph& GetStitchedGraph() const { return m_graphCellPtr.GetStitchedGraph(); }

	/*!	Returns a const reference to the StitchedGraphCell that contains the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedGraphCell& GetStitchedGraphCell() const { return m_graphCellPtr.GetStitchedGraphCell(); }
	
	/*!	Returns a const reference to the GraphCell that contains the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphCell& GetGraphCell() const { return m_graphCellPtr.GetGraphCell(); }


	/*!	Returns \c true if this object identifies an edge in a VirtualGraph: i.e. an edge that has been dynamically created to linked two static spatial graphs.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE bool IsVirtualEdge() const { return GetStitchedGraphCell().IsVirtual(); }

	/*!	Constructs a new instance of GraphEdgeSafePtr that identifies the same edge as this object.
		If the current instance is invalid, it returns an invalid GraphEdgeSafePtr. */
	inline GraphEdgeSafePtr ConvertToSafeEdgePtr() const
	{
		if (IsValid())
		{
			const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
			return GraphEdgeSafePtr(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx(), m_edgeIdx);
		}

		return GraphEdgeSafePtr();
	}

	/*!	Updates \c edgeSafePtr to identify the same edge as this object.
		If the current instance is invalid, \c edgeSafePtr is invalidated. */
	inline void ConvertToSafeEdgePtr(GraphEdgeSafePtr& edgeSafePtr) const
	{
		if (IsValid())
		{
			const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
			return edgeSafePtr.Set(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx(), m_edgeIdx);
		}
		else
			edgeSafePtr.Invalidate();
	}

	/*!	Constructs a new instance of GraphEdgeSafePtr that identifies the same edge as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline GraphEdgeSafePtr ConvertToSafeEdgePtr_UnSafe() const
	{
		const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
		return GraphEdgeSafePtr(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx(), m_edgeIdx);
	}

	/*!	Updates \c edgeSafePtr to identify the same edge as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline void ConvertToSafeEdgePtr_UnSafe(GraphEdgeSafePtr& edgeSafePtr) const
	{
		const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
		return edgeSafePtr.Set(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx(), m_edgeIdx);
	}




	/*!	Returns a const reference to the \c Vec3f position of the starting vertex of the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	const Vec3f& GetStartVertexPosition() const;

	/*!	Returns a const reference to the \c Vec3f position of the starting vertex of the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	const Vec3f& GetEndVertexPosition() const;

	/*!	Returns a GraphVertexPtr that identifies the starting vertex of the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	GraphVertexPtr GetStartVertexPtr() const;

	/*!	Returns a GraphVertexPtr that identifies the ending vertex of the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	GraphVertexPtr GetEndVertexPtr() const;

	/*!	Updates \c startVertexPtr to identify the starting vertex of the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	void GetStartVertexPtr(GraphVertexPtr& startVertexPtr) const;

	/*!	Updates \c startVertexPtr to identify the ending vertex of the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	void GetEndVertexPtr(GraphVertexPtr& endVertexPtr) const;

	/*!	Returns a GraphVertexSafePtr that identifies the ending vertex of the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	GraphVertexSafePtr GetStartVertexSafePtr() const;

	/*!	Returns a GraphVertexSafePtr that identifies the ending vertex of the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	GraphVertexSafePtr GetEndVertexSafePtr() const;

	/*!	Returns a pointer to the PathObject associated to the edge identified by this object, if any.
		Returns #KY_NULL if the edge is virtual, or if the edge is static but does not belong to a GraphCell that
		is associated to a PathObject.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE IPathObject* GetPathObject() const
	{
		return GetStitchedGraph().GetPathObject(m_edgeIdx);
	}

	/*!	Returns a const reference to the GraphEdge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphEdge& GetEdge() const
	{
		KY_FUNCTION("GraphVertexPtr::GetGraphCell");
		KY_ASSERT(!IsVirtualEdge(), ("You cannot call GetGraphCell on a GraphEdgePtr that identifies a virtual edge"));
		return GetGraphCell().GetEdge(m_edgeIdx);
	}

	/*!	Returns a const reference to the virtual edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	const VirtualEdge& GetVirtualEdge() const;

	/*!	Returns a reference to the EdgeData associated with the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	EdgeData& GetEdgeData() const;

	/*!	For internal use. */
	void IncreaseLockEdgeCount()
	{
		EdgeData& edgeData = GetEdgeData();
		edgeData.IncreaseLockCount();
	}

	/*!	For internal use. */
	void DecreaseLockEdgeCount()
	{
		EdgeData& edgeData = GetEdgeData();
		edgeData.DecreaseLockCount();
	}

	/*!	Retrieves the additional data associated with this edge, if any.
		\pp See also :USERGUIDE:"Using Additional Data".
		\tparam T	The type of additional data to retrieve.
		\pre		This object must be valid. Its validity is not checked. 
		\pre		The edge must have had additional data of the specified type created for it when the sector was generated, 
					and that data must have been loaded when the sector was added to the Database.
		\return An instance of the specified type that contains the data stored for this edge. */
	template <class T>
	T* GetAdditionalData() const
	{
		KY_FUNCTION("GraphVertexPtr::GetAdditionalData");
		KY_ASSERT(GetStitchedGraph().HasEdgeAdditionalData(), ("There is no Edge Additional Data for this graph"));

		EdgeAdditionalData<T>* edgeAdditionalData = (EdgeAdditionalData<T>*)GetStitchedGraph().m_edgeAdditionalDatas;
		KY_ASSERTK(edgeAdditionalData->m_guidCompound == GetGraph().m_guidCompound);
		
		const StitchedGraphCellIdx cellIndex = m_graphCellPtr.GetStitchedGraphCell().GetStitchedGraphCellIdx();
		return &(edgeAdditionalData->m_dataCells.Values()[cellIndex].Values()[m_edgeIdx]);
	}

private:
	friend class EdgeIteratorInWorld;
	GraphCellPtr m_graphCellPtr;
	GraphEdgeIdx m_edgeIdx;
};

}

#endif //KyPathData_GraphEdgePtr_H

