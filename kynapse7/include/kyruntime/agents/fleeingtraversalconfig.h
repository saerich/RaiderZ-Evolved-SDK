/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

#ifndef KY_FLEEINGTRAVERSALCONFIG_H
#define KY_FLEEINGTRAVERSALCONFIG_H

#include <kyruntime/graph/astartraversalconfig.h>
#include <kyruntime/agents/fleeingtraversal.h>


namespace Kaim
{

/*!	The FleeingTraversalConfig class allows the TraversalManager to create an instance of the FleeingTraversal class
	and make it available to other components in the World.
	\pp This class is closely coupled with the FleeingTraversal and the FleeAgent. For details on using this class,
	see :USERGUIDE:"Fleeing".
	\ingroup kyruntime_agents */
class FleeingTraversalConfig : public AstarTraversalConfig
{
public:
	/*!	\label_constructor If you use this constructor, you must at a minimum configure the #m_aperiodicTaskId class member
		before attempting to use your object. */
	FleeingTraversalConfig()
		:AstarTraversalConfig()
	{}

	/*!	\label_constructor 
		\param timeMgr				The TimeManager maintained by the World.
		\param aperiodicTaskName	Sets the value of #m_aperiodicTaskId from the unique name specified for the aperiodic task. 
		\param estimatedTaskName	Sets the value of #m_estimatedTaskId from the unique name specified for the estimation. */
	FleeingTraversalConfig(TimeManager& timeMgr, const char* aperiodicTaskName, const char* estimatedTaskName=KY_NULL)
		:AstarTraversalConfig(timeMgr, aperiodicTaskName, estimatedTaskName)
	{}

	/*!	\label_constructor 
		\param aperiodicTaskId		Sets the value of #m_aperiodicTaskId. 
		\param estimatedTaskId		Sets the value of #m_estimatedTaskId. */
	FleeingTraversalConfig(TimeManager::TaskId aperiodicTaskId, TaskRegistry::TaskId estimatedTaskId=TaskRegistry::InvalidTaskId)
		:AstarTraversalConfig(aperiodicTaskId, estimatedTaskId)
	{}

	virtual GraphTraversal* CreateGraphTraversal(Kaim::TraversalManager& traversalManager) const
	{ 
		FleeingTraversal* fleeingTraversal = KY_NEW_CLASS(FleeingTraversal);
		fleeingTraversal->Initialize(traversalManager, *this);
		return fleeingTraversal;
	}

	virtual FleeingTraversalConfig* Clone() const { return KY_NEW_CLASS(FleeingTraversalConfig)(*this); }
};


} // namespace Kaim

#endif // KY_FLEEINGTRAVERSALCONFIG_H
