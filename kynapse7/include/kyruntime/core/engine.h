/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ENGINE_H
#define KY_ENGINE_H

#include <kyruntime/core/global.h>

#include <kypathdata/generic/memory.h>
#include <kypathdata/containers/list.h>
#include <kypathdata/generic/macros.h>
#include <kypathdata/containers/array.h>
#include <kypathdata/basesystem/config.h>

#ifdef KY_PS3
class CellSpurs;
#endif

namespace Kaim
{
class EngineService;
class EngineServiceClass;
class World;
class Bot;
class RemoteDebugging;


/*!	The Engine is the highest-level \SDKName component. It provides a working environment for all other 
	\SDKName components, including a set of interfaces for interacting with the platform and with
	the \gameOrSim engine. 
	\pp You must open the Engine before you can use any other \SDKName components, and you must close the 
	Engine when you no longer need \SDKName AI in your \gameOrSim. Only one instance of the Engine can 
	exist at any time.
	For a general discussion, see :USERGUIDE:"The Runtime Environment". For information on setting up an Engine in your
	own \gameOrSim, see :USERGUIDE:"Integrating Kynapse".
	\ingroup kyruntime_core */
class Engine
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	friend class RemoteDebugging;
	friend class World;
	typedef Array<EngineService*> EngineServices;

public:
	/*! Closes and destroys the specified Engine instance. All \EngineServices owned by the Engine are transparently destroyed,
		but \Worlds contained by the Engine are not. */
	inline static void Destruct(Engine* & engine)
	{
		KY_DELETE_CLASS(engine);
		engine=KY_NULL;
	}

	/*! Creates a new instance of the Engine class.
		\param LicenseString			The customer identification string given to you by Autodesk Support to represent your project.
		\param LicenseKey				The runtime license key given to you by Autodesk Support that matches the \c LicenseString.
		\return A pointer to the newly created Engine, or KY_NULL if the BaseSystem has not been previously initialized or if the 
				specified license credentials are invalid. */
	inline static Engine* Construct(const char* LicenseString, const KyUInt32 LicenseKey)
	{
#if defined(KYDEBUG)
	return Construct_KYDEBUG(LicenseString, LicenseKey);
#elif defined(KYMETRICS)
	return Construct_KYMETRICS(LicenseString, LicenseKey);
#else
	return Construct_KYRELEASE(LicenseString, LicenseKey);
#endif
	}

	/*!	Updates the Engine. This method must be called at each frame.
		\pp Before calling this method, you should ensure that you have set up all your interface objects
		such as \Bots, \Bodies and \Obstacles with the most up-to-date information about their states and
		characteristics in the \gameOrSim engine.
		\pp This method:
		-	Resets the intra-frame timer by calling BaseSystem::ResetTimer().
		-	Calls EngineService::Update() for each EngineService that has been added to the Engine.
		-	Calls World::Update() for each World that has been added to the Engine.
		-	Calls EngineService::PostUpdate() for each EngineService that has been added to the Engine.
		\pp If you use the asynchronous processing framework, you must always stop all asynchronous processing
		by calling CompleteAsyncServices() before you call this method. Call StartAsyncServices() after the
		Update() method has completed in order to re-start asynchronous processing. See also
		:USERGUIDE:"Asynchronous Processing".
		\param deltaTime	The simulation time step in seconds. */
	void Update(KyFloat32 deltaTime);

	/*! Updates all \EngineServices activated for the Engine. Called transparently by the Update() method.
		\param deltaTime	The simulation time step in seconds. */
	void UpdateEngineServices(KyFloat32 deltaTime);

	/*!	Post-updates all \EngineServices activated for the Engine after all \Worlds have been updated. 
		Called transparently by the Update() method. */
	void PostUpdateEngineServices();

	/*! Updates each World in sequence. Called transparently by the Update() method.
		\param deltaTime	The simulation time step in seconds. */
	void UpdateWorlds(KyFloat32 deltaTime);


	/*! Temporarily pauses all \AsyncModules in all \Worlds until the end of the next update step. 
		\pp If you use the asynchronous processing framework, you must call this method before Engine::Update()
		in order to pause \AsyncModules for the duration of the main \SDKName update step. You can also call it
		at any other time that you need to ensure that asynchronous work is complete. 
		\pp Use this method with caution: it may block the calling thread for several milliseconds
		while it stops each module. */
	void CompleteAsyncServices();
	
	/*! Re-starts all \AsyncModules in all \Worlds. 
		\pp If you use the asynchronous processing framework, you must call this method after Engine::Update()
		in order to re-start \AsyncModules until the next \SDKName update step. You can also call it at any other time 
		to re-start asynchronous processing if you previously called CompleteAsyncServices(). */
	void StartAsyncServices();

	/*! Retrieves a pointer to the first EngineService whose MetaClass matches or derives
		from the specified MetaClass.
		\param cls		The MetaClass of the EngineService to retrieve.
		\pre AreEngineServicesCreated() == true
		\return A pointer to the EngineService, or #KY_NULL if none match the specified MetaClass. */
	EngineService* GetService(EngineServiceClass& cls);

	/*! Retrieves a pointer to the first EngineService whose MetaClass matches or derives
		from the specified MetaClass.
		\param cls		The MetaClass of the EngineService to retrieve.
		\pre AreEngineServicesCreated() == true
		\return A pointer to the EngineService, or #KY_NULL if none match the specified MetaClass. */
	const EngineService* GetService(EngineServiceClass& cls) const;

	/*! Retrieves a pointer to the first EngineService whose class matches or derives 
		from the specified class.
		\tparam T		The class of the EngineService to retrieve.
		\pre AreEngineServicesCreated() == true
		\return A pointer to the EngineService, or #KY_NULL if none match the specified class. */
	template < class T >
	T* GetService() { return static_cast<T*>(GetService(T::Class)); }

	/*! Retrieves a pointer to the first EngineService whose class matches or derives 
		from the specified class.
		\tparam T		The class of the EngineService to retrieve.
		\pre AreEngineServicesCreated() == true
		\return A pointer to the EngineService, or #KY_NULL if none match the specified class. */
	template < class T >
	const T* GetService() const { return static_cast<const T*>(GetService(T::Class)); }

	/*! Retrieves a pointer to the first EngineService whose MetaClass matches the specified 
		MetaClass. This method is more efficient than GetService(), but requires an exact match.
		\param cls		The MetaClass of the EngineService to retrieve.
		\pre AreEngineServicesCreated() == true
		\return A pointer to the EngineService, or #KY_NULL if none match the specified MetaClass. */
	EngineService* GetExactService(EngineServiceClass& cls);

	/*! Retrieves a pointer to the first EngineService whose class matches the specified 
		class. This method is more efficient than GetService(), but requires an exact match.
		\tparam T		The class of the EngineService to retrieve.
		\pre AreEngineServicesCreated() == true
		\return A pointer to the EngineService, or #KY_NULL if none match the specified class. */
	template < class T >
	T* GetExactService() { return static_cast<T*>(GetExactService(T::Class)); }

	/*!	Creates a new EngineService of the specified MetaClass. The Engine is responsible for managing the lifespan of this new object.
		\param cls		The MetaClass of the EngineService to create.
		\return A pointer to the newly created EngineService, or #KY_NULL if a EngineService of that class already exists
		in this World. */
	EngineService* ActivateService(EngineServiceClass& cls);

	/*!	Creates a new EngineService of the specified class. The World is responsible for managing the lifespan of this new object.
		\tparam T		The class of the EngineService to create.
		\return A pointer to the newly created EngineService, or #KY_NULL if a EngineService of that class already exists
		in this World. */
	template < class T >
	T* ActivateService()	{ return static_cast<T*>(ActivateService(T::Class)); }

	/*! Retrieves a list of all \Worlds currently maintained by the Engine. */
	const List<World*>& GetWorlds() const
	{
		return m_worlds;
	}

	/*! Retrieves a list of all \Worlds currently maintained by the Engine. */
	List<World*>& GetWorlds()
	{
		return m_worlds;
	}

	/*!	Retrieves a list of all \EngineServices currently maintained by the Engine. */
	const Array<EngineService*>& GetServices() const
	{
		return m_services;
	}


	/*!	Retrieves a pointer to the RemoteDebugging service, if it exists. */
	RemoteDebugging* GetRemoteDebugging();

	/*!	Retrieves the elapsed time, in seconds, since the last call to Engine::Update(). */
	KyFloat32 GetElapsedTimeInFrame() const
	{
		return GetBaseSystem().GetTime();
	}

	/*!	Indicates whether or not the performance profiler is ready for use. */
	bool IsProfilerReady() const;
	
	/*!	Retrieves the performance profiler maintained by the Engine. */
	Profiler& GetProfiler();

private:
	void AddWorld(World& world, List<World*>::Iterator& engineWorldListIt);
	void RemoveWorld(World& world);
	void OnUpdate(KyFloat32 deltaTime);
private:
	Engine();
	~Engine();
private:
	static Kaim::Engine* Construct_KYDEBUG(const char* LicenseString, const KyUInt32 LicenseKey);
	static Kaim::Engine* Construct_KYMETRICS(const char* LicenseString, const KyUInt32 LicenseKey);
	static Kaim::Engine* Construct_KYRELEASE(const char* LicenseString, const KyUInt32 LicenseKey);

private:
	EngineServices m_services; //< the table of all the EngineServices of the engine. This should not be modified.
	List<World*> m_worlds; //< List of the existing worlds.
	RemoteDebugging* m_remoteDebugging; //< The remote debugging session.
	KyUInt32 m_frameIndex;
	KyFloat32 m_lastElapsedTimeInFrame;
};

} // namespace Kaim

#endif // KY_ENGINE_H
