/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_GlobalCellGrid_H
#define KyPdg_GlobalCellGrid_H


#include <kypdg/pdginput/dynamicgrid.h>
#include <kypdg/pdginput/pdginputcell.h>
#include <kypdg/pdginput/pdginputcellinfo.h>
#include <kypathdata/containers/array.h>

#include <algorithm>


namespace Kaim
{

class PdgSystem;


class PdgInputCellsAtPos
{
	KY_DEFINE_NEW_DELETE_OPERATORS

private:
	struct Comp
	{
		bool operator()(const PdgInputCellInfo& a, const PdgInputCellInfo& b) const
		{
			if (a.m_sectorGenerator->m_doGeneratePathData == b.m_sectorGenerator->m_doGeneratePathData)
				return a.m_inputCell->m_sectorGuid < b.m_inputCell->m_sectorGuid;
			else
				return a.m_sectorGenerator->m_doGeneratePathData;
		}
	};

public:
	PdgInputCellsAtPos(KyInt32 x, KyInt32 y) : m_cellPos(x, y) {}

	void AddInputCellInfo(const PdgInputCellInfo& inputCellInfo) { m_inputCellInfos.PushBack(inputCellInfo); }

	// m_inputCells from sectors with m_doGenerateOverlapPathData==true are at the beginning of the m_inputCellInfos
	void SortInputCellInfos()
	{
		std::sort(m_inputCellInfos.Begin(), m_inputCellInfos.End(), Comp());
	}

public:
	CellPos m_cellPos;
	Array<PdgInputCellInfo> m_inputCellInfos;
};


class GlobalCellGrid
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	GlobalCellGrid(PdgSystem* sys) : m_sys(sys), m_grid() {}

	void AddInputCellInfo(const PdgInputCellInfo& inputCellInfo)
	{
		PdgInputCell* inputCell = inputCellInfo.m_inputCell;
		PdgInputCellsAtPos* cellsAtPos = m_grid.GetOrCreateElement(inputCell->m_cellPos.x, inputCell->m_cellPos.y);
		cellsAtPos->AddInputCellInfo(inputCellInfo);
	}

	PdgInputCellsAtPos* GetCellsAtPos(CellCoord x, CellCoord y) { return m_grid.GetElement(x, y); }

	PdgInputCellsAtPos** GetAllCellsAtPos() { return m_grid.GetElements(); }

	KyUInt32 GetCellsAtPosCount() { return m_grid.GetElementsCount(); }

public:
	PdgSystem* m_sys;
	DynamicGrid<PdgInputCellsAtPos> m_grid;
};


}


#endif

