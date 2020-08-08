/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IDETECTPATHNODEREACHED_H
#define KY_IDETECTPATHNODEREACHED_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/core/pointwrapper.h>


namespace Kaim
{

class IPathFinder;
class IDetectPathNodeReached;


///////////////////////////////////////////////////////////////////////////////////////////
// IDetectPathNodeReached
///////////////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IDetectPathNodeReached. Each class that derives from 
	IDetectPathNodeReached must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef IDetectPathNodeReached* (*DetectPathNodeReachedConstructor)();

/*!	DetectPathNodeReachedClass is the MetaClass for IDetectPathNodeReached, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class DetectPathNodeReachedClass: public MetaClass<IDetectPathNodeReached, DetectPathNodeReachedConstructor>
{
public:
	/*!	Constructor for an IDetectPathNodeReached MetaClass. For internal use. */
	DetectPathNodeReachedClass(const char* className, DetectPathNodeReachedConstructor ctr,
		DetectPathNodeReachedClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from IDetectPathNodeReached, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_DETECTPATHNODEREACHED(ky_class) \
	static Kaim::DetectPathNodeReachedClass Class; \
	static Kaim::IDetectPathNodeReached* KyCreate(); \
	Kaim::DetectPathNodeReachedClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from IDetectPathNodeReached, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_DETECTPATHNODEREACHED(ky_class,ky_base_class) \
	Kaim::DetectPathNodeReachedClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::IDetectPathNodeReached* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IDetectPathNodeReached* ptr = (Kaim::IDetectPathNodeReached*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	DetectPathNodeReached is an abstract base class for a type of IModifier that the PathFinder 
	uses to determine whether or not its Bot has reached its current target PathNode. 
	When this modifier indicates that the current target PathNode has been reached, the 
	PathFinder automatically switches to the next PathNode in the Path.
	\pp Typical implementations of this interface declare the PathNode to be reached as soon as
	the Bot gets within a certain distance of the PathNode.
	\section writingcustomidpnr Writing a custom IDetectPathNodeReached class
	Writing a custom class that derives from IDetectPathNodeReached involves the following considerations.
	\subsection idpnrconst Constructor
	All classes that derive from IDetectPathNodeReached must have a constructor that follows the #DetectPathNodeReachedConstructor type definition. 
	\subsection idpnrvm Virtual methods
	All classes that derive from IDetectPathNodeReached must implement the DetectPathNodeReached() method.
	\subsection idpnrmetaclass MetaClass macros
	In order for \SDKName to recognize your IDetectPathNodeReached class, you must include the following macro definitions:
	-	#KY_DECLARE_DETECTPATHNODEREACHED: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_DETECTPATHNODEREACHED: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from IDetectPathNodeReached.
			For example, &Kaim::DetectPathNodeReached_Distance2d5::Class. If your new class derives directly from IDetectPathNodeReached,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_pathfinding */
class IDetectPathNodeReached: public IModifier
{
public:
	/*!	\label_constructor */
	IDetectPathNodeReached() :
		IModifier() {}
		
	/*!	\label_getclass */
	virtual DetectPathNodeReachedClass& GetClass() const = 0;

	/*!	The PathFinder calls this method to determine whether or not the Bot has reached
		its current PathNode.
		\param pos					The current position of the Bot.
		\param pathNodePosition		The position of the current PathNode.
		\return #KY_TRUE if the Bot has reached the current PathNode, or #KY_FALSE otherwise. */
	virtual KyBool DetectPathNodeReached(const PointWrapper& pos, const PointWrapper& pathNodePosition) = 0;
};

} // namespace Kaim

#endif // KY_IDETECTPATHNODEREACHED_H
