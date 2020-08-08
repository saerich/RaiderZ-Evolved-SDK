/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETMESSAGE_H
#define KYDEBUGGERNETMESSAGE_H


#include <kyruntime/debugger/debuggernetfieldcollection.h>

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			struct Message
			{
				typedef FieldCollection data_type;

				data_type m_data;

				// \return true if message is empty.
				bool IsEmpty() const
				{
					return m_data.IsEmpty();
				}
				// \return total length of message (header length + length of field collection).
				SizeType Length() const
				{
					SizeType size = sizeof(SizeType);
					size += m_data.Length();

					return size;
				}

				void Clear()
				{
					m_data.Clear();
				}
			};

			/* Push back value converted as a field in the message.
			* \return generic error if anything goes wrong
			*/
			template <typename T>
			KyResult operator<<(Kaim::Debugger::Net::Message& message, const T& value)
			{
				return message.m_data << value;
			}

			/* Pop front field from message and convert it to value.
			* \return generic error if anything goes wrong
			*/
			template <typename T>
			KyResult operator>>(Kaim::Debugger::Net::Message& message, T& value)
			{
				return message.m_data >> value;
			}	

			/* Push back message in buffer.
			* \return generic error if anything goes wrong
			*/
			template <>
			KyResult operator<<(Kaim::Debugger::Net::NetBuffer& buffer, const Kaim::Debugger::Net::Message& value);

			/* Pop message from front of buffer.
			* \return generic error if anything goes wrong
			*/
			template <>
			KyResult operator>>(Kaim::Debugger::Net::NetBuffer& buffer,  Kaim::Debugger::Net::Message& value);

			/* Push back message in buffer.
			* \return generic error if anything goes wrong
			*/
			template <>
			KyResult operator<<(Kaim::Debugger::Net::NetArray& buffer, const Kaim::Debugger::Net::Message& value);

			template<typename T>
			KyResult operator<<(Kaim::Debugger::Net::Message& message, const Kaim::Deque<T>& value)
			{
				KY_FUNCTION("operator<<(Kaim::Debugger::Net::Message& message, const Kaim::Deque<T>& value)");

				KAIM_DEBUGGER_FORWARD_ERROR(message << value.Size());
				
				for (Kaim::Debugger::SizeType i = 0; i < value.Size(); i++)
				{
					KAIM_DEBUGGER_FORWARD_ERROR(message << value[i]);
				}

				return Kaim::Result::Success;
			}

			template<typename T>
			KyResult operator>>(Kaim::Debugger::Net::Message& message, Kaim::Deque<T>& value)
			{
				KY_FUNCTION("operator>>(Kaim::Debugger::Net::Message& message, Kaim::Deque<T>& value)");

				Kaim::Debugger::SizeType size = 0;
				KAIM_DEBUGGER_FORWARD_ERROR(message >> size);

				value.Clear();
				for (Kaim::Debugger::SizeType i = 0; i < size; i++)
				{
					KAIM_DEBUGGER_FORWARD_ERROR( value.PushBack());
					KAIM_DEBUGGER_FORWARD_ERROR( message >> value.Back());
				}

				return Kaim::Result::Success;
			}		
		}
	}
}




#endif // KYDEBUGGERNETMESSAGE_H
