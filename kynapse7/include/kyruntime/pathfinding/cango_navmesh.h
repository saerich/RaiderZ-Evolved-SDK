/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_CANGO_NAVMESH_H
#define KY_CANGO_NAVMESH_H

#include <kyruntime/pathfinding/icango.h>
#include <kyruntime/mesh/navmeshlayerquery.h>
#include <kyruntime/mesh/inavmeshlayer.h>

namespace Kaim
{
/*! This implementation of the ICanGo interface tests whether or not a Bot can move in a straight line
	between two points by testing the obstacles contained in the static
	NavMesh, and optionally the obstacles projected into one or more dynamic \ObstacleLayers.
	\pp The static NavMesh is retrieved from the Database configured for the PathFinder
	using this modifier. If no NavMesh is available in this Database, all CanGo() and TraceLine()
	tests will fail.
	\pp This modifier distinguishes between two different types of dynamic \ObstacleLayers:
	-	Lpf layers: \ObstacleLayers that contain outlines aggregated into \LpfAreas by the LpfManager. These
		layers are retrieved automatically from the \LpfContext set for the PathFinder.
		Whenever the path of a Bot crosses an outline contained in one of these layers, the PathFinder
		queries the LpfManager to generate a bypass trajectory. See :USERGUIDE:"Local PathFinding".
	-	Blocking layers: \ObstacleLayers that contain outlines of no specific type, but that are considered
		to block the movement of \Bots. You can define these layers by calling the AddBlockingLayer(), RemoveBlockingLayer()
		and ClearBlockingLayer() methods. Whenever the path of a Bot crosses an outline contained in one of these
		layers, the PathFinder considers the Path to be blocked by the ObstacleLayer. In typical pathfinding scenarios, this
		can occur when testing whether the Bot can smooth its Path to a new target PathNode, and when attempting
		to find the nearest reachable PathNode to the current position of the Bot.
	\pp Each ObstacleLayer may be used either as a source of blocking obstacles or as a source for Local PathFinding outlines,
	but not both.
	\pp The ObstacleManager automatically swaps each of your \Obstacles between these two \ObstacleLayers depending on its current
	speed of movement and on the class of IOutlineUpdatePolicy set up for the ObstacleManager. See :USERGUIDE:"Using Obstacles".
	\pp If you use \PathObjects, the PathObjectManager automatically projects their outlines into a special ObstacleLayer that the
	CanGo_NavMesh class automatically recognizes as a blocking layer. This allows the PathFinder to recognize when the path of its
	Bot crosses the boundaries of the PathObject control space.
	See also :USERGUIDE:"Using PathObjects for Special Movements".
	\pp By default, the implementation of the CanGo() and TraceLine() methods in this class take into account
	both BlockingLayers and LpfLayers. However, CanGo() and TraceLine() are overloaded to accept an optional argument
	that specifies which types of ObstacleLayer should be considered in the query. The PathFinder
	and LpfManager use these methods to evaluate different types of layers at different points
	in the pathfinding process. You can use these overloaded methods yourself by constructing a NavMeshLayerQuery
	object -- a wrapper around a set of pointers to ObstacleLayer instances -- and passing this object as an extra argument
	to the CanGo() and TraceLine() methods.
	\ingroup kyruntime_pathfinding */
class CanGo_NavMesh : public ICanGo
{
public:
	KY_DECLARE_CANGO(CanGo_NavMesh)

public:
	/*! Used to indicate different types of \ObstacleLayers that can be included in CanGo() and TraceLine() tests. */
	enum CANGO_NAVMESH_OBSTACLE_FLAGS
	{
		BLOCKING_LAYERS = 1, /*!< Reflects all \ObstacleLayers identified as BlockingLayers. */
		LPF_LAYERS = 2,      /*!< Reflects all \ObstacleLayers used to build the \LpfContext used by the PathFinder that owns this CanGo_NavMesh instance. */
		ObstacleType_DUMMY = KYFORCEENUMSIZEINT
	};

public:
	/*! \label_constructor */
	CanGo_NavMesh() :
		ICanGo(),
		m_layerManager(KY_NULL)
	{}

	/*! \label_virtualdestructor */
	virtual ~CanGo_NavMesh();


	//////////////////////////////////////////////////////////////////////////
	// Implementation of ICanGo
	//////////////////////////////////////////////////////////////////////////

	// Remove warning 4266: no override available for virtual member function from base; function is hidden.
	using ICanGo::CanGo;
	using ICanGo::TraceLine;

	/*! \copydoc ICanGo::CanGo()
		\pp Note that this implementation of ICanGo::CanGo() does not necessarily indicate whether or not a path exists
		between the \c start point and the \c dest point. It is simply a collision test that determines whether or not the 
		straight line between the two points is blocked by any border of the static NavMesh, or any Obstacle in a 
		blocking layer or Lpf layer. */
	virtual KyBool CanGo(const PointWrapper& start, const PointWrapper& dest, KyFloat32 radius)
	{
		NavMeshLayerQuery layerQuery(&m_allLayers);
		return CanGo(start, dest, radius, &layerQuery);
	}

	/*! Indicates whether or not a Bot with the specified radius can pass freely along a line
		segment that extends between the two specified points, taking into account the obstacles in
		the static NavMesh and the specified \ObstacleLayers.
		\param start				The start point of the line segment.
		\param dest					The end point of the line segment.
		\param radius				The radius of the Bot to be tested.
		\param layerQuery			An instance of NavMeshLayerQuery that contains pointers to the \ObstacleLayers
									that should be included in the query.
		\return #KY_TRUE if the Bot can follow the direct line from \c start to \c dest. 
		\pp Note that this method does not necessarily indicate whether or not a path exists
		between the \c start point and the \c dest point. It is simply a collision test that determines whether or not the 
		straight line between the two points is blocked by any border of the static NavMesh, or any Obstacle in a 
		blocking layer or Lpf layer. */
	virtual KyBool CanGo(const PointWrapper& start, const PointWrapper& dest,
		KyFloat32 radius, const NavMeshLayerQuery* layerQuery);

	/*! \copydoc ICanGo::TraceLine()
		\pp Note that this implementation of ICanGo::CanGo() does not necessarily indicate whether or not a path exists
		between the \c start point and the \c dest point. It is simply a collision test that determines whether or not the 
		straight line between the two points is blocked by any border of the static NavMesh, or any Obstacle in a 
		blocking layer or Lpf layer. */
	virtual void TraceLine(const PointWrapper& start, const PointWrapper& dest, KyFloat32 radius, PointWrapper& lastValidPoint)
	{
		NavMeshLayerQuery layerQuery(&m_allLayers);
		TraceLine(start, dest, radius, lastValidPoint, &layerQuery);
	}

	/*! Similar to CanGo(), this method determines whether or not a Bot with the specified radius
		can pass freely along a line segment that extends between the two specified points, taking into
		account the obstacles in the static NavMesh and the specified \ObstacleLayers. This method
		also computes the last valid position along the line segment. If an obstacle is detected, this point
		is expected to be the last point on that straight line before hitting the obstacle. If an obstacle
		is not detected, this point is the same as the destination point.
		\param start				The start point of the line segment.
		\param dest					The end point of the line segment.
		\param radius				The radius of the Bot to be tested.
		\param lastValidPoint		The last valid point along the line segment that can be occupied by
									the Bot without colliding with an obstacle.
		\param layerQuery			An instance of NavMeshLayerQuery that contains pointers to the \ObstacleLayers
									that should be included in the query.
		\pp Note that this method does not necessarily indicate whether or not a path exists
		between the \c start point and the \c dest point. It is simply a collision test that determines whether or not the 
		straight line between the two points is blocked by any border of the static NavMesh, or any Obstacle in a 
		blocking layer or Lpf layer. */
	virtual KyBool TraceLine(const PointWrapper& start, const PointWrapper& dest,
		KyFloat32 radius, PointWrapper& lastValidPoint, const NavMeshLayerQuery* layerQuery);

	/*! Similar to TraceLine(), except that it tests for collisions only against the specified \ObstacleLayers,
		without considering the static NavMesh. It also retrieves information that identifies the first obstacle
		detected.
		\param start						The start point of the line segment.
		\param destPos						The end point of the line segment.
		\param radius						The radius of the Bot to be tested.
		\param layerQuery					An instance of NavMeshLayerQuery that contains pointers to the \ObstacleLayers
											that should be included in the query.
		\param[out] layerLocation			If this method detects a collision, it updates this parameter to store
											the ObstacleLayer that contains the obstacle, and information about the
											obstacle drawn from that ObstacleLayer.
		\param[out] lastValidPosition		The last valid point along the line segment that can be occupied by
											the Bot without colliding with an obstacle.
		\return #KY_TRUE if the test was completed successfully, or #KY_FALSE if the test failed unexpectedly (i.e. not
				simply due to detecting a collision). For example, #KY_FALSE may indicate that the starting point was
				outside the boundaries of the static NavMesh. */
	virtual KyBool GetNearestLayerLocation(const PointWrapper& start, const Vec3f& destPos,
		KyFloat32 radius, const NavMeshLayerQuery& layerQuery,
		NavMeshLayerLocation& layerLocation, Vec3f *lastValidPosition = KY_NULL);

	/*! Similar to GetNearestLayerLocation(), but retrieves information about all obstacles that intersect the
		specified line segment.
		\param start						The start point of the line segment.
		\param destPos						The end point of the line segment.
		\param radius						The radius of the Bot to be tested.
		\param layerQuery					An instance of NavMeshLayerQuery that contains pointers to the \ObstacleLayers
											that should be included in the query.
		\param[out] layerLocations			If this method detects one or more collisions, it updates this parameter
											to store the \ObstacleLayers that contain the obstacle, and information
											about the obstacles drawn from the \ObstacleLayers.
		\return #KY_TRUE if the test was completed successfully, or #KY_FALSE if the test failed unexpectedly
				(i.e. not simply due to detecting a collision). For example, #KY_FALSE may indicate that the starting point was
				outside the boundaries of the static NavMesh, or that the \c layerLocations array was not large enough
				to hold all obstacles detected on the specified line segment. */
	virtual KyBool GetAllLayerLocations(const PointWrapper& start, const Vec3f& destPos,
		KyFloat32 radius, NavMeshLayerQuery& layerQuery, NavMeshLayerLocationList& layerLocations);


public:
	//////////////////////////////////////////////////////////////////////////
	// BlockingLayer management
	//////////////////////////////////////////////////////////////////////////

	/*! Clears the list of blocking layers. */
	void ClearBlockingLayerList();

	/*! Adds the specified ObstacleLayer to the list of blocking layers. */
	void AddBlockingLayer(INavMeshLayer *layer);

	/*! Removes the specified ObstacleLayer from the list of blocking layers. */
	void RemoveBlockingLayer(INavMeshLayer *layer);


	//////////////////////////////////////////////////////////////////////////
	// Extended interface used typically by the ILpfManager
	//////////////////////////////////////////////////////////////////////////

	/*! Similar to CanGo(), but accepts a bit mask that specifies the types of \ObstacleLayers that should
		be ignored during the test. This method is used internally by the LpfManager.
		\param start						The start point of the line segment.
		\param dest							The end point of the line segment.
		\param radius						The radius of the Bot to be tested.
		\param layersToIgnore				A bit mask constructed from one or more elements of the
											#CANGO_NAVMESH_OBSTACLE_FLAGS enumeration.
		\return #KY_TRUE if the Bot can follow the direct line from \c start to \c dest. */
	virtual KyBool CanGo_SelectLayerTypes(const PointWrapper& start, const PointWrapper& dest,
		KyFloat32 radius, KyUInt32 layersToIgnore);

	/*! Similar to TraceLine(), but accepts a bit mask that specifies the types of \ObstacleLayers that should
		be ignored during the test. This method is used internally by the LpfManager.
		\param start						The start point of the line segment.
		\param dest							The end point of the line segment.
		\param radius						The radius of the Bot to be tested.
		\param lastValidPoint				The last valid point along the line segment that can be occupied by
											the Bot without colliding with an obstacle.
		\param layersToIgnore				A bit mask constructed from one or more elements from the
											#CANGO_NAVMESH_OBSTACLE_FLAGS enumeration.
		\return #KY_TRUE if the Bot can follow the direct line from \c start to \c dest. */
	virtual KyBool TraceLine_SelectLayerTypes(const PointWrapper& start, const PointWrapper& dest,
		KyFloat32 radius, PointWrapper& lastValidPoint, KyUInt32 layersToIgnore);


protected:
	//////////////////////////////////////////////////////////////////////////
	// Internal methods
	//////////////////////////////////////////////////////////////////////////
	virtual void OnSetPathFinder();
	void SetupLayerQuery(NavMeshLayerQuery& layerQuery, KyUInt32 layersToIgnore);

	//////////////////////////////////////////////////////////////////////////
	// NavMesh queries
	//////////////////////////////////////////////////////////////////////////

	/* Called internally by CanGo(). */
	virtual KyBool CanGo_OnNavMesh(const PointWrapper& start, const PointWrapper& dest,
		KyFloat32 radius, const NavMeshLayerQuery* layerQuery);

	/* Called internally by TraceLine(). */
	virtual KyBool TraceLine_OnNavMesh(const PointWrapper& start, const PointWrapper& dest,
		KyFloat32 radius, PointWrapper& lastValidPoint, const NavMeshLayerQuery* layerQuery);

	/* Called internally by GetNearestLayerLocation(). */
	virtual KyBool GetNearestLayerLocation_OnNavMesh(const PointWrapper& start, const Vec3f& destPos,
		KyFloat32 radius, const NavMeshLayerQuery& layerQuery,
		NavMeshLayerLocation& layerLocation, Vec3f *lastValidPosition);

	/* Called internally by GetAllLayerLocations(). */
	virtual KyBool GetAllLayerLocations_OnNavMesh(const PointWrapper& start, const Vec3f& destPos,
		KyFloat32 radius, NavMeshLayerQuery& layerQuery, NavMeshLayerLocationList& layerLocations);


	//////////////////////////////////////////////////////////////////////////
	// Outside allowed / known manifold
	//////////////////////////////////////////////////////////////////////////

	/* Called transparently by CanGo() when the start point provided for the collision test (typically the current
		position of the Bot) lies inside the NavMesh but on a forbidden terrain type. This method first finds the
		nearest point on the straight line between the start and end points that lies inside the allowed terrain types.
		It then tests for obstacles along the segment that lies inside the allowed terrain types.
		\param start				The start position of the line segment.
		\param dest					The end position of the line segment. This point must lie within the boundaries of the NavMesh.
		\param radius				The radius of the Bot to be tested.
		\param halfHeight			The half height of the Bot to be tested.
		\param navMeshQueryServer	The NavMeshQueryServerWrapper through which the test will be be done.
		\param startTriangleSafePtr	The NavTrianglePtr at the start position.
		\param destTriangleSafePtr	The NavTrianglePtr at the destination position. If #KY_NULL, it will be computed.
		\param layerQuery			An instance of NavMeshLayerQuery that contains pointers to the \ObstacleLayers
									that should be included in the query.
		\return #KY_TRUE if both the segment outside and the segment inside the allowed terrain types are clear
				of obstacles, or #KY_FALSE otherwise. */
	KyBool CanGo_FromOutsideAllowedTerrainTypes(
		const PointWrapper& start, const PointWrapper& dest, KyFloat32 radius, KyFloat32 halfHeight,
		NavMeshQueryServerWrapper& navMeshQueryServer, const NavTriangleSafePtr& startTriangleSafePtr, NavTriangleSafePtr* destTriangleSafePtr,
		const NavMeshLayerQuery* layerQuery);

	/* Called transparently by CanGo() when the start point provided for the collision test (typically the current
		position of the Bot) lies outside the NavMesh but the destination point lies inside the NavMesh. This method
		first finds the nearest point on the straight line between the start and end points that lies inside the NavMesh.
		It then treats the segments of the line that lie outside and inside the NavMesh separately:
		-	To test for obstacles along the segment that lies outside the NavMesh, it calls IsColliding().
		-	To test for obstacles along the segment that lies inside the NavMesh, it calls CanGo() passing on the parameters
			listed below.
		\param start				The start point of the line segment.
		\param dest					The end point of the line segment. This point must lie within the boundaries of the NavMesh.
		\param radius				The radius of the Bot to be tested.
		\param layerQuery			An instance of NavMeshLayerQuery that contains pointers to the \ObstacleLayers
									that should be included in the query.
		\return #KY_TRUE if both the segment outside the NavMesh and the segment inside the NavMesh are clear
				of obstacles, or #KY_FALSE otherwise. */
	KyBool CanGo_FromOutsideNavMesh(const PointWrapper& start, const PointWrapper& dest, KyFloat32 radius, const NavMeshLayerQuery* layerQuery);


	//////////////////////////////////////////////////////////////////////////
	// Between SmoothablePathObjects
	//////////////////////////////////////////////////////////////////////////

	/* The 4 following methods treat the corresponding request on the section up to
		the provided SmoothablePathObject.

		They update the output bool parameter "treatNextSmoothablePOs" as this:
		- true:  when the request have to be propagated to next SmoothablePathObject,
		         in that case, they update nextSectionStart position.
		- false: when the request propagation have to be stopped and
		         the remaining section (including the one requested there)
		         as to be treated on the NavMesh.

		The return value has the same meaning than the one of the corresponding
		NavMesh method.
	*/
	/* Called internally by CanGo(). */
	bool CanGo_TreatSectionUpToPathObject(
		const Vec3f& currentSectionStart, const Vec3f& finalDestination,
		KyFloat32 radius, const NavMeshLayerQuery* layerQuery,
		IPathObject* smoothablePathObject, Vec3f& nextSectionStart, bool& treatNextSmoothablePOs);

	/* Called internally by TraceLine(). */
	bool TraceLine_TreatSectionUpToPathObject(
		const Vec3f& currentSectionStart, const Vec3f& finalDestination,
		KyFloat32 radius, PointWrapper& lastValidPoint, const NavMeshLayerQuery* layerQuery,
		IPathObject* smoothablePathObject, Vec3f& nextSectionStart, bool& treatNextSmoothablePOs);

	/* Called internally by GetNearestLayerLocation(). */
	bool GetNearestLayerLocation_TreatSectionUpToPathObject(
		const Vec3f& currentSectionStart, const Vec3f& finalDestination,
		KyFloat32 radius, const NavMeshLayerQuery& layerQuery,
		NavMeshLayerLocation& layerLocation, Vec3f *lastValidPosition,
		IPathObject* smoothablePathObject, Vec3f& nextSectionStart, bool& treatNextSmoothablePOs);

	/* Called internally by GetAllLayerLocations(). */
	bool GetAllLayerLocations_TreatSectionUpToPathObject(
		const Vec3f& currentSectionStart, const Vec3f& finalDestination,
		KyFloat32 radius, NavMeshLayerQuery& layerQuery, NavMeshLayerLocationList& layerLocations,
		IPathObject* smoothablePathObject, Vec3f& nextSectionStart, bool& treatNextSmoothablePOs);


public:
	NavMeshLayerManager* m_layerManager; /*!< The NavMeshLayerManager for the World, or #KY_NULL if the NavMeshLayerManager is not set up. */
	INavMeshLayerPtrList m_blockingLayers; /*!< The current list of blocking layers, including the special PathObject layer. Do not modify directly. */
	INavMeshLayerPtrList m_allLayers; /*!< Contains pointers to all blocking layers, all Lpf layers, and the special PathObject layer. Do not modify directly. */
	INavMeshLayerPtrList m_tmpLayerList; /*!< A temporary list of layers used to set up new queries. Do not modify directly. */
};

} // namespace Kaim

#endif // KY_CANGO_NAVMESH_H
