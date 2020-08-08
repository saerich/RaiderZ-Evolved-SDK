/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_agents */

#ifndef KY_HIDINGTRAVERSALCONFIG_H
#define KY_HIDINGTRAVERSALCONFIG_H

#include <kyruntime/graph/astartraversalconfig.h>
#include <kyruntime/agents/hidingtraversal.h>
#include <kyruntime/core/world.h>

namespace Kaim
{
/*!	The HidingTraversalConfig class allows the TraversalManager to create and serve an instance of the HidingTraversal
	class. 
	\pp This class is closely coupled with the HidingTraversal and the HideAgent. For details on using this class,
	see :USERGUIDE:"Hiding".
	\ingroup kyruntime_agents */
class HidingTraversalConfig : public AstarTraversalConfig
{
public:
	/*!	\label_constructor If you use this constructor, you must at a minimum configure the #m_aperiodicTaskId class member
		before attempting to use your object. */
	HidingTraversalConfig()
		:AstarTraversalConfig()
		,m_pointLockManager(KY_NULL)
	{}

	/*!	\label_constructor 
		\param timeMgr				The TimeManager maintained by the World.
		\param aperiodicTaskName	Sets the value of #m_aperiodicTaskId from the unique name of the aperiodic task. 
		\param estimatedTaskName	Sets the value of #m_estimatedTaskId from the unique name of the estimation.
		\param pointLockManager		Sets the value of #m_pointLockManager. Optional. */
	HidingTraversalConfig(TimeManager& timeMgr, const char* aperiodicTaskName, const char* estimatedTaskName=KY_NULL,
		PointLockManager* pointLockManager=KY_NULL)
		:AstarTraversalConfig(timeMgr, aperiodicTaskName, estimatedTaskName)
		,m_pointLockManager(pointLockManager)
	{}

	/*!	\label_constructor 
		\param aperiodicTaskId		Sets the value of #m_aperiodicTaskId. 
		\param estimatedTaskId		Sets the value of #m_estimatedTaskId.
		\param pointLockManager		Sets the value of #m_pointLockManager. Optional. */
	HidingTraversalConfig(TimeManager::TaskId aperiodicTaskId, TaskRegistry::TaskId estimatedTaskId=TaskRegistry::InvalidTaskId,
		PointLockManager* pointLockManager=KY_NULL)
		:AstarTraversalConfig(aperiodicTaskId, estimatedTaskId)
		,m_pointLockManager(pointLockManager)
	{}

	virtual GraphTraversal* CreateGraphTraversal(Kaim::TraversalManager& traversalManager) const
	{ 
		HidingTraversal* hidingTraversal = KY_NEW_CLASS(HidingTraversal);
		hidingTraversal->Initialize(traversalManager, *this);
		return hidingTraversal;
	}

	virtual HidingTraversalConfig* Clone() const { return KY_NEW_CLASS(HidingTraversalConfig)(*this); }

	/*!	The PointLockManager for the World. If set, the HidingTraversal will use this PointLockManager to avoid choosing destination
		points that are already locked by other \Bodies. */
	PointLockManager* m_pointLockManager;
};


} //Kaim

#endif // KY_HIDINGTRAVERSALCONFIG_H
