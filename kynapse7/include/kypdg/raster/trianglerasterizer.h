/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_TriangleRasterizer_H
#define KyPdg_TriangleRasterizer_H


#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/raster/rastertriangle.h"
#include "kypdg/pdginput/inputtriangle.h"


namespace Kaim
{

class Triangle3i;
class PdgSystem;
class RasterPoint;
class SingleStageRaster;


class TriangleRasterizer
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	TriangleRasterizer() { Init(KY_NULL, KY_NULL); }

	void Init(KyInt32 pixelSize, SingleStageRaster* singleStageRaster);

	void RasterizeTriangle(const InputTriangle& triangle);

private:
	void RasterizePoint(const RasterPoint& M);
	void RasterizeInterior();
	void WriteLine(Coord min_x, Coord max_x, Coord y, bool skipWriteEastOfFirst);
	void WriteSinglePixelLine(Coord min_x, Coord max_x, PixelCoord py, Coord y, bool skipWriteEastOfFirst);
	void WriteDoublePixelLine(Coord min_x, Coord max_x, Coord y, bool skipWriteEastOfFirst);
	void FlagLineEnds();

private:
	KyInt32 m_pixelSize;
	SingleStageRaster* m_singleStageRaster;
	RasterTriangle m_triangle;
	CoordBox m_rasterCoordBox;
};


}


#endif
