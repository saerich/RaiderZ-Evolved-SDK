/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: BEHE - secondary contact: NONE
#ifndef KyFlyPdg_FlyRasterBlobBuilder_H
#define KyFlyPdg_FlyRasterBlobBuilder_H

#include "kypathdata/blob/baseblobbuilder.h"
#include "kypdg/fly/raster/flyrasterblob.h"
#include "kypdg/fly/raster/flyraster.h"

namespace Kaim
{
class FlyRasterBlob;


class FlyRasterBlobBuilder : public BaseBlobBuilder<FlyRasterBlob>
{
public:
	FlyRasterBlobBuilder(FlyRaster* flyRaster) :
		m_flyRaster(flyRaster)
	{}

	virtual void DoBuild();

private:
	void FillBlobNodeArray(const FlyRasterNode& node, KyUInt32& indexInBlobArray, FlyRasterNodeForBlob* blobArray,
							KyUInt32& indexInConvexKernelsArray, ValidatedInterval* convexKernelsArray, FlyRasterBlob* blob);

public:
	FlyRaster* m_flyRaster;
};

} // namespace Kaim

#endif
