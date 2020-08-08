/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: LASI - secondary contact: NOBODY

// This class is meant to speed-up the evaluation of IsInside() for each pixel in a raster
// by pre-computing some early exit conditions. The benefits of this class regarding performance are
// critical as IsPixelPos() is called once for each pixel of the raster (~10 000 times / cell) and for 
// each coloration volume
// Call to ExtrudedPolyline::IsInside() can be very costly (especially when the polyline has many vertices) 
// and therefore need to be avoided as much as possible, hence this class. 

#ifndef KyPdg_RasterCellTerrainTypeVolume_H
#define KyPdg_RasterCellTerrainTypeVolume_H

#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/pdginput/pdginputterraintypevolume.h"
#include "kypathdata/math/extrudedpolyline.h"
#include "kypdg/pdginput/inputtriangle.h"

namespace Kaim
{

class DynamicRasterCell;
class PdgSystem;

class RasterCellTerrainTypeVolume
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	RasterCellTerrainTypeVolume();
	RasterCellTerrainTypeVolume& operator=( const RasterCellTerrainTypeVolume& other)
	{
		Initialize(other.m_volume,other.m_cell);
		return *this;
	}
	void Initialize(const PdgInputTerrainTypeVolume* volume, const DynamicRasterCell* cell);
	inline bool IsPixelPosInVolume(const Vec3f& pixelPos) const;
	TerrainTypeMask GetTerrainTypeMask() const { return m_volume->m_terrainTypeMask; }
private: 
	// compute the value of the 3 bools used to speedup IsPixelPosInVolume()
	// automatically called in Ctor()
	void ComputeStatus();
private:
	const PdgInputTerrainTypeVolume* m_volume;
	ExtrudedPolyline m_polyline;
	const DynamicRasterCell* m_cell;
	KyInt32 m_pixelSize;
	PixelBox m_pixelBox;

	bool m_cellFullyContained;
	bool m_cellFullyOutside;
	bool m_volumeFullyInside;
	
};

inline bool RasterCellTerrainTypeVolume::IsPixelPosInVolume(const Vec3f& pixelPos) const 
{
	// pixel is in volume if:
	//    (1) the volume is fully contained within the cell (small volume) and pixel is inside volume
	// or (2) the cell is fully contained within the volume and altitude is compatible
	// or (3) none of the 2 condition above is true but the pixel is inside the volume (costly evaluation)
	bool pixelInVolume = // optimize computation, use the volumes[v].IsInside3d as a last resort (can cost a lot if a polyline has many vertices)
		/*(1)*/ (m_volumeFullyInside && m_polyline.IsInside3d(pixelPos)) 
		/*(2)*/ || (m_cellFullyContained && pixelPos.z <= m_polyline.GetMaxAlt() && pixelPos.z >= m_polyline.GetMinAlt())
		/*(3)*/ || m_polyline.IsInside3d(pixelPos);
	return pixelInVolume;
}

} // namespace Kaim

#endif //KyPdg_RasterCellTerrainTypeVolume_H
