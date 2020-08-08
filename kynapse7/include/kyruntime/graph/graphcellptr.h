/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_graph */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphCellPtr_H
#define KyPathData_GraphCellPtr_H

#include "kypathdata/graph/graphtypes.h"
#include "kyruntime/graph/stitchedgraphcell.h"
#include "kyruntime/graph/graphcellsafeptr.h"

namespace Kaim
{

class GraphCell;
class Graph;


///////////////////////////////////////////////////////////////////////////////////////////
// GraphCellPtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! This class uniquely identifies a single GraphCell in a Graph.
	\pp This unique ID is guaranteed to be valid only in the frame in which it was retrieved.
	\pp Never store a GraphCellPtr for use in subsequent frames, because it has no protection against data
	streaming. Use GraphCellSafePtr instead.
	\ingroup kyruntime_graph */
class GraphCellPtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid GraphCellPtr. */
	KY_FORCE_INLINE GraphCellPtr(): m_stitchedGraphCell(KY_NULL) {}

	/*!	\label_constructor
		\param stitchedGraphCell		A pointer to the \c StitchedGraphCell that contains this cell. */
	KY_FORCE_INLINE GraphCellPtr(const StitchedGraphCell* stitchedGraphCell) : m_stitchedGraphCell(stitchedGraphCell) {}

	/*!	\label_constructor
		\param stitchedGraphCell		A reference to the \c StitchedGraphCell that contains this cell. */
	KY_FORCE_INLINE GraphCellPtr(const StitchedGraphCell& stitchedGraphCell) : m_stitchedGraphCell(&stitchedGraphCell) {}

	/*!	\label_reinit
		\param stitchedGraphCell		A pointer to the \c StitchedGraphCell that contains this cell. */
	KY_FORCE_INLINE void Set(const StitchedGraphCell* stitchedGraphCell) { m_stitchedGraphCell = stitchedGraphCell; }

	/*!	\label_reinit
		\param stitchedGraphCell		A reference to the \c StitchedGraphCell that contains this cell. */
	KY_FORCE_INLINE void Set(const StitchedGraphCell& stitchedGraphCell) { m_stitchedGraphCell = &stitchedGraphCell; }

	/*! Returns true if this object refers to a valid GraphCell: i.e. a GraphCell in a Graph that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_stitchedGraphCell != KY_NULL && m_stitchedGraphCell->IsStitched(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_stitchedGraphCell = KY_NULL; }

	/*!	Returns \c true if this object identifies the same GraphCell as \c graphCellPtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator==(const GraphCellPtr& graphCellPtr) const { return m_stitchedGraphCell == graphCellPtr.m_stitchedGraphCell; }

	/*!	Returns \c true if this object identifies a GraphCell different from the one identified by \c graphCellPtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator!=(const GraphCellPtr& graphCellPtr) const { return m_stitchedGraphCell != graphCellPtr.m_stitchedGraphCell; }


	KY_FORCE_INLINE bool operator< (const GraphCellPtr& graphCellPtr) const { return m_stitchedGraphCell < graphCellPtr.m_stitchedGraphCell; }
	KY_FORCE_INLINE bool operator<=(const GraphCellPtr& graphCellPtr) const { return !(graphCellPtr < *this); }


	/*!	Constructs a new instance of GraphCellSafePtr that identifies the same GraphCell as this object.
		If the current object is invalid, it returns an invalid GraphCellSafePtr. */
	GraphCellSafePtr ConvertToSafePtr() const
	{
		if (IsValid())
		{
			const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
			return GraphCellSafePtr(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx());
		}

		return GraphCellSafePtr();
	}

	/*!	Constructs a new instance of GraphCellSafePtr that identifies the same GraphCell as this object.
		If the current object is invalid, it returns an invalid GraphCellSafePtr. */
	void ConvertToSafePtr(GraphCellSafePtr& graphCellSafePtr) const
	{
		if (IsValid())
		{
			const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
			graphCellSafePtr.Set(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx());
		}
		else
			graphCellSafePtr.Invalidate();
	}

	/*!	Constructs a new instance of GraphCellSafePtr that identifies the same GraphCell as this object.
		\pre	This object must be valid. Its validity is not checked. */
	GraphCellSafePtr ConvertToSafePtr_UnSafe() const
	{
		const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
		return GraphCellSafePtr(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx());
	}

	/*!	Constructs a new instance of GraphCellSafePtr that identifies the same GraphCell as this object.
		\pre	This object must be valid. Its validity is not checked. */
	void ConvertToSafePtr_UnSafe(GraphCellSafePtr& graphCellSafePtr) const
	{
		const StitchedGraphCell& stitchedGraphCell = GetStitchedGraphCell();
		graphCellSafePtr.Set(stitchedGraphCell.GetGraphReference(), stitchedGraphCell.GetStitchedGraphCellIdx());
	}

	/*!	Returns a const reference to the Graph that contains the GraphCell identified by this object.
		\pre	This object must be valid, and the Graph must be static. */
	KY_FORCE_INLINE const Graph& GetGraph() const { return *(GetStitchedGraphCell().GetGraphReference()->GetData()->m_graph); }


	/*!	Returns a const reference to the StitchedGraph that contains the GraphCell identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedGraph& GetStitchedGraph() const { return GetStitchedGraphCell().GeParentStitchedGraph(); }


	/*!	Returns a const reference to the StitchedGraphCell that contains the GraphCell identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedGraphCell& GetStitchedGraphCell() const
	{
		KY_FUNCTION("GraphCellPtr::GetStitchedGraphCell");
		KY_ASSERT(IsValid(), ("You cannot call the GetStitchedGraphCell method on an invalid GraphCellId"));
		return *m_stitchedGraphCell;
	}

	/*!	Returns a const reference to the GraphCell identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphCell& GetGraphCell() const { return GetStitchedGraphCell().GetGraphCell(); }


	/*!	Returns a const reference to the GraphManager that maintains the Graph that contains the GraphCell identified by
		this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const GraphManager& GetGraphManager() const
	{
		KY_FUNCTION("GraphCellPtr::GetGraphManager");
		KY_ASSERT(IsValid(), ("You cannot call GetGraphManager on an invalid GraphCellPtr"));

		return *m_stitchedGraphCell->GetGraphManager();
	}

	/*!	Returns a pointer to the GraphReference that refers to the GraphCell identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE GraphReference* GetGraphReference() const { return m_stitchedGraphCell->GetGraphReference(); }

private:
	const StitchedGraphCell* m_stitchedGraphCell;
};

}

#endif // KyPathData_GraphCellPtr_H

