/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavCellSafePtr_H
#define KyPathData_NavCellSafePtr_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navmeshsafeptr.h"
#include "kypathdata/navmesh/stitchednavcell.h"

namespace Kaim
{

class NavCellPtr;
class NavCell;
class NavMesh;


///////////////////////////////////////////////////////////////////////////////////////////
// NavCellSafePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single NavCell.
	\pp Instances of this class can be safely stored and used in subsequent frames, unlike NavCellPtr.
	\pp When you are finished with an instance of this class, you must call Invalidate() to free its allocated resources. This
	must be done before you destroy the World.
	\ingroup kypathdata_navmesh */
class NavCellSafePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid NavCellSafePtr. */
	KY_FORCE_INLINE NavCellSafePtr() : m_meshSafePtr(), m_stitchedNavCellIdx(StitchedNavCellIdx_Invalid) {}

	/*!	\label_constructor
		\param navMeshReference		A pointer to the NavMeshReference associated with the NavMesh that contains this NavCell.
		\param stitchedNavCellIdx	The StitchedNavCellIdx that identifies this NavCell within its navMesh. */
	KY_FORCE_INLINE NavCellSafePtr(NavMeshReference* navMeshReference, StitchedNavCellIdx stitchedNavCellIdx) : m_meshSafePtr(navMeshReference), m_stitchedNavCellIdx(stitchedNavCellIdx) {}

	/*!	\label_constructor
		\param navMeshReference		A reference to the NavMeshReference associated with the NavMesh that contains this NavCell.
		\param stitchedNavCellIdx	The StitchedNavCellIdx that identifies this NavCell within its navMesh. */
	KY_FORCE_INLINE NavCellSafePtr(NavMeshReference& navMeshReference, StitchedNavCellIdx stitchedNavCellIdx) : m_meshSafePtr(navMeshReference), m_stitchedNavCellIdx(stitchedNavCellIdx) {}

	/*! \label_constructor Copies the state of the specified NavCellSafePtr to the newly created object. */
	KY_FORCE_INLINE NavCellSafePtr(const NavCellSafePtr& navCellSafePtr) : m_meshSafePtr(navCellSafePtr.GetMeshSafePtr()), m_stitchedNavCellIdx(navCellSafePtr.GetStitchedNavCellIdx()) {}

	/*!	\label_reinit
		\param navMeshReference		A pointer to the NavMeshReference associated with the NavMesh that contains this NavCell.
		\param stitchedNavCellIdx	The StitchedNavCellIdx that identifies this NavCell within its navMesh. */
	KY_FORCE_INLINE void Set(NavMeshReference* navMeshReference, StitchedNavCellIdx stitchedNavCellIdx)
	{
		m_meshSafePtr.Set(navMeshReference);
		m_stitchedNavCellIdx = stitchedNavCellIdx;
	}

	/*!	\label_reinit
		\param navMeshReference		A reference to the NavMeshReference associated with the NavMesh that contains this NavCell.
		\param stitchedNavCellIdx	The StitchedNavCellIdx that identifies this NavCell within its navMesh. */
	KY_FORCE_INLINE void Set(NavMeshReference& navMeshReference, StitchedNavCellIdx stitchedNavCellIdx)
	{
		m_meshSafePtr.Set(navMeshReference);
		m_stitchedNavCellIdx = stitchedNavCellIdx;
	}

	/*! Returns \c true if this object refers to a valid NavCell: i.e. a NavCell in a NavMesh that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const {	return m_meshSafePtr.IsValid() &&
											m_stitchedNavCellIdx < m_meshSafePtr.GetStitchedNavMesh_UnSafe().m_numberOfCells &&
											m_meshSafePtr.GetStitchedNavMesh_UnSafe().m_stitchedCells[m_stitchedNavCellIdx]->IsStitched(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_meshSafePtr.Invalidate(); m_stitchedNavCellIdx = StitchedNavCellIdx_Invalid; }

	/*!	Returns \c true if this object identifies the same NavCell as \c navCellSafePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const NavCellSafePtr& navCellSafePtr) const { return m_meshSafePtr == navCellSafePtr.m_meshSafePtr && m_stitchedNavCellIdx == navCellSafePtr.m_stitchedNavCellIdx; }

	/*!	Returns \c true if this object identifies a NavCell different from the one identified by \c navCellSafePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const NavCellSafePtr& navCellSafePtr) const { return m_meshSafePtr != navCellSafePtr.m_meshSafePtr || m_stitchedNavCellIdx != navCellSafePtr.m_stitchedNavCellIdx; }

	KY_FORCE_INLINE bool operator < (const NavCellSafePtr& navCellSafePtr) const { return m_meshSafePtr < navCellSafePtr.m_meshSafePtr || (m_meshSafePtr == navCellSafePtr.m_meshSafePtr && m_stitchedNavCellIdx < navCellSafePtr.m_stitchedNavCellIdx); }
	KY_FORCE_INLINE bool operator <=(const NavCellSafePtr& navCellSafePtr) const { return !(navCellSafePtr < *this); }

	/*!	Returns the index of the StitchedNavCell that handles this NavCell. */
	KY_FORCE_INLINE StitchedNavCellIdx GetStitchedNavCellIdx() const { return m_stitchedNavCellIdx; }

	/*!	Returns a reference to the NavMeshSafePtr that identifies the NavMesh that contains this NavCell.
		If this object is invalid, returns an invalid NavMeshSafePtr. */
	KY_FORCE_INLINE NavMeshSafePtr& GetMeshSafePtr() { return m_meshSafePtr; }

	/*!	Returns a const reference to the NavMeshSafePtr that identifies the NavMesh that contains this NavCell.
		If this object is invalid, returns an invalid NavMeshSafePtr. */
	KY_FORCE_INLINE const NavMeshSafePtr& GetMeshSafePtr() const { return m_meshSafePtr; }


	/*!	Constructs a new instance of NavCellPtr that identifies the same NavCell as this object.
		If this object is invalid, it returns an invalid NavCellPtr. */
	NavCellPtr ConvertToNavCellPtr() const;

	/*!	Updates \c cellPtr to identify the same NavCell as this object.
		If this object is invalid, \c cellPtr is invalidated. */
	void ConvertToNavCellPtr(NavCellPtr& cellPtr) const;

	/*!	Constructs a new instance of NavCellPtr that identifies the same NavCell as this object.
		\pre	This object must be valid. Its validity is not checked. */
	NavCellPtr ConvertToNavCellPtr_UnSafe() const;

	/*!	Updates \c cellPtr to identify the same NavCell as this object.
		\pre	This object must be valid. Its validity is not checked. */
	void ConvertToNavCellPtr_UnSafe(NavCellPtr& cellPtr) const;



	/*!	Returns a const pointer to the StitchedNavCell that handles this NavCell.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE StitchedNavCell* GetStitchedNavCell() const { return m_meshSafePtr.IsValid() ? m_meshSafePtr.GetStitchedNavMesh_UnSafe().m_stitchedCells[m_stitchedNavCellIdx] : KY_NULL; }

	/*!	Returns a const pointer to the NavMesh that contains this NavCell.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavMesh* GetNavMesh() const { return m_meshSafePtr.IsValid() ? &(GetNavMesh_UnSafe()) : KY_NULL; }

	/*!	Returns a const pointer to the NavCell identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavCell* GetNavCell() const { return m_meshSafePtr.IsValid() ? &(GetNavCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const reference to the StitchedNavMesh that handles the NavMesh that contains this NavCell.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavMesh& GetStitchedNavMesh_UnSafe() const { return m_meshSafePtr.GetStitchedNavMesh_UnSafe(); }

	/*!	Returns a const reference to the NavMesh that contains this NavCell.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavMesh& GetNavMesh_UnSafe() const { return m_meshSafePtr.GetNavMesh_UnSafe(); }

	/*!	Returns a const reference to the StitchedNavCell that handles this NavCell.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavCell& GetStitchedNavCell_UnSafe() const { return *(GetStitchedNavMesh_UnSafe().m_stitchedCells[m_stitchedNavCellIdx]); }

	/*!	Returns a const reference to the NavCell identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavCell& GetNavCell_UnSafe() const { return GetStitchedNavCell_UnSafe().GetNavCell(); }

private :
	NavMeshSafePtr m_meshSafePtr;
	StitchedNavCellIdx m_stitchedNavCellIdx;
};

}

#endif // KyPathData_NavCellSafePtr_H

