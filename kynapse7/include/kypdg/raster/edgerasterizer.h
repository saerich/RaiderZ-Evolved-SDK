/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_EdgeRasterizer_H
#define KyPdg_EdgeRasterizer_H


#include "kypdg/generator/squarepartitiontypes.h"


namespace Kaim
{

	class PdgSystem;
	class Box2i;
	class RasterPoint;
	class SingleStageRaster;


	class EdgeRasterizer
	{
	public:
		EdgeRasterizer(KyInt32 pixelSize, SingleStageRaster* singleStageRaster);

		bool InitToRasterizeAlong_X(const RasterPoint& A, const RasterPoint& B);

		bool InitToRasterizeAlong_Y(const RasterPoint& A, const RasterPoint& B);

		void Rasterize();

	private:
		void FinalizeInit();
		friend class EdgeRasterizerImpl;

	private:
		KyInt32 m_pixelSize;
		KyInt64 m_pixelSize64; // same in 64 bits

		SingleStageRaster* m_singleStageRaster;
		KyFloat32 m_meter_pixelSize;


		Coord m_Au; // Coordinate of A along "u" axis
		Coord m_Av; // Coordinate of A along "v" axis
		KyFloat32 m_meter_Az; // the altitude in meter of point A

		Coord m_Bu; // Coordinate of B along "u" axis
		Coord m_Bv; // Coordinate of B along "u" axis
		KyFloat32 m_meter_Bz;  // the altitude in meter of point B

		// Raster pixelBox in uv coordinates
		// retrieved during InitToRasterizeAlong_X/Y 
		PixelCoord m_MINpu;
		PixelCoord m_MAXpu;
		PixelCoord m_MINpv;
		PixelCoord m_MAXpv;

		KyFloat32 m_dz_on_du; // gradient of altitude along "u" 
		KyFloat32 m_meter_step_z; // increment of altitude by pixel along "u"

		// internal values we cache to speedUp GetPv() computation
		KyInt64 m_ABu; // = (KyInt64)(m_Bu - m_Au);
		KyInt64 m_ABv; // = (KyInt64)(m_Bv - m_Av); used in GetPv()
		KyInt64 m_ABu_by_Av; // = m_ABu * m_Av; used in GetPv()
		KyInt64 m_ABu_by_pixelSize; // = m_ABu * m_coord_pixelSize; used in GetPv()

		// so: Does "u" means X axis or Y axis ? and v ? 
		// m_x_in_uv = 0;
		// m_y_in_uv = 1; 
		// means Rasterize along X
		//	m_x_in_uv = 1;
		//  m_y_in_uv = 0;
		// means Rasterize along Y
		KyInt32 m_x_in_uv;
		KyInt32 m_y_in_uv;
	};


}


#endif
