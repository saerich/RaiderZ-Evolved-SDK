/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETSOCKET_H
#define KYDEBUGGERNETSOCKET_H

#include <kyruntime/debugger/debuggertypes.h>

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			namespace details
			{
				struct SocketImpl;
			}

			struct Socket
			{
				Socket();
				// Close socket, if any
				~Socket();

			public:
				/* Set implementation.
				*	\post IsInitialized() == true
				*/
				KyResult Initialize();

				/* Destroy implementation.
				*	\post IsInitialized() == false
				*	\post IsValid() == false
				*/
				void Terminate();

			public:
				/* Close socket.
				*	\pre IsInitialized() == true
				*	\post IsValid() == false
				*/
				KyResult Close();

			public:
				// \return true if socket implementation is initialized.
				bool IsInitialized() const;

				/* Valid state observer.
				*	\pre IsInitialized() == true
				*	\return true if socket is valid.
				*/
				bool IsValid() const;

			public:
				/* Set the blocking mode.
				*	\pre IsInitialized() == true
				*	\pre IsValid() == true
				*/
				KyResult Blocking(bool blocking);

				bool IsBlocking() const;

			protected:
				details::SocketImpl* pimpl;
				friend struct details::SocketImpl;
			};

			struct ListeningSocket : Socket
			{
				/*	Create Socket.
				*	\pre IsInitialized() == true
				*	\post IsValid() == true
				*/
				KyResult Socket();

				/*	Bind to specified port.
				*	\pre IsInitialized() == true
				*	\pre IsValid() == true
				*/
				KyResult Bind(KyUInt16 port);

				/*	Listen at incoming connection.
				*	\pre IsInitialized() == true
				*	\pre IsValid() == true
				*/
				KyResult Listen();

				/*	Accept connection and return the new socket 
				*	\pre IsInitialized() == true
				*	\pre IsValid() == true
				*	\pre socket.IsInitialized() == true
				*/
				KyResult Accept(Kaim::Debugger::Net::Socket& socket) const;
			};

			struct AcceptedSocket : Socket
			{
				/* Receive bytes and fill the buffer.
				*	\pre IsInitialized() == true
				*	\pre IsValid() == true
				*/
				KyResult Receive(ByteType* buffer, KyInt32 length, KyInt32& received) ;

				/* Send bytes from the buffer.
				*	\pre IsInitialized() == true
				*	\pre IsValid() == true
				*/
				KyResult Send(const ByteType* buffer, KyInt32 length, KyInt32& sent) ;
			};
		}
	}
}


#endif // KYDEBUGGERNETSOCKET_H

