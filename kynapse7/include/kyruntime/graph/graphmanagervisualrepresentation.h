/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


//  Primary contact: GUAL - secondary contact: JUBA
#ifndef KyPathData_GraphManagerVisualRepresentation_H
#define KyPathData_GraphManagerVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"


namespace Kaim
{

class GraphManager;
class ITerrainTypeColorPalette;

class GraphManagerVisualRepresentation : public VisualRepresentation
{
public:
	GraphManagerVisualRepresentation(const GraphManager* graphManager = KY_NULL, ITerrainTypeColorPalette* palette = KY_NULL)
		: VisualRepresentation(graphManager), m_palette(palette) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	ITerrainTypeColorPalette* m_palette;
};


}


#endif
