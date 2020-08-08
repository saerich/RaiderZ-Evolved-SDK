/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODY_H
#define KY_BODY_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/math/trigo.h>
#include <kypathdata/math/vec3futils.h>
#include <kyruntime/core/metaclass.h>
#include <kyruntime/core/shape.h>
#include "kypathdata/containers/list.h"

namespace Kaim
{
	class Bot;
	class BotClass;
	class BodyAttribute;
	class BodyAttributeClass;
	class PointLockManager;
	class PointLockDataPerBody;
	class World;
	class Body;
	class BodyProfile;
	class Team;

	typedef List<Team*> TeamPtrList;


	typedef KyUInt32 BodyRendering; /*!< Defines a type for a bitmask composed of elements from the #BodyMaskRendering enumeration. \ingroup kyruntime_core */
	/*!	This enumeration is used to indicate what information should be rendered in a call to Bot::Render() or Body::Render().
		\ingroup kyruntime_core */
	enum BodyMaskRendering
	{
		KY_R_BODY_SHAPE = 1<<0, /*!< Draws the outline of the Body. */
		KY_R_BODY_ORI_VEC = 1<<1, /*!< Draws the orientation of the Body, using a line that is proportional in length to the current speed of the Body. */
		KY_R_BODY_NAME = 1<<2, /*!< Draws the name of the Body. */
		KY_R_BODY_ALL = KY_R_BODY_SHAPE | KY_R_BODY_ORI_VEC, /*!< Draws all information about the Body. */
		KY_R_BODY_LAST = (1<<3), //< For internal use.
		KY_DUMMY_BodyMaskRendering = KYFORCEENUMSIZEINT
	};

////////////////////////////////////////////////////////////////////////////////////////
//		Body 
////////////////////////////////////////////////////////////////////////////////////////

	/*!	The Body class represents the current state and characteristics of a character in the \gameOrSim engine.
		\pp Every Bot maintains an instance of the Body class. You can also use Body objects on their own, if you need to
		represent within the \SDKName World a character that will not use \SDKName to make decisions. For example, a Body
		is typically used to represent a character controlled by the player.
		\pp For more information, see :USERGUIDE:"Characters and Objects".
		\pp For details on how to create and initialize \Bodies, see :USERGUIDE:"Integrating Kynapse".
	*/
	class Body
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		/*! \label_constructor \label_mustinit */
		Body();
		/*! \label_destructor */
		~Body(); 

	public:
		/*! \label_init
		\param world				The World to which this Body will be added. Accessible through GetWorld().
		\param name					A unique name to identify this Body. Accessible through GetName().
		\param userData				A void pointer typically used to identify the object in the \gameOrSim engine that this Body
									will represent. Accessible through GetUserData(). */
		KyResult Initialize(Kaim::World& world, const char* name, void* userData);

		/*! \label_clear */
		KyResult Clear();

	////////////////////////////////////////////////////////////////////////////////////////
	// World access
	public:
		/*! Indicates whether or not the Body has been added to a World by a call to Initialize(). */
		bool IsAttachedToWorld() const
		{
			return (m_world != KY_NULL && m_id.IsValid());
		}

		/*! Indicates whether or not the Body has been added to a World by a call to Initialize(). */
		bool IsAttachedToWorld(const World& world) const
		{
			return (&world == m_world && m_id.IsValid());
		}

		/*!	\label_getworld */
		const World& GetWorld() const 
		{
			return *m_world;
		}

		/*!	\label_getworld */
		World& GetWorld()
		{
			return *m_world;
		}

	private:


	////////////////////////////////////////////////////////////////////////////////////////
	// Bot access

	public:
		/*! Retrieves a pointer to the Bot that owns this Body, or #KY_NULL if the Body is used on its own and is
			not owned by any Bot. */
		const Bot* GetBot() const
		{
			return m_bot;
		}

		/*! Retrieves a pointer to the Bot that owns this Body, or #KY_NULL if the Body is used on its own and is
			not owned by any Bot. */
		Bot* GetBot() 
		{
			return m_bot;
		}

	////////////////////////////////////////////////////////////////////////////////////////
	// Attributes
	public:

		/*!	Creates and assigns a BodyAttribute with the specified MetaClass. 
			You cannot add more than one BodyAttribute of a given MetaClass.
			\param cls		The MetaClass of the BodyAttribute to create.
			\return A pointer to the new BodyAttibute, or #KY_NULL if the BodyAttribute could not be created. */
		BodyAttribute* AddAttribute(BodyAttributeClass& cls);

		/*!	Creates and assigns a BodyAttribute of the specified class. 
			You cannot add more than one BodyAttribute of a given class.
			\tparam			The class of the BodyAttribute to create.
			\return A pointer to the new BodyAttibute, or #KY_NULL if the BodyAttribute could not be created. */
		template < class T >
		T* AddAttribute() { return static_cast<T*>(AddAttribute(T::Class)); }

		/*!	Removes a BodyAttribute of the specified MetaClass.
			\param cls 		The MetaClass of the BodyAttribute to remove.
			\return #KY_TRUE if the BodyAttribute has been successfully removed, #KY_FALSE otherwise. */
		KyBool RemoveAttribute(const BodyAttributeClass& cls);

		/*!	Removes a BodyAttribute of the specified class.
			\tparam  		The class of the BodyAttribute to remove.
			\return #KY_TRUE if the BodyAttribute has been successfully removed, #KY_FALSE otherwise. */
		template < class T >
		KyBool RemoveAttribute() { return RemoveAttribute(T::Class); }

		/*!	Retrieves a pointer to the BodyAttribute that matches the specified MetaClass.
			\param cls			The MetaClass of the BodyAttribute to retrieve.
			\param exactType	Determines whether the MetaClass of the BodyAttribute to be retrieved must match the specified
								MetaClass exactly (#KY_TRUE), or whether it may be derived from the specified
								MetaClass (#KY_FALSE, the default). For best performance, use #KY_TRUE.
			\return A pointer to the BodyAttribute, or #KY_NULL if none match the specified MetaClass. */
		BodyAttribute* GetAttribute(BodyAttributeClass& cls, KyBool exactType=KY_TRUE) const;

		/*!	Retrieves a pointer to the BodyAttribute that matches the specified class.
			\tparam				The class of the BodyAttribute to retrieve.
			\param exactType	Determines whether the class of the BodyAttribute to be retrieved must match the specified
								class exactly (#KY_TRUE), or whether it may be derived from the specified
								class (#KY_FALSE, the default). For best performance, use #KY_TRUE.
			\return A pointer to the BodyAttribute, or #KY_NULL if none match the specified class. */
		template < class T >
		T* GetAttribute(KyBool exactType = KY_TRUE) const { return static_cast<T*>(GetAttribute(T::Class, exactType)); }

	////////////////////////////////////////////////////////////////////////////////////////

	public:
		/*! Sets the current position of the Body. */
		void SetPosition(const Vec3f& position)
		{
			m_position = position;
		}

		/*! Sets the current orientation of the Body. 
			\pp Note that this should be set to the direction of movement, which may be different from the facing
			direction of the character's model. To store the facing direction of the model, use the BodyTorsoOrientation
			class of BodyAttribute.
			\param oriAngles			The orientation of the Body, expressed in Euler angles (pitch, roll, yaw). */
		void SetOriAngles(const Vec3f& oriAngles)
		{
			m_oriAngles = oriAngles;
			Kaim::AnglesGetYaw(m_oriAngles) = Kaim::GetNormalizeAngleDeg( Kaim::AnglesGetYaw(m_oriAngles) );

			m_oriVector = Kaim::MakeVectorDeg(m_oriAngles);
		}

		/*! Sets the current orientation of the Body. 
			\pp Note that this should be set to the direction of movement, which may be different from the facing
			direction of the character's model. To store the facing direction of the model, use the BodyTorsoOrientation
			class of BodyAttribute. 
			\param oriVector			The orientation of the Body, expressed as a normalized vector.
										See :USERGUIDE:"The Kynapse Coordinate System" and :USERGUIDE:"Pitch, Roll and Yaw". */
		void SetOriVector(const Vec3f& oriVector) 
		{
			m_oriVector = oriVector;
			m_oriVector.Normalize();

			m_oriAngles = Kaim::VecToAnglesDeg(m_oriVector);
			Kaim::AnglesGetYaw(m_oriAngles) = Kaim::GetNormalizeAngleDeg( Kaim::AnglesGetYaw(m_oriAngles) );
		}

		/*! Sets the current scalar speed of the Body in its direction of movement. */
		void SetSpeed(KyFloat32 speed) 
		{
			m_speed = speed;
		}

		/*! Sets a void pointer stored by the Body, typically used to identify the character or object in the \gameOrSim engine
			that corresponds to this Body. */
		void SetUserData(void* userData) 
		{
			m_userData = userData;
		}

	public:
		/*! Retrieves the current position set for the Body. */
		const Vec3f& GetPosition() const
		{
			return m_position;
		}

		/*! Retrieves the current orientation of the Body, expressed using Euler angles (pitch, roll, yaw). 
			See :USERGUIDE:"The Kynapse Coordinate System" and :USERGUIDE:"Pitch, Roll and Yaw". */
		const Vec3f& GetOriAngles() const
		{
			return m_oriAngles;
		}

		/*! Retrieves the current orientation of the Body, expressed using Euler angles (pitch, roll, yaw). 
			See :USERGUIDE:"The Kynapse Coordinate System" and :USERGUIDE:"Pitch, Roll and Yaw". */
		Vec3f& GetOriAngles() 
		{
			return m_oriAngles;
		}

		/*! Retrieves the current orientation of the Body, expressed as a normalized vector. 
			See :USERGUIDE:"The Kynapse Coordinate System" and :USERGUIDE:"Pitch, Roll and Yaw". */
		const Vec3f& GetOriVector() const
		{
			return m_oriVector;
		}

		/*! Retrieves the current orientation of the Body, expressed as a normalized vector. 
			See :USERGUIDE:"The Kynapse Coordinate System" and :USERGUIDE:"Pitch, Roll and Yaw". */
		KyFloat32 GetSpeed() const
		{
			return m_speed;
		}

		/*! Retrieves the void pointer stored by the Body, typically used to identify the character or object in the \gameOrSim engine
			that corresponds to this Body. Set in the constructor or through a call to SetUserData(). */
		void* GetUserData() const
		{
			return m_userData;
		}

	////////////////////////////////////////////////////////////////////////////////////////
	// Shape


	public:
		/*! Retrieves the Shape that stores the physical dimensions of the Body. */
		Shape& GetShape() { return m_shape; }

		/*! Retrieves the Shape that stores the physical dimensions of the Body. */
		const Shape& GetShape() const { return m_shape; }

		/*! Retrieves the width of the Body. */
		KyFloat32 GetWidth() const { return m_shape.GetWidth(); }

		/*! Retrieves the length of the Body. */
		KyFloat32 GetLength() const { return m_shape.GetDefaultedLength(); }

		/*! Retrieves the height of the Body. */
		KyFloat32 GetHeight() const { return m_shape.GetDefaultedHeight(); }

		/*! Retrieves the radius of the Body: half of its width value. */
		KyFloat32 GetRadius() const { return m_shape.GetRadius(); }


	////////////////////////////////////////////////////////////////////////////////////////
	// Name

	public:
		/*! Retrieves the unique name set for this Body in the constructor. */
		const char* GetName() const
		{
			return m_name;
		}

	private:	
		static void CreateName(char* name, KyUInt32 allocatedSize);


	////////////////////////////////////////////////////////////////////////////////////////
	// Team support

	private:	
		/*	For internal use. */
		void AddTeam(Team* team);

		/*	For internal use. */
		void RemoveTeam(Team* team);

		/*	Removes this Body from all \Teams that currently include it as a member. For internal use. */
		void RemoveFromTeams();

	////////////////////////////////////////////////////////////////////////////////////////
	// Profile

	public:
		// Profile management
		/*!	Assigns a BodyProfile to this Body. This BodyProfile is used by the BodyInfoManager to determine which types
			of BodyInfo if any it should calculate and store for this Body. See :USERGUIDE:"Tracking Perception Data". */
		KyBool SetProfile(BodyProfile* profile);

		/*!	Retrieves the BodyProfile to this Body. This BodyProfile is used by the BodyInfoManager to determine which types
			of BodyInfo if any it should calculate and store for this Body. See :USERGUIDE:"Tracking Perception Data". */
		BodyProfile* GetProfile() const { return m_profile; }

		KyUInt32 GetPosInProfile() const { return m_posInProfile; } /*!< For internal use. */

	////////////////////////////////////////////////////////////////////////////////////////
	// Rendering

	public:

	/*!	Renders information about the Body for debugging purposes.
		\param drawingMask	Indicates the information that will be rendered. This value is a bit mask 
							composed of elements from the #BodyMaskRendering enumeration. */
	void Render(BodyRendering drawingMask) const;

	////////////////////////////////////////////////////////////////////////////////////////

	private:
		World* m_world; //< Pointer to the World containing the Bot. Do not modify. */
		Bot* m_bot;
		List<Body*>::Iterator m_id;/*< The position of the Body within the World::m_bodies list. Do not modify. */
		char* m_name; /*< The name of the Bot. It should be unique among all \Entities. Do not modify. */
		TeamPtrList m_teams; /*< A list of pointers to all \Teams the Bot belongs to. */
		BodyProfile* m_profile;/*< The BodyProfile assigned to this Bot for use by the BodyInfoManager. */
		KyUInt32 m_posInProfile; /*< For internal use. */

		PointLockDataPerBody* m_pointLockData;

	private:
		Shape m_shape;
	private:
		Vec3f m_position; /*< The absolute position of the Bot. The vertical coordinate must be at waist height (i.e. height/2 above the ground). */
		Vec3f m_oriAngles; /*< The orientation of the Bot, expressed in Euler angles (pitch, roll, yaw). */
		Vec3f m_oriVector; /*< The orientation of the Bot, expressed as a normalized vector. */
		KyFloat32 m_speed; /*< The speed of the Bot, in meters per second. */
	private:
		KyInt32 m_attrCount; /*< The number of \BodyAttributes assigned to this Bot. */
		BodyAttribute** m_attributes;/*< The list of \BodyAttributes assigned to this Bot. */
	private:
		void* m_userData;	

		friend class Kaim::World;
		friend class Kaim::BodyProfile;
		friend class Kaim::Team;
		friend class Kaim::PointLockManager;
		friend class Kaim::Bot;

	};


	////////////////////////////////////////////////////////////////////////////////////////
	//		Body attributes
	////////////////////////////////////////////////////////////////////////////////////////

	/*!	BodyAttribute is the base class for components that can be added to a Body to represent specific elements
		of a character's state, characteristics or abilities: for example, its ability to fly, its visual range,
		its maximum speed, etc.
		\pp For more information, see :USERGUIDE:"Data Flows and the Behavior Framework".
		\pp This class is purely abstract; only derived classes can be used.
		\section custombodyclass Writing a custom BodyAttribute class
		Writing a custom class that derives from BodyAttribute involves the following considerations.
		\subsection bodyattributeconst Constructor
		All classes that derive from BodyAttribute must have a constructor that follows the #BodyAttributeConstructor type definition. 
		\subsection bodyattributedatamembers Data members and accessors
		\pp Each class that derives from BodyAttribute typically maintains a focused set of one or more data members that encapsulate
		a single state, characteristic or ability. You are free to add data members to your derived class as necessary.
		\pp It is good practice to also implement accessor functions to set and retrieve all data maintained by your BodyAttribute. 
		\subsection bodyattributevm Virtual methods
		Every class that you derive from BodyAttribute must overload the C++ assignment operator. See #operator=.
		You may re-implement the other virtual methods of the BodyAttribute class if desired.
		\subsection bodyattributemacros MetaClass macros
		If you are creating a new BodyAttribute class that derives directly from BodyAttribute, you must include the 
		following macro definitions in order for \SDKName to recognize your new class:
		-	#KY_DECLARE_BODYATTRIBUTE: Place this macro in the declaration of your new class. This macro takes one argument:
			The name of your class.
		-	#KY_IMPLEMENT_BODYATTRIBUTE: Place this macro in the implementation file of your new class. This macro takes one 
			argument: The name of your class.
		\pp If you are creating a new BodyAttribute class that derives from one of the \BodyAttributes supplied with 
		\SDKName, you must use the following macro instead of #KY_IMPLEMENT_BODYATTRIBUTE:
		-	#KY_IMPLEMENT_DERIVEDBODYATTRIBUTE: Place this macro in the implementation file of your new class. This macro takes 
			two arguments:
			-	The name of your new class.
			-	A reference to the MetaClass of the BodyAttribute class from which your new class derives. For example: 
				<tt>&Kaim::BodyCanFly::Class</tt>
		\ingroup kyruntime_core */
	class BodyAttribute: public VirtualMemObject
	{
	public:
		/*!	\label_baseconstructor */
		BodyAttribute() {}
		
		/*!	\label_virtualdestructor */
		virtual ~BodyAttribute() {}
		
		/*!	This operator must be defined in each derived class to copy the state of the BodyAttribute to another instance
			of the same class. */
		virtual BodyAttribute& operator=(const BodyAttribute& attr) = 0; 

		/*!	\label_getclass */
		virtual BodyAttributeClass&  GetClass() const = 0;

		/*!	Called internally by Bot::AddAttribute() immediately after creating the BodyAttribute, in
			order to perform any initialization steps that may be required in derived classes. */
		virtual void Init() {}

	public:
		/*! Allocates and returns a pointer to a new copy of this object. It is up to you to destroy the new object and de-allocate
			its memory when no longer needed. */ 
		BodyAttribute* Clone() const;
	};


	/*!	This macro must be included in the declaration of any BodyAttribute class, to set up the MetaClass 
		used by the \SDKName object factory.
		\ingroup kyruntime_core */
	#define KY_DECLARE_BODYATTRIBUTE(ky_class) \
		static Kaim::BodyAttributeClass Class; \
		static Kaim::BodyAttribute* KyCreate(); \
		Kaim::BodyAttributeClass&  GetClass() const { return ky_class::Class; }

	/*!	This macro must be included in the implementation of any BodyAttribute class that derives directly
		from BodyAttribute, to set up the MetaClass used by the \SDKName object factory.
		\ingroup kyruntime_core */
	#define KY_IMPLEMENT_BODYATTRIBUTE(ky_class/*, ky_base_class = KY_NULL*/) \
		Kaim::BodyAttributeClass ky_class::Class(#ky_class,ky_class::KyCreate,KY_NULL); \
		Kaim::BodyAttribute* ky_class::KyCreate() \
		{ \
			KY_LOG_MEMORY(ky_class::Class.GetName()); \
			Kaim::BodyAttribute* ptr = (Kaim::BodyAttribute*) KY_NEW_CLASS(ky_class)(); \
			KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
			return ptr; \
		}

	/*!	This macro that must be included in the implementation of any BodyAttribute class that derives from
		an BodyAttribute supplied in the \SDKName SDK, to set up the MetaClass used by the \SDKName
		object factory.
		\ingroup kyruntime_core */
	#define KY_IMPLEMENT_DERIVEDBODYATTRIBUTE(ky_class,ky_base_class) \
		Kaim::BodyAttributeClass ky_class::Class(#ky_class,ky_class::KyCreate,(Kaim::BodyAttributeClass*)ky_base_class); \
		Kaim::BodyAttribute* ky_class::KyCreate() \
		{ \
			KY_LOG_MEMORY(ky_class::Class.GetName()); \
			Kaim::BodyAttribute* ptr = (Kaim::BodyAttribute*) KY_NEW_CLASS(ky_class)(); \
			KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
			return ptr; \
	}


	/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
		needs to create an instance of a class that derives from BodyAttribute. Each class that derives from 
		BodyAttribute must contain a constructor function that matches this type definition. The argument list
		of this type definition is empty, which indicates that the default constructor of your derived class
		will be invoked.
		\ingroup kyruntime_core */
	typedef BodyAttribute* (*BodyAttributeConstructor)();

	/*!	BodyAttributeClass is the MetaClass for BodyAttribute, created and used internally by
		the \SDKName object factory.
		\ingroup kyruntime_core */
	class BodyAttributeClass: public MetaClass<BodyAttribute, BodyAttributeConstructor>
	{
	public:
		/*!	Constructor for an BodyAttribute MetaClass. For internal use. */
		BodyAttributeClass(const char* className, BodyAttributeConstructor ctr, BodyAttributeClass* baseClass = KY_NULL);
	};

} // namespace Kaim

#endif // KY_BODY_H
