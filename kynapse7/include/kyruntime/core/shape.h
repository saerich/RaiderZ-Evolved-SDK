/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SHAPE_H
#define KY_SHAPE_H

#include <kypathdata/generic/memory.h>

namespace Kaim
{

////////////////////////////////////////////////////////////////////////////////////////
//		Shape 
////////////////////////////////////////////////////////////////////////////////////////

	/*!	The Shape class maintains a bounding box in two or three dimensions. 
		\pp An instance of this class is maintained by each Body, accessible by calling Body::GetShape(). You use this Shape
		object to indicate the 3D extents of the corresponding character in your \gameOrSim engine.
		\pp Every Shape needs to have a width value at a minimum. It may also have length and height values. 
		\ingroup kyruntime_core */
	class Shape
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		/*! \label_constructor Sets a default width of 1.0f, no length and no height. */
		Shape() : m_width(1.f), m_length(0.f), m_height(0.f)
		{
		}

	public:
		/*! Sets the width of the Shape to the specified value. Always use meters. */
		void SetWidth(KyFloat32 width)
		{
			KY_FUNCTION("Shape::SetWidth");
			KY_ASSERT(width > 0.f, ("Body Width must be set to a value strictly greater than zero. Its value will remain unchanged."));
			m_width = (width > 0.f ? width : m_width);
		}

		/*! Sets the height of the Shape to the specified value. Always use meters. */
		void SetHeight(KyFloat32 height)
		{
			KY_FUNCTION("Shape::SetHeight");
			KY_ASSERT(height >= 0.f, ("Body Height must be set to a value greater than zero. Its value will remain unchanged."));
			m_height = (height >= 0.f ? height : m_height);
		}

		/*! Sets the length of the Shape to the specified value. Always use meters. */
		void SetLength(KyFloat32 length)
		{
			KY_FUNCTION("Shape::SetLength");
			KY_ASSERT(length >= 0.f, ("Body Length must be set to a value greater than zero. Its value will remain unchanged."));
			m_length = (length >= 0.f ? length : m_length);
		}

	public:
		/*! Retrieves the width of the Shape. */
		KyFloat32 GetWidth() const { return m_width; }

		/*! Retrieves the height of the Shape. */
		KyFloat32 GetHeight() const { return m_height; }

		/*! Retrieves the length of the Shape. */
		KyFloat32 GetLength() const { return m_length; }

		/*! Retrieves the height of the Shape, if set. If no height value has been set, retrieves the width value instead. */
		KyFloat32 GetDefaultedHeight() const { return m_height!=0.f ? m_height : m_width; }

		/*! Retrieves the length of the Shape, if set. If no height value has been set, retrieves the width value instead. */
		KyFloat32 GetDefaultedLength() const { return m_length!=0.f ? m_length : m_width; }

		/*! Retrieves the radius of the Shape: half of its width value. */
		KyFloat32 GetRadius() const { return m_width * 0.5f; }

		/*! Returned by a call to GetFigure() to indicate the geometrical figure that summarizes a Shape, depending on whether
			or not the Shape has length and height values set. */
		enum Figure
		{
			Circle = 0, /*!< When the Shape has a width value but no length or height, it is interpreted as a horizontal circle. */
			Rectangle = 1, /*!< When the Shape has width and length values but no height, it is interpreted as a rectangle. */
			Cylinder = 2, /*!< When the Shape has width and height values but no length, it is interpreted as a cylinder. */
			Box = 3 /*!< When the Shape has width, length and height values, it is interpreted as a 3D box. */
		};

		/*! Returns an element of the #Figure enumeration that indicates the geometrical figure that summarizes the Shape,
			depending on whether or not the Shape has length and height values set.*/
		Figure GetFigure() const
		{
			bool hasHeight = m_height != 0.f;
			return (m_length == 0.0f ?
				(hasHeight ? Cylinder : Circle) :
				(hasHeight ? Box : Rectangle) );
		}

		/*! Returns #KY_TRUE if no height value is set for the Shape, or #KY_FALSE otherwise. */
		bool IsPlane() const { return m_height == 0.f; }

		/*! Returns #KY_TRUE if no length value is set for the Shape, or #KY_FALSE otherwise. */
		bool IsCircular() const { return m_length == 0.f; }

	public:
		/*! Indicates whether or not the Shape is set up with valid width, height and length values. */
		bool IsValid() const
		{
			return GetWidth() > 0.f && GetHeight() >= 0.f && GetLength() >= 0.f;
		}

	private:
		KyFloat32 m_width;
		KyFloat32 m_length;
		KyFloat32 m_height;	
	};
} // namespace Kaim

#endif // KY_SHAPE_H
