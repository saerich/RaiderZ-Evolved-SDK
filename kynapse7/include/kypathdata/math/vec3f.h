/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: GUAL - secondary contact: LAPA
#ifndef KyPathData_Vec3f_H
#define KyPathData_Vec3f_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/generic/macros.h"
#include "kypathdata/blob/endianness.h"
#include "kypathdata/math/vec2f.h"
#include <math.h>


namespace Kaim
{


/*!	This class defines a three-dimensional vector whose coordinates are stored using
	floating-point numbers.
	\pp This class is frequently used by all \SDKName components to refer to specific points in
	three-dimensional space.
	\ingroup kypathdata_math */
class Vec3f
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates a vector with coordinates (0,0,0). */
	Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}
	
	/*!	\label_constructor
		\param _x			The size of the vector on the X axis.
		\param _y			The size of the vector on the Y axis.
		\param _z			The size of the vector on the Z axis. */
	Vec3f(KyFloat32 _x, KyFloat32 _y, KyFloat32 _z) : x(_x), y(_y), z(_z) {}
	
	/*!	\label_constructor
		\param coords		An array of three floating-point numbers that contains the size of the
							vector along the X, Y and Z axes respectively. */
	explicit Vec3f(KyFloat32* coords) { Set(coords); }

	/*!	Sets the coordinates of the vector to match the specified values.
		\param _x			The size of the vector on the X axis.
		\param _y			The size of the vector on the Y axis.
		\param _z			The size of the vector on the Z axis. */
	KY_FORCE_INLINE void Set(KyFloat32 _x, KyFloat32 _y, KyFloat32 _z) { x = _x; y = _y; z = _z; }
	
	/*!	Sets the coordinates of the vector to match the specified values.
		\param coords		An array of three floating-point numbers that contains the size of the
							vector along the X, Y and Z axes respectively. */
	KY_FORCE_INLINE void Set(KyFloat32* coords) { x = coords[0]; y = coords[1]; z = coords[2]; }

	/*!	Retrieves the size of the vector around one of its axes. Use [0] for the X axis, [1] for
		the Y axis, or [2] for the Z axis. */
	KY_FORCE_INLINE KyFloat32& operator[](KyInt32 i) { return (&x)[i]; }
	
	/*!	Retrieves the size of the vector around one of its axes. Use [0] for the X axis, [1] for
		the Y axis, or [2] for the Z axis. */
	KY_FORCE_INLINE KyFloat32 operator[](KyInt32 i) const { return (&x)[i]; }

	/*!	Returns \c true if this object contains the same coordinates as \c v. */
	KY_FORCE_INLINE bool operator==(const Vec3f& v) const { return x == v.x && y == v.y && z == v.z; }
	
	/*!	Returns \c true if this object contains at least one different coordinate from \c v. */
	KY_FORCE_INLINE bool operator!=(const Vec3f& v) const { return x != v.x || y != v.y || z != v.z; }
	
	/*!	Returns \c true if the size of this vector along all three of the X, Y and Z axes is less than that of \c v. */
	KY_FORCE_INLINE bool operator<(const Vec3f& v) const;

	/*!	Multiplies the X, Y and Z coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec3f& operator*=(KyFloat32 s) { x *= s; y *= s; z *= s; return *this; }
	
	/*!	Divides the X, Y and Z coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec3f& operator/=(KyFloat32 d) { return operator*=(1.0f / d); }

	/*!	Adds the X coordinate of \c v to the X coordinate of this vector, adds the Y coordinate of \c v
		to the Y coordinate of this vector, and adds the Z coordinate of \c v to the Z coordinate of this vector. */
	KY_FORCE_INLINE Vec3f& operator+=(const Vec3f& v) { x += v.x; y += v.y; z += v.z; return *this; }

	/*!	Subtracts the X coordinate of \c v from the X coordinate of this vector, subtracts the Y coordinate of \c v
		from the Y coordinate of this vector, and subtracts the Z coordinate of \c v from the Z coordinate of this vector. */
	KY_FORCE_INLINE Vec3f& operator-=(const Vec3f& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

	/*!	Multiplies the X, Y and Z coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec3f operator*(KyFloat32 s) const { return Vec3f(x * s, y * s, z * s); }

	/*!	Divides the X, Y and Z coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec3f operator/(KyFloat32 d) const { return operator*(1.0f / d); }

	/*!	Adds the X coordinate of \c v to the X coordinate of this vector, adds the Y coordinate of \c v
		to the Y coordinate of this vector, and adds the Z coordinate of \c v to the Z coordinate of this vector. */
	KY_FORCE_INLINE Vec3f operator+(const Vec3f& v) const { return Vec3f(x + v.x, y + v.y, z + v.z); }

	/*!	Subtracts the X coordinate of \c v from the X coordinate of this vector, subtracts the Y coordinate of \c v
		from the Y coordinate of this vector, and subtracts the Z coordinate of \c v from the Z coordinate of this vector. */
	KY_FORCE_INLINE Vec3f operator-(const Vec3f& v) const { return Vec3f(x - v.x, y - v.y, z - v.z); }

	/*!	Negates the X, Y and Z coordinates of this vector, effectively flipping it around the origin. */
	KY_FORCE_INLINE Vec3f operator-() const { return Vec3f(-x, -y, -z); }

	/*!	Returns the dot product of this vector and \c v. */
	KY_FORCE_INLINE KyFloat32 operator*(const Vec3f& v) const { return x * v.x + y * v.y + z * v.z; } // DOT PRODUCT

	/*!	Returns the cross product of this vector and \c v. */
	KY_FORCE_INLINE Vec3f operator^(const Vec3f& v) const { return Vec3f( (y * v.z - z * v.y), (z * v.x - x * v.z), (x * v.y - y * v.x) ); } // CROSS PRODUCT


	//////////////////////////////////////////////////////////////////////////
	// Vec3f +/- operation with Vec2f
	//////////////////////////////////////////////////////////////////////////

	/*!	Adds the X coordinate of \c v to the X coordinate of this vector
		and the Y coordinate of \c v to the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec3f& operator+=(const Vec2f& v) { x += v.x; y += v.y; return *this; }

	/*!	Subtracts the X coordinate of \c v from the X coordinate of this vector
		and the Y coordinate of \c v from the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec3f& operator-=(const Vec2f& v) { x -= v.x; y -= v.y; return *this; }

	/*!	Adds the X coordinate of \c v to the X coordinate of this vector
		and the Y coordinate of \c v to the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec3f operator+(const Vec2f& v) const { return Vec3f(x + v.x, y + v.y, z); }

	/*!	Subtracts the X coordinate of \c v from the X coordinate of this vector
	and the Y coordinate of \c v from the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec3f operator-(const Vec2f& v) const { return Vec3f(x - v.x, y - v.y, z); }


	/*!	Calculates the cross product of \c u and \c v, and stores the result in this vector. */
	KY_FORCE_INLINE void Cross(const Vec3f& u, const Vec3f& v);

	/*!	Returns the square of the magnitude of the vector. */
	KY_FORCE_INLINE KyFloat32 GetSquareLength() const { return x * x + y * y + z * z; }
	
	/*!	Returns the square of the magnitude of the vector on the plane of the (X,Y) axes. */
	KY_FORCE_INLINE KyFloat32 GetSquareLength2d() const { return x * x + y * y; }
	
	/*!	Returns the magnitude of the vector. */
	KY_FORCE_INLINE KyFloat32 GetLength() const { return sqrtf(x * x + y * y + z * z); }
	
	/*!	Returns the magnitude of the vector on the plane of the (X,Y) axes. */
	KY_FORCE_INLINE KyFloat32 GetLength2d() const { return sqrtf(x * x + y * y); }

	/*!	Normalizes the vector, making it one unit in length without changing its orientation.
		\return The previous magnitude of this vector before normalization. */
	KY_FORCE_INLINE KyFloat32 Normalize();
	
	/*!	Normalizes the vector, making it one unit in length without changing its orientation.
		This method does not change this object. Instead, it stores the computed normal in 
		the \c normalized parameter.
		\param[out] normalized			Stores the normalized vector computed from this vector.
		\return The magnitude of this vector before normalization. */
	KY_FORCE_INLINE KyFloat32 GetNormalized(Vec3f& normalized) const;

	KY_FORCE_INLINE bool IsNormalized() const;

	/*!	Returns a Vec2f that contains the X and Y coordinates of this vector. The Z coordinate is ignored. */
	KY_FORCE_INLINE Vec2f Get2d() const { return Vec2f(x, y); }
	
	/*!	Normalizes the (X,Y) coordinates of the vector, making it one unit in length without changing its orientation. 
		This method ignores the Z coordinate of the vector. This method does not change this object. Instead, it 
		stores the computed normal in the \c normalized parameter.
		\param[out] normalized			Stores the normalized vector computed from X and Y coordinates of this vector.
		\return The magnitude of the vector on the plane of the (X,Y) axes before normalization. */
	KY_FORCE_INLINE KyFloat32 GetNormalized2d(Vec2f& normalized) const;

	/*!	Returns \c true if the X, Y and Z coordinates of this vector are all 0. */
	KY_FORCE_INLINE bool IsZero() const { return !((x!=0.0f) || (y!=0.0f) || (z!=0.0f));}

	/*!	Returns a vector of zero size: (0,0,0). */
	static KY_FORCE_INLINE Vec3f Zero() { return Vec3f(0.0f, 0.0f, 0.0f); }
	
	/*!	Returns the normalized orientation of the X axis. */
	static KY_FORCE_INLINE Vec3f UnitX() { return Vec3f(1.0f, 0.0f, 0.0f); }
	
	/*!	Returns the normalized orientation of the Y axis. */
	static KY_FORCE_INLINE Vec3f UnitY() { return Vec3f(0.0f, 1.0f, 0.0f); }
	
	/*!	Returns the normalized orientation of the Z axis. */
	static Vec3f UnitZ() { return Vec3f(0.0f, 0.0f, 1.0f); }

public:
	KyFloat32 x; /*!< The size of the vector along the X axis. */
	KyFloat32 y; /*!< The size of the vector along the Y axis. */
	KyFloat32 z; /*!< The size of the vector along the Z axis. */
};

/*!	\label_swapendianness 
	\ingroup kypathdata_math */
KY_FORCE_INLINE void SwapEndianness(Endianness::Target e, Vec3f& self)
{
	SwapEndianness(e, self.x);
	SwapEndianness(e, self.y);
	SwapEndianness(e, self.z);
}


// ----------------------------------- global functions -----------------------------------
/*!	Multiplies the X, Y and Z coordinates of \c v by \c s. \ingroup kypathdata_math */
KY_FORCE_INLINE Vec3f operator*(KyFloat32 s, const Vec3f& v) { return Vec3f(v.x * s, v.y * s, v.z * s); }

/*!	Returns the dot product of \c v1 and \c v2. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 DotProduct(const Vec3f& v1, const Vec3f& v2) { return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z); }

/*!	Returns the dot product of \c v1 and \c v2, ignoring Z coordinates. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 DotProduct2d(const Vec3f& v1, const Vec3f& v2) { return (v1.x * v2.x + v1.y * v2.y); }

/*!	Returns the X component of the cross product of \c v1 and \c v2. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 CrossProduct_x(const Vec3f& v1, const Vec3f& v2) { return v1.y * v2.z - v1.z * v2.y; }

/*!	Returns the Y component of the cross product of \c v1 and \c v2. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 CrossProduct_y(const Vec3f& v1, const Vec3f& v2) { return v1.z * v2.x - v1.x * v2.z; }

/*!	Returns the Z component of the cross product of \c v1 and \c v2. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 CrossProduct_z(const Vec3f& v1, const Vec3f& v2) { return v1.x * v2.y - v1.y * v2.x; }

/*!	Returns the cross product of \c v1 and \c v2. \ingroup kypathdata_math */
KY_FORCE_INLINE Vec3f CrossProduct(const Vec3f& v1, const Vec3f& v2) { return Vec3f(CrossProduct_x(v1, v2), CrossProduct_y(v1, v2), CrossProduct_z(v1, v2)); }

/*!	Returns the square of the distance between \c v1 and \c v2. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 SquareDistance(const Vec3f& v1, const Vec3f& v2) { return (v2 - v1).GetSquareLength(); }

/*!	Returns the square of the distance between \c v1 and \c v2, ignoring Z coordinates. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 SquareDistance2d(const Vec3f& v1, const Vec3f& v2)
{
	const KyFloat32 dx = v2.x - v1.x;
	const KyFloat32 dy = v2.y - v1.y;
	return dx * dx + dy * dy;
}

/*!	Returns the distance between \c v1 and \c v2. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 Distance(const Vec3f& v1, const Vec3f& v2) { return sqrtf(SquareDistance(v1, v2)); }

/*!	Returns the distance between \c v1 and \c v2, ignoring Z coordinates. \ingroup kypathdata_math */
KY_FORCE_INLINE KyFloat32 Distance2d(const Vec3f& v1, const Vec3f& v2) { return sqrtf(SquareDistance2d(v1, v2)); }



// ----------------------------------- KY_FORCE_INLINE implementation -----------------------------------

KY_FORCE_INLINE bool Vec3f::operator<(const Vec3f& v) const
{
	if (x < v.x) return true;
	if (x > v.x) return false;
	if (y < v.y) return true;
	if (y > v.y) return false;
	return (z < v.z);
}


KY_FORCE_INLINE void Vec3f::Cross(const Vec3f& u, const Vec3f& v)
{
	x = u.y * v.z - u.z * v.y;
	y = u.z * v.x - u.x * v.z;
	z = u.x * v.y - u.y * v.x;
}


KY_FORCE_INLINE KyFloat32 Vec3f::Normalize()
{
	const KyFloat32 length = GetLength();
	if (length != 0.0f)
		(*this) /= length;
	return length;
}


KY_FORCE_INLINE KyFloat32 Vec3f::GetNormalized(Vec3f& normalized) const
{
	// don't call Vec3f::Normalize() to avoid LHS
	const KyFloat32 length = GetLength();
	if (length != 0.0f)
	{
		const KyFloat32 invLength = 1.f / length;
		normalized.Set(x * invLength, y * invLength, z * invLength);
		return length;
	}
	else
	{
		normalized.Set(0.f, 0.f, 0.f);
		return 0.f;
	}
}

KY_FORCE_INLINE bool Vec3f::IsNormalized() const
{
	const KyFloat32 sqLength = GetSquareLength();
	return (fabsf(sqLength - 1.0f) < 1e-6f);
}

KY_FORCE_INLINE KyFloat32 Vec3f::GetNormalized2d(Vec2f& normalized2d) const
{
	// don't call Vec2f::Normalize() to avoid LHS
	const KyFloat32 length = GetLength2d();
	if (length != 0.0f)
	{
		const KyFloat32 invLength = 1.f / length;
		normalized2d.Set(x * invLength, y * invLength);
		return length;
	}
	else
	{
		normalized2d.Set(0.f, 0.f);
		return 0.f;
	}
}

template <typename T> class List;

typedef List<Vec3f> Vec3fList;


} // namespace Kaim

#endif
