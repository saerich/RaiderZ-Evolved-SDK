/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_VERTICALCYLINDERDATA_H
#define KY_VERTICALCYLINDERDATA_H

#include <kypathdata/math/vec3f.h>


namespace Kaim
{

class VerticalCylinderData
{
public:
	VerticalCylinderData() { Reset(); }
	VerticalCylinderData(const VerticalCylinderData& cylinderData) { *this = cylinderData; }
	VerticalCylinderData(const Vec3f& baseCenter, const KyFloat32 radius, const KyFloat32 height):
		m_baseCenter(baseCenter),
		m_radius(radius),
		m_height(height) {}

	VerticalCylinderData& operator=(const VerticalCylinderData& other)
	{
		m_baseCenter = other.m_baseCenter;
		m_radius     = other.m_radius;
		m_height     = other.m_height;
		return *this;
	}

	void Reset()
	{
		m_baseCenter = Vec3f::Zero();
		m_radius = 0.0f;
		m_height = 0.0f;
	}


public:
	Vec3f m_baseCenter;
	KyFloat32 m_radius;
	KyFloat32 m_height;
};

inline void SwapEndianness(Endianness::Target e, VerticalCylinderData& self)
{
	SwapEndianness(e, self.m_baseCenter);
	SwapEndianness(e, self.m_radius);
	SwapEndianness(e, self.m_height);
}

} //namespace Kaim

#endif // KY_VERTICALCYLINDERDATA_H
