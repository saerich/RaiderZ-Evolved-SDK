/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_basesystem */

#ifndef KYBASESYSTEM_H
#define KYBASESYSTEM_H

#include <kypathdata/generic/callstack.h>
#include <kypathdata/bridge/bridge.h>
#include <kypathdata/bridge/default/deterministictimer.h>

namespace Kaim
{
	class BaseComponents;
	class CoordSystem;
	class Profiler;
	class PerfCounterManager;
	class Logger;

	struct MemoryLog;
	struct EstimatedTaskRegistry;

	/*! Provides configuration parameters that govern low-level aspects of the BaseSystem. You must create an instance
		of this class and provide it in a call to BaseSystem::Init().
		\ingroup kypathdata_basesystem */
	struct BaseSystemParameters
	{
		/*!	Enumerates the possible settings for the activation of the \SDKName memory log. 
			See also :USERGUIDE:"The Internal Memory Monitoring System".
			\ingroup kypathdata_basesystem */
		enum EnableMemoryLog { 
			ENABLE_MEMORY_LOG = 0, /*!< Enables the memory logger. */
			DISABLE_MEMORY_LOG = 1,  /*!< Disables the memory logger. */
			DoEnableMemoryLog_FORCE32 = 0xFFFFFFFF 
		};

		/*!	Enumerates the possible settings for the use of the the ISocketBridge::Init() and ISocketBridge::Terminate() functions, which set up
			and terminate the network interface used by the remote visual debugging framework. See also :USERGUIDE:"Remote Visual Debugging".
			\ingroup kypathdata_basesystem */
		enum ManageNetwork { 
			MANAGE_NETWORK = 0, /*!< The BaseSystem will call ISocketBridge::Init() and ISocketBridge::Terminate() automatically to initialize and close the network interface.
									 Use this option if you are using the remote debugging framework in your \gameOrSim and you do not already manage the network interface in your
									 own code. */
			DO_NOT_MANAGE_NETWORK = 1,  /*!< The BaseSystem will not initialize or close the network interface. */
			InitNetwork_FORCE32 = 0xFFFFFFFF 
		};

		/*! \label_constructor */
		BaseSystemParameters() : m_enableMemoryLog(DISABLE_MEMORY_LOG), m_manageNetwork(DO_NOT_MANAGE_NETWORK) {}

		/*! Determines whether or not the BaseSystem activates the internal memory monitoring system.
			See also :USERGUIDE:"The Internal Memory Monitoring System".
			\acceptedvalues	Any element from the #EnableMemoryLog enumeration.
			\defaultvalue	#DISABLE_MEMORY_LOG */
		EnableMemoryLog m_enableMemoryLog;

		/*! Determines whether or not the BaseSystem intializes and terminates the network interface required by the remote
			visual debugging framework. See also :USERGUIDE:"Remote Visual Debugging".
			\acceptedvalues	Any element from the #ManageNetwork enumeration.
			\defaultvalue	#DO_NOT_MANAGE_NETWORK */
		ManageNetwork m_manageNetwork;
	};

	/*!	The BaseSystem class represents a static object that provides the \SDKName Engine and other \SDKName
		components with basic services and data, including:
		-	An instance of the Bridge, which contains a set of interfaces for interacting with the operating system and
			the \gameOrSim engine.
		-	An instance of the CoordSystem, which represents the mapping between the coordinate axes used within \SDKName
			and those used within the \gameOrSim engine.
		-	The Logger message logging manager.
		-	An optional memory logging system that tracks the allocations and de-allocations performed by \SDKName components.
		-	A registry of estimations used in deterministic time measurement mode.
		\pp Only one instance of the BaseSystem can exist at any time. You can
		access this instance at any time by calling the static GetBaseSystem() function.
		You must initialize the BaseSystem by calling its Initialize() method before you can construct an Engine or use
		any other \SDKName components.
		\ingroup kypathdata_basesystem */
	class BaseSystem 
	{
	public:
		/*!	Default constructor. For internal use.
			\post IsInitialized() == false */
		BaseSystem();

		/*!	Destructor. For internal use.
			\pre IsInitialized() == false */
		~BaseSystem();

	public:
		/*!	Initializes the BaseSystem.
			\param parameters				An object that provides configuration parameters for the BaseSystem. The parameter values are copied
											internally, so you do not have to keep this object in memory after the call to Init().
											You can create it on the stack, or delete it from the heap after you call Init().
			\post IsInitialized() == true
			\retval Result::OK				Indicates that the BaseSystem was successfully initialized.
			\retval Result::OutOfMemory		Indicates that insufficient memory remains for the BaseSystem. */
		KyResult Init(const BaseSystemParameters& parameters);

		/*!	Terminates the BaseSystem. For internal use.
			\post IsInitialized() == false */
		void Terminate();

		/*!	Indicates whether or not the BaseSystem has been successfully initialized. */
		bool IsInitialized() const { return m_baseComponents != KY_NULL; }

	public:
		static void PerThreadInit();

		static void PerThreadTerminate();

		class PerThreadInitObject
		{
		public:
			PerThreadInitObject() { PerThreadInit(); };
			~PerThreadInitObject() { PerThreadTerminate(); };
		};

	public:
		/*!	Retrieves the Bridge instance created and maintained by the BaseSystem. */
		Bridge& GetBridge() {return m_bridge;}

		/*!	Retrieves the Bridge instance created and maintained by the BaseSystem. */
		const Bridge& GetBridge() const {return m_bridge;}

	public:
		/*!	Retrieves the CoordSystem instance created and maintained by the BaseSystem.
			\pre IsInitialized() == true */
		CoordSystem& GetCoordSystem();

		/*!	Retrieves the CoordSystem instance created and maintained by the BaseSystem.
			\pre IsInitialized() == true */
		const CoordSystem& GetCoordSystem() const;

	public:
		//////////////////////////////////////////////////////////////////////////////
		// DeterministicTimer
		//////////////////////////////////////////////////////////////////////////////

		/*!	Activates deterministic time measurement mode.
			\pre IsInitialized() == true */
		void ActivateDeterministicTimer()
		{
			ITimerBridge& timer = GetBridge().GetDeterministicTimerBridge();
			GetBridge().ReplaceTimerBridge(&timer);
		}

		/*!	Indicates whether or not deterministic time measurement mode has been activated.
			\pre IsInitialized() == true */
		bool IsTimerDeterministic() const
		{
			const ITimerBridge& timer = GetBridge().GetTimerBridge();
			const ITimerBridge& deterministicTimer = GetBridge().GetDeterministicTimerBridge();
			return &timer == &deterministicTimer;
		}

		/*!	Retrieves the instance of DeterministicTimer created and maintained by the Bridge.
			\pre IsInitialized() == true */
		DeterministicTimer& GetDeterministicTimer()
		{
			return GetBridge().GetDeterministicTimerBridge();
		}

		/*!	Retrieves the instance of DeterministicTimer created and maintained by the Bridge.
			\pre IsInitialized() == true */
		const DeterministicTimer& GetDeterministicTimer() const
		{
			return GetBridge().GetDeterministicTimerBridge();
		}

	public:
		//////////////////////////////////////////////////////////////////////////////
		// EstimatedTasks
		//////////////////////////////////////////////////////////////////////////////

		/*!	Retrieves a registry that contains all estimations configured for deterministic time measurement. 
			\pre IsInitialized() == true */
		const EstimatedTaskRegistry& GetEstimatedTasks() const;

		/*!	Retrieves a registry that contains all estimations configured for deterministic time measurement. 
			\pre IsInitialized() == true */
		EstimatedTaskRegistry& GetEstimatedTasks() ;

	public:
		//////////////////////////////////////////////////////////////////////////////
		// Profiling
		//////////////////////////////////////////////////////////////////////////////

		/*!	Indicates whether or not the performance profiler that handles the #KY_PROFILE macro is enabled. This
			should return \c true for Debug and Verbose builds of the \SDKName libraries.
			\pre IsInitialized() == true */
		bool IsProfilerEnabled() const;

		/*!	Indicates whether or not the performance profiler that handles the #KY_PROFILE macro is ready for use.
			This should return \c true at any time during the main update of any Engine.
			\pre IsInitialized() == true */
		bool IsProfilerReady() const;

	public:
		//////////////////////////////////////////////////////////////////////////////
		// Performance counters
		//////////////////////////////////////////////////////////////////////////////

		/*!	Indicates whether or not performance counters are enabled. 
		\pre IsInitialized() == true */
		bool IsPerfCounterManagerEnabled() const;

	public:
		//////////////////////////////////////////////////////////////////////////////
		// Time facade
		//////////////////////////////////////////////////////////////////////////////

		/*!	Resets the timer.*/
		void ResetTimer()
		{
			m_accumulatedTime = ((m_accumulatedTime < 0) ? 0 : (m_accumulatedTime + GetTime()));
			return GetBridge().GetTimerBridge().Reset();
		}

		/*!	Retrieves the elapsed time (in seconds) since the last call to Reset(). */
		KyFloat32 GetTime() 
		{
			return GetBridge().GetTimerBridge().GetTime();
		}

		/*!	Retrieves the elapsed time (in seconds) since the first call to Reset(). */
		KyFloat32 GetAccumulatedTime() 
		{
			return ((m_accumulatedTime < 0) ? GetTime() : (m_accumulatedTime + GetTime()));
		}

	public:
		/*! Invokes the IMemoryBridge::Allocate() method of the IMemoryBridge instance set up for the Bridge. */
		inline void* Allocate(KyUInt32 size)
		{
			return GetBridge().GetMemoryBridge().Allocate(size);
		}

		/*! Invokes the IMemoryBridge::Free() method of the IMemoryBridge instance set up for the Bridge. */
		inline void DeAllocate(void* address)
		{
			if (address != KY_NULL)
			{
				GetBridge().GetMemoryBridge().Free(address);
			}
		}

		/*! Invokes the IMemoryBridge::AllocateAligned() method of the IMemoryBridge instance set up for the Bridge. */
		inline void* AllocateAligned(KyUInt32 alignement, KyUInt32 size)
		{
			return GetBridge().GetMemoryBridge().AllocateAligned(alignement, size);
		}

		/*! Invokes the IMemoryBridge::FreeAligned() method of the IMemoryBridge instance set up for the Bridge. */
		inline void DeAllocateAligned(void* address)
		{
			if (address != KY_NULL)
			{
				GetBridge().GetMemoryBridge().FreeAligned(address);
			}
		}

	#ifdef KY_MEMORY_LOG_ENABLED
		/*! Invokes the IMemoryBridge::Allocate() method of the IMemoryBridge instance set up for the Bridge.
			If memory logging is enabled for the BaseSystem, also registers the allocation. */
		void* Allocate(KyUInt32 size, const Kaim::Call& context);
		/*! Invokes the IMemoryBridge::Free() method of the IMemoryBridge instance set up for the Bridge.
			If memory logging is enabled for the BaseSystem, also registers the deallocation. */
		void DeAllocate(void* address, const Kaim::Call& context);

		/*! Invokes the IMemoryBridge::AllocateAligned() method of the IMemoryBridge instance set up for the Bridge.
			If memory logging is enabled for the BaseSystem, also registers the allocation. */
		void* AllocateAligned(KyUInt32 alignement, KyUInt32 size, const Kaim::Call& context);
		/*! Invokes the IMemoryBridge::FreeAligned() method of the IMemoryBridge instance set up for the Bridge.
			If memory logging is enabled for the BaseSystem, also registers the deallocation. */
		void DeAllocateAligned(void* address, const Kaim::Call& context);
	#endif // KY_MEMORY_LOG_ENABLED

		/*! Dumps a snapshot of all memory allocations and deallocations that have been made since the BaseSystem was
			initialized, and writes it to the specified file.*/ 
		KyResult MemorySnapshot(const char* filename);

		/*! Sets the BaseSystem to dump a memory snapshot automatically to the specified file when the BaseSystem is terminated. 
			Does nothing if memory logging is diasbled.*/
		KyResult SetAutoMemorySnapshot(const char* filename);

		/*! Sets the BaseSystem to dump a memory snapshot automatically to the specified file when the BaseSystem is terminated. 
			Does nothing if memory logging is diasbled.*/
		KyResult SetAutoMemorySnapshot();

		/*!	Indicates whether or not memory has been activated.*/
		bool IsMemoryLogEnabled()
		{
#if defined(KY_MEMORY_LOG_ENABLED)
			return (m_enableMemoryLog == BaseSystemParameters::ENABLE_MEMORY_LOG);
#else
			return false;
#endif 
		}

	protected:


		Bridge m_bridge;
		BaseComponents* m_baseComponents;
		KyFloat32 m_accumulatedTime;
		BaseSystemParameters::ManageNetwork m_manageNetwork;
		BaseSystemParameters::EnableMemoryLog m_enableMemoryLog;
	};

	// global access to base system
	namespace details
	{
		extern BaseSystem g_baseSystem;
	}

	/*!	Retrieves the instance of BaseSystem currently in use by the Engine.
		\ingroup kypathdata_basesystem */
	inline BaseSystem& GetBaseSystem()
	{
		return details::g_baseSystem;
	}

	/*! Retrieves the CoordSystem owned by the BaseSystem.
		\ingroup kypathdata_basesystem */
	inline CoordSystem& GetCoordSystem() { return GetBaseSystem().GetCoordSystem(); }

} // namespace Kaim


#include <kypathdata/basesystem/coordsystemmacros.inl>


//////////////////////////////////////////////////////////////////////////////
// PS3 - SPURS
//////////////////////////////////////////////////////////////////////////////

#ifdef KY_PS3
	inline CellSpurs* KyGetSpurs() {
		return Kaim::GetBaseSystem().GetBridge().GetThreadingBridge().GetSpurs(); }
	inline void KyReleaseSpurs(CellSpurs* spursInstance) {
		Kaim::GetBaseSystem().GetBridge().GetThreadingBridge().ReleaseSpurs(spursInstance); }
#endif //KY_PS3


#endif //KYBASESYSTEM_H
