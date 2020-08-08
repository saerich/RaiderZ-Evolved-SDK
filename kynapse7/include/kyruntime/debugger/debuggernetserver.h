/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETSERVER_H
#define KYDEBUGGERNETSERVER_H

#include <kyruntime/debugger/debuggernetsocket.h>
#include <kyruntime/debugger/debuggertypes.h>
#include <kyruntime/debugger/debuggernetmessagestore.h>
#include <kyruntime/debugger/debuggerobjecthandle.h>
#include <kyruntime/debugger/debuggermemoryhandle.h>
#include <kyruntime/debugger/debuggerlocalmemoryreader.h>
#include <kyruntime/debugger/debuggernetarray.h>
#include <kyruntime/debugger/debuggerdump.h>
#include <kyruntime/debugger/debuggernetserverconfig.h>

namespace Kaim
{
	class Engine;
	class Body;
	class IPathObject;

	namespace Debugger
	{
		namespace Net
		{
			class NetMessage;

			namespace Result
			{
				static const KyResult unknown_message = -201;
			}

			class Server
			{
				KY_DEFINE_NEW_DELETE_OPERATORS

			public:
				// \post IsInitialized() == false
				// \post IsConfigured() == false
				// \post IsListening() == false
				// \post IsConnected() == false
				// \post IsPaused() == false
				Server(Engine& engine); 

				// \post IsConfigured() == true
				KyResult Configure(const ServerConfig& config);

				// \post IsConfigured() == true
				KyResult Configure();

				// \post IsInitialized() == true
				// \post IsListening() == false
				// \post IsConnected() == false
				// \post IsPaused() == false
				KyResult Initialize();

				// \post IsInitialized() == false
				// \post IsListening() == false
				// \post IsConnected() == false
				// \post IsPaused() == false
				void Terminate();


				ServerConfig& Config()
				{
					return m_config;
				}

				const ServerConfig& Config() const
				{
					return m_config;
				}

				KyResult RegisterVariable(const char* name, void* address, const char* type);

				// \pre IsInitialized() == true
				// \post IsListening() == true
				KyResult Listen();

				// \pre IsListening() == true
				KyResult Run();

			// States
			public:
				bool IsInitialized() const
				{
					return m_acceptedSocket.IsInitialized() 
						&& m_listeningSocket.IsInitialized()
						&& m_message_store.IsInitialized();
				}

				bool IsConfigured() const
				{
					return m_configured == KY_TRUE;
				}

				// \pre IsInitialized() == true
				bool IsListening() const
				{
					return m_listeningSocket.IsValid();
				}

				// \pre IsInitialized() == true
				bool IsConnected() const
				{
					return m_acceptedSocket.IsValid();
				}

				bool IsPauseRequested() const
				{
					return m_pausedRequested == KY_TRUE;
				}

			public:
				bool IsMemoryRequested() const
				{
					return (m_memoryHandleCollection.IsEmpty() == false);
				}

				bool IsMemoryRequested(const void* address, ptr_size_type length) const;

			protected:
				KyResult Accept();

				KyResult Dispatch(Message& message);

				KyResult Push(const Message& message);

				KyResult Dump(const Message& message);
				KyResult DumpFrame();

				// Generic messages
				KyResult ReadRequestTypeVirtualTables(Message& message);
				KyResult ReadRequestObjects(Message& message);
				KyResult ReadRequestMemory(Message& message);
				// Helper messages
				KyResult ReadRequestBodies(Message& message);
				KyResult ReadRequestPathObjects(Message& message);

				// Generic messages
				KyResult WriteResponseTypeVirtualTables(Message& message) const;
				KyResult WriteResponseObjects(Message& message) const;
				KyResult WriteResponseMemory(Message& message) ;
				// Helper messages
				KyResult WriteResponseWorlds(Message& message) const;
				KyResult WriteResponseBodies(Message& message) const;
				KyResult WriteResponsePathObjects(Message& message) const;
				
				KyResult RequestPauseMode(bool pauseRequested);
				KyResult SetPauseMode();

			public:
				/*!	\label_getengine */
				Engine& GetEngine();

				/*!	\label_getengine */
				const Engine& GetEngine() const;

			protected:
				Engine* m_engine;

			protected:
				ServerConfig m_config;
				KyBool m_configured;

			protected:
				ListeningSocket m_listeningSocket;
				AcceptedSocket m_acceptedSocket;

			protected:
				NetArray m_received_byte_buffer;
				MessageStore m_message_store;

			protected:			
				NetArray m_sending_byte_buffer;
				KyInt32 m_sending_byte_buffer_offset;

			protected:
				Deque<ObjectHandle> m_objectHandleCollection;
				Deque<MemoryHandle> m_memoryHandleCollection;

			protected:
				LocalMemoryReader m_memoryReader;

			protected:		
				const List<Body*>* m_bodies;
				const List<IPathObject*>* m_pathObjects;

			protected:
				KyBool m_pausedRequested;

			protected:
				KyInt32 m_memoryTick;

			protected:
				Kaim::Debugger::Dump m_dump;

			protected:
				KyInt32 m_dumpFileFrameCount;
				KyInt32 m_dumpFileFrameStart;
				KyInt32 m_dumpFileFrameCurrent;

			protected:
				// Persistent to avoid reallocations
				NetBuffer m_message_buffer;

				Kaim::Debugger::Net::Message m_request_message;
				Kaim::Debugger::Net::Message m_response_message;
				Deque<Kaim::Debugger::NetPtrType> m_adresses;

				KyString request_verb;
			};
		}
	}
}


#endif // KYDEBUGGERNETSERVER_H

