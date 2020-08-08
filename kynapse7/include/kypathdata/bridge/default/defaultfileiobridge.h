/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KSFILEIOBRIDGE_H
#define KSFILEIOBRIDGE_H

#include <kypathdata/bridge/ifileiobridge.h>

namespace Kaim
{
	/*!	This class is the default implementation of the IFileIOBridge interface created by the Bridge at its
		initialization.
		\pp This implementation invokes the standard C++ file access functions, such as \c fopen_s, \c fread, 
		\c fseek, etc. You can use this implementation as is, or write your own implementation of IFileIOBridge 
		and set your own implementation in your Bridge.
		\pp See also :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class DefaultFileIOBridge : public Kaim::IFileIOBridge
	{
	public:
		/*!	\label_baseconstructor */
		DefaultFileIOBridge();
		
		/*!	\label_virtualdestructor */
		virtual ~DefaultFileIOBridge();

	public:
		/*!	\copydoc IFileIOBridge::Open()
			\pp This implementation of IFileIOBridge::Open() invokes \c fopen(). */
		virtual KyInt32 Open(void** file, const char* fileName, const char* mode);
		
		/*!	\copydoc IFileIOBridge::Close()
			\pp This implementation of IFileIOBridge::Close() invokes \c fclose(). */
		virtual KyInt32 Close(void* file);
		
		/*!	\copydoc IFileIOBridge::Eof()
			\pp This implementation of IFileIOBridge::Eof() invokes \c feof(). */
		virtual KyInt32 Eof(void* file);
		
		/*!	\copydoc IFileIOBridge::Gets()
			\pp This implementation of IFileIOBridge::Gets() invokes \c fgets(). */
		virtual char* Gets(char* string, KyInt32 n, void* file);
		
		/*!	\copydoc IFileIOBridge::Write()
			\pp This implementation of IFileIOBridge::Write() invokes \c fwrite(). */
		virtual KyUInt32 Write(const void* buffer, KyUInt32 size, KyUInt32 count, void* file);
		
		/*!	\copydoc IFileIOBridge::Read()
			\pp This implementation of IFileIOBridge::Read() invokes \c fread(). */
		virtual KyUInt32 Read(void* buffer, KyUInt32 size, KyUInt32 count, void* file);
		
		/*!	\copydoc IFileIOBridge::Seek()
			\pp This implementation of IFileIOBridge::Seek() invokes \c fseek(). */
		virtual KyInt32 Seek(void* file, KyUInt32 offset, KyInt32 origin);
		
		/*!	\copydoc IFileIOBridge::Tell()
			\pp This implementation of IFileIOBridge::Tell() invokes \c ftell(). */
		virtual KyUInt32 Tell(void* file);

		/*!	\copydoc IFileIOBridge::Length()
			\pp This implementation of IFileIOBridge::Length() invokes \c ftell(). */
		virtual KyUInt32 Length(void* file);
	};

} // namespace Kaim


#endif // KSFILEIOBRIDGE_H
