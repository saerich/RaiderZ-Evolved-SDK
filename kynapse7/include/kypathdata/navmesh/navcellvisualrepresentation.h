/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavCellVisualRepresentation_H
#define KyPathData_NavCellVisualRepresentation_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/visualsystem/visualrepresentation.h"
#include "kypathdata/generic/helpers.h"


namespace Kaim
{

class NavCell;
class ITerrainTypeColorPalette;

/* This class of VisualRepresentation defines the way a NavCell should be rendered using triangles.
	See :USERGUIDE:"Rendering PathData".
	\ingroup kypathdata_navmesh */
class NavCellVisualRepresentation : public VisualRepresentation
{
public:
	/*! \label_constructor
		\param navCell			The NavCell whose geometry will be built by this object. */
	NavCellVisualRepresentation(const NavCell* navCell, ITerrainTypeColorPalette* palette = KY_NULL)
		: VisualRepresentation(navCell), m_palette(palette) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	ITerrainTypeColorPalette* m_palette;
};


}

#endif // #KyPathData_NavCellVisualRepresentation_H
