/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_DynamicRasterCellBlobBuilder_H
#define KyPdg_DynamicRasterCellBlobBuilder_H


#include "kypathdata/blob/baseblobbuilder.h"
#include "kypdg/raster/rastercell.h"


namespace Kaim
{

class DynamicRasterCell;
class SquarePartition;


class DynamicRasterCellBlobBuilder : public BaseBlobBuilder<RasterCell>
{
public:
	DynamicRasterCellBlobBuilder(const DynamicRasterCell* dynamicRasterCell, const SquarePartition* partition) :
		m_dynamicRasterCell(dynamicRasterCell), m_partition(partition)
	{}

private:
	virtual void DoBuild();

	const DynamicRasterCell* m_dynamicRasterCell;
	const SquarePartition* m_partition; 
};


}


#endif

