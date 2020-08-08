/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IDETECTGOALREACHED_H
#define KY_IDETECTGOALREACHED_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/core/pointwrapper.h>


namespace Kaim
{

class IPathFinder;
class IDetectGoalReached;


///////////////////////////////////////////////////////////////////////////////////////////
// IDetectGoalReached
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IDetectGoalReached. Each class that derives from 
	IDetectGoalReached must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef IDetectGoalReached* (*DetectGoalReachedConstructor)();

/*!	DetectGoalReachedClass is the MetaClass for IDetectGoalReached, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class DetectGoalReachedClass: public MetaClass<IDetectGoalReached, DetectGoalReachedConstructor>
{
public:
	/*!	Constructor for an IDetectGoalReached MetaClass. For internal use. */
	DetectGoalReachedClass(const char* className, DetectGoalReachedConstructor ctr,
		DetectGoalReachedClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from ICanGo, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_DETECTGOALREACHED(ky_class) \
	static Kaim::DetectGoalReachedClass Class; \
	static Kaim::IDetectGoalReached* KyCreate(); \
	Kaim::DetectGoalReachedClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from ICanGo, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_DETECTGOALREACHED(ky_class,ky_base_class) \
	Kaim::DetectGoalReachedClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::IDetectGoalReached* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IDetectGoalReached* ptr = (Kaim::IDetectGoalReached*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	IDetectGoalReached is an abstract base class for a type of IModifier that the PathFinder 
	uses to determine whether or not its Bot has reached its final destination. 
	When the final destination is reached, the PathFinder will stop the Bot until
	a different destination is passed to the PathFinder::FindNextMove() method, or until this modifier
	indicates that the Bot should no longer be considered to have reached its goal (for example, if the
	Bot is pushed away from its goal point).
	\pp Typical implementations of this interface declare the destination reached as long as
	the Bot is within a certain distance of the goal.
	\section writingcustomidgr Writing a custom IDetectGoalReached class
	Writing a custom class that derives from IDetectGoalReached involves the following considerations.
	\subsection idgrconst Constructor
	All classes that derive from IDetectGoalReached must have a constructor that follows the #DetectGoalReachedConstructor type definition. 
	\subsection idgrvm Virtual methods
	All classes that derive from IDetectGoalReached must implement the DetectGoalReached() method.
	\subsection idgrmetaclass MetaClass macros
	In order for \SDKName to recognize your IDetectGoalReached class, you must include the following macro definitions:
	-	#KY_DECLARE_DETECTGOALREACHED: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_DETECTGOALREACHED: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from IDetectGoalReached.
			For example, &Kaim::DetectGoalReached_Distance2d5::Class. If your new class derives directly from IDetectGoalReached,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class IDetectGoalReached: public IModifier
{
public:
	/*!	\label_constructor */
	IDetectGoalReached():
		IModifier() {}
		
	/*!	\label_getclass */
	virtual DetectGoalReachedClass& GetClass() const = 0;

	/*!	The PathFinder calls this method to determine whether or not the Bot has reached
		its destination.
		\param pos			The current position of the Bot.
		\param goal			The "refined" destination point of the path computed by the IRefineGoal modifier.
		\return #KY_TRUE if the refined destination point has been reached, or #KY_FALSE otherwise. */
	virtual KyBool DetectGoalReached(const PointWrapper& pos, const PointWrapper& goal) = 0;

	/*!	Write accessor for the PathFinder::m_distMin and PathFinder::m_distMax class members. */
	void SetDistanceToTarget(KyFloat32 distMin, KyFloat32 distMax);

	/*!	Read accessor for the PathFinder::m_distMin and PathFinder::m_distMax class members. */
	void GetDistanceToTarget(KyFloat32& distMin, KyFloat32& distMax) const;
};

} // namespace Kaim

#endif // KY_IDETECTGOALREACHED_H
