/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_shapes */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_DRAW_H
#define KY_DRAW_H

#include <kypathdata/basesystem/profilermacro.h>

#include <kypathdata/basesystem/basesystem.h>
#include <kypathdata/bridge/bridge.h>
#include <kypathdata/bridge/idrawbridge.h>
#include <kypathdata/generic/macros.h>


namespace Kaim
{

class Vec3f;


/*!	Draws a line by invoking the IDrawBridge::Line() method. 
	\param p1			The start point of the line.
	\param p2			The end point of the line.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\ingroup kypathdata_shapes */
inline
void Draw3DLine(const Vec3f& p1, const Vec3f& p2,
					 KyUInt32 r, KyUInt32 g, KyUInt32 b)
{
	if (GetBaseSystem().GetBridge().GetDrawBridge() != KY_NULL)
		GetBaseSystem().GetBridge().GetDrawBridge()->Line(p1, p2, r, g, b, 255);
}

/*!	Draws a point by invoking the IDrawBridge::Point() method.
	\param p1			The position of the point.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\ingroup kypathdata_shapes */
inline
void Draw3DPoint(const Vec3f& p1,
					  KyUInt32 r, KyUInt32 g, KyUInt32 b)
{
	if (GetBaseSystem().GetBridge().GetDrawBridge() != KY_NULL)
		GetBaseSystem().GetBridge().GetDrawBridge()->Point(p1, r, g, b, 255);
}


/*!	Draws a triangle, defined by the positions of its corners.
	\param p0			The position of the first corner of the triangle.
	\param p1			The position of the second corner of the triangle.
	\param p2			The position of the third corner of the triangle.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\pp \image html "graphics/3Drender_triangle.png"
	For example, the image above was produced by the following call:
	\code
	DrawTriangle(Kaim::Vec3f(45.0f, 5.0f, 40.0f,)
				 Kaim::Vec3f(51.0f, 1.0f, 40.0f,)
				 Kaim::Vec3f(54.0f, 9.0f, 40.0f,)
				 255, 255, 0); \endcode
	\ingroup kypathdata_shapes */
/*
		        p0
		         +
		        / \
		       /   \
		      /     \
		     /       \
		    /         \
		   /           \
		  /             \
		 +---------------+
		p2               p1
*/
void DrawTriangle(
	const Vec3f& p0, const Vec3f& p1, const Vec3f& p2,
	KyUInt32 r, KyUInt32 g, KyUInt32 b);



/*!	Draws a parallelogram, whose shape is defined by three vectors: one (\c p0) that indicates the coordinates of one corner,
	and two (\c v1 and \c v2)that indicate the orientation and length of the two sides that extend from that corner. The width and
	orientation of the remaining sides are inferred.
	\pp If the vectors you specify for the \c v1 and \c v2 arguments are orthogonal (at 90 degrees to each other), the 
	resulting shape will be a rectangle.
	\param p0			The position of one corner of the parallelogram.
	\param v1			The orientation and length of one side that meets \c p0.
	\param v2			The orientation and length of the other side that meets \c p0.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\pp \image html "graphics/3Drender_rectangle.png"
	For example, the image above was produced by the following call:
	\code
	DrawParallelogram(Kaim::Vec3f(45.0f, 2.5f, 40.0f), 
					  Kaim::Vec3f(0.0f, 5.0f, 0.0f), 
					  Kaim::Vec3f(10.0f, 0.0f, 0.0f), 
					  255, 255, 0); \endcode
	\ingroup kypathdata_shapes */
/*
		       +--------------------------+
		      /                          /
		     /                          /
		    / v2                       /
		   /                          /
		  /                          /
		 +--------------------------+
		p0             v1
*/
void DrawParallelogram(const Vec3f& p0,
	const Vec3f& v1, const Vec3f& v2,
	KyUInt32 r, KyUInt32 g, KyUInt32 b);


/*!	Draws a parallelepiped, a three-dimensional volume formed by six parallelograms. The parallelepiped is defined by four 
	vectors: one (\c p0) that indicates the coordinates of one corner, and three (\c v1, \c v2 and \c v3) that indicate 
	the orientation and length of the three sides that extend from that corner. The width and orientation of the remaining sides 
	are inferred.
	\pp If the vectors you specify for the \c v1, \c v2 and \c v3 arguments are orthogonal (at 90 degrees to each other), the 
	resulting shape will be a rectangular prism.
	\param p0			The position of one corner of the parallelepiped.
	\param v1			The orientation and length of the first side that meets \c p0.
	\param v2			The orientation and length of the second side that meets \c p0.
	\param v3			The orientation and length of the third side that meets \c p0.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\pp \image html "graphics/3Drender_bbox.png"
	For example, the image above was produced by the following call:
	\code
	DrawParallelepiped(Kaim::Vec3f(45.0f, 5.0f, 40.0f), 
					   Kaim::Vec3f(4.0f, 4.0f, 0.0f), 
					   Kaim::Vec3f(6.0f, -4.0f, 0.0f), 
					   Kaim::Vec3f(0.0f, 0.0f, 4.0f),
					   255, 255, 0); \endcode
	\ingroup kypathdata_shapes */
/*
		       +--------------------------+
		      / \                        / \
		     /   \                      /   \
		    /     \                    /     \
		   /       \                  /       \
		  /         \                /         \
		 +--------------------------+           \
		  \           \              \           \
		   \           +--------------\-----------+
		    \         /                \         /
		     \ v3    /                  \       /
		      \     / v2                 \     /
		       \   /                      \   /
		        \ /                        \ /
		         +--------------------------+
		        p0            v1
*/
void DrawParallelepiped(const Vec3f& p0,
	const Vec3f& v1, const Vec3f& v2, const Vec3f& v3,
	KyUInt32 r, KyUInt32 g, KyUInt32 b);


/*!	Draws a three-dimensional rectangular prism, aligned to the axes of the \SDKName coordinate system. This
	implementation defines the extents of the bounding box using its minima and maxima. The position and
	length of the remaining vertices and sides are inferred.
	\param p0			The position of the minima: the corner with the lowest values on the X, Y and Z axes.
						Diagonally opposite to \c p1.
	\param p1			The position of the maxima: the corner with the highest values on the X, Y and Z axes.
						Diagonally opposite to \c p0.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\pp \image html "graphics/3Drender_aabbox_minmax.png"
	For example, the image above was produced by the following call:
	\code
	DrawAABBox(Kaim::Vec3f(46.0f, 2.0f, 40.0f), 
			   Kaim::Vec3f(54.0f, 8.0f, 44.0f), 
			   255, 255, 0); \endcode
	\ingroup kypathdata_shapes */
void DrawAABBox(const Vec3f& p0, const Vec3f& p1,
	KyUInt32 r, KyUInt32 g, KyUInt32 b);


/*!	Draws a three-dimensional rectangular prism, aligned to the axes of the \SDKName coordinate system. This
	implementation defines the extents of the bounding box using the position of one corner along with the height,
	length and width of the box. The position and length of the remaining vertices and sides are inferred.
	\param p0			The position of one corner of the bounding box.
	\param dx			The extents of the box measured along the X axis.
	\param dy			The extents of the box measured along the Y axis.
	\param dz			The extents of the box measured along the Z axis.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval'
	\pp \image html "graphics/3Drender_aabbox_xyz.png"
	For example, the image above was produced by the following call:
	\code
	DrawAABBox(Kaim::Vec3f(46.0f, 2.0f, 40.0f), 
			   8.0f, 6.0f, 4.0f, 
			   255, 255, 0); \endcode
	\ingroup kypathdata_shapes */
inline
void DrawAABBox(const Vec3f& p0,
	KyFloat32 dx, KyFloat32 dy, KyFloat32 dz,
	KyUInt32 r, KyUInt32 g, KyUInt32 b)
{
	DrawAABBox(p0, p0 + Vec3f(dx, dy, dz), r, g, b);
}

/*!	Draws a circle, whose shape is defined by a point and a radius.
	\param center		The position of the center of the circle.
	\param radius		The radius of the circle, in meters.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\param nmbLines		Determines the number of straight line segments used to approximate the circumference of the circle. 
						If you do not specify a value for this argument, the default value of 15 will be used.
	\param drawRadius	Determines whether or not the radius of the circle is to be rendered. When this value is set to #KY_TRUE, 
						this function draws a line from the center point to the end of each straight line segment used to approximate 
						the circumference of the circle. If you do not specify a value for this argument, the default value of 
						#KY_FALSE will be used.
	\param normal		Determines the normal of the circle: the orientation perpendicular to the two-dimensional plane in which the 
						circle is drawn. By default, the positive direction of the Up axis is used for this value, which results in 
						the drawing plane of the circle being horizontal, as in the image below. You can specify a different normal 
						in order to tilt the drawing plane in any direction. 
	\pp \image html "graphics/3Drender_circle.png"
	For example, the image above was produced by the following call:
	\code
	DrawCircle(Kaim::Vec3f(50.0f, 5.0f, 40.0f), 
			   4.0f, 
			   255, 255, 0); \endcode
	\ingroup kypathdata_shapes */
void DrawCircle(const Vec3f& center, KyFloat32 radius,
	KyUInt32 r, KyUInt32 g, KyUInt32 b,
	KyUInt32 nmbLines = 15, KyBool drawRadius = KY_FALSE, const Vec3f& normal = Vec3f::UnitZ());

/*!	Draws a two-dimensional projection of an arrow beginning at any starting coordinates in three-dimensional space, and pointing at 
	another set of coordinates.
	\param p1			The start point of the arrow.
	\param p2			The end point of the arrow.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\param edgeWidth	The width of the arrow, in meters. If you do not specify a value for this argument, the default value of 
						0.5 meters will be used.
	\param normal		Determines the normal of the arrow: the orientation perpendicular to the two-dimensional plane in which the 
						arrow is drawn. By default, the positive direction of the Up axis is used for this value, which results in 
						the drawing plane of the arrow being horizontal, as in the image below. You can specify a different normal 
						in order to tilt the drawing plane in any direction. 
						\pp Note that the start and end of the arrow (\c p1 and \c p2) are always placed at the points you specify. These 
						points determine the tilt of the arrow along its length. The normal argument controls only the rotation of the 
						two-dimensional drawing plane around the axis defined by the start and end points. 
	\pp \image html "graphics/3Drender_arrow.png"
	For example, the image above was produced by the following call:
	\code
	DrawArrow(Kaim::Vec3f(50.0f, 2.0f, 40.0f), 
			  Kaim::Vec3f(50.0f, 10.0f, 40.0f), 
			  255, 255, 0); \endcode
	\ingroup kypathdata_shapes */
void DrawArrow(const Vec3f& p1, const Vec3f& p2,
	KyUInt32 r, KyUInt32 g, KyUInt32 b,
	KyFloat32 edgeWidth = 0.5f, const Vec3f& normal = Vec3f::UnitZ());


/*!	Draws an approximation of a sphere that consists of nine circles: three around each axis in the \SDKName coordinate system.
	\param center		The position of the center of the sphere.
	\param radius		The radius of the sphere, in meters.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\param nmbLines		Determines the number of straight line segments used to approximate the circumference of each circle. 
						If you do not specify a value for this argument, the default value of 8 will be used.
	\pp \image html "graphics/3Drender_sphere.png"
	For example, the image above was produced by the following call:
	\code
	DrawSphere(Kaim::Vec3f(50.0f, 5.0f, 40.0f), 
			   4.0f, 
			   255, 255, 0,
			   15.0f); \endcode
	\ingroup kypathdata_shapes */
void DrawSphere(const Vec3f& center, const KyFloat32 radius,
	KyUInt32 r, KyUInt32 g, KyUInt32 b,
	KyUInt32 nmbLines = 8);


/*!	Calls DrawCapsule() to draw a three-dimensional capsule whose center line is constrained to the vertical axis of the \SDKName
	coordinate system.
	\param center		The position at the center of the capsule.
	\param halfHeight	Half the height of the central cylinder, in meters.
	\param radius		The radius of the central capsule and the hemisphere at each end, in meters.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\param nmbLines		Determines the number of straight line segments used to approximate the circumference of each hemisphere. 
						If you do not specify a value for this argument, the default value of 8 will be used.
	\ingroup kypathdata_shapes */
void DrawCapsule_Vertical(const Vec3f& center, KyFloat32 halfHeight, KyFloat32 radius,
	KyUInt32 r, KyUInt32 g, KyUInt32 b,
	KyUInt32 nmbLines = 16);


/*!	Draws a three-dimensional capsule between two endpoints. This capsule is equivalent to a cylinder capped by a hemisphere at
	each end.
	\param p1			The position at the center of the hemisphere at one end of the capsule.
	\param p2			The position at the center of the hemisphere at the other end of the capsule.
	\param radius		The radius of the central capsule and the hemisphere at each end, in meters.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\param nmbLines		Determines the number of straight line segments used to approximate the circumference of each hemisphere. 
						If you do not specify a value for this argument, the default value of 8 will be used.
	\pp \image html "graphics/3Drender_capsule.png"
	For example, the image above was produced by the following call:
	\code
	DrawCapsule(Kaim::Vec3f(46.5f, 3.0f, 41.0f), 
				Kaim::Vec3f(53.5f, 3.0f, 41.0f), 
				3.0f, 
				255, 255, 0); \endcode
	\ingroup kypathdata_shapes */
void DrawCapsule(const Vec3f& p1, const Vec3f& p2,
	KyFloat32 radius,
	KyUInt32 r, KyUInt32 g, KyUInt32 b,
	KyUInt32 nmbLines = 16);

/*!	Draws a three-dimensional cylinder between two endpoints. 
	\param p1			The position at the center of one end of the cylinder.
	\param p2			The position at the center of the other end of the cylinder.
	\param radius		The radius of the cylinder, in meters.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\param nmbLines		Determines the number of straight line segments used to approximate the circumference of the cylinder. 
						If you do not specify a value for this argument, the default value of 16 will be used.
	\param drawRadius	Determines whether or not the radius of the cylinder is to be rendered. When this value is set to #KY_TRUE, 
						this function draws a line from the center point to the end of each straight line segment used to approximate 
						the circumference of the cylinder. If you do not specify a value for this argument, the default value of 
						#KY_FALSE will be used.
	\pp \image html "graphics/3Drender_cylinder.png"
	For example, the image above was produced by the following call:
	\code
	DrawCylinder(Kaim::Vec3f(47.0f, 5.0f, 39.5f), 
				 Kaim::Vec3f(53.0f, 7.0f, 43.0f), 
				 4.0f, 
				 255, 255, 0); \endcode
	\ingroup kypathdata_shapes */
void DrawCylinder(const Vec3f& p1, const Vec3f& p2, const KyFloat32 radius,
	KyUInt32 r, KyUInt32 g, KyUInt32 b,
	KyUInt32 nmbLines = 16, KyBool drawRadius = KY_FALSE);

/*!	Calls DrawCylinder() to draw a three-dimensional cylinder whose center line is constrained to the vertical axis of the \SDKName
	coordinate system.
	\param center		The position of the center of the cylinder.
	\param halfHeight	Half the height of the cylinder, in meters.
	\param radius		The radius of the cylinder, in meters.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\param nmbLines		Determines the number of straight line segments used to approximate the circumference of the cylinder. 
						If you do not specify a value for this argument, the default value of 16 will be used.
	\param drawRadius	Determines whether or not the radius of the cylinder is to be rendered. When this value is set to #KY_TRUE, 
						this function draws a line from the center point to the end of each straight line segment used to approximate 
						the circumference of the cylinder. If you do not specify a value for this argument, the default value of 
						#KY_FALSE will be used.
	\pp \image html "graphics/3Drender_cylinder_vertical.png"
	For example, the image above was produced by the following call:
	\code
	DrawCylinder_Vertical(Kaim::Vec3f(50.0f, 41.25f, 5.0f), 
						  1.75f, 
						  4.0f, 
						  255, 255, 0); \endcode
	\ingroup kypathdata_shapes */
void DrawCylinder_Vertical(const Vec3f& center, KyFloat32 halfHeight, KyFloat32 radius,
	KyUInt32 r, KyUInt32 g, KyUInt32 b,
	KyUInt32 nmbLines = 16, KyBool drawRadius = KY_FALSE);

} // namespace Kaim

#endif // KY_DRAW_H
