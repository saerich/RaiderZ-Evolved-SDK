/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_GeneratorProfiling_H
#define KyPdg_GeneratorProfiling_H


#include <kypdg/generator/cellgeneratorprofiling.h>
#include <kypathdata/containers/array.h>


namespace Kaim
{


class CellProfilingMaxMilliseconds
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	CellProfilingMaxMilliseconds() { Clear(); }

	void Clear()
	{
		m_maxMilliseconds = 0.0f;
		m_cellPos = CellPos();
	}

public:
	KyFloat32 m_maxMilliseconds;
	CellPos m_cellPos;
};


class ProfilingReport
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	ProfilingReport() { Clear(); }

	void Clear()
	{
		m_count = 0;
		m_totalMilliseconds = 0.0f;
		m_cellMaxMilliseconds.Clear();
	}

	void Integrate(KyFloat32 milliseconds, const CellPos& cellPos)
	{
		++m_count;
		m_totalMilliseconds += milliseconds;
		if (milliseconds > m_cellMaxMilliseconds.m_maxMilliseconds)
		{
			m_cellMaxMilliseconds.m_maxMilliseconds = milliseconds;
			m_cellMaxMilliseconds.m_cellPos = cellPos;
		}
	}

	KyFloat32 GetAverageSeconds()
	{
		if (m_count == 0)
			return 0.0f;
		return GetTotalSeconds() / m_count;
	}

	KyFloat32 GetTotalSeconds() const { return m_totalMilliseconds * 0.001f; }

public:
	KyUInt32 m_count;
	KyFloat32 m_totalMilliseconds;
	CellProfilingMaxMilliseconds m_cellMaxMilliseconds;
};


class GeneratorProfiling
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	void Clear()
	{
		m_cellProfilings.Clear();
		m_generateRasterReport.Clear();
		m_generateNavRasterReport.Clear();
		m_navRasterColoringReport.Clear();
		m_generateBoundariesReport.Clear();
		m_generateNavCellReport.Clear();
		m_generateGraphCellReport.Clear();
		m_generateAllReport.Clear();
	}

	void IntegrateCellProfiling(const CellGeneratorProfiling& cell)
	{
		m_cellProfilings.PushBack(cell);

		KyFloat32 generateMilliseconds =
			cell.m_raster.m_milliseconds +
			cell.m_navRaster.m_milliseconds +
			cell.m_boundaries.m_milliseconds +
			cell.m_navRasterColoring.m_milliseconds +
			cell.m_navCell.m_milliseconds +
			cell.m_pointgraphCell.m_milliseconds;

		m_generateRasterReport.Integrate     (cell.m_raster.m_milliseconds           , cell.m_cellPos);
		m_generateNavRasterReport.Integrate  (cell.m_navRaster.m_milliseconds        , cell.m_cellPos);
		m_navRasterColoringReport.Integrate  (cell.m_navRasterColoring.m_milliseconds, cell.m_cellPos);
		m_generateBoundariesReport.Integrate (cell.m_boundaries.m_milliseconds       , cell.m_cellPos);
		m_generateNavCellReport.Integrate    (cell.m_navCell.m_milliseconds          , cell.m_cellPos);
		m_generateGraphCellReport.Integrate  (cell.m_pointgraphCell.m_milliseconds   , cell.m_cellPos);
		m_generateAllReport.Integrate        (generateMilliseconds                   , cell.m_cellPos);
	}

	// useful total time shortcuts
	KyFloat32 GenerateRasterTime()     const { return m_generateRasterReport.GetTotalSeconds(); }
	KyFloat32 GenerateNavRasterTime()  const { return m_generateNavRasterReport.GetTotalSeconds(); }
	KyFloat32 ColorNavRasterTime()     const { return m_navRasterColoringReport.GetTotalSeconds(); }
	KyFloat32 GenerateBoundariesTime() const { return m_generateBoundariesReport.GetTotalSeconds(); }
	KyFloat32 GenerateNavCellTime()    const { return m_generateNavCellReport.GetTotalSeconds(); }
	KyFloat32 GenerateGraphCellTime()  const { return m_generateGraphCellReport.GetTotalSeconds(); }
	KyFloat32 GenerateTime()           const { return m_generateAllReport.GetTotalSeconds(); }

	// useful time ratio shortcuts in percentage
	KyInt32 GenerateRasterRatio()     const { return (KyInt32)(m_generateRasterReport.GetTotalSeconds()     * 100.0f / GenerateTime()); }
	KyInt32 GenerateNavRasterRatio()  const { return (KyInt32)(m_generateNavRasterReport.GetTotalSeconds()  * 100.0f / GenerateTime()); }
	KyInt32 ColorNavRasterRatio()     const { return (KyInt32)(m_navRasterColoringReport.GetTotalSeconds()  * 100.0f / GenerateTime()); }
	KyInt32 GenerateBoundariesRatio() const { return (KyInt32)(m_generateBoundariesReport.GetTotalSeconds() * 100.0f / GenerateTime()); }
	KyInt32 GenerateNavCellRatio()    const { return (KyInt32)(m_generateNavCellReport.GetTotalSeconds()    * 100.0f / GenerateTime()); }
	KyInt32 GenerateGraphCellRatio()  const { return (KyInt32)(m_generateGraphCellReport.GetTotalSeconds()  * 100.0f / GenerateTime()); }

	// useful max times shortcuts
	CellProfilingMaxMilliseconds GenerateRasterTime_Max()     const { return m_generateRasterReport.m_cellMaxMilliseconds; }
	CellProfilingMaxMilliseconds GenerateNavRasterTime_Max()  const { return m_generateNavRasterReport.m_cellMaxMilliseconds; }
	CellProfilingMaxMilliseconds GenerateBoundariesTime_Max() const { return m_generateBoundariesReport.m_cellMaxMilliseconds; }
	CellProfilingMaxMilliseconds GenerateTime_Max()           const { return m_generateAllReport.m_cellMaxMilliseconds; }

public:
	Array<CellGeneratorProfiling> m_cellProfilings;
	ProfilingReport m_generateRasterReport;
	ProfilingReport m_generateNavRasterReport;
	ProfilingReport m_navRasterColoringReport;
	ProfilingReport m_generateBoundariesReport;
	ProfilingReport m_generateNavCellReport;
	ProfilingReport m_generateGraphCellReport;
	ProfilingReport m_generateAllReport;
};


}


#endif

