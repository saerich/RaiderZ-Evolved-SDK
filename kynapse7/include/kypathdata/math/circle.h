/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_CIRCLE_H
#define KY_CIRCLE_H

#include <kypathdata/math/vec3f.h>


namespace Kaim
{

/*!	Represents a two-dimensional circle.
	\ingroup kypathdata_math */
class Circle
{
public:
	/*! \label_constructor If you use this constructor, you must call Setup() to set the dimensions of the circle
		before it can be used. */
	Circle() { Reset(); }

	/*! \label_constructor
		\param center			The center of the circle.
		\param radius			The radius of the circle. */
	Circle(const Vec3f& center, const KyFloat32 radius) { Setup(center, radius); }

	/*! \label_constructor Copies the state of the specified Circle to the newly created object. */
	Circle(const Circle& circle) { *this = circle; }

	/*! Copies the state of the specified Circle to this object. */
	Circle& operator=(const Circle& other)
	{
		m_center  = other.m_center;
		m_radius  = other.m_radius;
		m_isValid = other.m_isValid;
		return *this;
	}

	/*! Indicates whether or not this object has been successfully set up and is ready for use. */
	KyBool IsValid() const {return m_isValid;}

	/*! Sets the dimensions of this circle.
		\param center			The center of the circle.
		\param radius			The radius of the circle. */
	KyResult Setup(const Vec3f& center, const KyFloat32 radius);

	/*! \label_reinit */
	void Reset()
	{
		m_center = Vec3f::Zero();
		m_radius = 0.0f;
		m_isValid = KY_FALSE;
	}

	/*! Retrieves the center of the circle. */
	const Vec3f& GetCenter() const {return m_center;}

	/*! Retrieves the radius of the circle. */
	KyFloat32 GetRadius()    const {return m_radius;}

	/*! Indicates whether or not the specified position is inside the area occupied by the circle, ignoring altitude. */
	KyBool IsInside2d(const Vec3f& point) const;


protected:
	Vec3f m_center;
	KyFloat32 m_radius;
	KyBool m_isValid;
};

} //namespace Kaim

#endif // KY_CIRCLE_H
