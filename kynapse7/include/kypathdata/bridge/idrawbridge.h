/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KY_IDRAWBRIDGE_H
#define KY_IDRAWBRIDGE_H

#include "kypathdata/generic/types.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{
	/*!	Defines an interface used by \SDKName components to render information in the \gameOrSim engine for
		debugging purposes.
		\pp The Bridge can maintain an instance of a class that implements this interface, making the interface available to other
		\SDKName components. Because the implementation of this interface is specific to the \gameOrSim engine and/or third-party
		tools in use, no implementations of this interface are supplied by default with the SDK. You must write
		your own implementation of IDrawBridge, and assign an instance of your class to the Bridge.
		\pp See :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class IDrawBridge 
	{
	public:
		/*!	\label_virtualdestructor */
		virtual ~IDrawBridge() {}

	public:
		/*!	Draws a point at the specified coordinates.
			\param p			The position of the point.
			\param r			\label_redval
			\param g			\label_greenval
			\param b			\label_blueval
			\param a			\label_alphaval */
		virtual void Point(const Vec3f& p, KyUInt32 r, KyUInt32 g, KyUInt32 b, KyUInt32 a) = 0;

		/*!	Draws a line between the specified points.
			\param p1			The starting point of the line.
			\param p2			The ending point of the line.
			\param r			\label_redval
			\param g			\label_greenval
			\param b			\label_blueval
			\param a			\label_alphaval */
		virtual void Line(const Vec3f& p1, const Vec3f& p2, KyUInt32 r, KyUInt32 g, KyUInt32 b, KyUInt32 a) = 0;

		/*!	Draws the specified text at the specified position.
			\param p			The position of the text, in three-dimensional space.
			\param r			\label_redval
			\param g			\label_greenval
			\param b			\label_blueval
			\param a			\label_alphaval
			\param text			The text to be rendered. */
		virtual void Text(const Vec3f& p, KyUInt32 r, KyUInt32 g, KyUInt32 b, KyUInt32 a, const char* text) = 0;
	};

} // namespace Kaim


#endif // KY_IDRAWBRIDGE_H
