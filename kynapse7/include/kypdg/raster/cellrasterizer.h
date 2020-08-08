/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_CellRasterizer_H
#define KyPdg_CellRasterizer_H

#include "kypathdata/generic/basetypes.h"
#include "kypdg/generator/squarepartitiontypes.h"

namespace Kaim
{

class DynamicRasterCell;
class SingleStageRaster;

class CellRasterizer : public VirtualMemObject
{
public:

	CellRasterizer(DynamicRasterCell* dynamicRaster) : m_dynamicRaster(dynamicRaster) {};

	KyResult Rasterize();

protected:
	void Push(const SingleStageRaster& singleStageRaster);

protected:
	virtual KyResult DoRasterize() = 0;

protected:
	DynamicRasterCell* m_dynamicRaster;
};


}


#endif
