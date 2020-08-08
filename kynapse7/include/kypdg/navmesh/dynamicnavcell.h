/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPdg_DynamicNavCell_H
#define KyPdg_DynamicNavCell_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include <kypathdata/generic/memory.h>
#include "kypdg/navmesh/dynamicnavfloor.h"
#include "kypdg/generator/pdgsystem.h"
#include "kypdg/generator/celldesc.h"
#include <kypdg/common/stlvector.h>

#include <assert.h>

namespace Kaim
{

struct DynamicCellBoundaryEdge
{
	KyUInt32 m_halfEdgeIdx;
	KyUInt32 m_floorIdx;
};

// --------------------------
// ----- DynamicNavCell -----
// --------------------------
class DynamicNavCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	DynamicNavCell(PdgSystem* sys, const CellDesc& cellDesc, bool isOnOverlap) : m_sys(sys), m_cellDesc(cellDesc), m_isOnOverlap(isOnOverlap) {}

public:
	KyResult WriteFiles();

	void Clear()
	{
		for (KyUInt32 i = 0; i < m_floors.size(); ++i)
			m_floors[i].Clear();

	}

	CardinalDir GetBoundaryDir(const NavVertex& start, const NavVertex& end) const
	{
		if (start.x == end.x)
		{
			if (start.x == m_cellDesc.m_box2f.m_min.x)
				return CardinalDir_WEST;
			else
			{
				assert(start.x == m_cellDesc.m_box2f.m_max.x);
				return CardinalDir_EAST;
			}
		}

		assert(start.y == end.y);

		if (start.y == m_cellDesc.m_box2f.m_min.y)
			return CardinalDir_SOUTH;
		else
		{
			assert(start.y == m_cellDesc.m_box2f.m_max.y);
			return CardinalDir_NORTH;
		}
	}

public:
	PdgSystem* m_sys;
	CellDesc m_cellDesc;

	StlVector<DynamicNavFloor> m_floors;
	StlVector<DynamicCellBoundaryEdge> m_cellBoundaryEdges[4];// one vector per Direction : CardinalDir_EST, CardinalDir_NORTH, CardinalDir_WEST, CardinalDir_SOUTH

	CellPos m_cellPos;

	bool m_isOnOverlap;
};

}


#endif //KyPdg_DynamicNavCell_H

