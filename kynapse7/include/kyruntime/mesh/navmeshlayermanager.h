/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_NAVMESHLAYERMANAGER_H
#define KY_NAVMESHLAYERMANAGER_H

#include <kyruntime/mesh/inavmeshlayer.h>

#include <kyruntime/core/service.h>

#include <kypathdata/containers/list.h>


namespace Kaim
{

class INavMeshAccessor;
class ObstacleProjectionVolume;
class ObstacleLayerObstacleRecord;
class PathObjectLayer;


/*! The NavMeshLayerManager is a class of WorldService that is responsible for creating and mantaining objects
	whose classes derive from INavMeshLayer, each of which stores a set of dynamic obstacles and forbidden areas
	that affect the movement possibilities allowed by the static NavMesh.
	\pp This class is a key part of the Obstacle management system. See :USERGUIDE:"Using Obstacles".
	\ingroup kyruntime_mesh */
class NavMeshLayerManager: public WorldService
{
public:
	KY_DECLARE_WORLDSERVICE(NavMeshLayerManager)

public:
	/*! Creates a new instance of an INavMeshLayer object with the specified MetaClass. The NavMeshLayerManager is responsible for 
		managing the lifespan of the newly created object.
		\param layerName			A unique name for the new INavMeshLayer.
		\param layerClass			The MetaClass of the INavMeshLayer to create. Typically Kaim::ObstacleLayer::Class, unless you have
									created your own sub-class of INavMeshLayer.
		\return A pointer to the newly created INavMeshLayer object. */
	INavMeshLayer* CreateAndGetLayer(const char* layerName, NavMeshLayerClass& layerClass);

	/*! Creates a new instance of an INavMeshLayer object with the specified class. The NavMeshLayerManager is responsible for 
		managing the lifespan of the newly created object.
		tparam T					The class name of the INavMeshLayer to create. Typically Kaim::ObstacleLayer, unless you have
									created your own sub-class of INavMeshLayer.
		\param layerName			A unique name for the new INavMeshLayer.
		\return A pointer to the newly created INavMeshLayer object. */
	template<class T>
	T* CreateAndGetLayer(const char* layerName)
	{ return static_cast<T*>(CreateAndGetLayer(layerName, T::Class)); }


	/*! \copydoc WorldService::Update()
		\pp This implementation of WorldService::Update() calls the INavMeshLayer::Update() method of each INavMeshLayer
		managed by the NavMeshLayerManager. */
	virtual void Update();

	/*! Retrieves the total elapsed time spent in the last call to Update(). */
	KyFloat32 GetStats_UpdateDuration() const { return m_stats_UpdateDuration;}

	/*! Retrieves the PathObjectLayer maintained by this NavMeshLayerManager, if any. */
	PathObjectLayer* GetPathObjectLayer() { return  m_poLayer; }

	/*! Retrieves a list of all INavMeshLayer objects created and maintained by the NavMeshLayerManager. */
	const INavMeshLayerPtrList& GetLayers() const { return  m_layers; }
	INavMeshLayerPtrList& GetLayers() { return  m_layers; }

	/*! Retrieves the INavMeshLayer object whose unique name (as specified in the call to CreateAndGetLayer()) matches the \c name
		parameter, or #KY_NULL if no matching INavMeshLayer object can be found. */
	INavMeshLayer* GetLayer(const char* name)
	{
		for (INavMeshLayerPtrList::Iterator it = m_layers.GetFirst(); it.IsValid(); ++it)
		{
			if (KY_STRICMP(name,(*it)->GetName()) == 0)
				return *it;
		}
		return KY_NULL;
	}

	/************************************************************************/
	/* ObstacleProjectionVolumes                                            */
	/************************************************************************/

	/*! For internal use. */
	ObstacleProjectionVolume* GetNextFreeObstacleProjectionVolume();

	/*! For internal use. */
	KyResult ReleaseObstacleProjectionVolume(ObstacleProjectionVolume* obstacleProjectionVolume);


	/*! For internal use. */
	KyResult RegisterObstacleProjectionVolume(ObstacleProjectionVolume* obstacleProjectionVolume);

	/*! For internal use. */
	KyResult UnregisterObstacleProjectionVolume(ObstacleProjectionVolume* obstacleProjectionVolume);

protected:
	explicit NavMeshLayerManager(Kaim::World& world);
	virtual ~NavMeshLayerManager();

	PathObjectLayer* m_poLayer;

	INavMeshLayerPtrList m_layers;
	KyFloat32 m_stats_UpdateDuration;
	SharedPoolList<ObstacleLayerObstacleRecord*>::Pool m_obstacleRecordPointerPool;
	List<ObstacleProjectionVolume> m_projectionVolumes;
};


} // namespace Kaim

#endif // KY_NAVMESHLAYERMANAGER_H
