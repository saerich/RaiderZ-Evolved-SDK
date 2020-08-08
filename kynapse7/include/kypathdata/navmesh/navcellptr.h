/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavCellPtr_H
#define KyPathData_NavCellPtr_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/stitchednavcell.h"
#include "kypathdata/navmesh/navcellsafeptr.h"

namespace Kaim
{

class NavCell;
class NavMesh;


///////////////////////////////////////////////////////////////////////////////////////////
// NavCellPtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single NavCell.
	\pp This pointer is guaranteed to be valid only in the frame in which it was retrieved.
	Never store a NavCellPtr for use in subsequent frames, because it has no protection against data
	streaming. Use NavCellSafePtr instead.
	\ingroup kypathdata_navmesh */
class NavCellPtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid NavCellPtr. */
	KY_FORCE_INLINE NavCellPtr(): m_stitchedNavCell(KY_NULL) {}

	/*!	\label_constructor
		\param stitchedNavCell		A pointer to the StitchedNavCell that contains this NavCell. */
	KY_FORCE_INLINE NavCellPtr(const StitchedNavCell* stitchedNavCell) : m_stitchedNavCell(stitchedNavCell) {}

	/*!	\label_constructor
		\param stitchedNavCell		A reference to the StitchedNavCell that contains this NavCell. */
	KY_FORCE_INLINE NavCellPtr(const StitchedNavCell& stitchedNavCell) : m_stitchedNavCell(&stitchedNavCell) {}

	/*!	\label_reinit
		\param stitchedNavCell		A pointer to the StitchedNavCell that contains this NavCell. */
	KY_FORCE_INLINE void Set(const StitchedNavCell* stitchedNavCell) { m_stitchedNavCell = stitchedNavCell; }

	/*!	\label_reinit
		\param stitchedNavCell		A reference to the StitchedNavCell that contains this NavCell. */
	KY_FORCE_INLINE void Set(const StitchedNavCell& stitchedNavCell) { m_stitchedNavCell = &stitchedNavCell; }

	/*! Returns \c true if this object refers to a valid NavCell: i.e. a NavCell in a NavMesh that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_stitchedNavCell != KY_NULL && m_stitchedNavCell->IsStitched(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_stitchedNavCell = KY_NULL; }

	/*!	Returns \c true if this object identifies the same NavCell as \c navCellPtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator==(const NavCellPtr& navCellPtr) const { return m_stitchedNavCell == navCellPtr.m_stitchedNavCell; }

	/*!	Returns \c true if this object identifies a NavCell different from the one identified by \c navCellPtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator!=(const NavCellPtr& navCellPtr) const { return m_stitchedNavCell != navCellPtr.m_stitchedNavCell; }

	KY_FORCE_INLINE bool operator< (const NavCellPtr& navCellPtr) const { return m_stitchedNavCell < navCellPtr.m_stitchedNavCell; }
	KY_FORCE_INLINE bool operator<=(const NavCellPtr& navCellPtr) const { return !(navCellPtr < *this); }

	/*!	Constructs a new instance of NavCellSafePtr that identifies the same NavCell as this object.
		If the current instance is invalid, it returns an invalid NavCellSafePtr. */
	KY_FORCE_INLINE NavCellSafePtr ConvertToSafePtr() const
	{
		if (m_stitchedNavCell != KY_NULL)
		{
			const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
			return NavCellSafePtr(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell);
		}

		return NavCellSafePtr();
	}

	/*!	Updates \c navCellSafePtr to identify the same NavCell as this object.
		If the current instance is invalid, \c navCellSafePtr is invalidated. */
	KY_FORCE_INLINE void ConvertToSafePtr(NavCellSafePtr& navCellSafePtr) const
	{
		if (m_stitchedNavCell != KY_NULL)
		{
			const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
			navCellSafePtr.Set(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell);
		}
		else
			navCellSafePtr.Invalidate();
	}

	/*!	Constructs a new instance of NavCellSafePtr that identifies the same NavCell as this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavCellSafePtr ConvertToSafePtr_UnSafe() const
	{
		const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
		return NavCellSafePtr(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell);
	}

	/*!	Updates \c navCellSafePtr to identify the same NavCell as this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE void ConvertToSafePtr_UnSafe(NavCellSafePtr& navCellSafePtr) const
	{
		const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
		navCellSafePtr.Set(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell);
	}

	/*!	Returns a const reference to the NavMesh that contains this NavCell.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavMesh& GetNavMesh() const { return GetStitchedNavCell().GetNavMeshReference()->GetData()->m_navMesh; }

	/*!	Returns a const reference to the StitchedNavCell that contains this NavCell.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedNavCell& GetStitchedNavCell() const
	{
		KY_FUNCTION("NavCellPtr::GetStitchedNavCell");
		KY_ASSERT(m_stitchedNavCell != KY_NULL, ("You cannot call the GetStitchedNavCell method on an invalid NavCellId"));
		return *m_stitchedNavCell;
	}

	/*!	Returns a const reference to the NavCell identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavCell& GetNavCell() const { return GetStitchedNavCell().GetNavCell(); }



private:
	const StitchedNavCell* m_stitchedNavCell;
};

}

#endif // KyPathData_NavCellPtr_H

