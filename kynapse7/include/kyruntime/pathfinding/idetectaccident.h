/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IDETECTACCIDENT_H
#define KY_IDETECTACCIDENT_H

#include <kyruntime/pathfinding/imodifier.h>


namespace Kaim
{

class IPathFinder;
class IDetectAccident;


///////////////////////////////////////////////////////////////////////////////////////////
// IDetectAccident
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IDetectAccident. Each class that derives from 
	IDetectAccident must contain a constructor function that matches this type definitio. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef IDetectAccident* (*DetectAccidentConstructor)();

/*!	DetectAccidentClass is the MetaClass for IDetectAccident, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class DetectAccidentClass: public MetaClass<IDetectAccident, DetectAccidentConstructor>
{
public:
	/*!	Constructor for an IDetectAccident MetaClass. For internal use. */
	DetectAccidentClass(const char* className, DetectAccidentConstructor ctr, DetectAccidentClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from IDetectAccident, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_DETECTACCIDENT(ky_class) \
	static Kaim::DetectAccidentClass Class; \
	static Kaim::IDetectAccident* KyCreate(); \
	Kaim::DetectAccidentClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from IDetectAccident, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_DETECTACCIDENT(ky_class,ky_base_class) \
	Kaim::DetectAccidentClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::IDetectAccident* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IDetectAccident* ptr = (Kaim::IDetectAccident*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	IDetectAccident is an abstract base class for a type of IModifier that the PathFinder uses to
	determine whether its Bot has encountered a serious problem while following its path, or whether
	the Bot is following its path as expected. For example, an accident might be detected if the Bot 
	falls or is pushed into a hole from which it cannot climb out. 
	\pp When an accident is detected, the PathFinder automatically re-computes a new path from the Bot
	to its destination. This allows the Bot to recover from the accident by choosing a new path that responds
	to its current movement opportunities.
	\pp For example, in the image below, the Bot has a goal represented by the flagpole, and a path to that 
	goal that passes through point X. An accident occurs in the middle image that pushes the Bot to a 
	new location. The dotted red line indicates that the shortest line segment to the current target point 
	(X) is blocked, so a new path needs to be calculated. In this case, the new path takes the Bot 
	along a completely different route.
	\pp \image html "graphics/modifiers_idetectaccident.png"
	\pp To improve performance, the PathFinder does not call the IDetectAccident modifier at every frame. 
	Instead, it uses the modifier only when a variable time interval has elapsed. By default, the base period 
	for this interval is set to one second, but you can change this value by calling 
	PathFinder::SetAccidentDetectionPeriod(). This base period is doubled if no time remains for path 
	following in the current frame, in order to share CPU time more evenly with other \Bots. If you have 
	problems with your \Bots not responding quickly enough to accidents that throw them off course, you 
	may need to lower the base period. 
	\pp Note that, when a Bot is currently following a PathObject or using an ILpfShortcut modifier, the PathFinder 
	first calls the PathObject::DetectAccident() or ILpfShortcut::DetectAccident() method of the PathObject 
	or the ILpfShortcut modifier. These methods may conduct the accident detection themselves, overriding the 
	IDetectAccident modifier. If this is the case, the PathFinder does not call the IDetectAccident 
	modifier at all. 
	\section writingcustomidetectacc Writing a custom IDetectAccident class
	Writing a custom class that derives from IDetectAccident involves the following considerations.
	\subsection idetectaccconst Constructor
	All classes that derive from IDetectAccident must have a constructor that follows the #DetectAccidentConstructor type definition. 
	\subsection idetectaccvm Virtual methods
	All classes that derive from IDetectAccident must implement the DetectAccident() method.
	\pp You may also re-implement any of the other virtual methods, or use the default implementations if you prefer.
	\subsection idetectaccmetaclass MetaClass macros
	In order for \SDKName to recognize your IDetectAccident class, you must include the following macro definitions:
	-	#KY_DECLARE_DETECTACCIDENT: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_DETECTACCIDENT: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from IDetectAccident.
			For example, &Kaim::DetectAccident_PredictMove::Class. If your new class derives directly from IDetectAccident,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class IDetectAccident: public IModifier
{
public:

	/*!	\label_constructor */
	IDetectAccident(): IModifier() {}
	
	/*!	\label_getclass */
	virtual DetectAccidentClass& GetClass() const = 0;

	/*!	Indicates to the PathFinder whether or not an accident has been detected.
		\return #KY_TRUE if an accident has occurred and the PathFinder should re-compute the
				Path for the Bot, or #KY_FALSE if the Bot is following its Path as expected. */
	virtual KyBool DetectAccident() = 0;

	/*!	The PathFinder calls this method at the end of the FindNextMove() method in order
		to update any variables that may be required by the next call to DetectAccident(). */
	virtual void Update() {}
};

} // namespace Kaim

#endif // KY_IDETECTACCIDENT_H
