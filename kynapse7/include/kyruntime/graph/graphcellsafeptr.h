/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_graph */

// Primary contact: JUBA - secondary contact: LAPA
#ifndef KyPathData_GraphCellSafePtr_H
#define KyPathData_GraphCellSafePtr_H

#include "kypathdata/containers/array.h"
#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/navmesh/navmeshtypes.h"
#include "kyruntime/graph/graphsafeptr.h"
#include "kyruntime/graph/stitchedgraphcell.h"


namespace Kaim
{

class GraphCell;

///////////////////////////////////////////////////////////////////////////////////////////
// GraphCellSafePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! This class uniquely identifies a single GraphCell in a Graph.
	\pp Instances of this class can be safely stored and used in subsequent frames, unlike GraphCellPtr.
	\pp When you are finished with an instance of this class, you must call Invalidate() to free its allocated resources. This
	must be done before you destroy the World.
	\ingroup kyruntime_graph */
class GraphCellSafePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid GraphCellPtr. */
	KY_FORCE_INLINE GraphCellSafePtr() : m_graphSafePtr(), m_stitchedGraphCellIdx(StitchedGraphCellIdx_Invalid) {}

	/*!	\label_constructor
		\param graphReference		A pointer to the GraphReference of the Graph that contains this cell.
		\param stitchedGraphCellIdx	The StitchedGraphCellIdx that identifies this cell in its Graph. */
	KY_FORCE_INLINE GraphCellSafePtr(GraphReference* graphReference, StitchedGraphCellIdx stitchedGraphCellIdx) : m_graphSafePtr(graphReference), m_stitchedGraphCellIdx(stitchedGraphCellIdx) {}

	/*!	\label_constructor
		\param graphReference		A reference to the GraphReference of the Graph that contains this cell.
		\param stitchedGraphCellIdx	The StitchedGraphCellIdx that identifies this cell in its Graph. */
	KY_FORCE_INLINE GraphCellSafePtr(GraphReference& graphReference, StitchedGraphCellIdx stitchedGraphCellIdx) : m_graphSafePtr(graphReference), m_stitchedGraphCellIdx(stitchedGraphCellIdx) {}

	/*! \label_constructor Copies the state of the specified GraphCellSafePtr to the newly created object. */
	KY_FORCE_INLINE GraphCellSafePtr(const GraphCellSafePtr& graphCellSafePtr) : m_graphSafePtr(graphCellSafePtr.GetGraphSafePtr()), m_stitchedGraphCellIdx(graphCellSafePtr.GetStitchedGraphCellIdx()) {}

	/*!	\label_reinit
		\param graphReference		A pointer to the GraphReference of the Graph that contains this cell.
		\param stitchedGraphCellIdx	The StitchedGraphCellIdx that identifies this cell in its Graph. */
	KY_FORCE_INLINE void Set(GraphReference* graphReference, StitchedGraphCellIdx stitchedGraphCellIdx)
	{
		m_graphSafePtr.Set(graphReference);
		m_stitchedGraphCellIdx = stitchedGraphCellIdx;
	}

	/*!	\label_reinit
		\param graphReference		A reference to the GraphReference of the Graph that contains this cell.
		\param stitchedGraphCellIdx	The StitchedGraphCellIdx that identifies this cell in its Graph. */
	KY_FORCE_INLINE void Set(GraphReference& graphReference, StitchedGraphCellIdx stitchedGraphCellIdx)
	{
		m_graphSafePtr.Set(graphReference);
		m_stitchedGraphCellIdx = stitchedGraphCellIdx;
	}

	/*! Returns true if this object refers to a valid GraphCell: i.e. a GraphCell in a Graph that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const {	return m_graphSafePtr.IsValid() &&
											m_stitchedGraphCellIdx < m_graphSafePtr.GetStitchedGraph_UnSafe().m_numberOfCells &&
											m_graphSafePtr.GetStitchedGraph_UnSafe().m_stitchedCells[m_stitchedGraphCellIdx]->IsStitched(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_graphSafePtr.Invalidate(); m_stitchedGraphCellIdx = StitchedGraphCellIdx_Invalid; }

	/*!	Returns \c true if this object identifies the same GraphCell as \c graphCellPtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const GraphCellSafePtr& graphCellSafePtr) const { return m_graphSafePtr == graphCellSafePtr.m_graphSafePtr && m_stitchedGraphCellIdx == graphCellSafePtr.m_stitchedGraphCellIdx; }

	/*!	Returns \c true if this object identifies a GraphCell different from the one identified by \c graphCellPtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const GraphCellSafePtr& graphCellSafePtr) const { return m_graphSafePtr != graphCellSafePtr.m_graphSafePtr || m_stitchedGraphCellIdx != graphCellSafePtr.m_stitchedGraphCellIdx; }

	KY_FORCE_INLINE bool operator < (const GraphCellSafePtr& graphCellSafePtr) const { return m_graphSafePtr < graphCellSafePtr.m_graphSafePtr || (m_graphSafePtr == graphCellSafePtr.m_graphSafePtr && m_stitchedGraphCellIdx < graphCellSafePtr.m_stitchedGraphCellIdx); }
	KY_FORCE_INLINE bool operator <=(const GraphCellSafePtr& graphCellSafePtr) const { return !(graphCellSafePtr < *this); }

	/*!	Returns a StitchedGraphCellIdx that refers to this GraphCell within its Graph. */
	KY_FORCE_INLINE StitchedGraphCellIdx GetStitchedGraphCellIdx() const { return m_stitchedGraphCellIdx; }

	/*!	Returns a reference to the GraphSafePtr that identifies the Graph that contains the GraphCell identified by this object.
		If this object is invalid, returns an invalid GraphSafePtr. */
	KY_FORCE_INLINE GraphSafePtr& GetGraphSafePtr() { return m_graphSafePtr; }

	/*!	Returns a reference to the GraphSafePtr that identifies the Graph that contains the GraphCell identified by this object.
		If this object is invalid, returns an invalid GraphSafePtr. */
	KY_FORCE_INLINE const GraphSafePtr& GetGraphSafePtr() const { return m_graphSafePtr; }


	/*!	Returns a const pointer to the StitchedGraphCell that contains the GraphCell identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE StitchedGraphCell* GetStitchedGraphCell() const { return IsValid() ? m_graphSafePtr.GetStitchedGraph_UnSafe().m_stitchedCells[m_stitchedGraphCellIdx] : KY_NULL; }

	/*!	Returns a const reference to the StitchedGraphCell that contains the GraphCell identified by this object.
		\pre	The current object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedGraphCell& GetStitchedGraphCell_UnSafe() const { return *(GetStitchedGraph_UnSafe().m_stitchedCells[m_stitchedGraphCellIdx]); }


	/*!	Returns a const pointer to the Graph that contains the GraphCell identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const Graph* GetGraph() const { return IsValid() ? &(GetGraph_UnSafe()) : KY_NULL; }

	/*!	Returns a const reference to the Graph that contains the GraphCell identified by this object.
		\pre	The current object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const Graph& GetGraph_UnSafe() const { return *m_graphSafePtr.GetGraph_UnSafe(); }


	/*!	Returns a const pointer to the GraphCell identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const GraphCell* GetGraphCell() const { return IsValid() ? &(GetGraphCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const reference to the GraphCell identified by this object.
		\pre	The current object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphCell& GetGraphCell_UnSafe() const { return GetStitchedGraphCell_UnSafe().GetGraphCell(); }


	/*!	Returns a pointer to the StitchedGraph that contains the GraphCell identified by this object.
		If current instance is Invalid, returns KY_NULL */
	KY_FORCE_INLINE StitchedGraph* GetStitchedGraph() const { return IsValid() ? &m_graphSafePtr.GetStitchedGraph_UnSafe() : KY_NULL; }

	/*!	Returns a const reference to the StitchedGraph that contains the GraphCell identified by this object.
		\pre	The current object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedGraph& GetStitchedGraph_UnSafe() const { return m_graphSafePtr.GetStitchedGraph_UnSafe(); }

	/*!	Returns a pointer to the GraphReference that refers to the Graph that contains the GraphCell identified by this object. */
	KY_FORCE_INLINE GraphReference* GetGraphReference() const { return m_graphSafePtr.GetGraphReference(); }

	/*!	Returns \c true if this object identifies a VirtualGraph (i.e. a graph that has been dynamically created to link two static \Graphs)
		\pre	The current object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE bool IsVirtualGraph() const { return m_graphSafePtr.IsVirtualGraph(); }

	/*!	Returns \c true if this object identifies a static GraphCell.
		\pre	The current object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE bool IsStaticGraph() const { return m_graphSafePtr.IsStaticGraph(); }

private :
	GraphSafePtr m_graphSafePtr;
	StitchedGraphCellIdx m_stitchedGraphCellIdx;
};

}

#endif // #ifndef KyPathData_GraphCellSafePtr_H
