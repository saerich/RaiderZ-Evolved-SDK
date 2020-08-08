/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file 
	\ingroup kyruntime_debugger */

// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETSERVERCONFIG_H
#define KYDEBUGGERNETSERVERCONFIG_H

#include <kyruntime/debugger/debuggertypes.h>

namespace Kaim
{
	class Engine;

	namespace Debugger
	{
		namespace Net
		{
			/*! This object contains configuration parameters used to initialize the remote debugging framework
				through a call to RemoteDebugging::Initialize().
				\ingroup kyruntime_debugger */
			struct ServerConfig
			{
				// parameters
				/*! Specifies the size of the buffer allocated by the remote debugging framework for receiving network communications.
					\units			bytes
					\defaultvalue	1024 */
				SizeType m_netBufferSize;

				/*! Specifies the TCP/IP port to which the RemoteDebugging service will listen for incoming requests from the visual
					%Debugger interface. 
					\pp Users of the visual %Debugger interface need to supply this port in order to connect to the \gameOrSim engine.
					The default value used by the %Debugger interface is pre-set to match the default value of this parameter. If you
					set a new value for this parameter, you will have to provide the same value in the Host toolbar of your %Debugger
					interface.
					\acceptedvalues	any valid TCP/IP port number
					\defaultvalue	The value of ListeningPort */
				KyUInt16 m_listeningPort;

				// default integral values
				/*! Value assigned to #m_netBufferSize by the default constructor and by Initialize(). */
				static const SizeType NetBufferSize = 1024;
				
				/*! Value assigned to #m_listeningPort by the default constructor and by Initialize(). */
				static const KyUInt16 ListeningPort = 13000;

				/*! \label_init */
				void Initialize()
				{
					m_netBufferSize = NetBufferSize;
					m_listeningPort = ListeningPort;
				}

				/*! \label_constructor After using this constructor but before passing this object to RemoteDebugging::Initialize(),
					you can set new configuration values directly in the data members of this object. */
				ServerConfig()
				{
					Initialize();
				}
			};
		}
	}
}


#endif // KYDEBUGGERNETSERVERCONFIG_H

