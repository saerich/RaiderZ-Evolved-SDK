/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_DynamicNavCellBuilder_H
#define KyPdg_DynamicNavCellBuilder_H

#include "kypdg/generator/celldesc.h"
namespace Kaim
{

class PdgSystem;
class BoundaryGraph;
class BoundaryPolygon;
class DynamicNavCell;
class DynamicNavCellHeightField;
template <typename T> 
class Array;

class DynamicNavCellBuilder
{

public:
	DynamicNavCellBuilder(PdgSystem* sys, const CellDesc& celldesc) : m_sys(sys), m_cellDesc(celldesc) {}
	KyResult BuildNavCellFromBoundaryPolygons(const BoundaryGraph& inputGraph, DynamicNavCell& cell, Array<KyUInt32>* terrainTypeTable, DynamicNavCellHeightField* heightField);

private: 
	KyResult BuildNavFloorLinks(DynamicNavCell& cell);
	KyResult BuildNavCellLinks(DynamicNavCell& cell);

public:
	PdgSystem* m_sys;
	CellDesc m_cellDesc;
};

}


#endif //KyPdg_DynamicNavCellBuilder_H

