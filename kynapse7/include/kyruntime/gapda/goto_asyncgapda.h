/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_gapda */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GOTO_ASYNCGAPDA_H
#define KY_GOTO_ASYNCGAPDA_H

#include <kyruntime/gapda/goto_gapda.h>


namespace Kaim
{

class Goto_AsyncGapDA_AsyncModule;


/*!	Goto_AsyncGapDA derives from Goto_GapDynamicAvoidance, and uses the same process to calculate 
	a new target point for the Bot that avoids other nearby \Bodies, and to construct an Action
	that moves the Bot toward that target point. However, it uses the asynchronous processing
	framework to distribute calculations to a secondary thread or subordinate processing unit.
	\pp See also :USERGUIDE:"Asynchronous Processing".
	\section gapdasetup Setting Up FindNodesFromPositions_AsyncNearestReachable
	Setting up this modifier involves the following considerations, in addition to those described in the
	parent Goto_GapDynamicAvoidance class.
	\subsection requirements Requirements
	-	You must activate the AsyncManager in your World through a call to World::ActivateService().
	-	You must call AsyncManager::AddModule() in order to set up an instance of an AsyncModule class that derives from 
		Goto_AsyncGapDA_AsyncModule, and you must call the Initialize() method of the instance
		returned. You can provide this instance in the call to Initialize(), but if you do not, OnInitialize() will attempt
		to retrieve the instance automatically from the AsyncManager.
	\subsection recommendations Recommendations
	-	It is recommended that you create an aperiodic task for the TimeManager to govern the time allotted to Goto_AsyncGapDA
		in each frame. You provide the ID of this task in the constructor. If you provide a valid aperiodic task ID, the Goto_AsyncGapDA
		AsyncModule will interrupt its calculations when no time remains in the current frame. If you do not provide a valid aperiodic
		task ID, you may see spikes of CPU usage in your asynchronous thread.
	\ingroup kyruntime_gapda */
class Goto_AsyncGapDA: public Goto_GapDynamicAvoidance
{
public:
	KY_DECLARE_GOTO(Goto_AsyncGapDA)


public:
	/*!	\label_constructor 
		\param gapManager			The GapManager that you have set up for the World. 
		\param asyncModule			An instance of FindNodesFromPositions_AsyncNearestReachable_AsyncModule that you have set up for the AsyncManager.
									If you do not provide it here, OnInitialize() will attempt to retrieve it automatically from the AsyncManager. 
		\param aperiodicTaskId		The unique ID of the aperiodic task that will govern the time allotted to Goto_AsyncGapDA in each frame. */
	Goto_AsyncGapDA(Kaim::GapManager* gapManager = KY_NULL, Goto_AsyncGapDA_AsyncModule* asyncModule = KY_NULL, TimeManager::TaskId aperiodicTaskId = TimeManager::InvalidTaskId);
	
	/*!	\label_virtualdestructor */
	~Goto_AsyncGapDA();

	virtual void ReInit();
	virtual KyBool Goto(const Kaim::PointWrapper& targetPoint, Kaim::Action& action);

	// async input

	/*!	Retrieves the elapsed time since the low-level refined mode last changed. For internal use. */
	KyFloat32 GetModeTimer() const { return m_modeTimer; }
	
	/*!	Retrieves the current target point maintained by the PathFinder and passed to the Goto() method. 
		For internal use. */
	const Vec3f& GetInputTargetPoint() const { return m_inputTargetPoint.GetPosition(); }


	// async process

	/*!	Called by the AsyncModule in order to begin the asynchronous calculations. */
	void AsyncGoto();


	// async result

	/*!	Sets the current low-level refined mode to the specified mode, and resets the mode timer
		to the specified value. For internal use. */
	void ForceCurrentMode(RefinedMode newMode, KyFloat32 modeTimer);
	
	/*!	Sets the speed of the Bot to the specified value. For internal use. */
	void SetOutputSpeed(KyFloat32 speed) { m_currentSpeed = speed; }


protected:
	virtual void OnSetPathFinder();

	// consts
	Goto_AsyncGapDA_AsyncModule* m_asyncDAModule;
	TimeManager::TaskId m_aperiodicTaskId;
	KyBool m_fullExternalUpdate;	// if true, a whole update is performed asynchroneously (SPU computation, typically)
									// if false, the asynchroneous update may be partial and require further work

	// input stored for later processing
	PointWrapper m_inputTargetPoint;
	Bot* m_bot;	

	// output of async process
	Action m_resultingAction;
	KyBool m_resultGoto;
};

} // namespace Kaim

#endif // KY_GOTO_ASYNCGAPDA_H
