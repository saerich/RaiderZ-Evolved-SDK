/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphVisualRepresentation_H
#define KyPathData_GraphVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"


namespace Kaim
{

class Graph;
class ITerrainTypeColorPalette;

/* This class of VisualRepresentation defines the way the data in a GraphCell
	should be rendered using triangles. See :USERGUIDE:"Rendering PathData".
	\pp A GraphCellVisualRepresentation is used to build the geometry for each GraphCell
	in the Graph, and the geometries of all \GraphCells are combined to build the geometry for the
	Graph.  
	\ingroup kypathdata_graph */
class GraphVisualRepresentation : public VisualRepresentation
{
public:
	/* \label_constructor
		\param graph			The Graph whose geometry will be built by this object. */
	GraphVisualRepresentation(const Graph* graph, ITerrainTypeColorPalette* palette = KY_NULL)
		: VisualRepresentation(graph), m_palette(palette) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	ITerrainTypeColorPalette* m_palette;
};


}

#endif

