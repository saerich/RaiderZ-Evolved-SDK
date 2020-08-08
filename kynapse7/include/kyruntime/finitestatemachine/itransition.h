/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_finitestatemachine */

// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_FSM_ITRANSITION_H
#define KYRUNTIME_FSM_ITRANSITION_H

namespace Kaim
{

class IState;
class FSM;

/*!	ITransition is the abstract base class for all transitions that make finite state machines that use or
	derive from the FSM class change from one IState to another. This class cannot be used as-is; you must
	write your own class that derives from ITransition. However, it is recommended that you derive your custom class
	from DefaultTransition instead. 
	\ingroup kyruntime_finitestatemachine */
class ITransition
{
public:
	/*! \label_virtualdestructor */
	virtual ~ITransition() {}

	/*! When ConditionIsTrue() returns #KY_TRUE for this ITransition, the FSM calls this method to retrieve 
		the next state it should adopt.
		\return A pointer to the IState that represents the next state for the finite state machine, or #KY_NULL to stop
				the finite state machine. */
	virtual IState* GetNextState() = 0;

	/*! The FSM calls this method to determine whether or not the conditions are right to carry out this ITransition.
		\return #KY_TRUE to make the FSM change its current state to the IState returned by GetNextState(), or #KY_FALSE
				to indicate that conditions are not right to carry out this ITransition. The FSM will move on to call the
				ConditionIsTrue() method of the next ITransition registered to its current IState. */
	virtual bool ConditionIsTrue() = 0;

	/*! Sets the FSM using this ITransition. */
	virtual void SetFsm(FSM* fsm) = 0;

	/*! Retrieves the FSM using this ITransition. */
	virtual FSM* GetFsm() = 0;
};

} // namespace Kaim

#endif // KYRUNTIME_FSM_ITRANSITION_H
