/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KY_BASECOMPONENTS_H
#define KY_BASECOMPONENTS_H

#include <kypathdata/generic/callstack.h>
#include <kypathdata/basesystem/estimatedtaskregistry.h>
#include <kypathdata/basesystem/coordsystem.h>
#include <kypathdata/basesystem/profiler.h>
#include <kypathdata/basesystem/perfcountermanager.h>

namespace Kaim
{
	/*	Initializes, maintains and provides access to basic \SDKName objects and services, including:
		-	The \SDKName coordinate system.
		-	The Logger message logging manager.
		-	A registry of estimations used in deterministic time measurement mode.
		\pp One instance of this class is automatically created and maintained by the BaseSystem. Once the
		BaseSystem has been successfully initialized, the components and services provided by the BaseComponents
		class are accessible through the BaseSystem.
		\ingroup kypathdata_basesystem */
	class BaseComponents 
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		/*	For internal use.
			\post IsInitialized() == false */
		BaseComponents() : m_isInitialized(KY_FALSE)
		{
		}

		/* For internal use. */
		~BaseComponents()
		{
			Terminate();
		}

		/*	Initializes BaseComponents. For internal use.
			\post IsInitialized() == true
			\post [KYDEBUG] GetDebug().IsInitialized() == true
			\retval Result::OK				Initialization was successful.
			\retval Result::OutOfMemory		No additional memory could be allocated. */
		KyResult Init();

		/*	Terminates BaseComponents. For internal use.
			\post IsInitialized() == false
			\post GetDebug().IsInitialized() == false
			\post GetEstimatedTasks().IsInitialized() == false */
		void Terminate();

		/*	Indicates whether or not BaseComponents has been successfully initialized. */
		bool IsInitialized() const;

	public:
		/*	Retrieves a registry that contains all estimations configured for deterministic time measurement. */
		const EstimatedTaskRegistry& GetEstimatedTasks() const { return m_estimatedTasks; }
		
		/*	Retrieves a registry that contains all estimations configured for deterministic time measurement. */
		EstimatedTaskRegistry& GetEstimatedTasks() { return m_estimatedTasks; }

	public:
		/*	Retrieves the CoordSystem. */
		const CoordSystem& GetCoordSystem() const { return m_coordSystem; }
		
		/*	Retrieves the CoordSystem. */
		CoordSystem& GetCoordSystem() { return m_coordSystem; }

	public:
		/*	Indicates whether or not the performance profiler is enabled. 
		\pre IsInitialized() == true
		*/
		bool IsProfilerEnabled() const;

	public:
		/*	Indicates whether or not the performance counters are enabled. 
		\pre IsInitialized() == true
		*/
		bool IsPerfCounterManagerEnabled() const;

	protected:
		EstimatedTaskRegistry m_estimatedTasks;
		CoordSystem m_coordSystem;
		KyBool m_isInitialized;
	};
}


#endif //KYBASESERVICES_H
