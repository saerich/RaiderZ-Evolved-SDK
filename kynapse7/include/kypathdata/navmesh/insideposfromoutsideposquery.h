/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_InsidePosFromOutsidePosQuery_H
#define KyPathData_InsidePosFromOutsidePosQuery_H


#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/navmesh/navtriangleptr.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*! Enumerates the possible results of an InsidePosFromOutsidePosQuery.
	\ingroup kypathdata_navmesh */
enum InsidePosFromOutsidePosQueryResult
{
	INSIDEPOSFROMOUTSIDE_POS_NOT_FOUND, /*!< Indicates that no valid position was found within the bounding box set for the query. */
	INSIDEPOSFROMOUTSIDE_POS_FOUND /*!< Indicates that the query was completed successfully, and a valid position was found inside a NavMesh. */
};

/*!	The InsidePosFromOutsidePosQuery class contains all the inputs and outputs for a query that can be passed in a call to
	NavMeshQueryServer::GetInsidePosFromOutsidePos().
	\pp This query finds a valid position inside the boundaries of the NavMesh, at a specified distance from the NavMesh boundary that lies
	nearest to a specified input position (#m_pos). 
	\pp This query is typically used to retrieve a valid NavMesh position that is as near as possible to an input position that lies outside the 
	(X,Y) boundaries of the NavMesh. However, it can also be used with an input position that is inside the NavMesh. In either case, it works the
	same way: it finds the NavMesh boundary nearest to the specified point, then finds the closest point to the input position that lies at the
	specified distance inside that boundary. 
	\pp The search space is constrained within an axis-aligned bounding box centered on #m_pos. The horizontal width of the bounding box is equal to
	2 * #m_horizontalSearchRange along both the X and Y axes. The vertical height of the bounding box is equal to 2 * #m_verticalSearchRange along the
	Z axis.
	\pp If a NavMesh boundary is found within the search space, and a valid position could be found near that boundary, the position is written to
	#m_insidePos, and the triangle that contains the output position is written to #m_resultTrianglePtr.
	\ingroup kypathdata_navmesh */
class InsidePosFromOutsidePosQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS


public:
	/*! \label_constructor */
	InsidePosFromOutsidePosQuery() : m_biasInAltitude(0.f), m_horizontalSearchRange(2.f), m_verticalSearchRange(1.f), m_distFromObstacle(0.05f),
		m_insidePos(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL) {}

	/*! \label_constructor
		\param pos						Sets the value of #m_pos.
		\param bias						Sets the value of #m_biasInAltitude.
		\param horizontalSearchRange	Sets the value of #m_horizontalSearchRange.
		\param verticalSearchRange		Sets the value of #m_verticalSearchRange.
		\param distFromObstacle			Sets the value of #m_distFromObstacle.*/
	InsidePosFromOutsidePosQuery(const Vec3f& pos, KyFloat32 bias = 0.0f, KyFloat32 horizontalSearchRange = 2.f, KyFloat32 verticalSearchRange = 1.f, KyFloat32 distFromObstacle = 0.05f) :
	m_pos(pos), m_biasInAltitude(bias), m_horizontalSearchRange(fabs(horizontalSearchRange)), m_verticalSearchRange(fabs(verticalSearchRange)), m_distFromObstacle(fabs(distFromObstacle)),
	m_insidePos(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL) {}

	/*! \label_clear */
	KY_FORCE_INLINE void Clean()
	{
		m_pos.Set(0.f, 0.f, 0.f);
		m_biasInAltitude = 0.0f;
		m_verticalSearchRange = 1.f;
		m_horizontalSearchRange = 2.f;
		m_distFromObstacle = 0.05f;

		CleanOutput();
	}
	
	/*! Clears the values set by the NavMeshQueryServer and used by GetResult() to interpret the results of the query. */
	KY_FORCE_INLINE void CleanOutput()
	{
		m_resultTrianglePtr.Invalidate();
		m_insidePos.Set(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL);
	}

	/*! \label_reinit
		\param pos						Sets the value of #m_pos.
		\param bias						Sets the value of #m_biasInAltitude.
		\param horizontalSearchRange	Sets the value of #m_horizontalSearchRange.
		\param verticalSearchRange		Sets the value of #m_verticalSearchRange.
		\param distFromObstacle			Sets the value of #m_distFromObstacle.*/
	KY_FORCE_INLINE void SetUp(const Vec3f& pos, KyFloat32 bias = 0.0f, KyFloat32 horizontalSearchRange = 2.f, KyFloat32 verticalSearchRange = 1.f, KyFloat32 distFromObstacle = 0.05f)
	{
		m_pos = pos;
		m_biasInAltitude = bias;
		m_horizontalSearchRange = fabs(horizontalSearchRange);
		m_verticalSearchRange = fabs(verticalSearchRange);
		m_distFromObstacle = fabs(distFromObstacle);
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

	/*! Write accessor for #m_distFromObstacle. */
	void SetDistFromObstacle(KyFloat32 distFromObstacle) { m_distFromObstacle = fabs(distFromObstacle); }

	/*! Summarizes the result of passing this object in a call to NavMeshQueryServer::GetInsidePosFromOutsidePos().
		\return An element from the #InsidePosFromOutsidePosQueryResult enumeration that indicates the results of the query. */
	InsidePosFromOutsidePosQueryResult GetResult() const
	{
		if (!m_resultTrianglePtr.IsValid())
			return INSIDEPOSFROMOUTSIDE_POS_NOT_FOUND;

		return INSIDEPOSFROMOUTSIDE_POS_FOUND;
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

	/*! Specifies a preferred distance between the final chosen point and the border of the NavMesh. If this value is specified, the query
		will attempt to find a final valid point that is this distance inside the border of the NavMesh. However, in some cases, the layout
		of the geometry may force the final valid point to be closer to the NavMesh border than this value. */
	KyFloat32 m_distFromObstacle;

	/*! Updated by the NavMeshQueryServer to store the valid point found inside the NavMesh, if any. */
	Vec3f m_insidePos;
	
	/*! Updated by the NavMeshQueryServer to store the NavMesh triangle that contains #m_insidePos. */
	NavTrianglePtr m_resultTrianglePtr;
};

}


#endif //KyPathData_InsidePosFromOutsidePosQuery_H

