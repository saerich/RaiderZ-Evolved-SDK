/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavTrianglePtr_H
#define KyPathData_NavTrianglePtr_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navfloorptr.h"
#include "kypathdata/navmesh/navhalfedgeptr.h"
#include "kypathdata/navmesh/navvertexptr.h"
#include "kypathdata/navmesh/navtrianglesafeptr.h"

namespace Kaim
{

class NavHalfEdgePtr;

///////////////////////////////////////////////////////////////////////////////////////////
// NavTrianglePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single NavMesh triangle.
	\pp This pointer is guaranteed to be valid only in the frame in which it was retrieved.
	Never store a NavTrianglePtr for use in subsequent frames, because it has no protection against data
	streaming. Use NavTriangleSafePtr instead.
	\ingroup kypathdata_navmesh */
class NavTrianglePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:

	/*!	\label_constructor Creates an invalid NavTrianglePtr. */
	KY_FORCE_INLINE NavTrianglePtr() : m_floorPtr(), m_triangleIdx(NavTriangleIdx_Unset) {}

	/*!	\label_constructor
		\param stitchedNavCell		A pointer to the StitchedNavCell that handles the NavCell that contains this triangle.
		\param floorIdx				The index of the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE NavTrianglePtr(StitchedNavCell* stitchedNavCell, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx) :
		m_floorPtr(stitchedNavCell, floorIdx), m_triangleIdx(triangleIdx) {}

	/*!	\label_constructor
		\param stitchedNavCell		A reference to the StitchedNavCell that handles the NavCell that contains this triangle.
		\param floorIdx				The index of the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE NavTrianglePtr(StitchedNavCell& stitchedNavCell, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx) :
		m_floorPtr(stitchedNavCell, floorIdx), m_triangleIdx(triangleIdx) {}

	/*!	\label_constructor
		\param cellPtr				A NavCellPtr that identifies the NavCell that handles the NavCell that contains this triangle.
		\param floorIdx				The index of the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE NavTrianglePtr(const NavCellPtr& cellPtr, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx) :
		m_floorPtr(cellPtr, floorIdx),  m_triangleIdx(triangleIdx) {}

	/*!	\label_constructor
		\param floorPtr				A NavFloorPtr that identifies the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE NavTrianglePtr(const NavFloorPtr& floorPtr, NavTriangleIdx triangleIdx) : m_floorPtr(floorPtr), m_triangleIdx(triangleIdx) {}

	/*! \label_constructor Copies the state of the specified NavTrianglePtr to the newly constructed object. */
	KY_FORCE_INLINE NavTrianglePtr(const NavTrianglePtr& trianglePtr) : m_floorPtr(trianglePtr.GetNavFloorPtr()), m_triangleIdx(trianglePtr.GetTriangleIdx()) {}

	/*!	\label_reinit
		\param stitchedNavCell		A pointer to the StitchedNavCell that handles the NavCell that contains this triangle.
		\param floorIdx				The index of the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE void Set(StitchedNavCell* stitchedNavCell, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx)
	{
		m_floorPtr.Set(stitchedNavCell, floorIdx);
		m_triangleIdx = triangleIdx;
	}

	/*!	\label_reinit
		\param stitchedNavCell		A reference to the StitchedNavCell that handles the NavCell that contains this triangle.
		\param floorIdx				The index of the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE void Set(StitchedNavCell& stitchedNavCell, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx)
	{
		m_floorPtr.Set(stitchedNavCell, floorIdx);
		m_triangleIdx = triangleIdx;
	}

	/*!	\label_reinit
		\param cellPtr				A NavCellPtr that identifies the NavCell that contains this triangle.
		\param floorIdx				The index of the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavCellPtr& cellPtr, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx)
	{
		m_floorPtr.Set(cellPtr, floorIdx);
		m_triangleIdx = triangleIdx;
	}

	/*!	\label_reinit
		\param floorPtr				A NavFloorPtr that identifies the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavFloorPtr& floorPtr, NavTriangleIdx triangleIdx)
	{
		m_floorPtr = floorPtr;
		m_triangleIdx = triangleIdx;
	}

	/*! Returns \c true if this object refers to a valid triangle: i.e. a triangle in a NavMesh that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_floorPtr.IsValid() && m_triangleIdx < m_floorPtr.GetNavFloor().GetTriangleCount(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_floorPtr.Invalidate(); m_triangleIdx = NavTriangleIdx_Unset; }

	/*!	Returns \c true if this object identifies the same triangle as \c trianglePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const NavTrianglePtr& trianglePtr) const { return m_triangleIdx == trianglePtr.m_triangleIdx && m_floorPtr == trianglePtr.m_floorPtr; }

	/*!	Returns \c true if this object identifies a triangle different from the one identified by \c trianglePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const NavTrianglePtr& trianglePtr) const { return m_triangleIdx != trianglePtr.m_triangleIdx || m_floorPtr != trianglePtr.m_floorPtr; }

	KY_FORCE_INLINE bool operator < (const NavTrianglePtr& trianglePtr) const { return m_floorPtr < trianglePtr.m_floorPtr || (m_floorPtr == trianglePtr.m_floorPtr && m_triangleIdx < trianglePtr.m_triangleIdx); }
	KY_FORCE_INLINE bool operator <=(const NavTrianglePtr& trianglePtr) const { return !(trianglePtr < *this); }

	/*!	Returns the index of this triangle within its NavFloor. */
	KY_FORCE_INLINE NavTriangleIdx GetTriangleIdx() const { return m_triangleIdx; }

	/*!	Returns the index of the NavFloor that contains this triangle. */
	KY_FORCE_INLINE NavFloorIdx GetFloorIdx() const { return GetNavFloorPtr().GetFloorIdx(); }

	/*!	Returns a reference to the NavFloorPtr that identifies the NavFloor that contains this triangle. */
	KY_FORCE_INLINE NavFloorPtr& GetNavFloorPtr() { return m_floorPtr; }

	/*!	Returns a const reference to the NavFloorPtr that identifies the NavFloor that contains this triangle. */
	KY_FORCE_INLINE const NavFloorPtr& GetNavFloorPtr() const { return m_floorPtr; }

	/*!	Returns a reference to the NavCellSafePtr that identifies the NavCell that contains this triangle. */
	KY_FORCE_INLINE NavCellPtr& GetNavCellPtr() { return m_floorPtr.GetNavCellPtr(); }

	/*!	Returns a const reference to the NavCellSafePtr that identifies the NavCell that contains this triangle. */
	KY_FORCE_INLINE const NavCellPtr& GetNavCellPtr() const { return m_floorPtr.GetNavCellPtr(); }


	/*!	Constructs a new instance of NavTriangleSafePtr that identifies the same triangle as this object.
		If the current instance is invalid, it returns an invalid NavTriangleSafePtr. */
	inline NavTriangleSafePtr ConvertToNavTriangleSafePtr() const
	{
		if (IsValid())
		{
			const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
			return NavTriangleSafePtr(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_triangleIdx);
		}

		return NavTriangleSafePtr();
	}

	/*!	Updates \c triangleSafePtr to identify the same triangle as this object.
		If the current instance is invalid, \c triangleSafePtr is invalidated. */
	inline void ConvertToNavTriangleSafePtr(NavTriangleSafePtr& triangleSafePtr) const
	{
		if (IsValid())
		{
			const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
			triangleSafePtr.Set(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_triangleIdx);
		}
		else
			triangleSafePtr.Invalidate();
	}

	/*!	Constructs a new instance of NavTriangleSafePtr that identifies the same triangle as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline NavTriangleSafePtr ConvertToNavTriangleSafePtr_UnSafe() const
	{
		const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
		return NavTriangleSafePtr(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_triangleIdx);
	}

	/*!	Updates \c triangleSafePtr to identify the same triangle as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline void ConvertToNavTriangleSafePtr_UnSafe(NavTriangleSafePtr& triangleSafePtr) const
	{
		const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
		triangleSafePtr.Set(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_triangleIdx);
	}

	/*!	Returns a const reference to the StitchedNavCell that handles the NavCell that contains this triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedNavCell& GetStitchedNavCell() const { return m_floorPtr.GetStitchedNavCell(); }
	
	/*!	Returns a const reference to the NavCell that contains this triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavCell& GetNavCell() const { return m_floorPtr.GetNavCell(); }

	/*!	Returns a const reference to the NavFloor that contains this triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavFloor& GetNavFloor() const { return m_floorPtr.GetNavFloor(); }

	/*!	Returns the terrain type of the triangle identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavTerrainType GetNavFloorTerrainType() const { return GetNavFloor().GetNavTerrainType(); }


	// ---------- triangle to halfEdge ----------

	/*! Returns the index of the specified edge in the triangle.
		\param halfEdgeNumber			The number of the edge within this triangle. May be 0, 1 or 2.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavHalfEdgeIdx GetHalfEdgeIdx(KyInt32 halfEdgeNumber) const { return NavFloor::TriangleIdxToHalfEdgeIdx(m_triangleIdx, halfEdgeNumber); }

	/*! Returns a const reference to the specified edge in the triangle.
		\param halfEdgeNumber			The number of the edge within this triangle. May be 0, 1 or 2.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavHalfEdge& GetHalfEdge(KyInt32 halfEdgeNumber) const { return GetNavFloor().TriangleIdxToHalfEdge(m_triangleIdx, halfEdgeNumber); }

	/*! Returns a NavHalfEdgePtr that identifies the specified edge in the triangle.
		\param halfEdgeNumber			The number of the edge within this triangle. May be 0, 1 or 2.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavHalfEdgePtr GetHalfEdgePtr(KyInt32 halfEdgeNumber) const { return NavHalfEdgePtr(m_floorPtr, GetHalfEdgeIdx(halfEdgeNumber)); }

	/*! Updates \c resultPtr to identify the specified edge in the triangle.
		\param halfEdgeNumber			The number of the edge within this triangle. May be 0, 1 or 2.
		\param[out] resultPtr			Stores the NavHalfEdgePtr that identifies the specified edge in the triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE void GetHalfEdgePtr(KyInt32 halfEdgeNumber, NavHalfEdgePtr& resultPtr) const { resultPtr.Set(m_floorPtr, GetHalfEdgeIdx(halfEdgeNumber)); }

	// ---------- triangle to the 3 vertices ----------

	/*! Updates the parameters to identify the vertices in the triangle.
		\param[out] v0Ptr				A NavVertexPtr that identifies the first vertex in this triangle.
		\param[out] v1Ptr				A NavVertexPtr that identifies the second vertex in this triangle.
		\param[out] v2Ptr				A NavVertexPtr that identifies the third vertex in this triangle.
		\pre	This object must be valid. Its validity is not checked. */
	inline void GetVertexPtrs(NavVertexPtr& v0Ptr, NavVertexPtr& v1Ptr, NavVertexPtr& v2Ptr) const
	{
		const NavFloor& floor = GetNavFloor();

		const NavHalfEdgeIdx firstNavHalfEdgeIdx = floor.TriangleIdxToHalfEdgeIdx(m_triangleIdx, 0);

		const NavVertexIdx startVertexIdx0 = floor.HalfEdgeIdxToStartVertexIdx(firstNavHalfEdgeIdx);
		const NavVertexIdx startVertexIdx1 = floor.HalfEdgeIdxToStartVertexIdx(firstNavHalfEdgeIdx + 1);
		const NavVertexIdx startVertexIdx2 = floor.HalfEdgeIdxToStartVertexIdx(firstNavHalfEdgeIdx + 2);

		v0Ptr.Set(m_floorPtr, startVertexIdx0);
		v1Ptr.Set(m_floorPtr, startVertexIdx1);
		v2Ptr.Set(m_floorPtr, startVertexIdx2);
	}

	/*! Updates the parameters to identify the vertices in the triangle.
		\param[out] v0Idx				The index of the first vertex in this triangle.
		\param[out] v1Idx				The index of the second vertex in this triangle.
		\param[out] v2Idx				The index of the third vertex in this triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE void GetVertexIndices(NavVertexIdx& v0Idx, NavVertexIdx& v1Idx, NavVertexIdx& v2Idx) const
	{
		GetNavFloor().TriangleIdxToVertexIndices(m_triangleIdx, v0Idx, v1Idx, v2Idx);
	}


	// ---------- triangle to vertex ----------

	/*! Returns the index of the specified vertex in the triangle.
		\param vertexNumber				The number of the vertex within this triangle. May be 0, 1 or 2.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavVertexIdx GetVertexIdx(KyInt32 vertexNumber) const { return GetNavFloor().TriangleIdxToVertexIdx(m_triangleIdx, vertexNumber); }

	/*! Returns a const reference to the specified vertex in the triangle.
		\param vertexNumber				The number of the vertex within this triangle. May be 0, 1 or 2.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavVertex& GetVertex(KyInt32 vertexNumber) const { return GetNavFloor().TriangleIdxToVertex(m_triangleIdx, vertexNumber); }

	/*! Returns a NavVertexPtr that identifies the specified vertex in the triangle.
		\param vertexNumber			The number of the vertex within this triangle. May be 0, 1 or 2.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavVertexPtr GetVertexPtr(KyInt32 vertexNumber) const { return NavVertexPtr(m_floorPtr, GetVertexIdx(vertexNumber)); }

	/*! Updates \c resultPtr to identify the specified vertex in the triangle.
		\param vertexNumber				The number of the vertex within this triangle. May be 0, 1 or 2.
		\param[out] resultPtr			Stores the NavHalfEdgePtr that identifies the specified vertex in the triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE void GetVertexPtr(KyInt32 vertexNumber, NavVertexPtr& resultPtr) const { resultPtr.Set(m_floorPtr, GetVertexIdx(vertexNumber)); }

private :

	NavFloorPtr m_floorPtr;
	NavTriangleIdx m_triangleIdx;
};


}

#endif //KyPathData_NavTrianglePtr_H

