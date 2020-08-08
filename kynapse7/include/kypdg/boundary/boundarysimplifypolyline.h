/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_BoundarySimplifyPolyline_H
#define KyPdg_BoundarySimplifyPolyline_H


#include "kypdg/boundary/boundaryedge.h"
#include "kypdg/boundary/boundaryvertex.h"
#include "kypdg/boundary/boundarypolygon.h"
#include <kypdg/common/stllist.h>


namespace Kaim
{


class BoundarySimplifyVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BoundarySimplifyVertex() : m_vertex(KY_NULL), m_inSmallEdge(KY_NULL), m_outSmallEdge(KY_NULL) {}

	BoundarySimplifyVertex(BoundaryEdge* inSmallEdge, BoundaryVertex* vertex, BoundaryEdge* outSmallEdge)
	{
		m_pos.x = (KyFloat32)vertex->m_exclBoundaryPos.x;
		m_pos.y = (KyFloat32)vertex->m_exclBoundaryPos.y;
		m_pos.z = vertex->m_altitude;
		m_vertex = vertex;
		m_inSmallEdge = inSmallEdge;
		m_outSmallEdge = outSmallEdge;
	}

public:
	Vec3f m_pos;
	BoundaryVertex* m_vertex;
	BoundaryEdge* m_inSmallEdge;
	BoundaryEdge* m_outSmallEdge;
};


// Linked list of BoundaryVertices for simplification usage
class BoundarySimplifyPolyline
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	typedef BoundarySimplifyVertex Vertex;

public:
	BoundarySimplifyPolyline()
	{
		Init(KyUInt32MAXVAL, Boundary::ContourWinding_Unset, Boundary::EdgeType_Unset, Boundary::PolylineCycle_Unset);
	}

	void Init(KyUInt32 index, Boundary::ContourWinding contourWinding, Boundary::EdgeType edgeType, Boundary::PolylineCycle cycle)
	{
		m_index = index;
		m_contourWinding = contourWinding;
		m_edgeType = edgeType;
		m_vertices.clear();
		m_cycle = cycle;
		m_horizontalTolerance = 0.01f;
		m_verticalTolerance = -1.0f;
		m_next = KY_NULL;
	}

	void AddVertex(BoundaryEdge* inEdge, BoundaryVertex* vertex, BoundaryEdge* outEdge)
	{
		BoundarySimplifyVertex simplifyVertex(inEdge, vertex, outEdge);
		m_vertices.push_back(simplifyVertex);

		if (inEdge)
			PutEdgeInPolyline(inEdge);

		if (outEdge)
			PutEdgeInPolyline(outEdge);
	}

	void PutEdgeInPolyline(BoundaryEdge* edge)
	{
		edge->m_simplifyPolyline = this;
		edge->m_simplifyPolylineOrder = Boundary::StraightOrder;

		if (edge->m_pair)
		{
			edge->m_pair->m_simplifyPolyline = this;
			edge->m_pair->m_simplifyPolylineOrder = Boundary::ReverseOrder;
		}
	}

public:
	KyUInt32 m_index;
	StlList<BoundarySimplifyVertex> m_vertices;
	Boundary::ContourWinding m_contourWinding;
	Boundary::EdgeType m_edgeType;
	Boundary::PolylineCycle m_cycle; // if (m_cycle == Polyline_Cycle) then m_vertices.first != vertices.last
	KyFloat32 m_horizontalTolerance;
	KyFloat32 m_verticalTolerance;
	BoundarySimplifyPolyline* m_next;
};


}


#endif
