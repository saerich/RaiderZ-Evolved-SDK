/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: None
#ifndef KYRUNTIME_LADDER_PATHOBJECT_H
#define KYRUNTIME_LADDER_PATHOBJECT_H

#include <kyruntime/pathobjects/offtheshelf/basepathobject.h>
#include <kypathdata/pathobject/offtheshelf/ladder/ladderpathobjectconnexionspace.h>

namespace Kaim
{

class LadderPathObjectConnexionSpace;

/*
*/
class LadderPathObject : public BasePathObject
{
public:
	LadderPathObject(IPathObjectInfo* info, IPathObjectTopology* topology, IPathObjectBehavior* behavior): BasePathObject(info, topology, behavior) {}
	~LadderPathObject() {}

	const Vec3f& GetGrabPosition(LadderPathObjectConnexionSpace* connexionSpace) { return connexionSpace->m_grabPosition; }
	const Vec3f& GetGrabOrientation(LadderPathObjectConnexionSpace* connexionSpace) { return connexionSpace->m_grabOrientation; }
};

} // namespace Kaim

#endif // KYRUNTIME_LADDER_PATHOBJECT_H
