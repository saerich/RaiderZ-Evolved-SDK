/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*	\file
	\ingroup kypathdata_bridge */

#ifndef KY_IDATAREADERBRIDGE_H
#define KY_IDATAREADERBRIDGE_H

#include "kypathdata/generic/types.h"

namespace Kaim
{
	// Kynapse specific

	class DataReader;

	/*	Deprecated. 
		\ingroup kypathdata_bridge */
	class IDataReaderBridge
	{
	public:
		/*	\label_virtualdestructor */
		virtual ~IDataReaderBridge() {}

	public:
		/*	Retrieves an instance of a class that derives from DataReader. Called once by the Engine
			at initialization time. */
		virtual Kaim::DataReader* Get() = 0;

		/*	Releases the object retrieved through a call to Get(). Called once when the Engine is closed. */
		virtual void Release(Kaim::DataReader* reader) = 0;
	};

} // namespace Kaim


#endif // KY_IDATAREADERBRIDGE_H
