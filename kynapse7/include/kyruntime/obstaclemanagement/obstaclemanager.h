/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_obstaclemanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_OBSTACLEMANAGER_H
#define KY_OBSTACLEMANAGER_H

#include <kyruntime/obstaclemanagement/obstacle.h>
#include <kyruntime/obstaclemanagement/outlineupdatepolicy_movementanalysis.h>

#include <kyruntime/core/service.h>

#include <kypathdata/containers/list.h>

namespace Kaim
{

class NavMeshLayerManager;
class ObstacleLayer;
class IOutlineUpdatePolicy;

/*!	The ObstacleManager is a class of WorldService that manages dynamic \Obstacles.
	\pp You create Obstacle objects in your \gameOrSim engine to represent dynamic obstacles that should block your
	Bots from moving freely. When you want an Obstacle represented in a \SDKName World, you register the Obstacle
	with the ObstacleManager through a call to RegisterObstacle().
	\pp The ObstacleManager adds each Obstacle into one of two possible \ObstacleLayers that you configure for the
	NavMeshLayerManager, which project the outline of the Obstacle into the NavMesh:
	-	It can project the Obstacle into an ObstacleLayer used by the Local PathFinding system. The LpfManager retrieves the
		outlines of the \Obstacles in this ObstacleLayer, and combines them into aggregates that \Bots can circle around.
		See also :USERGUIDE:"Local PathFinding".
	-	It can project the Obstacle into a "blocking" ObstacleLayer, which you identify to the CanGo_NavMesh modifier
		of the PathFinder using CanGo_NavMesh::AddBlockingLayer(). This is usually done for \Obstacles that are moving,
		in order to avoid expensive re-computations of Local PathFinding aggregates at each frame.
	\pp The ObstacleManager uses an object that derives from IOutlineUpdatePolicy to decide which ObstacleLayer each
	Obstacle should be projected into. 
	For details, see :USERGUIDE:"Using Obstacles".
	\ingroup kyruntime_obstaclemanagement */
class ObstacleManager: public WorldService
{
public:
	KY_DECLARE_WORLDSERVICE(ObstacleManager)

	/*! The ObstacleManager uses an instance of this class to record statistics about its managed \Obstacles. 
		You can access this instance through ObstacleManager::GetStatisticData().
		\ingroup kyruntime_obstaclemanagement */
	struct StatisticData
	{
	public:
		/*! \label_constructor For internal use. */
		StatisticData() :
			m_updateDuration(0.0f),
			m_obstaclesCount_ToUpdateThisFrame(0),
			m_obstaclesCount_UpdatedThisFrame(0),
			m_obstaclesCount_RemainingDelayed(0),
			m_obstaclesCount_ImmediateUpdated(0),
			m_obstaclesCount_ToRemoveThisFrame(0),
			m_obstaclesCount_RemovedThisFrame(0)
		{}
		/*! \label_clear */
		void Clear()
		{
			m_updateDuration = 0.0f;
			m_obstaclesCount_ToUpdateThisFrame = 0;
			m_obstaclesCount_ToRemoveThisFrame = 0;
			m_obstaclesCount_UpdatedThisFrame = 0;
			m_obstaclesCount_RemainingDelayed = 0;
			m_obstaclesCount_ImmediateUpdated = 0;
			m_obstaclesCount_RemovedThisFrame = 0;
		}
	public:
		/*! Stores the duration of the last call to ObstacleManager::Update(). */
		KyFloat32 m_updateDuration;

		/*! Stores the number of \Obstacles that needed to be updated during the last frame, 
			calculated at the beginning of ObstacleManager::Update(). */
		KyUInt32 m_obstaclesCount_ToUpdateThisFrame;

		/*! Stores the number of \Obstacles updated during the last call to ObstacleManager::Update(). */
		KyUInt32 m_obstaclesCount_UpdatedThisFrame;

		/*! Stores the number of \Obstacles whose update was deferred during the last call to 
			ObstacleManager::Update() due to a lack of time remaining in the frame. */
		KyUInt32 m_obstaclesCount_RemainingDelayed;

		/*! Stores the number of \Obstacles updated during the last call to ObstacleManager::Update()
			that were flagged for immediate update. */
		KyUInt32 m_obstaclesCount_ImmediateUpdated;

		/*! Stores the number of \Obstacles that needed to be removed during the last frame, 
			calculated at the beginning of ObstacleManager::Update(). */
		KyUInt32 m_obstaclesCount_ToRemoveThisFrame;

		/*! Stores the number of \Obstacles removed during the last call to ObstacleManager::Update(). */
		KyUInt32 m_obstaclesCount_RemovedThisFrame;
	};


public:
	/*! \label_init
		\param navMeshLayerManager		The NavMeshLayerManager for the World.
		\param blockingLayer			The ObstacleLayer in which the ObstacleManager will project \Obstacles that should block
										movement but that should not be aggregated into the Local PathFinding system.
		\param lpfLayer					The ObstacleLayer in which the ObstacleManager will project \Obstacles that should be
										aggregated into the Local PathFinding system.
		\param outlinePolicyToCopy		An instance of a class that derives from IOutlineUpdatePolicy, which will determine which
										ObstacleLayer each Obstacle should be projected in. The object provided is copied, and the
										ObstacleManager assumes responsibility for the lifespan of the copy. You can safely destroy 
										the object you provide at any time.
		\return A #KyResult that indicates the success or failure of the initialization. */
	KyResult Initialize(Kaim::NavMeshLayerManager& navMeshLayerManager, ObstacleLayer& blockingLayer, ObstacleLayer& lpfLayer, const IOutlineUpdatePolicy& outlinePolicyToCopy);

	/************************************************************************/
	/* Utility                                                              */
	/************************************************************************/

	/*! Returns the instance of IOutlineUpdatePolicy currently in use. */
	IOutlineUpdatePolicy* GetOutlineUpdatePolicy() { return m_outlineUpdatePolicy; }

	/************************************************************************/
	/* Inherited methods                                                    */
	/************************************************************************/

	virtual void ReInit();

	virtual void Update();


	/************************************************************************/
	/* Setup                                                                */
	/************************************************************************/

	/*! Sets the \ObstacleLayers in which the ObstacleManager will project its Obstacle outlines.
		\param blockingLayerName		The unique name of the ObstacleLayer in which the ObstacleManager will
									project \Obstacles that should block movement but that should not be aggregated
									into the Local PathFinding system.
		\param lpfLayerName			The unique name of the ObstacleLayer in which the ObstacleManager will project 
									\Obstacles that should be aggregated into the Local PathFinding system.
		\return #KY_TRUE if both layers are successfully found. */
	KyResult SetupLayers(const char* blockingLayerName, const char* lpfLayerName);

	/*! Sets the \ObstacleLayers in which the ObstacleManager will project its Obstacle outlines.
		\param blockingLayer		The ObstacleLayer in which the ObstacleManager will
									project \Obstacles that should block movement but that should not be aggregated
									into the Local PathFinding system.
		\param lpfLayer				The ObstacleLayer in which the ObstacleManager will project 
									\Obstacles that should be aggregated into the Local PathFinding system.
		\return #KY_TRUE if both layers are successfully found. */
	void SetupLayers(ObstacleLayer& blockingLayer, ObstacleLayer& lpfLayer);

	/*! Sets the instance of IOutlineUpdatePolicy that will determine which
		ObstacleLayer each Obstacle should be projected in. The object provided is copied, and the
		ObstacleManager assumes responsibility for the lifespan of the copy. You can safely destroy 
		the object you provide at any time.
		\pp Use GetOutlineUpdatePolicy() if you need to retrieve the internal copy made by the ObstacleManager. */
	void SetOutlineUpdatePolicy(const IOutlineUpdatePolicy& outlineUpdatePolicy);

	/*! Sets the maximum number of \Obstacles that can be projected in a single frame.
		\pp Each frame, all \Obstacles flagged for immediate projection are projected. Then, if the number of
		\Obstacles projected so far is less than the maximum value set by this method, the ObstacleManager
		continues to project \Obstacles that permit deferred projection until this maximum is reached. Any
		remaining \Obstacles are deferred to the next frame.
		\defaultvalue	\c 256 */
	KyResult SetMaxUpdatePerFrame(KyUInt32 maxUpdatePerFrame=256);


	/************************************************************************/
	/* Obstacle registration management                                     */
	/************************************************************************/

	/*! Registers the specified Obstacle with this ObstacleManager. */
	KyResult RegisterObstacle(Obstacle& obstacle);

	/*! Unregisters the specified Obstacle from this ObstacleManager. */
	KyResult UnregisterObstacle(Obstacle& obstacle);


	/************************************************************************/
	/* Obstacle update                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	// - activation update


	/* Kynapse internal use only. Activates an obstacle. Ask for immediate update of the obstacle.
		Called by Obstacle::Activate method. */
	/*! For internal use. */
	KyResult ActivateObstacle(Obstacle& obstacle);

	/* Kynapse internal use only. Activates an obstacle. Ask for immediate update of the obstacle.
		Called by Obstacle::Deactivate method. */
	/*! For internal use. */
	KyResult DeactivateObstacle(Obstacle& obstacle);


	//////////////////////////////////////////////////////////////////////////
	// - outline update

	/* Kynapse internal use only. Computes the exact outline of the provided obstacle accordingly to the provided
		OBB shape description. It calls the ComputeExactOutline_OBB method of the current OutlineUpdatePolicy.
		Called by Obstacle::Update methods. */
	/*! For internal use. */
	KyResult ComputeExactOutline_OBB(Obstacle& obstacle, const ShapeOBB& shapeObb);

	/* Kynapse internal use only. Computes the exact outline of the provided obstacle accordingly to the provided
		polyline shape description. It calls the ComputeExactOutline_Polyline method of the current OutlineUpdatePolicy.
		Called by Obstacle::Update methods. */
	/*! For internal use. */
	KyResult ComputeExactOutline_Polyline(Obstacle& obstacle, const ShapePolyline& shapePolyline);


	//////////////////////////////////////////////////////////////////////////
	// - registration update

	/* Kynapse internal use only. Insert the provided obstacle to the right update list accordingly to the ObstacleUpdateDelay value.
		Called by Obstacle::Update methods. */
	/*! For internal use. */
	KyResult InsertObstacleToUpdate(Obstacle& obstacle, ObstacleUpdateDelay updateDelay);


	//////////////////////////////////////////////////////////////////////////
	// - obstacle update

	/* Kynapse internal use only. Removes an obstacle from all obstacle management internal lists. */
	/*! For internal use. */
	KyResult RemoveObstacle(Obstacle& obstacle);

	/* Kynapse internal use only. Updates obstacle internal representations (ProjectedOutline and projection
		layer). It calls OutlineUpdatePolicy::UpdateProjectedOutline and move obstacle
		ProjectedOutline to the right layer accordingly to the result. */
	/*! For internal use. */
	KyResult UpdateObstacle(Obstacle& obstacle);

	/************************************************************************/
	/* Statistics                                                           */
	/************************************************************************/
	/*! Retrieves an instance of the StatisticData class that provides information about the number of \Obstacles managed
		by the ObstacleManager, their update status, and the time taken in the last update. */
	const StatisticData& GetStatisticData() const { return m_statisticData; }


	/*! For internal use. */
	SharedPoolList<Obstacle*>::Iterator GetFirstObstacleToConsiderInDA() { return m_obstacleToConsiderInDA.GetFirst(); }

	/*! For internal use. */
	const SharedPoolList<Obstacle*>& GetObstacles() const { return m_obstacles; }
	/*! For internal use. */
	const SharedPoolList<Obstacle*>& GetObstaclesToUpdate_Immediate() const { return m_obstacleToUpdate_immediate; }
	/*! For internal use. */
	const SharedPoolList<Obstacle*>& GetObstaclesToUpdate_Delayed() const { return m_obstacleToUpdate_delayed; }
	/*! For internal use. */
	const SharedPoolList<Obstacle*>& GetObstaclesToUpdate_NextTime() const { return m_obstacleToUpdateNextTime; }

protected:
	ObstacleManager(World& world);

	virtual ~ObstacleManager();

	//////////////////////////////////////////////////////////////////////////
	// - outline projection layer management

	/*
		Updates the projection layer of the obstacle.
		If the obstacle is not projected on the provided layer, it is:
		1. removed from the one its ProjectedOutline is referenced to and
		2. added to the layer it have to be projected on.
	*/
	KyResult UpdateObstacleProjectionLayer(Obstacle& obstacle, ObstacleLayer* layer);
	/* Removes an obstacle from a layer. */
	KyResult RemoveObstacleFromLayer(Obstacle& obstacle);
	/* Adds an obstacle to a layer. */
	KyResult AddObstacleToLayer(Obstacle& obstacle, ObstacleLayer* layer);

	// Obstacle list management
	SharedPoolList<Obstacle*>::Iterator FindObstacleInList(SharedPoolList<Obstacle*>& list, Obstacle& obstacle);
	KyResult AddObstacleToList(SharedPoolList<Obstacle*>& list, Obstacle& obstacle);
	KyResult RemoveObstacleFromList(SharedPoolList<Obstacle*>& list, Obstacle& obstacle);

protected:
	// NavMesh layers
	NavMeshLayerManager* m_layerManager;
	ObstacleLayer* m_lpfLayer;
	ObstacleLayer* m_blockingLayer;

	// Obstacle pool
	SharedPoolList<Obstacle*>::Pool m_obstablePool;
	SharedPoolList<Obstacle*> m_obstacles;

	// Obstacle management
	SharedPoolList<Obstacle*> m_obstacleToRemove; // all these ones will be removed during next Update
	SharedPoolList<Obstacle*> m_obstacleToUpdate_immediate; // all these ones will be updated during next Update
	SharedPoolList<Obstacle*> m_obstacleToUpdate_delayed; // these ones will be updated when there will be enough time left (they can wait several Update before beeing actually updated)
	SharedPoolList<Obstacle*> m_obstacleToUpdateNextTime; // these ones have to be checked until they are at rest
	SharedPoolList<Obstacle*> m_obstacleToConsiderInDA; // these ones have to be checked in DA

	// OutlineUpdatePolicy
	IOutlineUpdatePolicy* m_outlineUpdatePolicy;

	// TimeSlicing
	KyUInt32 m_maxUpdatePerFrame;

	// Statistics
	StatisticData m_statisticData;
};

} // Kaim

#endif // KY_OBSTACLEMANAGER_H
