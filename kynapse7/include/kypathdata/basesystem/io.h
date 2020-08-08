/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_basesystem */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <kypathdata/basesystem/basesystem.h>

#ifndef KY_IO_H
#define KY_IO_H

namespace Kaim
{

// File management


/*!	\copydoc PathName::ConvertToNative()
	\ingroup kypathdata_basesystem */
KyBool ConvertPathName(const char* in, char* out);

#ifdef KYDEBUG
/*!	In the Debug build of the \SDKName libraries, this variable maintains a counter of the number 
	of files opened through the Kaim::Fopen() function.
	\ingroup kypathdata_basesystem */
extern KyUInt32 g_fileOpenCounter;

/*!	Logs a warning if the number of calls to Kaim::Fclose() does not match the number of calls
	to Kaim::Fopen() since the \SDKName Engine was opened. This indicates that some file streams
	opened during the \SDKName session may not have been properly closed. If linking against the Debug
	build of the \SDKName libraries, this function is automatically called when the \SDKName
	Engine is closed.
	\return #KY_TRUE if the number of calls to Kaim::Fclose() and Kaim::Fopen() match, or
			#KY_FALSE otherwise.
	\ingroup kypathdata_basesystem */
KyBool CheckNoFileLeftOpened();
#endif //KYDEBUG

/*!	Invokes IFileIOBridge::Open(), and increments #g_fileOpenCounter.
	\ingroup kypathdata_basesystem */
inline KyFilePtr Fopen(const char* fileName, const char* mode)
{
	KyFilePtr file = KY_NULL;
	Kaim::GetBaseSystem().GetBridge().GetFileIOBridge().Open(&file, fileName, mode);
#ifdef KYDEBUG
	if (file != KY_NULL)
	{
		g_fileOpenCounter++;
	}
#endif
	return file;
}

/*!	Invokes IFileIOBridge::Close(), and decrements #g_fileOpenCounter.
	\ingroup kypathdata_basesystem */
inline KyInt32 Fclose(KyFilePtr file)
{
#ifdef KYDEBUG
	if (file != KY_NULL)
	{
		g_fileOpenCounter--;
	}
#endif
	return Kaim::GetBaseSystem().GetBridge().GetFileIOBridge().Close(file);
}

/*!	Invokes IFileIOBridge::Eof().
	\ingroup kypathdata_basesystem */
inline KyInt32 Feof(KyFilePtr file)
{
	return Kaim::GetBaseSystem().GetBridge().GetFileIOBridge().Eof(file);
}

/*!	Invokes IFileIOBridge::Gets().
	\ingroup kypathdata_basesystem */
inline char* Fgets(char* string, KyInt32 n, KyFilePtr file)
{
	return Kaim::GetBaseSystem().GetBridge().GetFileIOBridge().Gets(string,n,file);
}

/*!	Invokes IFileIOBridge::Write().
	\ingroup kypathdata_basesystem */
inline KyUInt32 Fwrite(const void *buffer, KyUInt32  size, KyUInt32  count, KyFilePtr file)
{
	return Kaim::GetBaseSystem().GetBridge().GetFileIOBridge().Write(buffer, size, count, file);
}

/*!	Invokes IFileIOBridge::Write().
	\ingroup kypathdata_basesystem */
KyInt32 Fprintf(KyFilePtr file, const char* format, ...);

/*!	Invokes IFileIOBridge::Read().
	\ingroup kypathdata_basesystem */
inline KyUInt32 Fread(void *buffer, KyUInt32  size, KyUInt32  count, KyFilePtr file)
{
	return Kaim::GetBaseSystem().GetBridge().GetFileIOBridge().Read(buffer, size, count, file);
}

/*!	Invokes IFileIOBridge::Seek().
	\ingroup kypathdata_basesystem */
inline KyInt32 Fseek(KyFilePtr file, KyFixed offset, KyInt32 origin)
{
	return Kaim::GetBaseSystem().GetBridge().GetFileIOBridge().Seek(file, offset, origin);
}

/*!	Invokes IFileIOBridge::Tell().
	\ingroup kypathdata_basesystem */
inline KyFixed Ftell(KyFilePtr file)
{
	return Kaim::GetBaseSystem().GetBridge().GetFileIOBridge().Tell(file);
}

/*!	Retrieves the total length of the specified file stream from its current position to its end.
	\ingroup kypathdata_basesystem */
inline KyFixed Flength(KyFilePtr file)
{
	return Kaim::GetBaseSystem().GetBridge().GetFileIOBridge().Length(file);
}

} // namespace Kaim

#endif //KYNO_KYIO_H
