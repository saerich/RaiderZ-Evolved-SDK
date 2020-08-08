/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IOBSTACLEACCESSOR_H
#define KY_IOBSTACLEACCESSOR_H

#include <kyruntime/core/world.h>

#include <kypathdata/basesystem/config.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{

class ObstacleAccessorClass;
class ObstacleLayer;

/*!	IObstacleAccessor is an abstract base class for an object that sets up the
	ObstacleLayer maintained in the #m_obstaclesLayer class member with the
	outlines of dynamic obstacles it should manage.
	\pp The supplied implementation (ObstacleAccessor_PathObjectOutline) retrieves
	outlines of blocking \PathObjects from the PathObjectOutline assigned to that PathObject.
	You can use this supplied implementation or create your own custom implementation
	of IObstacleAccessor that retrieves your obstacle outlines from another source,
	such as pulling them directly from your \gameOrSim engine.
	\pp Note that the ObstacleManager passes the outlines of your \Obstacles directly to the ObstacleLayer
	without passing through an IObstacleAccessor.
	\section writingcustomobstacc Writing a custom IObstacleAccessor class
	Writing a custom class that derives from IObstacleAccessor involves the following considerations.
	\subsection obstaccconst Constructor
	All classes that derive from IObstacleAccessor must have a constructor that follows the #ObstacleAccessorConstructor type definition. 
	\subsection obstaccvm Virtual methods
	You may re-implement any or all virtual methods of the IObstacleAccessor class. Typically, the Update() method is implemented to
	manage the list of obstacles maintained in the ObstacleLayer, but this is not absolutely required.
	\subsection obstaccaddobst Managing obstacles to the ObstacleLayer
	Your ObstacleAccessor maintains a pointer to the ObstacleLayer for which it is responsible in its #m_obstaclesLayer
	member. When your ObstacleAccessor class needs to provide an obstacle outline to the ObstacleLayer, it should call the 
	ObstacleLayer::AddObstacle() method of its #m_obstaclesLayer member. This method returns an object of the 
	ObstacleLayer::ObstacleIt class that defines the obstacle within the list maintained by the ObstacleLayer.
	When you need to remove an obstacle from the ObstacleLayer, call its ObstacleLayer::RemoveObstacle() method. This method 
	requires as an argument only the ObstacleLayer::ObstacleIt object initially returned for this obstacle by the 
	ObstacleLayer::AddObstacle() method.
	\subsection obstaccmetaclass MetaClass macros
	In order for \SDKName to recognize your IObstacleAccessor class, you must include the following macro definitions:
	-	#KY_DECLARE_OBSTACLEACCESSOR: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_OBSTACLEACCESSOR: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from IObstacleAccessor.
			For example, &Kaim::ObstacleAccessor_PathObjectOutline::Class. If your new class derives directly from IObstacleAccessor,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_mesh */
class IObstacleAccessor: public VirtualMemObject
{
public:
	/*!	\label_baseconstructor */
	IObstacleAccessor(ObstacleLayer* layer): m_obstaclesLayer(layer) {}
	
	/*!	\label_getclass */
	virtual ObstacleAccessorClass& GetClass() const = 0;
	
	/*!	Updates the obstacle records in the ObstacleLayer. This method is called transparently by ObstacleLayer::Update() at each
		frame. */
	virtual void GetInput() = 0;

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
	ObstacleLayer* m_obstaclesLayer; /*!< Stores a pointer to the ObstacleLayer that uses this IObstacleAccessor. */
};


/*!	This macro must be included in the declaration of any class that derives
	from IObstacleAccessor, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_mesh */
#define KY_DECLARE_OBSTACLEACCESSOR(ky_class) \
static Kaim::ObstacleAccessorClass Class; \
static Kaim::IObstacleAccessor* KyCreate(Kaim::ObstacleLayer* layer); \
Kaim::ObstacleAccessorClass&  GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any class that derives
	from IObstacleAccessor, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_mesh */
#define KY_IMPLEMENT_OBSTACLEACCESSOR(ky_class,ky_base_class) \
	Kaim::ObstacleAccessorClass ky_class::Class(#ky_class,ky_class::KyCreate); \
	Kaim::IObstacleAccessor* ky_class::KyCreate(Kaim::ObstacleLayer* layer) \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::IObstacleAccessor* ptr = (Kaim::IObstacleAccessor*) KY_NEW_CLASS(ky_class)(layer); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from IObstacleAccessor. Each class that derives from 
	IObstacleAccessor must contain a constructor function that matches this type definition, and that passes
	its arguments on to the constructor function of its parent class. 
	\param layer			A pointer to the ObstacleLayer that is creating and maintaining this IObstacleAccessor.
	\ingroup kyruntime_mesh */
typedef IObstacleAccessor* (*ObstacleAccessorConstructor)(ObstacleLayer* layer);

/*!	ObstacleAccessorClass is the MetaClass for IObstacleAccessor, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_mesh */
class ObstacleAccessorClass: public MetaClass<IObstacleAccessor,ObstacleAccessorConstructor>
{
public:
	/*!	Constructor for an IObstacleAccessor MetaClass. For internal use. */
	ObstacleAccessorClass(const char* className, ObstacleAccessorConstructor ctr);
};

} // namespace Kaim

#endif // KY_IOBSTACLEACCESSOR_H
