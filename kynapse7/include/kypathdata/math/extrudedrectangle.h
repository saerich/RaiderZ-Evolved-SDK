/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_EXTRUDEDRECTANGLE_H
#define KY_EXTRUDEDRECTANGLE_H

#include <kypathdata/math/blob/extrudedrectangledata.h>


namespace Kaim
{

/*! Represents a rectangle on the horizontal (X,Y) plane that is extruded along the vertical Z axis.
	\ingroup kypathdata_math */
class ExtrudedRectangle
{
public:
	/*! \label_constructor If you use this constructor, you must call Setup() to set the dimensions of the rectangle
		before it can be used. */
	ExtrudedRectangle() { Reset(); }

	/*! \label_constructor
		\param baseCenter		The center of the bottom face of the extruded rectangle.
		\param mainAxis			The orientation of one axis of the rectangle. This axis must not be vertical.
								If this value is not normalized, it will be normalized internally.
		\param length			The length of the rectangle in the direction of \c mainAxis.
		\param width			The width of the rectangle in the direction orthogonal to \c mainAxis.
		\param height			The distance that the rectangle is extruded along the Z axis. */
	ExtrudedRectangle(const Vec3f& baseCenter, const Vec3f& mainAxis, KyFloat32 length, KyFloat32 width, KyFloat32 height) { Setup(baseCenter, mainAxis, length, width, height); }

	/*! \label_constructor Copies the state of the specified ExtrudedRectangle to the newly created object. */
	ExtrudedRectangle(const ExtrudedRectangle& rectangle) { *this = rectangle; }

	/*! Copies the state of the specified ExtrudedRectangle to this object. */
	ExtrudedRectangle& operator=(const ExtrudedRectangle& other)
	{
		m_data = other.m_data;
		m_isValid = other.m_isValid;
		return *this;
	}

	/*! Indicates whether or not this object has been successfully set up and is ready for use. */
	KyBool IsValid() const {return m_isValid;}

	/*! Sets the dimensions of the extruded rectangle.
		\param baseCenter		The center of the bottom face of the extruded rectangle.
		\param mainAxis			The orientation of one axis of the rectangle. This axis must not be vertical.
								If this value is not normalized, it will be normalized internally.
		\param length			The length of the rectangle in the direction of \c mainAxis.
		\param width			The width of the rectangle in the direction orthogonal to \c mainAxis.
		\param height			The distance that the rectangle is extruded along the Z axis. */
	KyResult Setup(const Vec3f& baseCenter, const Vec3f& mainAxis, KyFloat32 length, KyFloat32 width, KyFloat32 height);

	/*! \label_reinit */
	void Reset()
	{
		m_data.Reset();
		m_isValid = KY_FALSE;
	}

	/*! Retrieves the center of the bottom face of the extruded rectangle. */
	const Vec3f& GetBaseCenter() const { return m_data.m_baseCenter; }

	/*! Retrieves the orientation of the main axis of the rectangle. */
	const Vec3f& GetMainAxis()   const { return m_data.m_mainAxis; }

	/*! Retrieves the orientation of the axis orthogonal to the main axis of the rectangle. */
	const Vec3f& GetSecondAxis() const { return m_data.m_secondAxis; }

	/*! Retrieves the length of the rectangle in the direction of its main axis. */
	KyFloat32 GetLength() const { return m_data.m_length; }

	/*! Retrieves the width of the rectangle in the direction orthogonal to its main axis. */
	KyFloat32 GetWidth()  const { return m_data.m_width; }

	/*! Retrieves the distance that the rectangle is extruded along the Z axis. */
	KyFloat32 GetHeight() const { return m_data.m_height; }

	/*! Indicates whether or not the specified position is inside the area occupied by the rectangle, ignoring altitude. */
	KyBool IsInside2d(const Vec3f& point) const;

	/*! Indicates whether or not the specified position is inside the three-dimensional volume occupied by the extruded rectangle. */
	KyBool IsInside3d(const Vec3f& point) const;


protected:
	ExtrudedRectangleData m_data;
	KyBool m_isValid;
};

} //namespace Kaim

#endif // KY_EXTRUDEDRECTANGLE_H
