/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_BoundarySimplifier_H
#define KyPdg_BoundarySimplifier_H

#include <kypathdata/generic/types.h>
#include <kypdg/common/stllist.h>
#include <kypdg/boundary/boundarygraph.h>

namespace Kaim
{


class BoundarySimplifier
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BoundarySimplifier(BoundaryGraph* boundaryGraph, KyFloat32 horizontalTolerance, KyFloat32 verticalTolerance = -1.0f) : 
		m_boundaryGraph(boundaryGraph), m_horizontalTolerance(horizontalTolerance), m_verticalTolerance(verticalTolerance), m_linkHorizontalTolerance(1.5f)
	{}

	//Smooth all the polylines of the input BoundaryGraph with a certain tolerance
	//Tolerance measures the maximum error allowed between a vertex of BoundaryGraph
	//and the corresponding simplified output polyline
	//Units of tolerance depends on Boundary vertex coordinates system. 
	//Tolerance can be interpreted in terms of "percentage" of pixelSize
	//for example, a tolerance of 0.5f means we accept an error of 0.5f * pixelSize
	//Vertical tolerance defines the maximum error along the up Axis. Its in meters and 
	//the value is ignored if it is negative.
	KyResult SimplifyBoundaries();

private:
	//real stuff: called on each polyline of input graph
	KyResult SimplifyPolyline(BoundarySimplifyPolyline& polyline);
	
	//based on Ramer Douglas Peucker approach
	//see: http://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm
	enum Clockwise { CCW, CW };
	KyResult SmoothRDP(BoundarySimplifyPolyline& polyline);

	//recursive algo. Max depth = log(polyline.size())
	void RecursiveRDP(
		BoundarySimplifyPolyline& polyline,
		StlList<BoundarySimplifyVertex>::iterator begin,
		StlList<BoundarySimplifyVertex>::iterator last);

private:
	BoundaryGraph* m_boundaryGraph;
	KyFloat32 m_horizontalTolerance;
	KyFloat32 m_verticalTolerance;
	KyFloat32 m_linkHorizontalTolerance;
};

}


#endif //KyPdg_BoundarySimplifier_H

