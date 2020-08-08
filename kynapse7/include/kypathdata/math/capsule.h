/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_CAPSULE_H
#define KY_CAPSULE_H

#include <kypathdata/math/segment.h>
#include <kypathdata/math/vec3f.h>


namespace Kaim
{

/*!	Represents a two-dimensional or three-dimensional capsule.
	\ingroup kypathdata_math */
class Capsule
{
public:
	/*! \label_constructor If you use this constructor, you must call Setup() to set the dimensions of the capsule
		before it can be used. */
	Capsule() { Reset(); }

	/*! \label_constructor
		\param centralSegment	A line segment that defines the length and orientation of the cylinder that makes up the center of the capsule.
		\param radius			The radius of the cylinder and of the hemispheres at either end of the capsule. */
	Capsule(const Segment& centralSegment, const KyFloat32 radius) { Setup(centralSegment, radius); }

	/*! \label_constructor
		\param start			The starting point of the line segment that defines the length and orientation of the cylinder that makes up the center of the capsule.
		\param end				The ending point of the line segment that defines the length and orientation of the cylinder that makes up the center of the capsule.
		\param radius			The radius of the cylinder and of the hemispheres at either end of the capsule. */
	Capsule(const Vec3f& start, const Vec3f& end, const KyFloat32 radius) { Setup(Segment(start, end), radius); }

	/*! \label_constructor Copies the state of the specified Capsule to the newly created object. */
	Capsule(const Capsule& capsule) { *this = capsule; }

	/*! Copies the state of the specified Capsule to this object. */
	Capsule& operator=(const Capsule& other)
	{
		m_centralSegment = other.m_centralSegment;
		m_radius         = other.m_radius;
		m_isValid        = other.m_isValid;
		return *this;
	}

	/*! Indicates whether or not this object has been successfully set up and is ready for use. */
	KyBool IsValid() const {return m_isValid;}

	/*! Sets the dimensions of this cylinder.
		\param centralSegment	A line segment that defines the length and orientation of the cylinder that makes up the center of the capsule.
		\param radius			The radius of the cylinder and of the hemispheres at either end of the capsule. */
	KyResult Setup(const Segment& centralSegment, const KyFloat32 radius);

	/*! \label_reinit */
	void Reset()
	{
		m_centralSegment.Reset();
		m_radius = 0.0f;
		m_isValid = KY_FALSE;
	}

	/*! Retrieves the line segment that defines the length and orientation of the cylinder that makes up the center of the capsule. */
	const Segment& GetCentralSegment() const {return m_centralSegment;}

	/*! Retrieves radius of the cylinder and of the hemispheres at either end of the capsule. */
	KyFloat32 GetRadius()              const {return m_radius;}


protected:
	Segment m_centralSegment;
	KyFloat32 m_radius;
	KyBool m_isValid;
};

} //namespace Kaim

#endif // KY_CAPSULE_H
