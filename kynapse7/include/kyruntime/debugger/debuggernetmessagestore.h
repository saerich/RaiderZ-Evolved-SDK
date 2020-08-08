/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETMESSAGESTORE_H
#define KYDEBUGGERNETMESSAGESTORE_H

#include <kyruntime/debugger/debuggertypes.h>
#include <kyruntime/debugger/debuggernetbuffer.h>
#include <kyruntime/debugger/debuggererrorutils.h>


namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			struct MessageStore : public VirtualMemObject
			{
				typedef ByteType ValueType;
				typedef NetBuffer BufferType;

				// Store
			protected:
				typedef Kaim::Deque<BufferType> StoreType;

			protected:
				BufferType	m_buffer;
				StoreType	m_store;
				SizeType	m_next_message_size;

				// Reader state
			protected:
				KyBool	m_initialized;
				KyBool	m_feeding;

			public:
				MessageStore();

			public:		
				/* initialize the message reader
				* \post IsInitialized()==true
				* \post IsFeeding()==false
				* \post IsEmpty()==true
				* \return generic error if anything goes wrong
				*/
				KyResult Initialize();

				/* release the message reader
				* \post IsInitialized()==false
				* \post IsFeeding()==false
				* \post IsEmpty()==true
				* \return generic error if anything goes wrong
				*/
				KyResult Terminate();

				/* clear the message reader
				* \pre IsInitialized()==true
				* \post IsFeeding()==false
				* \post IsEmpty()==true
				* \return generic error if anything goes wrong
				*/
				KyResult Clear();

				/* Push a byte in the buffer.
				* \param data byte to PushBack in the buffer
				* \pre IsInitialized()==true
				* \return generic error if anything goes wrong
				*/
				KyResult PushBack(ValueType data);

				/* Push a sequence in the buffer.
				* \param begin begin of sequence to PushBack in the buffer
				* \param end end of sequence to PushBack in the buffer
				* \pre IsInitialized()==true
				* \return generic error if anything goes wrong
				*/
				template <typename OtherIterator>
				KyResult PushBack(OtherIterator begin, OtherIterator end)
				{
					KY_FUNCTION("MessageStore::PushBack(OtherIterator begin, OtherIterator end)");

					KY_ASSERT((IsInitialized()==true), ("Invalid precondition"));

					for (OtherIterator it=begin; it!=end;it++)
					{
						KAIM_DEBUGGER_FORWARD_ERROR(PushBack((MessageStore::ValueType)*it));	
					}

					return Kaim::Result::Success;
				}

				/* PushBack current buffer in the store and clear it
				* \pre IsInitialized()==true
				* \return generic error if anything goes wrong
				*/
				KyResult  PushBuffer();

				// clear current buffer
				void ClearBuffer();

				/* Pop front (first received) message and return it
				* \pre IsInitialized()==true
				* \param [out] message front message, or null-string if the store is empty
				* \return generic error if anything goes wrong
				*/
				KyResult PopFront(BufferType& message);

				/* Pop back (last received) message and return it
				* \pre IsInitialized()==true
				* \param [out] message back message, or null-string if the store is empty
				* \return generic error if anything goes wrong
				*/
				KyResult PopBack(BufferType& message);

				/* Return front (first received) message
				* \pre IsInitialized()==true
				* \pre IsEmpty()==false
				*/
				const BufferType& Front() const;

				/* Return back (last received) message
				* \pre IsInitialized()==true
				* \pre IsEmpty()==false
				*/
				const BufferType& Back() const;

				/* Push a byte in the buffer.
				* \see MessageStore::PushBack(ValueType data)
				* \param data byte to PushBack in the buffer
				* \pre IsInitialized()==true
				* \return generic error if anything goes wrong
				*/
				KyResult operator<<(ValueType data);

				/* Push a sequence in the buffer.
				* \see MessageStore::PushBack(OtherIterator begin, OtherIterator end)
				* \param data sequence to PushBack in the buffer
				* \pre IsInitialized()==true
				* \return generic error if anything goes wrong
				*/
				KyResult operator<<(const Deque<ValueType>& data);

				/* Pop the first message.
				* \see MessageStore::PopFront(BufferType& message)
				* \param [out] message front message, or null-string if the store is empty
				* \pre IsInitialized()==true
				* \return generic error if anything goes wrong
				*/
				KyResult operator>>(BufferType& message);
			public:
				// return true if IsInitialized, else false
				bool IsInitialized() const;

				// return true if the buffer started to be feeded (e.g. the start byte has been seen)
				bool IsFeeding() const;

				// return true if the message store is empty
				bool IsEmpty() const;

				// return the current buffer
				const BufferType& Buffer() const;

				// return the current store size
				SizeType Size() const;
			};
		}
	}
}


#endif // KYDEBUGGERNETMESSAGESTORE_H
