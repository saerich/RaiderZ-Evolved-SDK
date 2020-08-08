/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_VEC3FUTILS_H
#define KY_VEC3FUTILS_H

#include <kypathdata/generic/macros.h>
#include <kypathdata/math/vec3f.h>
#include <kypathdata/generic/helpers.h>


namespace Kaim
{
/*!	Retrieves the absolute pitch value from the specified Vec3f.
	\ingroup kypathdata_math */
inline KyFloat32& AnglesGetPitch(Vec3f& vec) { return vec.x; }

/*!	Retrieves the absolute roll value from the specified Vec3f.
	\ingroup kypathdata_math */
inline KyFloat32& AnglesGetRoll(Vec3f& vec)  { return vec.y; }

/*!	Retrieves the absolute yaw value from the specified Vec3f.
	\ingroup kypathdata_math */
inline KyFloat32& AnglesGetYaw(Vec3f& vec)   { return vec.z; }

/*!	Retrieves the absolute pitch value from the specified Vec3f.
	\ingroup kypathdata_math */
inline KyFloat32 AnglesGetPitch(const Vec3f& vec) { return vec.x; }

/*!	Retrieves the absolute roll value from the specified Vec3f.
	\ingroup kypathdata_math */
inline KyFloat32 AnglesGetRoll(const Vec3f& vec)  { return vec.y; }

/*!	Retrieves the absolute yaw value from the specified Vec3f.
	\ingroup kypathdata_math */
inline KyFloat32 AnglesGetYaw(const Vec3f& vec)   { return vec.z; }


/*!	Enlarges an axis-aligned bounding box such that it will contain the specified 
	position. If the position is already contained within the bounding box, the 
	bounding box is not modified.
	\param position				The position to include within the bounding box.
	\param aabbMin				The minima of the bounding box.
	\param aabbMax				The maxima of the bounding box.
	\ingroup kypathdata_math */
inline void ExtendAabbBy(const Vec3f& position, Vec3f& aabbMin, Vec3f& aabbMax)
{
	aabbMin.x = Kaim::Min(aabbMin.x, position.x);
	aabbMin.y = Kaim::Min(aabbMin.y, position.y);
	aabbMin.z = Kaim::Min(aabbMin.z, position.z);

	aabbMax.x = Kaim::Max(aabbMax.x, position.x);
	aabbMax.y = Kaim::Max(aabbMax.y, position.y);
	aabbMax.z = Kaim::Max(aabbMax.z, position.z);
}

/*!	Indicates whether or not the specified position is inside the specified axis-aligned
	bounding box.
	\param position				The position to test.
	\param aabbMin				The minima of the bounding box.
	\param aabbMax				The maxima of the bounding box.
	\ingroup kypathdata_math */
inline bool IsInsideAabb(const Vec3f& position, const Vec3f& aabbMin, const Vec3f& aabbMax)
{
	if ((aabbMin.x > position.x) || (aabbMax.x < position.x) ||
		(aabbMin.y > position.y) || (aabbMax.y < position.y) ||
		(aabbMin.z > position.z) || (aabbMax.z < position.z))
		return false;

	return true;
}

/*!	Indicates whether or not two specified axis-aligned bounding boxes intersect each other.
	\param aabbMin_0				The minima of the first bounding box.
	\param aabbMax_0				The maxima of the first bounding box.
	\param aabbMin_1				The minima of the second bounding box.
	\param aabbMax_1				The maxima of the second bounding box.
	\ingroup kypathdata_math */
inline bool Intersect3dAabbAabb(const Vec3f& aabbMin_0, const Vec3f& aabbMax_0, const Vec3f& aabbMin_1, const Vec3f& aabbMax_1)
{
	if ((aabbMin_0.x > aabbMax_1.x) || (aabbMax_0.x < aabbMin_1.x) ||
		(aabbMin_0.y > aabbMax_1.y) || (aabbMax_0.y < aabbMin_1.y) ||
		(aabbMin_0.z > aabbMax_1.z) || (aabbMax_0.z < aabbMin_1.z))
		return false;

	return true;
}

/*!	Indicates whether or not the specified Vec3f contains valid coordinates in the \SDKName coordinate
	system. To be valid, each coordinate must be:
	-	A number (i.e. not \c NaN). 
	-	An absolute value less than \c 1e18, so that the square distance between any two
		positions is less than #KyFloat32MAXVAL.
	\ingroup kypathdata_math */
inline bool HasValidCoordinates(const Vec3f& v)
{
	if ((v.x != v.x) ||
		(v.y != v.y) ||
		(v.z != v.z))
		return false; // at least one of the coordinates is NaN

	if ((v.x > 1e18f) ||
		(v.x < -1e18f) ||
		(v.y > 1e18f) ||
		(v.y < -1e18f) ||
		(v.z > 1e18f) ||
		(v.z < -1e18f))
		return false; // at least one of the coordinates is out of the admitted range

	return true;
}

} // namespace Kaim

#endif // KY_VEC3FUTILS_H
