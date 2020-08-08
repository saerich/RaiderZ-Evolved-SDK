/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KSDRAWBRIDGE_H
#define KSDRAWBRIDGE_H

#include <kypathdata/bridge/idrawbridge.h>

namespace Kaim
{
	/*!	This implementation of the IDrawBridge interface is entirely empty. You must write 
		your own implementation of IDrawBridge and set your own implementation in the
		Bridge created by the BaseSystem.
		\pp See also :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class DefaultDrawBridge : public Kaim::IDrawBridge
	{
	public:
		/*!	\label_baseconstructor */
		DefaultDrawBridge();
		
		/*!	\label_virtualdestructor */
		virtual ~DefaultDrawBridge();

	public:
		virtual void Point(const Kaim::Vec3f& p, KyUInt32 r, KyUInt32 g, KyUInt32 b, KyUInt32 a);

		virtual void Line(const Kaim::Vec3f& p1, const Kaim::Vec3f& p2, KyUInt32 r, KyUInt32 g, KyUInt32 b, KyUInt32 a);

		virtual void Text(const Kaim::Vec3f& p, KyUInt32 r, KyUInt32 g, KyUInt32 b, KyUInt32 a, const char* text);

	};
} // namespace Kaim


#endif // KSDRAWBRIDGE_H
