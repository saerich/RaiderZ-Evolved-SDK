/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_basesystem */

#include <kypathdata/basesystem/taskregistry.h>

#ifndef KY_ESTIMATEDTASKREGISTRY_H
#define KY_ESTIMATEDTASKREGISTRY_H

namespace Kaim
{

	/*!	This class represents a registry of estimations used in deterministic time measurement mode
		to predict the CPU time that will be consumed by various \SDKName functions.
		\pp When you open the Engine, an instance of this registry is created automatically by
		the BaseSystem, accessible by calling GetBaseSystem().GetEstimatedTasks(). You can retrieve this
		instance in order to provide estimations for the CPU consumption of several runtime
		computations performed by \SDKName components, in order to achieve a more even, deterministic
		behavior across different platforms with different hardware performance.
		\pp See also :USERGUIDE:"Using Deterministic Time Measurement".
		\ingroup kypathdata_basesystem */
	struct EstimatedTaskRegistry : TaskRegistry
	{
		/*!	\label_constructor For internal use. */
		EstimatedTaskRegistry() : TaskRegistry(), m_consumptions(KY_NULL)
		{
		}

		/*!	\label_destructor For internal use. */
		~EstimatedTaskRegistry()
		{
			Clear();
		}

		/*!	\label_init */
		KyResult Initialize(KyUInt32 maxTaskCount);

		/*!	\label_clear */
		void Clear();

		/*!	Sets the estimated CPU consumption for the specified task.
			\param taskId					The numeric ID of the task whose estimation will be set. This ID
											can be retrieved through a call to GetTaskByName().
			\param estimatedConsumption		The time, in seconds, predicted for the task.
			\retval Result::BadParameter	Indicates that \c taskId is not valid. */
		KyResult SetConsumption(TaskId taskId, KyFloat32 estimatedConsumption);

		/*!	Retrieves the estimated CPU consumption set for the specified task. 
			\param taskId					The numeric ID of the task whose estimation will be retrieved. This ID
											can be retrieved through a call to GetTaskByName(). */
		KyFloat32 GetConsumption(TaskId taskId) const;

		/*!	Retrieves the estimated CPU consumption set for the specified task. 
			\param name						The name of the task whose estimation will be retrieved. */
		KyFloat32 GetConsumption(const char* name) const;

	protected:
		KyFloat32* m_consumptions;
	};
}

#endif //KY_ESTIMATEDTASKREGISTRY_H
