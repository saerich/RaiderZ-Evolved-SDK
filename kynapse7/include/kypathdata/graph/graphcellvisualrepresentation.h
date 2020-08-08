/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphCellVisualRepresentation_H
#define KyPathData_GraphCellVisualRepresentation_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/visualsystem/visualrepresentation.h"
#include "kypathdata/visualsystem/visualcolor.h"
#include "kypathdata/generic/helpers.h"

namespace Kaim
{

class GraphCell;
class ITerrainTypeColorPalette;

/* This class of VisualRepresentation defines the way the data in a GraphCell
	should be rendered using triangles. See :USERGUIDE:"Rendering PathData".
	\ingroup kypathdata_graph */
class GraphCellVisualRepresentation : public VisualRepresentation
{
public:
	/*! \label_constructor
		\param graph			The GraphCell whose geometry will be built by this object. */
	GraphCellVisualRepresentation(const GraphCell* graph, ITerrainTypeColorPalette* palette = KY_NULL)
		: VisualRepresentation(graph), m_palette(palette) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	ITerrainTypeColorPalette* m_palette;

protected:
	void DrawEdge(VisualGeometryBuilder& geometryBuilder, bool bidirectional, const Vec3f& edgeStart, const Vec3f& edgeEnd, KyFloat32 arrowHalfWidth);
	void DrawVertex(VisualGeometryBuilder& geometryBuilder, const Vec3f& pos, KyFloat32 halfWidth);
};


}

#endif

