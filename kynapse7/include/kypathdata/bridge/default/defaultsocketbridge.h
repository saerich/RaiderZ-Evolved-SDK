/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KSSOCKETBRIDGE_H
#define KSSOCKETBRIDGE_H

#include <kypathdata/bridge/isocketbridge.h>

namespace Kaim
{
	/*!	This class is the default implementation of the ISocketBridge interface created by the Bridge at its
		initialization.
		\pp This class is implemented only for platforms supported by the remote visual debugging
		framework. This page shows the declaration and implementation for 32-bit Windows platforms. 
		For details on other platforms, see the source code files in the \c kypathdata/bridge/default
		subdirectory.
		\ingroup kypathdata_bridge */
	class DefaultSocketBridge : public Kaim::ISocketBridge
	{
	public:
		/*!	\label_baseconstructor */
		DefaultSocketBridge();
		
		/*!	\label_virtualdestructor */
		virtual ~DefaultSocketBridge();

		virtual KyResult Init();

		virtual KyResult Terminate();

	public:
		virtual KyResult LastError() const;

		virtual	bool IsValid(handle_type handle) const;


		virtual handle_type Create(KyInt32 family, KyInt32 type, KyInt32 protocol);

		virtual	KyResult Accept(handle_type handle, SockAddr* addr, handle_type& out_handle);

		virtual KyResult Close(handle_type handle);

		virtual	KyResult Bind(handle_type handle, const SockAddr* addr);

		virtual	KyResult Listen(handle_type handle, KyInt32 backlog);

		virtual	KyResult Connect(handle_type handle, const SockAddr* addr);

		virtual KyResult Send(handle_type handle,const char* buffer, KyInt32 length, KyInt32 flags, KyInt32& sent);

		virtual KyResult Receive(handle_type handle, char* buffer, KyInt32 length, KyInt32 flags, KyInt32& received);

		virtual	KyResult Blocking(handle_type handle, bool blocking);
	};
}

#endif // KSSOCKETBRIDGE_H
