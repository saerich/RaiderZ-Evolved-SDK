/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_VERTICALCYLINDER_H
#define KY_VERTICALCYLINDER_H

#include <kypathdata/math/blob/verticalcylinderdata.h>


namespace Kaim
{

/*!	Represents a cylinder whose central axis is aligned with the vertical Z axis.
	\ingroup kypathdata_math */
class VerticalCylinder
{
public:
	/*! \label_constructor If you use this constructor, you must call Setup() to set the dimensions of the cylinder
		before it can be used. */
	VerticalCylinder() { Reset(); }

	/*! \label_constructor
		\param baseCenter			The center of the base of the cylinder.
		\param radius				The radius of the cylinder.
		\param height				The vertical height of the cylinder. */
	VerticalCylinder(const Vec3f& baseCenter, const KyFloat32 radius, const KyFloat32 height) { Setup(baseCenter, radius, height); }

	/*! \label_constructor Copies the state of the specified VerticalCylinder to the newly created object. */
	VerticalCylinder(const VerticalCylinder& cylinder) { *this = cylinder; }

	/*! Copies the state of the specified VerticalCylinder to this object. */
	VerticalCylinder& operator=(const VerticalCylinder& other)
	{
		m_data    = other.m_data;
		m_isValid = other.m_isValid;
		return *this;
	}

	/*! Indicates whether or not this object has been successfully set up and is ready for use. */
	KyBool IsValid() const {return m_isValid;}

	/*! Sets the dimensions that define this cylinder.
		\param baseCenter			The center of the base of the cylinder.
		\param radius				The radius of the cylinder.
		\param height				The vertical height of the cylinder. */
	KyResult Setup(const Vec3f& baseCenter, const KyFloat32 radius, const KyFloat32 height);

	/*! \label_reinit */
	void Reset()
	{
		m_data.Reset();
		m_isValid = KY_FALSE;
	}

	/*! Retrieves the center of the base of the cylinder. */
	const Vec3f& GetBaseCenter() const { return m_data.m_baseCenter; }

	/*! Retrieves the radius of the cylinder. */
	KyFloat32 GetRadius() const { return m_data.m_radius; }

	/*! Retrieves the height of the cylinder. */
	KyFloat32 GetHeight() const { return m_data.m_height; }

	/*! Indicates whether or not the X and Y coordinates of the specified point lies inside the area
		occupied by the cylinder, ignoring altitude. */
	KyBool IsInside2d(const Vec3f& point) const;

	/*! Indicates whether or not the specified point lies inside the three-dimensional volume occupied by the cylinder. */
	KyBool IsInside3d(const Vec3f& point) const;


protected:
	VerticalCylinderData m_data;
	KyBool m_isValid;
};

} //namespace Kaim

#endif // KY_VERTICALCYLINDER_H
