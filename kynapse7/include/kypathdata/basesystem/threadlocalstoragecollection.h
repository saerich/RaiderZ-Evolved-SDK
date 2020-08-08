/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_basesystem */

// primary contact: JAPA - secondary contact: GUAL
#ifndef KYTHREAD_LOCAL_STORAGE_COLLECTION_H
#define KYTHREAD_LOCAL_STORAGE_COLLECTION_H

#include <kypathdata/generic/memory.h>

#include <kypathdata/basesystem/memorylog.h>
#include <kypathdata/basesystem/profiler.h>
#include <kypathdata/basesystem/perfcountermanager.h>
#include <kypathdata/basesystem/logger.h>

namespace Kaim
{

struct MemoryLog;
class Profiler;
class PerfCounterManager;
class Logger;

// Class that holds all the TLS related to the BaseSystem for a given thread.
class ThreadLocalStorageCollection
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	ThreadLocalStorageCollection() : 
	m_memoryLog(KY_NULL),
	m_profiler(KY_NULL),
	m_perfCounterManager(KY_NULL),
	m_logger(KY_NULL)
	{}

	~ThreadLocalStorageCollection() {}

	void Init()
	{
		m_memoryLog = &MemoryLog::GetInstance();
		m_profiler = &Profiler::GetInstance();
		m_perfCounterManager = &PerfCounterManager::GetInstance();
		m_logger = Logger::GetInstance();
	}

	void Terminate()
	{
		Profiler::TerminateExplicitInstance(m_profiler);
		PerfCounterManager::TerminateExplicitInstance(m_perfCounterManager);
		Logger::TerminateExplicitInstance(m_logger);
		MemoryLog::TerminateExplicitInstance(m_memoryLog);
	}

private:
	MemoryLog* m_memoryLog;
	Profiler*  m_profiler;
	PerfCounterManager* m_perfCounterManager;
	Logger* m_logger;
};

}
#endif  //KYTHREAD_LOCAL_STORAGE_COLLECTION_H
