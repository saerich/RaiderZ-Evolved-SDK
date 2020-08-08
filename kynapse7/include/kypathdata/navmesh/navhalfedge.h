/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavHalfEdge_H
#define KyPathData_NavHalfEdge_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/blob/endianness.h"
#include "kypathdata/generic/memory.h"

namespace Kaim
{

/*! Enumerates the possible types of boundary that can be represented by a NavHalfEdge.
	\ingroup kypathdata_navmesh */
enum NavHalfEdgeType
{
	PAIRED = 0, /*!< Indicates that another NavHalfEdge in the same NavFloor lies adjacent to this object. */
	OBSTACLE = 1, /*!< Indicates that this NavHalfEdge lies on an external border of the NavMesh. */
	FLOOR_BOUNDARY = 2, /*!< Indicates that this NavHalfEdge lies on the border of its NavFloor. */
	CELL_BOUNDARY = 3, /*!< Indicates that this NavHalfEdge lies on the border of its NavFloor and its NavCell. */

	NavHalfEdgeType_FORCE32 = 0xFFFFFFFF
};

/*! Each instance of NavHalfEdge represents a single edge of a single triangle within the NavMesh. 
	\pp It is called a "half-edge" because adjacent triangles do not share the edge that forms their border;
	instead, each triangle maintains its own half-edge to represent its own boundary.
	\ingroup kypathdata_navmesh */ 
class NavHalfEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	NavHalfEdge() {}

	/*! Read accessor for #m_startVerticesIdx. */
	NavVertexIdx GetStartVertexIdx() const { return m_startVerticesIdx; }

	/*! If this edge borders an edge in an adjoining triangle, this method returns the index of that adjacent
		edge. Only call this method if GetHalfEdgeType() returns #NavHalfEdgeType::PAIRED. */
	KY_FORCE_INLINE NavHalfEdgeIdx GetPairHalfEdgeIdx() const;

	/*! If this edge borders an obstacle, this method returns the index of the next edge that borders that obstacle. Only call this method
		if GetHalfEdgeType() returns #NavHalfEdgeType::OBSTACLE. */
	KY_FORCE_INLINE NavHalfEdgeIdx GetNextObstacleHalfEdgeIdx() const;

	/*! If this edge lies on the border of the NavFloor, this method returns the index of this edge among the boundary edges
		maintained by its NavFloor. Only call this method if GetHalfEdgeType() returns #NavHalfEdgeType::FLOOR_BOUNDARY. */
	KY_FORCE_INLINE KyUInt32 GetFloorLinkIdx() const;

	/*! If this edge lies on the border of the NavCell, this method returns the index of this edge among the boundary edges
		maintained by its NavCell for the same cardinal direction. Only call this method if GetHalfEdgeType() returns
		#NavHalfEdgeType::CELL_BOUNDARY. */
	KY_FORCE_INLINE KyUInt32 GetCellLinkIdx() const;

	/*! If this edge lies on the border of the NavCell, this method returns the cardinal direction of the border
		where it lies in its NavCell. Only call this method if GetHalfEdgeType() returns
		#NavHalfEdgeType::CELL_BOUNDARY. */
	KY_FORCE_INLINE KyUInt32 GetCellBoundaryDir() const;

	/*! Returns an element from the #NavHalfEdgeType enumeration that indicates the type of border this edge represents. */
	KY_FORCE_INLINE NavHalfEdgeType GetHalfEdgeType() const { return (NavHalfEdgeType)(m_pairData >> 30); }
	
public:
	NavVertexIdx m_startVerticesIdx; /*!< The index of the starting vertex of this edge. Do not modify. */
	KyUInt32 m_pairData; /*!< The index of the adjoining half-edge. Access through GetPairHalfEdgeIdx(). Do not modify. */
};

/*! \label_swapendianness
	\ingroup kypathdata_navmesh */
KY_FORCE_INLINE void SwapEndianness(Endianness::Target e, NavHalfEdge& self)
{
	SwapEndianness(e, self.m_startVerticesIdx);
	SwapEndianness(e, self.m_pairData);
}

KY_FORCE_INLINE NavHalfEdgeIdx NavHalfEdge::GetPairHalfEdgeIdx() const
{
	KY_FUNCTION("NavHalfEdge::GetPairHalfEdgeIdx");
	KY_ASSERT((GetHalfEdgeType() == PAIRED), ("You must not call this function if the halfEdge is not of type NavHalfEdgeType::PAIRED"));

	return (NavHalfEdgeIdx)(m_pairData & 0x0FFFFFFF);
}

KY_FORCE_INLINE NavHalfEdgeIdx NavHalfEdge::GetNextObstacleHalfEdgeIdx() const
{
	KY_FUNCTION("NavHalfEdge::GetNextObstacleHalfEdgeIdx");
	KY_ASSERT(GetHalfEdgeType() == OBSTACLE, ("You must not call this function if the halfEdge is not of type NavHalfEdgeType::OBSTACLE"));

	return (NavHalfEdgeIdx)(m_pairData & 0x0FFFFFFF);
}

KY_FORCE_INLINE KyUInt32 NavHalfEdge::GetFloorLinkIdx() const
{
	KY_FUNCTION("NavHalfEdge::GetFloorLinkIdx");
	KY_ASSERT(GetHalfEdgeType() == FLOOR_BOUNDARY, ("You must not call this function if the halfEdge is not of type NavHalfEdgeType::FLOOR_BOUNDARY"));

	return (NavHalfEdgeIdx)(m_pairData & 0x0FFFFFFF);
}

KY_FORCE_INLINE KyUInt32 NavHalfEdge::GetCellLinkIdx() const
{
	KY_FUNCTION("NavHalfEdge::GetCellLinkIdx");
	KY_ASSERT(GetHalfEdgeType() == CELL_BOUNDARY, ("You must not call this function if the halfEdge is not of type NavHalfEdgeType::CELL_BOUNDARY"));

	return (NavHalfEdgeIdx)(m_pairData & 0x0FFFFFFF);
}

KY_FORCE_INLINE CardinalDir NavHalfEdge::GetCellBoundaryDir() const
{
	KY_FUNCTION("NavHalfEdge::GetCellBoundaryDir");
	KY_ASSERT(GetHalfEdgeType() == CELL_BOUNDARY, ("You must not call this function if the halfEdge is not of type NavHalfEdgeType::CELL_BOUNDARY"));

	return (CardinalDir)( (m_pairData & 0x30000000) >> 28);
}

}

#endif //KyPathData_NavTriangle_H

