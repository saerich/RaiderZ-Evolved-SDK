/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_obstaclemanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SHAPEOBB_H
#define KY_SHAPEOBB_H

#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"


namespace Kaim
{

/*! This class represents a free-rotating three-dimensional bounding box. You can pass an instance of this
	class to Obstacle::Update() in order to represent the physical dimensions of your Obstacle.
	\ingroup kyruntime_obstaclemanagement */
class ShapeOBB
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor You must call one of the implementations of SetOBB() before you can use the newly created object. */
	ShapeOBB() {}

	/*! \label_constructor
		\param center				The center of the box.
		\param firstAxis			A normalized vector that indicates the orientation of the first axis of the box.
		\param secondAxis			A normalized vector that indicates the orientation of the second axis of the box.
		\param thirdAxis			A normalized vector that indicates the orientation of the third axis of the box.
		\param firstAxisExtend		The length of the box in the direction of \c firstAxis.
		\param secondAxisExtend		The length of the box in the direction of \c secondAxis.
		\param thirdAxisExtend		The length of the box in the direction of \c thirdAxis.
		\pp All axes and extents must be specified in the \SDKName coordinate system. See :USERGUIDE:"The \SDKName Coordinate System". */
	ShapeOBB(const Vec3f& center,
		const Vec3f& firstAxis, const Vec3f& secondAxis, const Vec3f& thirdAxis,
		KyFloat32 firstAxisExtend, KyFloat32 secondAxisExtend, KyFloat32 thirdAxisExtend) :
		m_center(center), m_extends(firstAxisExtend, secondAxisExtend, thirdAxisExtend)
	{
		m_axes[0] = firstAxis;
		m_axes[1] = secondAxis;
		m_axes[2] = thirdAxis;
	}

	/*! \label_destructor */
	~ShapeOBB() {}

	/*! Sets the dimensions of the oriented bounding box.
		\param center				The center of the box.
		\param firstAxis			A normalized vector that indicates the orientation of the first axis of the box.
		\param secondAxis			A normalized vector that indicates the orientation of the second axis of the box.
		\param thirdAxis			A normalized vector that indicates the orientation of the third axis of the box.
		\param firstAxisExtend		The length of the box in the direction of \c firstAxis.
		\param secondAxisExtend		The length of the box in the direction of \c secondAxis.
		\param thirdAxisExtend		The length of the box in the direction of \c thirdAxis.
		\pp All axes and extents must be specified in the \SDKName coordinate system. See :USERGUIDE:"The \SDKName Coordinate System". */
	KyResult SetOBB(const Vec3f& center,
		const Vec3f& firstAxis, const Vec3f& secondAxis, const Vec3f& thirdAxis,
		KyFloat32 firstAxisExtend, KyFloat32 secondAxisExtend, KyFloat32 thirdAxisExtend);

	/*! Sets the dimensions of the oriented bounding box.
		\param center				The center of the box.
		\param axes					An array of three normalized vectors that indicate the orientation of the three axes of the box.
		\param extends				A vector that indicates the length of the box in the direction of each axis.
		\pp All axes and extents must be specified in the \SDKName coordinate system. See :USERGUIDE:"The \SDKName Coordinate System". */
	KyResult SetOBB(const Vec3f& center, const Vec3f axes[3], const Vec3f& extends)
	{
		return SetOBB(center, axes[0], axes[1], axes[2], extends[0], extends[1], extends[2]);
	}


public:
	Vec3f m_center; /*!< The center of the box. Do not modify directly. */
	Vec3f m_axes[3]; /*!< The axes of the box. Do not modify directly. */
	Vec3f m_extends; /*!< The length of the box along its three axes. Do not modify directly. */
};

} // Kaim

#endif // KY_SHAPEOBB_H
