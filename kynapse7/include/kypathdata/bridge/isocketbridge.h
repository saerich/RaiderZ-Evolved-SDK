/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KY_ISOCKETBRIDGE_H
#define KY_ISOCKETBRIDGE_H

#include "kypathdata/generic/types.h"

namespace Kaim
{
	namespace Result
	{
		/*!	Defines specific errors that may be returned by ISocketBridge.
			\ingroup kypathdata_bridge */
		namespace Socket
		{
			/*!	Indicates that a connection could not be established to a remote endpoint.
				\ingroup kypathdata_bridge */
			static const KyResult WouldBlockError = -10000;
			/*!	Indicates that a connection to a remote endpoint has been terminated.
				\ingroup kypathdata_bridge */
			static const KyResult Disconnected = WouldBlockError - 1;
		}
	}

	/*!	Defines an interface used by \SDKName components to manage network connections.
		\pp The Bridge maintains an instance of a class that implements this interface, making the interface available to other
		\SDKName components. By default, the Bridge uses an instance of the DefaultSocketBridge class. You can also write
		your own implementation of ISocketBridge, and assign an instance of your class to the Bridge.
		\pp See :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class ISocketBridge
	{
	public:
		/*!	\label_virtualdestructor */
		virtual ~ISocketBridge() {}

		/*!	When the BaseSystem is configured to manage the network interface offered by the target platform and required
			by the remote debugging framework, it calls this method at initialization time to initialize the network
			interface. */
		virtual KyResult Init() = 0;
		/*!	When the BaseSystem is configured to manage the network interface offered by the target platform and required
			by the remote debugging framework, it calls this method when it is terminated in order to close the network
			interface. */
		virtual KyResult Terminate() = 0;

	public:
		typedef KyInt32 handle_type;

		/*!	Portable mapping of the \c in_addr structure.
			\ingroup kypathdata_bridge */
		struct InAddr
		{
			KyUInt32 m_addr;
		};

		/*!	Portable mapping of the \c sockaddr_in structure.
			\ingroup kypathdata_bridge */
		struct SockAddr
		{
			KyInt16   m_family;
			KyUInt16 m_port;
			InAddr m_addr;

			KyUInt8 m_zero[8];
		};

		typedef SockAddr sockaddr_in_type;
		typedef SockAddr sockaddr_type;

		/*!	Possible values for the address family.
			\ingroup kypathdata_bridge */
		struct AddressFamily
		{
			enum
			{
				// AF_INET
				inet = 2
			};
		};

		/*!	Possible values for the socket type.
			\ingroup kypathdata_bridge */
		struct Type
		{
			enum
			{
				// SOCK_STREAM
				stream = 1
			};
		};

		/*!	Possible values for the protocol to be used.
			\ingroup kypathdata_bridge */
		struct Protocol
		{
			enum
			{
				ip = 0
			};
		};

		/*!	Portable mapping of an invalid socket.
			\ingroup kypathdata_bridge */
		static const handle_type Invalid = -1;

		/*!	Portable mapping of \c INADDR_ANY.
			\ingroup kypathdata_bridge */
		static const KyUInt32 AddressAny = 0;

		/*!	Retrieves the last error encountered by the socket management system, or KyResult::Success if no error 
			has yet been encountered. */
		virtual KyResult LastError() const = 0;

		/*!	Creates a socket.
			\param family				Currently only \c AddressFamily::inet must be supported.
			\param type					Currently only \c AddressFamily::inet must be supported.
			\param protocol				Currently only \c AddressFamily::inet must be supported.
			\return A valid socket handle, or Invalid if an error occurred. */
		virtual handle_type Create(KyInt32 family, KyInt32 type, KyInt32 protocol) = 0;

		/*!	Permits an incoming connection attempt on this socket.
			\pre IsValid(handle) == true
			\param handle						Socket identifier.
			\param addr							Currently only NULL must be supported.
			\param[out] out_handle				Used to store the socket handle if Result::Success is returned.
			\retval Result::Success				Indicates that the operation was completed successfully.
			\retval Result::WouldBlockError		Indicates that the socket is marked as nonblocking and the 
												receive operation would block. */
		virtual	KyResult Accept(handle_type handle, SockAddr* addr, handle_type& out_handle) = 0;

		/*!	Closes the specified socket.
			\pre IsValid(handle) == true
			\param handle				Socket identifier.
			\return Result::Success if the operation was completed successfully. */
		virtual KyResult Close(handle_type handle) = 0;

		/*!	Binds a socket to the specified address and port.
			\pre IsValid(handle) == true
			\param handle				Socket identifier.
			\param addr					Description of the endpoint address to which to connect.
			\return Result::Success if the operation was completed successfully. */
		virtual	KyResult Bind(handle_type handle, const SockAddr* addr) = 0;

		/*!	Starts listening for incoming messages on the specified socket.
			\pre IsValid(handle) == true
			\param handle				Socket identifier.
			\param backlog				Specifies the length of the connection backlog queue.
			\return Result::Success if the operation was completed successfully. */
		virtual	KyResult Listen(handle_type handle, KyInt32 backlog) = 0;

		/*!	Connects the specified socket to the specified address and port.
			\pre IsValid(handle) == true
			\param handle				Socket identifier.
			\param addr					Description of the endpoint address to which to connect.
			\return Result::Success if the operation was completed successfully. */
		virtual	KyResult Connect(handle_type handle, const SockAddr* addr) = 0;

		/*! Sends bytes over a socket.
			\pre IsValid(handle) == true
			\param handle				Socket identifier.
			\param buffer				Memory buffer containing the bytes to be sent.
			\param length				Size of the \p buffer argument.
			\param flags				Mask value determining the socket flags to be used in the communication.  
			\param sent					Number of bytes sent.
			\retval Result::Success					Indicates that the operation was completed successfully.
			\retval Result::Socket::WouldBlockError		Indicates that the socket is marked as nonblocking and the 
														receive operation would block.
			\retval Result::Socket::Disconnected		Indicates that the socket is not connected. */
		virtual KyResult Send(handle_type handle,const char* buffer, KyInt32 length, KyInt32 flags, KyInt32& sent) = 0;

		/*!	Receives bytes over a socket.
			\pre IsValid(handle) == true
			\param handle							Socket identifier.
			\param buffer							Memory buffer used to store the received bytes.
			\param length							Size of the \p buffer argument.
			\param flags							Mask value determining the socket flags to be used in the communication.
			\param received							Number of bytes received.
			\retval Result::Success					Indicates that the operation was completed successfully.  
			\retval Result::Socket::WouldBlockError	Indicates that the socket is marked as nonblocking and the receive operation would block.
			\retval Result::Socket::Disconnected	Indicates that the socket is not connected. */
		virtual KyResult Receive(handle_type handle, char* buffer, KyInt32 length, KyInt32 flags, KyInt32& received) = 0;

		/*!	Sets the specified socket as blocking or non-blocking.
			\pre IsValid(handle) == true
			\param handle				Socket identifier.
			\param blocking				Indicates whether the socket should be marked as blocking (#KY_TRUE) or non-blocking (#KY_FALSE). */
		virtual	KyResult Blocking(handle_type handle, bool blocking) = 0;

		/*!	Indicates whether or not the specified socket handle is valid.
			\param handle				Socket identifier. */
		virtual	bool IsValid(handle_type handle) const = 0;
	};

} // namespace Kaim


#endif // KY_ISOCKETBRIDGE_H
