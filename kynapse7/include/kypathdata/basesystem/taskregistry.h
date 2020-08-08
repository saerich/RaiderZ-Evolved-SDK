/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_basesystem */

#include <kypathdata/generic/types.h>

#ifndef KY_TASKREGISTRY_H
#define KY_TASKREGISTRY_H

namespace Kaim
{
	/*!	TaskRegistry is an abstract base class that represents a registry of time management
		tasks. It provides a set of base functions for EstimatedTaskRegistry.
		\ingroup kypathdata_basesystem */
	struct TaskRegistry
	{
		/*!	Defines the type used to store a unique ID for each task maintained in a TaskRegistry. */
		typedef KyUInt32 TaskId;

		/*!	Represents an invalid task ID value. */
		static const TaskId InvalidTaskId = KyUInt32MAXVAL;

		/*!	\label_constructor For internal use. */
		TaskRegistry();

		/*!	\label_destructor For internal use. */
		~TaskRegistry()
		{
			Clear();
		}

		/*!	Initializes the TaskRegistry. For internal use. 
			\param maxTaskCount				The maximum number of tasks that can be registered.
			\retval Result::BadParameter	Indicates that \c maxTaskCount is invalid (less than 0).
			\retval Result::OutOfMemory		Indicates that insufficient memory could be allocated.
			\post [maxTaskCount > 0] IsInitialized() == true
			\post IsEmpty() == true */
		KyResult Initialize(TaskId maxTaskCount);

		/*!	Terminates the TaskRegistry. For internal use.
			\post IsInitialized() == false
			\post IsEmpty() == true */
		void Clear();	

		/*!	Indicates whether or not the TaskRegistry has been successfully initialized. */
		bool IsInitialized() const
		{
			return m_maxTaskCount > 0;
		}

		/*!	Indicates whether or not the TaskRegistry is empty: i.e. it contains 0 registered tasks. */
		bool IsEmpty() const
		{
			return 0 == m_count;
		}

		/*!	Indicates whether or not the TaskRegistry is full: i.e. it contains a number of registered
			tasks equal to its maximum capacity. */
		bool IsFull() const
		{
			return m_maxTaskCount == m_count;
		}

		/*!	Indicates whether or not the specified task ID is valid: i.e. it is a valid ID value, and it
			refers to a task currently in the TaskRegistry. */
		bool IsValid(TaskId taskId) const
		{
			return  taskId < m_count;
		}

		/*!	Adds a new task to the TaskRegistry.
			\param name						A unique descriptive name for the new task.
			\retval	If the new task was successfully registered, the unique ID value assigned to the new task.
			\retval	If another task with the same name already exists, the ID of that existing task.
			\retval	If the task could not be registered, #InvalidTaskId. */
		TaskId RegisterTask(const char* name);

		/*!	Retrieves the unique ID value assigned to the task that matches the specified name.
			\param name						The name of the task whose ID will be retrieved.
			\return The ID of the task retrieved, or #InvalidTaskId if no registered task matches the specified name. */
		TaskId GetTaskByName(const char* name) const;

	protected:
		TaskId m_maxTaskCount;
		TaskId m_count;
		char** m_taskNames;
	};
}

#endif //KY_TASKREGISTRY_H
