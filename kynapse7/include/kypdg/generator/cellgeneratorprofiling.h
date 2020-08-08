/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_CellGeneratorProfiling_H
#define KyPdg_CellGeneratorProfiling_H



#include "kypdg/common/performancetickcount.h"
#include "kypathdata/navmesh/navmeshtypes.h"


namespace Kaim
{


/*
*/
class CellStepProfiling
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	CellStepProfiling() : m_milliseconds(0.0f) {}

	void Increment(PerformanceTickCount t0, PerformanceTickCount t1)
	{
		m_milliseconds += PerformanceTickCount::Milliseconds(t0, t1);
	};

public:
	KyFloat32 m_milliseconds;
};


/*
*/
class CellGeneratorProfiling
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	CellPos m_cellPos;
	CellStepProfiling m_raster;
	CellStepProfiling m_navRaster;
	CellStepProfiling m_navRasterColoring;
	CellStepProfiling m_boundaries;
	CellStepProfiling m_navCell;
	CellStepProfiling m_pointgraphCell;
};


}


#endif

