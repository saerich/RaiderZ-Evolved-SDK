/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KYRUNTIME_LADDER_TRAVERSE_BEHAVIOR_H
#define KYRUNTIME_LADDER_TRAVERSE_BEHAVIOR_H

#include <kypathdata/generic/memory.h>
#include <kyruntime/pathobjects/offtheshelf/ladder/laddertraversefsm.h>

namespace Kaim
{

class PathObjectBehaviorContext;

/*
*/
class LadderTraverseBehavior
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	Create a default ladder behavior with default fsm.*/
	LadderTraverseBehavior();

	/*!	Create a default ladder behavior with default fsm.
		\param maxBotsUsingThisBehaviorAtTheSameTime	Sets the max number of bots using this behavior
														at the same time (it is a growable pool, so if 
														more bots use this move at the same time, there 
														will be a dynamic allocation).
	*/
	LadderTraverseBehavior(KyUInt16 maxBotsUsingThisBehaviorAtTheSameTime);

	/*!	Create a default ladder behavior with custom fsm.*/
	LadderTraverseBehavior(LadderTraverseFSM* fsm);

	virtual ~LadderTraverseBehavior();

	virtual void OnEnterPathObject(PathObjectBehaviorContext& /*context*/);
	virtual void OnLeavePathObject(PathObjectBehaviorContext& /*context*/);

	virtual void OnStartTraversing(PathObjectBehaviorContext& /*context*/);
	virtual void Traverse(PathObjectBehaviorContext& /*context*/);
	virtual void OnEndTraversing(PathObjectBehaviorContext& /*context*/);
	virtual bool ExitReached(PathObjectBehaviorContext& context);

	virtual void OnStartEscaping(PathObjectBehaviorContext& /*context*/);
	virtual void Escape(PathObjectBehaviorContext& /*context*/);
	virtual void OnEndEscaping(PathObjectBehaviorContext& /*context*/);

protected:
	virtual void Init(LadderTraverseFSM* fsm, KyUInt16 maxBotsUsingThisBehaviorAtTheSameTime);

public:
	LadderTraverseFSM* m_fsm;
	bool m_useDefaultTraverseFsm;
};

}

#endif // #ifndef KYRUNTIME_LADDER_PATHOBJECT_BEHAVIOR_H
