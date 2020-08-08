/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_HORIZONTALRECTANGLE_H
#define KY_HORIZONTALRECTANGLE_H

#include <kypathdata/math/vec3f.h>


namespace Kaim
{

/*!	Represents a free-rotating rectangle on the horizontal (X,Y) plane.
	\ingroup kypathdata_math */
class HorizontalRectangle
{
public:
	/*! \label_constructor If you use this constructor, you must call Setup() to set the dimensions of the rectangle
		before it can be used. */
	HorizontalRectangle(): m_length(0.0f), m_width(0.0f) {}
	/*! \label_constructor
		\param center			The center of the rectangle.
		\param mainAxis			The orientation of one axis of the rectangle.
		\param length			The length of the rectangle in the direction of \c mainAxis.
		\param width			The width of the rectangle in the direction orthogonal to \c mainAxis. */
	HorizontalRectangle(const Vec3f& center, const Vec2f& mainAxis, KyFloat32 length, KyFloat32 width) { Setup(center, mainAxis, length, width); }

	/*! \label_constructor Copies the state of the specified HorizontalRectangle to the newly created object. */
	HorizontalRectangle(const HorizontalRectangle& rectangle) { *this = rectangle; }

	/*! Copies the state of the specified HorizontalRectangle to this object. */
	HorizontalRectangle& operator=(const HorizontalRectangle& other)
	{
		m_center   = other.m_center;
		m_mainAxis = other.m_mainAxis;
		m_length   = other.m_length;
		m_width    = other.m_width;
		m_isValid  = other.m_isValid;
		return *this;
	}

	/*! \label_reinit. */
	void Reset()
	{
		m_center   = Vec3f::Zero();
		m_mainAxis = Vec3f::UnitX();
		m_length   = 0.0f;
		m_width    = 0.0f;
		m_isValid  = KY_FALSE;
	}

	/*! Indicates whether or not this object has been successfully set up and is ready for use. */
	KyBool IsValid() const {return m_isValid;}

	/*! Sets the dimensions that define this rectangle.
		\param center			The center of the rectangle.
		\param mainAxis			The orientation of one axis of the rectangle.
		\param length			The length of the rectangle in the direction of the \c mainAxis.
		\param width			The width of the rectangle in the direction orthogonal to \c mainAxis. */
	KyResult Setup(const Vec3f& center, const Vec2f& mainAxis, KyFloat32 length, KyFloat32 width);

	/*! Retrieves the center of the rectangle. */
	const Vec3f& GetCenter()   const {return m_center;}

	/*! Retrieves the orientation of the main axis of the rectangle. */
	const Vec3f& GetMainAxis() const {return m_mainAxis;}

	/*! Retrieves the length of the rectangle in the direction of its main axis. */
	KyFloat32 GetLength()      const {return m_length;}

	/*! Retrieves the width of the rectangle in the direction orthogonal to its main axis. */
	KyFloat32 GetWidth()       const {return m_width;}

	/*! Indicates whether or not the specified position lies inside the area defined by the rectangle. */
	KyBool IsInside2d(const Vec3f& point) const;


protected:
	Vec3f m_center;
	Vec3f m_mainAxis;
	KyFloat32 m_length;
	KyFloat32 m_width;
	KyBool m_isValid;
};

} //namespace Kaim

#endif // KY_HORIZONTALRECTANGLE_H
