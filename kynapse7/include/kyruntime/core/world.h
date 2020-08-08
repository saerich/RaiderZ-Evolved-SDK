/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_WORLD_H
#define KY_WORLD_H

#include <kyruntime/core/bot.h>
#include <kyruntime/core/global.h>
#include <kyruntime/core/metaclass.h>
#include <kyruntime/core/engine.h>
#include <kyruntime/core/timemanager.h>

#include <kyruntime/graph/databasemanager.h>
#include <kyruntime/graph/traversalmanager.h>
#include <kypathdata/generic/memory.h>
#include <kyruntime/graph/astarcontext.h>


namespace Kaim
{

class Engine;
class WorldService;
class WorldServiceClass;
class DatabaseManager;
class Team;
class TimeManager;
class World;
class PathObject;
class IWorldObserver;

/*!	This macro must be called in the declaration of any World class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_DECLARE_WORLD(ky_class) \
	static Kaim::WorldClass Class; \
	static Kaim::World* KyCreate(Kaim::Engine& engine); \
	virtual Kaim::WorldClass&  GetClass() const { return ky_class::Class; }

/*!	This macro must be called in the implementation of any World class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_IMPLEMENT_WORLD(ky_class,ky_base_class) \
	Kaim::WorldClass ky_class::Class(#ky_class,ky_class::KyCreate,(Kaim::WorldClass*)ky_base_class); \
	Kaim::World* ky_class::KyCreate(Kaim::Engine& engine) \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::World* ptr = (Kaim::World*) KY_NEW_CLASS(ky_class)(engine); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from World. Each class that derives from 
	World must contain a constructor function that matches this type definition, and that passes
	its arguments on to the constructor function of its parent class. 
	\param engine			A reference to the Engine that will manage this World.
	\ingroup kyruntime_core */
typedef World* (*WorldConstructor)(Kaim::Engine& engine);

/*!	WorldClass is the MetaClass for World, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_core */
class WorldClass: public MetaClass<World, WorldConstructor>
{
public:
	/*!	Constructor for the World MetaClass. For internal use. */
	WorldClass(const char* className, WorldConstructor ctr, WorldClass* baseClass = KY_NULL);
};


/*!	This class encapsulates a single World: a runtime container for \SDKName components such as \Bots,
	\Bodies, \Obstacles, \PathObjects, \Teams, and \WorldServices. 
	For a general discussion, see :USERGUIDE:"The Runtime Environment". For information on setting up \Worlds in your
	own \gameOrSim, see :USERGUIDE:"Integrating Kynapse".
	\section writingcustomworld Writing a custom World class
	If you need to extend the functionality offered by the ready-to-use World class, you can write your own custom 
	class that derives from World. This involves the following considerations.
	\subsection worldconst Constructor
	All classes that derive from World must have a constructor that follows the #WorldConstructor type definition. 
	\subsection worldvm Virtual methods
	You may re-implement any or all virtual methods of the World class.
	\subsection worldmetaclass MetaClass macros
	In order for \SDKName to recognize your World class, you must include the following macro definitions:
	-	#KY_DECLARE_WORLD: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_WORLD: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from World.
			For example, &Kaim::CMyWorld::Class. If your new class derives directly from World,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_core */
class World: public VirtualMemObject
{
public:
	KY_DECLARE_WORLD(World);

	typedef List<IWorldObserver*> ObserverContainer;
public:

	/*! \label_constructor
		\param engine		The Engine that will contain this World. */
	explicit World(Engine& engine);

	/*!	\label_virtualdestructor */
	~World();

	/*!	\label_init
		\param maxAperiodicTask			The maximum number of aperiodic tasks that can be added to the TimeManager for this World.
		\param maxPeriodicTask			The maximum number of periodic tasks that can be added to the TimeManager for this World.
		\param globalTpf				The maximum amount of time per frame during which the TimeManager will allow components in this
										World to carry out registered aperiodic tasks. See :USERGUIDE:"Deferring and Time-Slicing Tasks". */
	KyResult Initialize(KyUInt32 maxAperiodicTask = 0, KyUInt32 maxPeriodicTask = 0, KyFloat32 globalTpf = 0.f);

	/*!	Updates all components in the World.
		\pp This method is called transparently by Engine::Update(). You typically do not need to call it directly.
		\pp This method:
		-	Stops all asynchronous processing modules configured for the World, if they have not already been stopped.
		-	Processes all requests to add and remove \Bots and \Bodies made since the last update.
		-	Sorts all \Bots in the World by priority. A Bot is automatically assigned a higher priority when the TimeManager 
			denies a request from the Bot for CPU time to carry out a task (such as calculating a new path), or when the Bot 
			is denied access to a shared resource (such as a GraphTraversal). This system of priorities helps to ensure that 
			the CPU time available to \SDKName is split equally among all \Bots. See also :USERGUIDE:"Managing Bot Priorities".
		-	Calls WorldService::Update() for each WorldService that has been added to the World.
		-	Calls Bot::Update() for each Bot in the World, in order of priority.
		-	Calls WorldService::PostUpdate() for each WorldService that has been added to the World.
		\param deltaTime			The simulation time step, in seconds. */
	virtual void Update(KyFloat32 deltaTime = 0);

	void UpdateServices();
	void UpdateBots();
	void PostUpdateServices();

	/*!	\label_reinit Calls the Service::ReInit() method of each WorldService 
		and the Bot::ReInit() method of each Bot. */
	virtual void ReInit();

	/*!	\label_clear */
	virtual void Clear();

	////////////////////////////////////////////////////////////////////////////////////////
	// Body and Bot management
	////////////////////////////////////////////////////////////////////////////////////////

	/*! Retrieves the first Bot in the World whose name matches the specified array of characters.
		\param name		The name of the Bot to retrieve.
		\return A pointer to the Bot, or #KY_NULL if no Bot matches the specified name. */
	Bot* GetBotByName(const char* name);

	/*! Retrieves the first Body in the World that is not controlled by a Bot, and whose name matches the specified array of characters.
		\param name		The name of the Body to retrieve.
		\return A pointer to the Body, or #KY_NULL if no Bot matches the specified name. */
	Body* GetErrantBodyByName(const char* name);

	/*! Retrieves the first Body in the World whose name matches the specified array of characters, regardless of whether or not
		that Body is controlled by a Bot. 
		\param name		The name of the Body to retrieve.
		\return A pointer to the Body, or #KY_NULL if no Bot matches the specified name. */
	Body* GetBodyByName(const char* name);

	/*! Registers an instance of a class that derives from IWorldObserver, which will be notified each time a Body is added
		to or removed from this World. */
	void RegisterWorldObserver(IWorldObserver& observer);
	
	/*! Unregisters the specified IWorldObserver, if it was previously registered in a call to RegisterWorldObserver(). */
	void UnregisterWorldObserver(IWorldObserver& observer);



	////////////////////////////////////////////////////////////////////////////////////////
	// Teams management
	////////////////////////////////////////////////////////////////////////////////////////
	/*!	Creates a new Team in the World. 
		\param teamName			A unique name for the new Team. If #KY_NULL, a name is generated automatically.
		\return A pointer to the newly created Team, or #KY_NULL if an error occured. */
	template <class TTeam>
	TTeam* CreateTeam(const char* teamName)
	{
		TTeam* team = KY_NEW_CLASS(TTeam)(*this, teamName);
		team->m_id = m_teams.Insert(team);
		return team;
	}

	/*!	Destroys the specified Team.
		\param team				The Team to destroy. */
	void DestroyTeam(Team* team);


	////////////////////////////////////////////////////////////////////////////////////////
	// Services management
	////////////////////////////////////////////////////////////////////////////////////////

	/*!	Retrieves a pointer to the first WorldService whose MetaClass matches or 
		derives from the specified MetaClass.
		\param cls		The MetaClass of the WorldService to retrieve.
		\return A pointer to the WorldService, or #KY_NULL if none match the specified MetaClass. */
	WorldService* GetService(WorldServiceClass& cls);

	/*!	\copydoc GetService(WorldServiceClass&) */
	const WorldService* GetService(WorldServiceClass& cls) const;

	/*!	Retrieves a pointer to the first WorldService whose class matches or is 
		inherited from the specified class.
		\tparam T		The class of the WorldService to retrieve.
		\return A pointer to the WorldService, or #KY_NULL if none match the specified class. */
	template < class T >
	T* GetService() { return static_cast<T*>(GetService(T::Class)); }

	/*!	\copydoc GetService() */
	template < class T >
	const T* GetService() const { return static_cast<const T*>(GetService(T::Class)); }

	/*!	Retrieves a pointer to the first WorldService whose MetaClass matches the specified 
		MetaClass. This method is more efficient than GetService(), but requires an exact match.
		\param cls		The MetaClass of the WorldService to retrieve.
		\return A pointer to the WorldService, or #KY_NULL if none match the specified MetaClass. */
	WorldService* GetExactService(WorldServiceClass& cls);

	/*!	Retrieves a pointer to the first WorldService whose class matches the specified 
		class. This method is more efficient than GetService(), but requires an exact match.
		\tparam T		The class of the WorldService to retrieve.
		\return A pointer to the WorldService, or #KY_NULL if none match the specified class. */
	template < class T >
	T* GetExactService() { return static_cast<T*>(GetExactService(T::Class)); }


	/*!	Creates a new WorldService of the specified MetaClass. The World is responsible for managing the lifespan of this new object.
		\param cls		The MetaClass of the WorldService to create.
		\return A pointer to the newly created WorldService, or #KY_NULL if a WorldService of that class already exists
		in this World. */
	WorldService* ActivateService(WorldServiceClass& cls);

	/*!	Creates a new WorldService of the specified class. The World is responsible for managing the lifespan of this new object.
		\tparam T		The class of the WorldService to create.
		\return A pointer to the newly created WorldService, or #KY_NULL if a WorldService of that class already exists
		in this World. */
	template < class T >
	T* ActivateService() { return static_cast<T*>(ActivateService(T::Class)); }


	////////////////////////////////////////////////////////////////////////////////////////
	// time management
	////////////////////////////////////////////////////////////////////////////////////////

	/*!	Retrieves the total elapsed time, in seconds, that has been spent updating this World
		since the Engine was opened. */
	KyFloat32 GetAccumulatedTimeInUpdate() const { return m_timeManager.GetCurrentFrameStart(); }

	/*!	Retrieves the elapsed time, in seconds, since the beginning of the last World update. */
	KyFloat32 GetElapsedTimeInUpdate() const;

	/*!	Retrieves the TimeManager instance that controls periodic and aperiodic tasks for the components in this World. */
	TimeManager& GetTimeManager() {	return m_timeManager; }

	/*!	\copydoc GetTimeManager() */
	const TimeManager& GetTimeManager() const { return m_timeManager; }

	////////////////////////////////////////////////////////////////////////////////////////
	// databaseManager
	////////////////////////////////////////////////////////////////////////////////////////

	/*!	Retrieves the DatabaseManager instance that manages the PathData available within this World. */
	DatabaseManager& GetDatabaseManager() {	return m_databaseManager; }

	/*!	\copydoc GetDatabaseManager() */
	const DatabaseManager& GetDatabaseManager() const { return m_databaseManager; }

public:
	/*!	Retrieves the total number of \Bots that have been initialized in this World. */ 
	KyUInt32 GetBotsCount() const
	{
		return m_bots.GetCount();
	}

	/*!	Retrieves the total number of \Bodies that are not controlled by \Bots that have been initialized in this World. */ 
	KyUInt32 GetErrantBodiesCount() const
	{
		return m_bodies.GetCount() - m_bots.GetCount();
	}

	/*!	Retrieves the total number of \Bodies that have been initialized in this World, regardless of whether or not those
		\Bodies are controlled by \Bots. */ 
	KyUInt32 GetBodiesCount() const
	{
		return m_bodies.GetCount();
	}

	/*!	Retrieves the TraversalManager instance that manages the GraphTraversal objects available to components in this World. */
	const TraversalManager& GetTraversalManager() const { return m_traversalManager; }

	/*!	Retrieves the TraversalManager instance that manages the GraphTraversal objects available to components in this World. */
	TraversalManager& GetTraversalManager() { return m_traversalManager; }

	/*!	Retrieves the AstarContext used by the PathFinder and GraphManager in this World to store tempoarary
		data during path calculations. You can use this AstarContext in your own code as well, but in this
		case you should be aware that it will likely conflict with time-sliced pathfinding computations already
		in progress, which will have the effect of resetting the path computation. */
	AstarContext& GetAstarContext() { return m_astarContext; }

protected:
	Engine& m_engine;
	DatabaseManager m_databaseManager;
	TimeManager m_timeManager; 
	TraversalManager m_traversalManager;
	AstarContext m_astarContext;

public:
	typedef List<Body*> BodyPtrList;

public:

	BotPtrList m_bots; /*!< The list of all \Bots in the World. Do not modify. */

public:
	BodyPtrList m_bodies; /*!< The list of all \Bodies in the World. Do not modify. */

	// various flags used for safety checks in functions that should not be used in specific contexts or at specific moments
	KyBool m_isRunning; /*!< #KY_TRUE after the first update until a call to Reset(). Do not modify. */
	KyBool m_isInAsyncMode; /*!< #KY_TRUE if the AsyncManager is configured. Do not modify. */
	KyBool m_isInUpdateLoop; /*!< #KY_TRUE only when updating the World. Do not modify. */
	KyBool m_isInDeferredRequestProcessingLoop; /*!< #KY_TRUE only during the deferred request processing loop. Do not modify. */

	TeamPtrList m_teams; /*!< The list of all \Teams in the World. Do not modify. */

	Array<WorldService*> m_services; /*!< The list of all \WorldServices in the World. Do not modify. */

	List<World*>::Iterator m_engineWorldListIt; /*!< The index of this World in the list of \Worlds maintained by the Engine. Do not modify. */

	KyFloat32 m_updateStartTime; /*!< The time at which the last World update was started, relative to the start of the Engine update. Do not modify. */


protected:

	friend class Bot;
	friend class Body;

public:
	/* deprecated */
	inline KyBool MustUseAsyncManagement() const {return m_isRunning && m_isInAsyncMode && ! m_isInUpdateLoop; }
	
	/*!	\label_getengine */
	const Engine& GetEngine() const	{ return m_engine; }

	/*!	\label_getengine */
	Engine& GetEngine() { return m_engine; }



private:
////////////////////////////////////////////////////////////////////////////////////////
// PathObjects
////////////////////////////////////////////////////////////////////////////////////////
	
	/* Adds a PathObject to the world. For internal use. 
		\param pathObject	Input pathObject
		\return				#KY_SUCCESS if the PathObject could not be added to the world. Reasons could be that 
							PathObject has been already added,or required services are missing. */
	KyResult AddPathObject(IPathObject* pathObject);

	/* Removes a PathObject from the world. */
	void RemovePathObject(IPathObject* pathObject);

////////////////////////////////////////////////////////////////////////////////////////
// Bots
////////////////////////////////////////////////////////////////////////////////////////

private:
	void AddBot(Bot& entity);
	void RemoveBot(Bot& entity);


////////////////////////////////////////////////////////////////////////////////////////
// Bodies
////////////////////////////////////////////////////////////////////////////////////////

private:
	KyResult AddBody(Body& body);
	KyResult RemoveBody(Body& body);

private:
	World& operator=(const World&);

	friend class IPathObject;

private:
	ObserverContainer m_observers;
};

} // namespace Kaim

#endif // KY_WORLD_H
