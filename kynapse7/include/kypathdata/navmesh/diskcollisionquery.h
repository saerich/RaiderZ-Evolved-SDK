/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_DiskCollisionQuery_H
#define KyPathData_DiskCollisionQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/propagationquerymemory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of a DiskCollisionQuery.
	\ingroup kypathdata_navmesh */
enum DiskCollisionQueryResult
{
	DISKCOLLISION_FIT, /*!< Indicates that the disk fits inside the navigable area of the NavMesh. */
	DISKCOLLISION_CENTER_OUTSIDE, /*!< Indicates that the center point lies outside the navigable area of the NavMesh. */
	DISKCOLLISION_CENTER_MARTERIAL_FORBIDDEN, /*!< Indicates that the terrain type at the center point was forbidden. */
	DISKCOLLISION_DOES_NOT_FIT /*!< Indicates that the center point lies inside the NavMesh, but that the radius collides with a border or crosses a forbidden terrain. */
};

/*!	The DiskCollisionQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::DiskCollision().
	\pp This query attempts to place the center of a disk with a specified radius (#m_radius) at a specified position 
	(#m_center), and tests whether the entire disk fits within the borders of the NavMesh, without intersecting any forbidden
	terrain types.
	\ingroup kypathdata_navmesh */
class DiskCollisionQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	DiskCollisionQuery() : m_radius(0.f), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_collision(true) {}

	/*! \label_constructor
		\param center				Sets the value of #m_center. 
		\param radius				Sets the value of #m_radius. */
	DiskCollisionQuery(const Vec3f& center, KyFloat32 radius) : 
	 m_center(center), m_radius(radius), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_collision(true) {}

	/*! \label_constructor
		\param center					Sets the value of #m_center. 
		\param radius					Sets the value of #m_radius.
		\param centerTriangleSafePtr	Sets the value of #m_centerTriangleSafePtr. */
	DiskCollisionQuery(const Vec3f& center, KyFloat32 radius, NavTriangleSafePtr centerTriangleSafePtr) : 
	 m_center(center), m_centerTriangleSafePtr(centerTriangleSafePtr), m_radius(radius), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_collision(true) {}

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::DiskCollision().
		\return An element from the #DiskCollisionQueryResult enumeration that indicates the results of the query. */
	KY_FORCE_INLINE DiskCollisionQueryResult GetResult() const
	{
		if (!m_centerTriangleSafePtr.IsValid())
			return DISKCOLLISION_CENTER_OUTSIDE;

		if(!IsTerrainTypeCompliant(m_terrainTypeMask, m_centerTriangleSafePtr.GetNavFloorTerrainType()))
			return DISKCOLLISION_CENTER_MARTERIAL_FORBIDDEN;

		if (m_collision)
			return DISKCOLLISION_DOES_NOT_FIT;

		return DISKCOLLISION_FIT;
	}

	/*! \label_clear */
	KY_FORCE_INLINE void Clear()
	{
		m_radius = 0.f;
		m_center.Set(0.f, 0.f, 0.f);
		m_centerTriangleSafePtr.Invalidate();
		m_terrainTypeMask = NavTerrainTypeMask_All;
		m_searchRange = 1.f;

		CleanOutput();
	}

	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	KY_FORCE_INLINE void CleanOutput() { m_collision = true; }

	/*! Read accessor for #m_center. */
	KY_FORCE_INLINE Vec3f& GetCenter() { return m_center; }

	/*! Write accessor for #m_center. */
	KY_FORCE_INLINE const Vec3f& GetCenter() const { return m_center; }

	/*! Read accessor for #m_centerTriangleSafePtr. */
	KY_FORCE_INLINE NavTriangleSafePtr& GetCenterTriangleSafePtr() { return m_centerTriangleSafePtr; }

	/*! Write accessor for #m_centerTriangleSafePtr. */
	KY_FORCE_INLINE const NavTriangleSafePtr& GetCenterTriangleSafePtr() const { return m_centerTriangleSafePtr; }

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
	/*! The position of the center of the disk. */
	Vec3f m_center;

	/*! The NavMesh triangle that corresponds to #m_center. If you do not set this value, it will be computed automatically
		during the query. If you do set this value to a valid NavTriangleSafePtr, the value you provide will be used instead. */
	NavTriangleSafePtr m_centerTriangleSafePtr;

	/*! The radius of the disk. */
	KyFloat32 m_radius;

	/*! A bitmask that specifies which terrain types should be considered navigable during the query. */
	NavTerrainTypeMask m_terrainTypeMask;

	/*! The range of altitudes to use when determining #m_centerTriangleSafePtr from the center point
		you provide. */
	KyFloat32 m_searchRange;

	/*! Updated by the NavMeshQueryServer to indicate whether or not the disk collides with a border of the NavMesh or a forbidden terrain type. */
	bool m_collision;

	/*! Determines whether or not to store all NavMesh triangles crossed by the query, and provides access to the memory buffer that
		contains the stored triangles after the query. */
	PropagationQueryMemory m_memory;
};


}


#endif //KyPathData_DiskCollisionQuery_H

