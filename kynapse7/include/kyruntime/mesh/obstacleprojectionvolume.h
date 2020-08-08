/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_VOLUMEOBSTACLEPROJECTION_H
#define KY_VOLUMEOBSTACLEPROJECTION_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/containers/list.h>


namespace Kaim
{

class IVolume;
class ObstacleLayerObstacleRecord;


/*	Used internally by the ObstacleLayer class to associate volumes with obstacle projections. You should
	need to interact with this class directly. */
class ObstacleProjectionVolume
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	ObstacleProjectionVolume(): m_volume(KY_NULL) {}
	ObstacleProjectionVolume(const ObstacleProjectionVolume& other) { *this = other; }


	//////////////////////////////////////////////////////////////////////////
	// Initialization
	//////////////////////////////////////////////////////////////////////////

	KY_FORCE_INLINE KyResult SetVolume(const IVolume* volume)
	{
		m_volume = volume;
		return KY_SUCCESS;
	}

	KyResult SetObstaclePointerPool(SharedPoolList<ObstacleLayerObstacleRecord*>::Pool* obstacleRecordPointerPool);

	/* Copy operator. */
	ObstacleProjectionVolume& operator=(const ObstacleProjectionVolume& other)
	{
		m_volume = other.m_volume;

		m_obstacles.Clear();
		for (SharedPoolList<ObstacleLayerObstacleRecord*>::ConstIterator orIt=other.m_obstacles.GetFirst(); orIt.IsValid(); ++orIt)
			m_obstacles.Insert(*orIt);

		return *this;
	}


	//////////////////////////////////////////////////////////////////////////
	// Obstacle management
	//////////////////////////////////////////////////////////////////////////

	KyResult UpdateObstacleProjection(ObstacleLayerObstacleRecord* obstacleRecord);
	KyResult RemoveObstacleProjection(ObstacleLayerObstacleRecord* obstacleRecord);
	KyResult ClearObstacleProjections();

	KY_FORCE_INLINE KyUInt32 GetObstacleCount() const { return m_obstacles.GetCount(); }
	KY_FORCE_INLINE SharedPoolList<ObstacleLayerObstacleRecord*>::ConstIterator GetFirstObstacle() const { return m_obstacles.GetFirst(); }

protected:
	KyResult UpdateObstacleProjection(ObstacleLayerObstacleRecord* obstacleRecord, IVolume* obstacleVolume);


protected:
	const IVolume* m_volume;
	SharedPoolList<ObstacleLayerObstacleRecord*> m_obstacles;
};

} // namespace Kaim

#endif // KY_VOLUMEOBSTACLEPROJECTION_H
