/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KSCOLLISIONBRIDGE_H
#define KSCOLLISIONBRIDGE_H

#include <kypathdata/bridge/icollisionbridge.h>

namespace Kaim
{
	/*!	This implementation of the ICollisionBridge interface is entirely empty. You must write 
		your own implementation of ICollisionBridge and set your own implementation in the
		Bridge created by the BaseSystem.
		\pp See also :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class DefaultCollisionBridge : public Kaim::ICollisionBridge
	{
	public:
		/*!	\label_baseconstructor */
		DefaultCollisionBridge();
		/*!	\label_virtualdestructor */
		virtual ~DefaultCollisionBridge();

	public:
		virtual void RayCast(const RayCastQuery& query, /*[out]*/RayCastResult& result);

		virtual bool HasRayCastHit(const RayCastQuery& query);

		virtual void SphereCast(const SphereCastQuery& query, /*[out]*/SphereCastResult& result);
		
		virtual bool HasSphereCastHit(const SphereCastQuery& query);

	};

} // namespace Kaim


#endif // KSCOLLISIONBRIDGE_H
