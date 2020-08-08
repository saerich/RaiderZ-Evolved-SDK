/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: JODA - secondary contact: NOBODY
#ifndef KyPathData_GraphStat_H
#define KyPathData_GraphStat_H


#include "kypathdata/generic/memory.h"

namespace Kaim
{
class Graph;
class GraphCell;
class GraphStat;

class GraphCellStat
{
	KY_DEFINE_NEW_DELETE_OPERATORS
	friend class GraphStat;
public:
	
	explicit GraphCellStat(const GraphCell& graphcell);

	KyFloat32 GetTotalEdgeLength3d() const { return m_totalEdgeLength3d; }
	KyFloat32 GetAverageEdgeLength3d() const { return m_nbEdges ? m_totalEdgeLength3d / m_nbEdges : 0.0f; }
	KyFloat32 GetMaxEdgeLength3d() const { return m_maxLength3d; }
	KyFloat32 GetTotalEdgeLength2d() const { return m_totalEdgeLength2d; }
	KyFloat32 GetAverageEdgeLength2d() const { return m_nbEdges ? m_totalEdgeLength2d / m_nbEdges : 0.0f; }
	KyFloat32 GetMaxEdgeLength2d() const { return m_maxLength2d; }
	KyUInt32 GetNumberOfVertices() const { return m_nbVertices; }
	KyUInt32 GetNumberOfEdges() const { return m_nbEdges; }
private:
	GraphCellStat();
	const GraphCellStat& operator += (const GraphCellStat&);
private:
	KyFloat32 m_totalEdgeLength3d;
	KyFloat32 m_maxLength3d;
	KyFloat32 m_totalEdgeLength2d;
	KyFloat32 m_maxLength2d;
	KyUInt32 m_nbVertices;
	KyUInt32 m_nbEdges;
};

class GraphStat
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	explicit GraphStat(const Graph& graph);

	KyFloat32 GetTotalEdgeLength3d() const { return m_cellsStat.GetTotalEdgeLength3d(); }
	KyFloat32 GetAverageEdgeLength3d() const { return m_cellsStat.GetAverageEdgeLength3d(); }
	KyFloat32 GetMaxEdgeLength3d() const { return m_cellsStat.GetMaxEdgeLength3d(); }
	KyFloat32 GetTotalEdgeLength2d() const { return m_cellsStat.GetTotalEdgeLength2d(); }
	KyFloat32 GetAverageEdgeLength2d() const { return m_cellsStat.GetAverageEdgeLength2d(); }
	KyFloat32 GetMaxEdgeLength2d() const { return m_cellsStat.GetMaxEdgeLength2d(); }
	KyUInt32 GetNumberOfVertices() const { return m_cellsStat.GetNumberOfVertices(); }
	KyUInt32 GetNumberOfEdges() const { return m_cellsStat.GetNumberOfEdges(); }
	KyUInt32 GetNumberofCells() const { return m_nbCells; }
private:
	GraphCellStat m_cellsStat;
	KyUInt32 m_nbCells;
};



} // namespace Kaim

#endif // KyPathData_GraphStat_H
