/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_asyncmanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ASYNCMANAGER_H
#define KY_ASYNCMANAGER_H

#include <kyruntime/core/service.h>
#include <kypathdata/basesystem/memorylog.h>


namespace Kaim
{

class AsyncModule;
class AsyncModuleClass;


/*!	AsyncManager is an abstract class of WorldService that manages the asynchronous processing 
	of \SDKName calculations in secondary threads or on secondary processing units. The 
	calculations that it handles must be wrapped in instances of classes that derive from AsyncModule.
	\pp Platform-specific implementations of this base class are provided for use on Windows, 
	Xbox 360 and PlayStation 3 platforms.
	\pp For details, see :USERGUIDE:"Asynchronous Processing".
	\ingroup kyruntime_asyncmanagement */
class AsyncManager : public WorldService
{
public:
	/*!	\label_constructor For internal use.
		\param world			The World that contains the AsyncManager. */
	explicit AsyncManager(World& world);
	
	/*!	\label_virtualdestructor For internal use.*/
	virtual ~AsyncManager();

	/*!	\label_reinit
		\pp This implementation of WorldService::ReInit() calls the AsyncModule::ReInit() method of
		each AsyncModule managed by the AsyncManager. */
	virtual void ReInit();

	/*!	Adds a new AsyncModule of the specified class to the list of modules managed by the AsyncManager.
		\tparam TAsyncModule		The name of a class that derives from AsyncModule. The AsyncManager
									transparently creates a new instance of this class and manages its lifespan.
		\return A pointer to the newly created instance of the TAsyncModule class. */
	template <class TAsyncModule>
	TAsyncModule* AddModule()
	{
		KY_LOG_MEMORY("AsyncManager");
		TAsyncModule* asyncModule = KY_NEW_CLASS(TAsyncModule)(*this);
		AddModule(asyncModule);
		return asyncModule;
	}


	/*!	Terminates all \AsyncModules maintained by the AsyncManager. This method is called
		transparently when the AsyncManager is destroyed. */
	void DeInit();

	virtual void Update() {}

	/*!	Temporarily pauses all \AsyncModules in the World that contains the AsyncManager, until the end of the 
		next update of the Engine. See also Engine::CompleteAsyncServices().
		\pp Use this method with caution: it may block the calling thread for several milliseconds
		while it stops each AsyncModule. */
	void CompleteAsyncProcessing();

	/*!	Re-starts the execution of all \AsyncModules in the World that contains the AsyncManager. */
	void StartAsyncProcessing();

	/*!	Indicates whether or not the asynchronous processing phase has been started. */
	KyBool IsRunning() const { return m_asyncRunning; };

public:
	//-------------------------------------------------
	// Public interface called by users
	//-------------------------------------------------

	/*!	Retrieves a pointer to the first AsyncModule whose class matches the specified class.
		\tparam TAsyncModule	The class name of the module to retrieve.
		\param exactType		Determines whether the AsyncModule to be retrieved must match the specified
								class exactly (#KY_TRUE), or whether it may be derived from the specified
								class (#KY_FALSE, the default). 
		\return A pointer to the AsyncModule instance, or #KY_NULL if none match the specified class. */
	template <class TAsyncModule>
	TAsyncModule* GetModule(KyBool exactType = KY_FALSE) const
	{
		return static_cast<TAsyncModule*>(this->GetModule(TAsyncModule::Class, exactType));
	}

	/*!	Returns the time spent to update all \AsyncModules during the last asynchronous phase.
		This value is updated at the end of each asynchronous phase. */
	inline KyFloat32 GetLastElapsedTime() const { return m_lastElapsedTime; }

protected:
	/*! Retrieves a pointer to the first AsyncModule whose MetaClass matches the specified MetaClass.
		\param cls 			The MetaClass of the AsyncModule to retrieve.
		\param exactType	Determines whether the AsyncModule to be retrieved must match the specified
							MetaClass exactly (#KY_TRUE), or whether it may be derived from the specified
							MetaClass (#KY_FALSE, the default). 
		\return A pointer to the AsyncModule, or #KY_NULL if none match the specified MetaClass. */
	AsyncModule* GetModule(const AsyncModuleClass& cls, KyBool exactType) const;

	//-------------------------------------------------
	// Specific virtual functions for AsyncManagers
	//-------------------------------------------------

	/*!	Called automatically by Engine::StartAsyncServices() in order to begin 
		asynchronous processing after the main Engine update. */
	virtual void BeginAsyncProcessing_Internal() = 0;

	/*!	This method contains the main update of the asynchronous frame. Classes that derive from
		AsyncManager must call this method. */
	virtual void MainAsyncProcessing();

	/*!	Called automatically by Engine::CompleteAsyncServices() in order to end 
		asynchronous processing for the duration of the main Engine update. */
	virtual void EndAsyncProcessing_Internal() = 0;

	//-------------------------------------------------
	// Internal use
	//-------------------------------------------------

	/*!	Adds the specified AsyncModule to the list of \AsyncModules maintained by the AsyncManager. For internal use. */
	void AddModule(AsyncModule* module);

	AsyncModule** m_modules; /*!< The list of all \AsyncModules maintained by the AsyncManager. Do not modify directly. */
	KyUInt32 m_moduleCount; /*!< The number of \AsyncModules currently maintained by the AsyncManager. Do not modify. */
	KyUInt32 m_moduleSize; /*!< For internal use. Do not modify. */

	KyBool m_asyncRunning; /*!< Indicates whether or not the asynchronous processing phase has been started. Do not modify. */

	static KyInt32 s_asyncManagerCount; /*!< The number of instances of the AsyncManager in this World. Currently may not be more than 1, in order to prevent potential thread conflicts. Do not modify. */

	KyFloat32 m_lastElapsedTime; /*!< Total time spent during the last asynchronous phase. Do not modify. */

public:
	static WorldServiceClass Class; // This helps for filtering

public:
	volatile bool m_asyncIsProcessing; /*!< Indicates whether or not any AsyncModule is currently carrying out computations. */
};

} // namespace Kaim

#endif // KY_ASYNCMANAGER_H
