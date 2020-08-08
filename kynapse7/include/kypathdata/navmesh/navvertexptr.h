/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavVertexPtr_H
#define KyPathData_NavVertexPtr_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navfloorptr.h"
#include "kypathdata/navmesh/navcellptr.h"
#include "kypathdata/navmesh/navvertexsafeptr.h"

namespace Kaim
{
///////////////////////////////////////////////////////////////////////////////////////////
// NavVertexPtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single vertex in a NavMesh triangle.
	\pp This pointer is guaranteed to be valid only in the frame in which it was retrieved.
	Never store a NavVertexPtr for use in subsequent frames, because it has no protection against data
	streaming. Use NavVertexSafePtr instead.
	\ingroup kypathdata_navmesh */
class NavVertexPtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:

	/*!	\label_constructor Creates an invalid NavVertexPtr. */
	KY_FORCE_INLINE NavVertexPtr() : m_floorPtr(), m_vertexIdx(NavVertexIdx_Unset) {}

	/*!	\label_constructor
		\param stitchedNavCell		A pointer to the StitchedNavCell that handles the NavCell that contains this vertex.
		\param floorIdx				The index of the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE NavVertexPtr(StitchedNavCell* stitchedNavCell, NavFloorIdx floorIdx, NavVertexIdx vertexIdx) :
		m_floorPtr(stitchedNavCell, floorIdx), m_vertexIdx(vertexIdx) {}

	/*!	\label_constructor
		\param stitchedNavCell		A reference to the StitchedNavCell that handles the NavCell that contains this vertex.
		\param floorIdx				The index of the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE NavVertexPtr(StitchedNavCell& stitchedNavCell, NavFloorIdx floorIdx, NavVertexIdx vertexIdx) :
		m_floorPtr(stitchedNavCell, floorIdx), m_vertexIdx(vertexIdx) {}

	/*!	\label_constructor
		\param cellPtr				A NavCellPtr that identifies the NavCell that contains this vertex.
		\param floorIdx				The index of the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE NavVertexPtr(const NavCellPtr& cellPtr, NavFloorIdx floorIdx, NavVertexIdx vertexIdx) :
		m_floorPtr(cellPtr, floorIdx),  m_vertexIdx(vertexIdx) {}

	/*!	\label_constructor
		\param floorPtr				A NavFloorPtr that identifies the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE NavVertexPtr(const NavFloorPtr& floorPtr, NavVertexIdx vertexIdx) : m_floorPtr(floorPtr), m_vertexIdx(vertexIdx) {}

	/*!	\label_constructor Copies the state of the specified NavVertexPtr to the newly created object. */
	KY_FORCE_INLINE NavVertexPtr(const NavVertexPtr& vertexPtr) : m_floorPtr(vertexPtr.GetNavFloorPtr()), m_vertexIdx(vertexPtr.GetVertexIdx()) {}

	/*!	\label_reinit
		\param stitchedNavCell		A pointer to the StitchedNavCell that handles the NavCell that contains this vertex.
		\param floorIdx				The index of the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE void Set(StitchedNavCell* stitchedNavCell, NavFloorIdx floorIdx, NavVertexIdx vertexIdx)
	{
		m_floorPtr.Set(stitchedNavCell, floorIdx);
		m_vertexIdx = vertexIdx;
	}

	/*!	\label_reinit
		\param stitchedNavCell		A reference to the StitchedNavCell that handles the NavCell that contains this vertex.
		\param floorIdx				The index of the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE void Set(StitchedNavCell& stitchedNavCell, NavFloorIdx floorIdx, NavVertexIdx vertexIdx)
	{
		m_floorPtr.Set(stitchedNavCell, floorIdx);
		m_vertexIdx = vertexIdx;
	}

	/*!	\label_reinit
		\param cellPtr				A NavCellPtr that identifies the NavCell that contains this vertex.
		\param floorIdx				The index of the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavCellPtr& cellPtr, NavFloorIdx floorIdx, NavVertexIdx vertexIdx)
	{
		m_floorPtr.Set(cellPtr, floorIdx);
		m_vertexIdx = vertexIdx;
	}

	/*!	\label_reinit
		\param floorPtr				A NavFloorPtr that identifies the NavFloor that contains this vertex.
		\param vertexIdx			The index of this vertex within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavFloorPtr& floorPtr, NavVertexIdx vertexIdx)
	{
		m_floorPtr = floorPtr;
		m_vertexIdx = vertexIdx;
	}

	/*! Returns \c true if this object refers to a valid vertex: i.e. a vertex in a NavMesh that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_floorPtr.IsValid() && m_vertexIdx < m_floorPtr.GetNavFloor().GetVertexCount(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_floorPtr.Invalidate(); m_vertexIdx = NavVertexIdx_Unset; }

	/*!	Returns \c true if this object identifies the same vertex as \c vertexPtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const NavVertexPtr& vertexPtr) const { return m_vertexIdx == vertexPtr.m_vertexIdx && m_floorPtr == vertexPtr.m_floorPtr; }

	/*!	Returns \c true if this object identifies a vertex different from the one identified by \c vertexPtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const NavVertexPtr& vertexPtr) const { return m_vertexIdx != vertexPtr.m_vertexIdx || m_floorPtr != vertexPtr.m_floorPtr; }

	KY_FORCE_INLINE bool operator < (const NavVertexPtr& vertexPtr) const { return m_floorPtr < vertexPtr.m_floorPtr || (m_floorPtr == vertexPtr.m_floorPtr && m_vertexIdx < vertexPtr.m_vertexIdx); }
	KY_FORCE_INLINE bool operator <=(const NavVertexPtr& vertexPtr) const { return !(vertexPtr < *this); }

	/*!	Returns the index of this vertex within its NavFloor. */
	KY_FORCE_INLINE NavVertexIdx GetVertexIdx() const { return m_vertexIdx; }

	/*!	Returns the index of the NavFloor that contains this vertex. */
	KY_FORCE_INLINE NavFloorIdx GetFloorIdx() const { return GetNavFloorPtr().GetFloorIdx(); }

	/*!	Returns a reference to the NavFloorPtr that identifies the NavFloor that contains this vertex. */
	KY_FORCE_INLINE NavFloorPtr& GetNavFloorPtr() { return m_floorPtr; }

	/*!	Returns a const reference to the NavFloorPtr that identifies the NavFloor that contains this vertex. */
	KY_FORCE_INLINE const NavFloorPtr& GetNavFloorPtr() const { return m_floorPtr; }

	/*!	Returns a reference to the NavCellSafePtr that identifies the NavCell that contains this vertex. */
	KY_FORCE_INLINE NavCellPtr& GetNavCellPtr() { return m_floorPtr.GetNavCellPtr(); }

	/*!	Returns a reference to the NavCellSafePtr that identifies the NavCell that contains this vertex. */
	KY_FORCE_INLINE const NavCellPtr& GetNavCellPtr() const { return m_floorPtr.GetNavCellPtr(); }


	/*!	Constructs a new instance of NavVertexSafePtr that identifies the same vertex as this object.
		If the current instance is invalid, it returns an invalid NavVertexSafePtr. */
	inline NavVertexSafePtr ConvertToNavVertexSafePtr() const
	{
		if (IsValid())
		{
			const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
			return NavVertexSafePtr(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_vertexIdx);
		}

		return NavVertexSafePtr();
	}

	/*!	Updates \c vertexSafePtr to identify the same vertex as this object.
		If the current instance is invalid, \c vertexSafePtr is invalidated. */
	inline void ConvertToNavVertexSafePtr(NavVertexSafePtr& vertexSafePtr) const
	{
		if (IsValid())
		{
			const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
			vertexSafePtr.Set(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_vertexIdx);
		}
		else
			vertexSafePtr.Invalidate();
	}

	/*!	Constructs a new instance of NavVertexSafePtr that identifies the same vertex as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline NavVertexSafePtr ConvertToNavVertexSafePtr_UnSafe() const
	{
		const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
		return NavVertexSafePtr(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_vertexIdx);
	}

	/*!	Updates \c vertexSafePtr to identify the same vertex as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline void ConvertToNavVertexSafePtr_UnSafe(NavVertexSafePtr& vertexSafePtr) const
	{
		const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
		vertexSafePtr.Set(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_vertexIdx);
	}

	/*!	Returns a const reference to the StitchedNavCell that handles the NavCell that contains this vertex.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedNavCell& GetStitchedNavCell() const { return m_floorPtr.GetStitchedNavCell(); }
	
	/*!	Returns a const reference to the NavCell that contains this vertex.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavCell& GetNavCell() const { return m_floorPtr.GetNavCell(); }

	/*!	Returns a const reference to the NavFloor that contains this vertex.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavFloor& GetNavFloor() const { return m_floorPtr.GetNavFloor(); }

	/*!	Returns the terrain type of the vertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavTerrainType GetNavFloorTerrainType() const { return GetNavFloor().GetNavTerrainType(); }


	/*!	Returns a const reference to the NavVertex identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavVertex& GetNavVertex() const { return GetNavFloor().GetVertex(m_vertexIdx); }


private :

	NavFloorPtr m_floorPtr;
	NavVertexIdx m_vertexIdx;
};


}

#endif //KyPathData_NavVertexPtr_H

