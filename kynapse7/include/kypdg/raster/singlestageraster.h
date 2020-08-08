/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_SingleStageRaster_H
#define KyPdg_SingleStageRaster_H


#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/common/bitarray2d_1024.h"
#include "kypdg/raster/singlestagepixel.h"
#include <kypdg/common/stlvector.h>


namespace Kaim
{

/*
Contains the memory that receives the rasterization of one triangle.
Given a clippingPixelBox, from a trianglePixelBox, make {rasterPixelBox, rasterPixelsMemory} available.
Has the ability to restore its initial state after a given geometry rasterization.

+-----------------------+
|clippingPixelBox       |
|                       |
|                       |
|                       |
|                       |
|                       |
|     +-----------------+---------+
|     |                 |         |
|     |rasterPixelBox   |         |
|     |                 |         |
+-----+-----------------+         |
      |                           |
      |           trianglePixelBox|
      +---------------------------+
*/
class SingleStageRaster
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	KyResult Init(const PixelBox& clippingPixelBox);

	bool SetupRasterPixelBox(const PixelBox& geometryPixelBox);

	void ClearPixels();

	void PushPixelValue(const PixelPos& pixelPos, KyFloat32 value)
	{
		assert(m_rasterPixelBox.IsInside(pixelPos));

		PixelPos localPixelPos = pixelPos - m_rasterPixelBox.Min();
		KyInt32 pixelIdx = m_rasterPixelBox.GetRowMajorIndexFromLocalPos(localPixelPos);
		m_pixels[pixelIdx].AddValue(value);
		m_bitArray2d.SetPixel(localPixelPos.x, localPixelPos.y);
	}

	void FlagPixel(const PixelPos& pixelPos)
	{
		assert(m_rasterPixelBox.IsInside(pixelPos));

		PixelPos localPixelPos = pixelPos - m_rasterPixelBox.Min();
		m_bitArray2d.SetPixel(localPixelPos.x, localPixelPos.y);
	}

	const SingleStagePixel& GetPixel(const PixelPos& pixelPos) const
	{
		return m_pixels[m_rasterPixelBox.GetRowMajorIndex(pixelPos)];
	}

	const SingleStagePixel& GetPixelFromLocalPos(const PixelPos& localPixelPos) const
	{
		return m_pixels[m_rasterPixelBox.GetRowMajorIndexFromLocalPos(localPixelPos)];
	}

	const PixelBox& GetRasterPixelBox() const { return m_rasterPixelBox; }

	PixelCoord GetFirstLine() { return m_bitArray2d.GetFirstY() + m_rasterPixelBox.Min().y; }
	PixelCoord GetLastLine()  { return m_bitArray2d.GetLastY()  + m_rasterPixelBox.Min().y; }
	PixelCoord GetNoLineIdx() { return m_rasterPixelBox.Min().y - 1; }

	PixelCoord GetFirstPixelOnLine(PixelCoord py) { return m_bitArray2d.GetFirstX(py - m_rasterPixelBox.Min().y) + m_rasterPixelBox.Min().x; }
	PixelCoord GetLastPixelOnLine(PixelCoord py)  { return m_bitArray2d.GetLastX( py - m_rasterPixelBox.Min().y) + m_rasterPixelBox.Min().x; }
	PixelCoord GetNoPixelIdx() { return m_rasterPixelBox.Min().x - 1; }

	// unit test usage
	bool IsPixelFlaggedFromLocalPos(const PixelPos& localPixelPos) const
	{
		return m_bitArray2d.GetPixel(localPixelPos.x, localPixelPos.y);
	}

	void SetTerrainTypeMask(KyUInt32 terrainTypeMask) { m_terrainTypeMask = terrainTypeMask; }
	KyUInt32 GetTerrainTypeMask() const { return m_terrainTypeMask; }

public:
	PixelBox m_clippingPixelBox;
	PixelBox m_rasterPixelBox;
	StlVector<SingleStagePixel> m_pixels; // row major
	BitArray2d_1024 m_bitArray2d;
	KyUInt32 m_terrainTypeMask;
};


}


#endif
