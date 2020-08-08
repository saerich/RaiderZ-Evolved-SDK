/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ICANGO_H
#define KY_ICANGO_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/core/pointwrapper.h>


namespace Kaim
{

class ICanGo;


///////////////////////////////////////////////////////////////////////////////////////////
// ICanGo
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from ICanGo. Each class that derives from 
	ICanGo must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef ICanGo* (*CanGoConstructor)();

/*!	CanGoClass is the MetaClass for ICanGo, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class CanGoClass: public MetaClass<ICanGo, CanGoConstructor>
{
public:
	/*!	Constructor for an ICanGo MetaClass. For internal use. */
	CanGoClass(const char* className, CanGoConstructor ctr, CanGoClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from ICanGo, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_CANGO(ky_class) \
	static Kaim::CanGoClass Class; \
	static Kaim::ICanGo* KyCreate(); \
	Kaim::CanGoClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from ICanGo, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_CANGO(ky_class,ky_base_class) \
	Kaim::CanGoClass ky_class::Class(#ky_class, ky_class::KyCreate, ky_base_class); \
	Kaim::ICanGo* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::ICanGo* ptr = (Kaim::ICanGo*) KY_NEW_CLASS(ky_class); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	ICanGo is an abstract base class for a type of IModifier that the PathFinder uses to
	determine whether or not its Bot can move in a straight line between two given positions in 
	3D space.
	\section writingcustomicango Writing a custom ICanGo class
	Writing a custom class that derives from ICanGo involves the following considerations.
	\subsection icangoconst Constructor
	All classes that derive from ICanGo must have a constructor that follows the #CanGoConstructor type definition. 
	\subsection icangivm Virtual methods
	All classes that derive from ICanGo must implement the CanGo(const PointWrapper&, const PointWrapper&, KyFloat32) and 
	TraceLine(const PointWrapper&, const PointWrapper&, KyFloat32, PointWrapper&) methods.
	\pp You may also re-implement any of the other virtual methods, or use the default implementations if you prefer.
	\subsection icangometaclass MetaClass macros
	In order for \SDKName to recognize your ICanGo class, you must include the following macro definitions:
	-	#KY_DECLARE_CANGO: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_CANGO: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from ICanGo.
			For example, &Kaim::CanGo_NavMesh::Class. If your new class derives directly from ICanGo,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class ICanGo: public IModifier
{
public:
	/*!	\label_constructor */
	ICanGo():
		IModifier() {}
		
	/*!	\label_getclass */
	virtual CanGoClass& GetClass() const = 0;

	// CanGo functions
	/*!	Indicates whether or not a Bot with the specified radius can pass freely along a line
		segment that extends between the two specified points.
		\param start				The start point of the line segment.
		\param dest					The end point of the line segment.
		\param radius				The radius of the Bot to be tested.
		\return #KY_TRUE if the Bot can follow the direct line from \c start to \c dest. */
	virtual KyBool CanGo(const PointWrapper& start, const PointWrapper& dest, KyFloat32 radius) = 0;

	/*!	Indicates whether or not a specified Bot can pass freely along a line segment that extends 
		between the two specified points.
		\param start				The start point of the line segment.
		\param dest					The end point of the line segment.
		\param bot					The Bot to be tested.
		\return #KY_TRUE if the Bot can follow the direct line from \c start to \c dest. */
	virtual KyBool CanGo(const PointWrapper& start, const PointWrapper& dest, const Bot& bot)
	{
		return CanGo(start, dest, bot.GetBody().GetRadius());
	}

	// TraceLine functions
	/*!	Similar to CanGo(), this method determines whether or not a Bot with the specified radius
		can pass freely along a line segment that extends between the two specified points. This method 
		also computes the last valid position along the line segment. If an obstacle is detected, this point 
		is expected to be the last point on that straight line before hitting the obstacle. If an obstacle 
		is not detected, this point is the same as the destination point. 
		\param startPoint			The start point of the line segment.
		\param destPoint			The end point of the line segment.
		\param radius				The radius of the Bot to be tested.
		\param lastValidPoint		The last valid point along the line segment that can be occupied by
									the Bot without colliding with an obstacle. */
	virtual void TraceLine(const PointWrapper& startPoint, const PointWrapper& destPoint,
		KyFloat32 radius, PointWrapper& lastValidPoint) = 0;

	/*!	Similar to CanGo(), this method determines whether or not a specified Bot can pass freely along a 
		line segment that extends between the two specified points. This method also computes the last valid 
		position along the line segment. If an obstacle is detected, this point is expected to be the last 
		point on that straight line before hitting the obstacle. If an obstacle is not detected, this point 
		is the same as the destination point. 
		\param startPoint			The start point of the line segment.
		\param destPoint			The end point of the line segment.
		\param bot					The Bot to be tested.
		\param lastValidPoint		The last valid point along the line segment that can be occupied by
									the Bot without colliding with an obstacle. */
	virtual void TraceLine(const PointWrapper& startPoint, const PointWrapper& destPoint,
		const Bot& bot, PointWrapper& lastValidPoint)
	{
		TraceLine(startPoint, destPoint, bot.GetBody().GetRadius(), lastValidPoint);
	}
};

} // namespace Kaim

#endif // KY_ICANGO_H
