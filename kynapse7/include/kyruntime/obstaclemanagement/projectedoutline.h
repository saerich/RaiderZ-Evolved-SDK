/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_obstaclemanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_PORJECTEDOUTLINE_H
#define KY_PORJECTEDOUTLINE_H

#include <kyruntime/obstaclemanagement/obstacleoutline.h>

#include <kyruntime/mesh/obstaclelayer.h>

#include <kypathdata/generic/types.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{

class Vec3f;


/*! The ProjectedOutline class derives from ObstacleOutline, and maintains the shape of an Obstacle that is projected
	into an ObstacleLayer.
	\pp Each Obstacle maintains an instance of this class in its Obstacle::m_projectedOutline member. This outline
	is updated by the IOutlineUpdatePolicy in use by the ObstacleManager when that Obstacle needs to be projected into
	an ObstacleLayer.
	\ingroup kyruntime_obstaclemanagement */
class ProjectedOutline: public ObstacleOutline
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor For internal use. */
	ProjectedOutline() :
		ObstacleOutline(),
		m_navFloorPtrs(KY_NULL),
		m_maxNavFloorCount(0),
		m_hasChanged(KY_FALSE),
		m_activated(KY_FALSE)
	{}

	/*! \label_destructor For internal use. */
	~ProjectedOutline()
	{
		DeletePointArray();
		DeleteNavFloorArray();
	}

	/*! Sets the maximum size of the #m_navFloorPtrs array. For internal use. */
	KyResult SetupNavFloorArray(KyUInt32 maxNavFloorCount);

	/*! Deletes all elements from the #m_navFloorPtrs array. For internal use. */
	void DeleteNavFloorArray();

	/*! Resizes the #m_navFloorPtrs array to the specified maximum size. For internal use. */
	KyResult ResizeNavFloorArray(KyUInt32 maxNavFloorCount);


public:
	NavFloorPtr* m_navFloorPtrs; /*!< The array of \NavFloors the outline has been projected in. Do not modify. */
	KyUInt32 m_maxNavFloorCount; /*!< The maximum size of the #m_navFloorPtrs array. Do not modify directly. */
	KyBool m_hasChanged; /*!< Indicates whether or not this outline has been updated. */
	KyBool m_activated; /*!< Indicates whether or not this outline is active. */
};

} // Kaim

#endif // KY_PORJECTEDOUTLINE_H
