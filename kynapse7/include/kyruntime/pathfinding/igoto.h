/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IGOTO_H
#define KY_IGOTO_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/core/pointwrapper.h>
#include <kyruntime/core/action.h>
#include <kyruntime/core/inextmovemanager.h>


namespace Kaim
{

class IPathFinder;
class IGoto;


///////////////////////////////////////////////////////////////////////////////////////////
// IGoto
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IGoto. Each class that derives from 
	IGoto must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef IGoto* (*GotoConstructor)();

/*!	GotoClass is the MetaClass for IGoto, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class GotoClass: public MetaClass<IGoto, GotoConstructor>
{
public:
	/*!	Constructor for an IGoto MetaClass. For internal use. */
	GotoClass(const char* className, GotoConstructor ctr, GotoClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from IGoto, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_GOTO(ky_class) \
	static Kaim::GotoClass Class; \
	static Kaim::IGoto* KyCreate(); \
	Kaim::GotoClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from IGoto, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_GOTO(ky_class,ky_base_class) \
	Kaim::GotoClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::IGoto* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IGoto* ptr = (Kaim::IGoto*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	IGoto is an abstract base class for a type of IModifier that the PathFinder uses to
	direct an Bot toward a target point, and to evaluate the cost of moving to a target point. 
	\pp Classes that derive from IGoto typically control small-scale movements the Bot makes as it travels along its calculated 
	path. For example, these modifiers are used to implement different types of dynamic avoidance to steer \Bots around
	other nearby \Bots. 
	See also :USERGUIDE:"Avoiding Other Bots and Bodies".
	\section writingcustomigoto Writing a custom IGoto class
	Writing a custom class that derives from IGoto involves the following considerations.
	\subsection igotoconst Constructor
	All classes that derive from IGoto must have a constructor that follows the #GotoConstructor type definition. 
	\subsection igotovm Virtual methods
	All classes that derive from IGoto must implement the Goto() method.
	\pp You may also re-implement any of the other virtual methods, or use the default implementations if you prefer.
	\subsection igotometaclass MetaClass macros
	In order for \SDKName to recognize your IGoto class, you must include the following macro definitions:
	-	#KY_DECLARE_GOTO: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_GOTO: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from IGoto.
			For example, &Kaim::Goto_GapDynamicAvoidance::Class. If your new class derives directly from IGoto,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class IGoto: public IModifier
{
public:
	/*!	\label_constructor */
	IGoto():
		IModifier() {}
		
	/*!	\label_getclass */
	virtual GotoClass& GetClass() const = 0;

	/*!	The PathFinder calls this method in order to generate an Action that moves the 
		Bot from its current position toward its target point.
		\param targetPoint			The target point the Bot should reach.
		\param[out] action			Stores the Action computed by this method.
		\return #KY_TRUE if the \c action parameter has been successfully updated, or #KY_FALSE otherwise.
				If this method returns #KY_FALSE, PathFinder::FindNextMove() will also return #KY_FALSE
				and pathfinding will fail. */
	virtual KyBool Goto(const PointWrapper& targetPoint, Action& action) = 0;

	/*!	The PathFinder calls this method in order to determine the cost of moving along a
		line segment between two specified points, using a specified IConstraint.
		\pp The default implementation of this method calculates the cost as follows:
		-	If the tested segment represents an edge that is linked to a PathObject, the returned 
			cost is the cost calculated by the PathObject::GetCost() method of the first PathObject linked 
			to that edge.
		-	Else, if the starting point passed to this method lies near the current position of the 
			Bot, the returned cost is the cost calculated by the PathObject::GetCost() method of the 
			current PathObject in use by the PathFinder, if any.
		-	Else, the returned cost is the cost calculated by the IConstraint::GetCost() method of the 
			IConstraint provided in the \c constraint parameter. 
		\pp \param bot				The Bot using the PathFinder.
		\param from					The starting point of the line segment.
		\param to					The ending point of the line segment.
		\param constraint			The IConstraint instance currently in use by the PathFinder.
		\param[out] cost			The cost of moving from \c begin to \c end.
		\return #KY_TRUE if the Bot can move along the line segment and the \c cost parameter was 
				successfully updated, or #KY_FALSE otherwise. */
	virtual KyBool GetCost(Bot* bot, const PointWrapper& from,
		const PointWrapper& to, IConstraint& constraint, KyFloat32& cost);
};

} // namespace Kaim

#endif // KY_IGOTO_H
