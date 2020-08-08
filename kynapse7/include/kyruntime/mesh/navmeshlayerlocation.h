/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_NAVMESHLAYERLOCATION_H
#define KY_NAVMESHLAYERLOCATION_H

#include <kypathdata/generic/memory.h>


namespace Kaim
{

class INavMeshLayer;


/*!	This class is a wrapper around an INavMeshLayer pointer and a void pointer. It is typically used in collision
	tests against dynamic \INavMeshLayers, to indicate which INavMeshLayer and which obstacle within that
	INavMeshLayer was hit by the collision test.
	\pp For example, if you call NavMeshQueryServerWrapper::TraceLine() to perform a collision test, and you provide
	a \c layerQuery parameter for that call to indicate one or more dynamic \INavMeshLayers that should also be considered
	during the test, you can also provide an instance of NavMeshLayerLocation to store information about any
	collision detected with an obstacle in one of those \INavMeshLayers. If a collision is detected by any INavMeshLayer,
	that INavMeshLayer updates the NavMeshLayerLocation with a pointer to itself, and a void pointer that may be used to
	identify the obstacle in the INavMeshLayer that caused the collision. 
	\pp The interpretation of the void pointer depends on the implementation of the INavMeshLayer. For example, the
	ObstacleLayer class of INavMeshLayer sets the void pointer to match the a void \c userData pointer that was 
	provided in the call to ObstacleLayer::AddObstacle() when the obstacle was initially added to the ObstacleLayer, if
	any. Concretely, this means that:
	-	If the obstacle was added to the ObstacleLayer by the ObstacleManager from an Obstacle that you provided, the
		NavMeshLayerLocation::m_userData pointer is set to match Obstacle::m_userData.
	-	If you added the obstacle to the ObstacleLayer yourself by calling ObstacleLayer::AddObstacle() directly, the
		NavMeshLayerLocation::m_userData pointer is set to the value you specified in the call.
	-	For obstacles retrieved from a PathObjectLayer, the NavMeshLayerLocation::m_userData pointer is always set to
		point to the control space of the blocking PathObject.
	\ingroup kyruntime_mesh */
class NavMeshLayerLocation
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	Default constructor. */
	NavMeshLayerLocation() { Clear(); }

	/*!	Clears the pointers contained in this NavMeshLayerLocation, resetting them to #KY_NULL. */
	void Clear()
	{
		m_layer = KY_NULL;
		m_userData = KY_NULL;
	}

	/*!	Sets up this NavMeshLayerLocation with the specified information.
		\param layer			The INavMeshLayer pointer to store in the #m_layer member. 
		\param userData			The void pointer to store in the #m_userData member. */
	void Set(const INavMeshLayer* layer, void* userData)
	{
		m_layer = layer;
		m_userData = userData;
	}

public:
	/*!	Stores a pointer to an INavMeshLayer, which typically indicates which instance of INavMeshLayer was
		hit during a collision test. */
	const INavMeshLayer* m_layer; 
	/*!	Stores a void pointer that may indicate which obstacle in the INavMeshLayer was hit during a collision test. 
		See the NavMeshLayerLocation class description for details. */
	void* m_userData;
};

template <typename T> class List;

typedef List<NavMeshLayerLocation> NavMeshLayerLocationList;


} // namespace Kaim

#endif // KY_NAVMESHLAYERLOCATION_H
