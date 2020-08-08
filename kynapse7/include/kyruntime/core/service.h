/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SERVICE_H
#define KY_SERVICE_H

#include <kyruntime/core/metaclass.h>

#include <kypathdata/basesystem/config.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{

class WorldServiceClass;
class EngineServiceClass;
class Bot;
class World;
class Engine;

/*!	BaseService is the base class for all \SDKName services.
	\pp A Service may be maintained at different levels:
	-	EngineService classes are created and maintained by the Engine. They can be accessed by any
		\SDKName component. 
	-	WorldService classes are created and maintained within a single World. They can be accessed
		only by other components within the same World. 
	\ingroup kyruntime_core */
class BaseService: public VirtualMemObject
{
public:
	/*!	\label_virtualdestructor */
	virtual ~BaseService() {}

	/*!	Re-initializes the Service. */
	virtual void ReInit() {}
};



/*!	WorldService is the base class for all services that exist within a single World. Each instance of a WorldService
	can provide calculations and data only for other components within its associated World. Only one instance of each
	WorldService class can exist within a single World.
	\section writingcustomworldservice Writing a custom WorldService class
	Writing a custom class that derives from WorldService involves the following considerations.
	\subsection worldserviceconst Constructor
	All classes that derive from WorldService must have a constructor that follows the #WorldServiceConstructor type definition. 
	\subsection worldservicevm Virtual methods
	You may re-implement any or all virtual methods of the WorldService class.
	\subsection worldservicemetaclass MetaClass macros
	In order for \SDKName to recognize your WorldService class, you must include the following macro definitions:
	-	#KY_DECLARE_WORLDSERVICE: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_WORLDSERVICE: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from WorldService.
			For example, &Kaim::SpawnManager::Class. If your new class derives directly from WorldService,
			the value of this argument should be #KY_NULL.
	\section worldservicesetup Setting up a WorldService
	To set up a WorldService within your World, you typically must:
	-#	Call World::ActivateService(), and provide the class name or MetaClass of the service that you want to set up.
	-#	Call the Initialize() method of your WorldService to configure it for use.
	\pp The ready-to-use classes of WorldService supplied with \SDKName may have other specific requirements. See their class
	descriptions or documentation for details.
	\ingroup kyruntime_core */
class WorldService: public BaseService
{
	WorldService& operator=(const WorldService& );
public:
	/*!	This method is called at each frame, before the \Bots in the World are updated. */
	virtual void Update() {}

	/*!	This method is called at each frame, after the \Bots in the World are updated. */
	virtual void PostUpdate() {}

	/*!	\label_getclass */
	virtual WorldServiceClass&  GetClass() = 0;

	/*!	\label_getworld */
	World& GetWorld() { return m_world; }

	/*!	\label_getworld */
	const World& GetWorld() const  { return m_world; }

	/*!	\label_getengine */
	Engine& GetEngine();

	/*!	\label_getengine */
	const Engine& GetEngine() const;

protected:
	/*!	\label_baseconstructor
	\param world		The World that contains this service. */
	explicit WorldService(World& world);
	World& m_world;
};


/*!	EngineService is the base class for all services that exist at the level of the Engine. They can provide calculations and
	data to any other components in any World running within that Engine. Only one instance of each EngineService class can exist
	within an Engine.
	\section writingcustomengineservice Writing a custom EngineService class
	Writing a custom class that derives from EngineService involves the following considerations.
	\subsection engineserviceconst Constructor
	All classes that derive from EngineService must have a constructor that follows the #EngineServiceConstructor type definition. 
	\subsection engineservicevm Virtual methods
	You may re-implement any or all virtual methods of the EngineService class.
	\subsection engineservicemetaclass MetaClass macros
	In order for \SDKName to recognize your EngineService class, you must include the following macro definitions:
	-	#KY_DECLARE_BOTSERVICE: Place this macro in the declaration of your new class. This macro takes one argument: the 
	-	#KY_IMPLEMENT_BOTSERVICE: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from EngineService.
			For example, &Kaim::RemoteDebugging::Class. If your new class derives directly from EngineService,
			the value of this argument should be #KY_NULL.
	\section engineservicesetup Setting up an EngineService
	To set up an EngineService, you typically must:
	-#	Call Engine::ActivateService(), and provide the class name or MetaClass of the service that you want to set up.
	-#	Call the Initialize() method of your EngineService to configure it for use.
	\pp The ready-to-use classes of EngineService supplied with \SDKName may have other specific requirements. See their class
	descriptions or documentation for details.
	\ingroup kyruntime_core */
class EngineService: public BaseService
{
	EngineService& operator=(const EngineService& );
public:

	/*!	This method is called at each frame, before updating any \Worlds. */
	virtual void Update() {}

	/*!	This method is called at each frame, after updating all \Worlds. */
	virtual void PostUpdate() {}

	/*!	\label_getclass */
	virtual EngineServiceClass&  GetClass() const = 0;

	/*!	\label_getengine */
	Engine& GetEngine() { return m_engine; }

	/*!	\label_getengine */
	const Engine& GetEngine() const  { return m_engine; }

protected:
	/*!	\label_baseconstructor */
	explicit EngineService(Engine& engine);
	Engine& m_engine;
};


/*!	This macro must be included in the declaration of any WorldService class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_DECLARE_WORLDSERVICE(ky_class) \
	static Kaim::WorldServiceClass Class; \
	static Kaim::WorldService* KyCreate(Kaim::World& world); \
	Kaim::WorldServiceClass&  GetClass() { return ky_class::Class; }

/*!	This macro must be included in the implementation of any WorldService class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_IMPLEMENT_WORLDSERVICE(ky_class,ky_base_class) \
	Kaim::WorldServiceClass ky_class::Class(#ky_class,ky_class::KyCreate,(Kaim::WorldServiceClass*)ky_base_class); \
	Kaim::WorldService* ky_class::KyCreate(Kaim::World& world) \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::WorldService* ptr = (Kaim::WorldService*) KY_NEW_CLASS(ky_class)(world); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from WorldService. Each class that derives from 
	WorldService must contain a constructor function that matches this type definition, and that passes
	its arguments on to the constructor function of its parent class. 
	\param world			The World that contains this service.
	\ingroup kyruntime_core */
typedef WorldService* (*WorldServiceConstructor)(World& world);

/*!	WorldServiceClass is the MetaClass for WorldService, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_core */
class WorldServiceClass: public MetaClass<WorldService, WorldServiceConstructor>
{
public:
	/*!	Constructor for the WorldService MetaClass. For internal use. */
	WorldServiceClass(const char* className,  
		WorldServiceConstructor ctr, WorldServiceClass* baseClass = (WorldServiceClass*)NULL);
};


/*!	This macro must be included in the declaration of any EngineService class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_DECLARE_ENGINESERVICE(ky_class) \
	static Kaim::EngineServiceClass Class; \
	static Kaim::EngineService* KyCreate(Kaim::Engine& engine); \
	Kaim::EngineServiceClass&  GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any EngineService class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_IMPLEMENT_ENGINESERVICE(ky_class,ky_base_class) \
	Kaim::EngineServiceClass ky_class::Class(#ky_class,ky_class::KyCreate,(Kaim::EngineServiceClass*)ky_base_class); \
	Kaim::EngineService* ky_class::KyCreate(Kaim::Engine& engine) \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::EngineService* ptr = (Kaim::EngineService*) KY_NEW_CLASS(ky_class)(engine); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from EngineService. Each class that derives from 
	EngineService must contain a constructor function that matches this type definition, and that passes
	its arguments on to the constructor function of its parent class. 
	\param engine			The Engine that contains this service.
	\ingroup kyruntime_core */
typedef EngineService* (*EngineServiceConstructor)(Kaim::Engine& engine);

/*!	EngineServiceClass is the MetaClass for EngineService, created and used internally by
	the \SDKName object factory. 
	\ingroup kyruntime_core */
class EngineServiceClass: public MetaClass<EngineService, EngineServiceConstructor>
{
public:
	/*!	Constructor for the EngineService MetaClass. For internal use. */
	EngineServiceClass(const char* className,
		EngineServiceConstructor ctr, EngineServiceClass* baseClass = (EngineServiceClass*)NULL);
};

} // namespace Kaim

#endif // KY_SERVICE_H
