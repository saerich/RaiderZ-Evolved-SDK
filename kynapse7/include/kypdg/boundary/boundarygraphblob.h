/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_BoundaryGraphBlob_H
#define KyPdg_BoundaryGraphBlob_H


#include "kypathdata/blob/endianness.h"
#include <kypathdata/basesystem/coordsystem.h>
#include "kypathdata/blob/blobarray.h"
#include "kypdg/generator/squarepartition.h"
#include "kypdg/generator/celldesc.h"
#include "kypdg/boundary/boundarytypes.h"
#include "kypathdata/containers/stringstream.h"


namespace Kaim
{


class BoundaryGraphBlobVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BoundaryPos m_boundaryPos; // in world (not local)
	KyFloat32 m_altitude;
	KyUInt32 m_ins[4];
	KyUInt32 m_outs[4];
	Boundary::VertexStaticStatus m_staticStatus;
	KyUInt32 m_index;
};
inline void SwapEndianness(Endianness::Target e, BoundaryGraphBlobVertex& self)
{
	SwapEndianness(e, self.m_boundaryPos);
	SwapEndianness(e, self.m_altitude);

	for (KyUInt32 i = 0; i < 4; ++i)
		SwapEndianness(e, self.m_ins[i]);

	for (KyUInt32 i = 0; i < 4; ++i)
		SwapEndianness(e, self.m_outs[i]);

	SwapEndianness(e, self.m_staticStatus);
	SwapEndianness(e, self.m_index);
}


class BoundaryGraphBlobSimplifyPolyline
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	KyUInt32 m_cycle;
	BlobArray<KyUInt32> m_vertices;
};
inline void SwapEndianness(Endianness::Target e, BoundaryGraphBlobSimplifyPolyline& self)
{
	SwapEndianness(e, self.m_cycle);
	SwapEndianness(e, self.m_vertices);
}


class BoundaryGraphBlobEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	Boundary::CardinalDir m_dir;
	Boundary::EdgeType m_type;
	KyUInt32 m_vertex[2];
	KyUInt32 m_next;
	KyUInt32 m_pair;
	KyUInt32 m_index;
};
inline void SwapEndianness(Endianness::Target e, BoundaryGraphBlobEdge& self)
{
	SwapEndianness(e, self.m_dir);
	SwapEndianness(e, self.m_type);
	SwapEndianness(e, self.m_vertex[0]);
	SwapEndianness(e, self.m_vertex[1]);
	SwapEndianness(e, self.m_next);
	SwapEndianness(e, self.m_pair);
	SwapEndianness(e, self.m_index);
}


class BoundaryGraphBlobContour
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	KyUInt32 m_edgeCount;
	KyUInt32 m_begin;
	Boundary::ContourWinding m_type;
	Boundary::Color m_leftColor;
	KyUInt32 m_index;
};
inline void SwapEndianness(Endianness::Target e, BoundaryGraphBlobContour& self)
{
	SwapEndianness(e, self.m_edgeCount);
	SwapEndianness(e, self.m_begin);
	SwapEndianness(e, self.m_type);
	SwapEndianness(e, self.m_leftColor);
	SwapEndianness(e, self.m_index);
}


class BoundaryGraphBlobPolygon
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	Boundary::Color m_leftColor;
	KyUInt32 m_exterior;
	BlobArray<KyUInt32> m_holes;
};
inline void SwapEndianness(Endianness::Target e, BoundaryGraphBlobPolygon& self)
{
	SwapEndianness(e, self.m_leftColor);
	SwapEndianness(e, self.m_exterior);
	SwapEndianness(e, self.m_holes);
}


class BoundaryGraphBlobSimplifiedHalfEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	Boundary::EdgeType m_type;
	KyUInt32 m_vertex[2];
	KyUInt32 m_next;
	KyUInt32 m_pair;
	KyUInt32 m_index;
};
inline void SwapEndianness(Endianness::Target e, BoundaryGraphBlobSimplifiedHalfEdge& self)
{
	SwapEndianness(e, self.m_type);
	SwapEndianness(e, self.m_vertex[0]);
	SwapEndianness(e, self.m_vertex[1]);
	SwapEndianness(e, self.m_next);
	SwapEndianness(e, self.m_pair);
	SwapEndianness(e, self.m_index);
}


class BoundaryGraphBlobSimplifiedContour
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	KyUInt32 m_edgeCount;
	KyUInt32 m_begin;
	Boundary::ContourWinding m_winding;
	Boundary::Color m_leftColor;
	KyUInt32 m_index;
};
inline void SwapEndianness(Endianness::Target e, BoundaryGraphBlobSimplifiedContour& self)
{
	SwapEndianness(e, self.m_edgeCount);
	SwapEndianness(e, self.m_begin);
	SwapEndianness(e, self.m_winding);
	SwapEndianness(e, self.m_leftColor);
	SwapEndianness(e, self.m_index);
}


class BoundaryGraphBlobSimplifiedPolygon
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	Boundary::Color m_leftColor;
	KyUInt32 m_exterior;
	BlobArray<KyUInt32> m_holes;
};
inline void SwapEndianness(Endianness::Target e, BoundaryGraphBlobSimplifiedPolygon& self)
{
	SwapEndianness(e, self.m_leftColor);
	SwapEndianness(e, self.m_exterior);
	SwapEndianness(e, self.m_holes);
}


class BoundaryGraphBlob
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	static const char* GetBlobType() { return "BoundaryGraph"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	typedef BoundaryGraphBlobVertex           Vertex;
	typedef BoundaryGraphBlobEdge             Edge;
	typedef BoundaryGraphBlobContour          Contour;
	typedef BoundaryGraphBlobPolygon          Polygon;
	typedef BoundaryGraphBlobSimplifyPolyline SimplifyPolyline;

	typedef BoundaryGraphBlobSimplifiedHalfEdge SimplifiedHalfEdge;
	typedef BoundaryGraphBlobSimplifiedContour  SimplifiedContour;
	typedef BoundaryGraphBlobSimplifiedPolygon  SimplifiedPolygon;

public:
	CoordSystem m_coordSystem;
	SquarePartition m_partition;
	CellDesc m_cellDesc;
	BlobArray<Vertex> m_vertices;
	BlobArray<Edge> m_edges;
	BlobArray<Contour> m_contours;
	BlobArray<Polygon> m_polygons;
	BlobArray<SimplifyPolyline> m_simplifyPolylines;
	BlobArray<SimplifiedHalfEdge> m_simplifiedEdges;
	BlobArray<SimplifiedContour> m_simplifiedContours;
	BlobArray<SimplifiedPolygon> m_simplifiedPolygons;
};
inline void SwapEndianness(Endianness::Target e, BoundaryGraphBlob& self)
{
	SwapEndianness(e, self.m_coordSystem);
	SwapEndianness(e, self.m_partition);
	SwapEndianness(e, self.m_cellDesc);
	SwapEndianness(e, self.m_vertices);
	SwapEndianness(e, self.m_edges);
	SwapEndianness(e, self.m_contours);
	SwapEndianness(e, self.m_polygons);
	SwapEndianness(e, self.m_simplifyPolylines);
	SwapEndianness(e, self.m_simplifiedEdges);
	SwapEndianness(e, self.m_simplifiedContours);
	SwapEndianness(e, self.m_simplifiedPolygons);
}


}


#endif

