/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_LADDER_TRAVERSE_FSM_H
#define KYRUNTIME_LADDER_TRAVERSE_FSM_H

#include "kyruntime/finitestatemachine/fsm.h"
#include "kyruntime/finitestatemachine/defaultstate.h"
#include "kyruntime/finitestatemachine/defaulttransition.h"

namespace Kaim
{

class LadderTraverseContext
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	LadderTraverseContext() : m_currentState(KY_NULL), m_lastState(KY_NULL), m_currentSpeedFactor(1.0f) {}

public:
	IState* m_currentState;
	IState* m_lastState;
	KyFloat32 m_currentSpeedFactor;
};

enum LadderTraverseStateType
{
	LADDER_TRAVERSE_STATE_GOTO_LADDER,
	LADDER_TRAVERSE_STATE_GRAB_LADDER,
	LADDER_TRAVERSE_STATE_CLIMB_LADDER,
	LADDER_TRAVERSE_STATE_GET_DOWN_OF_LADDER,
	LADDER_TRAVERSE_STATE_GO_TO_EXIT
};

class State_GotoLadder : public DefaultState
{
public:
	State_GotoLadder() {}

public:
	void ExecuteEnterAction();
	void ExecuteInsideAction();
	void ExecuteExitAction();
	
public:
	KyUInt32 GetType() { return (KyUInt32)LADDER_TRAVERSE_STATE_GOTO_LADDER; }
};

class State_GrabLadder : public DefaultState
{
public:
	State_GrabLadder() {}

public:
	void ExecuteEnterAction() {}
	void ExecuteInsideAction();
	void ExecuteExitAction() {}

public:
	KyUInt32 GetType() { return (KyUInt32)LADDER_TRAVERSE_STATE_GRAB_LADDER; }
};

class State_ClimbLadder : public DefaultState
{
public:
	State_ClimbLadder() {}

public:
	void ExecuteEnterAction();
	void ExecuteInsideAction();
	void ExecuteExitAction();

public:
	KyUInt32 GetType() { return (KyUInt32)LADDER_TRAVERSE_STATE_CLIMB_LADDER; }
};

class State_GetDownOfLadder : public DefaultState
{
public:
	State_GetDownOfLadder() {}

public:
	void ExecuteEnterAction() {}
	void ExecuteInsideAction();
	void ExecuteExitAction() {}

public:
	KyUInt32 GetType() { return (KyUInt32)LADDER_TRAVERSE_STATE_GET_DOWN_OF_LADDER; }
};

class State_GoToExit : public DefaultState
{
public:
	State_GoToExit() {}

public:
	void ExecuteEnterAction();
	void ExecuteInsideAction();
	void ExecuteExitAction();

public:
	KyUInt32 GetType() { return (KyUInt32)LADDER_TRAVERSE_STATE_GO_TO_EXIT; }
};


class Transition_GotoLadder_GrabLadder : public DefaultTransition
{
public:
	bool ConditionIsTrue();
};

class Transition_GrabLadder_ClimbLadder : public DefaultTransition
{
public:
	bool ConditionIsTrue();
};

class Transition_ClimbLadder_GetDownOfLadder : public DefaultTransition
{
public:
	bool ConditionIsTrue();
};

class Transition_GetDownOfLadder_GoToExit : public DefaultTransition
{
public:
	bool ConditionIsTrue();
};

class Transition_GoToExit_Exit : public DefaultTransition
{
public:
	bool ConditionIsTrue();
};


class PathObjectBehaviorContext;
class LadderPathObject;
class LadderPathObjectBehavior;

/*!	
*/
class LadderTraverseFSM : public FSM
{
public:
	LadderTraverseFSM(KyUInt16 defaultContextCount);
	virtual ~LadderTraverseFSM();

	virtual void OnEnterPathObject(PathObjectBehaviorContext& currentContext);
	virtual void OnLeavePathObject(PathObjectBehaviorContext& currentContext);

	virtual void StartTraverse(PathObjectBehaviorContext* currentContext);
	virtual void StartEscape(PathObjectBehaviorContext* currentContext);

	virtual void Update(PathObjectBehaviorContext* currentContext);

	virtual void Stop(PathObjectBehaviorContext* currentContext);

	/*!	Compute the state from PathObject behavior context.
		This is used when an accident happen 
		eg: Get the state from position of the bot:
		1. If bot near start connexion space, returns State_GotoLadder.
		2. If bot inside scale with wrong orientation, returns State_GrabLadder.
		3. If bot inside scale with correct orientation, returns State_ClimbLadder.
		4. If bot near target connexion space with wrong orientation, returns State_GetDownOfLadder.
		5. If bot near target connexion space with correct orientation, returns State_GoToExit.*/
	virtual IState* GetEscapeStateFromContext(PathObjectBehaviorContext* currentContext);

public:
	LadderTraverseContext* m_currentLadderTraverseContext;
	PathObjectBehaviorContext* m_currentPathObjectBehaviorContext;
	LadderPathObject* m_pathObject;
	LadderPathObjectBehavior* m_behavior;

	ChunkBasedPool<LadderTraverseContext>* m_contextPool;

private:
	void SetContext(PathObjectBehaviorContext* currentContext);

private:
	State_GotoLadder m_stateGoToLadder;
	State_GrabLadder m_stateGrabLadder;
	State_ClimbLadder m_stateClimbLadder;
	State_GetDownOfLadder m_stateGetDownOfLadder;
	State_GoToExit m_stateGoToExit;
	
	Transition_GotoLadder_GrabLadder m_transition_GoToLadder_GrabLadder;
	Transition_GrabLadder_ClimbLadder m_transition_GrabLadder_ClimbLadder;
	Transition_ClimbLadder_GetDownOfLadder m_transition_ClimbLadder_GetDownOfLadder;
	Transition_GetDownOfLadder_GoToExit m_transition_GetDownOfLadder_GoToExit;
	Transition_GoToExit_Exit m_transition_GoToExit_Exit;
};

} // namespace Kaim

#endif // KYRUNTIME_BASE_PATHOBJECT_FSM_H
