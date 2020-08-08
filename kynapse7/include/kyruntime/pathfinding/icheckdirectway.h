/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ICHECKDIRECTWAY_H
#define KY_ICHECKDIRECTWAY_H

#include <kyruntime/pathfinding/imodifier.h>


namespace Kaim
{

class ICheckDirectWay;


/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from ICheckDirectWay. Each class that derives from 
	ICheckDirectWay must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef ICheckDirectWay* (*CheckDirectWayConstructor)();

/*!	CheckDirectWayClass is the MetaClass for ICheckDirectWay, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class CheckDirectWayClass: public MetaClass<ICheckDirectWay, CheckDirectWayConstructor>
{
public:
	/*!	Constructor for an ICheckDirectWay MetaClass. For internal use. */
	CheckDirectWayClass(const char* className, CheckDirectWayConstructor ctr,
		CheckDirectWayClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from ICheckDirectWay, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_CHECKDIRECTWAY(ky_class) \
	static Kaim::CheckDirectWayClass Class; \
	static Kaim::ICheckDirectWay* KyCreate(); \
	Kaim::CheckDirectWayClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from ICheckDirectWay, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_CHECKDIRECTWAY(ky_class,ky_base_class) \
	Kaim::CheckDirectWayClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::ICheckDirectWay* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::ICheckDirectWay* ptr = (Kaim::ICheckDirectWay*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	ICheckDirectWay is an abstract base class for a type of IModifier that the PathFinder uses to
	determine whether or not the direct way from the starting point to the destination point is clear
	of obstacles. If so, it also determines the cost of following that direct way, and whether or not
	the PathFinder should run an A* calculation and compare the cost of the direct way against the cost
	of the path through the Graph. 
	\section writingcustomicheckdirect Writing a custom ICheckDirectWay class
	Writing a custom class that derives from ICheckDirectWay involves the following considerations.
	\subsection icheckdirectconst Constructor
	All classes that derive from ICheckDirectWay must have a constructor that follows the #CheckDirectWayConstructor type definition. 
	\subsection icheckdirectvm Virtual methods
	All classes that derive from ICheckDirectWay must implement the CheckDirectWay() method.
	\subsection icheckdirectmetaclass MetaClass macros
	In order for \SDKName to recognize your ICheckDirectWay class, you must include the following macro definitions:
	-	#KY_DECLARE_CHECKDIRECTWAY: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_CHECKDIRECTWAY: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from ICheckDirectWay.
			For example, &Kaim::CheckDirectWay_Distance:Class. If your new class derives directly from ICheckDirectWay,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class ICheckDirectWay: public IModifier
{
public:
	/*!	\label_constructor */
	ICheckDirectWay(): IModifier() {}

	/*!	\label_getclass */
	virtual CheckDirectWayClass& GetClass() const = 0;

	/*!	The PathFinder calls this method at the beginning of the path calculation process, to determine whether
		or not a clear straight-line path exists to the destination point.
		\param[in] start				The starting point for the new path calculation, typically the current position 
										of the Bot using the PathFinder.
		\param[in] dest					The refined goal for the path calculation.
		\param[out] ignoreAStar			Indicates to the PathFinder what to do if a direct way is found.
										-	Set this value to #KY_TRUE in order to have the PathFinder follow the direct
											path immediately without conducting an A*.
										-	Set this value to #KY_FALSE in order to have the PathFinder conduct an A*
											calculation through the Graph, compare the cost of that path with the cost
											stored in the \c directWayCost parameter, and choose the option with the
											lower cost.
		\param[out] directWayCost		The cost of the direct way to the destination, read when \c ignoreAStar 
										is set to #KY_FALSE.
		\return #KY_TRUE if the direct way to the destination is clear of obstacles, or #KY_FALSE otherwise. If #KY_TRUE, the
				PathFinder will read \c ignoreAStar to determine whether or not it should run an A* and compare the
				cost of the direct way with the cost of following the path through the Graph. If #KY_FALSE, the PathFinder will
				always run an A* and follow the path it finds through the Graph (if a path exists). */
	virtual KyBool CheckDirectWay(const PointWrapper& start, const PointWrapper& dest, KyBool& ignoreAStar, KyFloat32& directWayCost) = 0;
};

} // namespace Kaim

#endif // KY_ICHECKDIRECTWAY_H

