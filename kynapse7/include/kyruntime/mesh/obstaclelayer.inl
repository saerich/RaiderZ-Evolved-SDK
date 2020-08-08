/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY


//////////////////////////////////////////////////////////////////////////
// ObstacleLayerObstacleRecord
//////////////////////////////////////////////////////////////////////////
ObstacleLayerObstacleRecord::ObstacleLayerObstacleRecord() :
	m_body(KY_NULL),
	m_points(KY_NULL),
	m_pointCount(0),
	m_thickness(KY_NULL),
	m_navFloorPtrs(KY_NULL),
	m_navFloorCount(0),
	m_navFloorMax(0),
	m_hasChanged(KY_NULL),
	m_userData(KY_NULL)
{
}

ObstacleLayerObstacleRecord::ObstacleLayerObstacleRecord(Body *body, Vec3f* points, KyUInt32* pointCount, KyFloat32* thickness,
	NavFloorPtr* navFloors, KyUInt32 navFloorMax, SharedPoolList<ObstacleProjectionVolume*>::Pool* projectionVolumePool,
	KyBool* hasChanged, void* userData) :
	m_body(body),
	m_points(points),
	m_pointCount(pointCount),
	m_thickness(thickness),
	m_navFloorPtrs(navFloors),
	m_navFloorCount(0),
	m_navFloorMax(navFloorMax),
	m_hasChanged(hasChanged),
	m_userData(userData)
{
	m_projectionVolumes.SetPool(projectionVolumePool);
}

ObstacleLayerObstacleRecord::ObstacleLayerObstacleRecord(const ObstacleLayerObstacleRecord& ob):
	m_body(KY_NULL),
	m_points(KY_NULL),
	m_pointCount(0),
	m_thickness(KY_NULL),
	m_navFloorPtrs(KY_NULL),
	m_navFloorCount(0),
	m_navFloorMax(0),
	m_hasChanged(KY_NULL),
	m_userData(KY_NULL)
{
	*this = ob;
}

ObstacleLayerObstacleRecord& ObstacleLayerObstacleRecord::operator=(const ObstacleLayerObstacleRecord& ob)
{
	m_body = ob.m_body;
	m_points = ob.m_points;
	m_pointCount = ob.m_pointCount;
	m_thickness = ob.m_thickness;
	m_navFloorPtrs = ob.m_navFloorPtrs;
	m_navFloorCount = ob.m_navFloorCount;
	m_navFloorMax = ob.m_navFloorMax;
	m_hasChanged = ob.m_hasChanged;
	m_userData = ob.m_userData;
	m_aabb2d = ob.m_aabb2d;

	m_projectionVolumes.Clear();
	m_projectionVolumes.SetPool(ob.m_projectionVolumes.GetSharedPool());
	for (SharedPoolList<ObstacleProjectionVolume*>::ConstIterator opvIt=ob.m_projectionVolumes.GetFirst(); opvIt.IsValid(); ++opvIt)
		m_projectionVolumes.Insert(*opvIt);

	return *this;
}

KyResult ObstacleLayerObstacleRecord::AddProjectionVolume(ObstacleProjectionVolume* projectionVolume)
{
	if (m_projectionVolumes.Insert(projectionVolume).IsValid())
		return KY_SUCCESS;
	return KY_ERROR;
}

KyResult ObstacleLayerObstacleRecord::RemoveProjectionVolume(ObstacleProjectionVolume* projectionVolume)
{
	return m_projectionVolumes.RemoveFirstOccurrence(projectionVolume);
}

void ObstacleLayerObstacleRecord::ClearProjectionVolumes()
{
	for (SharedPoolList<ObstacleProjectionVolume*>::ConstIterator opvIt=m_projectionVolumes.GetFirst(); opvIt.IsValid(); ++opvIt)
		(*opvIt)->RemoveObstacleProjection(this);
	m_projectionVolumes.Clear();
}


//////////////////////////////////////////////////////////////////////////
// LayerNavMeshRecord
//////////////////////////////////////////////////////////////////////////
ObstacleLayer::LayerNavMeshRecord::LayerNavMeshRecord():
	m_navMesh(KY_NULL),
	m_queryServer(KY_NULL),
	m_beginId(KyUInt32MAXVAL),
	m_endId(KyUInt32MAXVAL),
	m_curObstItWhenAdded()
{
}

ObstacleLayer::LayerNavMeshRecord& ObstacleLayer::LayerNavMeshRecord::operator=(const ObstacleLayer::LayerNavMeshRecord& ob)
{
	m_navMesh = ob.m_navMesh;
	m_queryServer = ob.m_queryServer;
	m_beginId = ob.m_beginId;
	m_endId = ob.m_endId;
	m_curObstItWhenAdded = ob.m_curObstItWhenAdded;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// LayerProjectionRecord
//////////////////////////////////////////////////////////////////////////
ObstacleLayer::LayerProjectionRecord& ObstacleLayer::LayerProjectionRecord::operator=(const ObstacleLayer::LayerProjectionRecord& ob)
{
	m_navFloorPtr = ob.m_navFloorPtr;
	m_obsIt = ob.m_obsIt;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// LayerObstacleProjectionVolumeRecord
//////////////////////////////////////////////////////////////////////////
ObstacleLayer::LayerObstacleProjectionVolumeRecord::LayerObstacleProjectionVolumeRecord():
m_projectionVolume(KY_NULL),
m_curObstItWhenAdded()
{
}

ObstacleLayer::LayerObstacleProjectionVolumeRecord& ObstacleLayer::LayerObstacleProjectionVolumeRecord::operator=(const ObstacleLayer::LayerObstacleProjectionVolumeRecord& other)
{
	m_projectionVolume   = other.m_projectionVolume;
	m_curObstItWhenAdded = other.m_curObstItWhenAdded;
	return *this;
}

