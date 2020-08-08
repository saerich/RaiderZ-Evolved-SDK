/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_CellRasterizerFromDynamicTriangulation_H
#define KyPdg_CellRasterizerFromDynamicTriangulation_H

#include "kypdg/raster/cellrasterizer.h"
#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/generator/celldesc.h"
#include "kypdg/common/stlvector.h"
#include "kypdg/raster/trianglerasterizer.h"
#include "kypdg/raster/singlestageraster.h"

namespace Kaim
{

class PdgSystem;
class PdgInputCell;
class DynamicTriangulation;
template <typename T>
class Array;

class CellRasterizerFromDynamicTriangulation : public CellRasterizer
{
public:
	CellRasterizerFromDynamicTriangulation(KyInt32 pixelSize, DynamicTriangulation* triangulation, DynamicRasterCell* dynamicRaster, const PixelBox& pixelBox, const Array<KyUInt32>* modifiedTriangles)
		: CellRasterizer(dynamicRaster)
	{
		m_dynamicTriangulation = triangulation;
		m_pixelBox = pixelBox;
		m_modifiedTriangles = modifiedTriangles;
		m_singleStageRaster.Init(m_pixelBox);
		m_triangleRasterizer.Init(pixelSize, &m_singleStageRaster);
	}

public: 
	void SetDynamicTriangulation(DynamicTriangulation* triangulation)
	{
		m_dynamicTriangulation = triangulation;
	}
	void SetDynamicRasterCell(DynamicRasterCell* rasterCell)
	{
		m_dynamicRaster = rasterCell;
	}
	void SetModifiedTriangles(const Array<KyUInt32>* triangles)
	{
		m_modifiedTriangles = triangles;
	}

protected:
	virtual KyResult DoRasterize();

protected:
	DynamicTriangulation* m_dynamicTriangulation;
	PixelBox m_pixelBox;
	const Array<KyUInt32>* m_modifiedTriangles;
	SingleStageRaster m_singleStageRaster;
	TriangleRasterizer m_triangleRasterizer;
};


}


#endif
