/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NearestObstacleHalfEdgeFromPosQuery_H
#define KyPathData_NearestObstacleHalfEdgeFromPosQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navhalfedgeptr.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of a NearestObstacleHalfEdgeFromPosQuery.
	\ingroup kypathdata_navmesh */
enum NearestObstacleHalfEdgeFromPosQueryResult
{
	NEARESTHALFEDGE_HALFEDGE_NOT_FOUND, /*!< Indicates that no NavMesh border was found within the bounding box set for the query. */
	NEARESTHALFEDGE_HALFEDGE_FOUND /*!< Indicates that the query was completed successfully, and a NavMesh border was found. */
};

/*!	The NearestObstacleHalfEdgeFromPosQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::GetNearestObstacleHalfEdgeFromPos().
	\pp This query finds the NavMesh border nearest to a specified 3D position (#m_pos). The input position may be either inside or outside the
	boundaries of the NavMesh.
	\pp The search space is constrained within an axis-aligned bounding box centered on #m_pos. The horizontal width of the bounding box is equal to
	2 * #m_horizontalSearchRange along both the X and Y axes. The vertical height of the bounding box is equal to 2 * #m_verticalSearchRange along the
	Z axis.
	\pp If a NavMesh border is found in the bounding box, its NavHalfEdgePtr is written to #m_nearestHalfEdge, and the nearest point on this edge
	is written to #m_nearestPointOnHalfEdge.
	\ingroup kypathdata_navmesh */
class NearestObstacleHalfEdgeFromPosQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS


public:
	/*! \label_constructor */
	NearestObstacleHalfEdgeFromPosQuery() : m_biasInAltitude(0.f), m_horizontalSearchRange(2.f), m_verticalSearchRange(1.f),
		m_nearestPointOnHalfEdge(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL), m_squareDistFromHalfEdge(KyFloat32MAXVAL) {}

	/*! \label_constructor
		\param pos						Sets the value of #m_pos.
		\param bias						Sets the value of #m_biasInAltitude.
		\param horizontalSearchRange	Sets the value of #m_horizontalSearchRange.
		\param verticalSearchRange		Sets the value of #m_verticalSearchRange.*/
	NearestObstacleHalfEdgeFromPosQuery(const Vec3f& pos, KyFloat32 bias = 0.0f, KyFloat32 horizontalSearchRange = 2.f, KyFloat32 verticalSearchRange = 1.f) :
	m_pos(pos), m_biasInAltitude(bias), m_horizontalSearchRange(fabs(horizontalSearchRange)), m_verticalSearchRange(fabs(verticalSearchRange)),
	m_nearestPointOnHalfEdge(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL), m_squareDistFromHalfEdge(KyFloat32MAXVAL) {}

	/*! \label_clear */
	KY_FORCE_INLINE void Clean()
	{
		m_pos.Set(0.f, 0.f, 0.f);
		m_biasInAltitude = 0.0f;
		m_verticalSearchRange = 1.f;
		m_horizontalSearchRange = 2.f;

		CleanOutput();
	}
	
	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	KY_FORCE_INLINE void CleanOutput()
	{
		m_nearestHalfEdge.Invalidate();
		m_nextHalfEdgeOnObstacle.Invalidate();
		m_prevHalfEdgeOnObstacle.Invalidate();
		m_nearestPointOnHalfEdge.Set(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL);
		m_squareDistFromHalfEdge = KyFloat32MAXVAL;
	}

	/*! \label_reinit
		\param pos						Sets the value of #m_pos.
		\param bias						Sets the value of #m_biasInAltitude.
		\param horizontalSearchRange	Sets the value of #m_horizontalSearchRange.
		\param verticalSearchRange		Sets the value of #m_verticalSearchRange.*/
	KY_FORCE_INLINE void SetUp(const Vec3f& pos, KyFloat32 bias = 0.0f, KyFloat32 horizontalSearchRange = 2.f, KyFloat32 verticalSearchRange = 1.f)
	{
		m_pos = pos;
		m_biasInAltitude = bias;
		m_horizontalSearchRange = fabs(horizontalSearchRange);
		m_verticalSearchRange = fabs(verticalSearchRange);
		CleanOutput();
	}

	/*! Write accessor for #m_pos. */
	void SetPos(const Vec3f& pos) { m_pos = pos; }

	/*! Write accessor for #m_biasInAltitude. */
	void SetBias(KyFloat32 bias) { m_biasInAltitude = bias; }

	/*! Write accessor for #m_horizontalSearchRange. */
	void SetHorizontalSearchRange(KyFloat32 horizontalSearchRange) { m_horizontalSearchRange = fabs(horizontalSearchRange); }

	/*! Write accessor for #m_verticalSearchRange. */
	void SetVerticalSearchRange(KyFloat32 verticalSearchRange) { m_verticalSearchRange = fabs(verticalSearchRange); }

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::GetNearestObstacleHalfEdgeFromPos().
		\return An element from the #NearestObstacleHalfEdgeFromPosQueryResult enumeration that indicates the results of the query. */
	NearestObstacleHalfEdgeFromPosQueryResult GetResult() const
	{
		if (!m_nearestHalfEdge.IsValid())
			return NEARESTHALFEDGE_HALFEDGE_NOT_FOUND;

		return NEARESTHALFEDGE_HALFEDGE_FOUND;
	}

public:
	/*! The starting position for the query. */
	Vec3f m_pos;

	/*! This value is added to the altitude of #m_pos before conducting the query. */
	KyFloat32 m_biasInAltitude;

	/*! The horizontal half-width (along both the X and Y axes) of the axis-oriented bounding box within which the search is performed. */
	KyFloat32 m_horizontalSearchRange; 

	/*! The vertical half-height (along the Z axis) of the axis-oriented bounding box within which the search is performed. */
	KyFloat32 m_verticalSearchRange;

	/*! Updated by the NavMeshQueryServer to store the nearest triangle edge found to #m_pos. If no NavMesh border is found,
		this NavHalfEdgePtr will be invalid. */
	NavHalfEdgePtr m_nearestHalfEdge;

	/*! Updated by the NavMeshQueryServer to store the next triangle edge along the NavMesh border after #m_nearestHalfEdge. If no 
		NavMesh border is found, this NavHalfEdgePtr will be invalid. */
	NavHalfEdgePtr m_nextHalfEdgeOnObstacle;

	/*! Updated by the NavMeshQueryServer to store the previous triangle edge along the NavMesh border before #m_nearestHalfEdge. If no 
		NavMesh border is found, this NavHalfEdgePtr will be invalid. */
	NavHalfEdgePtr m_prevHalfEdgeOnObstacle;

	/*! Updated by the NavMeshQueryServer to indicate the point on #m_nearestHalfEdge that is the nearest to #m_pos. */
	Vec3f m_nearestPointOnHalfEdge;

	/*! Updated by the NavMeshQueryServer to indicate the square distance between #m_pos and #m_nearestPointOnHalfEdge. */
	KyFloat32 m_squareDistFromHalfEdge;
};

}


#endif //KyPathData_NearestObstacleHalfEdgeFromPosQuery_H

