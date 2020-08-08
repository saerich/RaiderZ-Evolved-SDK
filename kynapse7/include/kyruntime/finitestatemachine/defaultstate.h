/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_finitestatemachine */

// primary contact: MAMU - secondary contact: NOBODY
#ifndef KYRUNTIME_FSM_DEFAULT_STATE_H
#define KYRUNTIME_FSM_DEFAULT_STATE_H

#include <kyruntime/finitestatemachine/istate.h>
#include "kypathdata/containers/array.h"

namespace Kaim
{

class ITransition;

/*!	DefaultState is the base class you should use for writing your own custom classes that represent
	states in the finite state machine framework provided by the FSM class.
	\pp See also the FSM class description. 
	\ingroup kyruntime_finitestatemachine */
class DefaultState : public IState
{
public:
	/*! \label_constructor */
	DefaultState();

	/*! \label_virtualdestructor */
	~DefaultState();

	///////////////////////////////////////////////////////////////////////////////////////////
	// Create state
	///////////////////////////////////////////////////////////////////////////////////////////

	/*! \label_clear */
	void Clear();

	/*! Registers the specified ITransition in the list of transitions maintained by this state. */
	void RegisterTransition(ITransition* transition);

	/*! Clears the list of transitions maintained by this state. */
	void UnregisterAllTransitions();


	///////////////////////////////////////////////////////////////////////////////////////////
	// Run state
	///////////////////////////////////////////////////////////////////////////////////////////

	/*! \copydoc IState::ExecuteEnterAction()
		This implementation is empty. Re-implement it in your custom class if you want to trigger specific events
		or calculations when this state is first adopted by the FSM. */
	virtual void ExecuteEnterAction();

	/*! \copydoc IState::ExecuteInsideAction()
		This implementation is empty. Re-implement it in your custom class if you want to trigger specific events
		or calculations at each frame while this state is the current state maintained by the FSM. */
	virtual void ExecuteInsideAction();

	/*! \copydoc IState::ExecuteExitAction()
		This implementation is empty. Re-implement it in your custom class if you want to trigger specific events
		or calculations at each frame when the FSM switches from this state to a different state. */
	virtual void ExecuteExitAction();


	///////////////////////////////////////////////////////////////////////////////////////////
	// Access data
	///////////////////////////////////////////////////////////////////////////////////////////

	KyUInt32 GetTransitionCount();
	ITransition* GetTransition(KyUInt32 index);

	void SetFsm(FSM* fsm) { m_fsm = fsm; }
	FSM* GetFsm() { return m_fsm; }

	/*! \copydoc IState::GetType()
		This implementation returns 0 in all cases. */
	virtual KyUInt32 GetType() { return 0; }

private:
	FSM* m_fsm;
	Array<ITransition*> m_transitions;
};

} // namespace Kaim

#endif // KYRUNTIME_FSM_DEFAULT_STATE_H
