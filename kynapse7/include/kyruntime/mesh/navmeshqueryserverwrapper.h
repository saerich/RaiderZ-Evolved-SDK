/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: JUBA
#ifndef KY_NAVMESHQUERYSERVERWRAPPER_H
#define KY_NAVMESHQUERYSERVERWRAPPER_H

#include <kypathdata/math/vec3f.h>
#include <kypathdata/navmesh/navmeshqueryserver.h>
#include <kypathdata/navmesh/navmeshtypes.h>
#include <kypathdata/navmesh/navmeshutils.h>

#include <kyruntime/mesh/navmeshlayerlocation.h>

namespace Kaim
{

class NavTriangleSafePtr;
class NavMeshLayerQuery;

/*!	This implementation of NavMeshQueryServer conducts collision tests against both static \NavMeshes and against
	dynamic \INavMeshLayers.
	\pp Each Database served by the DatabaseManager transparently creates an instance of this class at
	initialization time, and keeps it up-to-date with the \NavMeshes streamed into and out of that Database. You 
	can retrieve this instance of NavMeshQueryServerWrapper by calling Database::GetNavMeshQueryServer().
	\pp Once you have retrieved this instance, you can conduct collision tests against the static \NavMeshes streamed into the
	corresponding Database by calling any of the methods of the base NavMeshQueryServer class.
	\pp To conduct collision tests against dynamic \INavMeshLayers in addition to the static \NavMeshes streamed into the Database, 
	call any of the methods specific to the NavMeshQueryServerWrapper class:
	-	CanGo()
	-	TraceLine()
	-	CanGo_Polyline()
	-	TraceLine_Polyline() 
	\pp These methods all accept a NavMeshLayerQuery parameter that specifies the list of \INavMeshLayers to take into 
	account during the collision test. If this list is empty, only the static \NavMeshes will be taken into account.
	\pp The GetAllLayerLocations() method retrieves information only about obstacles in the dynamic \INavMeshLayers, but not about
	obstacles in the static \NavMeshes. To conduct other collision tests only against dynamic \NavMeshes, do not use the 
	NavMeshQueryServerWrapper. Instead, use the collision testing methods exposed in the NavMeshLayerQuery class directly.
	\ingroup kyruntime_mesh */
class NavMeshQueryServerWrapper : public NavMeshQueryServer
{
public:
	/*!	\label_constructor For internal use. */
	NavMeshQueryServerWrapper() : NavMeshQueryServer() {}

	/*!	\label_constructor For internal use. */
	NavMeshQueryServerWrapper(const NavMeshManager& navMeshManager, KyUInt32 resultBufferSize, KyUInt32 workingBufferSize, KyUInt32 propagationBufferSize) :
		NavMeshQueryServer(navMeshManager, resultBufferSize, workingBufferSize, propagationBufferSize) {}

public:
	/**************************************************************************
	 *   Run-time interface methods
	 *
	 *   These methods translate runtime queries into NavMesh
	 * queries and then calls NavMeshQueryServer 
	 *************************************************************************/

	// ------------------------------- Propagation functions -------------------------------
	/*!	Indicates whether or not a Bot with a specified radius can move
		in a straight line from the specified starting point to the specified destination
		point without being stopped by any objects in the static \NavMeshes
		considered by this NavMeshQueryServerWrapper or any objects in the
		dynamic \INavMeshLayers specified by the \c layerQuery parameter, and without crossing
		any forbidden terrain types.
		\param startPoint						The starting point of the collision test.
		\param destPoint						The ending point of the collision test.
		\param radius							The radius of the Bot moving from \c startPoint to \c destPoint.
		\param halfHeight						Half the height of the Bot moving from \c startPoint to \c destPoint.
		\param startTriangleSafePtr_in			A NavTriangleSafePtr that indicates the NavMesh triangle that corresponds to the starting point. 
												If you do not know this value, omit this parameter or use #KY_NULL. The value will be computed
												automatically.
		\param destTriangleSafePtr_in			A NavTriangleSafePtr that indicates the NavMesh triangle that corresponds to the destination point. 
												If you do not know this value, omit this parameter or use #KY_NULL. The value will be computed
												automatically.
		\param[out] startTriangleSafePtr_out	A NavTriangleSafePtr that will be used to store the NavMesh triangle that corresponds to the start point. 
												If you pass a valid NavTriangleSafePtr instance, it will be updated automatically. If you pass #KY_NULL or
												omit this parameter, no value will be computed.
		\param[out] destTriangleSafePtr_out		A NavTriangleSafePtr that will be used to store the NavMesh triangle that corresponds to the last valid point. 
												If you pass a valid NavTriangleSafePtr instance, it will be updated automatically. If you pass #KY_NULL or
												omit this parameter, no value will be computed.
		\param allowedTerrainTypes				A bitmask that identifies the terrain types that should be considered navigable during the query.
		\param layerQuery						A NavMeshLayerQuery that lists the dynamic \INavMeshLayers that should be taken into account
												during the collision test.
		\return -	#KY_TRUE if the Bot can move in a direct line from \c startPoint to \c destPoint without hitting any obstacles in 
					a NavMesh or an INavMeshLayer, and if \c destPoint lies on the same floor as \c startPoint.
				-	#KY_FALSE in all other cases.
		\pp Note that this method does not necessarily indicate whether or not a path exists
		between the \c startPoint and the \c destPoint. It is simply a collision test that determines whether or not the 
		straight line between the two points is blocked by any border of the static NavMesh, any Obstacle in a 
		specified INavMeshLayer, or any forbidden terrain types. */
	KyBool CanGo(
		const Vec3f& startPoint, const Vec3f& destPoint, KyFloat32 radius, KyFloat32 halfHeight,
		const NavTriangleSafePtr* startTriangleSafePtr_in = KY_NULL, const NavTriangleSafePtr* destTriangleSafePtr_in = KY_NULL,
		NavTriangleSafePtr* startTriangleSafePtr_out = KY_NULL, NavTriangleSafePtr* destTriangleSafePtr_out = KY_NULL,
		NavTerrainTypeMask allowedTerrainTypes = NavTerrainTypeMask_All,
		const NavMeshLayerQuery* layerQuery = KY_NULL);

	/*!	Determines whether or not a Bot with a specified radius can move directly from a specified starting point to a specified 
		ending point in 2D without being stopped by any objects in the static \NavMeshes considered by this NavMeshQueryServerWrapper or 
		any objects in the dynamic \INavMeshLayers specified by the \c layerQuery parameter. This method also provides access to the point at 
		which the first collision was detected (if any), and information about the INavMeshLayer obstacle that caused the collision (if any).
		\param startPoint						The starting point of the collision test.
		\param destPoint						The ending point of the collision test. The altitude of this point is disregarded during the collision test.
		\param radius							The radius of the Bot moving from \c startPoint to \c destPoint.
		\param halfHeight						Half the height of the Bot moving from \c startPoint to \c destPoint.
		\param[out] lastValidPoint				Stores the coordinates of the last valid point for a Bot with the specified radius along the line segment.
		\param startTriangleSafePtr_in			A NavTriangleSafePtr that indicates the NavMesh triangle that corresponds to the starting point. 
												If you do not know this value, omit this parameter or use #KY_NULL. The value will be computed
												automatically.
		\param[out] startTriangleSafePtr_out	A NavTriangleSafePtr that will be used to store the NavMesh triangle that corresponds to the start point. 
												If you pass a valid NavTriangleSafePtr instance, it will be updated automatically. If you pass #KY_NULL or
												omit this parameter, no value will be computed.
		\param[out] lastValidTriangleSafePtr	A NavTriangleSafePtr that will be used to store the NavMesh triangle that corresponds to the last valid point. 
												If you pass a valid NavTriangleSafePtr instance, it will be updated automatically. If you pass #KY_NULL or
												omit this parameter, no value will be computed.
		\param allowedTerrainTypes				A bitmask that identifies the terrain types that should be considered navigable during the query.
		\param layerQuery						A NavMeshLayerQuery that lists the dynamic \INavMeshLayers that should be taken into account
												during the collision test.
		\param[out] layerLocation				If a collision is detected against an obstacle in an INavMeshLayer, this NavMeshLayerLocation object can be 
												used to identify the INavMeshLayer and the obstacle hit by the collision test. Its 
												NavMeshLayerLocation::m_layer member is set to point to the INavMeshLayer that caused the collision, and its 
												NavMeshLayerLocation::m_userData member is set by the INavMeshLayer to a value that may identity the obstacle 
												hit by the collision test. See the NavMeshLayerLocation class description for details on how to identify the
												obstacle from the data stored in the NavMeshLayerLocation.
		\return -	#KY_TRUE if the Bot can move in a direct line from \c startPoint to \c destPoint in 2D without hitting any obstacles in 
					a NavMesh or an INavMeshLayer.
				-	#KY_FALSE in all other cases.
		\pp Note that this method does not necessarily indicate whether or not a path exists
		between the \c startPoint and the \c destPoint. It is simply a collision test that determines whether or not the 
		straight line between the two points is blocked by any border of the static NavMesh, any Obstacle in a 
		specified INavMeshLayer, or any forbidden terrain types. */
	KyBool TraceLine(
		const Vec3f& startPoint, const Vec3f& destPoint, KyFloat32 radius, KyFloat32 halfHeight, Vec3f& lastValidPoint,
		const NavTriangleSafePtr* startTriangleSafePtr_in = KY_NULL,
		NavTriangleSafePtr* startTriangleSafePtr_out = KY_NULL,
		NavTriangleSafePtr* lastValidTriangleSafePtr = KY_NULL,
		NavTerrainTypeMask allowedTerrainTypes = NavTerrainTypeMask_All,
		const NavMeshLayerQuery* layerQuery = KY_NULL,
		NavMeshLayerLocation* layerLocation = KY_NULL);
	// -------------------------------------------------------------------------------------

	// --------------------- Propagation functions following a polyline --------------------
	/*!	Indicates whether or not a Bot with a specified radius can move along the specified polyline without 
		being stopped by any objects in the static \NavMeshes considered by this NavMeshQueryServerWrapper, 
		any objects in the dynamic \INavMeshLayers specified by the \c layerQuery parameter, or any forbidden terrain
		types. This method effectively chains a call to CanGo() for each segment in the polyline.
		\param points							An array that contains each point in the polyline.
		\param pointsCount						The number of points in the polyline.
		\param radius							The radius of the Bot moving along the polyline.
		\param halfHeight						Half the height of the Bot moving along the polyline.
		\param startTriangleSafePtr_in			A NavTriangleSafePtr that indicates the NavMesh triangle that corresponds to the starting point. 
												If you do not know this value, omit this parameter or use #KY_NULL. The value will be computed
												automatically.
		\param destTriangleSafePtr_in			A NavTriangleSafePtr that contains the NavMesh triangle that corresponds to the destination point. 
												If you do not know this value, omit this parameter or use #KY_NULL. The value will be computed
												automatically.
		\param[out] destTriangleSafePtr_out		A NavTriangleSafePtr that will be used to store the NavMesh triangle that corresponds to the last valid point. 
												If you pass a valid NavTriangleSafePtr instance, it will be updated automatically. If you pass #KY_NULL or
												omit this parameter, no value will be computed.
		\param[out] collisionSegmentIndex		The index of the segment in the polyline at which the first collision was detected (if any).
		\param allowedTerrainTypes				A bitmask that identifies the terrain types that should be considered navigable during the query.
		\param layerQuery						A NavMeshLayerQuery that lists the dynamic \INavMeshLayers that should be taken into account
												during the collision test.
		\return -	#KY_TRUE if the Bot can move in a direct line along the polyline without hitting any obstacles in 
					a NavMesh or an INavMeshLayer, and if \c destPoint lies on the same floor as \c startPoint.
				-	#KY_FALSE in all other cases. */
	KyBool CanGo_Polyline(
		const Vec3f* points, KyUInt32 pointsCount, KyFloat32 radius, KyFloat32 halfHeight,
		const NavTriangleSafePtr* startTriangleSafePtr_in = KY_NULL, const NavTriangleSafePtr* destTriangleSafePtr_in = KY_NULL,
		NavTriangleSafePtr* destTriangleSafePtr_out = KY_NULL,
		KyUInt32* collisionSegmentIndex = KY_NULL,
		NavTerrainTypeMask allowedTerrainTypes = NavTerrainTypeMask_All,
		const NavMeshLayerQuery* layerQuery = KY_NULL);

	/*!	Determines whether or not a Bot with a specified radius can move along the segments of the specified polyline in 2D without being 
		stopped by any objects in the static \NavMeshes considered by this NavMeshQueryServerWrapper, any objects in the dynamic \INavMeshLayers 
		specified by the \c layerQuery parameter, or any forbidden terrain types. This method also provides access to the point at which the 
		first collision was detected (if any), and information about the INavMeshLayer obstacle that caused the collision (if any). This method 
		effectively chains a call to TraceLine() for each segment in the polyline.
		\param points							An array that contains each point in the polyline. 
		\param pointsCount						The number of points in the polyline.
		\param radius							The radius of the Bot moving from \c startPoint to \c destPoint.
		\param halfHeight						Half the height of the Bot moving from \c startPoint to \c destPoint.
		\param[out] lastValidPoint				Stores the coordinates of the last valid point for a Bot with the specified radius along the line segment.
		\param startTriangleSafePtr_in			A NavTriangleSafePtr that indicates the NavMesh triangle that corresponds to the starting point. 
												If you do not know this value, omit this parameter or use #KY_NULL. The value will be computed
												automatically.
		\param[out] lastValidTriangleSafePtr	A NavTriangleSafePtr that will be used to store the NavMesh triangle that corresponds to the last valid point. 
												If you pass a valid NavTriangleSafePtr instance, it will be updated automatically. If you pass #KY_NULL or
												omit this parameter, no value will be computed.
		\param[out] collisionSegmentIndex		The index of the segment in the polyline at which the first collision was detected (if any).
		\param allowedTerrainTypes				A bitmask that identifies the terrain types that should be considered navigable during the query.
		\param layerQuery						A NavMeshLayerQuery that lists the dynamic \INavMeshLayers that should be taken into account
												during the collision test.
		\param[out] layerLocation				If a collision is detected against an obstacle in an INavMeshLayer, this NavMeshLayerLocation object can be 
												used to identify the INavMeshLayer and the obstacle hit by the collision test. Its 
												NavMeshLayerLocation::m_layer member is set to point to the INavMeshLayer that caused the collision, and its 
												NavMeshLayerLocation::m_userData member is set by the INavMeshLayer to a value that may identity the obstacle 
												hit by the collision test. See the NavMeshLayerLocation class description for details on how to identify the
												obstacle from the data stored in the NavMeshLayerLocation. 
		\return -	#KY_TRUE if the Bot can move in a direct line from \c startPoint to \c destPoint in 2D without hitting any obstacles in 
					a NavMesh or an INavMeshLayer.
				-	#KY_FALSE in all other cases. */
	KyBool TraceLine_Polyline(
		const Vec3f* points, KyUInt32 pointsCount, KyFloat32 radius, KyFloat32 halfHeight, Vec3f& lastValidPoint,
		const NavTriangleSafePtr* startTriangleSafePtr_in = KY_NULL,
		NavTriangleSafePtr* lastValidTriangleSafePtr = KY_NULL,
		KyUInt32* collisionSegmentIndex = KY_NULL,
		NavTerrainTypeMask allowedTerrainTypes = NavTerrainType_All,
		const NavMeshLayerQuery* layerQuery = KY_NULL,
		NavMeshLayerLocation* layerLocation = KY_NULL);
	// -------------------------------------------------------------------------------------



	// ------------------------------ LayerLocation functions ------------------------------
	/*!	Retrieves information about all points at which a collision is detected with an obstacle
		in an INavMeshLayer when a Bot with the specified radius moves in a straight line from \c startPoint to \c destPoint.
		\param startPoint				The starting point of the collision test.
		\param destPoint				The ending point of the collision test.
		\param radius					The radius of the Bot moving from \c startPoint to \c destPoint.
		\param halfHeight				Half the height of the Bot moving from \c startPoint to \c destPoint.
		\param startTriangleSafePtr		A NavTriangleSafePtr that contains the NavTrianglePtr that corresponds to the starting point. 
										If you do not know this value, omit this parameter or use #KY_NULL. The value will be computed
										automatically.
		\param layerQuery				A NavMeshLayerQuery that lists the dynamic \INavMeshLayers that should be taken into account
										during the collision test.
		\param[out] layerLocations		If one or more collisions are detected against an obstacle in an INavMeshLayer, this NavMeshLayerLocation object can be 
										used to identify the \INavMeshLayers and the obstacles hit by the collision test. For each collision, a NavMeshLayerLocation
										is added to this list. Its NavMeshLayerLocation::m_layer member is set to point to the INavMeshLayer that 
										caused the collision, and its NavMeshLayerLocation::m_userData member is set by the INavMeshLayer to a value 
										that may identity the obstacle hit by the collision test. For instance, the implementation of this method in the ObstacleLayer class sets 
										NavMeshLayerLocation::m_userData to the same value as the void \c userData pointer supplied when the obstacle 
										was added to the ObstacleLayer. 
		\return #KY_TRUE if the \c locations array was successfully updated with the collision points detected by this method, or #KY_FALSE otherwise. */
	KyBool GetAllLayerLocations(
		const Vec3f& startPoint, const Vec3f& destPoint, KyFloat32 radius, KyFloat32 halfHeight, const NavTriangleSafePtr* startTriangleSafePtr,
		NavMeshLayerQuery& layerQuery, NavMeshLayerLocationList& layerLocations);
	// -------------------------------------------------------------------------------------


	// ------------------------------ Location functions -----------------------------------
	/*!	Indicates whether or not the specified point is at a valid location
		according to all \NavMeshes considered by this NavMeshQueryServerWrapper.
		\pp Note that if you intend to retrieve a NavTriangleSafePtr for the same
		point by calling ComputeMultiMeshLocation(), you can test the validity
		of the point by calling ComputeMultiMeshLocation() directly, retrieving
		the NavTrianglePtr stored in the computed NavTriangleSafePtr, and calling
		its NavTrianglePtr::IsValid() method. */
	KyBool IsInsideNavMeshes(const Vec3f& pos, KyFloat32 halfHeight);

	/*!	Retrieves the NavTrianglePtr associated with the specified position in
		3D space, and saves it in the specified NavTriangleSafePtr object.
		\pp Use this method only to retrieve the NavTriangleSafePtr for a specific
		custom usage. If you intend to call the CanGo(), TraceLine() or
		GetAllLayerLocations() methods, do not compute a NavTriangleSafePtr for
		your starting point or ending point beforehand. Instead, pass an empty
		NavTriangleSafePtr to the CanGo(), TraceLine() or GetAllLayerLocations()
		methods, which will compute the NavTriangleSafePtr if necessary.
		\param pos					The position whose NavTriangleSafePtr will be computed.
		\param halfHeight			Half the height of the characters using the NavMesh.
		\param triangleSafePtr		Stores the NavTrianglePtr associated with the specified position. */
	KyBool ComputeNavTriangleSafePtr(const Vec3f& pos, KyFloat32 halfHeight, NavTriangleSafePtr& triangleSafePtr);

	/*!	Retrieves the altitude of the NavMesh that corresponds to the specified position in 3D space.
		\param pos				The position whose altitude is to be computed.
		\param halfHeight		Half the height of the characters using the NavMesh.
		\param meshAltitude		The altitude (or Z axis coordinate) of the static NavMesh under the specified point.
		\param trianglePtr		Specifies the NavTrianglePtr that corresponds to \c pos. This parameter is optional; if you do not
								provide it, it will be computed automatically. If you do provide it, the value you pass will be used
								without validation.
		\return The NavTrianglePtr that corresponds to the \c pos parameter (either the NavTrianglePtr computed by this method, or the
				one provided in the \c trianglePtr parameter). If you allowed this function to compute the NavTrianglePtr automatically,
				you can test whether or not \c pos is a valid position according to the NavMesh by calling the NavTrianglePtr::IsValid()
				method of the returned NavTrianglePtr. */
	NavTrianglePtr ComputeMeshAltitude(const Vec3f& pos, KyFloat32 halfHeight, KyFloat32& meshAltitude, const NavTrianglePtr& trianglePtr = NavTrianglePtr());


	// ------------------------------ Debug rendering functions -----------------------------------
	/*!	Draws the specified NavTrianglePtr for debugging purposes.
		\param navTrianglePtr			The NavTrianglePtr to be drawn.
		\param red						\label_redval
		\param green					\label_greenval
		\param blue						\label_blueval 
		\return #KY_TRUE if the NavTrianglePtr is valid, or #KY_FALSE otherwise. */
	KyBool Render_NavTriangle(const NavTrianglePtr& navTrianglePtr, KyUInt32 red, KyUInt32 green, KyUInt32 blue) const;
	
	/*!	Draws the specified NavFloorPtr for debugging purposes.
		\param navFloorPtr				The NavFloorPtr to be drawn.
		\param red						\label_redval
		\param green					\label_greenval
		\param blue						\label_blueval 
		\return #KY_TRUE if all NavTrianglePtr objects contained in the NavFloorPtr are valid, or #KY_FALSE otherwise. */
	KyBool Render_NavFloor(const NavFloorPtr& navFloorPtr, KyUInt32 red, KyUInt32 green, KyUInt32 blue) const;


	// ------------------------------ Utility functions -----------------------------------
	/*! For internal use. */
	KyBool GetNavFloorAnchor(const NavFloorPtr& navFloorPtr, Vec3f& anchor) const;

	/*! For internal use. */
	NavMeshQueryServerResult GetNavFloorPtrsInBoxObstacleForOneMesh(NavFloorsFromBoxObstacleQuery& query, const NavMesh& navMesh);
};


} // Kaim

#endif // KY_NAVMESHQUERYSERVERWRAPPER_H
