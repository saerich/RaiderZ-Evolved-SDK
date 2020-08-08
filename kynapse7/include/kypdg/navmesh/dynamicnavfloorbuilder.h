/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




// Primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_DynamicNavFloorBuilder_H
#define KyPdg_DynamicNavFloorBuilder_H

#include "kypdg/generator/pdgsystem.h"
#include "kypdg/common/performancetickcount.h"
#include "kypdg/boundary/boundaryvertex.h"
#include "kypdg/generator/celldesc.h"

#include <kypdg/common/stlvector.h>
#include <kypdg/common/stllist.h>



namespace Kaim
{

class IndexedTriangleSoup;
class DynamicNavFloor;
class BoundaryPolygon;
class BoundaryGraph;
class BoundarySimplifiedPolygon;
class BoundarySimplifiedContour;
class DynamicNavFloorHeightField;
class DynamicTriangulation;


class DynamicNavFloorBuilder
{
public:
	typedef StlList<BoundaryVertex> BoundaryVerticeList;
	typedef StlList<BoundaryVerticeList> BoundaryVerticeListCollection;
protected:
	struct IndexedPoint
	{
		Vec2i point;
		KyUInt32 index;
		bool operator == (const IndexedPoint& p) const
		{
			return point == p.point && index == p.index;
		}
	};
	typedef StlList<IndexedPoint> IndexedPolygon2D;

	struct EarCompElem
	{
		IndexedPolygon2D::iterator earIter;
		IndexedPolygon2D::iterator previousIter;
		IndexedPolygon2D::iterator nextIter;

		KyFloat32 earMinAngle;

		bool operator < (const EarCompElem& other) const { return earMinAngle < other.earMinAngle; }
	};

	struct FloorLinkEdge
	{
		BoundaryVertex v1; // SHOULD BE POINTER !
		BoundaryVertex v2; // SHOULD BE POINTER !
		KyUInt32 pairFloorIdx;
		Vec3f v1Pos;
		Vec3f v2Pos;

		FloorLinkEdge(const BoundaryVertex& v1_, const BoundaryVertex& v2_, KyUInt32 pairFloorIdx_) :
			v1(v1_), v2(v2_), pairFloorIdx(pairFloorIdx_) {}

		bool operator == (const FloorLinkEdge& other) const { return v1 == other.v1 && v2 == other.v2; }
	};
	typedef StlList<FloorLinkEdge> FloorLinkList;

	struct CellLinkEdge
	{
		BoundaryVertex v1; // SHOULD BE POINTER !
		BoundaryVertex v2; // SHOULD BE POINTER !
		KyUInt32 cardinalDir;
		Vec3f v1Pos;
		Vec3f v2Pos;

		CellLinkEdge(const BoundaryVertex& v1_, const BoundaryVertex& v2_, KyUInt32 cardinalDir_) :
			v1(v1_), v2(v2_), cardinalDir(cardinalDir_) {}

		bool operator == (const FloorLinkEdge& other) const { return v1 == other.v1 && v2 == other.v2; }
	};
	typedef StlList<CellLinkEdge> CellLinkList;

public:
	DynamicNavFloorBuilder(PdgSystem* sys, const CellDesc& cellDesc) : m_sys(sys), m_cellDesc(cellDesc), m_profilingEnabled(false) {}
	void ConsumeContour(const BoundarySimplifiedContour& contour, FloorLinkList& floorLinkEdges, CellLinkList& cellLinkEdges, BoundaryVerticeList& vertices);
	KyResult BuildNavFloorFromBoundaryPolygons(const BoundaryGraph& inputGraph, const BoundarySimplifiedPolygon& polygon, DynamicNavFloorHeightField* heightField, DynamicNavFloor& floor);
	KyResult BuildNavFloorFromIndexedTriangleSoup(const IndexedTriangleSoup& soup, DynamicNavFloor& floor, FloorLinkList& floorLinkEdges, CellLinkList& cellLinkEdges, bool mightHaveHoles = true);
	void ToggleProfiling(bool activate) {m_profilingEnabled = activate;}
	KyFloat32 GetLastBuildDuration() { return (m_profilingEnabled) ? PerformanceTickCount::Seconds(m_startTime, m_endTime) : 0; }

private:
	KyResult AbsorbHoles(BoundaryVerticeList& border, BoundaryVerticeListCollection& holes);
	KyResult Triangulate(IndexedTriangleSoup& result, const BoundaryVerticeList& inputPolygon);
	bool IsEar(const IndexedPolygon2D& workingPolygon,const IndexedPolygon2D::iterator& currentIter);

	void ReOrderEdges(DynamicNavFloor& navFloor);
	void RemoveDuplicatePointsFromSoup(const IndexedTriangleSoup& imputSoup, IndexedTriangleSoup& output);

	void RemoveDegeneratedTrianglesFromSoup(const IndexedTriangleSoup& imputSoup, IndexedTriangleSoup& output);

	KyUInt32 DiagnoseInputPolygon(const BoundaryVerticeList& inputPolygon);
	KyUInt32 DiagnoseTriangleSoup(const IndexedTriangleSoup& soup);

	KyResult MakeVerticallyAccurateTriangulation(DynamicTriangulation& triangulation, DynamicNavFloorHeightField* heightField);

public:
	PdgSystem* m_sys;
	CellDesc m_cellDesc;
	PerformanceTickCount m_startTime;
	PerformanceTickCount m_endTime;
	bool m_profilingEnabled;
};

} // namespace Kaim


#endif //KyPdg_DynamicNavFloorBuilder_H

