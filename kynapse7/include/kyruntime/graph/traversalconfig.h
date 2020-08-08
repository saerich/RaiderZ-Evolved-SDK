/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */


#ifndef KY_TRAVERSALCONFIG_H
#define KY_TRAVERSALCONFIG_H

#include <kypathdata/generic/memory.h>
#include <kyruntime/core/timemanager.h>
#include <kypathdata/basesystem/config.h>
#include <kypathdata/basesystem/estimatedtaskregistry.h>

namespace Kaim
{

	class GraphTraversal;
	class TraversalManager;

/*!	GraphTraversalConfig is an abstract base class for a configuration object that contains all information needed to
	create a GraphTraversal, and that the TraversalManager can call to create a new instance of a GraphTraversal and
	retrieve a pointer to the new instance.
	\pp You cannot use this class as-is; you must use a class that derives from GraphTraversalConfig but that is
	specialized for a particular class of GraphTraversal.
	\pp If you create a custom class that derives from GraphTraversal, you will also have to write a custom class that
	derives from GraphTraversalConfig, which should be able to create a new instance of your GraphTraversal class.
	Typically, you will not derive your new class of configuration object directly from GraphTraversalConfig, but rather
	from either AstarTraversalConfig or PropagationGraphTraversalConfig depending on whether your custom GraphTraversal class
	derives from AstarTraversal or PropagationGraphTraversal.
	\pp For additional information, see :USERGUIDE:"Using GraphTraversals".
	\ingroup kyruntime_graph */
class GraphTraversalConfig : public VirtualMemObject
{
public:
	/*!	\label_constructor You should not create instances of this class directly, but you can call this constructor from
		the constructors of your derived classes. If you use this constructor, you must at a minimum configure the 
		#m_aperiodicTaskId class member before attempting to use your object.  */
	GraphTraversalConfig();

	/*!	\label_constructor You should not create instances of this class directly, but you can call this constructor from
		the constructors of your derived classes.
		\param timeMgr				The TimeManager maintained by the World.
		\param aperiodicTaskName	Sets the value of #m_aperiodicTaskId from the unique name of the aperiodic task. 
		\param estimatedTaskName	Sets the value of #m_estimatedTaskId from the unique name of the estimation. Optional. */
	GraphTraversalConfig(TimeManager& timeMgr, const char* aperiodicTaskName, const char* estimatedTaskName=KY_NULL);

	/*!	\label_constructor You should not create instances of this class directly, but you can call this constructor from
		the constructors of your derived classes.
		\param aperiodicTaskId		Sets the value of #m_aperiodicTaskId. 
		\param estimatedTaskId		Sets the value of #m_estimatedTaskId. Optional. */
	GraphTraversalConfig(TimeManager::TaskId aperiodicTaskId, TaskRegistry::TaskId estimatedTaskId=TaskRegistry::InvalidTaskId);
	

	/*!	The TraversalManager calls this method internally to create an instance of the GraphTraversal class that corresponds
		to this GraphTraversalConfig. You should not need to call this method in your own code. However, if you write your own
		class of GraphTraversal and GraphTraversalConfig, you must implement this method to create a new instance of your 
		GraphTraversal class and return a pointer to that instance.
		\param TraversalManager&	The TraversalManager attempting to create the GraphTraversal. */
	virtual GraphTraversal* CreateGraphTraversal(Kaim::TraversalManager&) const =0;
	virtual GraphTraversalConfig* Clone() const = 0;

	/*!	Indicates whether or not this GraphTraversalConfig has been successfully initialized with all the information it needs
		in order for it to create a valid GraphTraversal. */
	virtual KyBool IsValid() const;

	/*!	The unique ID of the aperiodic task that will be used to determine whether or not time remains in the current frame for
		the GraphTraversal to continue propagating. Must be set for all GraphTraversalConfig objects. */
	TimeManager::TaskId m_aperiodicTaskId;
	
	/*!	The unique ID of an estimation that specifies the time taken for the GraphTraversal to evaluate a vertex or edge. For use
		in deterministic time measurement mode only. See :USERGUIDE:"Using Deterministic Time Measurement".*/
	TaskRegistry::TaskId m_estimatedTaskId;
};

} // namespace Kaim

#endif // CONFIG
