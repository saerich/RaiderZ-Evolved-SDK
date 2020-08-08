/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavHalfEdgeSafePtr_H
#define KyPathData_NavHalfEdgeSafePtr_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navfloorsafeptr.h"
#include "kypathdata/navmesh/navcellsafeptr.h"
#include "kypathdata/navmesh/navvertexsafeptr.h"

namespace Kaim
{

///////////////////////////////////////////////////////////////////////////////////////////
// NavHalfEdgeSafePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single edge in a NavMesh triangle.
	\pp Instances of this class can be safely stored and used in subsequent frames, unlike NavHalfEdgePtr.
	\pp When you are finished with an instance of this class, you must call Invalidate() to free its allocated resources. This
	must be done before you destroy the World.
	\ingroup kypathdata_navmesh */
class NavHalfEdgeSafePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor. Creates an invalid NavHalfEdgeSafePtr. */
	KY_FORCE_INLINE NavHalfEdgeSafePtr() : m_floorSafePtr(), m_halfEdgeIdx(NavHalfEdgeIdx_Unset) {}
	
	/*!	\label_constructor
		\param navMeshReference		A pointer to the NavMeshReference associated with the NavMesh that contains this edge.
		\param stitchedNavCellIdx	The index of the StitchedNavCell that handles the NavCell that contains this edge.
		\param floorIdx				The index of the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE NavHalfEdgeSafePtr(NavMeshReference* navMeshReference, StitchedNavCellIdx stitchedNavCellIdx, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx) :
		m_floorSafePtr(navMeshReference, stitchedNavCellIdx, floorIdx), m_halfEdgeIdx(halfEdgeIdx) {}

	/*!	\label_constructor
		\param navMeshReference		A reference to the NavMeshReference associated with the NavMesh that contains this edge.
		\param stitchedNavCellIdx	The index of the StitchedNavCell that handles the NavCell that contains this edge.
		\param floorIdx				The index of the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE NavHalfEdgeSafePtr(NavMeshReference& navMeshReference, StitchedNavCellIdx stitchedNavCellIdx, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx) :
		m_floorSafePtr(navMeshReference, stitchedNavCellIdx, floorIdx), m_halfEdgeIdx(halfEdgeIdx) {}
	
	/*!	\label_constructor
		\param cellSafePtr			A NavCellSafePtr that identifies the NavCell that contains this edge.
		\param floorIdx				The index of the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE NavHalfEdgeSafePtr(const NavCellSafePtr& cellSafePtr, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx) : m_floorSafePtr(cellSafePtr, floorIdx), m_halfEdgeIdx(halfEdgeIdx) {}

	/*!	\label_constructor
		\param floorSafePtr			A NavFloorSafePtr that identifies the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE NavHalfEdgeSafePtr(const NavFloorSafePtr& floorSafePtr, NavHalfEdgeIdx halfEdgeIdx) : m_floorSafePtr(floorSafePtr), m_halfEdgeIdx(halfEdgeIdx) {}

	/*! \label_constructor Copies the state of the specified NavHalfEdgeSafePtr to this object. */
	KY_FORCE_INLINE NavHalfEdgeSafePtr(const NavHalfEdgeSafePtr& navHalfEdgeSafePtr) : m_floorSafePtr(navHalfEdgeSafePtr.GetNavFloorSafePtr()), m_halfEdgeIdx(navHalfEdgeSafePtr.GetHalfEdgeIdx()) {}

	/*!	\label_reinit
		\param navMeshReference			A pointer to the NavMeshReference associated with the NavMesh that contains this edge.
		\param stitchedNavCellSafePtr	The index of the StitchedNavCell that handles the NavCell that contains this edge.
		\param floorIdx					The index of the NavFloor that contains this edge.
		\param halfEdgeIdx				The index of this edge within its NavFloor. */
	KY_FORCE_INLINE void Set(NavMeshReference* navMeshReference, StitchedNavCellIdx stitchedNavCellSafePtr, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx)
	{
		m_floorSafePtr.Set(navMeshReference, stitchedNavCellSafePtr, floorIdx);
		m_halfEdgeIdx = halfEdgeIdx;
	}

	/*!	\label_reinit
		\param navMeshReference			A reference to the NavMeshReference associated with the NavMesh that contains this edge.
		\param stitchedNavCellSafePtr	The index of the StitchedNavCell that handles the NavCell that contains this edge.
		\param floorIdx					The index of the NavFloor that contains this edge.
		\param halfEdgeIdx				The index of this edge within its NavFloor. */
	KY_FORCE_INLINE void Set(NavMeshReference& navMeshReference, StitchedNavCellIdx stitchedNavCellSafePtr, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx)
	{
		m_floorSafePtr.Set(navMeshReference, stitchedNavCellSafePtr, floorIdx);
		m_halfEdgeIdx = halfEdgeIdx;
	}

	/*!	\label_reinit
		\param cellSafePtr			A NavCellSafePtr that identifies the NavCell that contains this edge.
		\param floorIdx				The index of the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavCellSafePtr& cellSafePtr, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx)
	{
		m_floorSafePtr.Set(cellSafePtr, floorIdx);
		m_halfEdgeIdx = halfEdgeIdx;
	}

	/*!	\label_reinit
		\param floorSafePtr			A NavFloorSafePtr that identifies the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavFloorSafePtr& floorSafePtr, NavHalfEdgeIdx halfEdgeIdx)
	{
		m_floorSafePtr = floorSafePtr;
		m_halfEdgeIdx = halfEdgeIdx;
	}

	/*! Returns \c true if this object refers to a valid edge: i.e. an edge in a NavMesh that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_floorSafePtr.IsValid() && m_halfEdgeIdx < m_floorSafePtr.GetNavFloor_UnSafe().GetHalfEdgeCount(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_floorSafePtr.Invalidate(); m_halfEdgeIdx = NavHalfEdgeIdx_Unset; }

	/*!	Returns \c true if this object identifies the same edge as \c halfEdgeSafePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const NavHalfEdgeSafePtr& navHalfEdgeSafePtr) const { return m_halfEdgeIdx == navHalfEdgeSafePtr.m_halfEdgeIdx && m_floorSafePtr == navHalfEdgeSafePtr.m_floorSafePtr; }

	/*!	Returns \c true if this object identifies an edge different from the one identified by \c halfEdgeSafePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const NavHalfEdgeSafePtr& navHalfEdgeSafePtr) const { return m_halfEdgeIdx != navHalfEdgeSafePtr.m_halfEdgeIdx || m_floorSafePtr != navHalfEdgeSafePtr.m_floorSafePtr; }

	KY_FORCE_INLINE bool operator < (const NavHalfEdgeSafePtr& halfEdgeSafePtr) const { return m_floorSafePtr < halfEdgeSafePtr.m_floorSafePtr || (m_floorSafePtr == halfEdgeSafePtr.m_floorSafePtr && m_halfEdgeIdx < halfEdgeSafePtr.m_halfEdgeIdx); }
	KY_FORCE_INLINE bool operator <=(const NavHalfEdgeSafePtr& halfEdgeSafePtr) const { return !(halfEdgeSafePtr < *this); }

	/*!	Returns the index of this edge within its NavFloor. */
	KY_FORCE_INLINE NavHalfEdgeIdx GetHalfEdgeIdx() const { return m_halfEdgeIdx; }

	/*!	Returns the index of the NavFloor that contains this edge. */
	KY_FORCE_INLINE NavFloorIdx GetFloorIdx() const { return m_floorSafePtr.GetFloorIdx(); }

	/*!	Returns a reference to the NavFloorSafePtr that identifies the NavFloor that contains this edge. */
	KY_FORCE_INLINE NavFloorSafePtr& GetNavFloorSafePtr() { return m_floorSafePtr; }

	/*!	Returns a const reference to the NavFloorSafePtr that identifies the NavFloor that contains this edge. */
	KY_FORCE_INLINE const NavFloorSafePtr& GetNavFloorSafePtr() const { return m_floorSafePtr; }

	/*!	Returns a reference to the NavCellSafePtr that identifies the NavCell that contains this edge. */
	KY_FORCE_INLINE NavCellSafePtr& GetNavCellSafePtr() { return m_floorSafePtr.GetNavCellSafePtr(); }

	/*!	Returns a const reference to the NavCellSafePtr that identifies the NavCell that contains this edge. */
	KY_FORCE_INLINE const NavCellSafePtr& GetNavCellSafePtr() const { return m_floorSafePtr.GetNavCellSafePtr(); }
	
	/*!	Constructs a new instance of NavHalfEdgePtr that identifies the same edge as this object.
		If the current instance is invalid, it returns an invalid NavHalfEdgePtr. */
	NavHalfEdgePtr ConvertToNavHalfEdgePtr() const;

	/*!	Updates \c halfEdgePtr to identify the same edge as this object.
		If the current instance is invalid, \c halfEdgePtr is invalidated. */
	void ConvertToNavHalfEdgePtr(NavHalfEdgePtr& halfEdgePtr) const;

	/*!	Constructs a new instance of NavHalfEdgePtr that identifies the same edge as this object.
		\pre	This object must be valid. Its validity is not checked. */
	NavHalfEdgePtr ConvertToNavHalfEdgePtr_UnSafe() const;

	/*!	Updates \c halfEdgePtr to identify the same edge as this object.
		\pre	This object must be valid. Its validity is not checked. */
	void ConvertToNavHalfEdgePtr_UnSafe(NavHalfEdgePtr& halfEdgePtr) const;

	/*!	Returns a const pointer to the StitchedNavCell that handles the NavCell that contains this edge.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const StitchedNavCell* GetStitchedNavCell() const { return IsValid() ? &(m_floorSafePtr.GetStitchedNavCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const pointer to the NavCell that contains this edge.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavCell* GetNavCell() const { return IsValid() ? &(m_floorSafePtr.GetNavCell_UnSafe()) : KY_NULL; }

	/*!	Returns a const pointer to the NavFloor that contains this edge.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavFloor* GetNavFloor() const { return IsValid() ? &(m_floorSafePtr.GetNavFloor_UnSafe()) : KY_NULL; }

	/*!	Returns the terrain type of the edge identified by this object.
		If this object is invalid, returns #NavTerrainType_None. */
	KY_FORCE_INLINE NavTerrainType GetNavFloorTerrainType() const { return IsValid() ? GetNavFloor_UnSafe().GetNavTerrainType() : NavTerrainType_None; }

	/*!	Returns a const reference to the StitchedNavMesh that handles the NavMesh that contains this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavMesh& GetStitchedNavMesh_UnSafe() const { return m_floorSafePtr.GetStitchedNavMesh_UnSafe(); }

	/*!	Returns a const reference to the NavMesh that contains this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavMesh& GetNavMesh_UnSafe() const { return m_floorSafePtr.GetNavMesh_UnSafe(); }

	/*!	Returns a const reference to the StitchedNavCell that handles the NavCell that contains this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE StitchedNavCell& GetStitchedNavCell_UnSafe() const { return m_floorSafePtr.GetStitchedNavCell_UnSafe(); }

	/*!	Returns a const reference to the NavCell that contains this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavCell& GetNavCell_UnSafe() const { return m_floorSafePtr.GetNavCell_UnSafe(); }

	/*!	Returns a const reference to the NavFloor that contains this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavFloor& GetNavFloor_UnSafe() const { return m_floorSafePtr.GetNavFloor_UnSafe(); }



	/*!	Returns a const pointer to the NavHalfEdge identified by this object.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavHalfEdge* GetHalfEdge() const { return IsValid() ? &(GetNavFloor_UnSafe().GetHalfEdge(m_halfEdgeIdx)) : KY_NULL; }

	/*!	Returns the NavHalfEdgeType that indicates what type of boundary this edge represents.
		If this object is invalid, returns \c NavHalfEdgeType_FORCE32. */
	KY_FORCE_INLINE NavHalfEdgeType GetHalfEdgeType() const { return IsValid() ? GetNavFloor_UnSafe().GetHalfEdgeType(m_halfEdgeIdx) : NavHalfEdgeType_FORCE32; }


	// ---------- Edge to StartVertex ----------

	/*!	Returns the index of the starting vertex of this edge within its NavFloor.
		If this object is invalid, returns #NavVertexIdx_Invalid. */
	KY_FORCE_INLINE NavVertexIdx GetStartVertexIdx() const { return IsValid() ? GetNavFloor_UnSafe().HalfEdgeIdxToStartVertexIdx(m_halfEdgeIdx) : NavVertexIdx_Invalid; }

	/*!	Returns the starting vertex of this edge.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavVertex* GetStartVertex() const { return IsValid() ? &(GetNavFloor_UnSafe().HalfEdgeIdxToStartVertex(m_halfEdgeIdx)) : KY_NULL; }

	/*!	Returns a NavVertexSafePtr that identifies the starting vertex of this edge.
		If this object is invalid, returns an invalid NavVertexSafePtr. */
	KY_FORCE_INLINE NavVertexSafePtr GetStartVertexSafePtr() const { return IsValid() ? NavVertexSafePtr(m_floorSafePtr, GetNavFloor_UnSafe().HalfEdgeIdxToStartVertexIdx(m_halfEdgeIdx)) : NavVertexSafePtr(); }

	/*!	Updates \c resultSafePtr to identify the starting vertex of this edge.
		If this object is invalid, \c resultSafePtr is invalidated. */
	KY_FORCE_INLINE void GetStartVertexSafePtr(NavVertexSafePtr& resultSafePtr) const { IsValid() ? resultSafePtr.Set(m_floorSafePtr, GetNavFloor_UnSafe().HalfEdgeIdxToStartVertexIdx(m_halfEdgeIdx)) : resultSafePtr.Invalidate(); }


	// ---------- Edge to EndVertex ----------

	/*!	Returns the index of the end vertex of this edge within its NavFloor.
		If this object is invalid, returns #NavVertexIdx_Invalid. */
	KY_FORCE_INLINE NavVertexIdx GetEndVertexIdx() const { return IsValid() ? GetNavFloor_UnSafe().HalfEdgeIdxToEndVertexIdx(m_halfEdgeIdx) : NavVertexIdx_Invalid; }

	/*!	Returns the end vertex of this edge.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavVertex* GetEndVertex() const { return IsValid() ? &(GetNavFloor_UnSafe().HalfEdgeIdxToEndVertex(m_halfEdgeIdx)) : KY_NULL; }

	/*!	Returns a NavVertexSafePtr that identifies the end vertex of this edge.
		If this object is invalid, returns an invalid NavVertexSafePtr. */
	KY_FORCE_INLINE NavVertexSafePtr GetEndVertexSafePtr() const { return IsValid() ? NavVertexSafePtr(m_floorSafePtr, GetNavFloor_UnSafe().HalfEdgeIdxToEndVertexIdx(m_halfEdgeIdx)) : NavVertexSafePtr(); }

	/*!	Updates \c resultSafePtr to identify the end vertex of this edge.
		If this object is invalid, \c resultSafePtr is invalidated. */
	KY_FORCE_INLINE void GetEndVertexSafePtr(NavVertexSafePtr& resultSafePtr) const { IsValid() ? resultSafePtr.Set(m_floorSafePtr, GetNavFloor_UnSafe().HalfEdgeIdxToEndVertexIdx(m_halfEdgeIdx)) : resultSafePtr.Invalidate(); }


	// ---------- Edge to the third vertex of the triangle ----------

	/*!	Returns the index of the vertex that is in the same triangle as this edge but that is not connected to this edge.
		If this object is invalid, returns #NavVertexIdx_Invalid. */
	KY_FORCE_INLINE NavVertexIdx GetThirdVertexIdx() const { return IsValid() ? GetNavFloor_UnSafe().HalfEdgeIdxToThirdVertexIdx(m_halfEdgeIdx) : NavVertexIdx_Invalid; }

	/*!	Returns the vertex that is in the same triangle as this edge but that is not connected to this edge.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavVertex* GetThirdVertex() const { return IsValid() ? &(GetNavFloor_UnSafe().HalfEdgeIdxToThirdVertex(m_halfEdgeIdx)) : KY_NULL; }

	/*!	Returns a NavVertexSafePtr that identifies the vertex that is in the same triangle as this edge but that is not connected to this edge.
		If this object is invalid, returns an invalid NavVertexSafePtr. */
	KY_FORCE_INLINE NavVertexSafePtr GetThirdVertexSafePtr() const { return IsValid() ? NavVertexSafePtr(m_floorSafePtr, GetNavFloor_UnSafe().HalfEdgeIdxToThirdVertexIdx(m_halfEdgeIdx)) : NavVertexSafePtr(); }

	/*!	Updates \c resultSafePtr to identify the vertex that is in the same triangle as this edge but that is not connected to this edge.
		If this object is invalid, \c resultSafePtr is invalidated. */
	KY_FORCE_INLINE void GetThirdVertexSafePtr(NavVertexSafePtr& resultSafePtr) const { IsValid() ? resultSafePtr.Set(m_floorSafePtr, GetNavFloor_UnSafe().HalfEdgeIdxToThirdVertexIdx(m_halfEdgeIdx)) : resultSafePtr.Invalidate(); }


	// ---------- Edge to Next Edge ----------

	/*!	Returns the index of the next edge in the triangle.
		If this object is invalid, returns #NavHalfEdgeIdx_Invalid. */
	KY_FORCE_INLINE NavHalfEdgeIdx GetNextHalfEdgeIdx() const { return IsValid() ? NavFloor::HalfEdgeIdxToNextHalfEdgeIdx(m_halfEdgeIdx) : NavHalfEdgeIdx_Invalid; }

	/*!	Returns the next NavHalfEdge in the triangle.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavHalfEdge* GetNextHalfEdge() const { return IsValid() ? &(GetNavFloor_UnSafe().HalfEdgeIdxToNextHalfEdge(m_halfEdgeIdx)) : KY_NULL; }

	/*!	Returns a NavHalfEdgeSafePtr that identifies the next NavHalfEdge in the triangle.
		If this object is invalid, returns an invalid #NavHalfEdgeSafePtr. */
	KY_FORCE_INLINE NavHalfEdgeSafePtr GetNextHalfEdgeSafePtr() const { return IsValid() ? NavHalfEdgeSafePtr(m_floorSafePtr, NavFloor::HalfEdgeIdxToNextHalfEdgeIdx(m_halfEdgeIdx)) : NavHalfEdgeSafePtr(); }

	/*!	Updates \c resultSafePtr to identify the next NavHalfEdge in the triangle.
		If this object is invalid, \c resultSafePtr is invalidated. */
	KY_FORCE_INLINE void GetNextHalfEdgeSafePtr(NavHalfEdgeSafePtr& resultSafePtr) const { IsValid() ? resultSafePtr.Set(m_floorSafePtr, NavFloor::HalfEdgeIdxToNextHalfEdgeIdx(m_halfEdgeIdx)) : resultSafePtr.Invalidate(); }


	// ---------- Edge to Prev Edge ----------

	/*!	Returns the index of the previous edge in the triangle.
		If this object is invalid, returns #NavHalfEdgeIdx_Invalid. */
	KY_FORCE_INLINE NavHalfEdgeIdx GetPrevHalfEdgeIdx() const { return IsValid() ? NavFloor::HalfEdgeIdxToPrevHalfEdgeIdx(m_halfEdgeIdx) : NavHalfEdgeIdx_Invalid; }

	/*!	Returns the previous NavHalfEdge in the triangle.
		If this object is invalid, returns #KY_NULL. */
	KY_FORCE_INLINE const NavHalfEdge* GetPrevHalfEdge() const { return IsValid() ? &(GetNavFloor_UnSafe().HalfEdgeIdxToPrevHalfEdge(m_halfEdgeIdx)) : KY_NULL; }

	/*!	Returns a NavHalfEdgeSafePtr that identifies the previous NavHalfEdge in the triangle.
		If this object is invalid, returns an invalid #NavHalfEdgeSafePtr. */
	KY_FORCE_INLINE NavHalfEdgeSafePtr GetPrevHalfEdgeSafePtr() const { return IsValid() ? NavHalfEdgeSafePtr(m_floorSafePtr, NavFloor::HalfEdgeIdxToPrevHalfEdgeIdx(m_halfEdgeIdx)) : NavHalfEdgeSafePtr(); }

	/*!	Updates \c resultSafePtr to identify the previous NavHalfEdge in the triangle.
		If this object is invalid, \c resultSafePtr is invalidated. */
	KY_FORCE_INLINE void GetPrevHalfEdgeSafePtr(NavHalfEdgeSafePtr& resultSafePtr) const { IsValid() ? resultSafePtr.Set(m_floorSafePtr, NavFloor::HalfEdgeIdxToPrevHalfEdgeIdx(m_halfEdgeIdx)) : resultSafePtr.Invalidate(); }


	/*!	Returns \c true if the edge identified by this object can be traversed, or \c false if the edge cannot be crossed
		or is invalid.
		\param terrainTypeMask		A bitmask that indicates the terrain types that should be considered navigable during the test. */
	bool IsHalfEdgeCrossable(NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const 
	{
		return IsValid() ? IsHalfEdgeCrossable_UnSafe(terrainTypeMask) : KY_FALSE;
	}

	/*!	Returns \c true if the edge identified by this object can be traversed, and updates \c resultSafePtr. Returns \c
		false if the edge cannot be traversed or is invalid.
		\param[out] resultSafePtr	The edge in the next adjacent triangle that borders this edge.
		\param terrainTypeMask		A bitmask that indicates the terrain types that should be considered navigable during the test. */
	bool IsHalfEdgeCrossable(NavHalfEdgeSafePtr& resultSafePtr, NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const
	{
		return IsValid() ? IsHalfEdgeCrossable_UnSafe(resultSafePtr, terrainTypeMask) : KY_FALSE;
	}

	/*!	Returns \c true if the edge identified by this object is adjacent to a triangle in another cell.  Returns \c
		false if the edge is not linked or is invalid.
		\param terrainTypeMask		A bitmask that indicates the terrain types that should be considered navigable during the test.
		\pre	The type of this edge returned by GetHalfEdgeType() must be #CELL_BOUNDARY. */
	bool IsEdgeLinked(NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const
	{
		return IsValid() ? IsEdgeLinked_UnSafe(terrainTypeMask) : KY_FALSE;
	}

	/*!	Returns \c true if the edge identified by this object is adjacent to a triangle in another cell, and updates \c resultSafePtr.
		Returns \c false if the edge is not linked or is invalid.
		\param[out] resultSafePtr	The edge in the next adjacent triangle that borders this edge.
		\param terrainTypeMask		A bitmask that indicates the terrain types that should be considered navigable during the test.
		\pre	The type of this edge returned by GetHalfEdgeType must be #CELL_BOUNDARY. */
	bool IsEdgeLinked(NavHalfEdgeSafePtr& resultSafePtr, NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const
	{
		return IsValid() ? IsEdgeLinked_UnSafe(resultSafePtr, terrainTypeMask) : KY_FALSE;
	}

	/*!	Returns a NavHalfEdgePtr that identifies the edge in the next adjacent triangle that borders this edge. If this
		object is invalid, returns #KY_NULL.
		\pre	The type of this edge returned by GetHalfEdgeType() must be #CELL_BOUNDARY. */
	const NavHalfEdgePtr* GetEdgeLink() const
	{
		return IsValid() ? &(GetEdgeLink_UnSafe()) : KY_NULL;
	}

	/*!	Returns a NavHalfEdgeSafePtr that identifies the edge in the next adjacent triangle that borders this edge. If this
		object is invalid, or a paired edge does not exist, returns an invalid NavHalfEdgeSafePtr.
		\pre	The type of this edge returned by GetHalfEdgeType() must not be #OBSTACLE. */
	NavHalfEdgeSafePtr GetPairHalfEdgePtr() const
	{
		return IsValid() ? GetPairHalfEdgePtr_UnSafe() : NavHalfEdgeSafePtr();
	}

	/*!	Updates \c resultSafePtr to identify the edge in the next adjacent triangle that borders this edge. If this
		object is invalid, or a paired edge does not exist, \c resultSafePtr is invalidated.
		\pre	The type of this edge returned by GetHalfEdgeType() must not be #OBSTACLE. */
	void GetPairHalfEdgePtr(NavHalfEdgeSafePtr& resultSafePtr) const
	{
		IsValid() ? GetPairHalfEdgePtr_UnSafe(resultSafePtr) : resultSafePtr.Invalidate();
	}

private :
	bool IsHalfEdgeCrossable_UnSafe(NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const;
	bool IsHalfEdgeCrossable_UnSafe(NavHalfEdgeSafePtr& resultSafePtr, NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const;
	bool IsEdgeLinked_UnSafe(NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const;
	bool IsEdgeLinked_UnSafe(NavHalfEdgeSafePtr& resultSafePtr, NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const;
	const NavHalfEdgePtr& GetEdgeLink_UnSafe() const;
	NavHalfEdgeSafePtr GetPairHalfEdgePtr_UnSafe() const;
	void GetPairHalfEdgePtr_UnSafe(NavHalfEdgeSafePtr& resultSafePtr) const;


private :
	NavFloorSafePtr m_floorSafePtr;
	NavHalfEdgeIdx m_halfEdgeIdx;
};


}

#endif // KyPathData_NavHalfEdgeSafePtr_H

