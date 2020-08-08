/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_BASE_PATHOBJECT_FSM_H
#define KYRUNTIME_BASE_PATHOBJECT_FSM_H

#include "kyruntime/finitestatemachine/fsm.h"
#include "kyruntime/pathobjects/finitestatemachine_pathobject/state_approaching.h"
#include "kyruntime/pathobjects/finitestatemachine_pathobject/state_traversing.h"
#include "kyruntime/pathobjects/finitestatemachine_pathobject/state_escaping.h"
#include "kyruntime/pathobjects/finitestatemachine_pathobject/state_blocked.h"
#include "kyruntime/pathobjects/finitestatemachine_pathobject/transition_approach_exit.h"
#include "kyruntime/pathobjects/finitestatemachine_pathobject/transition_approach_traverse.h"
#include "kyruntime/pathobjects/finitestatemachine_pathobject/transition_traverse_exit.h"
#include "kyruntime/pathobjects/finitestatemachine_pathobject/transition_escape_blocked.h"
#include "kyruntime/pathobjects/finitestatemachine_pathobject/transition_escape_exit.h"
#include "kyruntime/pathobjects/finitestatemachine_pathobject/transition_blocked_exit.h"

namespace Kaim
{

class PathObjectBehaviorContext;
class PathObject;
class IPathObjectBehavior;

///////////////////////////////////////////////////////////////////////////////////////////
// FiniteStateMachine
///////////////////////////////////////////////////////////////////////////////////////////

/*	
*/
class BasePathObjectFSM : public FSM
{
public:
	BasePathObjectFSM();
	~BasePathObjectFSM() {}

	void StartApproach(PathObjectBehaviorContext* currentContext);
	void StartEscape(PathObjectBehaviorContext* currentContext);

	void Update(PathObjectBehaviorContext* currentContext);

	void Stop(PathObjectBehaviorContext* currentContext);

public:
	PathObjectBehaviorContext* m_currentContext;
	PathObject* m_currentPathObject;
	IPathObjectBehavior* m_currentBehavior;

private:
	State_Approaching m_stateApproaching;
	State_Traversing m_stateTraversing;
	State_Escaping m_stateEscaping;
	State_Blocked m_stateBlocked;
	Transition_Approach_Exit m_transitionApproachExit;
	Transition_Approach_Traverse m_transitionApproachTraverse;
	Transition_Traverse_Exit m_transitionTraverseExit;
	Transition_Escape_Blocked m_transitionEscapeBlocked;
	Transition_Escape_Exit m_transitionEscapeExit;
	Transition_Blocked_Exit m_transitionBlockedExit;
};

} // namespace Kaim

#endif // KYRUNTIME_BASE_PATHOBJECT_FSM_H
