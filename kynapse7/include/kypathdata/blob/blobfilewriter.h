/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobFileWriter_H
#define KyPathData_BlobFileWriter_H


#include "kypathdata/blob/endianness.h"
#include "kypathdata/blob/blobfileheader.h"
#include "kypathdata/basesystem/io.h"


namespace Kaim
{

/*! The BlobFileWriter class writes the data maintained in a blob to a file.
	\pp It is used internally by the BlobHandler class. You can also invoke it directly if you prefer.
	\pp See also the description for the \ref kypathdata_blob module.
	\ingroup kypathdata_blob */
class BlobFileWriter
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! Writes the binary data in \c blob to a file stream, invoking the IMemoryBridge interface hosted by the BaseSystem.
		\tparam T				The class of the blob to write.
		\param file				The file stream in which the data will be written.
		\param blob				The blob whose data will be written.
		\param blobSize			The size of \c blob in memory.
		\param endianness		The endianness with which the binary data should be saved.
		\return A #KyResult that indicates the success or failure of the operation. */
	template<class T>
	static KyResult Write(KyFilePtr file, const T* blob, KyUInt32 blobSize, Endianness::Type endianness = Endianness::KY_BIG_ENDIAN);

	/*! Writes the binary data in \c blob to a file on disk, invoking the IMemoryBridge interface hosted by the BaseSystem.
		\tparam T				The class of the blob to write.
		\param fileName			The path and file name of the file in which the data will be written.
		\param blob				The blob whose data will be written.
		\param blobSize			The size of \c blob in memory.
		\param endianness		The endianness with which the binary data should be saved.
		\return A #KyResult that indicates the success or failure of the operation. */
	template<class T>
	static KyResult Write(const char* fileName, const T* blob, KyUInt32 blobSize, Endianness::Type endianness = Endianness::KY_BIG_ENDIAN);

private:
	static KyResult WriteBuffers(KyFilePtr file, const void* fileHeaderMemory, const void* blobMemory, KyUInt32 blobSize);
};



// --------------------------------- inline implementation ---------------------------------

template<class T>
KyResult BlobFileWriter::Write(KyFilePtr file, const T* blob, KyUInt32 blobSize, Endianness::Type endianness)
{
	if (blob == KY_NULL || blobSize == 0)
		return Result::BadParameter;

	BlobFileHeader fileHeader;
	const KyResult resInit = fileHeader.Init(blobSize, T::GetBlobType(), T::GetBlobVersion());
	if (Result::Fail(resInit))
		return resInit;

	if (endianness == Endianness::GetSystemEndianness())
		return WriteBuffers(file, (const void*)&fileHeader, (const void*)blob, blobSize); // no endianess swap -> simple write

	// swap fileHeader and blob endianness
	fileHeader.SwapEndianness();
	SwapEndianness(Endianness::INVERSE_SYSTEM_ENDIANNESS, const_cast<T&>(*blob));

	// Write buffers to file
	const KyResult resWrite = WriteBuffers(file, (void*)&fileHeader, blob, blobSize);
	SwapEndianness(Endianness::SYSTEM_ENDIANNESS, const_cast<T&>(*blob)); // unswap blob endianness
	return resWrite;
}


template<class T>
KyResult BlobFileWriter::Write(const char* fileName, const T* blob, KyUInt32 blobSize, Endianness::Type endianness)
{
	KyFilePtr file = Fopen(fileName, "wb");

	if (file == KY_NULL)
		return KY_ERROR;
	const KyResult result = Write(file, blob, blobSize, endianness);

	Fclose(file);
	return result;
}

} // namespace Kaim


#endif
