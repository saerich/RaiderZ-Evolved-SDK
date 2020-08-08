/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavHalfEdgePtr_H
#define KyPathData_NavHalfEdgePtr_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navfloorptr.h"
#include "kypathdata/navmesh/navvertexptr.h"
#include "kypathdata/navmesh/navhalfedgesafeptr.h"

namespace Kaim
{

class NavTrianglePtr;


///////////////////////////////////////////////////////////////////////////////////////////
// NavHalfEdgePtr
///////////////////////////////////////////////////////////////////////////////////////////

/*! Each instance of this class uniquely identifies a single edge in a NavMesh triangle.
	\pp This pointer is guaranteed to be valid only in the frame in which it was retrieved.
	Never store a NavHalfEdgePtr for use in subsequent frames, because it has no protection against data
	streaming. Use NavHalfEdgeSafePtr instead.
	\ingroup kypathdata_navmesh */
class NavHalfEdgePtr
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:

	/*!	\label_constructor. Creates an invalid NavHalfEdgePtr. */
	KY_FORCE_INLINE NavHalfEdgePtr() : m_floorPtr(), m_halfEdgeIdx(NavHalfEdgeIdx_Unset) {}

	/*!	\label_constructor
		\param stitchedNavCell		A pointer to the StitchedNavCell that handles the NavCell that contains this edge.
		\param floorIdx				The index of the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE NavHalfEdgePtr(StitchedNavCell* stitchedNavCell, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx) :
		m_floorPtr(stitchedNavCell, floorIdx), m_halfEdgeIdx(halfEdgeIdx) {}

	/*!	\label_constructor
		\param stitchedNavCell		A reference to the StitchedNavCell that handles the NavCell that contains this edge.
		\param floorIdx				The index of the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE NavHalfEdgePtr(StitchedNavCell& stitchedNavCell, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx) :
		m_floorPtr(stitchedNavCell, floorIdx), m_halfEdgeIdx(halfEdgeIdx) {}

	/*!	\label_constructor
		\param cellPtr				A NavCellPtr that identifies the NavCell that contains this edge.
		\param floorIdx				The index of the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE NavHalfEdgePtr(const NavCellPtr& cellPtr, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx) :
		m_floorPtr(cellPtr, floorIdx),  m_halfEdgeIdx(halfEdgeIdx) {}

	/*!	\label_constructor
		\param floorPtr				A NavFloorPtr that identifies the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE NavHalfEdgePtr(const NavFloorPtr& floorPtr, NavHalfEdgeIdx halfEdgeIdx) : m_floorPtr(floorPtr), m_halfEdgeIdx(halfEdgeIdx) {}

	/*! \label_constructor Copies the state of the specified NavHalfEdgePtr to the newly created object. */
	KY_FORCE_INLINE NavHalfEdgePtr(const NavHalfEdgePtr& halfEdgePtr) : m_floorPtr(halfEdgePtr.GetNavFloorPtr()), m_halfEdgeIdx(halfEdgePtr.GetHalfEdgeIdx()) {}

	/*!	\label_reinit
		\param stitchedNavCell		A pointer to the StitchedNavCell that handles the NavCell that contains this edge.
		\param floorIdx				The index of the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE void Set(StitchedNavCell* stitchedNavCell, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx)
	{
		m_floorPtr.Set(stitchedNavCell, floorIdx);
		m_halfEdgeIdx = halfEdgeIdx;
	}

	/*!	\label_reinit
		\param stitchedNavCell		A reference to the StitchedNavCell that handles the NavCell that  contains this edge.
		\param floorIdx				The index of the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE void Set(StitchedNavCell& stitchedNavCell, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx)
	{
		m_floorPtr.Set(stitchedNavCell, floorIdx);
		m_halfEdgeIdx = halfEdgeIdx;
	}

	/*!	\label_reinit
		\param cellPtr				A NavCellPtr that identifies the NavCell that contains this edge.
		\param floorIdx				The index of the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavCellPtr& cellPtr, NavFloorIdx floorIdx, NavHalfEdgeIdx halfEdgeIdx)
	{
		m_floorPtr.Set(cellPtr, floorIdx);
		m_halfEdgeIdx = halfEdgeIdx;
	}

	/*!	\label_reinit
		\param floorPtr				A NavFloorPtr that identifies the NavFloor that contains this edge.
		\param halfEdgeIdx			The index of this edge within its NavFloor. */
	KY_FORCE_INLINE void Set(const NavFloorPtr& floorPtr, NavHalfEdgeIdx halfEdgeIdx)
	{
		m_floorPtr = floorPtr;
		m_halfEdgeIdx = halfEdgeIdx;
	}

	/*! Returns \c true if this object refers to a valid edge: i.e. an edge in a NavMesh that is currently loaded in memory. */
	KY_FORCE_INLINE bool IsValid() const { return m_floorPtr.IsValid() && m_halfEdgeIdx < m_floorPtr.GetNavFloor().GetHalfEdgeCount(); }

	/*! Invalidates this object. */
	KY_FORCE_INLINE void Invalidate() { m_floorPtr.Invalidate(); m_halfEdgeIdx = NavHalfEdgeIdx_Unset; }

	/*!	Returns \c true if this object identifies the same edge as \c halfEdgePtr, or if both are invalid. */
	KY_FORCE_INLINE bool operator ==(const NavHalfEdgePtr& halfEdgePtr) const { return m_halfEdgeIdx == halfEdgePtr.m_halfEdgeIdx && m_floorPtr == halfEdgePtr.m_floorPtr; }

	/*!	Returns \c true if this object identifies an edge different from the one identified by \c halfEdgePtr, or if only one of them is invalid. */
	KY_FORCE_INLINE bool operator !=(const NavHalfEdgePtr& halfEdgePtr) const { return m_halfEdgeIdx != halfEdgePtr.m_halfEdgeIdx || m_floorPtr != halfEdgePtr.m_floorPtr; }

	KY_FORCE_INLINE bool operator < (const NavHalfEdgePtr& halfEdgePtr) const { return m_floorPtr < halfEdgePtr.m_floorPtr || (m_floorPtr == halfEdgePtr.m_floorPtr && m_halfEdgeIdx < halfEdgePtr.m_halfEdgeIdx); }
	KY_FORCE_INLINE bool operator <=(const NavHalfEdgePtr& halfEdgePtr) const { return !(halfEdgePtr < *this); }

	/*!	Returns the index of this edge within its NavFloor. */
	KY_FORCE_INLINE NavHalfEdgeIdx GetHalfEdgeIdx() const { return m_halfEdgeIdx; }

	/*!	Returns the index of the NavFloor that contains this edge. */
	KY_FORCE_INLINE NavFloorIdx GetFloorIdx() const { return GetNavFloorPtr().GetFloorIdx(); }

	/*!	Returns a reference to the NavFloorPtr that identifies the NavFloor that contains this edge. */
	KY_FORCE_INLINE NavFloorPtr& GetNavFloorPtr() { return m_floorPtr; }

	/*!	Returns a const reference to the NavFloorPtr that identifies the NavFloor that contains this edge. */
	KY_FORCE_INLINE const NavFloorPtr& GetNavFloorPtr() const { return m_floorPtr; }

	/*!	Returns a reference to the NavCellPtr that identifies the NavCell that contains this edge. */
	KY_FORCE_INLINE NavCellPtr& GetNavCellPtr() { return m_floorPtr.GetNavCellPtr(); }

	/*!	Returns a const reference to the NavCellPtr that identifies the NavCell that contains this edge. */
	KY_FORCE_INLINE const NavCellPtr& GetNavCellPtr() const { return m_floorPtr.GetNavCellPtr(); }


	/*!	Constructs a new instance of NavHalfEdgeSafePtr that identifies the same edge as this object.
		If the current instance is invalid, it returns an invalid NavHalfEdgeSafePtr. */
	inline NavHalfEdgeSafePtr ConvertToNavHalfEdgeSafePtr() const
	{
		if (IsValid())
		{
			const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
			return NavHalfEdgeSafePtr(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_halfEdgeIdx);
		}

		return NavHalfEdgeSafePtr();
	}

	/*!	Updates \c halfEdgeSafePtr to identify the same edge as this object.
		If the current instance is invalid, \c halfEdgeSafePtr is invalidated. */
	inline void ConvertToNavHalfEdgeSafePtr(NavHalfEdgeSafePtr& halfEdgeSafePtr) const
	{
		if (IsValid())
		{
			const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
			halfEdgeSafePtr.Set(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_halfEdgeIdx);
		}
		else
			halfEdgeSafePtr.Invalidate();
	}

	/*!	Constructs a new instance of NavHalfEdgeSafePtr that identifies the same edge as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline NavHalfEdgeSafePtr ConvertToNavHalfEdgeSafePtr_UnSafe() const
	{
		const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
		return NavHalfEdgeSafePtr(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_halfEdgeIdx);
	}

	/*!	Updates \c halfEdgeSafePtr to identify the same edge as this object.
		\pre	This object must be valid. Its validity is not checked. */
	inline void ConvertToNavHalfEdgeSafePtr_UnSafe(NavHalfEdgeSafePtr& halfEdgeSafePtr) const
	{
		const StitchedNavCell& stitchedNavCell = GetStitchedNavCell();
		halfEdgeSafePtr.Set(stitchedNavCell.m_parentStitchedNavMesh->m_reference, stitchedNavCell.m_idxInTheBufferOfStitchedNavCell, GetFloorIdx(), m_halfEdgeIdx);
	}

	/*!	Returns a const reference to the StitchedNavCell that handles the NavCell that contains this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const StitchedNavCell& GetStitchedNavCell() const { return m_floorPtr.GetStitchedNavCell(); }
	
	/*!	Returns a const reference to the NavCell that contains this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavCell& GetNavCell() const { return m_floorPtr.GetNavCell(); }

	/*!	Returns a const reference to the NavFloor that contains this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavFloor& GetNavFloor() const { return m_floorPtr.GetNavFloor(); }

	/*!	Returns the terrain type of the edge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavTerrainType GetNavFloorTerrainType() const { return GetNavFloor().GetNavTerrainType(); }

	/*!	Returns a const reference to the NavHalfEdge identified by this object.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavHalfEdge& GetHalfEdge() const { return GetNavFloor().GetHalfEdge(m_halfEdgeIdx); }

	/*!	Returns the NavHalfEdgeType that indicates what type of boundary this edge represents.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavHalfEdgeType GetHalfEdgeType() const { return GetNavFloor().GetHalfEdgeType(m_halfEdgeIdx); }


	// ---------- Edge to StartVertex ----------

	/*!	Returns the index of the starting vertex of this edge within its NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavVertexIdx GetStartVertexIdx() const { return GetNavFloor().HalfEdgeIdxToStartVertexIdx(m_halfEdgeIdx); }

	/*!	Returns the starting vertex of this edge within its NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavVertex& GetStartVertex() const { return GetNavFloor().HalfEdgeIdxToStartVertex(m_halfEdgeIdx); }

	/*!	Returns a NavVertexPtr that identifies the starting vertex of this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavVertexPtr GetStartVertexPtr() const { return NavVertexPtr(m_floorPtr, GetStartVertexIdx()); }

	/*!	Updates \c resultPtr to identify the starting vertex of this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE void GetStartVertexPtr(NavVertexPtr& resultPtr) const { resultPtr.Set(m_floorPtr, GetStartVertexIdx()); }


	// ---------- Edge to EndVertex ----------

	/*!	Returns the index of the end vertex of this edge within its NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavVertexIdx GetEndVertexIdx() const { return GetNavFloor().HalfEdgeIdxToEndVertexIdx(m_halfEdgeIdx); }

	/*!	Returns the end vertex of this edge within its NavFloor.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavVertex& GetEndVertex() const { return GetNavFloor().HalfEdgeIdxToEndVertex(m_halfEdgeIdx); }

	/*!	Returns a NavVertexPtr that identifies the end vertex of this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavVertexPtr GetEndVertexPtr() const { return NavVertexPtr(m_floorPtr, GetEndVertexIdx()); }

	/*!	Updates \c resultPtr to identify the end vertex of this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE void GetEndVertexPtr(NavVertexPtr& resultPtr) const { resultPtr.Set(m_floorPtr, GetEndVertexIdx()); }


	// ---------- Edge to ThirdVertex ----------

	/*!	Returns the index of the vertex that is in the same triangle as this edge but that is not connected to this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavVertexIdx GetThirdVertexIdx() const { return GetNavFloor().HalfEdgeIdxToThirdVertexIdx(m_halfEdgeIdx); }

	/*!	Returns the vertex that is in the same triangle as this edge but that is not connected to this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavVertex& GetThirdVertex() const { return GetNavFloor().HalfEdgeIdxToThirdVertex(m_halfEdgeIdx); }

	/*!	Returns a NavVertexPtr that identifies the vertex that is in the same triangle as this edge but that is not connected to this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavVertexPtr GetThirdVertexPtr() const { return NavVertexPtr(m_floorPtr, GetThirdVertexIdx()); }

	/*!	Updates \c resultPtr to identify the vertex that is in the same triangle as this edge but that is not connected to this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE void GetThirdVertexPtr(NavVertexPtr& resultPtr) const { resultPtr.Set(m_floorPtr, GetThirdVertexIdx()); }


	// ---------- Edge to Next Edge ----------

	/*!	Returns the index of the next edge in the triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavHalfEdgeIdx GetNextHalfEdgeIdx() const { return NavFloor::HalfEdgeIdxToNextHalfEdgeIdx(m_halfEdgeIdx); }

	/*!	Returns the next edge in the triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavHalfEdge& GetNextHalfEdge() const { return GetNavFloor().HalfEdgeIdxToNextHalfEdge(m_halfEdgeIdx); }

	/*!	Returns a NavHalfEdgePtr that identifies the next edge in the triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavHalfEdgePtr GetNextHalfEdgePtr() const { return NavHalfEdgePtr(m_floorPtr, GetNextHalfEdgeIdx()); }

	/*!	Updates \c resultPtr to identify the next edge in the triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE void GetNextHalfEdgePtr(NavHalfEdgePtr& resultPtr) const { resultPtr.Set(m_floorPtr, GetNextHalfEdgeIdx()); }

	// ---------- Edge to Prev Edge ----------

	/*!	Returns the index of the previous edge in the triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavHalfEdgeIdx GetPrevHalfEdgeIdx() const { return NavFloor::HalfEdgeIdxToPrevHalfEdgeIdx(m_halfEdgeIdx); }

	/*!	Returns the previous edge in the triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE const NavHalfEdge& GetPrevHalfEdge() const { return GetNavFloor().HalfEdgeIdxToPrevHalfEdge(m_halfEdgeIdx); }

	/*!	Returns a NavHalfEdgePtr that identifies the previous edge in the triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavHalfEdgePtr GetPrevHalfEdgePtr() const { return NavHalfEdgePtr(m_floorPtr, GetPrevHalfEdgeIdx()); }

	/*!	Updates \c resultPtr to identify the previous edge in the triangle.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE void GetPrevHalfEdgePtr(NavHalfEdgePtr& resultPtr) const { resultPtr.Set(m_floorPtr, GetPrevHalfEdgeIdx()); }


	// ---------- Edge to Pair Edge ----------
private :

	/*	Returns the \c NavTerrainType of the pair \NavHalfEdge of the edge identified  by the current instance
		Pre-condition 1 : current instance must be valid
		Pre-condition 2 : the pair \NavHalfEdge must be in the same \c NavCell, which means the HalfEdgeType of
		the edge must be PAIRED or FLOOR_BOUNDARY
	*/
	inline NavTerrainType GetPairHalfEdgeTerrainTypeInCell() const
	{
		const NavCell& navCell = GetNavCell();
		const NavFloor& navFloor = navCell.GetNavFloor(m_floorPtr.GetFloorIdx());
		NavHalfEdgeType edgeType = navFloor.GetHalfEdgeType(m_halfEdgeIdx);

		KY_FUNCTION("NavHalfEdgePtr::GetPairHalfeEdgeTerrainTypeInCell");
		KY_ASSERT(edgeType != OBSTACLE && edgeType != CELL_BOUNDARY, ("Invalid edge type : cannot compute the paired half-edge of an half-edge of type OBSTACLE or CELL_BOUNDARY"));

		if (edgeType == PAIRED)
			return navFloor.GetNavTerrainType();

		const FloorLink& pairEdge = navFloor.GetFloorLink(m_halfEdgeIdx);
		return navCell.GetNavFloorTerrainType(pairEdge.m_floorIdx);
	}

	/*	Returns the \c NavHalfEdgePtr that identifies the pair \NavHalfEdge of the edge identified  by the current instance
		Pre-condition 1 : current instance must be valid
		Pre-condition 2 : the pair \NavHalfEdge must be in the same \c NavCell, which means the HalfEdgeType of
		the edge must be PAIRED or FLOOR_BOUNDARY
	*/
	inline NavHalfEdgePtr GetPairHalfEdgePtrInCell() const 
	{
		const NavCell& navCell = GetNavCell();
		const NavFloor& navFloor = navCell.GetNavFloor(m_floorPtr.GetFloorIdx());
		NavHalfEdgeType edgeType = navFloor.GetHalfEdgeType(m_halfEdgeIdx);

		KY_FUNCTION("NavHalfEdgePtr::GetPairHalfEdgePtrInCell");
		KY_ASSERT(edgeType != OBSTACLE && edgeType != CELL_BOUNDARY, ("Invalid edge type : cannot compute the paired half-edge of an half-edge of type OBSTACLE or CELL_BOUNDARY"));

		if (edgeType == PAIRED)
			return NavHalfEdgePtr(m_floorPtr, navFloor.HalfEdgeIdxToPairHalfEdgeIdx(m_halfEdgeIdx));

		const FloorLink& pairEdge = navFloor.GetFloorLink(m_halfEdgeIdx);
		return NavHalfEdgePtr(GetNavCellPtr(), pairEdge.m_floorIdx, pairEdge.m_halfEdgeIdx);
	}

	/*	Updates \c resultPtr to make it identify the pair \NavHalfEdge of the edge identified  by the current instance
		Pre-condition 1 : current instance must be valid
		Pre-condition 2 : the pair \NavHalfEdge must be in the same \c NavCell, which means the HalfEdgeType of
		the edge must be PAIRED or FLOOR_BOUNDARY
	*/
	inline void GetPairHalfEdgePtrInCell(NavHalfEdgePtr& resultPtr) const 
	{
		const NavCell& navCell = GetNavCell();
		const NavFloor& navFloor = navCell.GetNavFloor(m_floorPtr.GetFloorIdx());
		NavHalfEdgeType edgeType = navFloor.GetHalfEdgeType(m_halfEdgeIdx);

		KY_FUNCTION("NavHalfEdgePtr::GetPairHalfEdgePtrInCell");
		KY_ASSERT(edgeType != OBSTACLE && edgeType != CELL_BOUNDARY, ("Invalid edge type : cannot compute the paired half-edge of an half-edge of type OBSTACLE or CELL_BOUNDARY"));

		if (edgeType == PAIRED)
			return resultPtr.Set(m_floorPtr, navFloor.HalfEdgeIdxToPairHalfEdgeIdx(m_halfEdgeIdx));

		const FloorLink& pairEdge = navFloor.GetFloorLink(m_halfEdgeIdx);
		return resultPtr.Set(GetNavCellPtr(), pairEdge.m_floorIdx, pairEdge.m_halfEdgeIdx);
	}

public :
	/*!	Returns \c true if the edge identified by this object can be traversed.
		\param terrainTypeMask		A bitmask that indicates the terrain types that should be considered navigable during the test.
		\pre	This object must be valid. Its validity is not checked. */
	bool IsHalfEdgeCrossable(NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const;

	/*!	Returns \c true if the edge identified by this object can be traversed, and updates \c resultSafePtr.
		\param[out] resultPtr		The edge in the next adjacent triangle that borders this edge.
		\param terrainTypeMask		A bitmask that indicates the terrain types that should be considered navigable during the test.
		\pre	This object must be valid. Its validity is not checked. */
	bool IsHalfEdgeCrossable(NavHalfEdgePtr& resultPtr, NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const;

	/*!	Returns \c true if the edge identified by this object is adjacent to a triangle in another cell.
		\param terrainTypeMask		A bitmask that indicates the terrain types that should be considered navigable during the test.
		\pre	This object must be valid. Its validity is not checked.
		\pre	The type of this edge returned by GetHalfEdgeType() must be #CELL_BOUNDARY. */
	bool IsEdgeLinked(NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const;

	/*!	Returns \c true if the edge identified by this object is adjacent to a triangle in another cell, and updates \c resultPtr.
		\param[out] resultPtr		The edge in the next adjacent triangle that borders this edge.
		\param terrainTypeMask		A bitmask that indicates the terrain types that should be considered navigable during the test.
		\pre	This object must be valid. Its validity is not checked.
		\pre	The type of this edge returned by GetHalfEdgeType must be #CELL_BOUNDARY. */
	bool IsEdgeLinked(NavHalfEdgePtr& resultPtr, NavTerrainTypeMask terrainTypeMask = NavTerrainTypeMask_All) const;

	/*!	Returns a NavHalfEdgePtr that identifies the edge in the next adjacent triangle that borders this edge.
		\pre	This object must be valid. Its validity is not checked.
		\pre	The type of this edge returned by GetHalfEdgeType() must be #CELL_BOUNDARY. */
	const NavHalfEdgePtr& GetEdgeLink() const;

	/*!	Returns a NavHalfEdgePtr that identifies the edge in the next adjacent triangle that borders this edge.
		\pre	This object must be valid. Its validity is not checked.
		\pre	The type of this edge returned by GetHalfEdgeType() must not be #OBSTACLE. */
	NavHalfEdgePtr GetPairHalfEdgePtr() const;

	/*!	Updates \c resultPtr to identify the edge in the next adjacent triangle that borders this edge.
		\pre	This object must be valid. Its validity is not checked.
		\pre	The type of this edge returned by GetHalfEdgeType() must not be #OBSTACLE. */
	void GetPairHalfEdgePtr(NavHalfEdgePtr& resultPtr) const;

	// ---------- compute the number of the edge in the triangle (0, 1, 2) ----------

	/*!	Returns the index of this edge within its triangle.
		\return 0, 1, or 2
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE KyUInt32 GetHalfEdgeNumberInTriangle() const { return NavFloor::HalfEdgeIdxToHalfEdgeNumberInTriangle(m_halfEdgeIdx); }

	// ---------- edge to owner triangle ----------

	/*!	Returns the index of the triangle that contains this edge.
		\pre	This object must be valid. Its validity is not checked. */
	KY_FORCE_INLINE NavTriangleIdx GetTriangleIdx() const { return NavFloor::HalfEdgeIdxToTriangleIdx(m_halfEdgeIdx); }

	// ---------- edge along border ----------

	/*!	Returns a NavHalfEdgePtr that identifies the next edge along the border of the NavMesh.
		\pre	This object must be valid. Its validity is not checked.
		\pre	The edge identified by this object must be on the border of the NavMesh. It may not be paired with 
				another triangle (IsHalfEdgeCrossable() == false).
		\post	The returned edge is not paired (IsHalfEdgeCrossable() == false). */
	NavHalfEdgePtr GetNextNavHalfEdgePtrAlongObstacle();

	/*!	Returns a NavHalfEdgePtr that identifies the previous edge along the border of the NavMesh.
		\pre	This object must be valid. Its validity is not checked.
		\pre	The edge identified by this object must be on the border of the NavMesh. It may not be paired with 
				another triangle (IsHalfEdgeCrossable() == false).
		\post	The returned edge is not paired (IsHalfEdgeCrossable() == false). */
	NavHalfEdgePtr GetPrevNavHalfEdgePtrAlongObstacle();

private:
	NavFloorPtr m_floorPtr;
	NavHalfEdgeIdx m_halfEdgeIdx;
};


}

#endif // KyPathData_NavHalfEdgePtr_H

