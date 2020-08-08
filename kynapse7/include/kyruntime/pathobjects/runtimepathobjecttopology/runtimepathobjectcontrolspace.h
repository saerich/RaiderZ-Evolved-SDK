/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_RUNTIMEPATHOBJECTCONTROLSPACE_H
#define KY_RUNTIMEPATHOBJECTCONTROLSPACE_H

#include <kypathdata/containers/list.h>
#include <kyruntime/mesh/obstacleprojectionvolume.h>


namespace Kaim
{

class NavMeshLayerManager;
class RuntimePathObjectTopology;
class EdgeLockerVolume;
class PathObjectOutline;
class IPathObjectControlSpace;


/*! A runtime representation of a PathObject control space, created transparently when a PathObject
	is added to or removed from a World. The methods specific to this class are mostly intended for internal
	use.
	\ingroup kyruntime_pathobject */
class RuntimePathObjectControlSpace
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	RuntimePathObjectControlSpace(const RuntimePathObjectTopology* topology, IPathObjectControlSpace* userControlSpace);
	~RuntimePathObjectControlSpace();

	const IVolume* GetVolume() const;
	bool CanLockEdges() const;
	bool IsAnObstacle() const;
	bool IsASpaceToStartUsingThePathObject() const;

	/*! Returns a pointer to the RuntimePathObjectTopology using this object. */
	const RuntimePathObjectTopology* GetTopology() const { return m_topology; }

	/*! Read accessor for #m_controlSpace. */
	IPathObjectControlSpace* GetUserControlSpace() { return m_controlSpace; }

	/*! Renders debugging information according to the flags in the \c renderFlags bitmask.
		-	The volume that defines the control space is always rendered. It is drawn in red if the control
			space is obstructed by an Obstacle, or green otherwise.
		-	If \c renderFlags includes the #PathObjectRender_Obstacles flag, and the control space is
			obstructed by an Obstacle, a red line is drawn from the control space to the corner of that Obstacle.
		-	If \c renderFlags includes the #PathObjectRender_Data flag, the
			IPathObjectControlSpace::Render() method of the IPathObjectControlSpace is called, and is passed
			the same \c renderFlags value. */
	void Render(KyUInt32 renderFlags) const;


	//////////////////////////////////////////////////////////////////////////
	// Obstacle management
	//////////////////////////////////////////////////////////////////////////

	KyResult AttachToLayerManager(NavMeshLayerManager* layerManager);
	KyResult DetachFromLayerManager(NavMeshLayerManager* layerManager);
	KyResult RegisterProjectionVolumeToLayerManager(NavMeshLayerManager* layerManager);
	KyResult UnregisterProjectionVolumeFromLayerManager(NavMeshLayerManager* layerManager);

	KyResult UpdateOutline();

	KY_FORCE_INLINE KyUInt32 GetObstacleCount() const { return (m_obstacleProjectionVolume ? m_obstacleProjectionVolume->GetObstacleCount(): 0); }

	/* Return true if the ConnexionSpace intersects at least one obstacle.*/
	KY_FORCE_INLINE bool IntersectObstacles() const { return (GetObstacleCount()!= 0); }

private:
	void Init();

protected:
	const RuntimePathObjectTopology* m_topology;
	ObstacleProjectionVolume* m_obstacleProjectionVolume;
	PathObjectOutline* m_outline;

public:
	/*	Volume that contains the graph edges locked by this control space.
		To fully control an area, some PathObjects want to 
		lock edges intersecting their ControlVolume (eg: door).
		This EdgeLockerVolume comes from a pool stored in the associated PathObject.
		This is an iterator on the parent list in order to optimize its removal.*/
	List<EdgeLockerVolume>::Iterator m_edgeLockerVolume;

public:
	IPathObjectControlSpace* m_controlSpace; /*!< Stores a pointer to the IPathObjectControlSpace used to construct this object. */

};

} // namespace Kaim

#endif // #ifndef KY_RUNTIMEPATHOBJECTCONTROLSPACE_H
