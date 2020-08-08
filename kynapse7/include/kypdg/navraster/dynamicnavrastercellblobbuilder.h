/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_DynamicNavRasterCellBlobBuilder_H
#define KyPdg_DynamicNavRasterCellBlobBuilder_H


#include "kypathdata/blob/baseblobbuilder.h"
#include "kypdg/navraster/navrastercell.h"


namespace Kaim
{

class DynamicNavRasterCell;


class DynamicNavRasterCellBlobBuilder : public BaseBlobBuilder<NavRasterCell>
{
public:
	DynamicNavRasterCellBlobBuilder(DynamicNavRasterCell* dynamicNavRasterCell) :
		m_dynamicNavRasterCell(dynamicNavRasterCell)
	{}

	virtual void DoBuild();

public:
	DynamicNavRasterCell* m_dynamicNavRasterCell;
};


}


#endif

