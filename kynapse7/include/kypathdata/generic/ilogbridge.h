/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_generic */

#ifndef KY_ILOGBRIDGE_H
#define KY_ILOGBRIDGE_H

#include "kypathdata/generic/types.h"

namespace Kaim
{
	/*!	Enumerates the different levels of debug messages that can be logged through the #KY_MESSAGE and 
		#KY_COND_MESSAGE macros.
		\ingroup kypathdata_generic */
	enum LogLevel
	{
		/*!	Provides feedback about the current state of the system, 
			or an operation that has been completed successfully. */
		KY_LOG_MESSAGE,

		/*!	Indicates that a component may exhibit sub-optimal performance due to its current configuration. This 
			type of message typically indicates steps that you can take to correct the condition. */
		KY_LOG_PERF_WARNING,

		/*!	Indicates a problem that may result in a component not operating in the way you expect, but from 
			which the component can recover. For example, this type of message can be generated when a configuration 
			parameter is set to an invalid value, and must revert to its default value instead. */
		KY_LOG_WARNING,

		/*!	Indicates a problem from which a component cannot recover. For example, this type of message can 
			be generated at initialization time, when a requirement of the component, such as a mandatory service or 
			time management task, is not available. */
		KY_LOG_ERROR,

		/*!	Reserved for serious problems that compromise the functionality or stability of the system. */
		KY_LOG_ASSERT,

		KY_LastLogLevel,
		KY_LOG_dummy = KYFORCEENUMSIZEINT
	};

	/*! Enumerates the possible output locations for printing debug messages. These flags are used to construct a bitmask
		that you pass to DefaultLogBridge::SetLogMask().
		\ingroup kypathdata_bridge */
	enum LogMode
	{
		/*! Debug messages will be logged to a text file named <tt>kyoutput.txt</tt>. */
		KY_LOG_MODE_FILE = 1,

		/*! Debug messages will be logged to \c stdout. 
			\pp On Windows platforms, this flag is ignored if #KY_LOG_MODE_IDE is also used. */
		KY_LOG_MODE_STDOUT = 2,

		/*! Debug messages will be logged to the IDE if available: e.g. Visual Studio, ProDG, etc. */
		KY_LOG_MODE_IDE = 4,

		/*! Debug messages will be logged to all available output locations. */
		KY_LOG_MODE_ALL = KY_LOG_MODE_FILE | KY_LOG_MODE_STDOUT | KY_LOG_MODE_IDE
	};

	/*!	Defines an interface used by \SDKName components to log debug messages.
		\pp The Bridge maintains an instance of a class that implements this interface, making the interface available to other
		\SDKName components. By default, the Bridge uses an instance of the DefaultLogBridge class. You can also write
		your own implementation of ILogBridge, and assign an instance of your class to the Bridge.
		\pp See :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_generic */
	class ILogBridge
	{
	private:
		static ILogBridge* s_instance;

	public:
		/*!	Retrieves the instance of the ILogBridge currently maintained by the Bridge. */
		static ILogBridge* GetInstance() { return s_instance; }

		/*!	For internal use. Use Bridge::ReplaceLogBridge() instead. */
		static void SetInstance(ILogBridge* instance) { s_instance = instance; }

	public:
		/*!	\label_virtualdestructor */
		virtual ~ILogBridge() {}

	public:
		/*!	Formats and logs the specified message.
			\param file					The file from which the message is being sent.
			\param line					The line number at which the message occurs.
			\param funcName				The name of the function sending the message.
			\param msg					The contents of the message.
			\param level				The severity level of the message. May be any element from the LogLevel enumeration.
			\param id					A unique numeric ID for this message. */
		virtual void Message(const char* file, KyUInt32 line, const char* funcName, const char* msg, LogLevel level, KyUInt32 id) = 0;

		/*!	Logs the specified text exactly as-is. */
		virtual void Print(const char* text) = 0;

		/*!	Sets the output locations in which debug messages will be printed.
			\param newMask		A bitmask typically composed of elements from the Kaim::LogMode enumeration. */
		virtual void SetLogMask(KyUInt32 /*newMask*/) {}
	};

} // namespace Kaim


#endif // KY_ILOGBRIDGE_H
