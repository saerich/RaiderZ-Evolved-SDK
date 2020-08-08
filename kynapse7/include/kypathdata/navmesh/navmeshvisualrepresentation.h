/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// ---------- Primary contact: JUBA - secondary contact: MAMU
#ifndef KyPathData_NavMeshVisualRepresentation_H
#define KyPathData_NavMeshVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"


namespace Kaim
{

class NavMesh;
class ITerrainTypeColorPalette;

/* This class of VisualRepresentation defines the way the data in a NavMesh
	should be rendered using triangles. See :USERGUIDE:"Rendering PathData".
	\pp A NavCellVisualRepresentation is used to build the geometry for each NavCell
	in the NavMesh, and the geometries of all \NavCells are combined to build the geometry for the
	NavMesh.  
	\ingroup kypathdata_navmesh */
class NavMeshVisualRepresentation : public VisualRepresentation
{
public:
	/* \label_constructor
		\param navMesh			The NavMesh whose geometry will be built by this object. */
	NavMeshVisualRepresentation(const NavMesh* navMesh, ITerrainTypeColorPalette* palette = KY_NULL) : VisualRepresentation(navMesh), m_palette(palette) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	ITerrainTypeColorPalette* m_palette;
};


}


#endif

