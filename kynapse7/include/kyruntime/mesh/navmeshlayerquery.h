/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_NAVMESHLAYERQUERY_H
#define KY_NAVMESHLAYERQUERY_H

#include <kypathdata/navmesh/navmeshutils.h>
#include <kyruntime/mesh/inavmeshlayer.h>

namespace Kaim
{

class NavMeshLayerLocation;


/*! This class is a wrapper around a list of pointers to INavMeshLayer objects. It is used in collision
	tests against a NavMeshQueryServerWrapper to specify one or more \INavMeshLayers that should be taken into
	account during the collision test in addition to the static NavMesh.
	\pp You can also use the NavMeshLayerQuery class on its own to conduct collision tests against the array of
	\INavMeshLayers contained in its #m_layers member without taking any static NavMesh into consideration.
	\ingroup kyruntime_mesh */
class NavMeshLayerQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! Default constructor. Creates a NavMeshLayerQuery with an empty #m_layers list. */
	NavMeshLayerQuery() : m_layers(KY_NULL) {}

	/*! Constructs a new instance of the NavMeshLayerQuery class, and sets its #m_layers list to match the specified \c layers. */
	NavMeshLayerQuery(const INavMeshLayerPtrList* layers) : m_layers(layers) {}

	/*! Calls INavMeshLayer::IsValidPosition() for each INavMeshLayer in the #m_layers list, and stores in the \c locations list
		all collisions with the specified point in all \INavMeshLayers. For details, see INavMeshLayer::IsValidPosition(). */
	KyUInt32 GetLocationsFromPos(const NavFloorPtr& navFloorPtr, const Vec3f& pos, NavMeshLayerLocationList& locations) const;

	/*! Calls INavMeshLayer::GetAllLocations() for each INavMeshLayer in the #m_layers list, and stores in the \c locations list
		all collisions with the specified line segment in all \INavMeshLayers. For details, see INavMeshLayer::GetAllLocations(). */
	KyBool GetAllLocations(const NavFloorPtr& navFloorPtr, const Vec3f& p1, const Vec3f& p2, KyFloat32 radius, NavMeshLayerLocationList& locations) const;

	/*! Calls INavMeshLayer::CanGo() for each INavMeshLayer in the #m_layers list, and returns #KY_TRUE only if all calls to
		INavMeshLayer::CanGo() return #KY_TRUE. For details, see INavMeshLayer::CanGo(). */
	KyBool CanGo(const NavFloorPtr& navFloorPtr, const Vec3f& p1, const Vec3f& p2, KyFloat32 radius) const;

	/*! Calls INavMeshLayer::TraceLine() for each INavMeshLayer in the #m_layers list, and sets \c location to indicate the collision
		nearest to \c p1. For details, see INavMeshLayer::TraceLine(). */
	KyBool TraceLine(const NavFloorPtr& navFloorPtr, const Vec3f& p1, const Vec3f& p2, KyFloat32 radius, Vec3f& lvp, NavMeshLayerLocation* location) const;

public:
	const INavMeshLayerPtrList* m_layers; /*!< Stores the list of \INavMeshLayers that this NavMeshLayerQuery takes into account in its collision tests. */
};

} // namespace Kaim

#endif // KY_NAVMESHLAYERQUERY_H
