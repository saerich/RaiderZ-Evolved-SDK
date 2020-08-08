/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef _KY_LOGGERIMPLEMENTATION_H_
#define _KY_LOGGERIMPLEMENTATION_H_

#include <kypathdata/generic/types.h>
#include <kypathdata/generic/memory.h>
#include <kypathdata/basesystem/logger.h>


namespace Kaim
{

/* This class is used internally by the Logger to filter debug messages.
	You should not interact with this class. Use the Logger instead. */
class LoggerImplementation
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*
		The class LogInfo contains information about messages :
		- a unique id
		- a max number of displaying
		- the current number of displaying
		- a flag to inform is the message has to be ignored or not
	*/
	class LogInfo
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public :
		LogInfo() : m_id(0), m_maxCall(KyInt32MAXVAL), m_currentCall(0), m_isIgnored(KY_FALSE), m_isMaxCallCounted(KY_FALSE){}

		KyUInt32 m_id;
		KyUInt32 m_maxCall;
		KyUInt32 m_currentCall;
		KyBool m_isIgnored;
		KyBool m_isMaxCallCounted;
	};

	LoggerImplementation();

	~LoggerImplementation();
	
	/*	Add an id to the ignore table
		@param id : the id to add.
		\return
			KY_TRUE if the id is correctly added
			KY_FALSE otherwise */
	KyBool AddIgnoredId(KyUInt32 id);

	/*	Remove an id from the ignore list
		@param id the id to remove. */
	KyBool RemoveIgnoredId(KyUInt32 id);

	// return if an id is ignored or not
	KyBool IsIgnored(KyUInt32 id);
	
	// Enable logger
	void Enable() { m_enabled = KY_TRUE; }

	// Disable logger
	void Disable() { m_enabled = KY_FALSE; }

	// return if the logger is activated or not
	KyBool IsEnabled() { return m_enabled; }

	/*	Enable all the message of a level.
		@param messageType the level of message to enable. KY_MESSAGE, KY_WARNING etc... */
	KyBool Enable(LogLevel messageType);

	/*	Disable all the mesage of a level.
		@param messageType the level of message to disable. KY_MESSAGE, KY_WARNING etc... */
	KyBool Disable(LogLevel messageType);

	/*	Get if a message type is ignored or not
		@param messageType the level of message
		@return 
			-# KY_TRUE if the messageType is ignored
			-# KY_FALSE otherwise */
	KyBool IsIgnoredMessageType(LogLevel messageType);

	/*	Set a maxCall number to an id
		\return KY_TRUE if no problem
			KY_FALSE otherwise */
	KyBool SetIdMaxCall(KyUInt32 id, KyUInt32 maxCall, KyBool overwrite = KY_TRUE);

	/*	Unset the max call counter for the id
			\return KY_TRUE if no problem
			KY_FALSE otherwise */
	KyBool UnSetIdMaxCall(KyUInt32 id);

	// Reset the call counter of the id
	KyBool ResetIdCallCounter(KyUInt32 id);

	// Reset all call counter
	KyBool ResetAllCallCounter();

	/* Increase the current call number of an id */
	KyUInt32 IncreaseCall(KyUInt32 id);
	
	/*	Get the number of times the message was displayed.
		Return KY_FALSE if the message is not in the list or if it is not call counted (no max call set). */
	KyBool GetCallNumber(KyUInt32 id, KyUInt32& callCount);

	/*
		Return is the message is loggable or not
		@param messageType : the level of the message
		@param id : the id of the message

		@return :
			-# KY_TRUE if the message is not ignore or if the maxCall number is not reached yet
			-# KY_FALSE if the message has to be ignored
	*/
	KyBool IsLoggable(KyUInt32 id, LogLevel messageType);

	// The hash function. This function return an id for a message text
	KyUInt32 HashFct(const char *key) const;

	/* Return the text corresponding to the level of message.
		"MESSAGE" for KY_MESSAGE
		"WARNING" for KY_WARNING
		... */
	const char* GetLogLevelText(LogLevel logLevel) { return m_logLevelTexts[logLevel]; }
	
	char* GetBuffer() { return m_buffer; }
	const char* GetBuffer() const { return m_buffer; }

	void SetLastMsgId(KyUInt32 id) { m_lastMsgId = id; m_lastMsgIdUpToDate = KY_TRUE; }
	KyUInt32 GetLastMsgId() {return m_lastMsgId;}

	void Reset() { m_lastMsgIdUpToDate = KY_FALSE; }
	KyBool IsLastMsgIdUpToDate(){return m_lastMsgIdUpToDate;}

private:
	/*	Add an id to the table of message to manage.
		\param id : the id to add
		\return : 
			- The index of the message inserted.
			- MAX_LOG_INFO if no enough space left */
	KyUInt32 AddId(KyUInt32 id);

	/*	Get the index of the logInfo with identifier equal to id
		\return the index of the log info if exist
			MAX_LOG_INFO otherwise */
	KyUInt32 GetLogInfoIndex(KyUInt32 id);

	// The table of information on id
	#define MAX_LOG_INFO 256
	LogInfo* m_logInfos[MAX_LOG_INFO];

	// the number of elements in m_logInfos
	KyUInt32 m_logInfosCount;

	// Id of the last printed message
	KyUInt32 m_lastMsgId;
	
	// Is m_lastMsgId up to date
	KyBool m_lastMsgIdUpToDate;
	
	/*	A tab of flag. For each level of message a flag
		to know if the level has to be ignored or not */
	KyBool m_ignoredLogLevels[KY_LastLogLevel];

	/*	A tab of text for each level of message.
		"MESSAGE" for KY_MESSAGE
		"WARNING" for KY_WARNING
		... */
	const char* m_logLevelTexts[KY_LastLogLevel];

	// A flag to use or not debug message.
	// if KY_FALSE, no debug message display
	KyBool m_enabled;

	// a buffer used to format the message
	char m_buffer[1024];
};

} // end namespace Kaim

#endif // _KY_LOGGERIMPLEMENTATION_H_
