/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BOT_H
#define KY_BOT_H

#include <kypathdata/containers/list.h>

#include <kypathdata/basesystem/profilermacro.h>

#include <kyruntime/core/body.h>
#include <kyruntime/core/action.h>
#include <kyruntime/core/service.h>
#include <kyruntime/core/timemanager.h>
#include <kypathdata/basesystem/memorylog.h>

namespace Kaim
{

class BotClass;
class World;
class Engine;
class PathFinder;
class Bot;
class PathObject;
class PathObjectBehaviorContext;

	
//< Bot list iterator.
typedef List<Bot*> BotPtrList;

////////////////////////////////////////////////////////////////////////////////////////
//		Bot 
////////////////////////////////////////////////////////////////////////////////////////

/*!	Bot is the abstract base class for objects that represent characters in your \gameOrSim that use the \SDKName behavior,
	pathfinding and perception frameworks to think, make decisions, and act.
	\pp The Bot class is purely abstract. You must derive this class in order to specialize it for
	your own project.
	\pp For more information on how \Bots generate behaviors, see :USERGUIDE:"Data Flows and the Behavior Framework".
	\pp For details on how to create and initialize \Bots, see :USERGUIDE:"Integrating Kynapse".
	\section writingcustombots Writing a custom Bot class
	Writing a custom class that derives from Bot involves the following considerations.
	\subsection botconst Constructor
	All classes that derive from Bot must have a constructor that follows the #BotConstructor type definition. 
	\subsection botvm Virtual methods
	All classes that derive from Bot must implement the Think() method. Once you initialize your Bot in a World, its Think()
	method is called at each frame to make a decision about what the character should do next.
	\pp You may also re-implement other virtual methods, or use their default implementations if you prefer. Most custom Bot
	classes also implement the OnInitialize() method, in order to assign \BodyAttributes and \ActionAttributes to the Body and
	Action owned by the Bot. See :USERGUIDE:"Using BodyAttributes and ActionAttributes".
	\subsection botmetaclass MetaClass macros
	In order for \SDKName to recognize your Bot class, you must include the following macro definitions:
	-	#KY_DECLARE_BOT: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_BOT: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from Bot.
			For example, &CMyBot::Class. If your new class derives directly from Bot, the value of this argument 
			should be #KY_NULL. */
class Bot: public VirtualMemObject
{
	/*! Copies the state of the specified Bot instance to this. */
	Bot& operator=(const Bot&);

	/*! \label_constructor Sets the state of the newly constructed Bot to match the specified Bot. */
	Bot(const Bot&);
public:
	/*! \label_constructor \label_mustinit */
	Bot();
	
	/*!	\label_virtualdestructor */
	virtual ~Bot()
	{  
		KY_FUNCTION("Bot::~Bot");
		KY_ASSERT(m_body.IsAttachedToWorld()==KY_FALSE, ("The bot must be properly cleared before being destroyed"));
	}

public:

	/*! \label_reinit Called transparently by World::ReInit(). */
	virtual void ReInit();

	/*!	\label_getclass */
	virtual BotClass&  GetClass() const = 0;

	/*!	\label_init
		\param world				The World to which this Bot will be added. Accessible through GetWorld().
		\param name					A unique name to identify this Bot. Passed transparently to Body::Initialize() and accessible through 
									Body::GetName().
		\param userData				A void pointer typically used to identify the object in the \gameOrSim engine that this Bot
									will represent. Passed transparently to Body::Initialize() and accessible through 
									Body::GetUserData(). */
	KyResult Initialize(Kaim::World& world, const char* name, void* userData);

	/*! \label_clear */
	void Clear()
	{
		OnClear();
		DoClear();
	}

	/*!	Updates the Bot and generates a decision.
		\pp This method is called transparently by World::Update(). You typically do not need to call it directly.
		\pp This method:
		-	Calls Action::Reset() for the #m_action class member. This marks each ActionAttribute assigned to the 
			Action as being not up-to-date: i.e. not modified in the current frame. This makes the Action ready to 
			accept the results of the decision in the current frame.
		-	Calls the Bot::Think() method to generate a decision and save it in #m_action. */
	void Update()
	{
		m_action.Reset();
		{
			KY_PROFILE("Bot::Think");
			KY_LOG_MEMORY("Bot::Think");
			Think();
		}
	}


	/*! Executes the decision logic of the Bot. You must implement this method in your custom Bot classes.
		\pp Implementations of this method should generate a decision about what the character should do in the current
		frame, and save the elements of the decision in the \ActionAttributes assigned to #m_action.
		\pp For more information, see :USERGUIDE:"Data Flows and the Behavior Framework". */
	virtual void Think() = 0;

	/*! The PathFinder used by this Bot to calculate and follow paths through the terrain of the World. */
	PathFinder* GetPathFinder() const { return m_pathFinder; }

	/*! The Action used by this Bot to express the decision it makes at each frame. */
	Action m_action;

	/*!	\label_getworld 
	\pre IsAttachedToWorld() == true
	*/
	const World& GetWorld() const 
	{
		return GetBody().GetWorld();
	}

	/*!	\label_getworld 
	\pre IsAttachedToWorld() == true
	*/
	World& GetWorld()  
	{
		return GetBody().GetWorld();
	}

	/*!	\label_getengine */
	Engine& GetEngine();

	/*!	\label_getengine */
	const Engine& GetEngine() const;

	/*!	Renders information about the Bot for debugging purposes.
		\param drawingMask	Indicates the information that will be rendered. This value is a bit mask 
							composed of elements from the #BodyMaskRendering enumeration. */
	void Render(BodyRendering drawingMask) const
	{
		return GetBody().Render(drawingMask);
	}

	/*!	Write accessor for #m_priorityFactor. */
	void SetPriorityFactor(KyFloat32 factor) { m_botInfo.m_priorityFactor = factor; }

	/*! Read accessor for #m_priorityFactor. */
	KyFloat32 GetPriorityFactor() const { return m_botInfo.m_priorityFactor; }

	/*!	Write accessor for #m_priority. For internal use. */
	void SetPriority(KyFloat32 priority);

	/*!	Read accessor for #m_priority. For internal use. */
	KyFloat32 GetPriority() const { return m_botInfo.m_priority; }

////////////////////////////////////////////////////////////////////////////////////////
// PathObject

public:
	/*! @{ */
	/*! For internal use. */
	void AddPathObjectBehaviorContext(List<PathObjectBehaviorContext>::Iterator context);
	void RemovePathObjectBehaviorContext(List<PathObjectBehaviorContext>::Iterator context);
	PathObjectBehaviorContext* GetPathObjectBehaviorContext(PathObject* pathObject) const;
	KyResult GetPathObjectBehaviorContextIterator(PathObject* pathObject, List<PathObjectBehaviorContext>::Iterator& iterator);
	/*! @} */
protected:	
	/*! Called transparently during each call to Initialize(). Typical implementations set up the Body and Action owned
		by the Bot with the BodyAttribute and ActionAttribute classes required in order to exchange data with the \gameOrSim engine.
		See also :USERGUIDE:"Using BodyAttributes and ActionAttributes". */
	virtual KyResult OnInitialize() = 0;

	/*! Called transparently during each call to Clear(). You can implement this method in your custom class in order to carry out
		tasks that must be done when the Bot is removed from a World. */
	virtual void OnClear() = 0;

private:
	void DoClear();

	List<PathObjectBehaviorContext>::Iterator m_pathObjectBehaviorContext;

public:
	/*! Indicates whether or not the Bot has been added to a World by a call to Initialize(). */
	bool IsAttachedToWorld() const;

	/*! Indicates whether or not the Bot has been added to the specified World by a call to Initialize(). */
	bool IsAttachedToWorld(const World& world) const;

////////////////////////////////////////////////////////////////////////////////////////
// Body

public:
	/*! Retrieves the Body owned by this Bot, which maintains the current state and characteristics of the corresponding
		character in the \gameOrSim engine. */
	Body& GetBody();

	/*! Retrieves the Body owned by this Bot, which maintains the current state and characteristics of the corresponding
		character in the \gameOrSim engine. */
	const Body& GetBody() const;

////////////////////////////////////////////////////////////////////////////////////////

protected:
	friend class Kaim::World;
	friend class TimeManager;

	Body m_body; /*!< A pointer to the Body owned by this Bot. Do not modify directly. Do not modify. */
	PathFinder* m_pathFinder; /*!< A pointer to the PathFinder owned by this Bot. Do not modify directly. */
	TimeManager::BotTimerInfo m_botInfo; /*!< For internal use. Do not modify. */
};


/*!	This macro must be included in the declaration of any Bot class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_DECLARE_BOT(ky_class) \
	static Kaim::BotClass Class; \
	static Kaim::Bot* KyCreate(); \
	Kaim::BotClass&  GetClass() const { return ky_class::Class; } \


/*!	This macro must be included in the implementation of any Bot class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_IMPLEMENT_BOT(ky_class,ky_base_class) \
	Kaim::BotClass ky_class::Class(#ky_class,ky_class::KyCreate,(Kaim::BotClass*)ky_base_class); \
	Kaim::Bot* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::Bot* ptr = (Kaim::Bot*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from Bot. Each class that derives from 
	Bot must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_core */
typedef Bot* (*BotConstructor)();

/*!	BotClass is the MetaClass for Bot, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_core */
class BotClass: public MetaClass<Bot, BotConstructor>
{
public:
	/*!	Constructor for an Bot MetaClass. For internal use. */
	BotClass(const char* className, BotConstructor ctr, BotClass* baseClass = (BotClass*)NULL);
};

} // namespace Kaim

#endif // KY_BOT_H
