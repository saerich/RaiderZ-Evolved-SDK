/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_BoundaryGraphBlobVisualRepresentation_H
#define KyPdg_BoundaryGraphBlobVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"
#include "kypathdata/math/vec3f.h"
#include "kypdg/boundary/boundarytypes.h"


namespace Kaim
{

class VisualGeometryBuilder;
class BoundaryGraphBlob;
class BoundaryGraphBlobVertex;
class BoundaryGraphBlobEdge;
class BoundaryGraphBlobContour;
class BoundaryGraphBlobPolygon;
class BoundaryGraphBlobSimplifyPolyline;
class BoundaryGraphBlobSimplifiedHalfEdge;
class BoundaryGraphBlobSimplifiedContour;
class BoundaryGraphBlobSimplifiedPolygon;
class VisualColor;


class BoundaryGraphBlobVisualRepresentation : public VisualRepresentation
{
public:
	typedef BoundaryGraphBlobVertex            VertexBlob;
	typedef BoundaryGraphBlobEdge              EdgeBlob;
	typedef BoundaryGraphBlobContour           ContourBlob;
	typedef BoundaryGraphBlobPolygon           PolygonBlob;
	typedef BoundaryGraphBlobSimplifyPolyline  SimplifyPolylineBlob;
	typedef BoundaryGraphBlobSimplifiedHalfEdge SimplifiedEdgeBlob;
	typedef BoundaryGraphBlobSimplifiedContour SimplifiedContourBlob;
	typedef BoundaryGraphBlobSimplifiedPolygon SimplifiedPolygonBlob;

public:
	enum EdgeColorScheme
	{
		FromEdgeType,
		FromContourWinding,
		FromPolygonIdx,
		FromPolygonColor
	};

	class EdgeVisualInfo
	{
	public:
		EdgeVisualInfo() :
			m_polygonColor(Boundary::Color_Unset), m_contourWinding(Boundary::ContourWinding_Unset), m_polygonIdx(0) {}

	public:
		Boundary::Color m_polygonColor;
		Boundary::ContourWinding m_contourWinding;
		KyUInt32 m_polygonIdx;
	};

public:
	BoundaryGraphBlobVisualRepresentation(BoundaryGraphBlob* boundaryGraphBlob);

	void SetEdgeColorScheme(EdgeColorScheme edgeColorScheme);

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

private:
	Vec3f GetVertexPos(const VertexBlob& vertex);

	void BuildStaticVerticesGeometry(VisualGeometryBuilder& geometryBuilder);
	void BuildNarrowVerticesGeometry(VisualGeometryBuilder& geometryBuilder);

	void BuildEdgeGeometry(VisualGeometryBuilder& geometryBuilder, const EdgeBlob& edge, EdgeVisualInfo& edgeVisualInfo);
	void BuildContourGeometry(VisualGeometryBuilder& geometryBuilder, const ContourBlob& contour, EdgeVisualInfo& edgeVisualInfo);
	void BuildPolygonsGeometry(VisualGeometryBuilder& geometryBuilder);

	void BuildSimplifiedEdgeGeometry(VisualGeometryBuilder& geometryBuilder, const SimplifiedEdgeBlob& simplifiedEdge, EdgeVisualInfo& edgeVisualInfo);
	void BuildSimplifiedContourGeometry(VisualGeometryBuilder& geometryBuilder, const SimplifiedContourBlob& contour, EdgeVisualInfo& edgeVisualInfo);
	void BuildSimplifiedPolygonsGeometry(VisualGeometryBuilder& geometryBuilder);

	void BuildSimplifyPolylinesGeometry(VisualGeometryBuilder& geometryBuilder);
	void BuildSimplifySegmentGeometry(VisualGeometryBuilder& geometryBuilder, const VertexBlob& start, const VertexBlob& end,  KyUInt32 polylineIdx);

	VisualColor GetEdgeColor(const EdgeVisualInfo& edgeVisualInfo, const Boundary::EdgeType& edgeType);

private:
	EdgeColorScheme m_edgeColorScheme;
	const BoundaryGraphBlob* m_graph;
	const VertexBlob* m_vertices;
	const EdgeBlob* m_edges;
	const ContourBlob* m_contours;
	const PolygonBlob* m_polygons;
	const SimplifyPolylineBlob* m_simplifyPolylines;
	const SimplifiedEdgeBlob* m_simplifiedEdges;
	const SimplifiedContourBlob* m_simplifiedContours;
	const SimplifiedPolygonBlob* m_simplifiedPolygons;
};


}


#endif
