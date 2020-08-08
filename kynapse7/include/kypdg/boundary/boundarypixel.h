/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_BoundaryPixel_H
#define KyPdg_BoundaryPixel_H


#include "kypdg/boundary/boundarytypes.h"
#include "kypdg/navraster/navrastercell.h"


namespace Kaim
{

class BoundaryEdge;
typedef NavRasterColumnElement NavRasterPixel;
typedef NavRasterCornerColumnElement NavRasterCornerPixel;
typedef NavRasterColumnElement::ElementColor FloorColor;


/* There is one BoundaryPixel for each NavRasterPixel. */
class BoundaryPixel
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BoundaryPixel()
	{
		m_navRasterPixel = KY_NULL;
		m_navRasterCornerPixel = KY_NULL;
		m_color = Boundary::Color_Unset;
		m_edges[0] = KY_NULL;
		m_edges[1] = KY_NULL;
		m_edges[2] = KY_NULL;
		m_edges[3] = KY_NULL;
		m_edges[4] = KY_NULL;
		m_edges[5] = KY_NULL;
		m_edges[6] = KY_NULL;
		m_edges[7] = KY_NULL;
		m_isOutside = false;
	}

	void Init(NavPixelCoord x, NavPixelCoord y, KyInt32 z, const NavRasterPixel* navRasterPixel, bool isOutside, const NavRasterCornerPixel* navRasterCornerPixel = KY_NULL)
	{
		m_navPixelPos3d.Set(x, y, z);
		m_navRasterPixel = navRasterPixel;
		m_navRasterCornerPixel = navRasterCornerPixel;
		m_color = navRasterPixel->m_color;
		m_isOutside = isOutside;
	}

public:
	NavPixelPos3d m_navPixelPos3d;
	const NavRasterPixel* m_navRasterPixel;
	const NavRasterCornerPixel* m_navRasterCornerPixel; // null most of the time
	Boundary::Color m_color;
	bool m_isOutside;

	// +--------2=N------+ 
	// | \		       / |
	// |   \	     /   |
	// |    3=NW   1=NE  |
	// 4=W     \ /	     0=E
	// |       / \	     |
	// |     /     \     |
	// |   5=SW     7=SE |
	// | /             \ | 
	// +--------6--------+
	BoundaryEdge* m_edges[8];
};


}


#endif
