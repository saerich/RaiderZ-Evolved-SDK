/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_finitestatemachine */

// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_FSM_DEFAULT_TRANSITION_H
#define KYRUNTIME_FSM_DEFAULT_TRANSITION_H

#include <kyruntime/finitestatemachine/itransition.h>
#include "kypathdata/generic/basetypes.h"

namespace Kaim
{

class IState;
class FSM;

///////////////////////////////////////////////////////////////////////////////////////////
// FiniteStateMachine transition
///////////////////////////////////////////////////////////////////////////////////////////

/*!	DefaultTransition is the base class you should use for writing your own custom classes that represent
	transitions between states in the finite state machine framework provided by the FSM class.
	\pp See also the FSM class description. 
	\ingroup kyruntime_finitestatemachine */
class DefaultTransition : public ITransition
{
public:
	/*! \label_constructor */
	DefaultTransition() : m_fsm(KY_NULL), m_nextState(KY_NULL) {}
	DefaultTransition(FSM* parentFsm, IState* nextState) : m_fsm(parentFsm), m_nextState(nextState) {}

	/*! \label_virtualdestructor */
	~DefaultTransition() {}

	/*! Sets the IState that will be returned by each call to GetNextState(). */
	void SetNextState(IState* nextState) { m_nextState = nextState; }

	/*! \copydoc ITransition::GetNextState()
		This implementation returns the IState pointer set in the last call to SetNextState(), or #KY_NULL if
		SetNextState() was never called. */
	IState* GetNextState() { return m_nextState; }

	/*! \copydoc ITransition::ConditionIsTrue()
		This implementation returns \c true in all cases. Re-implement it in your custom class if you want this DefaultTransition
		to make the FSM change states only under certain circumstances. */
	bool ConditionIsTrue() { return true; }

	void SetFsm(FSM* fsm) { m_fsm = fsm;}
	FSM* GetFsm() { return m_fsm;}

private:
	FSM* m_fsm;
	IState* m_nextState;
};

} // namespace Kaim

#endif // KYRUNTIME_FSM_DEFAULT_TRANSITION_H
