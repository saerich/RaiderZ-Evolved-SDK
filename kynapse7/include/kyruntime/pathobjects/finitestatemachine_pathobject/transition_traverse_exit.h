/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_FSM_TRANSITION_TRAVERSE_EXIT_H
#define KYRUNTIME_FSM_TRANSITION_TRAVERSE_EXIT_H

#include <kyruntime/finitestatemachine/defaulttransition.h>

namespace Kaim
{

class IState;

/*	FiniteStateMachine transition from traverse state to fsm exit
*/
class Transition_Traverse_Exit : public DefaultTransition
{
public:
	bool ConditionIsTrue();
};

} // namespace Kaim

#endif // KYRUNTIME_FSM_TRANSITION_TRAVERSE_EXIT_H
