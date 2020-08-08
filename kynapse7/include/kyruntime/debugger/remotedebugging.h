/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup debugger */

// primary contact: ? - secondary contact: LAPA
#ifndef KYREMOTEDEBUGGING_H
#define KYREMOTEDEBUGGING_H

#include <kyruntime/core/service.h>

// shared data
#include "kyruntime/debugger/debuggersharedcamera.h"
#include "kyruntime/debugger/debuggershareddrawing.h"
#include "kyruntime/debugger/debuggersharedlog.h"
#include <kyruntime/debugger/debuggernetserverconfig.h>

#include "kyruntime/core/engine.h"

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			class Server;
		}
	}

	/*!	RemoteDebugging is a class of EngineService that embeds the \SDKName remote visual debugging
		framework into your \gameOrSim engine.
		\pp For details, see :USERGUIDE:"Remote Visual Debugging".
		\ingroup debugger */
	class RemoteDebugging : public Kaim::EngineService
	{

	public:
		KY_DECLARE_ENGINESERVICE(RemoteDebugging)

	// EngineService interface
	public:
		/*! \label_init
			\param config		An instance of the Debugger::Net::ServerConfig class that contains configuration
								values for the remote debugging framework.
			\post IsInitialized() == true */
		KyResult Initialize(const Debugger::Net::ServerConfig& config);

		/*! \label_init Uses a Debugger::Net::ServerConfig object created using its default constructor
			with default configuration values.
			\post IsInitialized() == true */
		KyResult Initialize()
		{
			return Initialize(Debugger::Net::ServerConfig());
		}

		/*!	\copydoc EngineService::PostUpdate()
			\pp This implementation of EngineService::PostUpdate() processes requests sent by the visual 
			%Debugger interface: it sends requested blocks of memory to the %Debugger, and handles
			requests to pause and un-pause the \gameOrSim engine. */
		virtual void PostUpdate();

		virtual void Update();

		/*!	\label_clear The RemoteDebugging service will no longer 
			listen at its assigned port for incoming requests from the visual %Debugger interface.
			\post IsInitialized() == false */
		void Clear();

	// States
	public:	
		/*!	Indicates the initialization state of the RemoteDebugging service.
			\return #true if this service is initialized and ready for use, or #false otherwise. */
		bool IsInitialized() const;

		/*!	Indicates whether or not the remote Debugger interface is currently connected to the \gameOrSim application. */
		bool IsConnected() const;

	// Global variables
	public:
		/*!	Registers a variable to be tracked by the remote debugging framework.
			\param name			The name of the variable. 
			\param address		The address in memory of the variable.
			\param type			The fully qualified type name of the variable.
			\pre IsInitialized()
			\return #Kaim::Result::Success if the variable was successfully registered. */
		KyResult RegisterVariable(const char* name, void* address, const char* type);

	// Shared variables helpers
	public:	
		/*!	Sends the position and orientation of the \gameOrSim camera to the visual %Debugger interface.
			\param position		The position of the camera in global space.
			\param right		The Right vector of the camera.
			\param up			The Up vector of the camera.
			\param forward		The Forward vector of the camera.
			\pre IsInitialized() */
		void UpdateCamera(const Kaim::Vec3f& position, const Kaim::Vec3f& right, const Kaim::Vec3f& up, const Kaim::Vec3f& forward);

	// Callbacks
	public:
		/*!	This interface defines an event handler for managing pause and resume operations.
			\pp If you need to carry out special actions before or after the visual %Debugger interface 
			pauses the thread running the \SDKName Engine, you can create a new class that implements the 
			IPauseHandler interface, and re-implement the necessary virtual methods in your class.
			\pp Set your pause handler for the RemoteDebugging service by calling RemoteDebugging::SetPauseHandler().
			\ingroup debugger */
		struct IPauseHandler
		{
			/*!	This method is called by the RemoteDebugging service each time the %Debugger requests
				the \gameOrSim to be paused.
				\pp You should implement this method if your \gameOrSim needs to perform any custom actions in 
				order to be paused, such as pausing other threads or temporarily disabling dead-reckoning 
				systems.
				\return This method must return #Kaim::Result::Success if the operation succeeded. 
						If this method failed, the RemoteDebugging service will not be updated in 
						the current frame. */
			virtual KyResult OnBeginPauseCallback() = 0;

			/*!	This method is called by the RemoteDebugging service each time the %Debugger requests
				the \gameOrSim to be resumed after a pause.
				\pp You should implement this method if your \gameOrSim needs to perform any custom actions in 
				order to recover from a pause, such as re-activating other threads or re-enabling 
				dead-reckoning systems.
				\return This method must return #Kaim::Result::Success if the operation succeeded. 
						If this method failed, the RemoteDebugging service will not be updated in 
						the current frame. */
			virtual KyResult OnEndPauseCallback() = 0;

			/*!	This method is called by the RemoteDebugging service if it encounters an error during 
				a frame in which it is paused.
				\pp Your implementation of this method should perform any tasks you want your \gameOrSim to 
				perform in order to recover from such an error. For example, if you want to continue 
				your \gameOrSim session normally, your implementation might be very similar to OnEndPauseCallback(). 
				Alternatively, you may prefer to log the error and quit your \gameOrSim.*/
			virtual void OnPauseErrorCallback() = 0;

		public:
			virtual ~IPauseHandler()
			{
			}
		};

	public:
		/*!	Registers a handler for pause and resume events. See the IPauseHandler class.
			\param handler		The object to register. Must be an instance of a class that implements IPauseHandler,
								or #KY_NULL (equal to 0). */
		void SetPauseHandler(IPauseHandler* handler)
		{
			m_pPauseHandler = handler;
		}

	protected:
		IPauseHandler* m_pPauseHandler;

	public:

		/*!	This virtual method is called each frame, before sending data to the %Debugger interface. 
			You can implement this method in a custom class you derive from RemoteDebugging, to 
			provide the RemoteDebugging service with your camera position and the lists of debug 
			points and lines that should be drawn in the %Debugger interface. 
			\return #Kaim::Result::Success if the method succeeded. */
		virtual KyResult OnUpdateSharedMembers()
		{
			return Kaim::Result::Success;
		}

	// shared members read by the remote debugger
	public:
		Kaim::Debugger::Shared::Camera m_camera; /*!< Stores the position of the \gameOrSim camera. Do not modify directly. Access through UpdateCamera(). */
		Kaim::Debugger::Shared::Drawing m_drawing; /*!< Stores the debug lines and points to be drawn in the %Debugger. Do not modify directly. Access through GetDrawBridge(). */
		Kaim::Debugger::Shared::Log m_log; /*!< Stores the list of debug messages to be printed in the %Debugger. Do not modify directly. Access through Log(). */

	public:
		/*!	Accesses the log of messages that will be sent to the remote %Debugger and printed on its
			Messages tab. You can use this object to add your own messages to the log. */
		Kaim::Debugger::Shared::Log& Log()
		{
			return m_log;
		}

	// Draw bridge
	public:
		/*!	This implementation of IDrawBridge adds debug points and lines to lists maintained by the RemoteDebugging service. Each
			frame, the RemoteDebugging service sends the points and lines in its lists to the visual %Debugger interface for drawing
			in its 3D View, and clears the lists for the next frame.
			\pp The RemoteDebugging service initializes and maintains an instance of this class, which you can access through 
			RemoteDebugging::GetDrawBridge().
			\ingroup debugger */
		class DrawBridge : public Kaim::IDrawBridge
		{
		public:
			/*!	\copydoc IDrawBridge::Point()
				\pp This implementation of IDrawBridge::Point() adds the specified point to the list of debug points that will be sent
				to the remote %Debugger for rendering. */
			virtual void Point(const Kaim::Vec3f& p, KyUInt32 r, KyUInt32 g, KyUInt32 b, KyUInt32 a);

			/*!	\copydoc IDrawBridge::Line()
				\pp This implementation of IDrawBridge::Line() adds the specified line segment to the list of debug lines that will be sent
				to the remote %Debugger for rendering. */
			virtual void Line(const Kaim::Vec3f& p1, const Kaim::Vec3f& p2, KyUInt32 r, KyUInt32 g, KyUInt32 b, KyUInt32 a);

			/*!	\copydoc IDrawBridge::Text()
				\pp This method is not currently implemented in this class. */
			virtual void Text(const Kaim::Vec3f& p, KyUInt32 r, KyUInt32 g, KyUInt32 b, KyUInt32 a, const char* text);
		protected:
			friend class Kaim::RemoteDebugging;
			RemoteDebugging* m_owner;
		};

	protected:
		/*!	\label_baseconstructor */
		explicit RemoteDebugging(Engine& engine);

		/*!	\label_virtualdestructor */
		virtual ~RemoteDebugging();

		DrawBridge m_drawBridge;

	public:
		/*!	Retrieves the instance of the RemoteDebugging::DrawBridge() class maintained by the RemoteDebugging service.
			You can use this object to send debug points and lines to the visual %Debugger, to be drawn in the 
			3D View of the visual %Debugger interface. */
		DrawBridge& GetDrawBridge()
		{
			return m_drawBridge;
		}

	protected:
		// Debugging server
		Kaim::Debugger::Net::Server* m_pServer; //< Debugging server. Do not modify.
		KyBool m_isMemoryRequested; //< For internal use. Do not modify.
	};
}

#endif // KYREMOTEDEBUGGING_H
