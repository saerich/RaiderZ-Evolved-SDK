/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KYRUNTIME_JUMP_PATHOBJECT_BEHAVIOR_H
#define KYRUNTIME_JUMP_PATHOBJECT_BEHAVIOR_H

#include "kyruntime/pathobjects/offtheshelf/basepathobjectbehavior.h"
#include "kyruntime/pathobjects/offtheshelf/moves/jumpmove.h"

namespace Kaim
{

/*
Specify the interface to implement the behavior of the PathObject.
*/
class JumpPathObjectBehavior : public BasePathObjectBehavior
{
public:
	/*!	Creates a jump PathObject behavior with a default jump move with a default jump height.*/
	JumpPathObjectBehavior();

	/*!	Creates a jump PathObject behavior with a default jump move.*/
	explicit JumpPathObjectBehavior(KyFloat32 jumpHeight);

	/*!	Creates a jump PathObject behavior with a default jump move.
		\param jumpHeight								The height of the jump.
		\param maxBotsUsingThisBehaviorAtheSameTime		Sets the max number of bots using this behavior at the same time (if more bots 
														use it at the same time, there will be a dynamic allocation).*/
	JumpPathObjectBehavior(KyFloat32 jumpHeight, KyUInt16 maxBotsUsingThisBehaviorAtheSameTime);

	/*!	Creates a jump PathObject behavior with a custom jump move passed in arguments.*/
	JumpPathObjectBehavior(IMove* jumpMove);

	virtual ~JumpPathObjectBehavior();

	virtual void Traverse(PathObjectBehaviorContext& /*context*/);

protected:
	virtual void Init(IMove* jumpMove, KyFloat32 jumpHeight, KyUInt16 maxBotsUsingThisBehaviorAtheSameTime);

public:
	IMove* m_jumpMove;
	bool m_useDefaultJumpMove;
};

}

#endif // #ifndef KYRUNTIME_JUMP_PATHOBJECT_BEHAVIOR_H
