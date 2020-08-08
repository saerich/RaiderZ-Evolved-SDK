/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavFloorsFromBoxObstacleQuery_H
#define KyPathData_NavFloorsFromBoxObstacleQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/math/boxobstacle.h"

namespace Kaim
{

/*! Enumerates the possible results of a NavFloorsFromBoxObstacleQuery.
	\ingroup kypathdata_navmesh */
enum NavFloorsFromBoxObstacleQueryResult
{
	NAVFLOORSFROMBOX_BOX_OUTSIDE, /*!< Indicates that the box provided was outside the navigable area of the NavMesh. */
	NAVFLOORSFROMBOX_FOUND /*!< Indicates that the query successfully found one or more \NavFloors. */
};

/*!	The NavFloorsFromBoxObstacleQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::GetNavFloorPtrsInBoxObstacle().
	\pp This query retrieves all \NavFloors whose triangles intersect a specified oriented bounding box, and
	writes them to #m_navFloorPtrsBuffer. The number of \NavFloors found by the query is indicated by #m_navFloorPtrsCount.
	Note that the array held in #m_navFloorPtrsBuffer is only valid until the next query made against the NavMeshQueryServer.
	\ingroup kypathdata_navmesh */
class NavFloorsFromBoxObstacleQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	NavFloorsFromBoxObstacleQuery() : m_toleranceBelow(0.f), m_toleranceAbove(0.f), m_navFloorPtrsBuffer(KY_NULL), m_navFloorPtrsCount(0) {}

	/*! \label_constructor
		\param oobb				Sets the value of #m_OOBB. */
	NavFloorsFromBoxObstacleQuery(const BoxObstacle& oobb) : m_OOBB(oobb), m_toleranceBelow(0.f), m_toleranceAbove(0.f), m_navFloorPtrsBuffer(KY_NULL), m_navFloorPtrsCount(0) {}

	/*! \label_constructor
		\param oobb				Sets the value of #m_OOBB.
		\param toleranceBelow	Sets the value of #m_toleranceBelow.
		\param toleranceAbove	Sets the value of #m_toleranceAbove. */
	NavFloorsFromBoxObstacleQuery(const BoxObstacle& oobb, KyFloat32 toleranceBelow, KyFloat32 toleranceAbove) : m_OOBB(oobb), m_toleranceBelow(toleranceBelow),
	 m_toleranceAbove(toleranceAbove), m_navFloorPtrsBuffer(KY_NULL), m_navFloorPtrsCount(0) {}

	/*! \label_clear */
	void Clean()
	{
		m_OOBB = BoxObstacle();
		m_toleranceAbove = 0.0f;
		m_toleranceAbove = 0.0f;

		CleanOutput();
	}
	
	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	void CleanOutput()
	{
		m_navFloorPtrsBuffer = KY_NULL;
		m_navFloorPtrsCount = 0;
	}

	/*! \label_reinit
		\param oobb				Sets the value of #m_OOBB. */
	void SetUp(const BoxObstacle& oobb)
	{
		m_OOBB = oobb;
		m_toleranceBelow = 0.0f;
		m_toleranceAbove = 0.0f;
		m_navFloorPtrsBuffer = KY_NULL;
		m_navFloorPtrsCount = 0;
	}

	/*! \label_reinit
		\param oobb				Sets the value of #m_OOBB.
		\param toleranceBelow	Sets the value of #m_toleranceBelow.
		\param toleranceAbove	Sets the value of #m_toleranceAbove. */
	void SetUp(const BoxObstacle& oobb, KyFloat32 toleranceBelow, KyFloat32 toleranceAbove)
	{
		m_OOBB = oobb;
		m_toleranceBelow = toleranceBelow;
		m_toleranceAbove = toleranceAbove;
		m_navFloorPtrsBuffer = KY_NULL;
		m_navFloorPtrsCount = 0;
	}

	/*! Write accessor for #m_OOBB. */
	void SetOOBB(const BoxObstacle& oobb) { m_OOBB = oobb; }

	/*! Write accessor for #m_toleranceBelow. */
	void SetToleranceBelow(KyFloat32 toleranceBelow) { m_toleranceBelow = toleranceBelow; }

	/*! Write accessor for #m_toleranceAbove. */
	void SetToleranceAbove(KyFloat32 toleranceAbove) { m_toleranceAbove = toleranceAbove; }

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::GetNavFloorPtrsInBoxObstacle().
		\return An element from the #NavFloorsFromBoxObstacleQueryResult enumeration that indicates the results of the query. */
	NavFloorsFromBoxObstacleQueryResult GetResult() const
	{
		if (m_navFloorPtrsCount == 0) return NAVFLOORSFROMBOX_BOX_OUTSIDE;
		return NAVFLOORSFROMBOX_FOUND;
	}

public:
	/*! The bounding box within which to search for \NavFloors. */
	BoxObstacle m_OOBB;

	/*! The search volume is extended by this distance below the bottom face of #m_OOBB. */
	KyFloat32 m_toleranceBelow;

	/*! The search volume is extended by this distance above the top face of #m_OOBB. */
	KyFloat32 m_toleranceAbove;

	/*! Updated by the NavMeshQueryServer to indicate the \NavFloors found during the query. Note that this memory buffer
		is maintained internally by the NavMeshQueryServer, and will be cleared by the next query. */
	NavFloorPtr* m_navFloorPtrsBuffer;

	/*! Updated by the NavMeshQueryServer to indicate the number of \NavFloors found during the query and written to #m_navFloorPtrsBuffer. */
	KyUInt32 m_navFloorPtrsCount;
};

}


#endif //KyPathData_NavFloorsFromBoxObstacleQuery_H

