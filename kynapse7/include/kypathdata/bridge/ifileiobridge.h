/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KY_IFILEIOBRIDGE_H
#define KY_IFILEIOBRIDGE_H

#include "kypathdata/generic/types.h"

namespace Kaim
{
	/*!	Defines an interface used by \SDKName components to manage and access files on disk.
		\pp The Bridge maintains an instance of a class that implements this interface, making the interface available to other
		\SDKName components. By default, the Bridge uses an instance of the DefaultFileIOBridge class. You can also write
		your own implementation of IFileIOBridge, and assign an instance of your class to the Bridge.
		\pp See :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class IFileIOBridge
	{
	public:
		/*!	\label_virtualdestructor */
		virtual ~IFileIOBridge() {}

	public:
		/*!	Opens the specified file.
			\param[out] file			Used to return a stream containing the data read from the specified file.
			\param fileName				The path and file name of the file to open.
			\param mode					Indicates the file access mode to use when opening the file.
			\return 0 if successful, or an error code to indicate failure. */
		virtual KyInt32 Open(void** file, const char* fileName, const char* mode) = 0;

		/*!	Closes the specified file stream.
			\param file					Pointer to the file stream to be closed.
			\return 0 if successful, or an error code to indicate failure. */
		virtual KyInt32 Close(void* file) = 0;

		/*!	Indicates whether or not the end of the specified file stream has been passed.
			\param file					Pointer to the file stream.
			\return 0 if the end of the file stream has not been passed, or a non-zero value to indicate that the
					stream has been read past the end of the file. */
		virtual KyInt32 Eof(void* file) = 0;

		/*!	Copies a file stream into an array of characters. The string is expected to stop at the first newline character,
			the end of file, or the maximum number of characters is read, whichever comes first.
			\param[out] string			Array of characters into which the file stream is read.
			\param n					The maximum number of characters to read from the file stream
			\param file					Pointer to the file stream.
			\return The array of characters read from the file. */
		virtual char* Gets(char* string, KyInt32 n, void* file) = 0;

		/*!	Writes the contents of a memory buffer to the specified file stream.
			\param buffer				Pointer to the data to be written.
			\param size					The byte size of each item to be read from the buffer.
			\param count				The maximum number of items to be written.
			\param[out] file			Pointer to the file stream.
			\return The number of items written to the file stream. */
		virtual KyUInt32 Write(const void* buffer, KyUInt32 size, KyUInt32 count, void* file) = 0;

		/*!	Reads data from a file stream into a memory buffer.
			\param[out] buffer			Pointer to the buffer that will contain the data.
			\param size					The byte size of each item to be read from the file.
			\param count				The maximum number of items to be written to the memory buffer.
			\param file					Pointer to the file stream.
			\return The number of items written to the memory buffer from the file stream. */
		virtual KyUInt32 Read(void* buffer, KyUInt32 size, KyUInt32 count, void* file) = 0;

		/*!	Moves the file pointer associated with the specified file stream to the specified location.
			\param file					Pointer to the file stream.
			\param offset				The number of bytes to offset the file pointer from its origin.
			\param origin				The initial position of the file pointer.
			\return 0 if successful, or a non-zero value to indicate failure. */
		virtual KyInt32 Seek(void* file, KyUInt32 offset, KyInt32 origin) = 0;

		/*!	Retrieves the current position of the file pointer associated with the specified file stream.
			\param file					Pointer to the file stream. */
		virtual KyUInt32 Tell(void* file) = 0;

		/*!	Retrieves the length of the file.
			\param file					Pointer to the file stream. */
		virtual KyUInt32 Length(void* file) = 0;

	};

} // namespace Kaim


#endif // KY_IFILEIOBRIDGE_H
