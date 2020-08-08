/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavCellStitcher_H
#define KyPathData_NavCellStitcher_H

#include "kypathdata/navmesh/navmeshtypes.h"

namespace Kaim
{

class StitchedNavCell;
class StitchedNavMesh;
class NavCellGrid;

class NavCellStitcher
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	NavCellStitcher() : m_altitudeTolerance(0.5f), m_navCellGrid(KY_NULL) {}

	void StitchNavCell(StitchedNavCell& newStitchedCell);
	void UnStitchNavCell(StitchedNavCell& stitchedCell);

protected :
	bool AreEdgesNeighbour(const NavVertex& start_A, const NavVertex& end_A, const NavVertex& start_B, const NavVertex& end_B, const CardinalDir dir);

public:
	KyFloat32 m_altitudeTolerance;
	NavCellGrid* m_navCellGrid;
};


}

#endif //KyPathData_NavCellStitcher_H

