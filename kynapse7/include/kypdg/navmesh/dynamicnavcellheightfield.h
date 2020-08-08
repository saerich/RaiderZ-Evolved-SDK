/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// Primary contact: LASI - secondary contact: NONE
#ifndef KyPdg_DynamicHeightFieldCell_H
#define KyPdg_DynamicHeightFieldCell_H

#include <kypdg/navmesh/dynamicnavfloorheightfield.h>
#include <kypdg/common/stlvector.h>

namespace Kaim
{

class NavRasterCell;
class PdgSystem;
class DynamicNavRasterCell;
class DynamicNavCell;

// For debug purpose
class DynamicPointGraphCell;

// -----------------------------------
// ----- DynamicHeightFieldCell -----
// -----------------------------------
class DynamicNavCellHeightField
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	DynamicNavCellHeightField(PdgSystem* sys) : m_sys(sys) {}
	~DynamicNavCellHeightField();

	KyResult ExtractHeightFieldFromDynamicNavRasterCell(const DynamicNavRasterCell* navRasterCell, KyUInt32 stride);
	// Can return KY_NULL if navFloor was too small to be sampled and has no heightFieldAssociated
	DynamicNavFloorHeightField* GetNavFloorHeightFieldFromColor(KyUInt32 color)
	{
		if (color >= (KyUInt32) m_heightfieldFloors.size() || m_heightfieldFloors[color] == KY_NULL)
		{
			return KY_NULL;
		}
		return m_heightfieldFloors[color];
	}

	// For debug purpose
	KyResult BuildDebugGraph(DynamicPointGraphCell& debugGraph);

public: 

	PdgSystem* m_sys;
	StlVector<DynamicNavFloorHeightField*> m_heightfieldFloors;
};

}


#endif //KyPdg_DynamicNavFloor_H

