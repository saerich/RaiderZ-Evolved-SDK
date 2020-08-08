/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file 
	\ingroup kyruntime_obstaclemanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_OBSTACLLE_H
#define KY_OBSTACLLE_H


#include <kypathdata/math/vec3f.h>
#include <kyruntime/obstaclemanagement/obstacleoutline.h>
#include <kyruntime/obstaclemanagement/projectedoutline.h>
#include <kyruntime/obstaclemanagement/shapeobb.h>
#include <kyruntime/obstaclemanagement/shapepolyline.h>


namespace Kaim
{

class ObstacleManager;


/*! Enumerates the possible controls an Obstacle has over how precisely its outline should be
	projected by the ObstacleManager. Passed in a call to Obstacle::Update().
	\ingroup kyruntime_obstaclemanagement */
enum OutlineUpdatePolicy
{
	OutlineUpdatePolicy_Approximate = 0, /*!< Indicates that the Obstacle outline may be altered slightly if needed. */
	OutlineUpdatePolicy_Exact, /*!< Indicates that the Obstacle outline should remain exact. */
	OutlineUpdatePolicy_dummy_forceenumsizeint = KYFORCEENUMSIZEINT
};

/*! Enumerates the possible controls an Obstacle has over how quickly the ObstacleManager should process
	updates. Passed in a call to Obstacle::Update().
	\ingroup kyruntime_obstaclemanagement */
enum ObstacleUpdateDelay
{
	ObstacleUpdateDelay_Delayed = 0, /*!< Indicates that the ObstacleManager may defer projecting this Obstacle until a later frame, if the ObstacleManager has already exceeded the maximum number of \Obstacles it is allowed to project in a single frame. */
	ObstacleUpdateDelay_Immediate, /*!< Indicates that the ObstacleManager must update the projection of this Obstacle immediately. */
	ObstacleUpdateDelay_dummy_forceenumsizeint = KYFORCEENUMSIZEINT
};

/*! Each instance of the Obstacle class represents a dynamic, physical object in your \gameOrSim engine that should block your
	Bots from moving freely through its bounding box. Typical examples are crates, furniture, moveable or destructible 
	walls, rubble, vehicles, etc.
	\pp \Obstacles are intended to be created as interface objects: i.e. you create instances of the Obstacle class yourself
	in your \gameOrSim engine as needed, and register them with a \SDKName World when you want them to be taken into account
	by the \Bots in that World. 
	-	Typically, the lifespan of an Obstacle is tied in some way to the particular object in the \gameOrSim engine
		that it represents: for example, you may add an Obstacle class member to the class that represents a crate in your
		\gameOrSim engine. When the crate is created and destroyed, you create and destroy its Obstacle too. 
	-	When you need the \Bots in a World to take the crate into account in their pathfinding, you register the Obstacle
		with the ObstacleManager in the World by calling ObstacleManager::RegisterObstacle(), and activate the Obstacle
		by calling Obstacle::Activate(). 
	-	When the obstacle in the \gameOrSim engine moves, rotates, or changes its bounding box, you call Obstacle::Update()
		to notify the ObstacleManager of the change.
	\pp Note that each instance of Obstacle can only be added to one World at a time. If you need an object in your 
	\gameOrSim engine to be projected in more than one World at a time, you need to create an Obstacle for each World.
	\pp For more information, see :USERGUIDE:"Using Obstacles".
	\ingroup kyruntime_obstaclemanagement */
class Obstacle
{
	KY_DEFINE_NEW_DELETE_OPERATORS


public:
	/*! \label_constructor */
	Obstacle() :
		m_manager(KY_NULL),
		m_layer(KY_NULL),
		m_layerObsIt(),
		m_exactOutline(),
		m_projectedOutline(),
		m_velocity(),
		m_updatePolicy(OutlineUpdatePolicy_Approximate),
		m_atRestDate(0.0f),
		m_atRest(KY_FALSE),
		m_userData(KY_NULL)
	{}

	/*! \label_destructor */
	~Obstacle();

	// Update interface

	/*! Notifies the ObstacleManager in charge of this Obstacle that it should begin taking this
		Obstacle into account. The ObstacleManager will start projecting the outline of this Obstacle
		into the NavMesh starting in the next frame. */
	KyResult Activate();

	/*! Notifies the ObstacleManager in charge of this Obstacle that it should temporarily stop taking this
		Obstacle into account. The ObstacleManager will remove the projected outline of the Obstacle, and
		will not process any further updates to the Obstacle outline until the Obstacle is re-enabled through
		a call to Activate(). */
	KyResult Deactivate();

	/*! Updates the current shape, velocity and update criteria for the Obstacle.
		\param shapeObb				An oriented bounding box that represents the shape of the obstacle in the \gameOrSim engine.
		\param velocity				The scalar speed of the obstacle in the \gameOrSim engine. Stored in #m_velocity.
		\param updatePolicy			An element from the #OutlineUpdatePolicy enumeration that indicates the fidelity
									with which the projection of this Obstacle in the NavMesh should match its actual
									bounding box. Stored in #m_updatePolicy.
		\param updateDelay			An element from the #ObstacleUpdateDelay enumeration that indicates how rapidly
									the ObstacleManager should deal with this update.
		\return A #KyResult that indicates the success or failure of the request. If the update succeeds, the Obstacle
				will be re-projected in the following frame (or later, if \c updateDelay
				permits delaying the request). If the update fails, this Obstacle may not be correctly registered with an
				ObstacleManager. */
	KyResult Update(const ShapeOBB& shapeObb, const Vec3f& velocity,
		OutlineUpdatePolicy updatePolicy=OutlineUpdatePolicy_Approximate,
		ObstacleUpdateDelay updateDelay=ObstacleUpdateDelay_Delayed);

	/*! Updates the current shape, velocity and update criteria for the Obstacle.
		\param shapePolyline		An extruded polyline that represents the shape of the obstacle in the \gameOrSim engine.
		\param velocity				The scalar speed of the obstacle in the \gameOrSim engine. Stored in #m_velocity.
		\param updatePolicy			An element from the #OutlineUpdatePolicy enumeration that indicates the fidelity
									with which the projection of this Obstacle in the NavMesh should match its actual
									bounding box. Stored in #m_updatePolicy.
		\param updateDelay			An element from the #ObstacleUpdateDelay enumeration that indicates how rapidly
									the ObstacleManager should deal with this update.
		\return A #KyResult that indicates the success or failure of the request. If the update succeeds, the Obstacle
				will be re-projected in the following frame (or later, if \c updateDelay
				permits delaying the request). If the update fails, this Obstacle may not be correctly registered with an
				ObstacleManager. */
	KyResult Update(const ShapePolyline& shapePolyline, const Vec3f& velocity,
		OutlineUpdatePolicy updatePolicy=OutlineUpdatePolicy_Approximate,
		ObstacleUpdateDelay updateDelay=ObstacleUpdateDelay_Delayed);

	/*! For internal use by the ObstacleManager. */
	KyResult SetManager(ObstacleManager* manager);

	/*! For internal use by the ObstacleManager. */
	KyResult UnsetManager()
	{
		m_manager = KY_NULL;
		return KY_SUCCESS;
	}

	/*! Sets the value stored in #m_userData. */
	void SetUserData(void* userData);

	/*! Retrieves the value stored in #m_userData. */
	void* GetUserData() {
		return m_userData;
	}


public:
	/*! A pointer to the ObstacleManager responsible for this Obstacle. Do not modify. */
	ObstacleManager* m_manager;

	/*! The ObstacleLayer this Obstacle is currently projected into. For internal use. Do not modify. */
	ObstacleLayer* m_layer; // We keep this info there to accelerate activity management

	/*! An iterator that points to this Obstacle within the list of Obstacles in its ObstacleLayer. 
		For internal use. Do not modify. */
	ObstacleLayer::ObstacleIt m_layerObsIt; // We keep this info there to accelerate activity management

	/*! A two-dimensional polyline with vertical thickness, calculated transparently from the shape provided in the 
		last call to Update(). Do not modify. */
	ObstacleOutline m_exactOutline;

	/*! A two-dimensional polyline with vertical thickness that is projected into the NavMesh. Calculated by the 
		IOutlineUpdatePolicy set for the ObstacleManager. Do not modify. */
	ProjectedOutline m_projectedOutline;

	/*! The scalar velocity of the Obstacle set in the last call to Update(). */
	Vec3f m_velocity;

	/*! The update policy set in the last call to Update(). */
	OutlineUpdatePolicy m_updatePolicy;

	/*! The date at which the Obstacle will be considered to be at rest. 
		Managed by OutlineUpdatePolicy_MovementAnalysis. Do not modify. */
	KyFloat32 m_atRestDate;

	/*! Indicates whether or not the Obstacle is currently at rest.  
		Managed by OutlineUpdatePolicy_MovementAnalysis. Do not modify. */
	KyBool m_atRest;

protected:
	/*! Stores user-defined data about the Obstacle, typically used to identify the corresponding object in the \gameOrSim
		engine. When you use the NavMeshQueryServerWrapper to conduct a collision test that hits an object in an ObstacleLayer,
		you can retrieve this value from a NavMeshLayerLocation object returned by the collision test, and use it to
		identify which object in the \gameOrSim engine caused the collision. */
	void* m_userData;
};

} // Kaim

#endif // KY_OBSTACLLE_H
