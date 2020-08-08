/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERBUFFER_H
#define KYDEBUGGERBUFFER_H

#include <kypathdata/containers/deque.h>
#include <kyruntime/debugger/debuggertypes.h>
#include <kyruntime/debugger/debuggererrorvalues.h>
#include <kyruntime/debugger/debuggererrorutils.h>
#include <kyruntime/debugger/debuggerbyteconverter.h>
#include <kyruntime/debugger/debuggertypes.h>

namespace Kaim
{
	namespace Debugger
	{
		struct Buffer : public Kaim::Deque<Kaim::Debugger::ByteType>
		{
			template <typename T>
			KyResult PeekFront(T& value)
			{	
				KY_FUNCTION("Buffer::PeekFront(T& value)");

				Kaim::Debugger::byte_converter<T> converter;

				KAIM_DEBUGGER_FORWARD_ERROR(Kaim::Deque<Kaim::Debugger::ByteType>::PeekFront(converter.begin(), converter.end()));

				value = converter.Value();

				return Kaim::Result::Success;
			}
		};

		template <typename T>
		KyResult operator>>(Kaim::Debugger::Buffer& buffer, T& value)
		{	
			KY_FUNCTION("operator>>(Kaim::Debugger::Buffer& buffer, T& value)");

			Kaim::Debugger::byte_converter<T> converter;

			KAIM_DEBUGGER_FORWARD_ERROR(buffer.PopFront(converter.begin(), converter.end()));

			value = converter.Value();

			return Kaim::Result::Success;
		}

		template <typename T>
		KyResult operator<<(Kaim::Debugger::Buffer& buffer, const T& value)
		{	
			KY_FUNCTION("operator<<(Kaim::Debugger::Buffer& buffer, const T& value)");

			Kaim::Debugger::byte_converter<T> converter;
			converter.Value(value);

			KAIM_DEBUGGER_FORWARD_ERROR(buffer.PushBack(converter.begin(), converter.end()));

			return Kaim::Result::Success;
		}

		template <>
		KyResult operator>>(Kaim::Debugger::Buffer& buffer, Kaim::Debugger::Buffer& value);

		template <>
		KyResult operator<<(Kaim::Debugger::Buffer& buffer, const Kaim::Debugger::Buffer& value);
	}
}

#endif // KYDEBUGGERBUFFER_H
