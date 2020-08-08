/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ISTEERING_H
#define KY_ISTEERING_H

#include <kyruntime/pathfinding/imodifier.h>
#include <kyruntime/pathfinding/pathfinder.h>


namespace Kaim
{
/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from ISteering. Each class that derives from 
	ISteering must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef ISteering* (*SteeringConstructor)();


/*!	SteeringClass is the MetaClass for ISteering, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class SteeringClass: public MetaClass<ISteering, SteeringConstructor>
{
public:
	/*!	Constructor for an ISteering MetaClass. For internal use. */
	SteeringClass(const char* className, SteeringConstructor ctr, SteeringClass* baseClass);
};


/*!	This macro must be included in the declaration of any IModifier class that derives
	from ISteering, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_STEERING(ky_class) \
	static Kaim::SteeringClass Class; \
	static Kaim::ISteering* KyCreate(); \
	Kaim::SteeringClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from ISteering, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_STEERING(ky_class,ky_base_class) \
	Kaim::SteeringClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::ISteering* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::ISteering* ptr = (Kaim::ISteering*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	ISteering is an abstract base class for a type of IModifier that moves a Bot toward a specified position. 
	ISteering classes are typically invoked by an IGoto modifier in order to construct an Action that moves the 
	Bot toward its target point, taking into account the physical characteristics and limitations of the character.
	\section writingcustomisteering Writing a custom ISteering class
	Writing a custom class that derives from ISteering involves the following considerations.
	\subsection isteeringconst Constructor
	All classes that derive from ISteering must have a constructor that follows the #SteeringConstructor type definition. 
	\subsection isteeringvm Virtual methods
	All classes that derive from ISteering must implement the ComputeAction() method.
	\subsection isteeringmetaclass MetaClass macros
	In order for \SDKName to recognize your ISteering class, you must include the following macro definitions:
	-	#KY_DECLARE_STEERING: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_STEERING: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from ISteering.
			For example, &Kaim::Steering_SimpleBiped::Class. If your new class derives directly from ISteering,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class ISteering: public IModifier
{
public:
	/*!	\label_constructor */
	ISteering():
		IModifier() {}
		
	/*!	\label_getclass */
	virtual SteeringClass& GetClass() const = 0;

	/*!	The IGoto modifiers supplied with \SDKName call this method to generate an Action that 
		moves a Bot to its target point.
		\pp This method takes the target speed and target orientation computed by the PathFinder 
		as the optimal trajectory for the Bot to reach its target point in the current 
		frame, interprets those values according to the physical characteristics of the Bot to 
		determine how the Bot should respond, and saves the final motion characteristics that 
		should be applied to the Bot in the values of \ActionAttributes that will be applied 
		back to the \gameOrSim engine. These generated \ActionAttributes should be saved in the 
		Action passed to the method in its \c action parameter.
		\pp The ComputeAction() method may apply the target speed and target orientation passed in its 
		parameters directly to the Action as-is. Alternatively, you may change those values to reflect 
		the physical abilities of the character: e.g. capping rotational speeds, or disallowing certain
		movements based on the state of the Bot in the \gameOrSim. If you need to carry out this kind of
		interpretation, you can also do it at the point in your data flow in which you retrieve the
		values stored in the Bot::m_action member and apply the values of the \ActionAttributes back to
		the character in your \gameOrSim engine.
		\param entity				The Bot using the PathFinder.
		\param maxSpeed				The maximum speed allowed for the Bot.
		\param targetPoint			The target point to be reached.
		\param targetSpeed			The desired speed when the target point is reached.
		\param targetOrientation	The desired orientation when the target point is reached. If 
									#KY_NULL, no constraint is given concerning the orientation when 
									reaching the target point.
		\param[out] action			The Action to be filled by this method.
		\return #KY_TRUE if \c action was successfully updated, or #KY_FALSE otherwise. If #KY_FALSE is
				returned, pathfinding will fail and the PathFinder::FindNextMove() method will return
				#KY_FALSE. */
	virtual KyBool ComputeAction(const Bot& entity,  KyFloat32 maxSpeed,  
		const PointWrapper& targetPoint,  KyFloat32 targetSpeed, const Vec3f* targetOrientation,
		Action& action) = 0;

	/*!	This method is considered deprecated. You do not need to implement it or call it in your own code. */
	virtual KyBool GetTime(const Bot& entity, const PointWrapper& startingPosition, 
		KyFloat32 startingSpeed, const Vec3f& startingOrientation, KyFloat32 maxSpeed, 
		const PointWrapper& targetPoint, KyFloat32& targetSpeed, const Vec3f* targetOrientation,
		KyFloat32& time) = 0;
};

} // namespace Kaim

#endif // KY_ISTEERING_H
