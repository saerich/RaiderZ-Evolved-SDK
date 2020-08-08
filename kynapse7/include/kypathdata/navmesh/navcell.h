/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavCell_H
#define KyPathData_NavCell_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navfloor.h"
#include "kypathdata/blob/blobarray.h"
#include "kypathdata/blob/endianness.h"
#include <kypathdata/basesystem/coordsystem.h>


namespace Kaim
{

/*! Represents one edge of the border around a NavCell. */
struct CellBoundaryEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	NavHalfEdgeIdx m_halfEdgeIdx; /*!< The half-edge that matches this border. */
	NavFloorIdx m_floorIdx; /*!< The NavFloor that contains this border. */
};

/*! \label_swapendianness
	\ingroup kypathdata_navmesh */
KY_FORCE_INLINE void SwapEndianness(Endianness::Target e, CellBoundaryEdge& self)
{
	SwapEndianness(e, self.m_halfEdgeIdx);
	SwapEndianness(e, self.m_floorIdx);
}

/*! Each instance of the NavCell class represents a single cell within the grid of cells maintained by a NavMesh.
	It is responsible for covering a localized area of the terrain with a square 2D footprint, and is responsible
	for covering the entire range of altitudes within that 2D area. 
	\pp Each NavCell contains a set of \NavFloors, which in turn contain the actual triangles that make up the NavMesh.
	\ingroup kypathdata_navmesh */
class NavCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_blobtype */
	static const char* GetBlobType() { return "NavCell"; }
	//version history:
	// 0: initial version
	// 1: change the BBox of floor from Box3f to Box2f
	// 2: Added TerrainTypes to navFloors
	// 3: remove cellSize and CoordSystem from the navCell, change the BBox of the navFloor from a Box2F to a Box3f
	// 4: ?? 
	// 5: Added bool m_isOnOverlap
	// 6: CellBoundary navHalfEdgee know their CardinalDir And Remove the previous bool !
	// 7: Remove bool m_isOnOverlap
	// 8: remove m_cellBoundaryEdgeCount + NavVertexIdx becomes KyUInt8;
	/*! \label_blobversion */
	static KyUInt32 GetBlobVersion() { return 8; }

public:
	/*! \label_constructor For internal use. */
	NavCell() {}

	/*! Retrieves the index of this NavCell on the X axis within the partitioning scheme of its NavMesh. */
	KY_FORCE_INLINE KyInt32 GetCellPosX() const { return m_cellPos.x; }

	/*! Retrieves the index of this NavCell on the Y axis within the partitioning scheme of its NavMesh. */
	KY_FORCE_INLINE KyInt32 GetCellPosY() const { return m_cellPos.y; }

	/*! Retrieves the number of \NavFloors maintained by this NavCell. */
	KY_FORCE_INLINE KyUInt32 GetFloorCount() const { return m_floors.m_count; }

	/*! Returns #KY_TRUE if this NavCell contains no \NavFloors at all. */
	KY_FORCE_INLINE bool IsEmpty() const { return GetFloorCount() == 0; }

	/*! Retrieves the position of this NavCell within the partitioning scheme of its NavMesh. */
	KY_FORCE_INLINE const CellPos& GetCellPos() const { return m_cellPos; }

	/*! Retrieves the axis-aligned bounding box that encloses all triangles in this NavCell. */
	KY_FORCE_INLINE const Box2f& GetAABB() const { return m_aabbox; }

	/*! Retrieves the NavFloor with the specified index. */
	KY_FORCE_INLINE const NavFloor& GetNavFloor(NavFloorIdx idx) const
	{
		KY_FUNCTION("NavCell::GetNavFloor");
		KY_ASSERT(idx < GetFloorCount(), ("Invalid NavFloor index"));

		return m_floors.Values()[idx];
	}

	/*! Retrieves the terrain type of the NavFloor with the specified index. */
	KY_FORCE_INLINE NavTerrainType GetNavFloorTerrainType(NavFloorIdx idx) const { return GetNavFloor(idx).GetNavTerrainType(); }

	/*! Retrieves the boundary edge with the specified index in the list of boundary edges maintained for the specified cardinal direction. */
	KY_FORCE_INLINE const CellBoundaryEdge& GetBoundaryEdge(CardinalDir dir, KyUInt32 idx) const
	{
		KY_FUNCTION("NavCell::GetBoundaryEdgesCount");
		KY_ASSERT(dir < 4, ("Invalid input direction : allowed paramaters are CardinalDir_EAST, NORTH, WEST and SOUTH"));
		KY_ASSERT(idx < m_cellBoundaryEdges[dir].m_count, ("Invalid Boundary edge index"));

		return m_cellBoundaryEdges[dir].Values()[idx];
	}

	/*! Retrieves the number of boundary edges maintained for the specified cardinal direction. */
	KY_FORCE_INLINE KyUInt32 GetBoundaryEdgesCount(CardinalDir dir) const
	{
		KY_FUNCTION("NavCell::GetBoundaryEdgesCount");
		KY_ASSERT(dir < 4, ("Invalid input direction : allowed paramaters are CardinalDir_EAST, NORTH, WEST and SOUTH"));

		return m_cellBoundaryEdges[dir].m_count;
	}

	/*! Retrieves the cardinal direction of the boundary on which the specified NavVertex lies, or #CardinalDir_INVALID
		if the NavVertex does not lie precisely on a boundary. */
	CardinalDir GetBoundaryDirFromPos(const NavVertex& pos) const
	{
		if (pos.x == m_aabbox.m_min.x) return CardinalDir_WEST;
		if (pos.x == m_aabbox.m_max.x) return CardinalDir_EAST;
		if (pos.y == m_aabbox.m_min.y) return CardinalDir_SOUTH;
		if (pos.y == m_aabbox.m_max.y) return CardinalDir_NORTH;

		return CardinalDir_INVALID;
	}

	/*! Indicates whether or not the specified position lies inside #m_aabbox on the (X,Y) plane. */
	KY_FORCE_INLINE bool IsPointInsideCellAABB(const Vec3f& pos) const
	{
		const KyFloat32 operand1 = Fsel(m_aabbox.m_min.x - pos.x, 0.f, 1.f); // operand1 == 1.f if pos.x > cellBox.m_min.x
		const KyFloat32 operand2 = Fsel(m_aabbox.m_max.x - pos.x, 1.f, 0.f); // operand2 == 1.f if cellBox.m_max.x >= pos.x
		const KyFloat32 operand3 = Fsel(pos.y - m_aabbox.m_min.y, 1.f, 0.f); // operand3 == 1.f if pos.y >= cellBox.m_min.y
		const KyFloat32 operand4 = Fsel(pos.y - m_aabbox.m_max.y, 0.f, 1.f); // operand4 == 1.f if cellBox.m_max.y > pos.y

		return operand1 * operand2 * operand3 * operand4 > 0.f;
	}

	/*! Indicates whether or not the specified position lies inside #m_aabbox on the (X,Y) plane. */
	KY_FORCE_INLINE bool IsPointInsideCellAABB(const Vec2f& pos) const
	{
		const KyFloat32 operand1 = Fsel(m_aabbox.m_min.x - pos.x, 0.f, 1.f); // operand1 == 1.f if pos.x > cellBox.m_min.x
		const KyFloat32 operand2 = Fsel(m_aabbox.m_max.x - pos.x, 1.f, 0.f); // operand2 == 1.f if cellBox.m_max.x >= pos.x
		const KyFloat32 operand3 = Fsel(pos.y - m_aabbox.m_min.y, 1.f, 0.f); // operand3 == 1.f if pos.y >= cellBox.m_min.y
		const KyFloat32 operand4 = Fsel(pos.y - m_aabbox.m_max.y, 0.f, 1.f); // operand4 == 1.f if cellBox.m_max.y > pos.y

		return operand1 * operand2 * operand3 * operand4 > 0.f;
	}

	/*! Indicates whether or not this object has been successfully set up and is ready for use. */
	//TODO : maybe the function should be moved elsewhere...
	bool IsValid() const;


public:
	CellPos m_cellPos; /*! The position of this NavCell within the partitioning scheme of its NavMesh. Do not modify. */
	Box2f m_aabbox; /*! The axis-aligned bounding box computed during PathData generation (depends on the size of the cell and on #m_cellPos). Do not modify. */

	BlobArray<NavFloor> m_floors; /*! The list of \NavFloors maintained by the NavCell. Do not modify. */
	BlobArray<CellBoundaryEdge> m_cellBoundaryEdges[4]; /*! The list of boundary edges maintained by the NavCell. Do not modify. */

private:
	NavCell& operator=(const NavCell& navCell);
	NavCell(const NavCell& navCell);
};

/*! \label_swapendianness
	\ingroup kypathdata_navmesh */
inline void SwapEndianness(Endianness::Target e, NavCell& self)
{
	SwapEndianness(e, self.m_cellPos);
	SwapEndianness(e, self.m_aabbox);

	SwapEndianness(e, self.m_floors);

	SwapEndianness(e, self.m_cellBoundaryEdges[CardinalDir_EAST]);
	SwapEndianness(e, self.m_cellBoundaryEdges[CardinalDir_NORTH]);
	SwapEndianness(e, self.m_cellBoundaryEdges[CardinalDir_WEST]);
	SwapEndianness(e, self.m_cellBoundaryEdges[CardinalDir_SOUTH]);
}

}


#endif //KyPathData_NavMesh_H

