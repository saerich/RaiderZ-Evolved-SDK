/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KY_ITHREADINGBRIDGE_H
#define KY_ITHREADINGBRIDGE_H

#include "kypathdata/generic/types.h"

// ps3 specific
class CellSpurs;

namespace Kaim
{
#if defined(KY_PS3)
	//#include <sys/sys_types.h>
	//typedef void (*threadfunc) (__CSTD uint64_t);
	typedef void (*threadfunc) (unsigned long);

#elif defined(KY_LINUX_ANY)
	typedef void* (*threadfunc) (void*);

#elif defined(KY_WII)
	typedef void* (*threadfunc) (void*);

#else
	typedef unsigned int (__stdcall*threadfunc) ( void * );
#endif

#if defined(KY_WINDOWS_ANY) || defined(KY_XBOX_ANY)
#define KY_THREAD_MAIN unsigned __stdcall ThreadMain( void* arg )
#elif defined(KY_PS3)
#define KY_THREAD_MAIN void ThreadMain( unsigned long arg )
#else
#define KY_THREAD_MAIN void* ThreadMain( void* arg )
#endif

#if defined(KY_PS3)
#define KY_THREAD_MAIN_EXIT(x) return;
#else
#define KY_THREAD_MAIN_EXIT(x) return x;
#endif


	/*!	Abstract definition for a thread, for use by classes that implement IThreadingBridge.
		\ingroup kypathdata_bridge */
	class IThread
	{
	protected:
		/*!	\label_constructor. */
		IThread() {}
		
		/*!	\label_destructor. */
		virtual ~IThread() {}
	};

	/*!	Abstract definition for a mutex, for use by classes that implement IThreadingBridge.
		\ingroup kypathdata_bridge */
	class IMutex
	{
	protected:
		/*!	\label_constructor. */
		IMutex() {}
		
		/*!	\label_destructor. */
		virtual ~IMutex() {}

	public:
		/*!	Locks access to the mutex. */
		virtual bool Lock(KyInt32 timeout = -1) = 0;
		
		/*!	Unlocks access to the mutex. */
		virtual void Unlock() = 0;
	};

	/*!	Defines an interface used by \SDKName components to manage multiple threads and mutexes. 
		\pp The Bridge maintains an instance of a class that implements this interface, making the interface available to other
		\SDKName components. By default, the Bridge uses an instance of the DefaultThreadingBridge class. You can also write
		your own implementation of IThreadingBridge, and assign an instance of your class to the Bridge.
		\pp See :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class IThreadingBridge
	{
	public:
		/*!	\label_virtualdestructor */
		virtual ~IThreadingBridge() {}

	public:
		/*!	Initializes a new thread.
			\param func				The function to be called to initialize the thread.
			\param argList			The list of arguments to be passed to \c func.
			\return The handle of the newly created thread. */
		virtual Kaim::IThread* InitThread(threadfunc func, void* argList) = 0;

		/*!	Destroys the specified thread. 
			\param thread			The handle of the thread to be destroyed. */
		virtual void DestroyThread(Kaim::IThread* thread) = 0;

		/*!	Initializes a new mutex.
			\param thisThreadHasOwnership		Indicates whether or not the current thread has access
												to the newly created mutex.
			\return The handle of the newly created mutex. */
		virtual Kaim::IMutex* InitMutex(bool thisThreadHasOwnership) = 0;

		/*!	Destroys the specified mutex.
			\param mutex			The handle to the mutex to be destroyed. */
		virtual void DestroyMutex(Kaim::IMutex* mutex) = 0;

	// ps3 specific
	public:
		/*!	Retrieves a SPURS instance in which the \SDKName tasksets will be created.
			This method is called only once.
			\pp PlayStation 3 only. */
		virtual CellSpurs* GetSpurs() {return KY_NULL;}

		/*!	Releases the SPURS instance retrieved through GetSpurs() when the taskset is no longer
			needed by \SDKName.
			\pp PlayStation 3 only. */
		virtual void ReleaseSpurs(CellSpurs* /*spursInstance*/) {}

	};
} // namespace Kaim

#endif // KY_ITHREADINGBRIDGE_H
