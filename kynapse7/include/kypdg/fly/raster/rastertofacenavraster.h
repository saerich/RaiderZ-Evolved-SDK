/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: JODA - secondary contact: NONE
#ifndef KyFlyPdg_RasterToFaceNavRaster_H
#define KyFlyPdg_RasterToFaceNavRaster_H

#include "kypdg/fly/generator/flycell.h"

namespace Kaim
{
class PdgSystem;
class RasterCell;
class DynamicNavRasterCell;

class RasterToFaceNavRaster
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	RasterToFaceNavRaster(PdgSystem* sys, const DynamicRasterCell* inputRaster, const FlyCellDesc& cellDesc);
	~RasterToFaceNavRaster();

	/*! Create the graph around the flyCell. */
	KyResult GenerateFaceGraph(DynamicPointGraphCell& graph);

protected:
	PdgSystem* m_sys;
	FlyCellDesc m_flyCellDesc;
	const DynamicRasterCell* m_inputRaster;
};

} // namespace Kaim

#endif
