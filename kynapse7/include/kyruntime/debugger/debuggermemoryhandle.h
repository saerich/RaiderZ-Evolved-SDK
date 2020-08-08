/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERMEMORYHANDLE_H
#define KYDEBUGGERMEMORYHANDLE_H

#include <kyruntime/debugger/debuggernetmessage.h>
			
namespace Kaim
{
	namespace Debugger
	{
		struct MemoryHandle
		{
			ptr_type m_address;
			ptr_size_type m_length;
		};

	}
}

// remove warning in allocator_adapter<>::destroy : no destructor is generated for MemoryHandle so destroy is a no-op hence parameter ptr is not used
namespace Kaim
{
	template <>
	void allocator_adapter<Kaim::Debugger::MemoryHandle>::destroy(Kaim::allocator_adapter<Kaim::Debugger::MemoryHandle>::pointer);
}

template struct Kaim::allocator_adapter<Kaim::Debugger::MemoryHandle>;

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			template <>
			KyResult operator<<(Kaim::Debugger::Net::Message& message, const Kaim::Debugger::MemoryHandle& value);

			template <>
			KyResult operator>>(Kaim::Debugger::Net::Message& message, Kaim::Debugger::MemoryHandle& value);
		}
	}
}
#endif // KYDEBUGGERMEMORYHANDLE_H
