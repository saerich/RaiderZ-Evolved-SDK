/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_INAVMESHACCESSOR_H
#define KY_INAVMESHACCESSOR_H

#include <kyruntime/mesh/inavmeshlayer.h>


namespace Kaim
{

class NavMeshAccessorClass;


/*!	INavMeshAccessor is an abstract base class for an object that sets up an ObstacleLayer with pointers to one or more \NavMeshes 
	within which the ObstacleLayer will project its obstacles. 
	\pp The INavMeshAccessor typically manages the \NavMeshes assigned to the ObstacleLayer by calling ObstacleLayer::AddMesh() 
	when a NavMesh becomes available, and ObstacleLayer::RemoveMesh() when a NavMesh should no longer be considered by the
	ObstacleLayer.
	\pp The supplied implementation, NavMeshAccessor_Database, should be suitable for use in most projects.
	\section writingcustomnavmeshacc Writing a custom NavMeshAccessor class
	Writing a custom class that derives from NavMeshAccessor involves the following considerations.
	\subsection navmeshaccconst Constructor
	All classes that derive from NavMeshAccessor must have a constructor that follows the #BotServiceConstructor type definition. 
	\subsection navmeshaccvm Virtual methods
	You may re-implement any or all virtual methods of the NavMeshAccessor class.
	\subsection navmeshaccmetaclass MetaClass macros
	In order for \SDKName to recognize your NavMeshAccessor class, you must include the following macro definitions:
	-	#KY_DECLARE_NAVMESHACCESSOR: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_NAVMESHACCESSOR: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from NavMeshAccessor.
			For example, &Kaim::NavMeshAccessor_Database::Class. If your new class derives directly from NavMeshAccessor,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_mesh */
class INavMeshAccessor: public VirtualMemObject
{
public:
	/*!	\label_getclass */
	virtual NavMeshAccessorClass& GetClass() const = 0;
	
	/*! \label_baseconstructor */
	INavMeshAccessor(INavMeshLayer* layer): m_layer(layer) {}


	/*!	Retrieves a pointer to the ObstacleLayer instance that uses this INavMeshAccessor object. */
	INavMeshLayer* GetLayer() { return m_layer; }

	/*!	\label_getworld */
	const World& GetWorld() const;

	/*!	\label_getworld */
	World& GetWorld();

	/*!	\label_getengine */
	Engine& GetEngine()
	{
		return GetWorld().GetEngine();
	}

	/*!	\label_getengine */
	const Engine& GetEngine() const
	{
		return GetWorld().GetEngine();
	}

protected:
	INavMeshLayer* m_layer;
};


/*!	This macro must be included in the declaration of any class that derives
	from INavMeshAccessor, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_mesh */
#define KY_DECLARE_NAVMESHACCESSOR(ky_class) \
static Kaim::NavMeshAccessorClass Class; \
static Kaim::INavMeshAccessor* KyCreate(Kaim::INavMeshLayer* layer); \
Kaim::NavMeshAccessorClass&  GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any class that derives
	from INavMeshAccessor, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_mesh */
#define KY_IMPLEMENT_NAVMESHACCESSOR(ky_class, ky_base_class) \
	Kaim::NavMeshAccessorClass ky_class::Class(#ky_class, ky_class::KyCreate, static_cast<Kaim::NavMeshAccessorClass*>(ky_base_class)); \
	Kaim::INavMeshAccessor* ky_class::KyCreate(Kaim::INavMeshLayer* layer) \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::INavMeshAccessor* ptr = (Kaim::INavMeshAccessor*) KY_NEW_CLASS(ky_class)(layer); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from INavMeshAccessor. Each class that derives from 
	INavMeshAccessor must contain a constructor function that matches this type definition, and that passes
	its arguments on to the constructor function of its parent class. 
	\param layer			A pointer to the INavMeshLayer that is creating and maintaining this INavMeshAccessor.
	\ingroup kyruntime_mesh */
typedef INavMeshAccessor* (*NavMeshAccessorConstructor)(Kaim::INavMeshLayer* layer);

/*!	NavMeshAccessorClass is the MetaClass for INavMeshAccessor, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_mesh */
class NavMeshAccessorClass: public MetaClass<INavMeshAccessor, NavMeshAccessorConstructor>
{
public:
	/*!	Constructor for an INavMeshAccessor MetaClass. For internal use. */
	NavMeshAccessorClass(const char* className, NavMeshAccessorConstructor ctr, NavMeshAccessorClass* baseClass = (NavMeshAccessorClass*)NULL);
};


} // namespace Kaim

#endif // KY_INAVMESHACCESSOR_H
