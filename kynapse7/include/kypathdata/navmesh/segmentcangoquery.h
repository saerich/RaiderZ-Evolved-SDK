/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_SegmentCanGoQuery_H
#define KyPathData_SegmentCanGoQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/propagationquerymemory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of a SegmentCanGoQuery.
	\ingroup kypathdata_navmesh */
enum SegmentCanGoQueryResult
{
	SEGMENTCANGO_SUCCESS, /*!< Indicates that the segment can pass in a straight line from its starting point to the ending point without a collision. */
	SEGMENTCANGO_START_OUTSIDE, /*!< Indicates that no NavMesh triangle could be found for the starting point. */
	SEGMENTCANGO_START_MARTERIAL_FORBIDDEN, /*!< Indicates that the terrain type at the starting point was forbidden. */
	SEGMENTCANGO_DEST_OUTSIDE, /*!< Indicates that no NavMesh triangle could be found for the ending point. */
	SEGMENTCANGO_DEST_MARTERIAL_FORBIDDEN, /*!< Indicates that the terrain type at the destination point was forbidden. */
	SEGMENTCANGO_COLLISION, /*!< Indicates that a collision was detected along the straight line between the starting and ending points. */
	SEGMENTCANGO_WRONG_FLOOR /*!< Indicates that none of the above results occurred, which may indicate that the start and end points lie on different floors. */
};

/*!	The SegmentCanGoQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::SegmentCanGo(). 
	\pp This query tests whether or not a line segment with a specified half-width (#m_radius) can move along a straight line segment from a 
	starting point (#m_startPos) to an ending point (#m_destPos) without colliding with the border of the NavMesh or crossing any
	forbidden terrain types. The line segment is considered to be orthogonal to the direction between #m_startPos and #m_destPos, as shown below.
	<pre>
    m_startA    +---->-------->--------->---------->--------------+
                |                                                 | \
                |                                                 |  m_radius
                |                                                 | /
                |                                                 |
    m_startPos  X                                                 X  m_endPos
                |                                                 |
                |                                                 | \
                |                                                 |  m_radius
                |                                                 | /
    m_startB    +---->-------->--------->---------->--------------+
	</pre>
	\ingroup kypathdata_navmesh */
class SegmentCanGoQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	SegmentCanGoQuery() :  m_radius(0.f), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_canGo(false), m_collision(false) {}

	/*! \label_constructor 
		\param startPos				Sets the value of #m_startPos.
		\param destPos				Sets the value of #m_destPos.
		\param radius				Sets the value of #m_radius. */
	SegmentCanGoQuery(const Vec3f& startPos, const Vec3f& destPos, KyFloat32 radius) :
	 m_startPos(startPos), m_destPos(destPos), m_radius(radius), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_canGo(false), m_collision(false)
	{
		InitSegmentExremity(startPos, destPos);
	}

	/*! \label_constructor 
		\param startPos				Sets the value of #m_startPos.
		\param startTriangleSafePtr	Sets the value of #m_startTriangleSafePtr.
		\param destPos				Sets the value of #m_destPos.
		\param radius				Sets the value of #m_radius. */
	SegmentCanGoQuery(const Vec3f& startPos, NavTriangleSafePtr startTriangleSafePtr, const Vec3f& destPos, KyFloat32 radius) :
	 m_startPos(startPos), m_startTriangleSafePtr(startTriangleSafePtr), m_destPos(destPos), m_radius(radius), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_canGo(false), m_collision(false)
	{
		InitSegmentExremity(startPos, destPos);
	}

	/*! \label_constructor 
		\param startPos				Sets the value of #m_startPos.
		\param startTriangleSafePtr	Sets the value of #m_startTriangleSafePtr.
		\param destPos				Sets the value of #m_destPos.
		\param destTriangleSafePtr	Sets the value of #m_destTriangleSafePtr.
		\param radius				Sets the value of #m_radius. */
	SegmentCanGoQuery(const Vec3f& startPos, NavTriangleSafePtr startTriangleSafePtr, const Vec3f& destPos, NavTriangleSafePtr destTriangleSafePtr, KyFloat32 radius) :
	 m_startPos(startPos), m_startTriangleSafePtr(startTriangleSafePtr), m_destPos(destPos), m_destTriangleSafePtr(destTriangleSafePtr), m_radius(radius), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f),
	 m_canGo(false), m_collision(false)
	{
		InitSegmentExremity(startPos, destPos);
	}

	/*! \label_constructor 
		\param startPos				Sets the value of #m_startPos.
		\param destPos				Sets the value of #m_destPos.
		\param destTriangleSafePtr	Sets the value of #m_destTriangleSafePtr.
		\param radius				Sets the value of #m_radius. */
	SegmentCanGoQuery(const Vec3f& startPos, const Vec3f& destPos, NavTriangleSafePtr destTriangleSafePtr, KyFloat32 radius) :
	 m_startPos(startPos), m_destPos(destPos), m_destTriangleSafePtr(destTriangleSafePtr), m_radius(radius), m_terrainTypeMask(NavTerrainTypeMask_All), m_searchRange(1.f), m_canGo(false), m_collision(false) 
	{
		InitSegmentExremity(startPos, destPos);
	}

private:
	void InitSegmentExremity(const Vec3f& start, const Vec3f& end)
	{
		Vec2f dir(end.x - start.x, end.y - start.y);
		dir.Normalize();
		dir = dir.PerpCCW();
		Vec3f normalizedDir3D(dir.x, dir.y, 0.f);
		m_startA = start + (normalizedDir3D * m_radius);
		m_startB = start - (normalizedDir3D * m_radius);
	}

public:

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::SegmentCanGo().
		\return An element from the #SegmentCanGoQueryResult enumeration that indicates the results of the query. */
	SegmentCanGoQueryResult GetResult() const
	{
		if (m_canGo)
			return SEGMENTCANGO_SUCCESS;

		if (!m_startTriangleSafePtr.IsValid())
			return SEGMENTCANGO_START_OUTSIDE;

		if(!IsTerrainTypeCompliant(m_terrainTypeMask, m_startTriangleSafePtr.GetNavFloorTerrainType()))
			return SEGMENTCANGO_START_MARTERIAL_FORBIDDEN;

		if (!m_destTriangleSafePtr.IsValid())
			return SEGMENTCANGO_DEST_OUTSIDE;

		if(!IsTerrainTypeCompliant(m_terrainTypeMask, m_destTriangleSafePtr.GetNavFloorTerrainType()))
			return SEGMENTCANGO_DEST_MARTERIAL_FORBIDDEN;

		if (m_collision)
			return SEGMENTCANGO_COLLISION;

		return SEGMENTCANGO_WRONG_FLOOR;
	}

	/*! \label_clear */
	void Clear()
	{
		m_radius = 0.f;
		m_searchRange = 1.f;

		m_startPos.Set(0.f, 0.f, 0.f);
		m_destPos.Set(0.f, 0.f, 0.f);
		m_startA.Set(0.f, 0.f, 0.f);
		m_startB.Set(0.f, 0.f, 0.f);

		m_startTriangleSafePtr.Invalidate();
		m_destTriangleSafePtr.Invalidate();

		CleanOutput();
	}

	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	void CleanOutput()
	{
		m_canGo = false;
		m_collision = true;
	}

	/*! Read accessor for #m_startPos. */
	Vec3f GetStartPos() const { return m_startPos; }

	/*! Read accessor for #m_destPos. */
	Vec3f GetDestPos()  const { return m_destPos; }

	/*! Read accessor for #m_startTriangleSafePtr. */
	NavTriangleSafePtr GetStartTriangleSafePtr() const { return m_startTriangleSafePtr; }

	/*! Read accessor for #m_destTriangleSafePtr. */
	NavTriangleSafePtr GetDestTriangleSafePtr()  const { return m_destTriangleSafePtr; }

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

	/*! The half-width of the segment. */
	KyFloat32 m_radius;

	/*! A bitmask that specifies which terrain types should be considered navigable during the query. */
	NavTerrainTypeMask m_terrainTypeMask;

	/*! The range of altitudes to use when determining #m_startTriangleSafePtr and #m_destTriangleSafePtr from the starting and ending points
		you provide. */
	KyFloat32 m_searchRange;

	/*! The point at the other end of the line segment from #m_startB. Computed transparently from #m_startPos and #m_radius. */
	Vec3f m_startA;

	/*! The point at the other end of the line segment from #m_startA. Computed transparently from #m_startPos and #m_radius. */
	Vec3f m_startB;

	/*! Updated by the NavMeshQueryServer. If \c true, the query succeeded, and the segment can go in the straight line from 
		#m_startPos to #m_destPos. */
	bool m_canGo;

	/*! Updated by the NavMeshQueryServer. If #m_canGo is \c false and #m_collision is \c true, the segment was blocked by a 
		border of the NavMesh or a triangle with a non-walkable terrain type. */
	bool m_collision;

	/*! Determines whether or not to store all NavMesh triangles crossed by the query, and provides access to the memory buffer
		that contains the stored triangles after the query. */
	PropagationQueryMemory m_memory;
};


}


#endif //KyPathData_SegmentCanGoQuery_H

