/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_FSM_TRANSITION_APPROACH_TRAVERSE_H
#define KYRUNTIME_FSM_TRANSITION_APPROACH_TRAVERSE_H

#include <kyruntime/finitestatemachine/defaulttransition.h>

namespace Kaim
{

class IState;
class FSM;

/*	FiniteStateMachine transition between approach and traverse states
*/
class Transition_Approach_Traverse : public DefaultTransition
{
public:
	bool ConditionIsTrue();
};

} // namespace Kaim

#endif // KYRUNTIME_FSM_TRANSITION_APPROACH_TRAVERSE_H
