/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETBUFFER_H
#define KYDEBUGGERNETBUFFER_H

#include <kyruntime/debugger/debuggertypes.h>
#include <kyruntime/debugger/debuggererrorutils.h>
#include <kyruntime/debugger/debuggernetbyteconverter.h>
#include <kyruntime/debugger/debuggertypes.h>

#include <kyruntime/debugger/debuggerbuffer.h>
#include <kyruntime/debugger/debuggernetarray.h>
#include <kypathdata/containers/deque.h>

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			struct NetBuffer : public Kaim::Deque<Kaim::Debugger::ByteType>
			{
				template <typename T>
				KyResult PeekFront(T& value)
				{	
					KY_FUNCTION("NetBuffer::PeekFront(T& value)");

					Kaim::Debugger::Net::net_byte_converter<T> converter;

					KAIM_DEBUGGER_FORWARD_ERROR(Kaim::Deque<Kaim::Debugger::ByteType>::PeekFront(converter.Begin(), converter.End()));

					value = converter.Value();

					return Kaim::Result::Success;
				}

				template <typename OtherIterator>
				KyResult PeekFront(OtherIterator _begin, OtherIterator _end)
				{
					return Kaim::Deque<Kaim::Debugger::ByteType>::PeekFront(_begin, _end);
				}
			};

			template <typename T>
			KyResult operator>>(Kaim::Debugger::Net::NetBuffer& buffer, T& value)
			{	
				KY_FUNCTION("operator>>(Kaim::Debugger::Net::NetBuffer& buffer, T& value)");

				Kaim::Debugger::Net::net_byte_converter<T> converter;

				KAIM_DEBUGGER_FORWARD_ERROR(buffer.PopFront(converter.Begin(), converter.End()));

				value = converter.Value();

				return Kaim::Result::Success;
			}

			template <typename T>
			KyResult operator<<(Kaim::Debugger::Net::NetBuffer& buffer, const T& value)
			{	
				KY_FUNCTION("operator<<(Kaim::Debugger::Net::NetBuffer& buffer, const T& value)");

				Kaim::Debugger::Net::net_byte_converter<T> converter(value);

				KAIM_DEBUGGER_FORWARD_ERROR(buffer.PushBack(converter.Begin(), converter.End()));

				return Kaim::Result::Success;
			}

			template <>
			KyResult operator>>(Kaim::Debugger::Net::NetBuffer& buffer, Kaim::Debugger::Net::NetBuffer& value);

			template <>
			KyResult operator<<(Kaim::Debugger::Net::NetBuffer& buffer, const Kaim::Debugger::Net::NetBuffer& value);


			template <>
			KyResult operator>>(Kaim::Debugger::Net::NetBuffer& buffer, Kaim::Debugger::Buffer& value);

			template <>
			KyResult operator<<(Kaim::Debugger::Net::NetBuffer& buffer, const Kaim::Debugger::Buffer& value);


			template <>
			KyResult operator>>(Kaim::Debugger::Net::NetBuffer& buffer, Kaim::Debugger::Net::NetArray& value);

			template <>
			KyResult operator<<(Kaim::Debugger::Net::NetBuffer& buffer, const Kaim::Debugger::Net::NetArray& value);

		}
	}
}

#endif // KYDEBUGGERNETBUFFER_H
