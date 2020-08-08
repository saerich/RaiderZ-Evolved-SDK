/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_RayCastQuery_H
#define KyPathData_RayCastQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/propagationquerymemory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of a RayCastQuery.
	\ingroup kypathdata_navmesh */
enum RayCastQueryResult
{
	RAYCAST_MAXDIST_REACHED, /*!< Indicates that the ray was able to travel its maximum distance without collision. */
	RAYCAST_START_OUTSIDE, /*!< Indicates that no NavMesh triangle could be found for the starting point. */
	RAYCAST_START_MARTERIAL_FORBIDDEN,
	RAYCAST_CANNOT_MOVE, /*!< Indicates that the starting point is very close to an obstacle, such that moving back the collision point resulted in an arrival position that is behind the starting point. */
	RAYCAST_ARRIVAL_ERROR, /*!< Indicates that moving back the collision point resulted in an arrival position that is outside of the NavMesh. */
	RAYCAST_ARIVALPOS_FOUND /*!< Indicates that a final arrival position was calculated successfully. */
};

/*!	The RayCastQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::RayCast().
	\pp This query casts a ray in a specified orientation (#m_normalizedDir2d) from a 
	starting point (#m_startPos). If the ray collides with a border of the NavMesh or crosses any forbidden terrain types,
	the collision point is recorded (#m_collisionPos). The ray stops if it travels a maximum distance without colliding 
	(#m_maxDist). In either case, the final valid position is recorded in #m_arrivalPos.
	\ingroup kypathdata_navmesh */
class RayCastQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor
		\param maxDist				Sets the value of #m_maxDist. 
		\param safetyDist			Sets the value of #m_safetyDist. */
	RayCastQuery(KyFloat32 maxDist = 0.f, KyFloat32 safetyDist = 0.01f) :
	 m_terrainTypeMask(NavTerrainTypeMask_All), m_maxDist(maxDist), m_safetyDist(safetyDist), m_searchRange(1.f), m_reachMaxDist(false), m_consistentResult(false) {}

	/*! \label_constructor
		\param startPos				Sets the value of #m_startPos. 
		\param normalizedDir2d		Sets the value of #m_normalizedDir2d.
		\param maxDist				Sets the value of #m_maxDist.
		\param safetyDist			Sets the value of #m_safetyDist. */
	RayCastQuery(const Vec3f& startPos, const Vec2f& normalizedDir2d, KyFloat32 maxDist, KyFloat32 safetyDist = 0.01f) :
	 m_startPos(startPos), m_normalizedDir2d(normalizedDir2d), m_terrainTypeMask(NavTerrainTypeMask_All), m_maxDist(maxDist), m_safetyDist(safetyDist), m_searchRange(1.f), m_reachMaxDist(false), m_consistentResult(false) {}

	/*! \label_constructor
		\param startPos				Sets the value of #m_startPos. 
		\param startTriangleSafePtr	Sets the value of #m_startTriangleSafePtr. 
		\param normalizedDir2d		Sets the value of #m_normalizedDir2d.
		\param maxDist				Sets the value of #m_maxDist.
		\param safetyDist			Sets the value of #m_safetyDist. */
	RayCastQuery(const Vec3f& startPos, NavTriangleSafePtr startTriangleSafePtr, const Vec2f& normalizedDir2d, KyFloat32 maxDist, KyFloat32 safetyDist = 0.01f) :
	 m_startPos(startPos),  m_normalizedDir2d(normalizedDir2d), m_startTriangleSafePtr(startTriangleSafePtr), m_terrainTypeMask(NavTerrainTypeMask_All), m_maxDist(maxDist), m_safetyDist(safetyDist), m_searchRange(1.f), m_reachMaxDist(false), m_consistentResult(false) {}

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::RayCast().
		\return An element from the #RayCastQueryResult enumeration that indicates the results of the query. */
	RayCastQueryResult GetResult() const
	{
		if (m_reachMaxDist)
			return RAYCAST_MAXDIST_REACHED;

		if (!m_startTriangleSafePtr.IsValid())
			return RAYCAST_START_OUTSIDE;

		if(!IsTerrainTypeCompliant(m_terrainTypeMask, m_startTriangleSafePtr.GetNavFloorTerrainType()))
			return RAYCAST_START_MARTERIAL_FORBIDDEN;

		if (m_startPos == m_arrivalPos)
			return RAYCAST_CANNOT_MOVE;

		if (m_arrivalTrianglePtr.IsValid())
			return RAYCAST_ARIVALPOS_FOUND;

		return RAYCAST_ARRIVAL_ERROR;
	}

	/*! \label_clear */
	KY_FORCE_INLINE void Clear()
	{
		m_maxDist = 0.f;
		m_safetyDist = 0.01f;
		m_terrainTypeMask = NavTerrainTypeMask_All;
		m_searchRange = 1.f;

		m_startTriangleSafePtr.Invalidate();

		m_normalizedDir2d = Vec2f();

		CleanOutput();
	}

	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	KY_FORCE_INLINE void CleanOutput()
	{
		m_reachMaxDist = false;
		m_consistentResult = false;

		m_arrivalTrianglePtr.Invalidate();
		m_intersectedHalfEdgePtr.Invalidate();

		m_arrivalPos.Set(0.f, 0.f, 0.f);
		m_collisionPos.Set(0.f, 0.f, 0.f);
	}

	/*! Read accessor for #m_normalizedDir2d. */
	KY_FORCE_INLINE Vec2f& GetNormalizedDir2d() { return m_normalizedDir2d; }

	/*! Read accessor for #m_normalizedDir2d. */
	KY_FORCE_INLINE const Vec2f& GetNormalizedDir2d() const { return m_normalizedDir2d; }

	/*! Read accessor for #m_collisionPos. */
	KY_FORCE_INLINE Vec3f& GetCollisionPos() { return m_collisionPos; }

	/*! Read accessor for #m_collisionPos. */
	KY_FORCE_INLINE const Vec3f& GetCollisionPos() const { return m_collisionPos; }

	/*! Read accessor for #m_arrivalPos. */
	KY_FORCE_INLINE Vec3f& GetArrivalPos() { return m_arrivalPos; }

	/*! Read accessor for #m_arrivalPos. */
	KY_FORCE_INLINE const Vec3f& GetArrivalPos() const { return m_arrivalPos; }

	/*! Read accessor for #m_startTriangleSafePtr. */
	KY_FORCE_INLINE NavTriangleSafePtr& GetStartTriangleSafePtr() { return m_startTriangleSafePtr; }

	/*! Read accessor for #m_startTriangleSafePtr. */
	KY_FORCE_INLINE const NavTriangleSafePtr& GetStartTriangleSafePtr() const { return m_startTriangleSafePtr; }

	/*! Read accessor for #m_arrivalTrianglePtr. */
	KY_FORCE_INLINE NavTrianglePtr& GetArrivalTrianglePtr() { return m_arrivalTrianglePtr; }

	/*! Read accessor for #m_arrivalTrianglePtr. */
	KY_FORCE_INLINE const NavTrianglePtr& GetArrivalTrianglePtr() const { return m_arrivalTrianglePtr; }

	/*! Read accessor for #m_intersectedHalfEdgePtr. */
	KY_FORCE_INLINE NavHalfEdgePtr& GetIntersectedHalfEdgePtr() { return m_intersectedHalfEdgePtr; }

	/*! Read accessor for #m_intersectedHalfEdgePtr. */
	KY_FORCE_INLINE const NavHalfEdgePtr& GetIntersectedHalfEdgePtr() const { return m_intersectedHalfEdgePtr; }

	/*! Read accessor for #m_safetyDist. */
	KY_FORCE_INLINE KyFloat32 GetSafetyDist() const { return m_safetyDist; }

	/*! Read accessor for #m_maxDist. */
	KY_FORCE_INLINE KyFloat32 GetMaxDist() const { return m_maxDist; }

	/*! Read accessor for #m_terrainTypeMask. */
	KY_FORCE_INLINE NavTerrainTypeMask GetTerrainTypeMask() const { return m_terrainTypeMask; }

	/*! Write accessor for #m_terrainTypeMask. */
	KY_FORCE_INLINE void SetTerrainTypeMask(NavTerrainTypeMask terrainTypeMask) { m_terrainTypeMask = terrainTypeMask; }

	/*! Read accessor for #m_normalizedDir2d. */
	KY_FORCE_INLINE KyFloat32 GetSearchRange() const { return m_searchRange; }

	/*! Write accessor for #m_normalizedDir2d. */
	KY_FORCE_INLINE void SetSearchRange(KyFloat32 searchRange) { m_searchRange = searchRange; }
public:
	//specific Inputs
	/*! The starting position of the ray. */
	Vec3f m_startPos;

	/*! A normalized vector that represents the direction in which the ray is cast. */
	Vec2f m_normalizedDir2d;

	/*! The NavMesh triangle that corresponds to #m_startPos. If you do not set this value, it will be computed automatically
		during the query. If you do set this value to a valid NavTriangleSafePtr, the value you provide will be used instead. */
	NavTriangleSafePtr m_startTriangleSafePtr;

	/*! A bitmask that specifies which terrain types should be considered navigable during the query. */
	NavTerrainTypeMask m_terrainTypeMask;

	// Generic inputs for castQueries
	/*! Determines the maximum distance the disk will travel. */
	KyFloat32 m_maxDist;

	/*! When a collision is detected, the collision point is moved back along #m_normalizedDir2d by this distance, in order
		to ensure that the final arrival position will lie within the valid area of the NavMesh.
		\units			meters
		\defaultvalue	\c 0.01*/
	KyFloat32 m_safetyDist;

	/*! The range of altitudes to use when determining #m_startTriangleSafePtr from the starting point
		you provide. */
	KyFloat32 m_searchRange;

	// Generic outputs for castQueries
	/*! Updated by the NavMeshQueryServer to indicate whether or not the ray was able to travel its maximum distance without
		hitting a NavMesh border or crossing a triangle with a forbidden terrain type. */
	bool m_reachMaxDist;

	/*! Updated by the NavMeshQueryServer to indicate the final position of the ray at the end of the cast.
		-	If #m_reachMaxDist is \c true, this is the triangle arrived at by traversing the NavMesh in the direction of #m_normalizedDir2d.
		-	If #m_reachMaxDist is \c false, this point is derived from #m_collisionPos by backtracking along the orientation
			of #m_normalizedDir2d a distance equal to #m_safetyDist. This tends to ensure that #m_arrivalPos will be inside the boundaries
			of the NavMesh. */
	Vec3f m_arrivalPos;

	/*! Updated by the NavMeshQueryServer to indicate the NavMesh triangle that corresponds to #m_arrivalPos. Due to the imprecision
		of floating-point calculations, this triangle may be invalid even after moving #m_arrivalPos back from the point of collision. */
	NavTrianglePtr m_arrivalTrianglePtr;

	/*! If #m_reachMaxDist is \c false, this indicates the position of the collision point between the disk and the NavMesh border. */
	Vec3f m_collisionPos;

	/*! If #m_reachMaxDist is \c false, this indicates the triangle edge that the ray collided with. */
	NavHalfEdgePtr m_intersectedHalfEdgePtr;

	/*! Updated by the NavMeshQueryServer to indicate whether or not the NavMesh triangle that corresponds to #m_arrivalPos is the same
		as #m_arrivalTrianglePtr. If this value is \c true, then a RayCanGoQuery carried out between #m_startPos and #m_endPos would have the
		same result as this RayCastQuery.
		\pp In some cases, the final triangle arrived at by traversing the NavMesh in the direction of #m_normalizedDir2d may not actually
		match the NavMesh triangle that corresponds to #m_arrivalPos. For example, if #m_startPos is at the bottom of a staircase, and
		the ray is cast towards the stairs, the raycast will climb the stairs as it propagates through the NavMesh, and will end up
		finding an #m_arrivalTrianglePtr on the second floor. However, the #m_arrivalPos value is calculated at the same altitude as 
		m_startPos, which would place it under the stairs in an area that may still be within the navigable area of the NavMesh, but whose
		triangle lies below the stairs. In this case, the RayCastQuery succeeds, but a RayCanGoQuery between the starting point and the 
		arrival position would fail due to the interposition of the stairs. */
	bool m_consistentResult;

	/*! Determines whether or not to store all NavMesh triangles crossed by the query, and provides access to the memory buffer that
		contains the stored triangles after the query. */
	PropagationQueryMemory m_memory;
};


}


#endif //KyPathData_RayCastQuery_H

