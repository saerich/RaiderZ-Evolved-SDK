/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_AXISALIGNEDBOX_H
#define KY_AXISALIGNEDBOX_H

#include <kypathdata/math/blob/axisalignedboxdata.h>
#include <kypathdata/generic/basetypes.h>


namespace Kaim
{

/*! Represents a bounding box whose orientations match the orientation of the axes in the \SDKName coordinate system.
	\ingroup kypathdata_math */
class AxisAlignedBox
{
public:
	/*! \label_constructor If you use this constructor, you must call Setup() to set the dimensions of the bounding box
		before it can be used. */
	AxisAlignedBox() { Reset(); }

	/*! \label_constructor
		\param aabbMin			The minima of the bounding box: the corner with the smallest coordinate values along the
								X, Y and Z axes.
		\param aabbMax			The maxima of the bounding box: the corner with the largest coordinate values along the
								X, Y and Z axes. */
	AxisAlignedBox(const Vec3f& aabbMin, const Vec3f& aabbMax) { Setup(aabbMin, aabbMax); }

	/*! \label_constructor Copies the state of the specified AxisAlignedBox to the newly created object. */
	AxisAlignedBox(const AxisAlignedBox& aabb) { *this = aabb; }

	/*! Copies the state of the specified AxisAlignedBox to this object. */
	AxisAlignedBox& operator=(const AxisAlignedBox& other)
	{
		m_data    = other.m_data;
		m_isValid = other.m_isValid;
		return *this;
	}

	/*! Indicates whether or not this object has been successfully set up and is ready for use. */
	KyBool IsValid() const {return m_isValid;}

	/*! Sets the dimensions of the bounding box.
		\param aabbMin			The minima of the bounding box: the corner with the smallest coordinate values along the
								X, Y and Z axes.
		\param aabbMax			The maxima of the bounding box: the corner with the largest coordinate values along the
								X, Y and Z axes. */
	KyResult Setup(const Vec3f& aabbMin, const Vec3f& aabbMax);

	/*! \label_reinit */
	void Reset()
	{
		m_data.Reset();
		m_isValid = KY_FALSE;
	}

	/*! Retrieves the minima of the bounding box: the corner with the smallest coordinate values along the X, Y and Z axes. */
	const Vec3f& GetMin() const {return m_data.m_min;}

	/*! Retrieves the maxima of the bounding box: the corner with the largest coordinate values along the X, Y and Z axes. */
	const Vec3f& GetMax() const {return m_data.m_max;}

	/*! Indicates whether or not the specified position is inside the area occupied by the bounding box, ignoring altitude. */
	KyBool IsInside2d(const Vec3f& point) const;

	/*! Indicates whether or not the specified position is inside the three-dimensional volume occupied by the bounding box. */
	KyBool IsInside3d(const Vec3f& point) const;


protected:
	AxisAlignedBoxData m_data;
	KyBool m_isValid;
};

} //namespace Kaim

#endif // KY_AXISALIGNEDBOX_H
