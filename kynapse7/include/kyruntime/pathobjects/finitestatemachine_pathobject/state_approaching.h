/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_FSM_STATE_APPROACHING_H
#define KYRUNTIME_FSM_STATE_APPROACHING_H

#include <kyruntime/finitestatemachine/defaultstate.h>

namespace Kaim
{

/*	PathObject FiniteStateMachine Approach State.
*/
class State_Approaching : public DefaultState
{
public:
	void ExecuteEnterAction();
	void ExecuteInsideAction();
	void ExecuteExitAction();
};

} // namespace Kaim

#endif // KYRUNTIME_FSM_STATE_APPROACHING_H
