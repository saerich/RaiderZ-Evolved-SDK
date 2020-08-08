/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IREFINEGOAL_H
#define KY_IREFINEGOAL_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/core/pointwrapper.h>


namespace Kaim
{

class IPathFinder;
class IRefineGoal;


///////////////////////////////////////////////////////////////////////////////////////////
// IRefineGoal
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IRefineGoal. Each class that derives from 
	IRefineGoal must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef IRefineGoal* (*RefineGoalConstructor)();

/*!	RefineGoalClass is the MetaClass for IRefineGoal, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class RefineGoalClass: public MetaClass<IRefineGoal, RefineGoalConstructor>
{
public:
	/*!	Constructor for an IRefineGoal MetaClass. For internal use. */
	RefineGoalClass(const char* className, RefineGoalConstructor ctr, RefineGoalClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from IRefineGoal, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_REFINEGOAL(ky_class) \
	static Kaim::RefineGoalClass Class; \
	static Kaim::IRefineGoal* KyCreate(); \
	Kaim::RefineGoalClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from IRefineGoal, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_REFINEGOAL(ky_class, ky_base_class) \
	Kaim::RefineGoalClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::IRefineGoal* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IRefineGoal* ptr = (Kaim::IRefineGoal*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	IRefineGoal is an abstract base class for a type of IModifier that the PathFinder uses to
	adjust the final destination point passed to PathFinder::FindNextMove() to ensure that the
	destination is valid and reachable. 
	\section writingcustomirefinegoal Writing a custom IRefineGoal class
	Writing a custom class that derives from IRefineGoal involves the following considerations.
	\subsection irefinegoalconst Constructor
	All classes that derive from IRefineGoal must have a constructor that follows the #RefineGoalConstructor type definition. 
	\subsection irefinegoalvm Virtual methods
	All classes that derive from IRefineGoal must implement the RefineGoal() method.
	\subsection irefinegoalmetaclass MetaClass macros
	In order for \SDKName to recognize your IRefineGoal class, you must include the following macro definitions:
	-	#KY_DECLARE_REFINEGOAL: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_REFINEGOAL: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from IRefineGoal.
			For example, &Kaim::RefineGoal_LpfCompatible::Class. If your new class derives directly from IRefineGoal,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class IRefineGoal: public IModifier
{
public:
	/*!	\label_constructor */
	IRefineGoal():
		IModifier() {}
		
	/*!	\label_getclass */
	virtual RefineGoalClass& GetClass() const = 0;

	/*!	The PathFinder calls this method in order to convert the destination point supplied to 
		its PathFinder::FindNextMove() method into a reachable, valid destination for pathfinding.
		\param goal					The position of the final destination point for the Bot.
		\param[out] refinedGoal 	Stores the position of the refined goal calculated by this method. */
	virtual void RefineGoal(const PointWrapper& goal, PointWrapper& refinedGoal) = 0;
};

} // namespace Kaim

#endif // KY_IREFINEGOAL_H
