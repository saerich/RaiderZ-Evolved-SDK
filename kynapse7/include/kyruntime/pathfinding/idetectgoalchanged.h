/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IDETECTGOALCHANDED_H
#define KY_IDETECTGOALCHANDED_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/core/pointwrapper.h>


namespace Kaim
{

class IPathFinder;
class IDetectGoalChanged;


///////////////////////////////////////////////////////////////////////////////////////////
// IDetectGoalChanged
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IDetectGoalChanged. Each class that derives from 
	IDetectGoalChanged must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef IDetectGoalChanged* (*DetectGoalChangedConstructor)();

/*!	IDetectGoalChangedClass is the MetaClass for IDetectGoalChanged, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class DetectGoalChangedClass: public MetaClass<IDetectGoalChanged, DetectGoalChangedConstructor>
{
public:
	/*!	Constructor for an IDetectGoalChanged MetaClass. For internal use. */
	DetectGoalChangedClass(const char* className, DetectGoalChangedConstructor ctr,
		DetectGoalChangedClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from IDetectGoalChanged, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_DETECTGOALCHANGED(ky_class) \
	static Kaim::DetectGoalChangedClass Class; \
	static Kaim::IDetectGoalChanged* KyCreate(); \
	Kaim::DetectGoalChangedClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from IDetectGoalChanged, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_DETECTGOALCHANGED(ky_class,ky_base_class) \
	Kaim::DetectGoalChangedClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::IDetectGoalChanged* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IDetectGoalChanged* ptr = (Kaim::IDetectGoalChanged*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	IDetectGoalChanged is an abstract base class for a type of IModifier that the PathFinder uses to
	determine whether or not the final destination position passed to the PathFinder::FindNextMove() method 
	has changed significantly. If the IDetectGoalChanged modifier determines that the goal has moved significantly, the 
	PathFinder re-computes the path.
	\section writingcustomidgc Writing a custom IDetectGoalChanged class
	Writing a custom class that derives from IDetectGoalChanged involves the following considerations.
	\subsection idgcconst Constructor
	All classes that derive from IDetectGoalChanged must have a constructor that follows the #DetectGoalChangedConstructor type definition. 
	\subsection idgcvm Virtual methods
	All classes that derive from IDetectGoalChanged must implement the HasGoalChanged() method.
	\subsection idgcmetaclass MetaClass macros
	In order for \SDKName to recognize your IDetectGoalChanged class, you must include the following macro definitions:
	-	#KY_DECLARE_DETECTGOALCHANGED: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_DETECTGOALCHANGED: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from IDetectGoalChanged.
			For example, &Kaim::DetectGoalChanged_Distance3d::Class. If your new class derives directly from IDetectGoalChanged,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class IDetectGoalChanged: public IModifier
{
public:
	/*!	\label_constructor */
	IDetectGoalChanged(): 
		IModifier() {}
		
	/*!	\label_getclass */
	virtual DetectGoalChangedClass& GetClass() const = 0;

	/*!	The PathFinder calls this method in order to determine whether a path re-calculation 
		is necessary for the Bot due to the displacement of its destination point.
		\param oldGoal			The destination point of the last path computation, or 
								(#KyFloat32MAXVAL,#KyFloat32MAXVAL,#KyFloat32MAXVAL) if no path has yet been computed.
		\param newGoal			The new destination point passed to PathFinder::FindNextMove().
		\return #KY_TRUE if the new destination is different enough from the previous destination to
				require a re-computation of the path. */
	virtual KyBool HasGoalChanged(const PointWrapper& oldGoal, const PointWrapper& newGoal) = 0;
};

} // namespace Kaim

#endif // KY_IDETECTGOALCHANDED_H
