/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_obstaclemanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_OBSTACLEOUTLINE_H
#define KY_OBSTACLEOUTLINE_H

#include <kyruntime/mesh/obstaclelayer.h>

#include <kypathdata/generic/types.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{

class Vec3f;


/*! The ObstacleOutline class represents a two-dimensional polyline with vertical thickness, used to
	reflect the actual physical dimensions of the Obstacle internally within the \SDKName World.
	\pp Each Obstacle maintains an instance of this class in its Obstacle::m_exactOutline member, which
	is automatically kept up to date with the shape of the Obstacle passed in each call to 
	Obstacle::Update().
	\pp Each Obstacle also maintains an instance of the derived ProjectedOutline class in its
	Obstacle::m_projectedOutline class, which reflects the outline that is actually projected into the
	NavMesh.
	\ingroup kyruntime_obstaclemanagement */
class ObstacleOutline
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	ObstacleOutline() :
		m_points(KY_NULL),
		m_maxPointCount(0),
		m_pointCount(0),
		m_thickness(0.0f),
		m_activated(KY_FALSE)
	{
	}

	/*! \label_destructor */
	~ObstacleOutline()
	{
		DeletePointArray();
	}

	/*! Sets the maximum size of the #m_points array. For internal use. */
	KyResult SetupPointArray(KyUInt32 maxPointCount);

	/*! Deletes all elements from the #m_points array. For internal use. */
	void DeletePointArray();

	/*! Resizes the #m_points array to the specified maximum size. For internal use. */
	KyResult ResizePointArray(KyUInt32 maxPointCount);


public:
	Vec3f* m_points; /*!< The array of points that make up the polyline. */
	KyUInt32 m_maxPointCount; /*!< The maximum number of points that can be held in the #m_points array. */
	KyUInt32 m_pointCount; /*!< The number of points currently used in the #m_points array. */
	KyFloat32 m_thickness; /*!< The vertical thickness of the Obstacle. */
	KyBool m_activated; /*!< Indicates whether or not the Obstacle is currently activated. */
};

} // Kaim

#endif // KY_OBSTACLEOUTLINE_H
