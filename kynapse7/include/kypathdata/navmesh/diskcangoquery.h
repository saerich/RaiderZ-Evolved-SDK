/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_DiskCanGoQuery_H
#define KyPathData_DiskCanGoQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/propagationquerymemory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of a DiskCanGoQuery.
	\ingroup kypathdata_navmesh */
enum DiskCanGoQueryResult
{
	DISKCANGO_SUCCESS, /*!< Indicates that the disk can pass in a straight line from the starting point to the ending point without a collision. */
	DISKCANGO_START_OUTSIDE, /*!< Indicates that no NavMesh triangle could be found for the starting point. */
	DISKCANGO_START_MARTERIAL_FORBIDDEN, /*!< Indicates that the terrain type at the starting point was forbidden. */
	DISKCANGO_DEST_OUTSIDE, /*!< Indicates that no NavMesh triangle could be found for the ending point. */
	DISKCANGO_DEST_MARTERIAL_FORBIDDEN, /*!< Indicates that the terrain type at the ending point was forbidden. */
	DISKCANGO_COLLISION, /*!< Indicates that a collision was detected along the straight line between the starting and ending points. */
	DISKCANGO_WRONG_FLOOR /*!< Indicates that none of the above results occurred, which may indicate that the start and end points lie on different floors. */
};

/*!	The DiskCanGoQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::DiskCanGo().
	\pp This query tests whether or not a disk with a specified radius (#m_radius) can move along a straight line segment from a
	starting point (#m_startPos) to an ending point (#m_destPos) without colliding with the border of the NavMesh or crossing any
	forbidden terrain types.
	\section closetomesh Tests close to the NavMesh boundary
	The actual extents of the disk used in the test are first computed by doing two raycast queries against the NavMesh at the
	starting position, in both directions orthogonally to the direction from #m_startPos to #m_destPos. Each raycast has a length
	equal to #m_radius.
	\pp In cases where the center of the disk lies in an open area of the NavMesh, this results in a disk with a radius equal to #m_radius.
	However, when the center of the disk lies near a boundary of the NavMesh, the raycast queries that determine the extents of the
	disk may encounter the NavMesh boundary before they travel the full #m_radius distance. If this occurs, the position of the collision
	is moved back very slightly from the NavMesh boundary, and that resulting position is used as the extent of the disk. This results in
	a disk that is smaller than the one implied by #m_radius, but whose extents are guaranteed to be fully inside the NavMesh at its
	starting point.
	\pp Because the actual disk used in the query may be smaller than you expect, the test may miss obstacles that intrude only partially
	into the path of the expected disk.
	\ingroup kypathdata_navmesh */
class DiskCanGoQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	DiskCanGoQuery() : m_radius(0.f), m_searchRange(1.f), m_canGo(false), m_collision(false) {}

	/*! \label_constructor
		\param startPos				Sets the value of #m_startPos.
		\param destPos				Sets the value of #m_destPos.
		\param radius				Sets the value of #m_radius. */
	DiskCanGoQuery(const Vec3f& startPos, const Vec3f& destPos, KyFloat32 radius) : m_startPos(startPos), m_destPos(destPos),
	 m_terrainTypeMask(NavTerrainTypeMask_All), m_radius(radius), m_searchRange(1.f), m_adaptQueryToStartPosition(true), m_canGo(false), m_collision(false) {}

	/*! \label_constructor
		\param startPos				Sets the value of #m_startPos.
		\param startTriangleSafePtr	Sets the value of #m_startTriangleSafePtr.
		\param destPos				Sets the value of #m_destPos.
		\param radius				Sets the value of #m_radius. */
	DiskCanGoQuery(const Vec3f& startPos, NavTriangleSafePtr startTriangleSafePtr, const Vec3f& destPos, KyFloat32 radius) : m_startPos(startPos),
	 m_startTriangleSafePtr(startTriangleSafePtr), m_destPos(destPos), m_terrainTypeMask(NavTerrainTypeMask_All), m_radius(radius), m_searchRange(1.f), m_adaptQueryToStartPosition(true), m_canGo(false), m_collision(false) {}

	/*! \label_constructor
		\param startPos				Sets the value of #m_startPos.
		\param startTriangleSafePtr	Sets the value of #m_startTriangleSafePtr.
		\param destPos				Sets the value of #m_destPos.
		\param destTriangleSafePtr	Sets the value of #m_destTriangleSafePtr.
		\param radius				Sets the value of #m_radius. */
	DiskCanGoQuery(const Vec3f& startPos, NavTriangleSafePtr startTriangleSafePtr, const Vec3f& destPos, NavTriangleSafePtr destTriangleSafePtr, KyFloat32 radius) : m_startPos(startPos),
	 m_startTriangleSafePtr(startTriangleSafePtr), m_destPos(destPos), m_destTriangleSafePtr(destTriangleSafePtr), m_terrainTypeMask(NavTerrainTypeMask_All), m_radius(radius), m_searchRange(1.f), m_adaptQueryToStartPosition(true),
	 m_canGo(false), m_collision(false) {}

	/*! \label_constructor
		\param startPos				Sets the value of #m_startPos.
		\param destPos				Sets the value of #m_destPos.
		\param destTriangleSafePtr	Sets the value of #m_destTriangleSafePtr.
		\param radius				Sets the value of #m_radius. */
	DiskCanGoQuery(const Vec3f& startPos, const Vec3f& destPos, NavTriangleSafePtr destTriangleSafePtr, KyFloat32 radius) : m_startPos(startPos), m_destPos(destPos),
	 m_destTriangleSafePtr(destTriangleSafePtr), m_terrainTypeMask(NavTerrainTypeMask_All), m_radius(radius), m_searchRange(1.f), m_adaptQueryToStartPosition(true), m_canGo(false), m_collision(false) {}

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::DiskCanGo().
		\return An element from the #DiskCanGoQueryResult enumeration that indicates the results of the query. */
	DiskCanGoQueryResult GetResult() const
	{
		if (m_canGo)
			return DISKCANGO_SUCCESS;

		if (!m_startTriangleSafePtr.IsValid())
			return DISKCANGO_START_OUTSIDE;

		if(!IsTerrainTypeCompliant(m_terrainTypeMask, m_startTriangleSafePtr.GetNavFloorTerrainType()))
			return DISKCANGO_START_MARTERIAL_FORBIDDEN;

		if (!m_destTriangleSafePtr.IsValid())
			return DISKCANGO_DEST_OUTSIDE;

		if(!IsTerrainTypeCompliant(m_terrainTypeMask, m_destTriangleSafePtr.GetNavFloorTerrainType()))
			return DISKCANGO_DEST_MARTERIAL_FORBIDDEN;

		if (m_collision)
			return DISKCANGO_COLLISION;

		return DISKCANGO_WRONG_FLOOR;
	}

	/*! \label_clear */
	KY_FORCE_INLINE void Clear()
	{
		m_radius = 0.f;

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

	/*! Read accessor for #m_radius. */
	KY_FORCE_INLINE KyFloat32 GetRadius() const { return m_radius; }

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

/*! The radius of the disk. */
	KyFloat32 m_radius;

	/*! The range of altitudes to use when determining #m_startTriangleSafePtr and #m_destTriangleSafePtr from the starting and ending points
		you provide. */
	KyFloat32 m_searchRange;

	/*! If this value is set to #KY_TRUE, the query will not fail if the starting point lies inside the NavMesh but the radius
		collides with the border of the NavMesh. */
	bool m_adaptQueryToStartPosition;

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


#endif //KyPathData_DiskCanGoQuery_H


