/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_JUMP_PATHOBJECT_H
#define KYRUNTIME_JUMP_PATHOBJECT_H

#include <kyruntime/pathobjects/offtheshelf/basepathobject.h>

namespace Kaim
{

/*
*/
class JumpPathObject : public BasePathObject
{
public:
	JumpPathObject(IPathObjectInfo* info, IPathObjectTopology* topology, IPathObjectBehavior* behavior): BasePathObject(info, topology, behavior) {}
	~JumpPathObject() {}
};

} // namespace Kaim

#endif // KYRUNTIME_JUMP_PATHOBJECT_H
