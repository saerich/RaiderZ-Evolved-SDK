/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


//  Primary contact: GUAL - secondary contact: JUBA
#ifndef KyPathData_NavMeshManagerVisualRepresentation_H
#define KyPathData_NavMeshManagerVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"


namespace Kaim
{

class NavMeshManager;
class ITerrainTypeColorPalette;


/* This class of VisualRepresentation defines the way the data that has been streamed into a NavMeshManager
	should be rendered using triangles. See :USERGUIDE:"Rendering PathData".
	\pp A NavMeshVisualRepresentation is used to build the geometry for each NavMesh that has been added
	to the NavMeshManager, and the geometries of all \NavMeshes are combined.  
	\ingroup kypathdata_navmesh */
class NavMeshManagerVisualRepresentation : public VisualRepresentation
{
public:
	/*! \label_constructor
		\param navMeshManager			The NavMeshManager whose geometry will be built by this object. */
	NavMeshManagerVisualRepresentation(const NavMeshManager* navMeshManager = KY_NULL, ITerrainTypeColorPalette* palette = KY_NULL)
		: VisualRepresentation(navMeshManager), m_palette(palette) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	ITerrainTypeColorPalette* m_palette;
};


}


#endif
