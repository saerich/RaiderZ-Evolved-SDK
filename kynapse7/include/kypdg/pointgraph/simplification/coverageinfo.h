/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* This computer source code and related instructions and comments are the
* unpublished confidential and proprietary information of Autodesk, Inc. and
* are protected under applicable copyright and trade secret law. They may
* not be disclosed to, copied or used by any third party without
* the prior written consent of Autodesk, Inc.
*/

// primary contact: JODA - secondary contact: NOBODY

#ifndef KyPdg_CoverageInfo_H
#define KyPdg_CoverageInfo_H

#include "kypdg/pointgraph/simplification/obstaclesinfo.h"

#include "kypathdata/math/vec2f.h"
#include "kypathdata/containers/array.h"

namespace Kaim
{

class DynamicNavFloor;
class PdgPointGraph;
class PdgGraphVertex;

class EdgeToCover
{
public:
	EdgeToCover() : m_halfEdge(KY_NULL), m_visiblePointCount(0) {} // To be compatible with Array
	EdgeToCover(const Vec2f& v1, const Vec2f& v2, const DynamicNavHalfEdge* halfEdge) :
		m_edge(v1, v2), m_halfEdge(halfEdge), m_visiblePointCount(0)
	{}

	const Vec2f& GetV1() const { return m_edge.GetV1(); }
	const Vec2f& GetV2() const { return m_edge.GetV2(); }
	const DynamicNavHalfEdge* GetHalfEdge() const { return m_halfEdge; }

	bool IsCovered() const { return 0 < m_visiblePointCount; }
	bool IsLastVisible() const { return m_visiblePointCount == 1; }
	void IncreaseVisible(PdgGraphVertex& /*vertex*/) { ++m_visiblePointCount; }
	void DecreaseVisible(PdgGraphVertex& /*vertex*/) { /* assert(m_visiblePointCount); */ --m_visiblePointCount; }
private:
	Edge2d m_edge;
	const DynamicNavHalfEdge* m_halfEdge;
	KyUInt32 m_visiblePointCount;
};

class CoverageInfo
{
public:

	CoverageInfo() : m_maxCoverageDistance(KyFloat32MAXVAL) {}

	void InitCoverage(PdgPointGraph& graph, const DynamicNavFloor& navFloor);
	bool IsLastCoverer(const PdgGraphVertex& vertex) const;
	void RemoveCoverage(PdgGraphVertex& vertex);

	void SetCoverageDistance(KyFloat32 dist) { m_maxCoverageDistance = dist; }
	KyFloat32 GetCoverageDistance() const { return m_maxCoverageDistance; }
	bool CheckValidity() const;
private:
	void InitSegment(const DynamicNavFloor& navFloor);
	void Compute(const DynamicNavFloor& navFloor, PdgPointGraph& graph);
	void ComputeForSingleVertex(PdgGraphVertex& vertex);
private:
	typedef Array<EdgeToCover> EdgesToCover;
private:
	EdgesToCover m_coverageEdges;
	KyFloat32 m_maxCoverageDistance;
};


} // namespace Kaim

#endif // KyPdg_CoverageInfo_H

