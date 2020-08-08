/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: JODA - secondary contact: NOBODY
#ifndef KyPdg_CollapseInfo_H
#define KyPdg_CollapseInfo_H

#include "kypdg/pointgraph/simplification/dynamicpointgraphedgecollapse.h"
#include "kypdg/pointgraph/simplification/obstaclesinfo.h"
#include "kypdg/pointgraph/simplification/coverageinfo.h"
#include "kypathdata/containers/chunkbasedpool.h"

namespace Kaim
{

class DynamicNavCell;
class DynamicNavFloor;
class PdgPointGraph;
class PdgSystem;

class CollapseInfo
{
public:
	CollapseInfo() :
		m_maxEdgeLength(KyFloat32MAXVAL),
		m_coverageInfo(KY_NULL),
		m_navFloor(KY_NULL)
	{}

	~CollapseInfo();

	void Init(const PdgSystem& sys, PdgPointGraph& graph, const DynamicNavFloor& navFloor, CoverageInfo& coverageInfo)
	{
		InitConfig(sys, navFloor);
		m_obstacles.Init(navFloor);
		m_coverageInfo = &coverageInfo;
		InitPriorityQueue(graph, navFloor);
	}

	void PreCollapseEdge(PdgGraphEdge& edge);
	void PostCollapseEdge(const Array<PdgGraphEdge*>& toRemove, const Array<PdgGraphEdge*>& toUpdate);

	PriorityQueueEdge& GetPriorityQueueEdge() { return m_priorityQueueEdge; }

	PdgGraphEdge* RemoveFirstCollapseEdge();

	KyFloat32 GetMaxGraphEdgeLength() const { return m_maxEdgeLength; }
	const DynamicNavFloor& GetNavFloor() const { return *m_navFloor; }
	const ObstaclesInfo& GetObstacles() const { return m_obstacles; }
	const CoverageInfo& GetCoverageInfo() const { return *m_coverageInfo; }
private:
	void InitConfig(const PdgSystem& sys, const DynamicNavFloor& navFloor);
	void InitPriorityQueue(PdgPointGraph& graph, const DynamicNavFloor& navFloor);
	void InitObstacles(const DynamicNavFloor& navFloor);
	CoverageInfo& GetCoverageInfo() { return *m_coverageInfo; }
	void RemoveEdges(const Array<PdgGraphEdge*>& toRemove);
	void UpdateEdges(const Array<PdgGraphEdge*>& toUpdate);
protected:
	KyFloat32 m_maxEdgeLength;
	PriorityQueueEdge m_priorityQueueEdge;
	ObstaclesInfo m_obstacles;
	CoverageInfo* m_coverageInfo;
	const DynamicNavFloor* m_navFloor;
	ChunkBasedPool<PdgGraphEdgeCollapse> m_pool;
};

} // namespace Kaim


#endif // KyPdg_CollapseInfo_H
