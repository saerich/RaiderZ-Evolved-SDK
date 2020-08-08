/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PdgInputCellVisualRepresentation_H
#define KyPathData_PdgInputCellVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"
#include "kypdg/pdginput/pdginputcell.h"

namespace Kaim
{

class PdgInputCell;
class ITerrainTypeColorPalette;

/* Implements the visual representation of an input cell */
class PdgInputCellVisualRepresentation : public VisualRepresentation
{
public:
	PdgInputCellVisualRepresentation(PdgInputCell* inputCell, ITerrainTypeColorPalette* palette = KY_NULL) : VisualRepresentation(inputCell), m_palette(palette), m_integerPrecision(0.0f), m_inv_integerPrecision(0.0f) {}

	virtual ~PdgInputCellVisualRepresentation() {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	ITerrainTypeColorPalette* m_palette;
	KyFloat32 m_integerPrecision;
	KyFloat32 m_inv_integerPrecision;
};


}

#endif // KyPathData_PdgInputCellVisualRepresentation_H
