/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_asyncmanagement */

// primary contact: JAPA - secondary contact: NOBODY
#ifndef KY_ASYNCMANAGER_WINDOWS_H
#define KY_ASYNCMANAGER_WINDOWS_H

#if !defined(__GCCXML__)

#include <kyruntime/asyncmanagement/asyncmanager.h>

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>


namespace Kaim
{

/*!	This implementation of AsyncManager uses a secondary thread to manage the execution of 
	\AsyncModules on 32-bit and 64-bit Windows platforms.
	\pp See :USERGUIDE:"Asynchronous Processing".
	\ingroup kyruntime_asyncmanagement */
class AsyncManager_Windows : public AsyncManager
{
public:
	KY_DECLARE_WORLDSERVICE(AsyncManager_Windows)


public:
	/*!	\label_constructor \label_mustinit
		\param world			The World that contains the AsyncManager. */
	explicit AsyncManager_Windows(World& world);
	
	/*!	\label_virtualdestructor */
	virtual ~AsyncManager_Windows();

	/*!	\label_init */
	KyResult Initialize();

	/*!	Returns the thread handle. You can use this handle to manage the priority of
		the asynchronous thread relative to the other threads by use in your application. However, you
		should make sure that all asynchronous processing has been completed before the next time the
		\SDKName Engine is updated. If not, the thread updating the Engine may be blocked for several
		milliseconds while it halts the asynchronous processing. */
	HANDLE GetThreadHandle() { return m_asyncThreadHandle; }


	//-------------------------------------------------
	// Specific virtual functions for AsyncManagers
	//-------------------------------------------------
protected:
	virtual void BeginAsyncProcessing_Internal();
	virtual void EndAsyncProcessing_Internal();
	static unsigned __stdcall AsyncProcessingLoop(void* param);


protected:
	// secondary thread management
	HANDLE				m_asyncThreadHandle;	// async processing management thread
	HANDLE				m_mutex[2];				// thread synchronization mutexes
	KyUInt32			m_mainThreadActiveMutexIdx;	
	KyUInt32			m_secondThreadActiveMutexIdx;	
	KyBool				m_exitThread;			// mutex-ed variable, triggers async thread exit
};

} // namespace Kaim

#endif //__GCCXML__
#endif // KY_ASYNCMANAGER_WINDOWS_H
