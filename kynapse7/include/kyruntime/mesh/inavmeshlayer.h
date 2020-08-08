/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_INAVMESHLAYER_H
#define KY_INAVMESHLAYER_H

#include <kyruntime/core/world.h>

#include <kypathdata/containers/list.h>
#include <kypathdata/basesystem/config.h>
#include <kypathdata/generic/memory.h>
#include <kypathdata/math/vec3f.h>
#include <kypathdata/navmesh/navmeshtypes.h>

#include <kyruntime/mesh/navmeshlayerlocation.h>

namespace Kaim
{

class NavMeshLayerClass;
class NavMeshLayerLocation;
class NavMeshLayerManager;
class ObstacleProjectionVolume;


/*! INavMeshLayer provides an abstract base class for a dynamic layer that affects the reachability of locations in a NavMesh.
	Its typical use is to add dynamically moving obstacles or areas to a NavMesh, to block \Bots from moving through those areas.
	\pp This class cannot be used as-is; you must use a class that derives from INavMeshLayer and that implements its virtual
	methods. Most projects will use the ObstacleLayer implementation, which is integrated into many other \SDKName pathfinding
	components, such as the CanGo_AiMesh modifier used by the PathFinder, and the Obstacle management system.
	\pp You can create your own implementation of INavMeshLayer. However, your INavMeshLayer cannot be tied into the \SDKName
	pathfinding system. You will therefore be able to use your own implementation only for custom perception queries that you
	conduct directly against your layer itself or against the NavMeshQueryServerWrapper. It is therefore recommended that if you
	need to create a custom implementation of INavMeshLayer, you should derive your class from ObstacleLayer instead.
	\pp INavMeshLayer objects are created and maintained by the NavMeshLayerManager class of WorldService.
	\section writingcustomnavmeshlayer Writing a custom INavMeshLayer class
	Writing a custom class that derives from INavMeshLayer involves the following considerations.
	\subsection navmeshlayerconst Constructor
	All classes that derive from INavMeshLayer must have a constructor that follows the #NavMeshLayerConstructor type definition. 
	\subsection navmeshlayervm Virtual methods
	All classes that derive from INavMeshLayer must implement the CanGo(), TraceLine(), GetAllLocations() and IsValidPosition() methods.
	\subsection navmeshlayeremetaclass MetaClass macros
	In order for \SDKName to recognize your INavMeshLayer class, you must include the following macro definitions:
	-	#KY_DECLARE_NAVMESHLAYER: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_NAVMESHLAYER: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from INavMeshLayer.
			For example, &Kaim::ObstacleLayer::Class. If your new class derives directly from INavMeshLayer,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_mesh */
class INavMeshLayer : public VirtualMemObject
{
public:
	/*! \label_baseconstructor */
	INavMeshLayer(NavMeshLayerManager& manager, const char* name);

	/*! \label_virtualdestructor */
	virtual ~INavMeshLayer();

	/*! Retrieves the unique name for this INavMeshLayer set in the WorldDefinition. */
	const char* GetName() const { return m_name; }

	/*! \label_getclass */
	virtual NavMeshLayerClass& GetClass() const = 0;

	/*! Updates the state of the INavMeshLayer to match the current state of the obstacles it represents in the \gameOrSim world.
		Called automatically at each frame by the NavMeshLayerManager. */
	virtual void Update() {}

	/*! Determines whether or not a Bot with a specified radius can move in a straight line from a specified starting point to a specified
		ending point without being stopped by objects in this INavMeshLayer.
		\param navFloorPtr				Identifies the NavFloor related to the collision test.
		\param startPoint				The starting point of the collision test.
		\param destPoint				The ending point of the collision test.
		\param radius					The radius of the Bot moving from \c startPoint to \c destPoint.
		\return #KY_FALSE if the obstacles in the INavMeshLayer block the Bot from moving from the start point to the end point,
				or #KY_TRUE otherwise. */
	virtual KyBool CanGo(const NavFloorPtr& navFloorPtr, const Vec3f& startPoint, const Vec3f& destPoint, KyFloat32 radius) const = 0;

	/*! Determines whether or not a Bot with a specified radius can move in a straight line from a specified starting point to a specified
		ending point without being stopped by objects in this INavMeshLayer, and provides access to the point at which the first collision
		was detected (if any).
		\param navFloorPtr				Identifies the NavFloor related to the collision test.
		\param startPoint				The starting point of the collision test.
		\param destPoint				The ending point of the collision test.
		\param radius					The radius of the Bot moving from \c startPoint to \c destPoint.
		\param[out] lastValidPoint		The coordinates of the last valid point for a Bot with the specified radius along the line segment
		\param[out] location			A NavMeshLayerLocation object that can be used to identify the obstacle hit by the collision test. Its
										NavMeshLayerLocation::m_layer member is set to point to this INavMeshLayer, and its
										NavMeshLayerLocation::m_userData member is set to a value that identifies the obstacle hit by the collision test.
										See the NavMeshLayerLocation class description for details on how to identify the obstacle from the data stored
										in the NavMeshLayerLocation.
		\return #KY_FALSE if the obstacles in your INavMeshLayer should prevent the Bot from moving from the start point to the end point,
				or #KY_TRUE otherwise. */
	virtual KyBool TraceLine(const NavFloorPtr& navFloorPtr, const Vec3f& startPoint, const Vec3f& destPoint, KyFloat32 radius,
								Vec3f& lastValidPoint, NavMeshLayerLocation* location) const = 0;

	/*! Retrieves information about all points along the specified line segment at which a collision is detected (if any) for a Bot with the
		specified radius that moves from \c startPoint to \c destPoint.
		\param navFloorPtr				Identifies the NavFloor related to the collision test.
		\param startPoint				The starting point of the collision test.
		\param destPoint				The ending point of the collision test.
		\param radius					The radius of the Bot moving from \c startPoint to \c destPoint.
		\param[out] locations			An array of NavMeshLayerLocation objects that can be used to identify the obstacles hit by the collision test.
										For each collision that was detected, this creates a NavMeshLayerLocation, with its NavMeshLayerLocation::m_layer
										member set to point to this INavMeshLayer, and its NavMeshLayerLocation::m_userData member set to a value that
										identifies the obstacle hit by the collision test. See the NavMeshLayerLocation class description for details
										on how to identify the obstacle from the data stored in the NavMeshLayerLocation. 
										This NavMeshLayerLocation object is then inserted into the \c locations array. This method never removes any
										pre-existing objects in the \c locations array.
		\return #KY_TRUE if the \c locations array was successfully updated with the collision points detected by this method, or #KY_FALSE otherwise. */
	virtual KyBool GetAllLocations(const NavFloorPtr& navFloorPtr, const Vec3f& startPoint, const Vec3f& destPoint, KyFloat32 radius,
									NavMeshLayerLocationList& locations) const = 0;

	/*! Determines whether or not a specified point is inside an obstacle or forbidden area in the INavMeshLayer.
		\param navFloorPtr				Identifies the NavFloor related to the test.
		\param pos						The coordinates to test.
		\param[out] location			A NavMeshLayerLocation object that identifies the obstacle that contains \c pos, if any. Its
										NavMeshLayerLocation::m_layer member is set to point to this INavMeshLayer, and its
										NavMeshLayerLocation::m_userData member is set to a value that identifies the obstacle hit by the collision test.
										See the NavMeshLayerLocation class description for details on how to identify the obstacle from the data stored
										in the NavMeshLayerLocation.
		\return #KY_TRUE if \c pos is a valid position outside of all obstacles and forbidden areas in this INavMeshLayer, or #KY_FALSE
				if the \c location parameter was updated with the obstacle that contains \c pos. */
	virtual KyBool IsValidPosition(const NavFloorPtr& navFloorPtr, const Vec3f& pos, NavMeshLayerLocation* location) const = 0;

	/*! \label_getworld */
	const World& GetWorld() const;

	/*! \label_getworld */
	World& GetWorld();


	/************************************************************************/
	/* ObstacleProjectionVolumes                                            */
	/************************************************************************/
	/*! For internal use. */
	virtual KyResult RegisterObstacleProjectionVolume(ObstacleProjectionVolume* /*obstacleProjectionVolume*/) {return KY_SUCCESS;}
	/*! For internal use. */
	virtual KyResult UnregisterObstacleProjectionVolume(ObstacleProjectionVolume* /*obstacleProjectionVolume*/) {return KY_SUCCESS;}

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
	char* m_name;
	NavMeshLayerManager* m_manager;
};

typedef List<INavMeshLayer*> INavMeshLayerPtrList; /*!< Represents a List that contains pointers to INavMeshLayer instances. */


/*! This macro must be included in the declaration of any class that derives from INavMeshLayer, to set up the MetaClass
	used by the \SDKName object factory.
	\ingroup kyruntime_mesh */
#define KY_DECLARE_NAVMESHLAYER(ky_class) \
static Kaim::NavMeshLayerClass Class; \
static Kaim::INavMeshLayer* KyCreate(Kaim::NavMeshLayerManager& manager, const char* name); \
Kaim::NavMeshLayerClass& GetClass() const { return ky_class::Class; }

/*! This macro must be included in the implementation of any class that derives from INavMeshLayer, to set up the MetaClass
	used by the \SDKName object factory.
	\ingroup kyruntime_mesh */
#define KY_IMPLEMENT_NAVMESHLAYER(ky_class, ky_base_class) \
	Kaim::NavMeshLayerClass ky_class::Class(#ky_class, ky_class::KyCreate, ky_base_class); \
	Kaim::INavMeshLayer* ky_class::KyCreate(Kaim::NavMeshLayerManager& manager, const char* name) \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::INavMeshLayer* ptr = (Kaim::INavMeshLayer*) KY_NEW_CLASS(ky_class)(manager, name); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from INavMeshLayer. Each class that derives from 
	INavMeshLayer must contain a constructor function that matches this type definition, and that passes
	its arguments on to the constructor function of its parent class.
	\param manager			The NavMeshLayerManager Service that is creating and maintaining this INavMeshLayer.
	\param name				A unique name for the INavMeshLayer, derived from the WorldDefinition.
	\ingroup kyruntime_mesh */
typedef INavMeshLayer* (*NavMeshLayerConstructor)(NavMeshLayerManager& manager, const char* name);

/*! NavMeshLayerClass is the MetaClass for INavMeshLayer, created and used internally by the \SDKName object factory.
	\ingroup core */
class NavMeshLayerClass: public MetaClass<INavMeshLayer, NavMeshLayerConstructor>
{
public:
	/*! Constructor for the INavMeshLayer MetaClass. For internal use. */
	NavMeshLayerClass(const char* className, NavMeshLayerConstructor ctr, NavMeshLayerClass* baseClass);
};

} // Kaim

#endif // KY_INAVMESHLAYER_H
