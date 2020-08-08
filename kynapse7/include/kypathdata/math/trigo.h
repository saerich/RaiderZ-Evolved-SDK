/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




/*!	\file
	\ingroup kypathdata_math */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_TRIGO_H
#define KY_TRIGO_H

#include <kypathdata/generic/types.h>
#include <kypathdata/math/vec3f.h>


namespace Kaim
{

class Vec3f;


/*!	Stores the value of pi.
	\ingroup kypathdata_math */
static const KyFloat32 KY_PI = 3.14159265f;

/*!	Stores one half the value of #KY_PI.
	\ingroup kypathdata_math */
const KyFloat32 KY_PI_DIVIDED_BY_2 = KY_PI / 2.0f;

/*!	Stores the value of #KY_PI divided by 180.
	\ingroup kypathdata_math */
const KyFloat32 KY_PI_DIVIDED_BY_180 = KY_PI / 180.0f;

/*!	Returns the specified object or variable multiplied by itself.
	\tparam T				The type of the object or variable.
	\ingroup kypathdata_math */
template <typename T>
inline T Square(T v) { return v * v; }

/*!	Returns the cosine of the specified angle.
	\param angle			The angle, expressed in degrees.
	\ingroup kypathdata_math */
KyFloat32 CosDeg(KyFloat32 angle);

/*!	Returns the sine of the specified angle.
	\param angle			The angle, expressed in degrees.
	\ingroup kypathdata_math */
KyFloat32 SinDeg(KyFloat32 angle);

/*!	Returns the cosine of the specified angle.
	\param angle			The angle, expressed in radians.
	\ingroup kypathdata_math */
KyFloat32 CosRad(KyFloat32 angle);

/*!	Returns the sine of the specified angle.
	\param angle			The angle, expressed in radians.
	\ingroup kypathdata_math */
KyFloat32 SinRad(KyFloat32 angle);

/*!	Calculates a normalized directional vector equivalent to the specified Euler angles
	(pitch, roll, yaw).
	\param angles			A Vec3f that contains the Euler angles in the following order:
							(pitch, roll, yaw).
	\param[out] direction	Used to store the calculated directional vector.
	\ingroup kypathdata_math */
void MakeVectorDeg(const Vec3f& angles, Vec3f& direction);

/*!	Returns the normalized directional vector equivalent to the specified Euler angles
	(pitch, roll, yaw).
	\param angles			A Vec3f that contains the Euler angles in the following order:
							(pitch, roll, yaw).
	\ingroup kypathdata_math */
inline Vec3f MakeVectorDeg(const Vec3f& angles) { Vec3f direction; MakeVectorDeg(angles, direction); return direction; }

/*!	Calculates the Euler angles (pitch, roll, yaw) that are equivalent to the specified normalized directional vector.
	For example, passing Vec3f::UnitY as the direction will give \c Vec3f(0,0,0) as angles, and Vec3f::UnitX will give \c Vec3f(0,0,-90).
	\param direction		A Vec3f that contains the normalized directional vector.
	\param[out] angles		Used to store the calculated Euler angles in the following order:
							(pitch, roll, yaw).
	\ingroup kypathdata_math */
void VecToAnglesDeg(const Vec3f& direction, Vec3f& angles);

/*!	Returns the Euler angles (pitch, roll, yaw) that are equivalent to the specified normalized directional vector.
	For example, passing Vec3f::UnitY as the direction will give \c Vec3f(0,0,0) as angles, and Vec3f::UnitX will give \c Vec3f(0,0,-90).
	\param direction		A Vec3f that contains the normalized directional vector.
	\ingroup kypathdata_math */
inline Vec3f VecToAnglesDeg(const Vec3f& direction) { Vec3f angles; VecToAnglesDeg(direction, angles); return angles; }

/*!	Normalizes a specified angle expressed in degrees, so that it falls within the range ]-180,180].
	\ingroup kypathdata_math */
inline KyFloat32 GetNormalizeAngleDeg(KyFloat32 angle)
{
	return (angle > 180 ? angle - 360 : (angle <= -180 ? angle + 360 : angle));
}

/*!	Normalizes a specified angle expressed in degrees, so that it falls within the range [0,360[.
	\ingroup kypathdata_math */
inline KyFloat32 GetNormalizeAngle2Deg(KyFloat32 angle)
{
	return (angle >= 360 ? angle - 360 : (angle < 0 ? angle + 360 : angle));
}

/*! This function is due to Rob Fowler. Given dx and dy between 2 points
	A and B, we calculate a number in [0.0, 8.0[ which is a monotonic
	function of the direction from A to B.

	(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0) correspond to
	(  0,  45,  90, 135, 180, 225, 270, 315, 360) degrees, measured
	counter-clockwise from the positive x axis.
*/
KyFloat32 FowlerAngle(KyFloat32 dx, KyFloat32 dy);

inline KyFloat32 FowlerAngle(const Vec2f& v) { return FowlerAngle(v.x, v.y); }

} //namespace Kaim

#endif // KY_TRIGO_H
