/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPdg_DynamicNavMeshBlobBuilder_H
#define KyPdg_DynamicNavMeshBlobBuilder_H

#include <kypathdata/generic/memory.h>
#include "kypathdata/blob/baseblobbuilder.h"
#include "kypathdata/navmesh/navmesh.h"


namespace Kaim
{

class DynamicNavMesh;


class DynamicNavMeshBlobBuilder : public BaseBlobBuilder<NavMesh>
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	DynamicNavMeshBlobBuilder(const DynamicNavMesh* dynamicNavMesh) { m_dynamicNavMesh = dynamicNavMesh; }

private:
	virtual void DoBuild();

	const DynamicNavMesh* m_dynamicNavMesh;
};


}


#endif
