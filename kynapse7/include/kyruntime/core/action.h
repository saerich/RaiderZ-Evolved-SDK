/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ACTION_H
#define KY_ACTION_H

#include <kyruntime/core/metaclass.h>

#include <kypathdata/generic/memory.h>


namespace Kaim
{

class Bot;
class ActionAttribute;
class ActionAttributeClass;


/*!	The Action class is used to store the results of the decision logic of a thinking Bot. 
	\pp Each Bot maintains an instance of the Action class in its Bot::m_action member. During the \SDKName update
	sequence, the Bot::Think() method updates Bot::m_action to indicate the decision the Bot has taken about what
	it should do and where it should go in the current frame.
	\pp An Action is essentially a container for different types of \ActionAttributes, each of which maintains 
	a specific element of information about the movement the Bot intends to carry out: moving forward at a given 
	speed, turning to a specific orientation, jumping, crouching, etc.
	\pp For more information, see :USERGUIDE:"Data Flows and the Behavior Framework".
	\ingroup kyruntime_core */
class Action
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor For internal use. */
	Action();

	/*!	\label_virtualdestructor */
	~Action();

	/*!	Copies the state of the specified Action to this. */
	Action& operator=(const Action& action);

	/*!	Marks all \ActionAttributes assigned to this Action as not up-to-date. */
	void Reset();

	/*!	Copies the value of each ActionAttribute in the specified Action that has been updated in the last frame 
		into this Action, if this Action has been assigned that ActionAttribute. */
	void Synchronize(const Action& action);

	/*!	Assigns an ActionAttribute of the specified MetaClass to the Action. 
		You cannot add more than one ActionAttribute of a given class.
		\param cls		The MetaClass of the ActionAttribute to create.
		\return A pointer to the new ActionAttibute, or #KY_NULL if the ActionAttribute could not be created. */
	ActionAttribute* AddAttribute(ActionAttributeClass& cls);

	/*!	Assigns an ActionAttribute of the specified class to the Action. 
		You cannot add more than one ActionAttribute of a given class.
		\tparam T		The class of the ActionAttribute to create.
		\return A pointer to the new ActionAttibute, or #KY_NULL if the ActionAttribute could not be created. */
	template < class T >
	T* AddAttribute() { return static_cast<T*>(AddAttribute(T::Class)); }

	/*!	Remove an ActionAttribute of the specified MetaClass from the Action.
		\param cls 		The MetaClass of the ActionAttribute to remove.
		\return #KY_TRUE if the ActionAttribute has been successfully removed, or #KY_FALSE otherwise. */
	KyBool RemoveAttribute(const ActionAttributeClass& cls);

	/*!	Removes an ActionAttribute of the specified class from the Action.
		\tparam T		The class of the ActionAttribute to remove.
		\return #KY_TRUE if the ActionAttribute has been successfully removed, or #KY_FALSE otherwise. */
	template < class T >
	KyBool RemoveAttribute() { return RemoveAttribute(T::Class); }

	/*!	Retrieves a pointer to the ActionAttribute assigned to this Action
		whose class matches or derives from the specified class.
		\param className	The class of the ActionAttribute to retrieve.
		\return A pointer to the ActionAttribute, or #KY_NULL if none match the specified class. */
	ActionAttribute* GetAttribute(const char* className);

	/*!	Retrieves a pointer to the ActionAttribute assigned to this Action
		whose MetaClass matches the specified MetaClass.
		\param cls			The MetaClass of the ActionAttribute to retrieve.
		\param exactType	Determines whether the MetaClass of the ActionAttribute to be retrieved must match the specified
							MetaClass exactly (#KY_TRUE), or whether it may be derived from the specified
							MetaClass (#KY_FALSE, the default). For best performance, use #KY_TRUE.
		\return A pointer to the ActionAttribute, or #KY_NULL if none match the specified class. */
	ActionAttribute* GetAttribute(ActionAttributeClass& cls, KyBool exactType=KY_TRUE);

	/*!	Retrieves a pointer to the ActionAttribute assigned to this Action
		whose class matches the specified class.
		\tparam T			The class of the ActionAttribute to retrieve.
		\param exactType	Determines whether the class of the ActionAttribute to be retrieved must match the specified
							class exactly (#KY_TRUE), or whether it may be derived from the specified
							class (#KY_FALSE, the default). For best performance, use #KY_TRUE.
		\return A pointer to the ActionAttribute, or #KY_NULL if none match the specified class. */
	template < class T >
	T* GetAttribute(KyBool exactType = KY_TRUE) { return static_cast<T*>(GetAttribute(T::Class, exactType)); }


protected:
	KyInt32 m_attrCount; /*!< The number of \ActionAttributes assigned to this Action. Do not modify directly. */
	ActionAttribute** m_attributes; /*!< The list of \ActionAttributes assigned to this Action. Do not modify directly. */
};

/*!	ActionAttribute is the base class for components that can be added to an Action to represent specific 
	elements of each decision made by a Bot: for example, moving forward at a given speed, rotating to a given
	orientation, jumping, crouching, etc. 
	\pp For more information, see :USERGUIDE:"Data Flows and the Behavior Framework".
	\pp This class is purely abstract; only derived classes can be used.
	\section customactionclass Writing a custom ActionAttribute class
	Writing a custom class that derives from ActionAttribute involves the following considerations.
	\subsection actionattrconst Constructor
	All classes that derive from ActionAttribute must have a constructor that follows the #ActionAttributeConstructor type definition. 
	\subsection actionattrdatamembers Data members and accessors
	\pp Each class that derives from ActionAttribute typically maintains a focused set of one or more data members that encapsulate
	an element of movement. You are free to add data members to your derived class as necessary.
	\pp It is good practice to also implement accessor functions to set and retrieve all data maintained by your ActionAttribute. 
	These functions do not have to follow any specific format or naming convention. However, they should call the 
	ActionAttribute::Update() method, to indicate that the value of the ActionAttribute has been updated in the current frame. 
	That way, you do not have to remember to call ActionAttribute::Update() directly every time you set a new value for a
	data member maintained by the attribute.
	\subsection actionattrvm Virtual methods
	Every class that you derive from ActionAttribute must overload the C++ assignment operator. See #operator=.
	You may re-implement the other virtual methods of the ActionAttribute class if desired.
	\subsection actionattrmetaclass MetaClass macros
	If you are creating a new ActionAttribute class that derives directly from ActionAttribute, you must include the 
	following macro definitions in order for \SDKName to recognize your new class:
	-	#KY_DECLARE_ACTIONATTRIBUTE: Place this macro in the declaration of your new class. This macro takes one argument:
		The name of your class.
	-	#KY_IMPLEMENT_ACTIONATTRIBUTE: Place this macro in the implementation file of your new class. This macro takes one 
		argument: The name of your class.
	\pp If you are creating a new ActionAttribute class that derives from one of the \ActionAttributes supplied with 
	\SDKName, you must use the following macro instead of #KY_IMPLEMENT_ACTIONATTRIBUTE:
	-	#KY_IMPLEMENT_DERIVEDACTIONATTRIBUTE: Place this macro in the implementation file of your new class. This macro takes 
		two arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the ActionAttribute class from which your new class derives. For example: 
			<tt>&Kaim::ActionRotate::Class</tt>
	\ingroup kyruntime_core */
class ActionAttribute: public VirtualMemObject
{
	/*!	Indicates whether or not the value stored in the ActionAttribute has been updated in the current
		frame. Do not modify directly; use the accessor methods instead. */
	KyBool m_upToDate;

public:
	/*!	\label_baseconstructor */
	ActionAttribute() : m_upToDate(KY_FALSE) {}
	
	/*!	\label_virtualdestructor */
	virtual ~ActionAttribute() {} 

	/*!	Called internally by Action::AddAttribute() immediately after creating the ActionAttribute, in
		order to perform any initialization steps that may be required in derived classes. */
	virtual void Init() {}

	/*!	This operator must be defined in each derived class. It must copy the values from all data members of the \c attr object
		to this ActionAttribute, if the two \ActionAttributes share the same class. 
		\pp For example, if your ActionAttribute possesses two private class members, \c m_item1 and \c m_item2, and the methods 
		\c SetValue1() and \c SetValue2() that assign values to those items, your assignment operator should copy those two values 
		as follows:
		\code
		Kaim::ActionAttribute& CMyActionAttribute::operator=(const ActionAttribute& attr)
		{
		   if (&attr.GetClass() == &GetClass())
		   {
			 SetValue1(((CMyEntityAttribute&)attr).m_item1);
			 SetValue2(((CMyEntityAttribute&)attr).m_item2);
		   }
		   return *this;
		} \endcode
		\pp Overloading the assignment operator in this way gives \SDKName components the ability to reliably copy values from 
		one instance of an ActionAttribute class to another instance of the same class.
		\pp The implementations of Action::operator= and Action::Synchronize() rely on this operator. */
	virtual ActionAttribute& operator=(const ActionAttribute& attr) = 0;

	/*!	\label_getclass */
	virtual ActionAttributeClass&  GetClass() const = 0;

	/*!	Indicates whether or not this ActionAttribute is up-to-date: i.e. whether or not its value
		has been changed in the current frame.*/
	KyBool IsUpToDate() const { return m_upToDate; }
	
	/*!	Marks the ActionAttribute as up-to-date, indicating that it has changed in the current frame and that its 
		value should be taken into account by Action::Apply(). */
	void Update() { m_upToDate = KY_TRUE; }

	/*!	Marks the ActionAttribute to be not up-to-date, indicating that it has not changed in the current frame
		and that its value should not be taken into account by Action::Apply(). */
	virtual void Reset() { m_upToDate = KY_FALSE; }
};


/*!	This macro must be called in the declaration of any ActionAttribute class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_DECLARE_ACTIONATTRIBUTE(ky_class) \
	static Kaim::ActionAttributeClass Class; \
	static Kaim::ActionAttribute* KyCreate(); \
	Kaim::ActionAttributeClass&  GetClass() const { return ky_class::Class; }

/*!	This macro must be called in the implementation of an ActionAttribute class that
	derives directly from ActionAttribute, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_IMPLEMENT_ACTIONATTRIBUTE(ky_class/*, ky_base_class = KY_NULL*/) \
	Kaim::ActionAttributeClass ky_class::Class(#ky_class,ky_class::KyCreate,KY_NULL); \
	Kaim::ActionAttribute* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::ActionAttribute* ptr = (Kaim::ActionAttribute*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}
	
/*!	This macro must be called in the implementation of any ActioAttribute class that
	derives from an ActionAttribute supplied in the \SDKName SDK, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_core */
#define KY_IMPLEMENT_DERIVEDACTIONATTRIBUTE(ky_class, ky_base_class) \
	Kaim::ActionAttributeClass ky_class::Class(#ky_class,ky_class::KyCreate,(Kaim::ActionAttributeClass*)ky_base_class); \
	Kaim::ActionAttribute* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::ActionAttribute* ptr = (Kaim::ActionAttribute*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from ActionAttribute. Each class that derives from 
	ActionAttribute must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_core */
typedef ActionAttribute* (*ActionAttributeConstructor)();

/*!	ActionAttributeClass is the MetaClass for ActionAttribute, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_core */
class ActionAttributeClass: public MetaClass<ActionAttribute, ActionAttributeConstructor>
{
public:
	/*!	Constructor for the ActionAttribute MetaClass. For internal use. */
	ActionAttributeClass(const char* className, ActionAttributeConstructor ctr, ActionAttributeClass* baseClass = KY_NULL);
};

} // namespace Kaim

#endif // KY_ACTION_H
