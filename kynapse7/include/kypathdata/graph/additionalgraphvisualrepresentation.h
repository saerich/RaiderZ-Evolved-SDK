/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// Primary contact: LASI - secondary contact: NOBODY
#ifndef KyPathData_AdditionalGraphVisualRepresentation_H
#define KyPathData_AdditionalGraphVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"


namespace Kaim
{

class AdditionalGraph;
class ITerrainTypeColorPalette;

/* This class of VisualRepresentation defines the way the data in a AdditionalGraph
	should be rendered using triangles. See :USERGUIDE:"Rendering PathData".
	\ingroup kypathdata_graph */
class AdditionalGraphVisualRepresentation : public VisualRepresentation
{
public:
	/* \label_constructor
		\param graph			The AdditionalGraph whose geometry will be built by this object. */
	explicit AdditionalGraphVisualRepresentation(const AdditionalGraph* graph, ITerrainTypeColorPalette* palette = KY_NULL)
		: VisualRepresentation(graph), m_palette(palette) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

private:
	void DrawEdge(VisualGeometryBuilder& geometryBuilder, bool bidirectional, const Vec3f& edgeStart, const Vec3f& edgeEnd, KyFloat32 arrowHalfWidth);
	void DrawVertex(VisualGeometryBuilder& geometryBuilder, const Vec3f& pos, KyFloat32 halfWidth);

public:
	ITerrainTypeColorPalette* m_palette;
};


}

#endif

