/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KYBRIDGE_H
#define KYBRIDGE_H

#include <assert.h>

#include <kypathdata/generic/types.h>
// include all components (even if a forward declaration is enough), easier to use
#include <kypathdata/bridge/icollisionbridge.h>
#include <kypathdata/bridge/idatareaderbridge.h>
#include <kypathdata/bridge/idrawbridge.h>
#include <kypathdata/bridge/ifileiobridge.h>
#include <kypathdata/generic/ilogbridge.h>
#include <kypathdata/generic/imemorybridge.h>
#include <kypathdata/bridge/iperfcounterbridge.h>
#include <kypathdata/bridge/isocketbridge.h>
#include <kypathdata/bridge/itimerbridge.h>
#include <kypathdata/bridge/ithreadingbridge.h>
#include <kypathdata/bridge/default/deterministictimer.h> 

namespace Kaim
{
	/*!	This class represents the runtime bridge between the \SDKName BaseSystem (including all components running
		within the Engine) and its operating platform: the operating system and the \gameOrSim engine. 
		\pp The BaseSystem creates and maintains an instance of the Bridge class. You can
		access this instance at any time by calling GetBaseSystem().GetBridge().
		\pp The Bridge offers a consistent set of interfaces used by \SDKName components for accessing services provided by
		the operating system and the \gameOrSim engine -- for example, allocating memory, working with files,
		drawing debug information, etc. Each of these interfaces is represented by a different base class -- 
		for example, IMemoryBridge, IFileIOBridge, IDrawBridge, etc.
		\pp By default, the Bridge uses pre-written implementations of many of these interfaces. These default 
		implementations are suitable for use in most cases, but you may wish to replace them with custom implementations 
		of your own. For example, the default implementation of IMemoryBridge uses \c malloc and \c free to allocate 
		and de-allocate memory. If you want \SDKName components to use a different memory allocation scheme, you can 
		create your own implementation of the IMemoryBridge interface, and replace the default implementation used 
		by the Bridge by calling GetBaseSystem().GetBridge().ReplaceMemoryBridge().
		\pp Three interfaces of the Bridge do not have default implementations: IDrawBridge, ICollisionBridge, and
		IPerfCounterBridge. These
		interfaces are too tightly coupled to the working of your \gameOrSim engine to have default implementations
		pre-written for them. In order to provide \SDKName components with the ability to draw debugging information
		and conduct collision tests directly in your \gameOrSim, you must write your own implementations of these
		interfaces, and set them in the Bridge by calling GetBaseSystem().GetBridge().SetDrawBridge() and 
		GetBaseSystem().GetBridge().SetCollisionBridge().
		\pp In addition, ISocketBridge is used only by the remote debugging framework. Default implementations are therefore
		supplied only for the platforms supported by that framework.
		\pp See also :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class Bridge
	{
	public:
		/*!	\label_constructor. For internal use. */
		Bridge();

	public:
		/*!	Sets the implementation of ICollisionBridge maintained by the Bridge. 
			\param bridge				An instance of a class that implements ICollisionBridge, or #KY_NULL
										to disable all collision testing against the \gameOrSim engine. */
		void SetCollisionBridge(ICollisionBridge* bridge)
		{
			m_collisionBridge = bridge;
		}

		/*!	Retrieves the implementation of ICollisionBridge maintained by the Bridge. */
		ICollisionBridge* GetCollisionBridge()
		{
			return m_collisionBridge;
		}

		/*!	Indicates whether or not an implementation of ICollisionBridge has been set for the Bridge. */
		bool HasCollisionBridge() const
		{
			return !(m_collisionBridge == KY_NULL);
		}

	public:
		/*!	Sets the implementation of IDrawBridge maintained by the Bridge. 
			\param bridge				An instance of a class that implements IDrawBridge, or #KY_NULL
										to disable all debug drawing within the \gameOrSim engine. */
		void SetDrawBridge(IDrawBridge* bridge)
		{
			m_drawBridge = bridge;
		}

		/*!	Retrieves the implementation of IDrawBridge maintained by the Bridge. */
		IDrawBridge* GetDrawBridge()
		{
			return m_drawBridge;
		}

		/*!	Indicates whether or not an implementation of IDrawBridge has been set for the Bridge. */
		bool HasDrawBridge() const
		{
			return !(m_drawBridge == KY_NULL);
		}

	public:
		/*!	Replaces the implementation of IFileIOBridge maintained by the Bridge with the specified
			object. 
			\param bridge					An instance of a class that implements IFileIOBridge.
			\retval Result::OK				Indicates that the IFileIOBridge was successfully set.
			\retval Result::BadParameter	Indicates that the \c bridge parameter was #KY_NULL. */
		KyResult ReplaceFileIOBridge(IFileIOBridge* bridge);

		/*!	Retrieves the implementation of IFileIOBridge maintained by the Bridge. */
		IFileIOBridge& GetFileIOBridge()
		{
			assert(m_fileIOBridge!=KY_NULL); //can't be null, except if the bridge's design changes
			return *m_fileIOBridge;
		}

	public:
		/*!	Replaces the implementation of ILogBridge maintained by the Bridge with the specified
			object. 
			\param bridge					An instance of a class that implements ILogBridge.
			\retval Result::OK				Indicates that the ILogBridge was successfully set.
			\retval Result::BadParameter	Indicates that the \c bridge parameter was #KY_NULL. */
		KyResult ReplaceLogBridge(ILogBridge* bridge);

		/*!	Retrieves the implementation of ILogBridge maintained by the Bridge. */
		ILogBridge& GetLogBridge()
		{
			ILogBridge* logBridge = ILogBridge::GetInstance();
			assert(logBridge!=KY_NULL); //can't be null, except if the bridge's design changes
			return *logBridge;
		}

	public:
		/*!	Replaces the implementation of IMemoryBridge maintained by the Bridge with the specified
			object. 
			\pp Note that it is not generally recommended that you call this method at any time after
			you initialize the \SDKName BaseSystem. During the initialization of the BaseSystem, and during
			the subsequent initialization of other \SDKName components, the IMemoryBridge instance currently
			set up for the Bridge will be used to allocate memory. If you substitute another implementation
			of IMemoryBridge after this time, your substitute implementation will be called to free that memory.
			This may cause memory leaks or other unexpected behavior.
			\param bridge					An instance of a class that implements IMemoryBridge.
			\retval Result::OK				Indicates that the IMemoryBridge was successfully set.
			\retval Result::BadParameter	Indicates that the \c bridge parameter was #KY_NULL. */
		KyResult ReplaceMemoryBridge(IMemoryBridge* bridge);

		/*!	Retrieves the implementation of IMemoryBridge maintained by the Bridge. */
		IMemoryBridge& GetMemoryBridge()
		{
			IMemoryBridge* memoryBridge = IMemoryBridge::GetInstance();
			assert(memoryBridge!=KY_NULL); //can't be null, except if the bridge's design changes
			return *memoryBridge;
		}

	public:
		/*!	Sets the implementation of IPerfCounterBridge maintained by the Bridge. 
			\param bridge				An instance of a class that implements IPerfCounterBridge, or #KY_NULL
										to disable sending performance counters to the \gameOrSim engine. */
		void SetPerfCounterBridge(IPerfCounterBridge* bridge)
		{
			m_perfCounterBridge = bridge;
		}

		/*!	Retrieves the implementation of IPerfCounterBridge maintained by the Bridge. */
		IPerfCounterBridge* GetPerfCounterBridge()
		{
			return m_perfCounterBridge;
		}

		/*!	Indicates whether or not an implementation of IPerfCounterBridge has been set for the Bridge. */
		bool HasPerfCounterBridge() const
		{
			return !(m_perfCounterBridge == KY_NULL);
		}

	public:
		/*!	Replaces the implementation of ISocketBridge maintained by the Bridge with the specified
			object. 
			\param bridge					An instance of a class that implements ISocketBridge.
			\retval Result::OK				Indicates that the ISocketBridge was successfully set.
			\retval Result::BadParameter	Indicates that the \c bridge parameter was #KY_NULL. */
		KyResult ReplaceSocketBridge(ISocketBridge* bridge);

		/*!	Retrieves the implementation of ISocketBridge maintained by the Bridge. */
		ISocketBridge& GetSocketBridge()
		{
			assert(m_socketBridge!=KY_NULL); //can't be null, except if the bridge's design changes
			return *m_socketBridge;
		}

	public:
		/*!	Replaces the implementation of IThreadingBridge maintained by the Bridge with the specified
			object. 
			\param bridge					An instance of a class that implements IThreadingBridge.
			\retval Result::OK				Indicates that the IThreadingBridge was successfully set.
			\retval Result::BadParameter	Indicates that the \c bridge parameter was #KY_NULL. */
		KyResult ReplaceThreadingBridge(IThreadingBridge* bridge);

		/*!	Retrieves the implementation of IThreadingBridge maintained by the Bridge. */
		IThreadingBridge& GetThreadingBridge()
		{
			assert(m_threadingBridge!=KY_NULL); //can't be null, except if the bridge's design changes
			return *m_threadingBridge;
		}

	public:
		/*!	Replaces the implementation of ITimerBridge maintained by the Bridge with the specified
			object. 
			\param bridge					An instance of a class that implements ITimerBridge.
			\retval Result::OK				Indicates that the ITimerBridge was successfully set.
			\retval Result::BadParameter	Indicates that the \c bridge parameter was #KY_NULL. */
		KyResult ReplaceTimerBridge(ITimerBridge* bridge);

		/*!	Retrieves the implementation of ITimerBridge maintained by the Bridge. */
		ITimerBridge& GetTimerBridge()
		{
			//this doesn't break the constness, and ensure that if the const version of this function changes, this funciton will change as well:
			// 1- it adds const to 'this' in order to call the const version of this function
			// 2- it removes the constness on the return type.
			return const_cast<ITimerBridge&>(  //1- remove const on the return type
			static_cast<const Bridge*>(this) //2a- add const to the type of 'this' type 
			->GetTimerBridge() ); //2b-in order to call the const function
		}

		/*!	Retrieves the implementation of ITimerBridge maintained by the Bridge. */
		const ITimerBridge& GetTimerBridge() const
		{
			assert(m_timerBridge!=KY_NULL); //can't be null, except if the bridge's design changes
			return *m_timerBridge;
		}

	public:
		/*!	Retrieves the instance of DefaultTimerBridge created by the Bridge at initialization time. */
		ITimerBridge& GetDefaultTimerBridge();

		/*!	Retrieves the instance of DeterministicTimer created by the Bridge at initialization time. */
		DeterministicTimer& GetDeterministicTimerBridge();

		/*!	Retrieves the instance of DeterministicTimer created by the Bridge at initialization time. */
		const DeterministicTimer& GetDeterministicTimerBridge() const;

	private:
		ICollisionBridge* m_collisionBridge;
		IDrawBridge* m_drawBridge;
		IFileIOBridge* m_fileIOBridge;
		IPerfCounterBridge* m_perfCounterBridge;
		ISocketBridge* m_socketBridge;
		IThreadingBridge* m_threadingBridge;
		ITimerBridge* m_timerBridge;
	};

} // namespace Kaim


#endif //KYBRIDGE_H
