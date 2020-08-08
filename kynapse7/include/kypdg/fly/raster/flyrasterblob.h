/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: BEHE - secondary contact: NONE
#ifndef KyFlyPdg_FlyRasterBlob_H
#define KyFlyPdg_FlyRasterBlob_H


#include "kypathdata/blob/endianness.h"
#include <kypathdata/basesystem/coordsystem.h>
#include "kypdg/generator/squarepartition.h"
#include "kypathdata/blob/blobarray.h"
#include "kypdg/generator/celldesc.h"
#include "kypdg/fly/raster/flyraster.h"
#include "kypdg/raster/rasterpoint.h"
#include "kypathdata/containers/stringstream.h"


namespace Kaim
{

class FlyRasterNodeForBlob
{
public:
	PixelBox m_horiBounds; // inclusive
	KyFloat32 m_bottom;
	KyFloat32 m_top;
	KyUInt32 m_splitPlane; /*SplitPlane*/
	KyUInt32 m_childLow; // index of child in the blob array; KyUInt32MAXVAL if none
	KyUInt32 m_childHigh; // index of child in the blob array; KyUInt32MAXVAL if none
	KyUInt32 m_convexKernelIdx; // index of convexKernel start in the blob array; KyUInt32MAXVAL if none
	RasterPoint m_point;
};
inline void SwapEndianness(Endianness::Target e, FlyRasterNodeForBlob& self)
{
	SwapEndianness(e, self.m_horiBounds);
	SwapEndianness(e, self.m_bottom);
	SwapEndianness(e, self.m_top);
	SwapEndianness(e, self.m_splitPlane);
	SwapEndianness(e, self.m_childLow);
	SwapEndianness(e, self.m_childHigh);
	SwapEndianness(e, self.m_convexKernelIdx);
	SwapEndianness(e, self.m_point);
}

class FlyRasterBlob
{
public:
	static const char* GetBlobType() { return "FlyRaster"; }
	static KyUInt32 GetBlobVersion() { return 0; }

	CoordSystem m_coordSystem;
	SquarePartition m_partition;
	CellDesc m_cellDesc;
	BlobArray<FlyRasterNodeForBlob> m_nodes;
	BlobArray<ValidatedInterval> m_convexKernels;
};
inline void SwapEndianness(Endianness::Target e, FlyRasterBlob& self)
{
	SwapEndianness(e, self.m_coordSystem);
	SwapEndianness(e, self.m_partition);
	SwapEndianness(e, self.m_cellDesc);
	SwapEndianness(e, self.m_nodes);
}

} // namespace Kaim

#endif
