/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_RayCanGoQuery_H
#define KyPathData_RayCanGoQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/propagationquerymemory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of a RayCanGoQuery.
	\ingroup kypathdata_navmesh */
enum RayCanGoQueryResult
{
	RAYCANGO_SUCCESS, /*!< Indicates that the ray can pass in a straight line from the starting point to the ending point without a collision. */
	RAYCANGO_START_OUTSIDE, /*!< Indicates that no NavMesh triangle could be found for the starting point. */
	RAYCANGO_START_MARTERIAL_FORBIDDEN,
	RAYCANGO_DEST_OUTSIDE, /*!< Indicates that no NavMesh triangle could be found for the ending point. */
	RAYCANGO_DEST_MARTERIAL_FORBIDDEN,
	RAYCANGO_COLLISION, /*!< Indicates that a collision was detected along the straight line between the starting and ending points. */
	RAYCANGO_WRONG_FLOOR /*!< Indicates that none of the above results occurred, which may indicate that the start and end points lie on different floors. */
};

/*!	The RayCanGoQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::RayCanGo(). 
	\pp This query tests whether or not a ray can pass along a straight line segment from a starting point (#m_startPos)
	to an ending point (#m_endPos) without colliding with the border of the NavMesh or crossing any forbidden terrain types.
	\ingroup kypathdata_navmesh */
class RayCanGoQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	RayCanGoQuery() : m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_canGo(false), m_collision(false) {}

	/*! \label_constructor 
		\param startPos				Sets the value of #m_startPos.
		\param destPos				Sets the value of #m_destPos. */
	RayCanGoQuery(const Vec3f& startPos, const Vec3f& destPos) :
	 m_startPos(startPos), m_destPos(destPos), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_canGo(false), m_collision(false) {}

	/*! \label_constructor 
		\param startPos				Sets the value of #m_startPos.
		\param startTriangleSafePtr	Sets the value of #m_startTriangleSafePtr.
		\param destPos				Sets the value of #m_destPos. */
	RayCanGoQuery(const Vec3f& startPos, NavTriangleSafePtr startTriangleSafePtr, const Vec3f& destPos) :
	 m_startPos(startPos), m_startTriangleSafePtr(startTriangleSafePtr), m_destPos(destPos), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_canGo(false), m_collision(false) {}

	/*! \label_constructor 
		\param startPos				Sets the value of #m_startPos.
		\param startTriangleSafePtr	Sets the value of #m_startTriangleSafePtr.
		\param destPos				Sets the value of #m_destPos.
		\param destTriangleSafePtr	Sets the value of #m_destTriangleSafePtr. */
	RayCanGoQuery(const Vec3f& startPos, NavTriangleSafePtr startTriangleSafePtr, const Vec3f& destPos, NavTriangleSafePtr destTriangleSafePtr) : m_startPos(startPos),
	 m_startTriangleSafePtr(startTriangleSafePtr),  m_destPos(destPos), m_destTriangleSafePtr(destTriangleSafePtr), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_canGo(false), m_collision(false) {}

	/*! \label_constructor 
		\param startPos				Sets the value of #m_startPos.
		\param destPos				Sets the value of #m_destPos.
		\param destTriangleSafePtr	Sets the value of #m_destTriangleSafePtr. */
	RayCanGoQuery(const Vec3f& startPos, const Vec3f& destPos, NavTriangleSafePtr destTriangleSafePtr) : m_startPos(startPos), m_destPos(destPos),
	 m_destTriangleSafePtr(destTriangleSafePtr), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_canGo(false), m_collision(false) {}

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::RayCanGo().
		\return An element from the #RayCanGoQueryResult enumeration that indicates the results of the query. */
	RayCanGoQueryResult GetResult() const
	{
		if (m_canGo) return RAYCANGO_SUCCESS;

		if (!m_startTriangleSafePtr.IsValid())
			return RAYCANGO_START_OUTSIDE;

		if(!IsTerrainTypeCompliant(m_terrainTypeMask, m_startTriangleSafePtr.GetNavFloorTerrainType()))
			return RAYCANGO_START_MARTERIAL_FORBIDDEN;

		if (!m_destTriangleSafePtr.IsValid())
			return RAYCANGO_DEST_OUTSIDE;
		
		if(!IsTerrainTypeCompliant(m_terrainTypeMask, m_destTriangleSafePtr.GetNavFloorTerrainType()))
			return RAYCANGO_DEST_MARTERIAL_FORBIDDEN;

		if (m_collision)
			return RAYCANGO_COLLISION;

		return RAYCANGO_WRONG_FLOOR;
	}

	/*! \label_clear */
	KY_FORCE_INLINE void Clear()
	{
		m_startPos.Set(0.f, 0.f, 0.f);
		m_destPos.Set(0.f, 0.f, 0.f);

		m_startTriangleSafePtr.Invalidate();
		m_destTriangleSafePtr.Invalidate();

		m_terrainTypeMask = NavTerrainTypeMask_All;
		m_searchRange = 1.f;

		CleanOutput();
	}

	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	KY_FORCE_INLINE void CleanOutput()
	{
		m_canGo = false;
		m_collision = true;
	}

	/*! Read accessor for #m_startPos. */
	KY_FORCE_INLINE Vec3f& GetStartPos() { return m_startPos; }

	/*! Write accessor for #m_startPos. */
	KY_FORCE_INLINE const Vec3f& GetStartPos() const { return m_startPos; }

	/*! Read accessor for #m_destPos. */
	KY_FORCE_INLINE Vec3f& GetDestPos() { return m_destPos; }

	/*! Write accessor for #m_destPos. */
	KY_FORCE_INLINE const Vec3f& GetDestPos() const { return m_destPos; }

	/*! Read accessor for #m_startTriangleSafePtr. */
	KY_FORCE_INLINE NavTriangleSafePtr& GetStartTriangleSafePtr() { return m_startTriangleSafePtr; }

	/*! Write accessor for #m_startTriangleSafePtr. */
	KY_FORCE_INLINE const NavTriangleSafePtr& GetStartTriangleSafePtr() const { return m_startTriangleSafePtr; }

	/*! Read accessor for #m_destTriangleSafePtr. */
	KY_FORCE_INLINE NavTriangleSafePtr& GetDestTriangleSafePtr() { return m_destTriangleSafePtr; }

	/*! Write accessor for #m_destTriangleSafePtr. */
	KY_FORCE_INLINE const NavTriangleSafePtr& GetDestTriangleSafePtr() const { return m_destTriangleSafePtr; }

	/*! Read accessor for #m_terrainTypeMask. */
	KY_FORCE_INLINE NavTerrainTypeMask GetTerrainTypeMask() const { return m_terrainTypeMask; }

	/*! Write accessor for #m_terrainTypeMask. */
	KY_FORCE_INLINE void SetTerrainTypeMask(NavTerrainTypeMask terrainTypeMask) { m_terrainTypeMask = terrainTypeMask; }

	/*! Read accessor for #m_searchRange. */
	KY_FORCE_INLINE KyFloat32 GetSearchRange() const { return m_searchRange; }

	/*! Write accessor for #m_searchRange. */
	KY_FORCE_INLINE void SetSearchRange(KyFloat32 searchRange) { m_searchRange = searchRange; }

public:
	/*! The starting point of the line segment to be tested. */
	Vec3f m_startPos;

	/*! The NavMesh triangle that corresponds to #m_startPos. If you do not set this value, it will be computed automatically
		during the query. If you do set this value to a valid NavTriangleSafePtr, the value you provide will be used instead. */
	NavTriangleSafePtr m_startTriangleSafePtr;

	/*! The ending point of the line segment to be tested. */
	Vec3f m_destPos;

	/*! The NavMesh triangle that corresponds to #m_destPos. If you do not set this value, it will be computed automatically
		during the query. If you do set this value to a valid NavTriangleSafePtr, the value you provide will be used instead. */
	NavTriangleSafePtr m_destTriangleSafePtr;

	/*! A bitmask that specifies which terrain types should be considered navigable during the query. */
	NavTerrainTypeMask m_terrainTypeMask;

	/*! The range of altitudes to use when determining #m_startTriangleSafePtr and #m_destTriangleSafePtr from the starting and ending points
		you provide. */
	KyFloat32 m_searchRange;

	/*! Updated by the NavMeshQueryServer. If \c true, the query succeeded, and the disk can go in the straight line from 
		#m_startPos to #m_destPos. */
	bool m_canGo;

	/*! Updated by the NavMeshQueryServer. If #m_canGo is \c false and #m_collision is \c true, the disk was blocked by a 
		border of the NavMesh or a triangle with a non-walkable terrain type. */
	bool m_collision;

	/*! Determines whether or not to store all NavMesh triangles crossed by the query, and provides access to the memory buffer
		that contains the stored triangles after the query. */
	PropagationQueryMemory m_memory;
};

}


#endif //KyPathData_RayCanGoQuery_H

