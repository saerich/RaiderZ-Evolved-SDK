/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_mesh */

// primary contact: GUAL - secondary contact: LAPA
#ifndef KY_POLYGONMERGER_POLYGON_H
#define KY_POLYGONMERGER_POLYGON_H

#include <kypathdata/generic/basetypes.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{

typedef KyUInt32 VertexIdx;
typedef KyUInt32 EdgeIdx;
typedef KyUInt32 ContourIdx;
typedef KyUInt32 ProxyIdx;
typedef KyUInt32 InputIdx;
typedef KyUInt32 IntersectionIdx;

static const KyUInt32 VertexIdxNone = KyUInt32MAXVAL;
static const KyUInt32 EdgeIdxNone = KyUInt32MAXVAL;
static const KyUInt32 ContourIdxNone = KyUInt32MAXVAL;
static const KyUInt32 ProxyIdxNone = KyUInt32MAXVAL;
static const KyUInt32 InputIdxNone = KyUInt32MAXVAL;
static const KyUInt32 IntersectionIdxNone = KyUInt32MAXVAL;


class PolygonMerger_Polygon_Vertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	KyFloat32 m_x; // right
	KyFloat32 m_y; // -front
	KyFloat32 m_alt; // alt
	VertexIdx m_next; // next in output contour
};


class PolygonMerger_Polygon_Contour
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	VertexIdx m_vertex_left;
	VertexIdx m_vertex_right;
	KyBool m_hole;
};


class PolygonMerger_Polygon_Proxy
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	ContourIdx m_contour;
};


class PolygonMerger_Polygon_Edge;
class PolygonMerger_Polygon_Intersection;


/*!	This class merges together a set of input polylines into a set of output contours by testing for intersections between the
	polygons represented by the input polylines.
	\pp You should not need to interact with the PolygonMerger_Polygon class directly. This class is typically used only indirectly 
	through the LpfPreMerger_Polygon class, which the LpfManager can use to merge obstacle outlines into \LpfPreAggregates. It is 
	also used internally by the LpfManager to merge \LpfPreAggregate outlines into \LpfAreas. 
	\ingroup kyruntime_mesh */
class PolygonMerger_Polygon
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	enum MergeResult
	{
		MERGE_OK = 0,
		TOPOLOGICAL_ERROR = 1,
		NB_VERTS_MAX_REACHED = 2,
		NB_CONTOURS_MAX_REACHED = 3,
		NB_INTERSECTIONS_MAX_REACHED = 4,

		KY_DUMMY_MergeResult = KYFORCEENUMSIZEINT
	};

public:
	static const KyUInt8 LEFT = 0;
	static const KyUInt8 RIGHT = 1;

	static const KyUInt8 COLINEAR_NOT = 0;
	static const KyUInt8 COLINEAR_MAIN = 1;
	static const KyUInt8 COLINEAR_PHANTOM = 2;

public:
	PolygonMerger_Polygon();

	virtual ~PolygonMerger_Polygon();

	void ClearAll();

	// will init the merger with:
	// nbVertsMax = nbVertsInOriginalPolygons * 4
	// nbContoursMax = nbVertsInOriginalPolygons * 4
	// nbIntersectionsMax = nbVertsInOriginalPolygons * 4
	void Init(KyUInt32 nbVertsInOriginalPolygons);

	void InitWithDetails(KyUInt32 nbVertsMax, KyUInt32 nbContoursMax, KyUInt32 nbIntersectionsMax);

	void ClearBeforeAddInputs();

	KyBool Merge();

public:
	// Setup input polygons
	KyBool AddInputContourVertex(KyFloat32 x, KyFloat32 y, KyFloat32 alt);
	void FinalizeInputContour();
	void FinalizeInputPolygon();

public:
	void AddEdge(VertexIdx bottom, VertexIdx top, KyBool isBottomMinimum, KyBool isTopMaximum);
	void CreateInputContourEdges();

	KyBool NextScanBeam();
	void AddLocalMinimaToAet();
	void AddEdgeToAet(EdgeIdx edgeIdx, EdgeIdx prevEdgeIdx);
	void CalculateAboveStates();
	KyBool ProcessScanBeamBottom();
	void PrepareIntersections();
	KyBool CalculateIntersections();
	KyBool ProcessIntersection(const PolygonMerger_Polygon_Intersection& intersection);
	void FinalizeScanBeam();

	KyBool AddLocalMin(EdgeIdx edgeIdx, KyFloat32 x, KyFloat32 y, KyFloat32 alt, ProxyIdx& proxyIdx);
	void AppendRight(ContourIdx appendIdx, ContourIdx mainIdx);
	void AppendLeft(ContourIdx appendIdx, ContourIdx mainIdx);

protected:
	KyUInt32 m_nbInputPolygons;

	// vertices
	KyUInt32 m_nbInputVerts;
public:
	KyUInt32 m_nbVertsMax;
	KyUInt32 m_nbVerts;
	PolygonMerger_Polygon_Vertex* m_verts;
protected:
	// output contours (nbContoursMax = m_nbVertsMax by default)
	// in the worse case: m_nbContoursMax = (m_nbVertsMax*m_nbVertsMax + m_nbVertsMax) / 2
	// yet we prefer to set a more reasonnable m_nbContoursMax and to test this limit
	KyUInt32 m_nbContoursMax;
public:
	KyUInt32 m_nbContours;
	PolygonMerger_Polygon_Contour* m_contours;
	MergeResult m_lastMergeResult;

	// if (m_contribs[0] == KY_TRUE) means that input_polygon_0
	// contributes to at least one contour of the output polygon
	KyBool m_contribs[2];

protected:
	// (nbEdgesMax = m_nbVertsMax)
	KyUInt32 m_nbEdges;
public:
	PolygonMerger_Polygon_Edge* m_edges;
	// output contour proxies (nbProxiesMax = m_nbContoursMax)
	// nbProxies = m_nbContours;
	PolygonMerger_Polygon_Proxy* m_proxies;
protected:
	// (nbLocalMinimaMax = m_nbVertsMax / 2)
	KyUInt32 m_nbLocalMinima;
	EdgeIdx* m_localMinima;

	// scanBeams (nbScanBeamsMax = m_nbVertsMax)
	KyFloat32* m_scanBeams;

	// sortedEdges (nbSortedEdgesMax = nbEdgesMax = m_nbVertsMax)
	KyUInt32 m_nbSortedEdges;
	EdgeIdx* m_sortedEdges;

	// intersections (m_nbIntersectionsMax = m_nbVertsMax by default)
	// the nbIntersectionsMax should be m_nbVertsMax*m_nbVertsMax in the worse case
	// yet we prefer to set a more reasonnable m_nbIntersectionsMax and to test this limit
	KyInt32 m_nbIntersectionsMax;
	KyInt32 m_nbIntersections;
	PolygonMerger_Polygon_Intersection* m_intersections;

	// sorted intersection indices (nbSortedIntersectionIndicesMax = nbIntersectionsMax = m_nbVertsMax)
	IntersectionIdx* m_sortedIntersectionIndices;

	// used when adding input polygon/contour/vertices
	ContourIdx m_nbInputContoursInCurrentPolygon;
	VertexIdx m_inputVertexIdx;

	// currently processed...
	KyUInt32 m_scanBeamIdx;
	KyUInt32 m_localMinimaIdx;

	KyFloat32 m_top_y;
	KyFloat32 m_bottom_y;
	KyFloat32 m_delta_y;

	EdgeIdx m_aet_first;

	// parity (nbMax = m_nbVertsMax / 3)
	KyUInt8 m_aboveParities[2];
	KyUInt8 m_horizSwitches[2];
};

}

#endif // KY_POLYGONMERGER_POLYGON_H
