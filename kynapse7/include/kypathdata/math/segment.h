/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SEGMENT_H
#define KY_SEGMENT_H

#include <kypathdata/math/vec3f.h>


namespace Kaim
{

/*! Represents a straight-line segment between two points.
	\ingroup kypathdata_math */
class Segment
{
public:
	/*! \label_constructor If you use this constructor, you must call Setup() to set the positions of the start
		and end points of the segment before it can be used. */
	Segment() { Reset(); }

	/*! \label_constructor
		\param start			The starting point of the line segment.
		\param end				The ending point of the line segment. */
	Segment(const Vec3f& start, const Vec3f& end) { Setup(start, end); }

	/*! \label_constructor Copies the state of the specified Segment to the newly created object. */
	Segment(const Segment& segment) { *this = segment; }

	/*! Copies the state of the specified Segment to this object. */
	Segment& operator=(const Segment& other)
	{
		m_start   = other.m_start;
		m_end     = other.m_end;
		m_isValid = other.m_isValid;
		return *this;
	}

	/*! Indicates whether or not this object has been successfully set up and is ready for use. */
	KyBool IsValid() const {return m_isValid;}

	/*! Sets the starting and ending points that define this line segment.
		\param start			The starting point of the line segment.
		\param end				The ending point of the line segment. */
	KyResult Setup(const Vec3f& start, const Vec3f& end);

	/*! \label_reinit. */
	void Reset()
	{
		m_start = Vec3f::Zero();
		m_end = Vec3f::Zero();
		m_isValid = KY_FALSE;
	}

	/*! Retrieves the starting point of the line segment. */
	const Vec3f& GetStart() const {return m_start;}

	/*! Retrieves the ending point of the line segment. */
	const Vec3f& GetEnd()   const {return m_end;}


protected:
	Vec3f m_start;
	Vec3f m_end;
	KyBool m_isValid;
};

} //namespace Kaim

#endif // KY_SEGMENT_H
