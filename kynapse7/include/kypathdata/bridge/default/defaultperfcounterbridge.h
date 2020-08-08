/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KSPROFILERBRIDGE_H
#define KSPROFILERBRIDGE_H

#include <kypathdata/bridge/iperfcounterbridge.h>

namespace Kaim
{
	/*!	This implementation of the IPerfCounterBridge interface is entirely empty. You must write 
		your own implementation of IPerfCounterBridge and set your own implementation in the
		Bridge.
		\pp See also :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class DefaultPerfCounterBridge : public Kaim::IPerfCounterBridge
	{
	public:
		/*!	\label_baseconstructor */
		DefaultPerfCounterBridge();

		/*!	\label_virtualdestructor */
		virtual ~DefaultPerfCounterBridge();

	public:
		virtual void AddCounter(const char* name, KyFloat32 value); 

		virtual void BeginEvent(const char* name);

		virtual void EndEvent();
	};

} // namespace Kaim


#endif // KSPROFILERBRIDGE_H
