/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_RUNTIMEPATHOBJECTCONNEXIONSPACE_H
#define KY_RUNTIMEPATHOBJECTCONNEXIONSPACE_H

#include <kypathdata/containers/list.h>
#include <kyruntime/mesh/obstacleprojectionvolume.h>
#include <kyruntime/graph/stitchedgraphcell.h>
#include <kyruntime/graph/graphvertexptr.h>
#include <kyruntime/graph/graphvertexsafeptr.h>

namespace Kaim
{

class NavMeshLayerManager;
class ObstacleLayerObstacleRecord;
class ObstacleProjectionVolume;
class RuntimePathObjectTopology;
class IPathObjectConnexionSpace;


/*! A runtime representation of a PathObject connection space, created transparently when a PathObject
	is added to or removed from a World. The methods specific to this class are mostly intended for internal
	use, except for IsConnectedToThisVertex() and IntersectObstacles().
	\ingroup kyruntime_pathobject */
class RuntimePathObjectConnexionSpace
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	RuntimePathObjectConnexionSpace(const RuntimePathObjectTopology* topology, IPathObjectConnexionSpace* userConnexionSpace);
	~RuntimePathObjectConnexionSpace();

	/*! Retrieves the connection position. This position is at the altitude of the Graph, not the NavMesh.
		It corresponds to IPathObjectConnexionSpace::GetConnexionPosition(), offset to the altitude of the Graph.*/
	const Vec3f& GetConnexionPosition() const;

	/*! Returns the IVolume stored in the IPathObjectConnexionSpace used to construct this object. */
	const IVolume* GetVolume() const;

	/*! Returns a pointer to the RuntimePathObjectTopology using this object. */
	const RuntimePathObjectTopology* GetTopology() const { return m_topology; }

	/*! Read accessor for #m_connexionSpace. */
	IPathObjectConnexionSpace* GetUserConnexionSpace() { return m_connexionSpace; }

	/*! Returns the number of \Obstacles that currently intersect the volume of this connection space. */
	KY_FORCE_INLINE KyUInt32 GetObstacleCount() const { return (m_obstacleProjectionVolume ? m_obstacleProjectionVolume->GetObstacleCount(): 0); }

	/*! Returns \c true if this connection space intersects at least one Obstacle. */
	KY_FORCE_INLINE bool IntersectObstacles() const { return (GetObstacleCount()!= 0); }

	/*! Renders debugging information according to the flags in the \c renderFlags bitmask.
		-	The volume that defines the connection space is always rendered. It is drawn in magenta if the control
			space is obstructed by an Obstacle or connected to the Graph, or orange otherwise.
		-	If \c renderFlags includes the #PathObjectRender_Obstacles flag, and the connection space is
			obstructed by an Obstacle, a dark magenta line is drawn from the control space to the corner of that Obstacle.
		-	If \c renderFlags includes the #PathObjectRender_Data flag, the
			IPathObjectConnexionSpace::Render() method of the IPathObjectConnexionSpace is called, and is passed
			the same \c renderFlags value. */
	void Render(KyUInt32 renderFlags) const;


	//////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////

	void SetGraphVertexPtr(const GraphVertexPtr& graphVertexPtr) { m_graphVertexPtr = graphVertexPtr; }

	/*! Tests whether or not this connection space is connected to the specified Graph vertex. */
	bool IsConnectedToThisVertex(const GraphVertexSafePtr& vertexSafePtr) const;

	GraphVertexSafePtr GetVertexSafePtr() const { return m_graphVertexPtr.ConvertToGraphVertexSafePtr(); }
	void ClearGraphVertexPtr() { m_graphVertexPtr.Invalidate(); }


	//////////////////////////////////////////////////////////////////////////
	// Obstacle management
	//////////////////////////////////////////////////////////////////////////
	KyResult AttachToLayerManager(NavMeshLayerManager* layerManager);
	KyResult DetachFromLayerManager(NavMeshLayerManager* layerManager);
	KyResult RegisterProjectionVolumeToLayerManager(NavMeshLayerManager* layerManager);
	KyResult UnregisterProjectionVolumeFromLayerManager(NavMeshLayerManager* layerManager);


protected:
	friend class PathObject;
	const RuntimePathObjectTopology* m_topology;            //< The parent runtime topology.
	ObstacleProjectionVolume* m_obstacleProjectionVolume;   //< Get access to all obstacles in the connexionSpace.
	KyBool m_isConnected;                                   //< True if this connexionspace could be connected.
	GraphVertexPtr m_graphVertexPtr;                        //< Ptr of the vertex associated with this connexion space in m_stitchedGraphCell.
	Vec3f m_connexionPosition;                              //< The position at which the PathObject graph is connected to the other graphs (usually at half entity height above the ground).

public:
	IPathObjectConnexionSpace* m_connexionSpace; /*!< Stores a pointer to the IPathObjectConnexionSpace used to construct this object. */
};

} // namespace Kaim

#endif // #ifndef KY_RUNTIMEPATHOBJECTCONNEXIONSPACE_H
