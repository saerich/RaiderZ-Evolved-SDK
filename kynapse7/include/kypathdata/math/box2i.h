/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_math */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_Box2i_H
#define KyPathData_Box2i_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec2i.h"
#include "kypathdata/math/vec3i.h"
#include "kypathdata/math/triangle3i.h"

namespace Kaim
{
	class Box2i;
}

template <class OSTREAM>
OSTREAM& operator<<(OSTREAM& os, const Kaim::Box2i& v);

namespace Kaim
{

/*
----------------- CoordBox -----------------
minX=2 , maxX=8 , countX=7 (=8-2 +1)
minY=3 , maxY=6 , countY=4 (=6-3 +1)
 /\Y
 |   |   |   |   |   |   |   |   |   |
7+---+---+---+---+---+---+---+---+---+--
 |   |   |   |   |   |   |   |   |   |
6+---+---#########################---+--
 |   |   #   |   |   |   |   |   #   |
5+---+---#---+---+---+---+---+---#---+--
 |   |   #   |   |   |   |   |   #   |
4+---+---#---+---+---+---+---+---#---+--
 |   |   #   |   |   |   |   |   #   |
3+---+---#########################---+--
 |   |   |   |   |   |   |   |   |   |
2+---+---+---+---+---+---+---+---+---+--
 |   |   |   |   |   |   |   |   |   |
1+---+---+---+---+---+---+---+---+---+--
 |   |   |   |   |   |   |   |   |   |
0+---+---+---+---+---+---+---+---+---+-> X
 0   1   2   3   4   5   6   7   8   9

--------- PixelBox, CellBox, TileBox ---------
minX=2 , maxX=7 , countX=6 (=7-2 +1)
minY=3 , maxY=5 , countY=3 (=5-3 +1)
/\Y
|   |   |   |   |   |   |   |   |   |
+---+---+---+---+---+---+---+---+---+--
|0,6|1,6|2,6|   |   |   |   |   |   |
+---+---#########################---+--
|0,5|1,5#2,5|3,5|4,5|5,5|6,5|7,5#   |
+---+---#---+---+---+---+---+---#---+--
|0,4|1,4#2,4|3,4|4,4|5,4|6,4|7,4#   |
+---+---#---+---+---+---+---+---#---+--
|0,3|1,3#2,3|3,3|4,3|5,3|6,3|7,3#   |
+---+---#########################---+--
|0,2|1,2|2,2|   |   |   |   |   |   |
+---+---+---+---+---+---+---+---+---+--
|0,1|1,1|2,1|   |   |   |   |   |   |
+---+---+---+---+---+---+---+---+---+--
|0,0|1,0|2,0|   |   |   |   |   |   |
+---+---+---+---+---+---+---+---+---+-> X
*/

/*!	This class represents a two-dimensional axis-aligned bounding box whose dimensions are stored using
	32-bit integers.
	\pp This class may be used to store (X,Y) coordinates. For example, the CoordBox type (used during PathData
	generation) uses this class as follows:
	\code
	minX=2 , maxX=8 , countX=7 (=8-2 +1)
	minY=3 , maxY=6 , countY=4 (=6-3 +1)
	 /\Y
	 |   |   |   |   |   |   |   |   |   |
	7+---+---+---+---+---+---+---+---+---+--
	 |   |   |   |   |   |   |   |   |   |
	6+---+---#########################---+--
	 |   |   #   |   |   |   |   |   #   |
	5+---+---#---+---+---+---+---+---#---+--
	 |   |   #   |   |   |   |   |   #   |
	4+---+---#---+---+---+---+---+---#---+--
	 |   |   #   |   |   |   |   |   #   |
	3+---+---#########################---+--
	 |   |   |   |   |   |   |   |   |   |
	2+---+---+---+---+---+---+---+---+---+--
	 |   |   |   |   |   |   |   |   |   |
	1+---+---+---+---+---+---+---+---+---+--
	 |   |   |   |   |   |   |   |   |   |
	0+---+---+---+---+---+---+---+---+---+-> X
	 0   1   2   3   4   5   6   7   8   9
	\endcode
	\pp Alternatively, this class may be used to refer to elements within a regular grid according to their
	column and row. For example, the PixelBox, CellBox and TileBox types (used during PathData generation)
	use this class as follows:
	\code
	minX=2 , maxX=7 , countX=6 (=7-2 +1)
	minY=3 , maxY=5 , countY=3 (=5-3 +1)
	/\Y
	|   |   |   |   |   |   |   |   |   |
	+---+---+---+---+---+---+---+---+---+--
	|0,6|1,6|2,6|   |   |   |   |   |   |
	+---+---#########################---+--
	|0,5|1,5#2,5|3,5|4,5|5,5|6,5|7,5#   |
	+---+---#---+---+---+---+---+---#---+--
	|0,4|1,4#2,4|3,4|4,4|5,4|6,4|7,4#   |
	+---+---#---+---+---+---+---+---#---+--
	|0,3|1,3#2,3|3,3|4,3|5,3|6,3|7,3#   |
	+---+---#########################---+--
	|0,2|1,2|2,2|   |   |   |   |   |   |
	+---+---+---+---+---+---+---+---+---+--
	|0,1|1,1|2,1|   |   |   |   |   |   |
	+---+---+---+---+---+---+---+---+---+--
	|0,0|1,0|2,0|   |   |   |   |   |   |
	+---+---+---+---+---+---+---+---+---+-> X
	\endcode
	\pp Each instance of this class also maintains a count of the number of elements (or grid cells) contained
	by the box along its X and Y axes, accessible through the CountX() and CountY() methods. For coordinate
	boxes, all coordinates along the edge of the box are counted, including the corners. For grid boxes, all
	grid cells that are contained within the box are counted. See the images above for examples.
	\ingroup kypathdata_math */
class Box2i
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_constructor This constructor creates a box with invalid extents; you must follow this method
		with a call to Set() before you can use the box. */
	Box2i() { Clear(); }

	/*!	\label_constructor
		\param min_			The minima of the bounding box.
		\param max_			The maxima of the bounding box. */
	Box2i(const Vec2i& min_, const Vec2i& max_) : m_min(min_), m_max(max_) { UpdateCountXY(); }

	/*!	\label_constructor
		\param min_x		The minimum position of the bounding box on the X axis.
		\param min_y		The minimum position of the bounding box on the Y axis.
		\param max_x		The maximum position of the bounding box on the X axis.
		\param max_y		The maximum position of the bounding box on the Y axis. */
	Box2i(KyInt32 min_x, KyInt32 min_y, KyInt32 max_x, KyInt32 max_y) : m_min(min_x, min_y), m_max(max_x, max_y) { UpdateCountXY(); }

	/*!	Indicates whether or not the extents of the bounding box are valid. */
	KY_FORCE_INLINE bool IsValid() { return CountX() >= 0 && CountY() >= 0; }

	/*!	Sets both the minima and maxima of the bounding box to (0,0). */
	KY_FORCE_INLINE void MakeZero() { m_min.Set(0, 0); m_max.Set(0, 0); UpdateCountXY(); }

	/*!	Sets the extents of the bounding box to the specified values.
		\param min_			The minima of the bounding box.
		\param max_			The maxima of the bounding box. */
	KY_FORCE_INLINE void Set(const Vec2i& min_, const Vec2i& max_) { m_min = min_; m_max = max_; UpdateCountXY(); }

	/*!	Sets the minima of the bounding box to the specified coordinates. */
	KY_FORCE_INLINE void SetMin(const Vec2i& min_) { m_min = min_; UpdateCountXY(); }

	/*!	Sets the maxima of the bounding box to the specified coordinates. */
	KY_FORCE_INLINE void SetMax(const Vec2i& max_) { m_max = max_; UpdateCountXY(); }

	/*!	Retrieves the minima of the bounding box. */
	KY_FORCE_INLINE const Vec2i& Min() const { return m_min; }

	/*!	Retrieves the maxima of the bounding box. */
	KY_FORCE_INLINE const Vec2i& Max() const { return m_max; }

	/*!	Retrieves the coordinates of the South-West corner of the bounding box. Since North is considered
		to be the positive direction of the Y axis and East is considered to be the positive direction of
		the Y axis, this corner is equivalent to the minima. */
	KY_FORCE_INLINE Vec2i CornerSouthWest() const { return m_min; }

	/*!	Retrieves the coordinates of the North-East corner of the bounding box. Since North is considered
		to be the positive direction of the Y axis and East is considered to be the positive direction of
		the Y axis, this corner is equivalent to the maxima. */
	KY_FORCE_INLINE Vec2i CornerNorthEast() const { return m_max; }

	/*!	Retrieves the coordinates of the South-East corner of the bounding box. Since North is considered
		to be the positive direction of the Y axis and East is considered to be the positive direction of
		the Y axis, this corner has the minimum value on the Y axis and the maximum value on the X axis. */
	KY_FORCE_INLINE Vec2i CornerSouthEast() const { return Vec2i(m_max.x, m_min.y); }

	/*!	Retrieves the coordinates of the North-West corner of the bounding box. Since North is considered
		to be the positive direction of the Y axis and East is considered to be the positive direction of
		the Y axis, this corner has the maximum value on the Y axis and the minimum value on the X axis. */
	KY_FORCE_INLINE Vec2i CornerNorthWest() const { return Vec2i(m_min.x, m_max.y); }

	/*!	Retrieves the number of grid cells contained within this box along its X and Y axes. */
	KY_FORCE_INLINE const Vec2i& CountXY() const { return m_countXY; }

	/*!	Retrieves the number of grid cells contained within this box along its X axis. */
	KY_FORCE_INLINE KyInt32 CountX() const { return m_countXY.x; }

	/*!	Retrieves the number of grid cells contained within this box along its Y axis. */
	KY_FORCE_INLINE KyInt32 CountY() const { return m_countXY.y; }

	/*!	Returns \c true if the specified position is contained within the extents of the bounding box
		or if the specified position is located on the edge of the bounding box. */
	KY_FORCE_INLINE bool IsInside(const Vec2i& pos) const { return pos.x >= m_min.x && pos.x <= m_max.x && pos.y >= m_min.y && pos.y <= m_max.y; }

	/*!	Returns \c true if the specified position is contained entirely within the extents of the bounding box. */
	KY_FORCE_INLINE bool IsInsideStrictly(const Vec2i& pos) const { return pos.x >  m_min.x && pos.x <  m_max.x && pos.y >  m_min.y && pos.y <  m_max.y; }

	/*!	\label_clear This method sets the extents of the box to invalid values;
		you must follow this method with a call to Set() before you can use the box. */
	void Clear()
	{
		m_min.Set(KyInt32MAXVAL, KyInt32MAXVAL);
		m_max.Set(-KyInt32MAXVAL, -KyInt32MAXVAL);
		ClearCountXY();
	}

	/*!	Multiplies both the minima and maxima of the box by the specified value. */
	KY_FORCE_INLINE Box2i& operator*=(KyInt32 s) { m_min *= s; m_max *= s; UpdateCountXY(); return *this; }

	/*!	Divides both the minima and maxima of the box by the specified value. */
	KY_FORCE_INLINE Box2i& operator/=(KyInt32 s) { m_min /= s; m_max /= s; UpdateCountXY(); return *this; }

	/*!	Adds the specified two-dimensional vector to both the minima and maxima of the box. */
	KY_FORCE_INLINE Box2i& operator+=(const Vec2i& v) { m_min += v; m_max += v; return *this; }

	/*!	Subtracts the specified two-dimensional vector from both the minima and maxima of the box. */
	KY_FORCE_INLINE Box2i& operator-=(const Vec2i& v) { m_min -= v; m_max -= v; return *this; }

	/*!	Check equality */
	KY_FORCE_INLINE bool operator==(const Box2i& other) const { return other.m_min == m_min && other.m_max == m_max; }

	/*!	Retrieves the index of the grid cell at the specified (X,Y) position, counting cells column-by-column.
		This indexes the cells in the box according to the following pattern:
		\code
		/\Y
		|   |   |   |   |   |   |   |   |   |
		+---+---+---+---+---+---+---+---+---+--
		|   |   |   |   |   |   |   |   |   |
		+---+---#########################---+--
		|   |   # 2 | 5 | 8 | 11| 14| 17#   |
		+---+---#---+---+---+---+---+---#---+--
		|   |   # 1 | 4 | 7 | 10| 13| 16#   |
		+---+---#---+---+---+---+---+---#---+--
		|   |   # 0 | 3 | 6 | 9 | 12| 15#   |
		+---+---#########################---+--
		|   |   |   |   |   |   |   |   |   |
		+---+---+---+---+---+---+---+---+---+-> X
		\endcode */
	KY_FORCE_INLINE KyInt32 GetColumnMajorIndex(const Vec2i& pos) const { return (pos.x - m_min.x) * CountY() + pos.y - m_min.y; }

	/*!	Retrieves the index of the grid cell at the specified (X,Y) position, counting cells row-by-row.
		This indexes the cells in the box according to the following pattern:
		\code
		/\Y
		|   |   |   |   |   |   |   |   |   |
		+---+---+---+---+---+---+---+---+---+--
		|   |   |   |   |   |   |   |   |   |
		+---+---#########################---+--
		|   |   # 12| 13| 14| 15| 16| 17#   |
		+---+---#---+---+---+---+---+---#---+--
		|   |   # 6 | 7 | 8 | 9 | 10| 11#   |
		+---+---#---+---+---+---+---+---#---+--
		|   |   # 0 | 1 | 2 | 3 | 4 | 5 #   |
		+---+---#########################---+--
		|   |   |   |   |   |   |   |   |   |
		+---+---+---+---+---+---+---+---+---+-> X
		\endcode */
	KY_FORCE_INLINE KyInt32 GetRowMajorIndex(const Vec2i& pos) const { return (pos.y - m_min.y) * CountX() + pos.x - m_min.x; }

	/*!	Retrieves the index of the grid cell at the specified (X,Y) position, counting cells column-by-column.
		The X and Y coordinates within \c localPos should be relative to the minima of the box.
		See also GetColumnMajorIndex() for information on the index value returned. */
	KY_FORCE_INLINE KyInt32 GetColumnMajorIndexFromLocalPos(const Vec2i& localPos) const { return localPos.x * CountY() + localPos.y; }

	/*!	Retrieves the index of the grid cell at the specified (X,Y) position, counting cells row-by-row.
		The X and Y coordinates within \c localPos should be relative to the minima of the box.
		See also GetRowMajorIndex() for information on the index value returned. */
	KY_FORCE_INLINE KyInt32 GetRowMajorIndexFromLocalPos(const Vec2i& localPos)    const { return localPos.y * CountX() + localPos.x; }

	/*!	Enlarges the extents of the bounding box to include the specified two-dimensional point.
		If the point is already contained within the bounding box, the box is not modified. */
	void ExpandByVec2(const Vec2i& pos)
	{
		ExpandByVec2_MinMaxOnly(pos);
		UpdateCountXY();
	}

	/*!	Enlarges the extents of the bounding box to include the (X,Y) coordinates of the specified
		three-dimensional point. If the (X,Y) coordinates of the point are already contained within
		the bounding box, the box is not modified. */
	void ExpandByVec3(const Vec3i& pos)
	{
		ExpandByVec3_MinMaxOnly(pos);
		UpdateCountXY();
	}

	/*!	Enlarges the extents of the bounding box to include the (X,Y) extents of the specified
		three-dimensional triangle. If the (X,Y) extents of the triangle are already contained within
		the bounding box, the box is not modified. */
	void ExpandByTriangle(const Triangle3i& triangle)
	{
		ExpandByVec3_MinMaxOnly(triangle.A);
		ExpandByVec3_MinMaxOnly(triangle.B);
		ExpandByVec3_MinMaxOnly(triangle.C);
		UpdateCountXY();
	}

	/*!	Enlarges the extents of the bounding box to include the area covered by the specified bounding box.
		If the bounding box already encompasses the area of \c box, the bounding box is not modified. */
	void ExpandByBox2(const Box2i& box)
	{
		ExpandByVec2_MinMaxOnly(box.m_min);
		ExpandByVec2_MinMaxOnly(box.m_max);
		UpdateCountXY();
	}

	/*!	Enlarges the extents of the bounding box by the specified amount in all directions. */
	void Enlarge(KyInt32 enlargement)
	{
		m_min -= enlargement;
		m_max += enlargement;
		UpdateCountXY();
	}

	/*!	Enlarges the extents of the bounding box by the specified amount in all directions, and
		stores the new dimensions in the \c enlarged parameter.
		\param enlargement			The amount to enlarge the box in each direction.
		\param[out] enlarged		Stores the enlarged bounding box. */
	void GetEnlarged(KyInt32 enlargement, Box2i& enlarged) const
	{
		enlarged = *this;
		enlarged.Enlarge(enlargement);
	}

	/*!	Resizes this bounding box to contain only the grid cells that are common to both it and \c box.
		This has the following results:
		-	If \c box is entirely contained within this bounding box, this bounding box is resized to match
			the extents of \c box.
		-	If the edges of this bounding box cross the edges of \c box, the extents of this bounding box
			are resized to include only the area common to both bounding boxes.
		-	If this bounding box is entirely contained within the extents of \c box, this bounding box is
			not altered.
		-	If this bounding box does not intersect \c box at all, the extents of this bounding box are
			set to invalid values and this method returns \c false.
		\pp \return \c true if the boxes intersect, or \c false if they do not. If this method
				returns false, the extents of this bounding box will be set to invalid values.
				You must call Set() before you can use the box again. */
	bool IntersectWith(const Box2i& box)
	{
		m_min.x = Kaim::Max(m_min.x, box.m_min.x);
		m_max.x = Kaim::Min(m_max.x, box.m_max.x);
		m_min.y = Kaim::Max(m_min.y, box.m_min.y);
		m_max.y = Kaim::Min(m_max.y, box.m_max.y);
		UpdateCountXY();
		return CountX() > 0 && CountY() > 0;
	}

	/*!	Resizes this bounding box to contain only the grid cells that are common to both \c box_1 and \c box_2.
		This has the following results:
		-	If one box is entirely contained within the other, this bounding box is set to match
			the extents of the smaller box.
		-	If the edges of the two boxes cross, the extents of this bounding box are resized to include
			only the area common to both \c box_1 and \c box_2.
		-	If \c box_1 and \c box_2 do not intersect at all, the extents of this bounding box are
			set to invalid values and this method returns \c false.
		\pp \return \c true if \c box_1 and \c box_2 intersect, or \c false if they do not. If this method
				returns false, the extents of this bounding box will be set to invalid values.
				You must call Set() before you can use the box again. */
	bool SetAsIntersection(const Box2i& box_1, const Box2i& box_2)
	{
		m_min.x = Kaim::Max(box_1.m_min.x, box_2.m_min.x);
		m_max.x = Kaim::Min(box_1.m_max.x, box_2.m_max.x);
		m_min.y = Kaim::Max(box_1.m_min.y, box_2.m_min.y);
		m_max.y = Kaim::Min(box_1.m_max.y, box_2.m_max.y);
		UpdateCountXY();
		return CountX() > 0 && CountY() > 0;
	}

private:
	KY_FORCE_INLINE void UpdateCountX() { m_countXY.x = m_max.x - m_min.x + 1; }
	KY_FORCE_INLINE void UpdateCountY() { m_countXY.y = m_max.y - m_min.y + 1; }
	KY_FORCE_INLINE void UpdateCountXY() { UpdateCountX(); UpdateCountY(); }
	KY_FORCE_INLINE void ClearCountXY() { m_countXY.x = 0; m_countXY.y = 0; }

	void ExpandByVec2_MinMaxOnly(const Vec2i& pos)
	{
		const KyInt32 posx = pos.x;
		const KyInt32 posy = pos.y;
		m_min.x = Kaim::Min(m_min.x, posx);
		m_max.x = Kaim::Max(m_max.x, posx);
		m_min.y = Kaim::Min(m_min.y, posy);
		m_max.y = Kaim::Max(m_max.y, posy);
	}

	void ExpandByVec3_MinMaxOnly(const Vec3i& pos)
	{
		const KyInt32 posx = pos.x;
		const KyInt32 posy = pos.y;
		m_min.x = Kaim::Min(m_min.x, posx);
		m_max.x = Kaim::Max(m_max.x, posx);
		m_min.y = Kaim::Min(m_min.y, posy);
		m_max.y = Kaim::Max(m_max.y, posy);
	}

private:
	Vec2i m_min;
	Vec2i m_max;
	Vec2i m_countXY;

// friends declarations
	friend void SwapEndianness(Kaim::Endianness::Target e, Box2i& self);
	friend class SquarePartitionUtils;
};

/*!	\label_swapendianness
	\ingroup kypathdata_math */
inline void SwapEndianness(Kaim::Endianness::Target e, Box2i& self)
{
	SwapEndianness(e, self.m_min);
	SwapEndianness(e, self.m_max);
	SwapEndianness(e, self.m_countXY);
}

} // namespace Kaim

// Out of namespace Kaim
template <class OSTREAM>
inline OSTREAM& operator<<(OSTREAM& os, const Kaim::Box2i& v)
{
	os << "{ " << v.Min() << " ; " << v.Max() << " }";
	return os;
}

#endif

