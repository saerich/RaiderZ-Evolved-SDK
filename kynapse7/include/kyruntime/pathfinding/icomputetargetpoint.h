/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ICOMPUTETARGETPOINT_H
#define KY_ICOMPUTETARGETPOINT_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/core/pointwrapper.h>


namespace Kaim
{

class IPathFinder;
class IComputeTargetPoint;


///////////////////////////////////////////////////////////////////////////////////////////
// IComputeTargetPoint
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IComputeTargetPoint. Each class that derives from 
	IComputeTargetPoint must contain a constructor function that matches this type . The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef IComputeTargetPoint* (*ComputeTargetPointConstructor)();

/*!	ComputeTargetPointClass is the MetaClass for IComputeTargetPoint, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class ComputeTargetPointClass: public MetaClass<IComputeTargetPoint, ComputeTargetPointConstructor>
{
public:
	/*!	Constructor for an IComputeTargetPoint MetaClass. For internal use. */
	ComputeTargetPointClass(const char* className, ComputeTargetPointConstructor ctr,
		ComputeTargetPointClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from IComputeTargetPoint, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_COMPUTETARGETPOINT(ky_class) \
	static Kaim::ComputeTargetPointClass Class; \
	static Kaim::IComputeTargetPoint* KyCreate(); \
	Kaim::ComputeTargetPointClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from IComputeTargetPoint, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_COMPUTETARGETPOINT(ky_class,ky_base_class) \
	Kaim::ComputeTargetPointClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::IComputeTargetPoint* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IComputeTargetPoint* ptr = (Kaim::IComputeTargetPoint*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	IComputeTargetPoint is an abstract base class for a type of IModifier that the PathFinder uses to
	determine the point in space toward which it directs its Bot in the current frame. Implementations of 
	this class typically apply offsets to the position of the current PathNode, in order to change the
	way the PathFinder follows and smooths \Paths. 
	\pp For example, the IComputeTargetPoint_Trivial implementation of this class simply chooses the current 
	PathNode as the target point; when that PathNode is reached, the next PathNode becomes the new target point. 
	The IComputeTargetPoint_Shortcut implementation, by contrast, chooses as its target point the furthest point along the
	next edge of the Path that is directly reachable, resulting in a smoother trajectory for the Bot.
	\section writingcustomicomptarget Writing a custom IComputeTargetPoint class
	Writing a custom class that derives from IComputeTargetPoint involves the following considerations.
	\subsection icomptargetconst Constructor
	All classes that derive from IComputeTargetPoint must have a constructor that follows the #ComputeTargetPointConstructor type definition. 
	\subsection icomptargetvm Virtual methods
	All classes that derive from IComputeTargetPoint must implement the ComputeTargetPoint() method.
	\subsection igotometaclass MetaClass macros
	In order for \SDKName to recognize your IComputeTargetPoint class, you must include the following macro definitions:
	-	#KY_DECLARE_COMPUTETARGETPOINT: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_COMPUTETARGETPOINT: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from IComputeTargetPoint.
			For example, &Kaim::ComputeTargetPoint_Shortcut::Class. If your new class derives directly from IComputeTargetPoint,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class IComputeTargetPoint: public IModifier
{
public:
	/*!	\label_constructor */
	IComputeTargetPoint():
		IModifier() {}
		
	/*!	\label_getclass */
	virtual ComputeTargetPointClass& GetClass() const = 0;

	/*!	Computes a new target point for the Bot.
		\param currentPathNodePosition			The current PathNode being targeted by the PathFinder.
		\param[out] targetPoint					The new target point computed by this method.
		\return #KY_TRUE if the \c targetPoint parameter was successfully updated, or #KY_FALSE otherwise. If this method
				returns #KY_FALSE, the PathFinder::FindNextMove() method will also return #KY_FALSE and pathfinding will fail. */
	virtual KyBool ComputeTargetPoint(const PointWrapper& currentPathNodePosition, PointWrapper &targetPoint) = 0;
};

} // namespace Kaim

#endif // KY_ICOMPUTETARGETPOINT_H
