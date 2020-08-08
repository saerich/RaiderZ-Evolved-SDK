/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGEROBJECTHANDLE_H
#define KYDEBUGGEROBJECTHANDLE_H

#include <kyruntime/debugger/debuggernetmessage.h>
#include <kypathdata/containers/kystring.h>
			
namespace Kaim
{
	namespace Debugger
	{
		struct ObjectHandle
		{
			typedef KyString string_type;
			typedef ptr_type address_type;

			string_type m_name;
			address_type m_address;
			string_type m_type;

			void Swap(ObjectHandle& other)
			{
				Kaim::Swap(m_name, other.m_name);
				Kaim::Swap(m_address, other.m_address);
				Kaim::Swap(m_type, other.m_type);
			}

			// Move semantic
			KyResult operator>>(Kaim::Deque<ObjectHandle>& container)
			{
				KY_FUNCTION("ObjectHandle::operator>>(Kaim::Deque<ObjectHandle>& container)");

				KAIM_DEBUGGER_FORWARD_ERROR(container.PushBack());
				Swap(container.Back());

				return Kaim::Result::Success;
			}
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
			KyResult operator<<(Kaim::Debugger::Net::Message& message, const Kaim::Debugger::ObjectHandle& value);

			template <>
			KyResult operator>>(Kaim::Debugger::Net::Message& message, Kaim::Debugger::ObjectHandle& value);
		}
	}
}
#endif // KYDEBUGGEROBJECTHANDLE_H
