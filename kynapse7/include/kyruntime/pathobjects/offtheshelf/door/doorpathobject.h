/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*	\file
	\ingroup kyruntime_pathobjects */

// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_DOOR_PATHOBJECT_H
#define KYRUNTIME_DOOR_PATHOBJECT_H

#include <kyruntime/pathobjects/offtheshelf/basepathobject.h>

namespace Kaim
{

/*
*/
class DoorPathObject : public BasePathObject
{
public:
	DoorPathObject(IPathObjectInfo* info, IPathObjectTopology* topology, IPathObjectBehavior* behavior): BasePathObject(info, topology, behavior) {}
	~DoorPathObject() {}
};

} // namespace Kaim

#endif // KYRUNTIME_DOOR_PATHOBJECT_H
