/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*!	\file
	\ingroup kypathdata_math */

#ifndef KY_INTERVAL_H
#define KY_INTERVAL_H

#include "kypathdata/generic/basetypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/blob/endianness.h"

namespace Kaim
{

/*!	Represents a range of floating-point numeric space between a lower bound and an upper bound.
	\ingroup kypathdata_math */
class Interval
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	Interval() {} // To make Array Happy
	/*! \label_constructor
		\param low			The lower bound of the range.
		\param high			The upper bound of the range. */
	Interval(KyFloat32 low, KyFloat32 high) : m_low(low), m_high(high) {}

	/*! Returns #KY_TRUE if the upper bound is lower than the lower bound. */
	bool IsEmpty() const { return m_high < m_low; }

//	Interval& operator = (const Interval& rhs) {  }

	/*! Returns #KY_TRUE if the bounds of the specified Interval are equal to the corresponding bounds of this object. */
	bool operator == (const Interval& rhs) const { return m_low == rhs.m_low && m_high == rhs.m_high;}
	/*! Returns #KY_TRUE if at least one bound of the specified Interval differs from the corresponding bound of this object. */
	bool operator != (const Interval& rhs) const { return !(*this == rhs); }
public:
	KyFloat32 m_low; /*!< Stores the lower bound of the range. */
	KyFloat32 m_high; /*!< Stores the upper bound of the range. */
};

/*!	\label_swapendianness
	\ingroup kypathdata_math */
inline void SwapEndianness(Endianness::Target e, Interval& self)
{
	SwapEndianness(e, self.m_low);
	SwapEndianness(e, self.m_high);
}

} // namespace Kaim


#endif // KY_INTERVAL_H
