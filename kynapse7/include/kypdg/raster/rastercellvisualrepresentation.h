/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_RasterCellVisualRepresentation_H
#define KyPdg_RasterCellVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"


namespace Kaim
{

class VisualGeometryBuilder;
class RasterCell;


class RasterCellVisualRepresentation : public VisualRepresentation
{
public:
	RasterCellVisualRepresentation(RasterCell* rasterCell) : VisualRepresentation(rasterCell) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);
};


}


#endif
