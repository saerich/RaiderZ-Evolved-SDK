/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_finitestatemachine */

// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_FSM_ISTATE_H
#define KYRUNTIME_FSM_ISTATE_H

#include "kypathdata/generic/basetypes.h"

namespace Kaim
{

class ITransition;
class FSM;

/*!	IState is the abstract base class for all states that can be managed by finite state machines that use or
	derive from the FSM class. This class cannot be used as-is; you must write your own class that derives from
	IState. However, it is recommended that you derive your custom class from DefaultState, which implements a method for
	registering ITransition objects.
	\ingroup kyruntime_finitestatemachine */
class IState
{
public:
	/*! \label_virtualdestructor */
	virtual ~IState() {}

	/*! Called automatically by the FSM class when this object is adopted as the current state for the finite state machine. */
	virtual void ExecuteEnterAction() = 0;

	/*! Called automatically by the FSM class when this object is the current state for the finite state machine. */
	virtual void ExecuteInsideAction() = 0;

	/*! Called automatically by the FSM class when this object was the current state for the finite state machine, but the
		finite state machine enters a new state. */
	virtual void ExecuteExitAction() = 0;

	/*! Retrieves the number of ITransition objects registered with this IState. */
	virtual KyUInt32 GetTransitionCount() = 0;

	/*! Retrieves the ITransition object identified by the specified index. */
	virtual ITransition* GetTransition(KyUInt32 index) = 0;

	/*! Sets the FSM using this IState. */
	virtual void SetFsm(FSM* fsm) = 0;

	/*! Retrieves the FSM using this IState. */
	virtual FSM* GetFsm() = 0;

	/*! Retrieves an integer that identifies the type for the state.
		If you dont want to identify your states, you can simply implement this function to return 1. */
	virtual KyUInt32 GetType() = 0;
};

} // namespace Kaim

#endif // KYRUNTIME_FSM_ISTATE_H
