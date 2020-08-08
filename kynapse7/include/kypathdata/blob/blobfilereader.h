/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobFileReader_H
#define KyPathData_BlobFileReader_H


#include "kypathdata/blob/endianness.h"
#include "kypathdata/blob/blobfileheader.h"


namespace Kaim
{

/*! The BlobFileReader class reads the binary data in a file or file stream, and uses it to create a new object
	of a specified type within a specified memory buffer.
	\pp It is used internally by the BlobHandler class. You can also invoke it directly if you prefer. Use this
	class as follows: \code
	BlobFileReader reader("MyBlobFile.MyData"); // construct the object
	KyUInt32 blobSize = reader.ReadBlobSize();  // read the size of the object in the file
	if (blobSize == 0) return KY_ERROR;
	char* mem = new char[blobSize];             // allocate a buffer big enough to hold the object
	MyData* myData = reader.Read<MyData>(mem);  // create the object in the buffer
	if (myData == KY_NULL) return KY_ERROR;
	\endcode
	\pp See also the description for the \ref kypathdata_blob module.
	\ingroup kypathdata_blob */
class BlobFileReader
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor */
	BlobFileReader();

	/*! \label_constructor
		\param fileName			The path and file name of the binary file to be read. */
	BlobFileReader(const char* fileName);

	/*! \label_constructor
		\param file				The binary file stream to be read. */
	BlobFileReader(KyFilePtr file);

	/*! \label_destructor */
	~BlobFileReader() { Clear(); }

	/*! \label_reinit
		\param fileName			The path and file name of the binary file to be read. */
	void SetFileName(const char* fileName);

	/*! \label_reinit
		\param file				The binary file stream to be read. */
	void SetFile(KyFilePtr file);

	/*! Opens the file or file stream, and reads only the BlobFileHeader if it has not already been read.
		ReadBlobSize() is useful to retrieve the size of the buffer that must be allocated and passed to Read().
		\tparam T				The class of the blob that will be created.
		\return The size of the buffer needed for the blob contained in the file, or 0 if there was an error when
				opening and reading the file. */
	template<class T>
	KyUInt32 ReadBlobSize();

	/*!
	Loads the file to destinationBuffer.
	It is mandatory to call ReadBlobSize() before Read().
	Return KY_NULL on error else return a T* ptr = destinationBuffer.
	*/
	/*! Loads the file or file stream, and uses its binary data to create a new object within the memory buffer provided.
		\pre You must precede this call with a call to ReadBlobSize().
		\tparam T					The class of the blob that will be created.
		\param destinationBuffer	The memory buffer in which the new object will be created.
		\return A pointer to the newly created object. */
	template<class T>
	T* Read(void* destinationBuffer);

private:
	KyResult Read_NoEndiannessChange(void* destinationBuffer, const char* expectedBlobType, KyUInt32 expectedBlobVersion);
	KyResult ReadHeader(const char* expectedBlobType, KyUInt32 expectedBlobVersion);
	KyResult ReadBlob(void* destinationBuffer);
	void Clear();

private:
	const char* m_fileName;
	KyFilePtr m_file;
	BlobFileHeader m_fileHeader;
	bool m_isFileHeaderRead;
	bool m_isEndiannessSwapNeeded;
	bool m_fileOwnedByMe;
};



// --------------------------------- inline implementation ---------------------------------

template<class T>
T* BlobFileReader::Read(void* destinationBuffer)
{
	if (Result::Fail(Read_NoEndiannessChange(destinationBuffer, T::GetBlobType(), T::GetBlobVersion())))
		return KY_NULL;

	if (m_isEndiannessSwapNeeded)
		SwapEndianness(Endianness::SYSTEM_ENDIANNESS, (*(T*)destinationBuffer));

	return (T*)destinationBuffer;
}

template<class T>
KyUInt32 BlobFileReader::ReadBlobSize()
{
	if (Result::Fail(ReadHeader(T::GetBlobType(), T::GetBlobVersion())))
		return 0;

	return m_fileHeader.m_blobSize;
}

}


#endif

