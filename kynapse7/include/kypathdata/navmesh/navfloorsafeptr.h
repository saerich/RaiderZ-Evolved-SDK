/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavFloorSafePtr_H
#define KyPathData_NavFloorSafePtr_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navcellsafeptr.h"
#include "kypathdata/navmesh/navcell.h"


namespace Kaim
{

class NavFloorPtr;
class NavFloor;
class NavCell;
class NavMesh;

///////////////////////////////////////////////////////////////////////////////////////////
// NavFloorSafePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single NavFloor.
	\pp Instances of this class can be safely stored and used in subsequent frames, unlike NavFloorPtr.
	\pp When you are finished with an instance of this class, you must call Invalidate() to free its allocated resources. This
	must be done before you destroy the World.
	\ingroup kypathdata_navmesh */
class NavFloorSafePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid NavFloorSafePtr. */
	KY_FORCE_INLINE NavFloorSafePtr() : m_cellSafePtr(), m_floorIdx(NavFloorIdx_Unset) {}
	
	/*!	\label_constructor
		\param navMeshReference		A pointer to the NavMeshReference associated with the NavMesh that contains this NavFloor.
		\param stitchedNavCellIdx	The index that identifies the StitchedNavCell that handles the NavCell that contains this NavFloor.
		\param floorIdx				The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE NavFloorSafePtr(NavMeshReference* navMeshReference, StitchedNavCellIdx stitchedNavCellIdx, NavFloorIdx floorIdx) :
		m_cellSafePtr(navMeshReference, stitchedNavCellIdx), m_floorIdx(floorIdx) {}

	/*!	\label_constructor
		\param navMeshReference		A reference to the NavMeshReference associated with the NavMesh that contains this NavFloor.
		\param stitchedNavCellIdx	The index that identifies the StitchedNavCell that handles the NavCell that contains this NavFloor.
		\param floorIdx				The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE NavFloorSafePtr(NavMeshReference& navMeshReference, StitchedNavCellIdx stitchedNavCellIdx, NavFloorIdx floorIdx) :
		m_cellSafePtr(navMeshReference, stitchedNavCellIdx), m_floorIdx(floorIdx) {}
	
	/*!	\label_constructor
		\param cellSafePtr			A NavCellSafePtr that identifies the NavCell that contains this NavFloor.
		\param floorIdx				The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE NavFloorSafePtr(const NavCellSafePtr& cellSafePtr, NavFloorIdx floorIdx) : m_cellSafePtr(cellSafePtr), m_floorIdx(floorIdx) {}

	/*! \label_constructor Copies the state of the specified NavFloorSafePtr to the newly created object. */
	KY_FORCE_INLINE NavFloorSafePtr(const NavFloorSafePtr& navFloorSafePtr) : m_cellSafePtr(navFloorSafePtr.GetNavCellSafePtr()), m_floorIdx(navFloorSafePtr.GetFloorIdx()) {}

	/*!	\label_reinit
		\param navMeshReference			A pointer to the NavMeshReference associated with the NavMesh that contains this NavFloor.
		\param stitchedNavCellSafePtr	The index that identifies the StitchedNavCell that handles the NavCell that contains this NavFloor.
		\param floorIdx					The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE void Set(NavMeshReference* navMeshReference, StitchedNavCellIdx stitchedNavCellSafePtr, NavFloorIdx floorIdx)
	{
		m_cellSafePtr.Set(navMeshReference, stitchedNavCellSafePtr);
		m_floorIdx = floorIdx;
	}

	/*!	\label_reinit
		\param navMeshReference			A reference to the NavMeshReference of the NavMesh that contains this NavFloor.
		\param stitchedNavCellSafePtr	The index that identifies the StitchedNavCell that handles the NavCell that contains this NavFloor.
		\param floorIdx					The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE void Set(NavMeshReference& navMeshReference, StitchedNavCellIdx stitchedNavCellSafePtr, NavFloorIdx floorIdx)
	{
		m_cellSafePtr.Set(navMeshReference, stitchedNavCellSafePtr);
		m_floorIdx = floorIdx;
	}

	/*!	\label_reinit
		\param cellSafePtr			A NavCellSafePtr that identifies the NavCell that contains this NavFloor.
		\param floorIdx				The index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE void Set(const NavCellSafePtr& cellSafePtr, NavFloorIdx floorIdx)
	{
		m_cellSafePtr = cellSafePtr;
		m_floorIdx = floorIdx;
	}

	/*! Returns \c true if this object refers to a valid NavFloor: i.e. a NavFloor in a NavCell that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_cellSafePtr.IsValid() && m_floorIdx < m_cellSafePtr.GetNavCell_UnSafe().GetFloorCount(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_cellSafePtr.Invalidate(); m_floorIdx = NavFloorIdx_Unset; }

	/*!	Returns \c true if this object identifies the same NavFloor as \c navFloorSafePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const NavFloorSafePtr& navFloorSafePtr) const { return m_floorIdx == navFloorSafePtr.m_floorIdx && m_cellSafePtr == navFloorSafePtr.m_cellSafePtr; }

	/*!	Returns \c true if this object identifies a NavFloor different from the one identified by \c navFloorSafePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const NavFloorSafePtr& navFloorSafePtr) const { return m_floorIdx != navFloorSafePtr.m_floorIdx || m_cellSafePtr != navFloorSafePtr.m_cellSafePtr; }


	KY_FORCE_INLINE bool operator < (const NavFloorSafePtr& floorSafePtr) const { return m_cellSafePtr < floorSafePtr.m_cellSafePtr || (m_cellSafePtr == floorSafePtr.m_cellSafePtr && m_floorIdx < floorSafePtr.m_floorIdx); }
	KY_FORCE_INLINE bool operator <=(const NavFloorSafePtr& navFloorSafePtr) const { return !(navFloorSafePtr < *this); }

	/*!	Returns the index of this NavFloor within its NavCell. */
	KY_FORCE_INLINE NavFloorIdx GetFloorIdx() const { return m_floorIdx; }

	/*!	Returns a reference to the NavCellSafePtr that identifies the NavCell that contains this NavFloor.
		If this object is invalid, returns an invalid NavMeshSafePtr. */
	KY_FORCE_INLINE NavCellSafePtr& GetNavCellSafePtr() { return m_cellSafePtr; }

	/*!	Returns a const reference to the NavCellSafePtr that identifies the NavCell that contains this NavFloor.
		If this object is invalid, returns an invalid NavMeshSafePtr. */
	KY_FORCE_INLINE const NavCellSafePtr& GetNavCellSafePtr() const { return m_cellSafePtr; }

	/*!	Constructs a new instance of NavFloorPtr that identifies the same NavFloor as this object.
		If this object is invalid, it returns an invalid NavCellPtr. */
	NavFloorPtr ConvertToNavFloorPtr() const;

	/*!	Updates \c floorPtr to identify the same NavFloorPtr as this object.
		If this object is invalid, \c floorPtr is invalidated. */
	void ConvertToNavFloorPtr(NavFloorPtr& floorPtr) const;

	/*!	Constructs a new instance of NavFloorPtr that identifies the same NavFloor as this object.
		\pre	This object must be valid. Its validity is not checked. */
	NavFloorPtr ConvertToNavFloorPtr_UnSafe() const;

	/*!	Updates \c floorPtr to identify the same NavFloor as this object.
		\pre	This object must be valid. Its validity is not checked. */
	void ConvertToNavFloorPtr_UnSafe(NavFloorPtr& floorPtr) const;

	/*!	Returns a const reference to the StitchedNavMesh that contains this NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavMesh& GetStitchedNavMesh_UnSafe() const { return m_cellSafePtr.GetStitchedNavMesh_UnSafe(); }

	/*!	Returns a const pointer to the StitchedNavCell that handles the NavCell that contains this NavFloor.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const StitchedNavCell* GetStitchedNavCell() const { return IsValid() ? &(m_cellSafePtr.GetStitchedNavCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const pointer to the NavCell that contains this NavFloor.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavCell* GetNavCell() const { return IsValid() ? &(m_cellSafePtr.GetNavCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const pointer to the NavFloor identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavFloor* GetNavFloor() const { return IsValid() ? &(m_cellSafePtr.GetNavCell_UnSafe().GetNavFloor(m_floorIdx)) : KY_NULL; }


	/*!	Returns a const reference to the NavMesh that contains this NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavMesh& GetNavMesh_UnSafe() const { return m_cellSafePtr.GetNavMesh_UnSafe(); }

	/*!	Returns a const reference to the StitchedNavCell that handles the NavCell that contains this NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavCell& GetStitchedNavCell_UnSafe() const { return m_cellSafePtr.GetStitchedNavCell_UnSafe(); }

	/*!	Returns a const reference to the NavCell that contains this NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavCell& GetNavCell_UnSafe() const { return m_cellSafePtr.GetNavCell_UnSafe(); }

	/*!	Returns a const reference to the NavFloor identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavFloor& GetNavFloor_UnSafe() const { return m_cellSafePtr.GetNavCell_UnSafe().GetNavFloor(m_floorIdx); }


	/*!	Returns the terrain type of the NavFloor identified by this object.
		If this object is invalid, returns #NavTerrainType_None. */
	KY_FORCE_INLINE NavTerrainType GetNavFloorTerrainType() const { return IsValid() ? GetNavFloor_UnSafe().GetNavTerrainType() : NavTerrainType_None; }

private :
	NavCellSafePtr m_cellSafePtr;
	NavFloorIdx m_floorIdx;
};



}

#endif //KyPathData_NavFloorSafePtr_H

