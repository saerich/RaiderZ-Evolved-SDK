/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_RasterTriangle_H
#define KyPdg_RasterTriangle_H



#include "kypathdata/blob/endianness.h"
#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/raster/rasterpoint.h"
#include "kypathdata/math/vec3f.h"



namespace Kaim
{

class CoordSystem;
class Triangle3i;


class RasterTriangle
{
public:
	RasterTriangle();

	bool Setup(const Triangle3i& triangle);

	bool Setup(const RasterPoint& rpA, const RasterPoint& rpB, const RasterPoint& rpC);
	
	KyFloat32 meterZ(Coord x, Coord y)
	{
		KyFloat32 meter_AMx = Kaim::GetCoordSystem().IntToKynapse_Dist(x - iA.x);
		KyFloat32 meter_AMy = Kaim::GetCoordSystem().IntToKynapse_Dist(y - iA.y);
		return A.z - (meter_AMx * N.x + meter_AMy * N.y) * inv_Nz;
	}

	KyFloat32 meterStepZ_AlongX(Coord dx) { return dz_on_dx * Kaim::GetCoordSystem().IntToKynapse_Dist(dx); }

	/* Returns true if M is inside triangle or exactly on one edge. */
	bool IsInside2d(const Vec2i& M) const { return M.IsInsideTriangle(iA, iB, iC); }

	/* Returns true if M is inside triangle or exactly on one edge. We assume the triangle is not colinear in this test. */
	bool IsInside2d_NoColinear(const Vec2i& M) const { return M.IsInsideNotColinearTriangle(iA, iB, iC); }

	/* Returns true if M is strictly inside triangle. */
	bool IsStrictlyInside2d(const Vec2i& M) const { return M.IsStrictlyInsideTriangle(iA, iB, iC); }

public:
	Vec2i iA, iB, iC;

	Vec3f A;
	Vec3f AB;
	Vec3f AC;
	Vec3f N;
	KyFloat32 inv_Nz;
	KyFloat32 dz_on_dx;
};


}


#endif
