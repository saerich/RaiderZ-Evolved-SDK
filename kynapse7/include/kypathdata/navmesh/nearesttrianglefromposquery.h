/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NearestTriangleFromPosQuery_H
#define KyPathData_NearestTriangleFromPosQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navtriangleptr.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of a NearestTriangleFromPosQuery.
	\ingroup kypathdata_navmesh */
enum NearestTriangleFromPosQueryResult
{
	NEARESTTRIANGLE_POINT_OUTSIDE, /*!< Indicates that the point provided was outside the navigable boundaries of the NavMesh. */
	NEARESTTRIANGLE_TRIANGLE_FOUND /*!< Indicates that the query was completed successfully, and a triangle was found. */
};

/*! Enumerates the possible ways that a NearestTriangleFromPosQuery can choose the triangle to retrieve.
	\ingroup kypathdata_navmesh */
enum NearestTriangleFromPosQueryType
{
	NEARESTTRIANGLE_FIND_NEAREST, /*!< Indicates that the query should retrieve the nearest triangle to the point provided, regardless of whether it lies above or below the point. */
	NEARESTTRIANGLE_FIND_NEAREST_BELOW, /*!< Indicates that the query should retrieve the first triangle below the point provided. */
	NEARESTTRIANGLE_FIND_NEAREST_ABOVE /*!< Indicates that the query should retrieve the first triangle above the point provided. */
};

/*!	The NearestTriangleFromPosQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::GetNearestTrianglePtrFromPos().
	\pp This query takes a position in 3D space (#m_pos), and finds the nearest triangle in the NavMesh that lies at the same (X,Y)
	coordinates. The search can look only above the position, only below the position, or both above and below
	(determined by #m_queryType).
	\pp If a triangle is found, it is written to #m_result.
	\ingroup kypathdata_navmesh */
class NearestTriangleFromPosQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS


public:
	/*! \label_constructor */
	NearestTriangleFromPosQuery() : m_biasInAltitude(0.f), m_searchRange(1.f), m_queryType(NEARESTTRIANGLE_FIND_NEAREST), m_relativeAltitude(KyFloat32MAXVAL) {}

	/*! \label_constructor
		\param pos				Sets the value of #m_pos.
		\param bias				Sets the value of #m_biasInAltitude.
		\param searchRange		Sets the value of #m_searchRange.
		\param type				Sets the value of #m_queryType. */
	NearestTriangleFromPosQuery(const Vec3f& pos, KyFloat32 bias = 0.0f, KyFloat32 searchRange = 1.f, NearestTriangleFromPosQueryType type = NEARESTTRIANGLE_FIND_NEAREST) :
	m_pos(pos), m_biasInAltitude(bias), m_searchRange(fabs(searchRange)), m_queryType(type), m_relativeAltitude(KyFloat32MAXVAL) {}

	/*! \label_clear */
	KY_FORCE_INLINE void Clean()
	{
		m_pos.Set(0.f, 0.f, 0.f);
		m_biasInAltitude = 0.0f;
		m_searchRange = 1.f;
		m_queryType = NEARESTTRIANGLE_FIND_NEAREST;

		CleanOutput();
	}
	
	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	KY_FORCE_INLINE void CleanOutput()
	{
		m_result.Invalidate();
		m_relativeAltitude = KyFloat32MAXVAL;
	}

	/*! \label_reinit
		\param pos				Sets the value of #m_pos.
		\param bias				Sets the value of #m_biasInAltitude.
		\param searchRange		Sets the value of #m_searchRange.
		\param type				Sets the value of #m_queryType. */
	KY_FORCE_INLINE void SetUp(const Vec3f& pos, KyFloat32 bias = 0.0f, KyFloat32 searchRange = 1.f, NearestTriangleFromPosQueryType type = NEARESTTRIANGLE_FIND_NEAREST)
	{
		m_pos = pos;
		m_biasInAltitude = bias;
		m_searchRange = searchRange;
		m_queryType = type;
		CleanOutput();
	}

	/*! Write accessor for #m_pos. */
	void SetPos(const Vec3f& pos) { m_pos = pos; }

	/*! Write accessor for #m_biasInAltitude. */
	void SetBias(KyFloat32 bias) { m_biasInAltitude = bias; }

	/*! Write accessor for #m_searchRange. */
	void SetSearchRange(KyFloat32 searchRange) { m_searchRange = fabs(searchRange); }

	/*! Write accessor for #m_queryType. */
	void SetQueryType(NearestTriangleFromPosQueryType type) { m_queryType = type; }

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::GetNearestTrianglePtrFromPos().
		\return An element from the #NearestTriangleFromPosQueryResult enumeration that indicates the results of the query. */
	NearestTriangleFromPosQueryResult GetResult() const
	{
		if (!m_result.IsValid())
			return NEARESTTRIANGLE_POINT_OUTSIDE;

		return NEARESTTRIANGLE_TRIANGLE_FOUND;
	}

public:
	/*! The position on the (X,Y) plane whose nearest triangle will be retrieved. */
	Vec3f m_pos;

	/*! This value is added to the altitude of #m_pos before conducting the query. */
	KyFloat32 m_biasInAltitude;

	/*! The range of altitude above and below #m_pos that will be searched for triangles. */
	KyFloat32 m_searchRange;

	/*! An element from the #NearestTriangleFromPosQueryType enumeration that indicates whether to look for
		triangles only above #m_pos (#NEARESTTRIANGLE_FIND_NEAREST_ABOVE), only below #m_pos 
		(#NEARESTTRIANGLE_FIND_NEAREST_BELOW), or both above and below (#NEARESTTRIANGLE_FIND_NEAREST). */
	NearestTriangleFromPosQueryType m_queryType;

	/*! Updated by the NavMeshQueryServer to store the nearest triangle found to #m_pos. If no triangle is found,
		it is set to #KyNavTrianglePtr_Outside. */
	NavTrianglePtr m_result;

	/*! Updated by the NavMeshQueryServer to indicate the difference in altitude between #m_pos and the projected
		altitude of #m_pos on the triangle stored in #m_result. */
	KyFloat32 m_relativeAltitude;
};

}


#endif //KyPathData_NearestTriangleFromPosQuery_H

