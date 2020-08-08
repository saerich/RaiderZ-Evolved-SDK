/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_bodyinfomanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BODYINFOFILTER_H
#define KY_BODYINFOFILTER_H

#include <kyruntime/bodyinfomanagement/bodyinfo.h>

#include <kyruntime/core/bot.h>

#include <kypathdata/generic/memory.h>


namespace Kaim
{

class BodyInfoFilterClass;


/////////////////////////////////////////////////////////////////////////////////
// BodyInfoFilter
/////////////////////////////////////////////////////////////////////////////////

/*!	BodyInfoFilter is the base class for dynamic filters that can forbid the BodyInfoManager
	from spending CPU time computing BodyInfo values in certain circumstances. For example,
	you may want to use an BodyInfoFilter to prevent computing a BodyInfo when the distance between 
	a pair of \Bodies or the distance between a Body and a point of interest is too large,
	when a certain BodyInfo is guaranteed not to be needed for a given length of time due to 
	gameplay reasons, etc.
	\pp Some implementations of this class are provided with \SDKName, to forbid BodyInfo computations
	in certain common scenarios. You can also write your own BodyInfoFilter classes.
	\pp For more information on the BodyInfoManager, see :USERGUIDE:"Tracking Perception Data".
	\section usingcustombodyinfofilter Using a BodyInfoFilter
	\pp To create an instance of a BodyInfoFilter class, you call BodyInfoManager::RegisterFilter(). The lifespan of
	that BodyInfoFilter object is managed by the BodyInfoManager. You then assign the newly created BodyInfoFilter
	to one or more BodyInfo objects by calling their BodyInfo::AddFilter() methods.
	\section writingcustombodyinfofilter Writing a custom BodyInfoFilter class
	Writing a custom class that derives from BodyInfoFilter involves the following considerations.
	\subsection bodyinfofilterconst Constructor
	All classes that derive from BodyInfoFilter must have a constructor that follows the #BodyInfoFilterConstructor type definition. 
	\subsection bodyinfofiltervm Virtual methods
	\pp Each time the BodyInfoManager updates a BodyInfo, it consults each BodyInfoFilter that has been added to that BodyInfo
	in order to determine whether or not the computation of that BodyInfo should be forbidden as follows:
	-	First, the ForbidComputation(BodyInfo&) method of the BodyInfoFilter is called. If this method returns
		#KY_TRUE, the BodyInfo is not updated for any \Bodies in the BodyProfile.
	-	Before updating the BodyInfo value for each Bot, the ForbidComputation(BodyInfo&, const Body&) 
		method of the BodyInfoFilter is called. If this method returns #KY_TRUE, the BodyInfo is not updated for
		that Body.
	-	If updating an BodyInfo that derives from TwoBodiesInfo, the 
		ForbidComputation(BodyInfo&, const Body&, const Body&) method is then called. If this method
		returns #KY_TRUE, the BodyInfo is not updated for that pair of \Bodies.
	\pp If all these methods return #KY_FALSE, the BodyInfo is computed normally. These ForbidComputation() methods
	are virtual, so you can re-implement them in your custom class. You may re-implement any or all of these methods.
	All default implementations return #KY_FALSE, so if you want the BodyInfoFilter to ever prevent the computation 
	of the BodyInfo, you must re-implement at least one of these methods so that it returns #KY_TRUE in the desired
	set of circumstances.
	\subsection bodyinfofiltermetaclass MetaClass macros
	In order for \SDKName to recognize your BodyInfoFilter class, you must include the following macro definitions:
	-	#KY_DECLARE_BODYINFOFILTER: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_BODYINFOFILTER: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from BodyInfoFilter.
			For example, &Kaim::DistanceFilter::Class. If your new class derives directly from BodyInfoFilter,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_bodyinfomanagement */
class BodyInfoFilter: public Kaim::VirtualMemObject
{
public:
	/*!	\label_baseconstructor
		\param name			A unique name for the BodyInfoFilter, which is stored in the #m_name member. This name 
							must be unique among all BodyInfoFilter objects assigned to the same BodyProfile. */
	BodyInfoFilter(const char* name); 
	
	/*!	\label_virtualdestructor For internal use. */
	~BodyInfoFilter();
	
	/*!	\label_getclass */
	virtual BodyInfoFilterClass& GetClass() const = 0;
	
	/*!	Retrieves the unique name set for the BodyInfoFilter. */
	const char* GetName() const { return m_name; }

	/*!	Determines whether or not this BodyInfoFilter forbids computating the specified BodyInfo for all \Bodies.
		You can re-implement this method to prevent the BodyInfo from being computed under certain circumstances.
		\return #KY_TRUE to forbid computations for all \Bodies, or #KY_FALSE to allow computations.  */
	virtual KyBool ForbidComputation(BodyInfo& /*info*/) { return KY_FALSE; }

	/*!	Determines whether or not this BodyInfoFilter forbids computating the specified BodyInfo for the specified Body.
		You can re-implement this method to prevent the BodyInfo from being computed for Bodies that meet certain conditions.
		\return #KY_TRUE to forbid computations for the specified Body, or #KY_FALSE to allow the computation. */
	virtual KyBool ForbidComputation(BodyInfo& info, const Body& /*body1*/) { 
		if (ForbidComputation(info))
			return KY_TRUE;
		return KY_FALSE;
	}

	/*!	Determines whether or not this BodyInfoFilter forbids computating the specified BodyInfo for the specified pair
		of \Bodies. Called only for \BodyInfos that derive from TwoBodiesInfo.
		You can re-implement this method to prevent the BodyInfo from being computed if either or both of the \Bodies meet
		certain conditions.
		\return #KY_TRUE to forbid computations for the specified pair of \Bodies, or #KY_FALSE to allow the computation. */
	virtual KyBool ForbidComputation(BodyInfo& info, const Body& body1, 
		const Body& /*body2*/) { 
		if (ForbidComputation(info,body1))
			return KY_TRUE;
		return KY_FALSE;
	}

	/*! Determines whether or not this BodyInfoFilter forbids computating the specified BodyInfo for the specified Bot.
		You can re-implement this method to prevent the BodyInfo from being computed for \Bots that meet certain conditions.
		\return #KY_TRUE to forbid computations for the specified Bot, or #KY_FALSE to allow the computation. */
	KyBool ForbidComputation(BodyInfo& info, const Bot& entity1)
	{
		return ForbidComputation(info, entity1.GetBody());
	}

	/*!	Determines whether or not this BodyInfoFilter forbids computating the specified BodyInfo for the specified pair
		of \Bots. Called only for \BodyInfos that derive from TwoBodiesInfo.
		You can re-implement this method to prevent the BodyInfo from being computed if either or both of the \Bots meet
		certain conditions.
		\return #KY_TRUE to forbid computations for the specified pair of \Bots, or #KY_FALSE to allow the computation. */
	KyBool ForbidComputation(BodyInfo& info, const Bot& entity1, const Bot& entity2)
	{
		return ForbidComputation(info, entity1.GetBody(), entity2.GetBody());
	}

protected:
	char* m_name; /*!< Stores the unique name assigned to each instance of this class. Do not modify. */
};


/*!	This macro must be included in the declaration of any BodyInfoFilter class for the 
	BodyInfoManager, to set up the metaclass used by the \SDKName object factory.
	\ingroup kyruntime_bodyinfomanagement */
#define KY_DECLARE_BODYINFOFILTER( cls ) \
	static Kaim::BodyInfoFilterClass Class; \
	static Kaim::BodyInfoFilter* KyCreate(const char* name); \
	Kaim::BodyInfoFilterClass& GetClass() const { return cls::Class; }

/*!	This macro must be included in the implementation of any BodyInfoFilter class for the 
	BodyInfoManager, to set up the metaclass used by the \SDKName object factory.
	\ingroup kyruntime_bodyinfomanagement */
#define KY_IMPLEMENT_BODYINFOFILTER( cls, baseClass) \
	Kaim::BodyInfoFilterClass cls::Class( #cls, cls::KyCreate, baseClass); \
	Kaim::BodyInfoFilter* cls::KyCreate(const char* name) \
	{ \
		KY_LOG_MEMORY(cls::Class.GetName()); \
		Kaim::BodyInfoFilter* ptr = (Kaim::BodyInfoFilter*) KY_NEW_CLASS(cls)(name); \
		KY_REGISTER_METACLASS_VPTR(cls::Class, (*(void**)ptr)); \
		return ptr; \
	}


/////////////////////////////////////////////////////////////////////////////////
// BodyInfoFilterClass
/////////////////////////////////////////////////////////////////////////////////

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from BodyInfoFilter. Each class that derives from 
	BodyInfoFilter must contain a constructor function that matches this type definition, and that passes
	its arguments on to the constructor function of its parent class. 
	\param name			A unique name for the filter, which is stored in the BodyInfoFilter::m_name member. This name 
						must be unique among all filters assigned to the same BodyProfile.
	\ingroup kyruntime_bodyinfomanagement */
typedef BodyInfoFilter* (*BodyInfoFilterConstructor)(const char* name);

/*!	BodyInfoFilterClass is the MetaClass for BodyInfoFilter, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_bodyinfomanagement */
class BodyInfoFilterClass : public MetaClass<BodyInfoFilter, BodyInfoFilterConstructor>
{
public:
	/*!	Constructor for the BodyInfoFilter MetaClass. For internal use. */
	BodyInfoFilterClass(const char* className,
		BodyInfoFilterConstructor ctr,
		BodyInfoFilterClass* baseClass = KY_NULL);
};

} // namespace Kaim

#endif // KY_BODYINFOFILTER_H
