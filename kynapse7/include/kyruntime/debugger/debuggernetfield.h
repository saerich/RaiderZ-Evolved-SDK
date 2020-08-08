/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETFIELD_H
#define KYDEBUGGERNETFIELD_H

#include <kyruntime/debugger/debuggertypes.h>
#include <kyruntime/debugger/debuggernetbuffer.h>
#include <kypathdata/containers/kystring.h>
#include <kyruntime/debugger/debuggernetbyteconverter.h>
#include <kypathdata/generic/algorithm.h>
#include <kyruntime/debugger/debuggernetarray.h>

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			struct Field
			{
				typedef NetArray data_type;

				data_type m_data;

				bool IsEmpty() const
				{
					return m_data.IsEmpty();
				}

				SizeType Length() const
				{
					SizeType size = sizeof(SizeType);
					size += (SizeType)m_data.Size();

					return size;
				}

				KyResult operator<<(const data_type& value)
				{
					m_data = value;

					return Kaim::Result::Success;
				}

				KyResult operator<<(const char* value)
				{
					const SizeType size = StrLen(value);

					m_data.Resize(size);

					Kaim::Copy(value, value + size, m_data.Begin());
					
					return Kaim::Result::Success;
				}

				KyResult operator<<(const KyString& value)
				{
					m_data.Resize(value.Size());

					Kaim::Copy(value.Begin(), value.End(), m_data.Begin());

					return Kaim::Result::Success;
				}

				KyResult operator<<(const Buffer& value)
				{
					m_data.Resize(value.Size());
					Kaim::Copy(value.Begin(), value.End(), m_data.Begin());

					return Kaim::Result::Success;
				}

				template <typename T>
				KyResult operator<<(const T& value)
				{
					Net::net_byte_converter<T> converter(value);

					m_data.Resize(sizeof(T));

					Kaim::Copy(converter.Begin(), converter.End(), m_data.Begin());

					return Kaim::Result::Success;
				}

				KyResult operator>>(data_type& value)
				{
					value = m_data;

					return Kaim::Result::Success;
				}

				KyResult operator>>(KyString& value)
				{
					value.Resize(m_data.Size());

					Kaim::Copy(m_data.Begin(), m_data.End(), value.Begin());

					return Kaim::Result::Success;
				}

				KyResult operator>>(Buffer& value)
				{
					value.Clear();
					value.PushBack(m_data.Begin(), m_data.End());

					return Kaim::Result::Success;
				}

				template <typename T>
				KyResult operator>>(T& value)
				{
					if (m_data.Size() < sizeof(T))
						return -1;

					Net::net_byte_converter<T> converter(m_data.Begin());

					value = (T)converter.Value();

					return Kaim::Result::Success;
				}
			};

			KyResult operator<<(Kaim::Debugger::Net::NetBuffer& buffer, const Kaim::Debugger::Net::Field& value);
			KyResult operator>>(Kaim::Debugger::Net::NetBuffer& buffer, Kaim::Debugger::Net::Field& value);

			KyResult operator<<(Kaim::Debugger::Net::NetArray& buffer, const Kaim::Debugger::Net::Field& value);
		}
	}
}

#endif // KYDEBUGGERNETFIELD_H
