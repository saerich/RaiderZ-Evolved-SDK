/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



/*! \file
	\ingroup kypathdata_basesystem */

#ifndef KyPathData_PerfCounterManager_H
#define KyPathData_PerfCounterManager_H


#include "kypathdata/basesystem/perfcounter.h"

namespace Kaim
{

typedef ThreadLocalStorageWrapper<PerfCounterManager*> TlsPerfCounterManager;

/*	Used internally by the BaseSystem to store and manage performance counters. */
class PerfCounterManager
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	enum Id
	{
		FrameIndex = 0,
		FrameElapsedTime = 1
	};

	static PerfCounterManager& GetInstance()
	{
		TlsPerfCounterManager& tlsPerfCounterManager = GetTls();
		PerfCounterManager* perfCounterManager = tlsPerfCounterManager.Get();
		if (perfCounterManager != KY_NULL)
			return *perfCounterManager;

		perfCounterManager = KY_NEW_CLASS(PerfCounterManager);
		tlsPerfCounterManager.Set(perfCounterManager);
		return *perfCounterManager;
	}

	static void TerminateInstance()
	{
		TlsPerfCounterManager& tlsPerfCounterManager = GetTls();
		PerfCounterManager* perfCounterManager = tlsPerfCounterManager.Get();
		if (perfCounterManager == KY_NULL)
			return;

		TerminateExplicitInstance(perfCounterManager);
		tlsPerfCounterManager.Set(KY_NULL);
	}

	static TlsPerfCounterManager& GetTls()
	{
		static TlsPerfCounterManager tlsPerfCounterManager;
		return tlsPerfCounterManager;
	}

	static void TerminateExplicitInstance(PerfCounterManager* instance)
	{
		KY_DELETE_CLASS(instance);
	}

public:
	static const KyUInt32 capacity = 64;

public:
	bool IsInside(KyUInt32 index) const
	{
		return index < capacity;
	}

public:
	bool IsEnabled(KyUInt32 index) const
	{
		return IsInside(index) && counters[index].IsEnabled();
	}

	KyResult Enable(KyUInt32 index, const char* name)
	{
		if (IsInside(index) == false)
			return Kaim::Result::BadParameter;

		if (name == KY_NULL)
			return Kaim::Result::BadParameter;

		if (counters[index].IsEnabled() == false)
			size++;

		counters[index] = PerfCounter(name);

		return Kaim::Result::Success;
	}

	KyResult Disable(KyUInt32 index)
	{
		if (IsInside(index) == false)
			return Kaim::Result::BadParameter;

		if (counters[index].IsEnabled() == true)
			size--;

		counters[index] = PerfCounter();

		return Kaim::Result::Success;
	}

	KyResult Update(KyUInt32 index, KyFloat32 value)
	{
		if (IsEnabled(index) == false)
			return Kaim::Result::BadParameter;

		counters[index].SetValue(value);

		return Kaim::Result::Success;
	}

protected:
	PerfCounterManager()
		: size(0)
	{
		Enable(FrameIndex, "Frame index");
		Enable(FrameElapsedTime, "Elapsed time in frame");
	}


	PerfCounter counters[capacity];
	KyUInt32 size; // Internal, to speed up remote debugging
};

} // Kaim

#if defined(KY_PERF_COUNTERS_ENABLED)
	/*! Indicates to the IPerfCounterBridge that a block of code has begun that it should track.
		Transparently calls IPerfCounterBridge::BeginEvent(). */
	#define KY_PERF_COUNTERS_BEGIN(name) \
		if (Kaim::GetBaseSystem().GetBridge().HasPerfCounterBridge()) \
			Kaim::GetBaseSystem().GetBridge().GetPerfCounterBridge()->BeginEvent((name))

	/*! Indicates to the IPerfCounterBridge that the currently tracked block of code has been completed.
		Transparently calls IPerfCounterBridge::EndEvent(). */
	#define KY_PERF_COUNTERS_END() \
		if (Kaim::GetBaseSystem().GetBridge().HasPerfCounterBridge()) \
			Kaim::GetBaseSystem().GetBridge().GetPerfCounterBridge()->EndEvent()

	/*! Indicates to the IPerfCounterBridge that a counter should be incremented.
		Transparently calls IPerfCounterBridge::AddCounter(). */
	#define KY_PERF_COUNTERS_UPDATE(index, value) \
	Kaim::PerfCounterManager::GetInstance().Update((index), (KyFloat32)(value))
#else
	#define KY_PERF_COUNTERS_BEGIN(name)
	#define KY_PERF_COUNTERS_END()
	#define KY_PERF_COUNTERS_UPDATE(index, value)
#endif	// KY_PERF_COUNTERS_ENABLED


#endif // KyPathData_PerfCounterManager_H

