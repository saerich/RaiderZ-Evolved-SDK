/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavTriangleSafePtr_H
#define KyPathData_NavTriangleSafePtr_H

#include "kypathdata/navmesh/navcellsafeptr.h"
#include "kypathdata/navmesh/navfloorsafeptr.h"
#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navvertexsafeptr.h"
#include "kypathdata/navmesh/navhalfedgesafeptr.h"

namespace Kaim
{

class NavTrianglePtr;

///////////////////////////////////////////////////////////////////////////////////////////
// NavTriangleSafePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single NavMesh triangle.
	\pp Instances of this class can be safely stored and used in subsequent frames, unlike NavTrianglePtr.
	\pp When you are finished with an instance of this class, you must call Invalidate() to free its allocated resources. This
	must be done before you destroy the World.
	\ingroup kypathdata_navmesh */
class NavTriangleSafePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates an invalid NavTriangleSafePtr. */
	KY_FORCE_INLINE NavTriangleSafePtr() : m_floorSafePtr(), m_triangleIdx(NavTriangleIdx_Unset) {}
	
	/*!	\label_constructor
		\param navMeshReference		A pointer to the NavMeshReference associated with the NavMesh that contains this triangle.
		\param stitchedNavCellIdx	The index of the StitchedNavCell that handles the NavCell that contains this triangle.
		\param floorIdx				The index of the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE NavTriangleSafePtr(NavMeshReference* navMeshReference, StitchedNavCellIdx stitchedNavCellIdx, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx) :
		m_floorSafePtr(navMeshReference, stitchedNavCellIdx, floorIdx), m_triangleIdx(triangleIdx) {}

	/*!	\label_constructor
		\param navMeshReference		A reference to the NavMeshReference associated with the NavMesh that contains this triangle.
		\param stitchedNavCellIdx	The index of the StitchedNavCell that handles the NavCell that contains this triangle.
		\param floorIdx				The index of the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE NavTriangleSafePtr(NavMeshReference& navMeshReference, StitchedNavCellIdx stitchedNavCellIdx, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx) :
		m_floorSafePtr(navMeshReference, stitchedNavCellIdx, floorIdx), m_triangleIdx(triangleIdx) {}
	
	/*!	\label_constructor
		\param cellSafePtr			A NavCellSafePtr that identifies the NavCell that contains this triangle.
		\param floorIdx				The index of the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE NavTriangleSafePtr(const NavCellSafePtr& cellSafePtr, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx) : m_floorSafePtr(cellSafePtr, floorIdx), m_triangleIdx(triangleIdx) {}

	/*!	\label_constructor
		\param floorSafePtr			A NavFloorSafePtr that identifies the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE NavTriangleSafePtr(const NavFloorSafePtr& floorSafePtr, NavTriangleIdx triangleIdx) : m_floorSafePtr(floorSafePtr), m_triangleIdx(triangleIdx) {}

	/*! \label_constructor Copies the state of the specified NavTriangleSafePtr to the newly created object. */
	KY_FORCE_INLINE NavTriangleSafePtr(const NavTriangleSafePtr& navTriangleSafePtr) : m_floorSafePtr(navTriangleSafePtr.GetNavFloorSafePtr()), m_triangleIdx(navTriangleSafePtr.GetTriangleIdx()) {}

	/*!	\label_reinit
		\param navMeshReference			A pointer to the NavMeshReference associated with the NavMesh that contains this triangle.
		\param stitchedNavCellSafePtr	The index of the StitchedNavCell that handles the NavCell that contains this triangle.
		\param floorIdx					The index of the NavFloor that contains this triangle.
		\param triangleIdx				The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE void Set(NavMeshReference* navMeshReference, StitchedNavCellIdx stitchedNavCellSafePtr, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx)
	{
		m_floorSafePtr.Set(navMeshReference, stitchedNavCellSafePtr, floorIdx);
		m_triangleIdx = triangleIdx;
	}

	/*!	\label_reinit
		\param navMeshReference			A pointer to the NavMeshReference associated with the NavMesh that contains this triangle.
		\param stitchedNavCellSafePtr	The index of the StitchedNavCell that handles the NavCell that contains this triangle.
		\param floorIdx					The index of the NavFloor that contains this triangle.
		\param triangleIdx				The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE void Set(NavMeshReference& navMeshReference, StitchedNavCellIdx stitchedNavCellSafePtr, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx)
	{
		m_floorSafePtr.Set(navMeshReference, stitchedNavCellSafePtr, floorIdx);
		m_triangleIdx = triangleIdx;
	}

	/*!	\label_reinit
		\param cellSafePtr			A NavCellSafePtr that identifies the NavCell that contains this triangle.
		\param floorIdx				The index of the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavCellSafePtr& cellSafePtr, NavFloorIdx floorIdx, NavTriangleIdx triangleIdx)
	{
		m_floorSafePtr.Set(cellSafePtr, floorIdx);
		m_triangleIdx = triangleIdx;
	}

	/*!	\label_reinit
		\param floorSafePtr			A NavFloorSafePtr that identifies the NavFloor that contains this triangle.
		\param triangleIdx			The index of this triangle within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavFloorSafePtr& floorSafePtr, NavTriangleIdx triangleIdx)
	{
		m_floorSafePtr = floorSafePtr;
		m_triangleIdx = triangleIdx;
	}

	/*! Returns \c true if this object refers to a valid triangle: i.e. a triangle in a NavMesh that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_floorSafePtr.IsValid() && m_triangleIdx < m_floorSafePtr.GetNavFloor_UnSafe().GetTriangleCount(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_floorSafePtr.Invalidate(); m_triangleIdx = NavTriangleIdx_Unset; }

	/*!	Returns \c true if this object identifies the same triangle as \c navTriangleSafePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const NavTriangleSafePtr& navTriangleSafePtr) const { return m_triangleIdx == navTriangleSafePtr.m_triangleIdx && m_floorSafePtr == navTriangleSafePtr.m_floorSafePtr; }

	/*!	Returns \c true if this object identifies a triangle different from the one identified by \c navTriangleSafePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const NavTriangleSafePtr& navTriangleSafePtr) const { return m_triangleIdx != navTriangleSafePtr.m_triangleIdx || m_floorSafePtr != navTriangleSafePtr.m_floorSafePtr; }

	KY_FORCE_INLINE bool operator < (const NavTriangleSafePtr& triangleSafePtr) const { return m_floorSafePtr < triangleSafePtr.m_floorSafePtr || (m_floorSafePtr == triangleSafePtr.m_floorSafePtr && m_triangleIdx < triangleSafePtr.m_triangleIdx); }
	KY_FORCE_INLINE bool operator <=(const NavTriangleSafePtr& triangleSafePtr) const { return !(triangleSafePtr < *this); }

	/*!	Returns the index of this triangle within its NavFloor. */
	KY_FORCE_INLINE NavTriangleIdx GetTriangleIdx() const { return m_triangleIdx; }

	/*!	Returns the index of the NavFloor that contains this triangle. */
	KY_FORCE_INLINE NavFloorIdx GetFloorIdx() const { return m_floorSafePtr.GetFloorIdx(); }

	/*!	Returns a reference to a NavFloorSafePtr that identifies the NavFloor that contains this triangle.
		If this object is invalid, returns an invalid NavFloorSafePtr. */
	KY_FORCE_INLINE NavFloorSafePtr& GetNavFloorSafePtr() { return m_floorSafePtr; }

	/*!	Returns a const reference to a NavFloorSafePtr that identifies the NavFloor that contains this triangle.
		If this object is invalid, returns an invalid NavFloorSafePtr. */
	KY_FORCE_INLINE const NavFloorSafePtr& GetNavFloorSafePtr() const { return m_floorSafePtr; }

	/*!	Returns a reference to a NavCellSafePtr that identifies the NavCell that contains this triangle.
		If this object is invalid, returns an invalid NavCellSafePtr. */
	KY_FORCE_INLINE NavCellSafePtr& GetNavCellSafePtr() { return m_floorSafePtr.GetNavCellSafePtr(); }

	/*!	Returns a const reference to a NavCellSafePtr that identifies the NavCell that contains this triangle.
		If this object is invalid, returns an invalid NavCellSafePtr. */
	KY_FORCE_INLINE const NavCellSafePtr& GetNavCellSafePtr() const { return m_floorSafePtr.GetNavCellSafePtr(); }
	
	/*!	Constructs a new instance of NavTrianglePtr that identifies the same triangle as this object.
		If this object is invalid, it returns an invalid NavTrianglePtr. */
	NavTrianglePtr ConvertToNavTrianglePtr() const;

	/*!	Updates \c trianglePtr to identify the same triangle as this object.
		If this object is invalid, \c trianglePtr is invalidated. */
	void ConvertToNavTrianglePtr(NavTrianglePtr& trianglePtr) const;

	/*!	Constructs a new instance of NavTrianglePtr that identifies the same triangle as this object.
		\pre	This object must be valid. Its validity is not checked. */
	NavTrianglePtr ConvertToNavTrianglePtr_UnSafe() const;

	/*!	Updates \c trianglePtr to identify the same triangle as this object.
		\pre	This object must be valid. Its validity is not checked. */
	void ConvertToNavTrianglePtr_UnSafe(NavTrianglePtr& trianglePtr) const;

	/*!	Returns a const pointer to the StitchedNavCell that handles the NavCell that contains this triangle.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const StitchedNavCell* GetStitchedNavCell() const { return IsValid() ? &(m_floorSafePtr.GetStitchedNavCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const pointer to the NavCell that contains this triangle.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavCell* GetNavCell() const { return IsValid() ? &(m_floorSafePtr.GetNavCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const pointer to the NavFloor that contains this triangle.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavFloor* GetNavFloor() const { return IsValid() ? &(m_floorSafePtr.GetNavFloor_UnSafe()) : KY_NULL; }

	/*!	Returns the terrain type of the triangle identified by this object.
		If this object is invalid, returns #NavTerrainType_None. */
	KY_FORCE_INLINE NavTerrainType GetNavFloorTerrainType() const { return IsValid() ? GetNavFloor_UnSafe().GetNavTerrainType() : NavTerrainType_None; }

	/*!	Returns a const reference to the StitchedNavMesh that handles the NavMesh that contains this triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavMesh& GetStitchedNavMesh_UnSafe() const { return m_floorSafePtr.GetStitchedNavMesh_UnSafe(); }

	/*!	Returns a const reference to the NavMesh that contains this triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavMesh& GetNavMesh_UnSafe() const { return m_floorSafePtr.GetNavMesh_UnSafe(); }

	/*!	Returns a const reference to the StitchedNavCell that handles the NavCell that contains this triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavCell& GetStitchedNavCell_UnSafe() const { return m_floorSafePtr.GetStitchedNavCell_UnSafe(); }

	/*!	Returns a const reference to the NavCell that contains this triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavCell& GetNavCell_UnSafe() const { return m_floorSafePtr.GetNavCell_UnSafe(); }

	/*!	Returns a const reference to the NavFloor that contains this triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavFloor& GetNavFloor_UnSafe() const { return m_floorSafePtr.GetNavFloor_UnSafe(); }



	// ---------- triangle to halfEdge ----------

	/*! Returns the index of the specified edge in the triangle, or #NavHalfEdgeIdx_Invalid if this object is invalid.
		\param halfEdgeNumber			The number of the edge within this triangle. May be 0, 1 or 2. */
	KY_FORCE_INLINE NavHalfEdgeIdx GetHalfEdgeIdx(KyInt32 halfEdgeNumber) const
	{
		return IsValid() ? GetNavFloor_UnSafe().TriangleIdxToHalfEdgeIdx(m_triangleIdx, halfEdgeNumber) : NavHalfEdgeIdx_Invalid;
	}

	/*! Returns a pointer to the specified NavHalfEdge in the triangle, or #KY_NULL if this object is invalid.
		\param halfEdgeNumber			The number of the edge within this triangle. May be 0, 1 or 2. */
	KY_FORCE_INLINE const NavHalfEdge* GetHalfEdge(KyInt32 halfEdgeNumber) const
	{
		return IsValid() ? &(GetNavFloor_UnSafe().TriangleIdxToHalfEdge(m_triangleIdx, halfEdgeNumber)) : KY_NULL;
	}

	/*! Returns a NavHalfEdgeSafePtr that identifies the specified edge in the triangle, or an invalid NavHalfEdgeSafePtr
		if this object is invalid.
		\param halfEdgeNumber			The number of the edge within this triangle. May be 0, 1 or 2. */
	KY_FORCE_INLINE NavHalfEdgeSafePtr GetHalfEdgeSafePtr(KyInt32 halfEdgeNumber) const
	{
		return IsValid() ? NavHalfEdgeSafePtr(m_floorSafePtr, NavFloor::TriangleIdxToHalfEdgeIdx(m_triangleIdx, halfEdgeNumber)) : NavHalfEdgeSafePtr();
	}

	/*!	Updates \c resultSafePtr to identify the the specified edge in the triangle, or invalidates \c resultSafePtr if
		this object is invalid.
		\param halfEdgeNumber			The number of the edge within this triangle. May be 0, 1 or 2.
		\param[out] resultSafePtr		Stores the retrieved edge.*/
	KY_FORCE_INLINE void GetHalfEdgeSafePtr(KyInt32 halfEdgeNumber, NavHalfEdgeSafePtr& resultSafePtr) const
	{
		IsValid() ? resultSafePtr.Set(m_floorSafePtr, NavFloor::TriangleIdxToHalfEdgeIdx(m_triangleIdx, halfEdgeNumber)) : resultSafePtr.Invalidate();
	}

	// ---------- triangle to the 3 vertices ----------

	/*! Updates the parameters to identify the vertices in the triangle, or invalidates them if this object
		is invalid.
		\param[out] v0SafePtr			A NavVertexSafePtr that identifies the first vertex in this triangle.
		\param[out] v1SafePtr			A NavVertexSafePtr that identifies the second vertex in this triangle.
		\param[out] v2SafePtr			A NavVertexSafePtr that identifies the third vertex in this triangle. */
	inline void GetVertexSafePtrs(NavVertexSafePtr& v0SafePtr, NavVertexSafePtr& v1SafePtr, NavVertexSafePtr& v2SafePtr) const
	{
		if(IsValid())
		{
			const NavFloor& floor = GetNavFloor_UnSafe();

			const NavHalfEdgeIdx firstNavHalfEdgeIdx = floor.TriangleIdxToHalfEdgeIdx(m_triangleIdx, 0);

			const NavVertexIdx startVertexIdx0 = floor.HalfEdgeIdxToStartVertexIdx(firstNavHalfEdgeIdx);
			const NavVertexIdx startVertexIdx1 = floor.HalfEdgeIdxToStartVertexIdx(firstNavHalfEdgeIdx + 1);
			const NavVertexIdx startVertexIdx2 = floor.HalfEdgeIdxToStartVertexIdx(firstNavHalfEdgeIdx + 2);

			v0SafePtr.Set(m_floorSafePtr, startVertexIdx0);
			v1SafePtr.Set(m_floorSafePtr, startVertexIdx1);
			v2SafePtr.Set(m_floorSafePtr, startVertexIdx2);
		}
		else
		{
			v0SafePtr.Invalidate();
			v1SafePtr.Invalidate();
			v2SafePtr.Invalidate();
		}
	}


	/*! Updates the parameters to identify the vertices in the triangle, or sets them to #NavVertexIdx_Invalid if
		this object is invalid.
		\param[out] v0Idx				The index of the first vertex in this triangle.
		\param[out] v1Idx				The index of the second vertex in this triangle.
		\param[out] v2Idx				The index of the third vertex in this triangle. */
	KY_FORCE_INLINE void GetVertexIndices(NavVertexIdx& v0Idx, NavVertexIdx& v1Idx, NavVertexIdx& v2Idx) const
	{
		if(IsValid())
		{
			GetNavFloor_UnSafe().TriangleIdxToVertexIndices(m_triangleIdx, v0Idx, v1Idx, v2Idx);
		}
		else
		{
			v0Idx = NavVertexIdx_Invalid;
			v1Idx = NavVertexIdx_Invalid;
			v2Idx = NavVertexIdx_Invalid;
		}
	}

	// ---------- triangle to vertex ----------

	/*! Returns the index of the specified vertex in the triangle, or #NavVertexIdx_Invalid if this object is invalid.
		\param vertexNumber				The number of the vertex within this triangle. May be 0, 1 or 2. */
	KY_FORCE_INLINE NavVertexIdx GetVertexIdx(KyInt32 vertexNumber) const { return IsValid() ? GetNavFloor_UnSafe().TriangleIdxToVertexIdx(m_triangleIdx, vertexNumber) : NavVertexIdx_Invalid; }

	/*! Returns a const pointer to the specified vertex in the triangle, or #KY_NULL if this object is invalid.
		\param vertexNumber				The number of the vertex within this triangle. May be 0, 1 or 2. */
	KY_FORCE_INLINE const NavVertex* GetVertex(KyInt32 vertexNumber) const { return IsValid() ? &(GetNavFloor_UnSafe().TriangleIdxToVertex(m_triangleIdx, vertexNumber)) : KY_NULL; }

	/*! Returns a NavVertexSafePtr that identifies the specified vertex in the triangle, or an invalid NavVertexSafePtr if this object is invalid.
		\param vertexNumber				The number of the vertex within this triangle. May be 0, 1 or 2. */
	KY_FORCE_INLINE NavVertexSafePtr GetVertexPtr(KyInt32 vertexNumber) const
	{
		return IsValid() ? NavVertexSafePtr(m_floorSafePtr, GetNavFloor_UnSafe().TriangleIdxToVertexIdx(m_triangleIdx, vertexNumber)) : NavVertexSafePtr();
	}

	/*! Updates \c resultSafePtr to identify the specified vertex within the triangle, or invalidates \c resultSafePtr if this object is invalid.
		\param vertexNumber				The number of the vertex within this triangle. May be 0, 1 or 2.
		\param[out] resultSafePtr		Stores the retrieved vertex. */
	KY_FORCE_INLINE void GetVertexPtr(KyInt32 vertexNumber, NavVertexSafePtr& resultSafePtr) const
	{
		IsValid() ? resultSafePtr.Set(m_floorSafePtr, GetNavFloor_UnSafe().TriangleIdxToVertexIdx(m_triangleIdx, vertexNumber)) : resultSafePtr.Invalidate();
	}

private :
	NavFloorSafePtr m_floorSafePtr;
	NavTriangleIdx m_triangleIdx;
};


}

#endif //KyPathData_NavTriangleSafePtr_H

