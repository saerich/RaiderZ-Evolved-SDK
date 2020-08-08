/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPdg_PdgInputTerrainTypeVolumeVisualRepresentation_H
#define KyPdg_PdgInputTerrainTypeVolumeVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"
#include "kypdg/pdginput/pdginputterraintypevolume.h"
#include "kypathdata/visualsystem/visualcolor.h"
#include "kypathdata/generic/helpers.h"

namespace Kaim
{

class PdgInputCell;
class ITerrainTypeColorPalette;


class PdgInputTerrainTypeVolumeVisualRepresentation : public VisualRepresentation
{
public:
	PdgInputTerrainTypeVolumeVisualRepresentation(PdgInputTerrainTypeVolume* data, ITerrainTypeColorPalette* palette = KY_NULL)
		: VisualRepresentation(data), m_palette(palette) {}

	virtual ~PdgInputTerrainTypeVolumeVisualRepresentation() {}

	/* Build the geometry of the volume depending on the arrow size and point size. */
	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	ITerrainTypeColorPalette* m_palette;
};


}

#endif
