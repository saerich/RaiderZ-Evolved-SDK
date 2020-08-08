/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: NOBODY
#ifndef KY_EXTRUDEDPOLYLINEDATA_H
#define KY_EXTRUDEDPOLYLINEDATA_H

#include <kypathdata/math/vec3f.h>
#include "kypathdata/blob/blobarray.h"


namespace Kaim
{

class ExtrudedPolylineData
{
	//use ExtrudedPolylineDataBuilder to build an ExtrudedPolylineData
public:
	static const char* GetBlobType() { return "ExtrudedPolylineData"; }
	static KyUInt32 GetBlobVersion() { return 0; }
	bool operator ==(const ExtrudedPolylineData& other) const
	{
		if(m_polylinePoints.m_count != other.m_polylinePoints.m_count
		|| m_altMin != other.m_altMin
		|| m_altMax != other.m_altMax) 
		{
			return false;
		}
		for (KyUInt32 i = 0 ; i < m_polylinePoints.m_count; ++i)
		{
			if(m_polylinePoints.Values()[i] !=  other.m_polylinePoints.Values()[i])
			{
				return false;
			}
		}
		return true;
	}

public:
	KyFloat32 m_altMin;
	KyFloat32 m_altMax;
	BlobArray<Vec3f> m_polylinePoints;
};

inline void SwapEndianness(Endianness::Target e, ExtrudedPolylineData& self)
{
	SwapEndianness(e, self.m_altMin);
	SwapEndianness(e, self.m_altMax);
	SwapEndianness(e, self.m_polylinePoints);
}

} //namespace Kaim

#endif // KY_EXTRUDEDPOLYLINEDATA_H
