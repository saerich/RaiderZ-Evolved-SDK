/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPdg_PdgInputTerrainTypeVolumesVisualRepresentation_H
#define KyPdg_PdgInputTerrainTypeVolumesVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"
#include "kypdg/pdginput/pdginputterraintypevolume.h"
#include "kypathdata/visualsystem/visualcolor.h"
#include "kypathdata/generic/helpers.h"

namespace Kaim
{

class PdgInput;

/*
Implements the visual representation of all terrain type volumes in PdgInputs.
*/
class PdgInputTerrainTypeVolumesVisualRepresentation : public VisualRepresentation
{
public:
	PdgInputTerrainTypeVolumesVisualRepresentation(PdgInput* pdgInput) : VisualRepresentation(pdgInput) {}

	/* Build the geometry of the volume depending on the arrow size and point size. */
	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);
};


}

#endif // #KyPdg_PdgInputTerrainTypeVolumesVisualRepresentation_H
