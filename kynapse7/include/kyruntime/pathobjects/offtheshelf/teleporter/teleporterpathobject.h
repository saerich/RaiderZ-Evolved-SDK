/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_TELEPORTER_PATHOBJECT_H
#define KYRUNTIME_TELEPORTER_PATHOBJECT_H

#include <kyruntime/pathobjects/offtheshelf/basepathobject.h>

namespace Kaim
{

/*
*/
class TeleporterPathObject : public BasePathObject
{
public:
	TeleporterPathObject(IPathObjectInfo* info, IPathObjectTopology* topology, IPathObjectBehavior* behavior) : BasePathObject(info, topology, behavior) {}
	~TeleporterPathObject() {}
};

} // namespace Kaim

#endif // KYRUNTIME_TELEPORTER_PATHOBJECT_H
