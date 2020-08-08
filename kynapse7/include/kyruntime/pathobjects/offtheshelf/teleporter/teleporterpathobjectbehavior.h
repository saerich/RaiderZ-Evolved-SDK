/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KYRUNTIME_TELEPORTER_PATHOBJECT_BEHAVIOR_H
#define KYRUNTIME_TELEPORTER_PATHOBJECT_BEHAVIOR_H

#include "kyruntime/pathobjects/offtheshelf/basepathobjectbehavior.h"
#include "kyruntime/pathobjects/offtheshelf/moves/teleportmove.h"

namespace Kaim
{

/*
Specify the interface to implement the behavior of the PathObject.
*/
class TeleporterPathObjectBehavior : public BasePathObjectBehavior
{
public:
	/*!	Creates a teleporter PathObject behavior with a custom teleport move.*/
	TeleporterPathObjectBehavior(IMove* teleportMove);

	/*!	Creates a ladder PathObject behavior with default teleport move.*/
	TeleporterPathObjectBehavior();
	
	virtual ~TeleporterPathObjectBehavior();

	virtual void Activate() { m_active = false; }
	virtual void Deactivate() { m_active = true; }

public:
	virtual void Init(IMove* teleportMove);

public:
	bool m_active;
	IMove* m_teleportMove;
	bool m_useDefaultTeleportMove;
};

}

#endif // #ifndef KYRUNTIME_DOOR_PATHOBJECT_BEHAVIOR_H
