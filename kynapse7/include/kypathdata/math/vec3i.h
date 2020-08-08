/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_Vec3i_H
#define KyPathData_Vec3i_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/blob/endianness.h"
#include "kypathdata/math/vec2i.h"


namespace Kaim
{


/*!	This class defines a three-dimensional vector whose coordinates are stored using
	32-bit integers.
	\ingroup kypathdata_math */
class Vec3i
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates a vector with coordinates (0,0,0). */
	Vec3i() : x(0), y(0), z(0) {}
	
	/*!	\label_constructor
		\param _x			The size of the vector on the X axis.
		\param _y			The size of the vector on the Y axis.
		\param _z			The size of the vector on the Z axis. */
	Vec3i(KyInt32 _x, KyInt32 _y, KyInt32 _z) : x(_x), y(_y), z(_z) {}
	
	/*!	\label_constructor
		\param coords		An array of three integers that contains the size of the
							vector along the X, Y and Z axes respectively. */
	explicit Vec3i(KyInt32* coords) { Set(coords); }

	/*!	Sets the coordinates of the vector to match the specified values.
		\param _x			The size of the vector on the X axis.
		\param _y			The size of the vector on the Y axis.
		\param _z			The size of the vector on the Z axis. */
	KY_FORCE_INLINE void Set(KyInt32 _x, KyInt32 _y, KyInt32 _z) { x = _x; y = _y; z = _z; }
	
	/*!	Sets the coordinates of the vector to match the specified values.
		\param v			A two-dimensional vector that specifies the size on the X and Y axes.
		\param _z			The size of the vector on the Z axis. */
	KY_FORCE_INLINE void Set(const Vec2i& v, KyInt32 _z) { x = v.x; y = v.y; z = _z; }
	
	/*!	Sets the coordinates of the vector to match the specified values.
		\param coords		An array of three floating-point numbers that contains the size of the
							vector along the X, Y and Z axes respectively. */
	KY_FORCE_INLINE void Set(KyInt32* coords) { x = coords[0]; y = coords[1]; z = coords[2]; }

	/*!	Retrieves the size of the vector around one of its axes. Use [0] for the X axis, [1] for
		the Y axis, or [2] for the Z axis. */
	KY_FORCE_INLINE KyInt32& operator[](KyInt32 i) { return (&x)[i]; }
	
	/*!	Retrieves the size of the vector around one of its axes. Use [0] for the X axis, [1] for
		the Y axis, or [2] for the Z axis. */
	KY_FORCE_INLINE KyInt32 operator[](KyInt32 i) const { return (&x)[i]; }

	/*!	Returns \c true if this object contains the same coordinates as \c v. */
	KY_FORCE_INLINE bool operator==(const Vec3i& v) const { return x == v.x && y == v.y && z == v.z; }
	
	/*!	Returns \c true if this object contains at least one different coordinate from \c v. */
	KY_FORCE_INLINE bool operator!=(const Vec3i& v) const { return x != v.x || y != v.y || z != v.z; }

	/*!	Returns \c true if the size of this vector along all three of the X, Y and Z axes is less than that of \c v. */
	bool operator<(const Vec3i& v) const
	{
		if (x < v.x) return true;
		if (x > v.x) return false;
		if (y < v.y) return true;
		if (y > v.y) return false;
		return (z < v.z);
	}

	/*!	Multiplies the X, Y and Z coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec3i& operator*=(KyInt32 s) { x *= s; y *= s; z *= s; return *this; }
	
	/*!	Divides the X, Y and Z coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec3i& operator/=(KyInt32 d) { x /= d; y /= d; z /= d; return *this; }
	
	/*!	Adds the X coordinate of \c v to the X coordinate of this vector, adds the Y coordinate of \c v
		to the Y coordinate of this vector, and adds the Z coordinate of \c v to the Z coordinate of this vector. */
	KY_FORCE_INLINE Vec3i& operator+=(const Vec3i& v) { x += v.x; y += v.y; z += v.z; return *this; }
	
	/*!	Subtracts the X coordinate of \c v from the X coordinate of this vector, subtracts the Y coordinate of \c v
		from the Y coordinate of this vector, and subtracts the Z coordinate of \c v from the Z coordinate of this vector. */
	KY_FORCE_INLINE Vec3i& operator-=(const Vec3i& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

	/*!	Multiplies the X, Y and Z coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec3i operator*(KyInt32 s) const { return Vec3i(x * s, y * s, z * s); }
	
	/*!	Divides the X, Y and Z coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec3i operator/(KyInt32 d) const { return Vec3i(x / d, y / d, z / d); }
	
	/*!	Adds the X coordinate of \c v to the X coordinate of this vector, adds the Y coordinate of \c v
		to the Y coordinate of this vector, and adds the Z coordinate of \c v to the Z coordinate of this vector. */
	KY_FORCE_INLINE Vec3i operator+(const Vec3i& v) const { return Vec3i(x + v.x, y + v.y, z + v.z); }
	
	/*!	Subtracts the X coordinate of \c v from the X coordinate of this vector, subtracts the Y coordinate of \c v
		from the Y coordinate of this vector, and subtracts the Z coordinate of \c v from the Z coordinate of this vector. */
	KY_FORCE_INLINE Vec3i operator-(const Vec3i& v) const { return Vec3i(x - v.x, y - v.y, z - v.z); }
	
	/*!	Negates the X, Y and Z coordinates of this vector, effectively flipping it around the origin. */
	KY_FORCE_INLINE Vec3i operator-() const { return Vec3i(-x, -y, -z); }

	/*!	Returns the dot product of this vector and \c v. */
	KY_FORCE_INLINE KyInt32 operator*(const Vec3i& v) const { return x * v.x + y * v.y + z * v.z; } // DOT PRODUCT
	
	/*!	Returns the cross product of this vector and \c v. */
	KY_FORCE_INLINE Vec3i operator^(const Vec3i& v) const { return Vec3i( (y * v.z - z * v.y), (z * v.x - x * v.z), (x * v.y - y * v.x) ); } // CROSS PRODUCT

	/*!	Indicates whether or not the polyline formed by ( \c A to \c B to this vector ) outlines a triangle in
		counter-clockwise order when seen from above.
		\return	Twice the area of the triangle outlined by ( \c A to \c B to this vector ).
				-	If this value is positive, ( \c A to \c B to this vector ) outlines a triangle in counter-clockwise order.
				-	If this value is negative, ( \c A to \c B to this vector ) outlines a triangle in clockwise order. 
				-	If this value is 0, then \c A, \c B and this vector are all colinear. */
	KY_FORCE_INLINE KyInt32 Orient2d(const Vec3i& A, const Vec3i& B) const { return (A.x - x) * (B.y - y) - (B.x - x) * (A.y - y); }

	/*!	Returns \c true if this vector is on the left side when moving from \c A to \c B, or if this vector
		is on the line between \c A and \c B. */
	KY_FORCE_INLINE bool IsOnLeftSide(const Vec3i& A, const Vec3i& B) const { return Orient2d(A, B) >= 0; }

	/*!	Returns \c true if this vector is on the left side when moving from \c A to \c B, but not if this vector
		is on the line between \c A and \c B. */
	KY_FORCE_INLINE bool IsStrictlyOnLeftSide(const Vec3i& A, const Vec3i& B) const { return Orient2d(A, B) > 0; }

	/*!	Returns the square of the magnitude of the vector. */
	KY_FORCE_INLINE KyInt32 GetSquareLength() const { return x * x + y * y + z * z; }

	/*!	Returns the normalized orientation of the X axis. */
	static KY_FORCE_INLINE Vec3i UnitX() { return Vec3i(1, 0, 0); }
	
	/*!	Returns the normalized orientation of the Y axis. */
	static KY_FORCE_INLINE Vec3i UnitY() { return Vec3i(0, 1, 0); }
	
	/*!	Returns the normalized orientation of the Y axis. */
	static KY_FORCE_INLINE Vec3i UnitZ() { return Vec3i(0, 0, 1); }

public:
	KyInt32 x; /*!< The size of the vector along the X axis. */
	KyInt32 y; /*!< The size of the vector along the Y axis. */
	KyInt32 z; /*!< The size of the vector along the Z axis. */
};

/*!	\label_swapendianness 
	\ingroup kypathdata_math */
KY_FORCE_INLINE  void SwapEndianness(Endianness::Target e, Vec3i& self)
{
	SwapEndianness(e, self.x);
	SwapEndianness(e, self.y);
	SwapEndianness(e, self.z);
}


}


#endif

