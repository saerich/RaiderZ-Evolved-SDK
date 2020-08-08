/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file 
	\ingroup kyruntime_debugger */

// primary contact: ? - secondary contact: NOBODY
#ifndef KY_DEBUGGERLOGBUFFER_H
#define KY_DEBUGGERLOGBUFFER_H

#include <kyruntime/core/circulararray.h>

#include <kypathdata/generic/algorithm.h>
#include <kyruntime/debugger/debuggersharedlog.h>

namespace Kaim
{
	namespace Debugger
	{
		/*!	This structure represents a buffer that holds text messages. It is not used by any \SDKName components; 
			instead, it is provided as an optional convenience for use with the remote debugging framework. 
			\pp Because the log of debug messages sent to the remote %Debugger interface is maintained in a class member of 
			the RemoteDebugging service, the log cannot contain any messages generated before the initialization of that 
			service (i.e. before the Engine is initialized). If you need messages generated before that time to be visible 
			within the %Debugger interface, you can create an instance of the LogBuffer structure, fill it with the log 
			messages generated during the initialization of the Engine, and write the contents of the LogBuffer to the 
			remote debugging log as soon as the RemoteDebugging service becomes available.
			\pp This class allocates enough memory for 100 messages of 1024 bytes each. The list is circular: once the maximum 
			list size is exceeded, each new addition overwrites the oldest existing record.
			\pp Usage: \code
			// Initialize the LogBuffer.
			LogBuffer myBuffer;
			myBuffer.Initialize();
			...
			// Write messages to the buffer. To write the messages generated during Engine initialization, you
			// can include this call in an implementation of the ILogBridge interface that you set for your Bridge.
			myBuffer << "A message or variable to be printed to the %Debugger log";
			...
			// When the RemoteDebugging service becomes available, write the messages in the buffer to its log.
			Kaim::RemoteDebugging* rd = GetEngine().GetService<Kaim::RemoteDebugging>();
			if (rd)
			{
				rd->Log() << myBuffer;
			}
			...
			// When the buffer is no longer necessary, de-allocate the memory reserved for its messages.
			myBuffer.Terminate();
			\endcode
			\ingroup kyruntime_debugger */
		struct LogBuffer
		{
		public:
			/*!	Represents a single message stored in a LogBuffer.
				\ingroup kyruntime_debugger */
			struct Message
			{
			public:
				typedef char ValueType;
				typedef KyInt32 SizeType;

			public:
				/*!	1024 bytes are allocated for each message. */
				static const SizeType Size = 1024;

			public:
				/*!	Stores the contents of the message. */
				ValueType m_msg[Size];

			public:
				/*!	Sets the contents of the message.
					\param buffer			A pointer to a memory buffer that contains your message.
					\param size				The size of the memory buffer. */
				void Write(const ValueType* buffer, SizeType size)
				{
					Kaim::Copy(buffer, buffer + size, m_msg);
					m_msg[size] = ValueType();
				}
			};

		public:
			typedef Message MessageType;
			typedef KyInt32 SizeType;
			typedef CircularArray<MessageType> MessageContainerType;

		public:
			/*!	Enough memory space to hold 100 messages is reserved for each instance of this class. */
			static const SizeType Capacity = 100;

		public:
			/*!	Allocates memory for the messages contained by the buffer, and sets it up for initial use. */
			void Initialize()
			{
				m_messages.Reserve(Capacity);
			}

			/*!	De-allocates the message buffer when no longer needed. */
			void Terminate()
			{
				m_messages.Terminate();
			}

			/*!	Indicates whether or not the buffer has been successfully initialized and is ready for use. */
			bool IsInitialized() const
			{
				return m_messages.Capacity() > 0;
			}

			/*!	Adds a new message to the buffer.
				\param buffer			A pointer to a memory buffer that contains your message.
				\param size				The size of the memory buffer. */
			void Write(const Message::ValueType* buffer, Message::SizeType size)
			{
				if (IsInitialized() == true)
				{
					Message message;
					message.Write(buffer, size);
					m_messages.PushBack(message);
				}
			}

			/*!	Removes all messages from the buffer. */
			void Clear()
			{
				m_messages.Clear();
			}

		public:
			/*!	Retrieves a CircularArray list of Message objects used to store all messages added to this buffer.
				You can use this list to iterate through the messages that have been added to the buffer. */
			const MessageContainerType& Messages() const
			{
				return m_messages;
			}

		protected:
			MessageContainerType m_messages;
		};
	}
}
namespace Kaim
{
	namespace Debugger
	{
		/*!	Adds the specified string to the LogBuffer. \ingroup kyruntime_debugger */ 
		Kaim::Debugger::LogBuffer& operator<<(Kaim::Debugger::LogBuffer& buffer, const char* value);
	}
}

namespace Kaim
{
	namespace Debugger
	{
		namespace Shared
		{
			template <typename T>
			/*!	Adds all messages contained in the specified CircularArray object to the Log. \ingroup kyruntime_debugger */
			Kaim::Debugger::Shared::Log& operator<<(Kaim::Debugger::Shared::Log& buffer, const Kaim::CircularArray<T>& value);

			/*!	Adds the message contained in the specified LogBuffer::Message object to the Log. \ingroup kyruntime_debugger */
			Kaim::Debugger::Shared::Log& operator<<(Kaim::Debugger::Shared::Log& buffer, const Kaim::Debugger::LogBuffer::Message& value);

			/*!	Adds all messages contained in the specified LogBuffer object to the Log. \ingroup kyruntime_debugger */
			Kaim::Debugger::Shared::Log& operator<<(Kaim::Debugger::Shared::Log& buffer, const Kaim::Debugger::LogBuffer& value);
		}
	}
}

#endif // KY_DEBUGGERLOGBUFFER_H
