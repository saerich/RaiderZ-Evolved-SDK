/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_TrianglesFromPosQuery_H
#define KyPathData_TrianglesFromPosQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of a TrianglesFromPosQuery.
	\ingroup kypathdata_navmesh */
enum TrianglesFromPosQueryResult
{
	TRIANGLESFROMPOS_POINT_OUTSIDE, /*!< Indicates that the point provided was outside the navigable boundaries of the NavMesh. */
	TRIANGLESFROMPOS_TRIANGLES_FOUND /*!< Indicates that the query was completed successfully, and at least one triangle was found. */
};

/*! Enumerates the possible ways that a TrianglesFromPosQuery can choose the triangles to retrieve.
	\ingroup kypathdata_navmesh */
enum TrianglesFromPosQueryType
{
	TRIANGLESFROMPOS_FIND_ALL, /*!< Indicates that the query should retrieve all triangles at the same (X,Y) coordinates as the point provided, regardless of whether they lie above or below the point. */
	TRIANGLESFROMPOS_FIND_ALL_BELOW, /*!< Indicates that the query should retrieve all triangles that lie below the point provided. */
	TRIANGLESFROMPOS_FIND_ALL_ABOVE, /*!< Indicates that the query should retrieve all triangles that lie above the point provided. */
	TRIANGLESFROMPOS_FIND_NEAREST_ABOVE_AND_BELOW /*!< Indicates that the query should retrieve the first triangle that lies above the point provided, and the first triangle that lies below the point provided. */
};

/*!	The NearestTriangleFromPosQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::GetTriangleIndicesFromPosition().
	\pp This query takes a position in 3D space (#m_pos), and finds the triangles in the NavMesh that lie at the same (X,Y)
	coordinates. The search can look only above the position, only below the position, or both above and below
	(determined by #m_queryType).
	\pp If one or more triangles are found, they are written to #m_trianglesIdsBuffer. The number of triangles found is
	indicated by #m_trianglesIdsCount. Note that the array held in #m_navFloorPtrsBuffer is only valid until the next query
	made against the NavMeshQueryServer.
	\ingroup kypathdata_navmesh */
class TrianglesFromPosQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	TrianglesFromPosQuery() : m_biasInAltitude(0.f), m_queryType(TRIANGLESFROMPOS_FIND_ALL), m_trianglesIdsBuffer(KY_NULL), m_trianglesIdsCount(0)  {}

	/*! \label_constructor
		\param pos				Sets the value of #m_pos.
		\param bias				Sets the value of #m_biasInAltitude.
		\param type				Sets the value of #m_queryType. */
	TrianglesFromPosQuery(const Vec3f& pos, KyFloat32 bias = 0.0f, TrianglesFromPosQueryType type = TRIANGLESFROMPOS_FIND_ALL) : m_pos(pos), m_biasInAltitude(bias), m_queryType(type), m_trianglesIdsBuffer(KY_NULL), m_trianglesIdsCount(0)  {}

	/*! \label_clear */
	KY_FORCE_INLINE void Clean()
	{
		m_pos.Set(0.f, 0.f, 0.f);
		m_biasInAltitude = 0.0f;
		m_queryType = TRIANGLESFROMPOS_FIND_ALL;

		CleanOutput();
	}

	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	KY_FORCE_INLINE void CleanOutput()
	{
		m_trianglesIdsBuffer = KY_NULL;
		m_trianglesIdsCount = 0;
	}

	/*! \label_reinit
		\param pos				Sets the value of #m_pos.
		\param bias				Sets the value of #m_biasInAltitude.
		\param type				Sets the value of #m_queryType. */
	KY_FORCE_INLINE void SetUp(const Vec3f& pos, KyFloat32 bias = 0.0f, TrianglesFromPosQueryType type = TRIANGLESFROMPOS_FIND_ALL)
	{
		m_pos = pos;
		m_biasInAltitude = bias;
		m_queryType = type;
		m_trianglesIdsBuffer = KY_NULL;
		m_trianglesIdsCount = 0;
	}

	/*! Write accessor for #m_pos. */
	KY_FORCE_INLINE void SetPos(const Vec3f& pos) { m_pos = pos; }

	/*! Write accessor for #m_biasInAltitude. */
	KY_FORCE_INLINE void SetBias(KyFloat32 bias) { m_biasInAltitude = bias; }

	/*! Write accessor for #m_queryType. */
	KY_FORCE_INLINE void SetQueryType(TrianglesFromPosQueryType type) { m_queryType = type; }

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::GetTriangleIndicesFromPosition().
		\return An element from the #TrianglesFromPosQueryResult enumeration that indicates the results of the query. */
	KY_FORCE_INLINE TrianglesFromPosQueryResult GetResult() const
	{
		if (m_trianglesIdsCount == 0) return TRIANGLESFROMPOS_POINT_OUTSIDE;
		return TRIANGLESFROMPOS_TRIANGLES_FOUND;
	}

public:
	/*! The position on the (X,Y) plane for which the triangles will be retrieved. */
	Vec3f m_pos;

	/*! This value is added to the altitude of #m_pos before conducting the query. */
	KyFloat32 m_biasInAltitude;

	/*! An element from the #TrianglesFromPosQueryType enumeration that indicates whether to look for
		triangles only above #m_pos (#TRIANGLESFROMPOS_FIND_ALL_ABOVE), only below #m_pos 
		(#TRIANGLESFROMPOS_FIND_ALL_BELOW), both above and below (#TRIANGLESFROMPOS_FIND_ALL), or to take
		only the nearest triangle above and below (TRIANGLESFROMPOS_FIND_NEAREST_ABOVE_AND_BELOW). */
	TrianglesFromPosQueryType m_queryType;

	/*! Updated by the NavMeshQueryServer to store the triangles found during the query. Note that this memory buffer
		is maintained internally by the NavMeshQueryServer, and will be cleared by the next query.. */
	NavTrianglePtr* m_trianglesIdsBuffer;

	/*! Updated by the NavMeshQueryServer to indicate the number of triangles found during the query and written to #m_trianglesIdsBuffer. */
	KyUInt32 m_trianglesIdsCount;
};

}


#endif //KyPathData_TrianglesFromPosQuery_H

