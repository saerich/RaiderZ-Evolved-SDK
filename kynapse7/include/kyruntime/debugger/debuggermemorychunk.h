/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERMEMORYCHUNK_H
#define KYDEBUGGERMEMORYCHUNK_H

#include <kyruntime/debugger/debuggernetmessage.h>
#include <kyruntime/debugger/debuggerbuffer.h>
			
namespace Kaim
{
	namespace Debugger
	{
		struct MemoryChunk
		{
			ptr_type m_address;
			Buffer m_bytes;
		};
	}
}	

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			template <>
			KyResult operator<<(Kaim::Debugger::Net::Message& message, const Kaim::Debugger::MemoryChunk& value);

			template <>
			KyResult operator>>(Kaim::Debugger::Net::Message& message, Kaim::Debugger::MemoryChunk& value);
		}
	}
}
#endif // KYDEBUGGERMEMORYCHUNK_H
