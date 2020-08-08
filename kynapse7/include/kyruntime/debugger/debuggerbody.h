/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERENTITY_H
#define KYDEBUGGERENTITY_H

#include <kyruntime/debugger/debuggernetmessage.h>
#include <kyruntime/core/body.h>

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			template <>
			KyResult operator<<(Kaim::Debugger::Net::Message& message, const Kaim::Body& value);
		}
	}
}
#endif // KYDEBUGGERENTITY_H
