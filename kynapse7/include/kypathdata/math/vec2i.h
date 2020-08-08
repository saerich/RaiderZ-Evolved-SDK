/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_Vec2i_H
#define KyPathData_Vec2i_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/generic/macros.h"
#include "kypathdata/blob/endianness.h"
#include <kypathdata/math/cardinaldir.h>
#include <math.h>

namespace Kaim
{


/*!	This class defines a two-dimensional vector whose coordinates are stored using
	32-bit integers.
	\pp This class is typically used to identify a cell within a regular two-dimensional grid.
	\ingroup kypathdata_math */
class Vec2i
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor Creates a vector with coordinates (0,0). */
	Vec2i() : x(0), y(0) {}

	/*!	\label_constructor
		\param _x			The size of the vector on the X axis.
		\param _y			The size of the vector on the Y axis. */
	Vec2i(KyInt32 _x, KyInt32 _y) : x(_x), y(_y) {}

	/*!	\label_constructor
		\param coords		An array of two integers that contains the size of the
							vector along the X and Y axes respectively. */
	explicit Vec2i(KyInt32* coords) { Set(coords); }

	/*!	Retrieves the size of the vector around one of its axes. Use [0] for the X axis, or [1] for
		the Y axis. */
	KY_FORCE_INLINE KyInt32 operator[](KyInt32 idx) const { return (&x)[idx]; }

	/*!	Retrieves the size of the vector around one of its axes. Use [0] for the X axis, or [1] for
		the Y axis. Non-const and returned as a reference to allow modification of the member. */
	KY_FORCE_INLINE KyInt32& operator[](KyInt32 idx) { return (&x)[idx]; }

	/*!	Sets the coordinates of the vector to match the specified values.
		\param _x			The size of the vector on the X axis.
		\param _y			The size of the vector on the Y axis. */
	KY_FORCE_INLINE void Set(KyInt32 _x, KyInt32 _y) { x = _x; y = _y; }

	/*!	Sets the coordinates of the vector to match the specified values.
		\param coords		An array of two floating-point numbers that contains the size of the
							vector along the X and Y axes respectively. */
	KY_FORCE_INLINE void Set(KyInt32* coords) { x = coords[0]; y = coords[1]; }

	/*!	Returns \c true if this object contains the same coordinates as \c v. */
	KY_FORCE_INLINE bool operator==(const Vec2i& v) const { return x == v.x && y == v.y; }

	/*!	Returns \c true if this object contains at least one different coordinate from \c v. */
	KY_FORCE_INLINE bool operator!=(const Vec2i& v) const { return !operator==(v);/*x != v.x || y != v.y;*/ }

	/*!	Returns \c true if the size of this vector along both the X and Y axes is less than that of \c v. */
	KY_FORCE_INLINE bool operator<(const Vec2i& v) const
	{
		if (x < v.x) return true;
		if (x > v.x) return false;
		return (y < v.y);
	}

	/*!	Multiplies both the X and Y coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec2i& operator*=(KyInt32 s)      { x *= s;   y *= s;   return *this; }
	
	/*!	Divides both the X and Y coordinates of this vector by the specified value. */
	KY_FORCE_INLINE Vec2i& operator/=(KyInt32 d)      { x /= d;   y /= d;   return *this; }
	
	/*!	Adds \c t to both the X and Y coordinates of this vector. */
	KY_FORCE_INLINE Vec2i& operator+=(KyInt32 t)      { x += t;   y += t;   return *this; }
	
	/*!	Subtracts \c t from both the X and Y coordinates of this vector. */
	KY_FORCE_INLINE Vec2i& operator-=(KyInt32 t)      { x -= t;   y -= t;   return *this; }
	
	/*!	Adds the X coordinate of \c v to the X coordinate of this vector, and adds the Y coordinate of \c v
		to the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec2i& operator+=(const Vec2i& v) { x += v.x; y += v.y; return *this; }
	
	/*!	Subtracts the X coordinate of \c v from the X coordinate of this vector, and subtracts the Y coordinate of \c v
		from the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec2i& operator-=(const Vec2i& v) { x -= v.x; y -= v.y; return *this; }

	/*!	Multiplies both the X and Y coordinates of the vector by the specified value. */
	KY_FORCE_INLINE Vec2i operator*(KyInt32 s)      const { return Vec2i(x * s  , y * s  ); }
	
	/*!	Divides both the X and Y coordinates of the vector by the specified value. */
	KY_FORCE_INLINE Vec2i operator/(KyInt32 d)      const { return Vec2i(x / d  , y / d  ); }
	
	/*!	Adds the X coordinate of \c v to the X coordinate of this vector, and adds the Y coordinate of \c v
		to the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec2i operator+(const Vec2i& v) const { return Vec2i(x + v.x, y + v.y); }
	
	/*!	Subtracts the X coordinate of \c v from the X coordinate of this vector, and subtracts the Y coordinate of \c v
		from the Y coordinate of this vector. */
	KY_FORCE_INLINE Vec2i operator-(const Vec2i& v) const { return Vec2i(x - v.x, y - v.y); }
	
	/*!	Negates the X and Y coordinates of this vector, effectively flipping it around the origin. */
	KY_FORCE_INLINE Vec2i operator-()               const { return Vec2i(-x     , -y     ); }

	/*!	Returns the dot product of this vector and \c v. */
	KY_FORCE_INLINE KyInt32 operator*(const Vec2i& v) const { return x * v.x + y * v.y; } // DOT PROD
	
	/*!	Rotates this vector 90 degrees counter-clockwise (negating the Y coordinate). */
	KY_FORCE_INLINE Vec2i PerpCCW() const{return Vec2i(y, -x);}
	
	/*!	Rotates this vector 90 degrees clockwise (negating the X coordinate). */
	KY_FORCE_INLINE Vec2i PerpCW() const{return Vec2i(-y, x);}
	
	/*!	Returns the magnitude on the Z axis of the cross product between this vector and \c v. */
	KY_FORCE_INLINE KyInt32 CrossProd(const Vec2i& v) const { return x * v.x + y * v.y; } // CROSS PROD 2D aka PERP PROD

	/*!	Indicates whether or not the polyline formed by ( \c A to \c B to this vector ) outlines a triangle in
		counter-clockwise order when seen from above.
		\return	Twice the area of the triangle outlined by ( \c A to \c B to this vector ).
				-	If this value is positive, ( \c A to \c B to this vector ) outlines a triangle in counter-clockwise order.
				-	If this value is negative, ( \c A to \c B to this vector ) outlines a triangle in clockwise order. 
				-	If this value is 0, then \c A, \c B and this vector are all colinear. */
	KY_FORCE_INLINE KyInt32 Orient2d(const Vec2i& A, const Vec2i& B) const { return (A.x - x) * (B.y - y) - (B.x - x) * (A.y - y); } // > 0 if (MA,MB) is CCW

	/*!	Returns \c true if this vector is on the left side when moving from \c A to \c B, or if this vector
		is on the line between \c A and \c B. */
	KY_FORCE_INLINE bool IsOnLeftSide(const Vec2i& A, const Vec2i& B) const { return Orient2d(A, B) >= 0; }

	/*!	Returns \c true if this vector is on the left side when moving from \c A to \c B, but not if this vector
		is on the line between \c A and \c B. */
	KY_FORCE_INLINE bool IsStrictlyOnLeftSide(const Vec2i& A, const Vec2i& B) const { return Orient2d(A, B) > 0; }

	/*!	Returns the square of the magnitude of the vector. */
	KY_FORCE_INLINE KyInt32 GetSquareLength() const { return x * x + y * y; }

	/*!	Returns a vector that identifies the next grid cell to the East: i.e. in the positive direction of the X axis. */
	KY_FORCE_INLINE Vec2i NeighborEast() const { return Vec2i(x + 1, y    ); }

	/*!	Set the \c neighbor vector coordinates to identify the next grid cell to the East: i.e. in the positive direction of the X axis. */
	KY_FORCE_INLINE void NeighborEast(Vec2i& neighbor) const { neighbor.Set(x + 1, y    ); }
	
	/*!	Returns a vector that identifies the next grid cell to the West: i.e. in the negative direction of the X axis. */
	KY_FORCE_INLINE Vec2i NeighborWest() const { return Vec2i(x - 1, y    ); }

	/*!	Set the \c neighbor vector coordinates to identify the next grid cell to the West: i.e. in the negative direction of the X axis. */
	KY_FORCE_INLINE void NeighborWest(Vec2i& neighbor) const { neighbor.Set(x - 1, y    ); }
	
	/*!	Returns a vector that identifies the next grid cell to the North: i.e. in the positive direction of the Y axis. */
	KY_FORCE_INLINE Vec2i NeighborNorth() const { return Vec2i(x    , y + 1); }

	/*!	Set the \c neighbor vector coordinates to identify the next grid cell to the North: i.e. in the positive direction of the Y axis. */
	KY_FORCE_INLINE void NeighborNorth(Vec2i& neighbor) const { neighbor.Set(x    , y + 1); }
	
	/*!	Returns a vector that identifies the next grid cell to the South: i.e. in the negative direction of the Y axis. */
	KY_FORCE_INLINE Vec2i NeighborSouth() const { return Vec2i(x    , y - 1); }

	/*!	Set the \c neighbor vector coordinates to identify the next grid cell to the South: i.e. in the negative direction of the Y axis. */
	KY_FORCE_INLINE void NeighborSouth(Vec2i& neighbor) const { neighbor.Set(x    , y - 1); }

	/*!	Returns a vector that identifies the next grid cell in the direction of the specified CardinalDir. */
	KY_FORCE_INLINE Vec2i Neighbor(CardinalDir dir) const
	{
		if (dir == CardinalDir_EAST)
			return NeighborEast();

		if (dir == CardinalDir_NORTH)
			return NeighborNorth();

		if (dir == CardinalDir_WEST)
			return NeighborWest();

		return NeighborSouth();
	}

	/*!	Set the \c neighbor vector coordinates to identify the next grid cell in the direction of the specified CardinalDir. */
	KY_FORCE_INLINE void Neighbor(CardinalDir dir, Vec2i& neighbor) const
	{
		if (dir == CardinalDir_EAST)
		{
			NeighborEast(neighbor);
			return;
		}

		if (dir == CardinalDir_NORTH)
		{
			NeighborNorth(neighbor);
			return;
		}

		if (dir == CardinalDir_WEST)
		{
			NeighborWest(neighbor);
			return;
		}

		NeighborSouth(neighbor);
	}

	
	/*!	Returns a vector that identifies the next grid cell to the North-East: 
		i.e. in the positive direction of the X and Y axes. */
	KY_FORCE_INLINE Vec2i NeighborNorthEast()  const { return Vec2i(x + 1, y + 1); }
	
	/*!	Returns a vector that identifies the next grid cell to the North-West: 
		i.e. in the positive direction of the Y axis and the negative direction of the X axis. */
	KY_FORCE_INLINE Vec2i NeighborNorthWest()  const { return Vec2i(x - 1, y + 1); }
	
	/*!	Returns a vector that identifies the next grid cell to the South-East: 
		i.e. in the positive direction of the X axis and the negative direction of the Y axis. */
	KY_FORCE_INLINE Vec2i NeighborSouthEast()  const { return Vec2i(x + 1, y - 1); }
	
	/*!	Returns a vector that identifies the next grid cell to the South-West: 
		i.e. in the negative direction of the X and Y axes. */
	KY_FORCE_INLINE Vec2i NeighborSouthWest()  const { return Vec2i(x - 1, y - 1); }

	/*!	Returns \c true if this vector is inside the clockwise triangle formed by the \c A, \c B and
		\c C parameters, or if this vector is on any edge between \c A, \c B and
		\c C. */
	KY_FORCE_INLINE bool IsInsideTriangle(const Vec2i& A, const Vec2i& B, const Vec2i& C) const;

	/*!	Returns \c true if this vector is inside the clockwise triangle formed by the \c A, \c B and
		\c C parameters, or if this vector is on any edge between \c A, \c B and
		\c C. This method assumes that the three points that make up the triangle are not colinear: i.e.
		they do not lie all in a straight line. */
	KY_FORCE_INLINE bool IsInsideNotColinearTriangle(const Vec2i& A, const Vec2i& B, const Vec2i& C) const;

	/*!	Returns \c true if this vector is inside the clockwise triangle formed by the \c A, \c B and
		\c C parameters, but not on any edge between \c A, \c B and \c C.  */
	KY_FORCE_INLINE bool IsStrictlyInsideTriangle(const Vec2i& A, const Vec2i& B, const Vec2i& C) const;

	/*!	Returns the normalized orientation of the X axis. */
	static KY_FORCE_INLINE Vec2i UnitX() { return Vec2i(1, 0); }
	
	/*!	Returns the normalized orientation of the Y axis. */
	static KY_FORCE_INLINE Vec2i UnitY() { return Vec2i(0, 1); }

public:
	KyInt32 x; /*!< The size of the vector along the X axis. */
	KyInt32 y; /*!< The size of the vector along the Y axis. */
};

inline Vec2i operator * (KyInt32 lhs, const Vec2i& rhs) { return rhs * lhs; }

/*!	\label_swapendianness
	\ingroup kypathdata_math */
KY_FORCE_INLINE void SwapEndianness(Kaim::Endianness::Target e, Vec2i& self)
{
	SwapEndianness(e, self.x);
	SwapEndianness(e, self.y);
}

KY_FORCE_INLINE bool Vec2i::IsInsideTriangle(const Vec2i& A, const Vec2i& B, const Vec2i& C) const
{
	if (A.Orient2d(B, C) == 0) // flat triangle
	{
		if (A.x == B.x) // vertical
		{
			// not vertical
			if (y < A.y && y < B.y && y < C.y) 
				return false; // M is below of ABC
			if (y > A.y && y > B.y && y > C.y)
				return false; // M is above of ABC
			return true; // M is on ABC
		}
		else // not vertical
		{
			if (x < A.x && x < B.x && x < C.x) 
				return false; // M is "left" of ABC
			if (x > A.x && x > B.x && x > C.x)
				return false; // M is "right" of ABC
			return true; // M is on ABC
		}
	}

	return IsInsideNotColinearTriangle(A, B, C);
}


KY_FORCE_INLINE bool Vec2i::IsInsideNotColinearTriangle(const Vec2i& A, const Vec2i& B, const Vec2i& C) const
{
	return IsOnLeftSide(A, B) && IsOnLeftSide(B, C) && IsOnLeftSide(C, A);
}


KY_FORCE_INLINE bool Vec2i::IsStrictlyInsideTriangle(const Vec2i& A, const Vec2i& B, const Vec2i& C) const
{
	return IsStrictlyOnLeftSide(A, B) && IsStrictlyOnLeftSide(B, C) && IsStrictlyOnLeftSide(C, A);
}


} // namespace Kaim

// Out of namespace Kaim
template <class OSTREAM>
KY_FORCE_INLINE OSTREAM& operator<<(OSTREAM& os, const Kaim::Vec2i& v)
{
	os << "{ " << v.x << " ; " << v.y << " }";
	return os;
}

#endif

