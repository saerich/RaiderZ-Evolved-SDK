/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: None
#ifndef KYPATHDATA_DOOR_PATHOBJECT_INFO_H
#define KYPATHDATA_DOOR_PATHOBJECT_INFO_H

#include "kypathdata/pathobject/info/dynamicpathobjectinfo.h"

namespace Kaim
{

/*	DoorPathObjectInfo contains information about a basic door pathobject.
*/
class DoorPathObjectInfo : public DynamicPathObjectInfo
{

public:
	DoorPathObjectInfo(const PathObjectId& id, const char* databaseName, KyFloat32 averageEntityHeight) :
		DynamicPathObjectInfo(id, "KynapseDoor", databaseName, averageEntityHeight) {}
	~DoorPathObjectInfo() {}
};

} // namespace Kaim

#endif // KYPATHDATA_DOOR_PATHOBJECT_INFO_H
