/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobHandler_H
#define KyPathData_BlobHandler_H


#include "kypathdata/blob/blobfilereader.h"
#include "kypathdata/blob/blobfilewriter.h"
#include "kypathdata/blob/baseblobhandler.h"


namespace Kaim
{

/*! The BlobHandler class provides a top-level mechanism for serializing blobs between objects in memory
	and files on disk. It is responsible for maintaining an instance of its blob class, allocating memory
	for that object when needed by calling #KY_MALLOC.
	\tparam T			The class of blob that this handler will manage.
	\pp For usage, see the description for the \ref kypathdata_blob module.
	\pp This class transparently makes use of the BlobFileWriter and BlobFileReader classes to carry out the 
	actual writing and reading of the data. If you want to use a different custom memory allocation scheme for your
	blobs, you can invoke the BlobFileReader and BlobFileWriter classes directly. Mimic the inline implementations of
	the BlobHandler class methods provided in the blobhandler.h file.
	\pp Note that the copy constructor and operator=() of this class transfer ownership over the blob memory to this object.
	\ingroup kypathdata_blob */
template<class T>
class BlobHandler : public BaseBlobHandler
{
public:
	/*! \label_constructor */
	BlobHandler() : BaseBlobHandler() {}

	/*! \label_constructor Copies the state of the specified BlobHandler to the newly constructed object, and
		transfers responsibility for managing the memory allocated by \c other to this object. */
	BlobHandler(BlobHandler& other) : BaseBlobHandler(other) {}

	/*! Copies the state of the specified BlobHandler to this object, and transfers responsibility
		for managing the memory allocated by \c other to this object. */
	BlobHandler<T>& operator=(BlobHandler<T>& other) { return (BlobHandler<T>&)BaseBlobHandler::operator=(other); }

	/*! Allocates a memory buffer of the specified size by calling #KY_MALLOC. */
	void* Allocate(KyUInt32 size);

	/*! Creates and returns a new object of type \c T from the binary data contained in the specified file pointer. 
		Returns #KY_NULL if the load fails. */
	T* Load(KyFilePtr file);

	/*! Creates and returns a new object of type \c T from the binary data contained in the specified file on disk. 
		Returns #KY_NULL if the load fails. */
	T* Load(const char* fileName);

	/*! Saves the binary data in the blob maintained by this handler to the specified file stream. 
		\param file			The file stream in which to write the blob data.
		\param endianness	The endianness with which the binary data should be saved.
		\return A #KyResult that indicates the success or failure of the operation. */
	KyResult Save(KyFilePtr file, Endianness::Type endianness = Endianness::KY_BIG_ENDIAN) const;

	/*! Saves the binary data in the blob maintained by this handler to the specified file on disk. 
		\param fileName		The path and file name of the file in which the data will be written
		\param endianness	The endianness with which the binary data should be saved.
		\return A #KyResult that indicates the success or failure of the operation. */
	KyResult Save(const char* fileName, Endianness::Type endianness = Endianness::KY_BIG_ENDIAN) const;

	/*! Retrieves the blob maintained by this handler. */
	const T* Blob() const { return (const T*) m_blob; }

	/*! Retrieves the blob maintained by this handler. */
	T* Blob() { return (T*) m_blob; }

	/*! Retrieves the size of the blob maintained by this handler. */
	KyUInt32 BlobSize() const { return m_blobSize; }

private:
	T* LoadWithReader(BlobFileReader& reader);
};



// --------------------------------- inline implementation ---------------------------------

template<class T>
T* BlobHandler<T>::Load(KyFilePtr file)
{
	BlobFileReader reader(file);
	return LoadWithReader(reader);
}


template<class T>
T* BlobHandler<T>::Load(const char* fileName)
{
	BlobFileReader reader(fileName);
	return LoadWithReader(reader);
}


template<class T>
T* BlobHandler<T>::LoadWithReader(BlobFileReader& reader)
{
	m_blobSize = reader.ReadBlobSize<T>();
	Allocate(m_blobSize);
	return reader.Read<T>(m_blob);
}


template<class T>
KyResult BlobHandler<T>::Save(KyFilePtr file, Endianness::Type endianness) const
{
	return BlobFileWriter::Write(file, Blob(), BlobSize(), endianness);
}


template<class T>
KyResult BlobHandler<T>::Save(const char* fileName, Endianness::Type endianness) const
{
	return BlobFileWriter::Write(fileName, Blob(), BlobSize(), endianness);
}


template<class T>
void* BlobHandler<T>::Allocate(KyUInt32 size)
{
	Clear();
	if (size > 0)
		m_blob = (T*)KY_MALLOC(char, size);
	m_blobSize = size;
	return m_blob;
}


}


#endif

