/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_FSM_TRANSITION_BLOCKED_EXIT_H
#define KYRUNTIME_FSM_TRANSITION_BLOCKED_EXIT_H

#include <kyruntime/finitestatemachine/defaulttransition.h>

namespace Kaim
{

class IState;
class FSM;

/*	FiniteStateMachine transition between blocked and exit states
*/
class Transition_Blocked_Exit : public DefaultTransition
{
public:
	bool ConditionIsTrue();
};

} // namespace Kaim

#endif // KYRUNTIME_FSM_TRANSITION_BLOCKED_EXIT_H
