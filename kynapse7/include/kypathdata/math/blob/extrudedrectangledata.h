/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_EXTRUDEDRECTANGLEDATA_H
#define KY_EXTRUDEDRECTANGLEDATA_H

#include <kypathdata/math/vec3f.h>


namespace Kaim
{

class ExtrudedRectangleData
{
public:
	ExtrudedRectangleData() { Reset(); }
	ExtrudedRectangleData(const ExtrudedRectangleData& rectangleData) { *this = rectangleData; }
	ExtrudedRectangleData(const Vec3f& baseCenter, const Vec3f& mainAxis, KyFloat32 length, KyFloat32 width, KyFloat32 height):
		m_baseCenter(baseCenter),
		m_mainAxis(mainAxis),
		m_length(length),
		m_width(width),
		m_height(height) {}

	ExtrudedRectangleData& operator=(const ExtrudedRectangleData& other)
	{
		m_baseCenter = other.m_baseCenter;
		m_mainAxis   = other.m_mainAxis;
		m_secondAxis = other.m_secondAxis;
		m_length     = other.m_length;
		m_width      = other.m_width;
		m_height     = other.m_height;
		return *this;
	}

	void Reset()
	{
		m_baseCenter = Vec3f::Zero();
		m_mainAxis = Vec3f::UnitX();
		m_length = 0.0f;
		m_width = 0.0f;
		m_height = 0.0f;
	}


public:
	Vec3f m_baseCenter;
	Vec3f m_mainAxis;
	Vec3f m_secondAxis;
	KyFloat32 m_length;
	KyFloat32 m_width;
	KyFloat32 m_height;
};

inline void SwapEndianness(Endianness::Target e, ExtrudedRectangleData& self)
{
	SwapEndianness(e, self.m_baseCenter);
	SwapEndianness(e, self.m_mainAxis);
	SwapEndianness(e, self.m_secondAxis);
	SwapEndianness(e, self.m_length);
	SwapEndianness(e, self.m_width);
	SwapEndianness(e, self.m_height);
}

} //namespace Kaim

#endif // KY_EXTRUDEDRECTANGLEDATA_H
