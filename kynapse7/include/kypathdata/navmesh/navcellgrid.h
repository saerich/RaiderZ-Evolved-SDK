/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavcellGrid_H
#define KyPathData_NavcellGrid_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navcellstitcher.h"
#include "kypathdata/containers/array.h"
#include "kypathdata/containers/list.h"
#include "kypathdata/generic/kyguid.h"
#include "kypathdata/generic/guidcompound.h"
#include "kypathdata/navmesh/navcellgridcallbacks.h"

namespace Kaim
{

class NavCell;
class StitchedNavCell;
class StitchedNavMesh;
class NavMeshStitchDataManager;
class ActiveGuids;


class StitchedNavCellVersions
{
public:

	bool operator ==(const StitchedNavCellVersions& stitchedNavCellVersions)
	{
		return &m_sharedPoolListOfStitchedNavCell == &stitchedNavCellVersions.m_sharedPoolListOfStitchedNavCell;
	}

	SharedPoolList<StitchedNavCell* > m_sharedPoolListOfStitchedNavCell;
};

class NavCellPosInfo
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	NavCellPosInfo() : m_activeStitchedNavCell(KY_NULL), m_stitchedNavCellVersions(KY_NULL) {}
	void Clear()
	{
		m_activeStitchedNavCell = KY_NULL;
		m_stitchedNavCellVersions = KY_NULL;
	}

	StitchedNavCell* m_activeStitchedNavCell;
	StitchedNavCellVersions* m_stitchedNavCellVersions;
};

class NavCellGrid
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	NavCellGrid();
	~NavCellGrid();

	void Clear();

	bool IsCellAvailiable(const CellPos& pos) const;
	const NavCell* GetNavCell(const CellPos& pos) const;

	KY_FORCE_INLINE StitchedNavCell* GetStitchedCell(const CellPos& pos) const
	{
		KY_FUNCTION("NavCellGrid::GetStitchedCell");
		KY_ASSERTK(m_sizeOfBuffer != 0);

		return m_cellBox.IsInside(pos) ? m_bufferOfNavCellPosInfo[m_cellBox.GetRowMajorIndex(pos)].m_activeStitchedNavCell : KY_NULL;
	}

	void EnlargeGrid(const CellBox& cellBox);

	// This function insert all the navCell of the stitchedNavMesh in the grid.
	// If one cell must be activated, the current active one at the same CellPos is Unstitched and the new is considered as "active"
	// Still, it is not stitched yet, you have to call function UpdateCellStitchingInBox for stitching active cells together
	void InsertNavMesh(StitchedNavMesh& stitchedNavMesh);

	// This function remove all the navCell of the stitchedNavMesh from the grid.
	// If one cell was active, it is Unstitched and, we check wether a cell of an other navMesh at the same CellPos must be activated
	// Still, it is not stitched yet, you have to call function UpdateCellStitchingInBox for stitching active cells together
	void RemoveNavMesh(StitchedNavMesh& stitchedNavMesh);


	void UpdateCellStitchingInBox(const CellBox& cellBox);

	KyResult ResetGrid(const NavMeshStitchDataManager& stitchDataManager);

protected:
	KyResult InsertNavCell(StitchedNavCell& stitchedNavCell, bool doesNewStitchedNavCellCanBeActivated);
	KyResult RemoveNavCell(StitchedNavCell& stitchedNavCell);

	void UnStitchedCellFromTheGrid(NavCellPosInfo& navCellPosInfo);
	void StitchedCellInTheGrid(NavCellPosInfo& navCellPosInfo);

	void ComputeRulers(const CellBox& cellBox);

private:

	friend class NavMeshManager;
	// This function is called by the NavMeshManager to be sure computed CellPos that has been computed from float is the true one
	KY_FORCE_INLINE CellPos ComputeNavCellPos(const Vec2f& pos, const KyInt32 computedPosX, const KyInt32 computedPosY) const
	{
		return CellPos(GetCorrectCellPosOnX(pos.x, computedPosX), GetCorrectCellPosOnY(pos.y, computedPosY));
	}

	// This function is called by the NavMeshManager to be sure computed CellPos that has been computed from float is the true one
	KY_FORCE_INLINE void ComputeNavCellPos(const Vec2f& pos, const KyInt32 computedPosX, const KyInt32 computedPosY, CellPos& cellPos) const
	{
		return cellPos.Set(GetCorrectCellPosOnX(pos.x, computedPosX), GetCorrectCellPosOnY(pos.y, computedPosY));
	}

	KY_FORCE_INLINE KyInt32 GetCorrectCellPosOnX(const KyFloat32 posX, const KyInt32 computedPosX) const;

	KY_FORCE_INLINE KyInt32 GetCorrectCellPosOnY(const KyFloat32 posY, const KyInt32 computedPosY) const;

public:
	NavCellGrid& operator=(const NavCellGrid& navCellGrid);
	bool operator==(const NavCellGrid& navCellGrid) const;
	bool operator==(const NavCellGrid& navCellGrid);
	NavCellGrid(const NavCellGrid& navCellGrid);

	NavCellPosInfo* m_bufferOfNavCellPosInfo;
	KyUInt32 m_sizeOfBuffer;
	NavCellStitcher m_stitcher;

	CellBox m_cellBox;

	KyFloat32* m_rulerX;
	KyFloat32* m_rulerY;

	List<StitchedNavCellVersions > m_listOfStitchedNavCellVersions;
	SharedPoolList<StitchedNavCell*>::Pool m_poolForSharedPoolListOfStitchedNavCell;

	NavCellGridCallBacks m_callBacks;
public:
	ActiveGuids* m_activatedGuids;
};

KY_FORCE_INLINE KyInt32 NavCellGrid::GetCorrectCellPosOnX(const KyFloat32 posX, const KyInt32 computedPosX) const
{
	if (computedPosX >= m_cellBox.Min().x && computedPosX <= m_cellBox.Max().x)
	{
		const KyFloat32* const& cellBoxValuesX = m_rulerX + (computedPosX - m_cellBox.Min().x);

		const KyFloat32& cellBoxMinX = cellBoxValuesX[0];
		const KyFloat32& cellBoxMaxX = cellBoxValuesX[1];

		const KyFloat32 iscellBoxMinXInit = Fsel(cellBoxMinX - KyFloat32MAXVAL, 0.f, 1.f); // iscellBoxMinXInit == 1.f if cellBoxMinX < KyFloat32MAXVAL
		const KyFloat32 iscellBoxMaxXInit = Fsel(cellBoxMaxX - KyFloat32MAXVAL, 0.f, 1.f); // iscellBoxMaxXInit == 1.f if cellBoxMaxX < KyFloat32MAXVAL

		const KyFloat32 isLowerThanXmin = Fsel(cellBoxMinX - posX, 1.f, 0.f); // isLowerThanXmin == 1.f if pos.x <= cellBox.m_min.x
		const KyFloat32 isStrictlyGreaterThanXmax = Fsel(cellBoxMaxX - posX, 0.f, 1.f); // isStrictlyGreaterThanXmax == 1.f if cellBox.m_max.x > pos.x

		const KyInt32 errorOnX = (iscellBoxMinXInit * isLowerThanXmin > 0.f) ? -1 : (iscellBoxMaxXInit * isStrictlyGreaterThanXmax > 0.f) ? 1 : 0;
		return computedPosX + errorOnX;
	}

	if (computedPosX == m_cellBox.Min().x - 1 && m_rulerX[0] != KyFloat32MAXVAL && posX > m_rulerX[0])
		return computedPosX + 1;

	if (computedPosX == m_cellBox.Max().x + 1 && m_rulerX[m_cellBox.CountX()] != KyFloat32MAXVAL && posX <= m_rulerX[m_cellBox.CountX()])
		return computedPosX - 1;

	return computedPosX;
}

KY_FORCE_INLINE KyInt32 NavCellGrid::GetCorrectCellPosOnY(const KyFloat32 posY, const KyInt32 computedPosY) const
{
	if (computedPosY >= m_cellBox.Min().y && computedPosY <= m_cellBox.Max().y)
	{
		const KyFloat32* const& cellBoxValuesY = m_rulerY + (computedPosY - m_cellBox.Min().y);

		const KyFloat32& cellBoxMinY = cellBoxValuesY[0];
		const KyFloat32& cellBoxMaxY = cellBoxValuesY[1];

		const KyFloat32 iscellBoxMinYInit = Fsel(cellBoxMinY - KyFloat32MAXVAL, 0.f, 1.f); // iscellBoxMinYInit == 1.f if cellBoxMaxY < KyFloat32MAXVAL
		const KyFloat32 iscellBoxMaxYInit = Fsel(cellBoxMaxY - KyFloat32MAXVAL, 0.f, 1.f); // iscellBoxMaxYInit == 1.f if cellBoxMaxY < KyFloat32MAXVAL

		const KyFloat32 isStrictlyLowerThanYmin = Fsel(posY - cellBoxMinY, 0.f, 1.f); // isStrictlyLowerThanYmin == 1.f if pos.y < cellBox.m_min.y
		const KyFloat32 isGreaterThanYmax = Fsel(posY - cellBoxMaxY, 1.f, 0.f); // isStrictlyLowerThanYmax == 1.f if cellBox.m_max.y <= pos.y

		const KyInt32 errorOnY = (iscellBoxMinYInit * isStrictlyLowerThanYmin > 0.f) ? -1 : (iscellBoxMaxYInit * isGreaterThanYmax > 0.f) ? 1 : 0;
		return computedPosY + errorOnY;
	}

	if (computedPosY == m_cellBox.Min().y - 1 && m_rulerY[0] != KyFloat32MAXVAL && posY >= m_rulerY[0])
		return computedPosY + 1;

	if (computedPosY == m_cellBox.Max().y + 1 && m_rulerY[m_cellBox.CountY()] != KyFloat32MAXVAL && posY < m_rulerY[m_cellBox.CountY()])
		return computedPosY - 1;

	return computedPosY;
}

}

#endif //KyPathData_NavcellGrid_H

