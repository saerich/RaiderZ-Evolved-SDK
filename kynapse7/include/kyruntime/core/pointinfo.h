/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTINFO_H
#define KY_POINTINFO_H

#include <kypathdata/generic/memory.h>
#include <kyruntime/core/metaclass.h>


namespace Kaim
{

class PointInfoClass;


/*!	PointInfo is the base class for all types of additional information about 3D positions
	that can be added to an instance of PointWrapper.
	\pp Some ready-to use derivations of PointInfo are supplied in the \SDKName SDK. If you need to associate
	a different type of information to a Vec3f, you can write your own custom class that derives from PointInfo,
	create an instance of your class, and add that instance to the PointWrapper.
	\section pointinfocustom Writing a custom PointInfo class
	Writing a custom class that derives from PointInfo involves the following considerations.
	\subsection pointinfoconst Constructor
	All classes that derive from PointInfo must have a constructor that follows the #PointInfoConstructor type definition. 
	\subsection pointinfovm Data members and accessors
	You may add any data members and accessors to your PointInfo class.
	\subsection pointinfometaclass MetaClass macros
	In order for \SDKName to recognize your PointInfo class, you must include the following macro definitions:
	-	#KY_DECLARE_POINTINFO: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_POINTINFO: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from PointInfo.
			For example, &Kaim::PointInfo_Body::Class. If your new class derives directly from PointInfo,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_core */
class PointInfo: public VirtualMemObject
{
public:
	/*!	\label_getclass */
	virtual PointInfoClass&  GetClass() const = 0;

	/*!	Resets the value held in the PointInfo. Typically implemented to reset the value to an invalid value. */
	virtual void Reset() = 0;
};

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from PointInfo. Each class that derives from 
	PointInfo must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_core */
typedef PointInfo* (*PointInfoConstructor)();

/*!	PointInfoClass is the MetaClass for PointInfo, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_core */
class PointInfoClass: public MetaClass<PointInfo, PointInfoConstructor>
{
public:
	/*!	Constructor for the PointInfo MetaClass. For internal use. */
	PointInfoClass(const char* className, PointInfoConstructor ctr, PointInfoClass* baseClass = KY_NULL);
};

/*!	This macro must be included in the declaration of any PointInfo class, to set up the
	\SDKName object factory.
	\ingroup kyruntime_core */
#define KY_DECLARE_POINTINFO(ky_class) \
	static Kaim::PointInfoClass Class; \
	Kaim::PointInfoClass&  GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any PointInfo class, to set up the
	\SDKName object factory.
	\ingroup kyruntime_core */
#define KY_IMPLEMENT_POINTINFO(ky_class,ky_base_class) \
	Kaim::PointInfoClass ky_class::Class(#ky_class,0,ky_base_class);

} // namespace Kaim

#endif // KY_POINTINFO_H
