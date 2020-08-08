/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPdg_DynamicNavCellBlobBuilder_H
#define KyPdg_DynamicNavCellBlobBuilder_H

#include <kypathdata/generic/memory.h>
#include "kypathdata/blob/baseblobbuilder.h"
#include "kypathdata/navmesh/navcell.h"

namespace Kaim
{

class DynamicNavCell;

class DynamicNavCellBlobBuilder : public BaseBlobBuilder<NavCell>
{
public:
	DynamicNavCellBlobBuilder(const DynamicNavCell* dynamicNavCell) { m_dynamicNavCell = dynamicNavCell; }

private:
	virtual void DoBuild();
	
	//void WriteTriangles(NavHalfEdge* edges, KyUInt32 numberOfTriangles);

	const DynamicNavCell* m_dynamicNavCell;
};


}


#endif
