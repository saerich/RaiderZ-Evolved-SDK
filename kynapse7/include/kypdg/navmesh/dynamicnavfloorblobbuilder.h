/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPdg_DynamicNavFloorBlobBuilder_H
#define KyPdg_DynamicNavFloorBlobBuilder_H

#include <kypathdata/generic/memory.h>
#include "kypathdata/blob/baseblobbuilder.h"
#include "kypathdata/navmesh/navfloor.h"

namespace Kaim
{

class DynamicNavFloor;


class DynamicNavFloorBlobBuilder : public BaseBlobBuilder<NavFloor>
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	DynamicNavFloorBlobBuilder(const DynamicNavFloor* dynamicNavFloor) { m_dynamicNavFloor = dynamicNavFloor; }

private:
	virtual void DoBuild();
	
	void WriteTriangles(NavHalfEdge* edges, KyUInt32 numberOfTriangles);

	const DynamicNavFloor* m_dynamicNavFloor;
};


}


#endif
