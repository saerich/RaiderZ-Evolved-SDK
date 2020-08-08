/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_DiskExpansionQuery_H
#define KyPathData_DiskExpansionQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/propagationquerymemory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of a DiskExpansionQuery.
	\ingroup kypathdata_navmesh */
enum DiskExpansionQueryResult
{
	DISKEXPANSION_FIT, /*!< Indicates that the disk fits inside the navigable areas of the NavMesh even at maximum size. */
	DISKEXPANSION_CENTER_OUTSIDE, /*!< Indicates that the center point lies outside the navigable area of the NavMesh. */
	DISKEXPANSION_CENTER_MARTERIAL_FORBIDDEN, /*!< Indicates that the terrain type at the center point was forbidden. */
	DISKEXPANSION_CANNOT_EXPAND, /*!< Indicates that the center point lies within the NavMesh, but the disk could not be expanded. */
	DISKEXPANSION_COLLISION_FOUND /*!< Indicates that a collision was detected when expanding the disk before the maximum size of the disk was reached. */
};

/*!	The DiskExpansionQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::DiskExpansion().
	\pp This query attempts to place the center of a disk at a specified position (#m_center), and expands the radius of the disk
	until the disk collides with a NavMesh border, intersects a forbidden terrain type, or reaches a maximum size (#m_radiusMax).
	\ingroup kypathdata_navmesh */
class DiskExpansionQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor
		\param safetyDist				Sets the value of #m_safetyDist. */
	DiskExpansionQuery(KyFloat32 safetyDist = 0.01f) : 
	 m_radiusMax(0.f), m_terrainTypeMask(NavTerrainTypeMask_All), m_safetyDist(safetyDist), m_searchRange(1.f), m_collision(true), m_resultRadius(0.f) {}

	/*! \label_constructor
		\param center					Sets the value of #m_center.
		\param radius					Sets the value of #m_radiusMax. 
		\param safetyDist				Sets the value of #m_safetyDist. */
	DiskExpansionQuery(const Vec3f& center, KyFloat32 radius, KyFloat32 safetyDist = 0.01f) :
	 m_center(center), m_radiusMax(radius), m_terrainTypeMask(NavTerrainTypeMask_All), m_safetyDist(safetyDist), m_searchRange(1.f), m_collision(true), m_resultRadius(0.f) {}

	/*! \label_constructor
		\param center					Sets the value of #m_center.
		\param centerTriangleSafePtr	Sets the value of #m_centerTriangleSafePtr.
		\param radius					Sets the value of #m_radiusMax. 
		\param safetyDist				Sets the value of #m_safetyDist. */
	DiskExpansionQuery(const Vec3f& center, KyFloat32 radius, NavTriangleSafePtr centerTriangleSafePtr, KyFloat32 safetyDist = 0.01f) :
	 m_center(center),  m_centerTriangleSafePtr(centerTriangleSafePtr), m_radiusMax(radius), m_terrainTypeMask(NavTerrainTypeMask_All), m_safetyDist(safetyDist), m_searchRange(1.f),
	 m_collision(true), m_resultRadius(0.f) {}

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::DiskExpansion().
		\return An element from the #DiskExpansionQueryResult enumeration that indicates the results of the query. */
	DiskExpansionQueryResult GetResult() const
	{
		if (!m_centerTriangleSafePtr.IsValid())
			return DISKEXPANSION_CENTER_OUTSIDE;

		if(!IsTerrainTypeCompliant(m_terrainTypeMask, m_centerTriangleSafePtr.GetNavFloorTerrainType()))
			return DISKEXPANSION_CENTER_MARTERIAL_FORBIDDEN;

		if (!m_collision)
			return DISKEXPANSION_FIT;

		if (m_resultRadius == 0.f)
			return DISKEXPANSION_CANNOT_EXPAND;

		return DISKEXPANSION_COLLISION_FOUND;
	}

	/*! \label_clear */
	KY_FORCE_INLINE void Clear()
	{
		m_radiusMax = 0.f;
		m_safetyDist = 0.f;
		m_resultRadius = 0.f;
		m_terrainTypeMask = NavTerrainTypeMask_All;
		m_searchRange = 1.f;

		m_center.Set(0.f, 0.f, 0.f);

		m_centerTriangleSafePtr.Invalidate();

		CleanOutput();
	}

	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	KY_FORCE_INLINE void CleanOutput()
	{
		m_collision = true;
		m_collisionPos.Set(0.f, 0.f, 0.f);
		m_intersectedHalfEdgePtr.Invalidate();
	}

	/*! Read accessor for #m_terrainTypeMask. */
	KY_FORCE_INLINE NavTerrainTypeMask GetTerrainTypeMask() const { return m_terrainTypeMask; }

	/*! Write accessor for #m_terrainTypeMask. */
	KY_FORCE_INLINE void SetTerrainTypeMask(NavTerrainTypeMask terrainTypeMask) { m_terrainTypeMask = terrainTypeMask; }

	/*! Read accessor for #m_searchRange. */
	KY_FORCE_INLINE KyFloat32 GetSearchRange() const { return m_searchRange; }

	/*! Write accessor for #m_searchRange. */
	KY_FORCE_INLINE void SetSearchRange(KyFloat32 searchRange) { m_searchRange = searchRange; }
public:
	/*! The position of the center of the disk. */
	Vec3f m_center;

	/*! The NavMesh triangle that corresponds to #m_center. If you do not set this value, it will be computed automatically
		during the query. If you do set this value to a valid NavTriangleSafePtr, the value you provide will be used instead. */
	NavTriangleSafePtr m_centerTriangleSafePtr;

	/*! The maximum radius for the disk. If the disk expands to this radius without a collision, the query is stopped. */
	KyFloat32 m_radiusMax;

	/*! A bitmask that specifies which terrain types should be considered navigable during the query. */
	NavTerrainTypeMask m_terrainTypeMask;

	/*! When a collision is detected, the radius is decreased this distance, in order
		to ensure that the final disk will lie completely within the valid area of the NavMesh.
		\units			meters
		\defaultvalue	\c 0.01*/
	KyFloat32 m_safetyDist;

	/*! The range of altitudes to use when determining #m_centerSafePtr from the starting and ending points
		you provide. */
	KyFloat32 m_searchRange;

	/*! Updated by the NavMeshQueryServer to indicate whether or not a collision was detected during the query. */
	bool m_collision;

	/*! Updated by the NavMeshQueryServer to indicate the position of the collision on the NavMesh border,
		if #m_collision is \c true. */
	Vec3f m_collisionPos;

	/*! Updated by the NavMeshQueryServer to indicate the triangle edge that the disk collided with,
		if #m_collision is \c true. */
	NavHalfEdgePtr m_intersectedHalfEdgePtr;

	/*! Updated by the NavMeshQueryServer to indicate the final result of the disk at the end of the query.
		-	If #m_collision is \c false, this value will match #m_maxRadius.
		-	If #m_collision is \c true, this value will be the largest valid radius that will fit in the available space. */
	KyFloat32 m_resultRadius;

	/*! Determines whether or not to store all NavMesh triangles crossed by the query, and provides access to the memory buffer that
		contains the stored triangles after the query. */
	PropagationQueryMemory m_memory;
};


}


#endif //KyPathData_DiskCollisionQuery_H

