/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavFloorPtr_H
#define KyPathData_NavFloorPtr_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navcellptr.h"
#include "kypathdata/navmesh/navcell.h"
#include "kypathdata/navmesh/navfloor.h"
#include "kypathdata/navmesh/navfloorsafeptr.h"


namespace Kaim
{

class NavFloor;
class NavCell;

///////////////////////////////////////////////////////////////////////////////////////////
// NavFloorPtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single NavFloor.
	\pp This pointer is guaranteed to be valid only in the frame in which it was retrieved.
	Never store a NavFloorPtr for use in subsequent frames, because it has no protection against data
	streaming. Use NavFloorSafePtr instead.
	\ingroup kypathdata_navmesh */
class NavFloorPtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:

	/*!	\label_constructor Creates an invalid NavFloorPtr. */
	KY_FORCE_INLINE NavFloorPtr() : m_cellPtr(), m_floorIdx(NavFloorIdx_Unset) {}

	/*!	\label_constructor
		\param stitchedNavCell		A pointer to the StitchedNavCell that contains this NavFloor.
		\param floorIdx				The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE NavFloorPtr(StitchedNavCell* stitchedNavCell, NavFloorIdx floorIdx) : m_cellPtr(stitchedNavCell), m_floorIdx(floorIdx) {}

	/*!	\label_constructor
		\param stitchedNavCell		A reference to the StitchedNavCell that contains this NavFloor.
		\param floorIdx				The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE NavFloorPtr(StitchedNavCell& stitchedNavCell, NavFloorIdx floorIdx) : m_cellPtr(stitchedNavCell), m_floorIdx(floorIdx) {}

	/*!	\label_constructor
		\param cellPtr				A NavCellPtr that identifies the NavCell that contains this NavFloor.
		\param floorIdx				The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE NavFloorPtr(const NavCellPtr& cellPtr, NavFloorIdx floorIdx) : m_cellPtr(cellPtr), m_floorIdx(floorIdx) {}

	/*! \label_constructor Copies the state of the specified NavFloorPtr to the newly created object. */
	KY_FORCE_INLINE NavFloorPtr(const NavFloorPtr& floorPtr) : m_cellPtr(floorPtr.GetNavCellPtr()), m_floorIdx(floorPtr.GetFloorIdx()) {}


	/*!	\label_reinit
		\param stitchedNavCell		A pointer to the StitchedNavCell that contains this NavFloor.
		\param floorIdx				The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE void Set(StitchedNavCell* stitchedNavCell, NavFloorIdx floorIdx) { m_cellPtr.Set(stitchedNavCell); m_floorIdx = floorIdx; }

	/*!	\label_reinit
		\param stitchedNavCell		A reference to the StitchedNavCell that contains this NavFloor.
		\param floorIdx				The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE void Set(StitchedNavCell& stitchedNavCell, NavFloorIdx floorIdx) { m_cellPtr.Set(stitchedNavCell); m_floorIdx = floorIdx; }

	/*!	\label_reinit
		\param cellPtr				A NavCellPtr that identifies the NavCell that contains this NavFloor.
		\param floorIdx				The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE void Set(const NavCellPtr& cellPtr, NavFloorIdx floorIdx) { m_cellPtr = cellPtr; m_floorIdx = floorIdx; }

	/*! Returns \c true if this object refers to a valid NavFloor: i.e. a NavFloor in a NavCell that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_cellPtr.IsValid() && m_floorIdx < m_cellPtr.GetNavCell().GetFloorCount(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_cellPtr.Invalidate(); m_floorIdx = NavFloorIdx_Unset; }

	/*!	Returns \c true if this object identifies the same NavFloor as \c floorPtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const NavFloorPtr& floorPtr) const { return m_floorIdx == floorPtr.m_floorIdx && m_cellPtr == floorPtr.m_cellPtr; }

	/*!	Returns \c true if this object identifies a NavFloor different from the one identified by \c floorPtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const NavFloorPtr& floorPtr) const { return m_floorIdx != floorPtr.m_floorIdx || m_cellPtr != floorPtr.m_cellPtr; }

	KY_FORCE_INLINE bool operator < (const NavFloorPtr& floorPtr) const { return m_cellPtr < floorPtr.m_cellPtr || (m_cellPtr == floorPtr.m_cellPtr && m_floorIdx < floorPtr.m_floorIdx); }
	KY_FORCE_INLINE bool operator <=(const NavFloorPtr& floorPtr) const { return !(floorPtr < *this); }

	/*!	Returns a reference to the NavCellPtr that identifies the NavCell that contains this NavFloor. */
	KY_FORCE_INLINE NavCellPtr& GetNavCellPtr() { return m_cellPtr; }

	/*!	Returns a const reference to the NavCellPtr that identifies the NavCell that contains this NavFloor. */
	KY_FORCE_INLINE const NavCellPtr& GetNavCellPtr() const { return m_cellPtr; }

	/*!	Returns the index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE NavFloorIdx GetFloorIdx() const { return m_floorIdx; }



	/*!	Constructs a new instance of NavFloorSafePtr that identifies the same NavFloor as this object.
		If the current instance is invalid, it returns an invalid NavFloorSafePtr. */
	inline NavFloorSafePtr ConvertToNavFloorSafePtr() const
	{
		if (IsValid())
		{
			const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
			return NavFloorSafePtr(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, m_floorIdx);
		}

		return NavFloorSafePtr();
	}

	/*!	Updates \c floorSafePtr to identify the same NavFloor as this object.
		If the current instance is invalid, \c floorSafePtr is invalidated. */
	inline void ConvertToNavFloorSafePtr(NavFloorSafePtr& floorSafePtr) const
	{
		if (IsValid())
		{
			const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
			floorSafePtr.Set(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, m_floorIdx);
		}
		else
			floorSafePtr.Invalidate();
	}

	/*!	Constructs a new instance of NavFloorSafePtr that identifies the same NavFloor as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline NavFloorSafePtr ConvertToNavFloorSafePtr_UnSafe() const
	{
		const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
		return NavFloorSafePtr(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, m_floorIdx);
	}

	/*!	Updates \c floorSafePtr to identify the same NavFloor as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline void ConvertToNavFloorSafePtr_UnSafe(NavFloorSafePtr& floorSafePtr) const
	{
		const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
		floorSafePtr.Set(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, m_floorIdx);
	}


	/*!	Returns a const reference to the NavMesh that contains this NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavMesh& GetNavMesh() const { return m_cellPtr.GetNavMesh(); }

	/*!	Returns a const reference to the StitchedNavCell that handles the NavCell that contains this NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedNavCell& GetStitchedNavCell() const { return m_cellPtr.GetStitchedNavCell(); }
	
	/*!	Returns a const reference to the NavCell that contains this NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavCell& GetNavCell() const { return m_cellPtr.GetNavCell(); }

	/*!	Returns a const reference to the NavFloor identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavFloor& GetNavFloor() const { return GetNavCell().GetNavFloor(m_floorIdx); }

	/*!	Returns the terrain type of the NavFloor identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavTerrainType GetNavFloorTerrainType() const { return GetNavFloor().GetNavTerrainType(); }



	/* Returns the ConnectedComponentId of the NavFloor identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE ConnectedComponentId& GetConnectedComponentId() const { return GetStitchedNavCell().m_navFloorConnectedComponent[GetFloorIdx()]; }

	KY_FORCE_INLINE void SetConnectedComponentId(ConnectedComponentId connectedComponentId) const { GetStitchedNavCell().m_navFloorConnectedComponent[GetFloorIdx()] = connectedComponentId; }

private:
	NavCellPtr m_cellPtr;
	NavFloorIdx m_floorIdx;
};



}

#endif //KyPathData_NavFloorPtr_H

