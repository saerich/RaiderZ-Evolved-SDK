/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphCellStitcher_H
#define KyPathData_GraphCellStitcher_H

#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/math/cardinaldir.h"

namespace Kaim
{

class StitchedGraphCell;
class StitchedGraph;
class GraphCellGrid;
class Vec3f;

/* For internal use. */
class GraphCellStitcher
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	GraphCellStitcher() : m_altitudeTolerance(0.1f), m_graphCellGrid(KY_NULL) {}

	void StitchGraphCell(StitchedGraphCell& newStitchedCell);
	void UnStitchGraphCell(StitchedGraphCell& stitchedCell);

protected :
	bool AreVerticesNeighbour(const Vec3f& vertex1, const Vec3f& vertex2, CardinalDir dir);

public:
	KyFloat32 m_altitudeTolerance;
	GraphCellGrid* m_graphCellGrid;
};


}

#endif //KyPathData_GraphCellStitcher_H

