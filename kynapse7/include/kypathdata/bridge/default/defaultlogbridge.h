/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef DEFAULT_LOGBRIDGE_H
#define DEFAULT_LOGBRIDGE_H

#include <kypathdata/generic/ilogbridge.h>

namespace Kaim
{


	/*!	This class is the default implementation of the ILogBridge interface created by the Bridge at its
		initialization.
		\pp This implementation provides a standard formatting for log messages that should be suitable
		for most projects. You can use this implementation as is, or write your own implementation of 
		ILogBridge and set your own implementation in your Bridge.
		\pp See also :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class DefaultLogBridge : public Kaim::ILogBridge
	{
	public:
		/*!	\label_baseconstructor */
		DefaultLogBridge();
		
		/*!	\label_virtualdestructor */
		virtual ~DefaultLogBridge();

	public:
		/*!	\copydoc ILogBridge::Message()
			\pp This implementation of Kaim::ILogBridge::Message() reformats the message and other data provided 
			into a readable, user-friendly string, and invokes the Print() method to send the text to the log. 
			If \c KS_LOG_USING_WIN32 is set, this implementation also sends the message to the output window of 
			the Windows debugger.
			\pp Override this method if you want to control the formatting of the messages generated by \SDKName. */
		virtual void Message(const char* file, KyUInt32 line, const char* funcName, const char* msg, LogLevel level, KyUInt32 id);

		/*!	\copydoc ILogBridge::Print()
			\pp This implementation of ILogBridge::Print() prints the message to standard output. If 
			\c KS_LOG_IN_FILE is set, the message is also recorded in the \c kyoutput.txt log file in the 
			current output directory.
			\pp Override this method if you want to change the location in which debug messages are logged. */
		virtual void Print(const char* text);

		/*!	Sets the output locations in which debug messages will be printed.
			\param newMask		A bitmask composed of elements from the Kaim::LogMode enumeration. */
		virtual void SetLogMask(KyUInt32 newMask) { m_modeMask = newMask; }

	protected:
		KyUInt32 m_modeMask;
		KyFilePtr m_file;
	};

} // namespace Kaim


#endif // DEFAULT_LOGBRIDGE_H
