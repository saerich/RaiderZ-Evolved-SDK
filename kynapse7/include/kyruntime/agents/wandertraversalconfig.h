/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_WANDERTRAVERSALCONFIG_H
#define KY_WANDERTRAVERSALCONFIG_H

#include <kyruntime/graph/astartraversalconfig.h>
#include <kyruntime/agents/wandertraversal.h>
#include <kypathdata/basesystem/config.h>

namespace Kaim
{

/*!	The WanderTraversalConfig class allows the TraversalManager to create and serve an instance of the WanderTraversal
	class. 
	\pp This class is closely coupled with the WanderTraversal and the WanderAgent. For details on using this class,
	see :USERGUIDE:"Wandering".
	\ingroup kyruntime_agents */
class WanderTraversalConfig : public AstarTraversalConfig
{
public:
	/*!	\label_constructor If you use this constructor, you must at a minimum configure the #m_aperiodicTaskId class member
		before attempting to use your object. */
	WanderTraversalConfig();

	/*!	\label_constructor 
		\param timeMgr					The TimeManager maintained by the World.
		\param aperiodicTaskName		Sets the value of #m_aperiodicTaskId from the unique name specified for the aperiodic task. 
		\param estimatedTaskName		Sets the value of #m_estimatedTaskId from the unique name specified for the estimation.
		\param wanderDistance			Sets the value of #m_wanderDistance.
		\param wanderPropagationAngle	Sets the value of #m_wanderPropagationAngle. */
	WanderTraversalConfig(TimeManager& timeMgr, const char* aperiodicTaskName, const char* estimatedTaskName=KY_NULL,
		KyFloat32 wanderDistance=15.f, 
		KyFloat32 wanderPropagationAngle=150.f);

	/*!	\label_constructor 
		\param aperiodicTaskId			Sets the value of #m_aperiodicTaskId.
		\param estimatedTaskId			Sets the value of #m_estimatedTaskId.
		\param wanderDistance			Sets the value of #m_wanderDistance.
		\param wanderPropagationAngle	Sets the value of #m_wanderPropagationAngle. */
	WanderTraversalConfig(TimeManager::TaskId aperiodicTaskId, TaskRegistry::TaskId estimatedTaskId=TaskRegistry::InvalidTaskId,
		KyFloat32 wanderDistance=15.f, 
		KyFloat32 wanderPropagationAngle=150.f);

	virtual GraphTraversal* CreateGraphTraversal(Kaim::TraversalManager& traversalManager) const;

	virtual KyBool IsValid() const;

	virtual WanderTraversalConfig* Clone() const { return KY_NEW_CLASS(WanderTraversalConfig)(*this); }

	/*!	Determines the minimum preferred distance between the starting point of the WanderTraversal and any candidate vertex.
		\units			meters
		\acceptedvalues	any positive value or \c 0.0f 
		\defaultvalue	\c 15.0f */
	KyFloat32 m_wanderDistance;
	
	/*!	Determines the angle of the cone within which the WanderTraversal prefers to select destination vertices. The value of
		this parameter should represent the full width of the cone, which will be bisected by the orientation of the Bot 
		using the WanderAgent.
		\units			degrees
		\acceptedvalues	any negative or positive value
		\defaultvalue	\c 150.0f */
	KyFloat32 m_wanderPropagationAngle;
};

} // namespace Kaim

#endif // KY_WANDERTRAVERSALCONFIG_H
