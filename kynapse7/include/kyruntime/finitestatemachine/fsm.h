/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_finitestatemachine */

// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_FSM_H
#define KYRUNTIME_FSM_H

#include "kyruntime/finitestatemachine/itransition.h"
#include "kypathdata/containers/list.h"

namespace Kaim
{

class IState;

/*!	This class represents a basic finite state machine. A specialized implementation of this class is used by the
	BasePathObject class and all off-the-shelf PathObject classes supplied with the \SDKName SDK. You can also use it
	in your own decision logic, agents, and other custom components if you wish.
	\section usingfsm Using the finite state machine
	To use the finite state machine provided by the FSM class:
	-#	Write new classes that derive from the abstract base IState and ITransition classes to represent each of the different
		possible states that the finite state machine can enter, and the different transitions that are possible between those
		states. It is recommended that you derive your classes from the DefaultState and DefaultTransition implementations.
	-#	Register your ITransition objects with your IStates. If you use classes that derive from DefaultState and DefaultTransition,
		you can call the DefaultState::RegisterTransition() method of each state to register the set of transitions that can make
		the FSM switch to a different state.
	-#	Create a new FSM object using its constructor.
	-#	Call Start() to start the machine, providing an object that implements the IState interface. This IState will
		be used as the initial state.
	-#	The machine calls the IState::ExecuteEnterAction() and IState::ExecuteInsideAction() methods of its current IState.
	-#	Each time Update() is called, the machine retrieves each instance of ITransition that is registered in the IState,
		in the order in which those ITransition objects were registered with the IState. It tests each ITransition by calling
		ITransition::ConditionIsTrue(). 
		-	If ITransition::ConditionIsTrue() returns \c true, the machine calls the IState::ExecuteExitAction() method of the
			current IState. It sets its current IState to the one returned by ITransition::GetNextState(), and calls the 
			IState::ExecuteEnterAction() and IState::ExecuteInsideAction() methods of this new IState. It then repeats this step.
		-	If ITransition::ConditionIsTrue() returns \c false for all ITransitions registered for the current IState, the
			machine calls IState::ExecuteInsideAction() for the current IState again. It then repeats this step.
	-#	When an implementation of ITransition::GetNextState() indicates that the next IState is #KY_NULL, the machine stops.
		You can also call Stop() explicitly. The IState::ExecuteExitAction() method of the current IState is called automatically,
		then the machine stops.
	\ingroup kyruntime_finitestatemachine */
class FSM : public VirtualMemObject
{
public:
	/*! \label_constructor */
	FSM();

	/*! \label_virtualdestructor */
	~FSM();

	//////////////////////////////////////////////////////////////////////////////////////////
	// Run finite state machine
	//////////////////////////////////////////////////////////////////////////////////////////

	/*! Starts the finite state machine at the specified IState. */
	IState* Start(IState* startState);

	/*! Updates the finite state machine. */
	void Update();

	/*! Forces the current IState to \c currentState, then updates the finite state machine. Note that the
		IState::ExecuteExitAction() method of the previous IState is not called, nor is the IState::ExecuteEnterAction()
		method of the new IState. */
	IState* Update(IState* currentState);

	/*! Stops the finite state machine after calling the IState::ExecuteExitAction() method of the current IState. */
	void Stop();

	/*! Forces the current IState to \c currentState, then stops the finite state machine. Note that the
		IState::ExecuteExitAction() method of the previous IState is not called, but the IState::ExecuteExtAction()
		method of the new IState is called. */
	void Stop(IState* currentState);

	//////////////////////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS & MEMBERS
	//////////////////////////////////////////////////////////////////////////////////////////
private:
	void ChangeState(IState* state);

private:
	IState* m_currentState;
	bool m_isRunning;
};

} // namespace Kaim

#endif // KYRUNTIME_FSM_H
