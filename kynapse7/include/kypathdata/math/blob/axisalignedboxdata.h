/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_AXISALIGNEDBOXDATA_H
#define KY_AXISALIGNEDBOXDATA_H

#include <kypathdata/math/vec3f.h>


namespace Kaim
{

class AxisAlignedBoxData
{
public:
	AxisAlignedBoxData() { Reset(); }
	AxisAlignedBoxData(const AxisAlignedBoxData& aabbData) { *this = aabbData; }
	AxisAlignedBoxData(const Vec3f& aabbMin, const Vec3f& aabbMax):
		m_min(aabbMin),
		m_max(aabbMax) {}

	AxisAlignedBoxData& operator=(const AxisAlignedBoxData& other)
	{
		m_min = other.m_min;
		m_max = other.m_max;
		return *this;
	}

	void Reset()
	{
		m_min.Set(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL);
		m_max.Set(-KyFloat32MAXVAL, -KyFloat32MAXVAL, -KyFloat32MAXVAL);
	}


public:
	Vec3f m_min;
	Vec3f m_max;
};

inline void SwapEndianness(Endianness::Target e, AxisAlignedBoxData& self)
{
	SwapEndianness(e, self.m_min);
	SwapEndianness(e, self.m_max);
}

} //namespace Kaim

#endif // KY_AXISALIGNEDBOXDATA_H
