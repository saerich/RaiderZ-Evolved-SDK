/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavMeshQueryServer_H
#define KyPathData_NavMeshQueryServer_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/navmeshquerybuffer.h"
#include "kypathdata/navmesh/navmeshtraversalcommon.h"
#include "kypathdata/navmesh/navmeshmanager.h"

namespace Kaim
{

class Box2f;
class Box2i;

class RayCanGoQuery;
class DiskCanGoQuery;
class SegmentCanGoQuery;

class RayCastQuery;
class DiskCastQuery;
class SegmentCastQuery;

class DiskCollisionQuery;
class DiskExpansionQuery;

class NearestTriangleFromPosQuery;
class TrianglesFromPosQuery;
class TrianglesFromBoxObstacleQuery;
class NavFloorsFromBoxObstacleQuery;
class NearestObstacleHalfEdgeFromPosQuery;
class InsidePosFromOutsidePosQuery;

template <class Item> class NmqsArray;

enum TraversalResult;


/*! Enumerates the possible results of a query made against a NavMeshQueryServer.
	\ingroup kypathdata_navmesh */
enum NavMeshQueryServerResult
{
	NMQSResult_DONE, /*!< Indicates that the query was completed successfully. */
	NMQSResult_FAILED, /*!< Indicates that an error occurred, causing the query to fail. */
	NMQSResult_LACK_OF_RESULT_MEMORY, /*!< Indicates that the query failed due a lack of memory for storing its results. */
	NMQSResult_LACK_OF_WORKING_MEMORY, /*!< Indicates that the query failed due to a lack of working memory. */
	NMQSResult_LACK_OF_PROPAGATION_MEMORY, /*!< Indicates that the query failed due to a lack of memory for storing the triangles crossed by the query. */

	NMQSResult_FORCE32 = 0xFFFFFFFF
};

/*! The NavMeshQueryServer class provides an interface for making perceptual queries against the static \NavMeshes loaded into
	a NavMeshManager.
	\pp At runtime, the methods of this class are typically used only indirectly through the instance of the derived
	NavMeshQueryServerWrapper class accessible through Database::GetNavMeshQueryServer(), which also takes into account
	dynamic \Obstacles and \PathObjects.
	\ingroup kypathdata_navmesh */
class NavMeshQueryServer
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	NavMeshQueryServer() : m_navMeshManager(KY_NULL), m_queryBuffer() {}

	/*! \label_constructor
		\param navMeshManager			The NavMeshManager whose \NavMeshes will be taken into account during queries.
		\param resultBufferSize			The size in bytes of a memory buffer to reserve for storing query results.
		\param workingBufferSize		The size in bytes of a memory buffer to reserve for temporary use during queries.
		\param propagationBufferSize	The size in bytes of a memory buffer to reserve for storing the triangles crossed during queries. */
	NavMeshQueryServer(const NavMeshManager& navMeshManager, KyUInt32 resultBufferSize, KyUInt32 workingBufferSize = 0, KyUInt32 propagationBufferSize = 0) : m_navMeshManager(&navMeshManager), m_queryBuffer(resultBufferSize, workingBufferSize, propagationBufferSize) {}

	/*! Sets the sizes of the memory buffers allocated for use by the NavMeshQueryServer.
		\param resultBufferSize			The size in bytes of a memory buffer to reserve for storing query results.
		\param workingBufferSize		The size in bytes of a memory buffer to reserve for temporary use during queries.
		\param propagationBufferSize	The size in bytes of a memory buffer to reserve for storing the triangles crossed during queries. */
	void SetMemoryBufferSizes(KyUInt32 resultBufferSize, KyUInt32 workingBufferSize = 0, KyUInt32 propagationBufferSize = 0)
	{
		m_queryBuffer.Set(resultBufferSize, workingBufferSize, propagationBufferSize);
	}

	/*! Sets the NavMeshManager whose \NavMeshes will be taken into account during queries. */
	void SetNavMeshManager(const NavMeshManager& navMeshManager) { m_navMeshManager = &navMeshManager; }

public:
	/*! Retrieves the nearest triangle in the NavMesh that covers the (X,Y) coordinates of the
		specified position.
		See NearestTriangleFromPosQuery for details on inputs and outputs.
		\sa To retrieve a valid point within the NavMesh from an input point that is outside the (X,Y) boundaries
		of the NavMesh, see GetInsidePosFromOutsidePos().
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult GetNearestTrianglePtrFromPos(NearestTriangleFromPosQuery& query);

	/*! Retrieves all or multiple triangles in the NavMesh that cover the (X,Y) coordinates of the
		specified position.
		See TrianglesFromPosQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult GetTriangleIndicesFromPosition(TrianglesFromPosQuery& query);

	/*! Retrieves all triangles in the NavMesh that intersect a specified bounding box.
		See TrianglesFromBoxObstacleQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult GetTriangleIndicesInBoxObstacle(TrianglesFromBoxObstacleQuery& query);

	/*! Retrieves all \NavFloors in the NavMesh that intersect a specified bounding box.
		See NavFloorsFromBoxObstacleQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult GetNavFloorPtrsInBoxObstacle(NavFloorsFromBoxObstacleQuery& query);


	/*! Retrieves the NavMesh border nearest to a specified position. The input position may be inside or outside the NavMesh.
		See NearestObstacleHalfEdgeFromPosQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult GetNearestObstacleHalfEdgeFromPos(NearestObstacleHalfEdgeFromPosQuery& query);

	/*! Retrieves a position inside the boundaries of the NavMesh, at a specified distance from the NavMesh boundary that lies nearest
		to a specified input position. See InsidePosFromOutsidePosQuery for details on inputs and outputs.
		\pp This query is typically used to retrieve a valid NavMesh position that is as near as possible to a position that
		lies outside the (X,Y) boundaries of the NavMesh. To retrieve the nearest NavMesh triangle from an input position that
		is inside the (X,Y) boundaries of the NavMesh, see GetNearestTrianglePtrFromPos().
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult GetInsidePosFromOutsidePos(InsidePosFromOutsidePosQuery& query);

	/*********************************************************************************/
	/*                             RayCanGo / DiskCanGo                              */
	/*********************************************************************************/
public:
	/*! Tests whether or not a ray (with a width of 0) can pass from a starting point to an ending point
		without colliding with any NavMesh borders or crossing any forbidden terrain types.
		See RayCanGoQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult RayCanGo(RayCanGoQuery& query);

	/*! Tests whether or not a disk with a specified radius can pass from a starting point to an ending point
		without colliding with any NavMesh borders or crossing any forbidden terrain types.
		See DiskCanGoQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult DiskCanGo(DiskCanGoQuery& query);
private:
	/*
	This function is called by the previous one if the start disk collides with the wall.
	We modify the query to be able to extract from wall
	*/
	NavMeshQueryServerResult ForceDiskCanGo(DiskCanGoQuery& query);




	/*********************************************************************************/
	/*                             RayCast / DiskCast                                */
	/*********************************************************************************/
public:
	/*! Tests whether or not a ray (with a width of 0) can be cast from a starting point in a specified
		direction without colliding with the NavMesh border or crossing any forbidden terrain types.
		See RayCastQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return #NMQSResult_DONE if the query was completed successfully, #NMQSResult_LACK_OF_RESULT_MEMORY if too
				many triangles were crossed, or #NMQSResult_FAILED if an error occurred. */
	NavMeshQueryServerResult RayCast(RayCastQuery& query);

	/*! Tests whether or not a disk with a specified radius can be cast from a starting point in a specified
		direction without colliding with the NavMesh border or crossing any forbidden terrain types.
		See DiskCastQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return #NMQSResult_DONE if the query was completed successfully, #NMQSResult_LACK_OF_RESULT_MEMORY if too
				many triangles were crossed, or #NMQSResult_FAILED if an error occurred. */
	NavMeshQueryServerResult DiskCast(DiskCastQuery& query);
private:
	/*
	This function is called by the previous one if the start disk collides with the wall.
	We modify the query to be able to extract from wall
	*/
	NavMeshQueryServerResult ForceDiskCast(DiskCastQuery& query);




	/*********************************************************************************/
	/*                             SegmentCanGo / SegmentCast                        */
	/*********************************************************************************/
public:
	/*! Tests whether or not a line segment with a specified half-width can pass from a starting point to an ending point
		without colliding with any NavMesh borders or crossing any forbidden terrain types.
		See SegmentCanGoQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult SegmentCanGo(SegmentCanGoQuery& query);

	/*! Tests whether or not a line segment with a specified half-width can pass from a starting point to an ending point
		without colliding with any NavMesh borders or crossing any forbidden terrain types.
		See DiskCanGoQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult SegmentCast(SegmentCastQuery& query);



	/*********************************************************************************/
	/*                           DiskCollision / DiskExpansion                       */
	/*********************************************************************************/
public:
	/*! Tests whether or not a disk with a specified radius can be placed at a specified position without intersecting
		any NavMesh border or crossing any forbidden terrain types.
		See DiskCollisionQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult DiskCollision(DiskCollisionQuery& query);

	/*! Places a disk at a specified position, and expands it until it intersects a NavMesh border or crosses a forbidden
		terrain type.
		See DiskExpansionQuery for details on inputs and outputs.
		\param query		An object that encapsulates all input parameters and results of the query.
		\return An element from the #NavMeshQueryServerResult enumeration that indicates the success or failure of
				the query. */
	NavMeshQueryServerResult DiskExpansion(DiskExpansionQuery& query);


public:
	/*! Retrieves the altitude of the point on the specified triangle that has the same (X,Y) coordinates as
		the specified position. */
	KyFloat32 GetAltitudeOfPointInTriangle(const Vec3f& pos, const NavTrianglePtr& trianglePtr)
	{
		return GetAltitudeOfPointInTriangle(pos, trianglePtr.GetNavFloor(), trianglePtr.GetTriangleIdx());
	}

protected :
	/*
	Compute the altitude of the point in the plane define by the triangle
	*/
	KyFloat32 GetAltitudeOfPointInTriangle(const Vec3f& pos, const NavFloor& floor, NavTriangleIdx triangleIdx);

	/*
	This function is used by RayCast and RayCanGo to know which edge to cross in the start triangle
	*/
	KyInt32 GetfirstEdgeToCrossInStartTriangle(const NavTrianglePtr& startTrianglePtr, const Vec3f& startPos, const Vec3f& destPos);

	/*
	The stop condition during the RayCast or the RayCanGo
	*/
	bool EdgeIsBeyondDestPos(const NavHalfEdgePtr& halfEdgeId, const Vec3f& startPos, const Vec3f& destPos);

	/*
	In RayCast and RayCanGo, we may stop in a triangle that is different from the one returned by GetNearestTrianglePtrFromPos(destPos), we need to solve this inconsistency
	*/
	bool SolveInconsistencyBetweenArrivalTriangleAndGetNearestTriangle(const NavTrianglePtr& lastTrianglePtr, const NavTrianglePtr& destTrianglePtr, const Vec3f& destPos);

	/*
	This function is called in function that use a traversal to propagate the potential Traversal Error
	*/
	NavMeshQueryServerResult PropagateTraversalError(TraversalResult traversalRC);


	void GetCellBoxOfAnAABBox(const Box2f& aABBofOOBB, Box2i& overlappedSquareBox) const;

public:
	const NavMeshManager* m_navMeshManager; /*!< The NavMeshManager whose data will be taken into account by queries made through this object. Do not modify. */
	NavMeshQueryBuffer m_queryBuffer;       /*!< Contains memory buffers used for performing queries and storing results. For internal use. Do not modify. */
};

}

#endif //KyPathData_NavMeshQueryServer_H

