/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPdg_DynamicNavMesh_H
#define KyPdg_DynamicNavMesh_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypdg/navmesh/dynamicnavcell.h"
#include "kypdg/generator/pdgsystem.h"
#include <kypdg/common/stlvector.h>


namespace Kaim
{


// --------------------------
// ----- DynamicNavMesh -----
// --------------------------
class DynamicNavMesh
{
public:
	DynamicNavMesh(PdgSystem* sys = KY_NULL) : m_sys(sys) {}

	void Clear()
	{
		for (KyUInt32 i = 0; i < m_cells.size(); ++i)
			m_cells[i].Clear();

		m_cellBox.Clear();
		m_aabbox.Clear();
	}

public:
	PdgSystem* m_sys;

	StlVector<DynamicNavCell> m_cells;
	Box3f m_aabbox;
	CellBox m_cellBox;

	KyFloat32 m_cellSize;
	Vec2f m_originOfParition;
};

}


#endif //KyPdg_DynamicNavMesh_H

