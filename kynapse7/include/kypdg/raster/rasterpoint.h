/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_RasterPoint_H
#define KyPdg_RasterPoint_H



#include "kypathdata/math/vec2i.h"
#include "kypathdata/math/vec3i.h"
#include <kypathdata/basesystem/coordsystem.h>


namespace Kaim
{


class RasterPoint : public Vec2i
{
public:
	RasterPoint() : meter_z(0.0f) {}

	explicit RasterPoint(const Vec3i& v) { Set(v); }

	void Set(const Vec3i& v)
	{
		x = v.x;
		y = v.y;
		meter_z = Kaim::GetCoordSystem().IntToKynapse_Dist(v.z);
	}

public:
	KyFloat32 meter_z;
};


}


#endif
