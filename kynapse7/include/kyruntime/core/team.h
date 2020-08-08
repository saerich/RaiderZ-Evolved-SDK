/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_TEAM_H
#define KY_TEAM_H

#include <kyruntime/core/metaclass.h>
#include <kyruntime/core/timemanager.h>
#include <kyruntime/core/world.h>

#include <kypathdata/generic/memory.h>


namespace Kaim
{

class Team;
class TeamClass;


// Team list.
typedef List<Team*> TeamPtrList;

// Macros definition
/*!	This macro must be included in the declaration of any Team class, to set up the MetaClass
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_DECLARE_TEAM(ky_class) \
	static Kaim::TeamClass Class; \
	static Kaim::Team* KyCreate(Kaim::World& world, const char* name); \
	virtual Kaim::TeamClass&  GetClass() { return ky_class::Class; }

/*! This macro must be included in the implementation of any Team class, to set up the MetaClass
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_IMPLEMENT_TEAM(ky_class, ky_base_class) \
	Kaim::TeamClass ky_class::Class(#ky_class, ky_class::KyCreate, (Kaim::TeamClass*)ky_base_class); \
	Kaim::Team* ky_class::KyCreate(Kaim::World& world, const char* name) \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::Team* ptr = (Kaim::Team*) KY_NEW_CLASS(ky_class)(world, name); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*! Team is the base class for a group of cooperating members. A Team contains a list of
	\Bodies that can be dynamically added and removed. When a Body is de-activated from the 
	World, it is automatically removed from all the \Teams it belongs to.
	\pp See also :USERGUIDE:"Writing Cooperative Behaviors".
	\pp The Team class cannot be used as-is; you must write your own implementation.
	\section customteam Writing a custom Team class
	Writing a custom Team class involves the following considerations.
	\subsection teamserviceconst Constructor
	All classes that derive from Team must have a constructor that follows the #TeamConstructor type definition. 
	\subsection teamvirtualmethods Virtual methods
	All virtual methods of the Team class are optional. You may re-implement them if you wish, or leave the default
	implementations. 
	\subsection teammetaclassmacros MetaClass macros
	In order for \SDKName to recognize your Team class, you must include the following macro definitions:
	-	#KY_DECLARE_TEAM: Place this macro in the declaration of your new class. This macro takes one argument:
		the name of your class.
	-	#KY_IMPLEMENT_TEAM: Place this macro in the implementation file of your new class. This macro requires the
		following arguments:
		-	The name of your new class.
		-	A reference to the metaclass of the class from which your Team directly derives, if different from Team.
			For example, &Kaim::CMyTeam::Class. If your Team class derives directly from Team, the value of this
			argument should be #KY_NULL.
	\section teamusing Using Teams
	\pp Teams are created and destroyed through the World, which manages the lifespan of all Team instances. Use
	World::CreateTeam() and World::DestroyTeam().
	\ingroup kyruntime_core */
class Team: public VirtualMemObject
{
public:
	KY_DECLARE_TEAM(Team);
	friend class World;
	friend class TimeManager;

public:
	/*! \label_baseconstructor
		\param world			The World that contains this Team.
		\param name				A unique name for this Team. */
	explicit Team(World& world, const char* name = 0);

	/*! \label_virtualdestructor */
	virtual ~Team();

	/*! Adds the specified Body to the Team.
		\return #KY_TRUE if the Body was successfully added to the Team, or #KY_FALSE
				otherwise. #KY_FALSE typically indicates that the maximum number of
				members in the Team has been exceeded. */
	virtual KyBool AddBody(Body& body);

	/*! Removes the specified Body from the Team. */
	virtual void RemoveBody(Body& body);

	/*! Retrieves the name set for the Team. */
	const char* GetName() const { return m_name; }

	/*! For internal use. */
	void RemoveFromBody();

	/*! Retrieves an iterator that points to the first Body in the list of members. */
	List<Body*>::ConstIterator GetFirstBody() const { return m_bodies.GetFirst(); }
	
	/*! Retrieves an iterator that points to the first Body in the list of members. */
	List<Body*>::Iterator GetFirstBody() { return m_bodies.GetFirst(); }

protected:
	/*! \label_getworld */
	const World& GetWorld() const { return m_world; }

	/*! \label_getworld */
	World& GetWorld() { return m_world; }

	/*! \label_getengine */
	Engine& GetEngine()
	{
		return GetWorld().GetEngine();
	}

	/*! \label_getengine */
	const Engine& GetEngine() const
	{
		return GetWorld().GetEngine();
	}

protected:
	List<Body*> m_bodies; /*!< The list of the \Bodies in this Team. */

protected:
	TimeManager::TeamTimerInfo m_teamInfo; /*!< For internal use. Do not modify. */

private:
	TeamPtrList::Iterator m_id; /*< The position of this Team in the list of \Teams maintained by the World. Do not modify. */
	World& m_world; /*< Pointer to the World that contains this Team. Do not modify. */
	char* m_name; /*< The name of the Team. */
};



/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from Team. Each class that derives from 
	Team must contain a constructor function that matches this type definition, and that passes
	its arguments on to the constructor function of its parent class.
		\param world			The World that contains this Team.
		\param name				A unique name for this Team, stored in the Team::m_name member.
	\ingroup kyruntime_core */
typedef Team* (*TeamConstructor)(World& world, const char* name);

/*! TeamClass is the MetaClass for Team, created and used internally by the \SDKName object factory.
	\ingroup kyruntime_core */
class TeamClass: public MetaClass<Team, TeamConstructor>
{
public:
	/*!	Constructor for a Team MetaClass. For internal use. */
	TeamClass(const char* className, TeamConstructor ctr, TeamClass* baseClass = (TeamClass*) NULL);
};

} // namespace Kaim

#endif //KY_TEAM_H
