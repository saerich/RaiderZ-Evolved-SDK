/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavVertexSafePtr_H
#define KyPathData_NavVertexSafePtr_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navfloorsafeptr.h"
#include "kypathdata/navmesh/navcellsafeptr.h"

namespace Kaim
{


class NavVertexPtr;

///////////////////////////////////////////////////////////////////////////////////////////
// NavVertexSafePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single vertex in a NavMesh triangle.
	\pp Instances of this class can be safely stored and used in subsequent frames, unlike NavVertexPtr.
	\pp When you are finished with an instance of this class, you must call Invalidate() to free its allocated resources. This
	must be done before you destroy the World.
	\ingroup kypathdata_navmesh */
class NavVertexSafePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid NavVertexSafePtr. */
	KY_FORCE_INLINE NavVertexSafePtr() : m_floorSafePtr(), m_vertexIdx(NavVertexIdx_Unset) {}
	
	/*!	\label_constructor
		\param navMeshReference		A pointer to the NavMeshReference associated with the NavMesh that contains this vertex.
		\param stitchedNavCellIdx	The index of the StitchedNavCell that handles the NavCell that contains this vertex.
		\param floorIdx				The index of the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE NavVertexSafePtr(NavMeshReference* navMeshReference, StitchedNavCellIdx stitchedNavCellIdx, NavFloorIdx floorIdx, NavVertexIdx vertexIdx) :
		m_floorSafePtr(navMeshReference, stitchedNavCellIdx, floorIdx), m_vertexIdx(vertexIdx) {}

	/*!	\label_constructor
		\param navMeshReference		A reference to the NavMeshReference associated with the NavMesh that contains this vertex.
		\param stitchedNavCellIdx	The index of the StitchedNavCell that handles the NavCell that contains this vertex.
		\param floorIdx				The index of the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE NavVertexSafePtr(NavMeshReference& navMeshReference, StitchedNavCellIdx stitchedNavCellIdx, NavFloorIdx floorIdx, NavVertexIdx vertexIdx) :
		m_floorSafePtr(navMeshReference, stitchedNavCellIdx, floorIdx), m_vertexIdx(vertexIdx) {}
	
	/*!	\label_constructor
		\param cellSafePtr			A NavCellSafePtr that identifies the NavCell that contains this vertex.
		\param floorIdx				The index of the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE NavVertexSafePtr(const NavCellSafePtr& cellSafePtr, NavFloorIdx floorIdx, NavVertexIdx vertexIdx) : m_floorSafePtr(cellSafePtr, floorIdx), m_vertexIdx(vertexIdx) {}

	/*!	\label_constructor
		\param floorSafePtr			A NavFloorSafePtr that identifies the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE NavVertexSafePtr(const NavFloorSafePtr& floorSafePtr, NavVertexIdx vertexIdx) : m_floorSafePtr(floorSafePtr), m_vertexIdx(vertexIdx) {}

	/*! \label_constructor Copies the state of the specified NavVertexSafePtr to the newly created object. */
	KY_FORCE_INLINE NavVertexSafePtr(const NavVertexSafePtr& navVertexSafePtr) : m_floorSafePtr(navVertexSafePtr.GetNavFloorSafePtr()), m_vertexIdx(navVertexSafePtr.GetVertexIdx()) {}

	/*!	\label_reinit
		\param navMeshReference			A pointer to the NavMeshReference associated with the NavMesh that contains this vertex.
		\param stitchedNavCellSafePtr	The index of the StitchedNavCell that handles the NavCell that contains this vertex.
		\param floorIdx					The index of the NavFloor that contains this vertex.
		\param vertexIdx				The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE void Set(NavMeshReference* navMeshReference, StitchedNavCellIdx stitchedNavCellSafePtr, NavFloorIdx floorIdx, NavVertexIdx vertexIdx)
	{
		m_floorSafePtr.Set(navMeshReference, stitchedNavCellSafePtr, floorIdx);
		m_vertexIdx = vertexIdx;
	}

	/*!	\label_reinit
		\param navMeshReference			A reference to the NavMeshReference associated with the NavMesh that contains this vertex.
		\param stitchedNavCellSafePtr	The index of the StitchedNavCell that handles the NavCell that contains this vertex.
		\param floorIdx					The index of the NavFloor that contains this vertex.
		\param vertexIdx				The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE void Set(NavMeshReference& navMeshReference, StitchedNavCellIdx stitchedNavCellSafePtr, NavFloorIdx floorIdx, NavVertexIdx vertexIdx)
	{
		m_floorSafePtr.Set(navMeshReference, stitchedNavCellSafePtr, floorIdx);
		m_vertexIdx = vertexIdx;
	}

	/*!	\label_reinit
		\param cellSafePtr			A NavCellSafePtr that identifies the NavCell that contains this vertex.
		\param floorIdx				The index of the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavCellSafePtr& cellSafePtr, NavFloorIdx floorIdx, NavVertexIdx vertexIdx)
	{
		m_floorSafePtr.Set(cellSafePtr, floorIdx);
		m_vertexIdx = vertexIdx;
	}

	/*!	\label_reinit
		\param floorSafePtr			A NavFloorSafePtr that identifies the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavFloorSafePtr& floorSafePtr, NavVertexIdx vertexIdx)
	{
		m_floorSafePtr = floorSafePtr;
		m_vertexIdx = vertexIdx;
	}

	/*! Returns \c true if this object refers to a valid vertex: i.e. a vertex in a NavMesh that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_floorSafePtr.IsValid() && m_vertexIdx < m_floorSafePtr.GetNavFloor_UnSafe().GetVertexCount(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_floorSafePtr.Invalidate(); m_vertexIdx = NavVertexIdx_Unset; }

	/*!	Returns \c true if this object identifies the same vertex as \c navVertexSafePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const NavVertexSafePtr& navVertexSafePtr) const { return m_vertexIdx == navVertexSafePtr.m_vertexIdx && m_floorSafePtr == navVertexSafePtr.m_floorSafePtr; }

	/*!	Returns \c true if this object identifies a vertex different from the one identified by \c NavVertex, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const NavVertexSafePtr& navVertexSafePtr) const { return m_vertexIdx != navVertexSafePtr.m_vertexIdx || m_floorSafePtr != navVertexSafePtr.m_floorSafePtr; }

	KY_FORCE_INLINE bool operator < (const NavVertexSafePtr& vertexSafePtr) const { return m_floorSafePtr < vertexSafePtr.m_floorSafePtr || (m_floorSafePtr == vertexSafePtr.m_floorSafePtr && m_vertexIdx < vertexSafePtr.m_vertexIdx); }
	KY_FORCE_INLINE bool operator <=(const NavVertexSafePtr& vertexSafePtr) const { return !(vertexSafePtr < *this); }

	/*!	Returns the index of this vertex within its NavFloor. */
	KY_FORCE_INLINE NavVertexIdx GetVertexIdx() const { return m_vertexIdx; }

	/*!	Returns the index of the NavFloor that contains this vertex. */
	KY_FORCE_INLINE NavFloorIdx GetFloorIdx() const { return m_floorSafePtr.GetFloorIdx(); }

	/*!	Returns a reference to the NavFloorSafePtr that identifies the NavFloor that contains this vertex.
		If this object is invalid, returns an invalid NavFloorSafePtr. */
	KY_FORCE_INLINE NavFloorSafePtr& GetNavFloorSafePtr() { return m_floorSafePtr; }

	/*!	Returns a const reference to the NavFloorSafePtr that identifies the NavFloor that contains this vertex.
		If this object is invalid, returns an invalid NavFloorSafePtr. */
	KY_FORCE_INLINE const NavFloorSafePtr& GetNavFloorSafePtr() const { return m_floorSafePtr; }

	/*!	Returns a reference to the NavCellSafePtr that identifies the NavCell that contains this vertex.
		If this object is invalid, returns an invalid NavFloorSafePtr. */
	KY_FORCE_INLINE NavCellSafePtr& GetNavCellSafePtr() { return m_floorSafePtr.GetNavCellSafePtr(); }

	/*!	Returns a const reference to the NavCellSafePtr that identifies the NavCell that contains this vertex.
		If this object is invalid, returns an invalid NavFloorSafePtr. */
	KY_FORCE_INLINE const NavCellSafePtr& GetNavCellSafePtr() const { return m_floorSafePtr.GetNavCellSafePtr(); }
	
	/*!	Constructs a new instance of NavVertexPtr that identifies the same vertex as this object.
		If this object is invalid, it returns an invalid NavVertexPtr. */
	NavVertexPtr ConvertToNavVertexPtr() const;

	/*!	Updates \c vertexPtr to identify the same vertex as this object.
		If this object is invalid, \c vertexPtr is invalidated. */
	void ConvertToNavVertexPtr(NavVertexPtr& vertexPtr) const;

	/*!	Constructs a new instance of NavVertexPtr that identifies the same vertex as this object.
		\pre	This object must be valid. Its validity is not checked. */
	NavVertexPtr ConvertToNavVertexPtr_UnSafe() const;

	/*!	Updates \c vertexPtr to identify the same vertex as this object.
		\pre	This object must be valid. Its validity is not checked. */
	void ConvertToNavVertexPtr_UnSafe(NavVertexPtr& vertexPtr) const;

	/*!	Returns a const pointer to the StitchedNavCell that handles the NavCell that contains this vertex.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const StitchedNavCell* GetStitchedNavCell() const { return IsValid() ? &(m_floorSafePtr.GetStitchedNavCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const pointer to the NavCell that contains this vertex.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavCell* GetNavCell() const { return IsValid() ? &(m_floorSafePtr.GetNavCell_UnSafe()) : KY_NULL; }


	/*!	Returns a const pointer to the NavFloor that contains this vertex.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavFloor* GetNavFloor() const { return IsValid() ? &(m_floorSafePtr.GetNavFloor_UnSafe()) : KY_NULL; }

	/*!	Returns the terrain type of the vertex identified by this object.
		If this object is invalid, returns #NavTerrainType_None. */
	KY_FORCE_INLINE NavTerrainType GetNavFloorTerrainType() const { return IsValid() ? GetNavFloor_UnSafe().GetNavTerrainType() : NavTerrainType_None; }

	/*!	Returns a const reference to the StitchedNavMesh that handles the NavMesh that contains this vertex.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavMesh& GetStitchedNavMesh_UnSafe() const { return m_floorSafePtr.GetStitchedNavMesh_UnSafe(); }

	/*!	Returns a const reference to the NavMesh that contains this vertex.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavMesh& GetNavMesh_UnSafe() const { return m_floorSafePtr.GetNavMesh_UnSafe(); }

	/*!	Returns a const reference to the StitchedNavCell that handles the NavCell that contains this vertex.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavCell& GetStitchedNavCell_UnSafe() const { return m_floorSafePtr.GetStitchedNavCell_UnSafe(); }

	/*!	Returns a const reference to the NavCell that contains this vertex.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavCell& GetNavCell_UnSafe() const { return m_floorSafePtr.GetNavCell_UnSafe(); }

	/*!	Returns a const reference to the NavFloor that contains this vertex.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavFloor& GetNavFloor_UnSafe() const { return m_floorSafePtr.GetNavFloor_UnSafe(); }


	/*!	Returns a const pointer to the NavVertex identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavVertex* GetNavVertex() const { return IsValid() ? &(GetNavFloor_UnSafe().GetVertex(m_vertexIdx)) : KY_NULL; }

private :
	NavFloorSafePtr m_floorSafePtr;
	NavVertexIdx m_vertexIdx;
};


}

#endif // KyPathData_NavVertexSafePtr_H

