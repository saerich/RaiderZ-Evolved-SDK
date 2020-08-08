/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_BoundaryPolygon_H
#define KyPdg_BoundaryPolygon_H


#include "kypdg/boundary/boundarytypes.h"
#include "kypdg/boundary/boundaryedge.h"
#include "kypdg/common/stlvector.h"


namespace Kaim
{


class BoundaryOrderedSimplifyPolyline
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BoundaryOrderedSimplifyPolyline()
		: m_polyline(KY_NULL), m_order(Boundary::Order_Unset) {}

	BoundaryOrderedSimplifyPolyline(BoundarySimplifyPolyline* polyline, Boundary::Order order)
		: m_polyline(polyline), m_order(order) {}

public:
	BoundarySimplifyPolyline* m_polyline;
	Boundary::Order m_order;
};


class BoundaryContour
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	void Init(KyUInt32 index);

	void ComputeContourWinding();

public:
	KyUInt32 m_index;
	KyUInt32 m_edgeCount;
	BoundaryEdge* m_begin;
	Boundary::ContourWinding m_winding;
	Boundary::Color m_leftColor;
	StlVector<BoundaryOrderedSimplifyPolyline> m_orderedSimplifyPolylines;
};


class BoundaryPolygon
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BoundaryPolygon()
	{
		Init(KyUInt32MAXVAL, Boundary::Color_Unset);
	}

	void Init(KyUInt32 index, Boundary::Color leftColor)
	{
		m_index = index;
		m_leftColor = leftColor;
		m_exterior = KY_NULL;
	}

public:
	KyUInt32 m_index;
	Boundary::Color m_leftColor;
	BoundaryContour* m_exterior;
	StlVector<BoundaryContour*> m_holes;
};


}


#endif
