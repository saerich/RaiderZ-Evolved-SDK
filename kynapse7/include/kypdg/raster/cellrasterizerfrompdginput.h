/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_CellRasterizerFromPdgInput_H
#define KyPdg_CellRasterizerFromPdgInput_H

#include "kypdg/raster/cellrasterizer.h"
#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/generator/celldesc.h"

namespace Kaim
{

class PdgSystem;
class PdgInputCell;
template <typename T>
class Array;

class CellRasterizerFromPdgInput : public CellRasterizer
{
public:

	CellRasterizerFromPdgInput(PdgSystem* sys, Array<const PdgInputCell*>* inputCells, const CellDesc& cellDesc, DynamicRasterCell* dynamicRaster)
		: CellRasterizer(dynamicRaster)
	{
		m_sys = sys;
		m_inputCells = inputCells;
		m_pixelBox =  cellDesc.m_enlargedPixelBox;
	}

protected:
	virtual KyResult DoRasterize();

protected:
	PdgSystem* m_sys;
	Array<const PdgInputCell*>* m_inputCells;
	PixelBox m_pixelBox; // = cellDesc.m_enlargedPixelBox
};


}


#endif
