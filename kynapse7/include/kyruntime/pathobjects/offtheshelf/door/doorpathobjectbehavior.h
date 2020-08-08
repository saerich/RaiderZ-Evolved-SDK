/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*	\file
	\ingroup kyruntime_pathobjects */

#ifndef KYRUNTIME_DOOR_PATHOBJECT_BEHAVIOR_H
#define KYRUNTIME_DOOR_PATHOBJECT_BEHAVIOR_H

#include "kyruntime/pathobjects/offtheshelf/basepathobjectbehavior.h"

namespace Kaim
{

/*
Specify the interface to implement the behavior of the PathObject.
*/
class DoorPathObjectBehavior : public BasePathObjectBehavior
{
public:
	/*!	Creates a door PathObject behavior with a default translation move.*/
	DoorPathObjectBehavior();

	/*!	Creates a door PathObject behavior with a default translation move.
		\param maxBotsUsingThisBehaviorAtheSameTime		Sets the typical number of bots using this behavior at the same time.
														Each time a bot uses this behavior, it retrieves a context from a pool which
														is initialized with this parameter. See ChunkBasedPool class for more details.
	*/
	DoorPathObjectBehavior(KyUInt16 maxBotsUsingThisBehaviorAtheSameTime);

	/*!	Creates a door PathObject behavior with a custom traverse move.*/
	DoorPathObjectBehavior(IMove* traverseDoorMove);

	virtual ~DoorPathObjectBehavior();

protected:
	virtual void Init(IMove* traverseDoorMove, KyUInt16 maxBotsUsingThisBehaviorAtheSameTime);

public:
	IMove* m_traverseDoorMove;
	bool m_useDefaultTraverseDoorMove;
};

}

#endif // #ifndef KYRUNTIME_DOOR_PATHOBJECT_BEHAVIOR_H
