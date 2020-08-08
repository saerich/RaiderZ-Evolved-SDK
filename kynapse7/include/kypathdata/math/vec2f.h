/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




/*!	\file
	\ingroup kypathdata_math */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_Vec2f_H
#define KyPathData_Vec2f_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/generic/macros.h"
#include "kypathdata/blob/endianness.h"
#include <math.h>


namespace Kaim
{

/*!	This class defines a two-dimensional vector whose coordinates are stored using
	floating-point numbers.
	\ingroup kypathdata_math */
class Vec2f
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates a vector with coordinates (0,0). */
	Vec2f() : x(0.0f), y(0.0f) {}

	/*!	\label_constructor
		\param _x			The size of the vector on the X axis.
		\param _y			The size of the vector on the Y axis. */
	Vec2f(KyFloat32 _x, KyFloat32 _y) : x(_x), y(_y) {}

	/*!	\label_constructor
		\param coords		An array of two floating-point numbers that contains the size of the
							vector along the X and Y axes respectively. */
	explicit Vec2f(KyFloat32* coords) { Set(coords); }

	/*!	Sets the coordinates of the vector to match the specified values.
		\param _x			The size of the vector on the X axis.
		\param _y			The size of the vector on the Y axis. */
	KY_FORCE_INLINE void Set(KyFloat32 _x, KyFloat32 _y) { x = _x; y = _y; }

	/*!	Sets the coordinates of the vector to match the specified values.
		\param coords		An array of two floating-point numbers that contains the size of the
							vector along the X and Y axes respectively. */
	KY_FORCE_INLINE void Set(KyFloat32* coords) { x = coords[0]; y = coords[1]; }

	/*!	Retrieves the size of the vector around one of its axes. Use [0] for the X axis, or [1] for
		the Y axis. */
	KY_FORCE_INLINE KyFloat32& operator[](KyInt32 i) { return (&x)[i]; }

	/*!	Retrieves the size of the vector around one of its axes. Use [0] for the X axis, or [1] for
		the Y axis. */
	KY_FORCE_INLINE KyFloat32 operator[](KyInt32 i) const { return (&x)[i]; }

	/*!	Returns \c true if this object contains the same coordinates as \c v. */
	KY_FORCE_INLINE bool operator==(const Vec2f& v) const { return x == v.x && y == v.y; }

	/*!	Returns \c true if this object contains at least one different coordinate from \c v. */
	KY_FORCE_INLINE bool operator!=(const Vec2f& v) const { return x != v.x || y != v.y; }

	/*!	Returns \c true if the size of this vector along both the X and Y axes is less than that of \c v. */
	KY_FORCE_INLINE bool operator<(const Vec2f& v) const;

	/*!	Multiplies both the X and Y coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec2f& operator*=(KyFloat32 s)    { x *= s; y *= s;     return *this; }

	/*!	Divides both the X and Y coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec2f& operator/=(KyFloat32 d)    { return operator*=(1.0f / d);      }

	/*!	Adds the X coordinate of \c v to the X coordinate of this vector, and adds the Y coordinate of \c v
		to the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec2f& operator+=(const Vec2f& v) { x += v.x; y += v.y; return *this; }

	/*!	Subtracts the X coordinate of \c v from the X coordinate of this vector, and subtracts the Y coordinate of \c v
		from the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec2f& operator-=(const Vec2f& v) { x -= v.x; y -= v.y; return *this; }

	/*!	Multiplies both the X and Y coordinates of the vector by the specified value. */
	KY_FORCE_INLINE Vec2f operator*(KyFloat32 s)const { return Vec2f(x * s, y * s); }

	/*!	Divides both the X and Y coordinates of the vector by the specified value. */
	KY_FORCE_INLINE Vec2f operator/(KyFloat32 d) const { return operator*(1.0f / d); }

	/*!	Adds the X coordinate of \c v to the X coordinate of this vector, and adds the Y coordinate of \c v
		to the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec2f operator+(const Vec2f& v) const { return Vec2f(x + v.x, y + v.y); }

	/*!	Subtracts the X coordinate of \c v from the X coordinate of this vector, and subtracts the Y coordinate of \c v
		from the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec2f operator-(const Vec2f& v) const { return Vec2f(x - v.x, y - v.y); }

	/*!	Negates the X and Y coordinates of this vector, effectively flipping it around the origin. */
	KY_FORCE_INLINE Vec2f operator-() const { return Vec2f(-x, -y); }

	/*!	Returns the dot product of this vector and \c v. */
	KY_FORCE_INLINE KyFloat32 operator*(const Vec2f& v) const { return x * v.x + y * v.y; } // DOT PRODUCT

	/*!	Returns the magnitude on the Z axis of the cross product between this vector and \c v. */
	KY_FORCE_INLINE KyFloat32 operator^(const Vec2f& v) const { return x * v.y - y * v.x; } // CROSS PRODUCT

	/*!	Rotates this vector 90 degrees clockwise (negating the X coordinate). */
	KY_FORCE_INLINE Vec2f PerpCW() const { return Vec2f(y, -x); }

	/*!	Rotates this vector 90 degrees counter-clockwise (negating the Y coordinate). */
	KY_FORCE_INLINE Vec2f PerpCCW() const { return Vec2f(-y, x); }

	/*!	Returns the square of the magnitude of the vector. */
	KY_FORCE_INLINE KyFloat32 GetSquareLength() const { return x * x + y * y; }

	/*!	Returns the magnitude of the vector. */
	KY_FORCE_INLINE KyFloat32 GetLength() const { return sqrtf(x * x + y * y); }

	/*!	Normalizes the vector, making it one unit in length without changing its orientation.
		\return The previous magnitude of this vector before normalization. */
	KY_FORCE_INLINE KyFloat32 Normalize();

	/*!	Normalizes the vector, making it one unit in length without changing its orientation.
		This method does not change this object. Instead, it stores the computed normal in
		the \c normalized parameter.
		\param[out] normalized			Stores the normalized vector computed from this vector.
		\return The magnitude of this vector before normalization. */
	KY_FORCE_INLINE KyFloat32 GetNormalized(Vec2f& normalized) const;

	/*!	Returns the normalized orientation of the X axis. */
	static KY_FORCE_INLINE Vec2f UnitX() { return Vec2f(1.0f, 0.0f); }

	/*!	Returns the normalized orientation of the Y axis. */
	static KY_FORCE_INLINE Vec2f UnitY() { return Vec2f(0.0f, 1.0f); }

public:
	KyFloat32 x; /*!< The size of the vector along the X axis. */
	KyFloat32 y; /*!< The size of the vector along the Y axis. */
};

/*!	\label_swapendianness
	\ingroup kypathdata_math */
inline void SwapEndianness(Endianness::Target e, Vec2f& self)
{
	SwapEndianness(e, self.x);
	SwapEndianness(e, self.y);
}

// ----------------------------------- global functions -----------------------------------
/*!	Multiplies the X and Y coordinates of \c v by \c s. \ingroup kypathdata_math */
KY_FORCE_INLINE Vec2f operator*(KyFloat32 s, const Vec2f& v) { return Vec2f(v.x * s, v.y * s); }

/*!	Returns the dot product of \c v1 and \c v2. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 DotProduct(const Vec2f& v1, const Vec2f& v2) { return (v1.x * v2.x + v1.y * v2.y); }

/*! Returns the magnitude on the Z axis of the cross product between this vector and \c v. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 CrossProduct (const Vec2f& v1, const Vec2f& v2) { return v1 ^ v2; }


/*!	Returns the square of the distance between \c v1 and \c v2. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 SquareDistance(const Vec2f& v1, const Vec2f& v2)
{
	const KyFloat32 dx = v2.x - v1.x;
	const KyFloat32 dy = v2.y - v1.y;
	return dx * dx + dy * dy;
}

/*!	Returns the distance between \c v1 and \c v2. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 Distance(const Vec2f& v1, const Vec2f& v2)
{
	return sqrtf(SquareDistance(v1, v2));
}

// ----------------------------------- inline implementation -----------------------------------

KY_FORCE_INLINE  bool Vec2f::operator<(const Vec2f& v) const
{
	if (x < v.x) return true;
	if (x > v.x) return false;
	return (y < v.y);
}


KY_FORCE_INLINE KyFloat32 Vec2f::Normalize()
{
	const KyFloat32 length = GetLength();
	if (length != 0.0f)
		operator/=(length);
	return length;
}


KY_FORCE_INLINE KyFloat32 Vec2f::GetNormalized(Vec2f& normalized) const
{
	// don't call Vec2f::Normalize() to avoid LHS
	const KyFloat32 length = GetLength();
	if (length != 0.0f)
	{
		const KyFloat32 invLength = 1.f / length;
		normalized.Set(x * invLength, y * invLength);
		return length;
	}
	else
	{
		normalized.Set(0.f, 0.f);
		return 0.f;
	}
}

} // namespace Kaim

#endif
