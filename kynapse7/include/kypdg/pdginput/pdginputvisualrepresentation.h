/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyPathData_PdgInputVisualRepresentation_H
#define KyPathData_PdgInputVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"


namespace Kaim
{


class PdgInput;
class ITerrainTypeColorPalette;


class PdgInputVisualRepresentation : public VisualRepresentation
{
public:
	PdgInputVisualRepresentation(PdgInput* pdgInput, ITerrainTypeColorPalette* palette = KY_NULL)
		: VisualRepresentation(pdgInput), m_palette(palette) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	ITerrainTypeColorPalette* m_palette;
};


}

#endif // KyPathData_PdgInputVisualRepresentation_H
