/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_SegmentCastQuery_H
#define KyPathData_SegmentCastQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/propagationquerymemory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of a SegmentCastQuery.
	\ingroup kypathdata_navmesh */
enum SegmentCastQueryResult
{
	SEGMENTCAST_MAXDIST_REACHED, /*!< Indicates that the segment was able to travel its maximum distance without collision. */
	SEGMENTCAST_START_OUTSIDE, /*!< Indicates that no NavMesh triangle could be found for the starting point. */
	SEGMENTCAST_START_MARTERIAL_FORBIDDEN, /*!< Indicates that the terrain type at the starting point was forbidden. */
	SEGMENTCAST_CANNOT_MOVE, /*!< Indicates that the starting point is very close to an obstacle. Either the segment does not fit in the NavMesh at its start position, or moving back the collision point resulted in an arrival position that is behind the starting point. */
	SEGMENTCAST_ARRIVAL_ERROR, /*!< Indicates that moving back the collision point resulted in an arrival position that is outside of the NavMesh. */
	SEGMENTCAST_ARIVALPOS_FOUND /*!< Indicates that a final arrival position was calculated successfully. */
};

/*!	The SegmentCastQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::SegmentCast().
	\pp This query casts a line segment with a specified half-width (#m_radius) on either side of a starting point (#m_startPos) in
	a specified orientation (#m_normalizedDir2d). If the segment collides with a border of the NavMesh or crosses any forbidden terrain types,
	the collision point is recorded (#m_collisionPos). The segment stops if it travels a maximum distance without colliding 
	(#m_maxDist). In either case, the final valid position is recorded in #m_arrivalPos.
	\pp The line segment is considered to be orthogonal to the direction between #m_startPos and #m_endPos, as shown below.
	<pre>
    m_startA    +---->-------->--------->---------->--------------+
                |                                                 | \
                |                                                 |  m_radius
                |                                                 | /
                |                                                 |
    m_startPos  X                                                 X  m_arrivalPos
                |                                                 |
                |                                                 | \
                |                                                 |  m_radius
                |                                                 | /
    m_startB    +---->-------->--------->---------->--------------+
	</pre>
	\ingroup kypathdata_navmesh */
class SegmentCastQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor
		\param maxDist				Sets the value of #m_maxDist. 
		\param safetyDist			Sets the value of #m_safetyDist. */
	SegmentCastQuery(KyFloat32 maxDist = 0.f, KyFloat32 safetyDist = 0.01f) :  m_radius(0.f), m_terrainTypeMask(NavTerrainTypeMask_All),
	 m_maxDist(maxDist), m_safetyDist(safetyDist), m_searchRange(1.f), m_reachMaxDist(false), m_consistentResult(false) {}

	/*! \label_constructor
		\param startPos				Sets the value of #m_startPos. 
		\param radius				Sets the value of #m_radius.
		\param normalizedDir2d		Sets the value of #m_normalizedDir2d.
		\param maxDist				Sets the value of #m_maxDist.
		\param safetyDist			Sets the value of #m_safetyDist. */
	SegmentCastQuery(const Vec3f& startPos, KyFloat32 radius, const Vec2f& normalizedDir2d, KyFloat32 maxDist, KyFloat32 safetyDist = 0.01f) : m_startPos(startPos),
	 m_normalizedDir2d(normalizedDir2d), m_radius(radius), m_terrainTypeMask(NavTerrainTypeMask_All), m_maxDist(maxDist), m_safetyDist(safetyDist), m_searchRange(1.f), m_reachMaxDist(false), m_consistentResult(false)
	{
		Vec2f ortho = m_normalizedDir2d.PerpCCW();
		m_startA = Vec3f(startPos.x + (ortho.x * radius), startPos.y + (ortho.y * radius), startPos.z);
		m_startB = Vec3f(startPos.x - (ortho.x * radius), startPos.y - (ortho.y * radius), startPos.z);
	}

	/*! \label_constructor
		\param startPos				Sets the value of #m_startPos. 
		\param radius				Sets the value of #m_radius.
		\param startTriangleSafePtr	Sets the value of #m_startTriangleSafePtr. 
		\param normalizedDir2d		Sets the value of #m_normalizedDir2d.
		\param maxDist				Sets the value of #m_maxDist.
		\param safetyDist			Sets the value of #m_safetyDist. */
	SegmentCastQuery(const Vec3f& startPos, KyFloat32 radius, NavTriangleSafePtr startTriangleSafePtr, const Vec2f& normalizedDir2d, KyFloat32 maxDist, KyFloat32 safetyDist = 0.01f) :
	 m_startPos(startPos), m_startTriangleSafePtr(startTriangleSafePtr), m_normalizedDir2d(normalizedDir2d), m_radius(radius), m_terrainTypeMask(NavTerrainTypeMask_All), m_maxDist(maxDist), m_safetyDist(safetyDist), m_searchRange(1.f), 
	 m_reachMaxDist(false), m_consistentResult(false)
	{
		Vec2f ortho = m_normalizedDir2d.PerpCCW();
		m_startA = Vec3f(startPos.x + (ortho.x * radius), startPos.y + (ortho.y * radius), startPos.z);
		m_startB = Vec3f(startPos.x - (ortho.x * radius), startPos.y - (ortho.y * radius), startPos.z);
	}

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::SegmentCast().
		\return An element from the #SegmentCastQueryResult enumeration that indicates the results of the query. */
	SegmentCastQueryResult GetResult() const
	{
		if (m_reachMaxDist)
			return SEGMENTCAST_MAXDIST_REACHED;

		if (!m_startTriangleSafePtr.IsValid())
			return SEGMENTCAST_START_OUTSIDE;

		if(!IsTerrainTypeCompliant(m_terrainTypeMask, m_startTriangleSafePtr.GetNavFloorTerrainType()))
			return SEGMENTCAST_START_MARTERIAL_FORBIDDEN;

		if (m_startPos == m_arrivalPos)
			return SEGMENTCAST_CANNOT_MOVE;

		if (m_arrivalTrianglePtr.IsValid())
			return SEGMENTCAST_ARIVALPOS_FOUND;

		return SEGMENTCAST_ARRIVAL_ERROR;
	}

	/*! \label_clear */
	void Clear()
	{
		m_radius = 0.f;
		m_maxDist = 0.f;
		m_safetyDist = 0.01f;

		m_startTriangleSafePtr.Invalidate();
		m_searchRange = 1.f;

		m_normalizedDir2d = Vec2f();
		m_startA.Set(0.f, 0.f, 0.f);
		m_startB.Set(0.f, 0.f, 0.f);

		CleanOutput();
	}

	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	void CleanOutput()
	{
		m_reachMaxDist = false;
		m_consistentResult = false;

		m_arrivalTrianglePtr.Invalidate();
		m_intersectedHalfEdgePtr.Invalidate();

		m_arrivalPos.Set(0.f, 0.f, 0.f);
		m_collisionPos.Set(0.f, 0.f, 0.f);
	}

	/*! Read accessor for #m_normalizedDir2d. */
	Vec2f GetNormalizedDir2d() const { return m_normalizedDir2d; }

	/*! Read accessor for #m_collisionPos. */
	Vec3f GetCollisionPos()    const { return m_collisionPos; }

	/*! Read accessor for #m_arrivalPos. */
	Vec3f GetArrivalPos()      const { return m_arrivalPos; }

	/*! Read accessor for #m_startTriangleSafePtr. */
	NavTriangleSafePtr GetStartTriangleSafePtr()       const { return m_startTriangleSafePtr; }

	/*! Read accessor for #m_arrivalTrianglePtr. */
	NavTrianglePtr GetArrivalTrianglePtr()     const { return m_arrivalTrianglePtr; }

	/*! Read accessor for #m_intersectedHalfEdgePtr. */
	NavHalfEdgePtr GetIntersectedHalfEdgePtr() const { return m_intersectedHalfEdgePtr; }

	/*! Read accessor for #m_safetyDist. */
	KyFloat32 GetSafetyDist() const { return m_safetyDist; }

	/*! Read accessor for #m_maxDist. */
	KyFloat32 GetMaxDist()    const { return m_maxDist; }

	/*! Read accessor for #m_radius. */
	KyFloat32 GetRadius() const { return m_radius; }

	/*! Read accessor for #m_terrainTypeMask. */
	KY_FORCE_INLINE NavTerrainTypeMask GetTerrainTypeMask() const { return m_terrainTypeMask; }

	/*! Write accessor for #m_terrainTypeMask. */
	KY_FORCE_INLINE void SetTerrainTypeMask(NavTerrainTypeMask terrainTypeMask) { m_terrainTypeMask = terrainTypeMask; }

	/*! Read accessor for #m_searchRange. */
	KY_FORCE_INLINE KyFloat32 GetSearchRange() const { return m_searchRange; }

	/*! Write accessor for #m_searchRange. */
	KY_FORCE_INLINE void SetSearchRange(KyFloat32 searchRange) { m_searchRange = searchRange; }
public:
	/*! The starting position of the segment. */
	Vec3f m_startPos;

	/*! The NavMesh triangle that corresponds to #m_startPos. If you do not set this value, it will be computed automatically
		during the query. If you do set this value to a valid NavTriangleSafePtr, the value you provide will be used instead. */
	NavTriangleSafePtr m_startTriangleSafePtr;

	/*! A normalized vector that represents the direction in which the segment is cast. */
	Vec2f m_normalizedDir2d;

	/*! The half-width of the segment. */
	KyFloat32 m_radius;

	/*! A bitmask that specifies which terrain types should be considered navigable during the query. */
	NavTerrainTypeMask m_terrainTypeMask;

	/*! The point at the other end of the line segment from #m_startB. Computed transparently from #m_startPos and #m_radius. */
	Vec3f m_startA;

	/*! The point at the other end of the line segment from #m_startA. Computed transparently from #m_startPos and #m_radius. */
	Vec3f m_startB;

	// Generic inputs for castQueries
	/*! Determines the maximum distance the segment will travel. */
	KyFloat32 m_maxDist;

	/*! When a collision is detected, the collision point is moved back along #m_normalizedDir2d by this distance, in order
		to ensure that the final arrival position will lie within the valid area of the NavMesh. */
	KyFloat32 m_safetyDist;

	/*! The range of altitudes to use when determining #m_startTriangleSafePtr from the starting point
		you provide. */
	KyFloat32 m_searchRange;

	// Generic outputs for castQueries
	/*! Updated by the NavMeshQueryServer to indicate whether or not the segment was able to travel its maximum distance without
		hitting a NavMesh border or crossing a triangle with a forbidden terrain type. */
	bool m_reachMaxDist;

	/*! Updated by the NavMeshQueryServer to indicate the final position of the segment at the end of the cast.
		-	If #m_reachMaxDist is \c true, this is the point that lies at a distance of #m_maxDist from the starting point
			in the direction of #m_normalizedDir2d.
		-	If #m_reachMaxDist is \c false, this point is derived from #m_collisionPos by backtracking along the orientation
			of #m_normalizedDir2d a distance equal to #m_safetyDist. This tends to ensure that #m_arrivalPos will be inside the boundaries
			of the NavMesh. */
	Vec3f m_arrivalPos;

	/*! Updated by the NavMeshQueryServer to indicate the NavMesh triangle that corresponds to #m_arrivalPos. Due to the imprecision
		of floating-point calculations, this triangle may be invalid even after moving #m_arrivalPos back from the point of collision. */
	NavTrianglePtr m_arrivalTrianglePtr;

	/*! Updated by the NavMeshQueryServer to indicate the position of the collision point between the segment and the NavMesh border,
		if #m_reachMaxDist is \c false. */
	Vec3f m_collisionPos;

	/*! Updated by the NavMeshQueryServer to indicate the triangle edge that the segment collided with, if #m_reachMaxDist is \c false. */
	NavHalfEdgePtr m_intersectedHalfEdgePtr;

	/*! Not used by this query. */
	bool m_consistentResult;
	
	/*! Determines whether or not to store all NavMesh triangles crossed by the query, and provides access to the memory buffer that
		contains the stored triangles after the query. */
	PropagationQueryMemory m_memory;
};


}


#endif //KyPathData_SegmentCastQuery_H

