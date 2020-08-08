/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_BoundarySimplifiedPolygon_H
#define KyPdg_BoundarySimplifiedPolygon_H


#include "kypdg/boundary/boundarytypes.h"
#include "kypdg/boundary/boundaryvertex.h"
#include "kypdg/common/stlvector.h"


namespace Kaim
{

class BoundarySimplifiedContour;
class BoundarySimplifiedPolygon;


class BoundarySimplifiedEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BoundarySimplifiedEdge()
	{
		m_index = KyUInt32MAXVAL;
		m_edgeType = KY_NULL;
		m_firstEdge = KY_NULL;
		m_lastEdge = KY_NULL;
		m_vertex[0] = KY_NULL;
		m_vertex[1] = KY_NULL;
		m_leftColor = Boundary::Color_Unset;
		m_contour = KY_NULL;
		m_pair = KY_NULL;
		m_next = KY_NULL;
	}

	void Init(KyUInt32 index, BoundaryEdge* firstEdge, BoundaryEdge* lastEdge, BoundarySimplifiedContour* contour)
	{
		m_index = index;
		m_edgeType = firstEdge->m_type;
		m_firstEdge = firstEdge;
		m_lastEdge = lastEdge;
		m_vertex[0] = firstEdge->m_vertex[0];
		m_vertex[1] = lastEdge->m_vertex[1];
		m_leftColor = firstEdge->m_leftColor;
		m_contour = contour;
		m_pair = KY_NULL;
		m_next = KY_NULL;
	}

public:
	KyUInt32 m_index;
	Boundary::EdgeType m_edgeType;
	BoundaryEdge* m_firstEdge;
	BoundaryEdge* m_lastEdge;
	BoundaryVertex* m_vertex[2];
	Boundary::Color m_leftColor;
	BoundarySimplifiedContour* m_contour;
	BoundarySimplifiedEdge* m_pair;
	BoundarySimplifiedEdge* m_next;
};


class BoundarySimplifiedContour
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BoundarySimplifiedContour()
	{
		m_index = KyUInt32MAXVAL;
		m_edgeCount = 0;
		m_begin = KY_NULL;
		m_last = KY_NULL;
		m_winding = Boundary::ContourWinding_Unset;
		m_leftColor = Boundary::Color_Unset;
		m_polygon = KY_NULL;
	}

	void Init(KyUInt32 index, Boundary::Color leftColor, BoundarySimplifiedPolygon* polygon)
	{
		m_index = index;
		m_edgeCount = 0;
		m_begin = KY_NULL;
		m_last = KY_NULL;
		m_winding = Boundary::ContourWinding_Unset;
		m_leftColor = leftColor;
		m_polygon = polygon;
	}

	void PushEdge(BoundarySimplifiedEdge* edge)
	{
		if (m_edgeCount == 0)
		{
			m_begin = edge;
			m_last = edge;
			m_edgeCount = 1;
			return;
		}

		m_leftColor = edge->m_leftColor;
		m_last->m_next = edge;
		m_last = edge;
		++m_edgeCount;
	}

public:
	KyUInt32 m_index;
	KyUInt32 m_edgeCount;
	BoundarySimplifiedEdge* m_begin;
	BoundarySimplifiedEdge* m_last;
	Boundary::ContourWinding m_winding;
	Boundary::Color m_leftColor;
	BoundarySimplifiedPolygon* m_polygon;
};


class BoundarySimplifiedPolygon
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	void Init(KyUInt32 index, Boundary::Color leftColor)
	{
		m_index = index;
		m_leftColor = leftColor;
		m_exterior = KY_NULL;
	}

public:
	KyUInt32 m_index;
	Boundary::Color m_leftColor;
	BoundarySimplifiedContour* m_exterior;
	StlVector<BoundarySimplifiedContour*> m_holes;
};


}


#endif
