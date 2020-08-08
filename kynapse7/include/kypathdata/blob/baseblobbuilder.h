/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BaseBlobBuilder_H
#define KyPathData_BaseBlobBuilder_H


#include "kypathdata/blob/blobbuffer.h"


namespace Kaim
{

/*! BaseBlobBuilder provides an abstract base class that builds a blob within a contiguous block of memory.
	\tparam T			The name of the blob class that this object will handle.
	\pp This class cannot be used directly. If you are building a blob for a class that is supplied with the \SDKName
	SDK, then use the derived BaseBlobBuilder class that corresponds to the blob class you want to build. If you want
	to use the blob framework to serialize a custom class, you must derive your own custom class from 
	BaseBlobBuilder<YourBlobClassName>, and implement DoBuild().
	\ingroup kypathdata_blob */
template<class T>
class BaseBlobBuilder : public VirtualMemObject
{
public:
	/*! \label_baseconstructor */
	BaseBlobBuilder() : m_blobBuffer(KY_NULL), m_blob(KY_NULL) {}

	/*! Indicates whether the builder is operating in COUNT mode or in WRITE mode. You can use this in your
		implementations of DoBuild(). */
	bool IsWriteMode() { return m_blobBuffer->IsWriteMode(); }

	/*! This method:
		-#	Calls DoBuild() in COUNT mode to determine the amount of memory needed for the blob to be built. 
		-#	Allocates that amount of memory in the specified BlobHandler.
		-#	Calls DoBuild() again in WRITE mode to actually build the blob in the buffer allocated by the BlobHandler. */
	T* Build(BlobHandler<T>& blobHandler);

	virtual ~BaseBlobBuilder() {}

private:
	/*! You must implement this function in any class that derives from BaseBlobBuilder. Its responsibility is to
		set the data for each class member maintained by #m_blob. In your implementation, you should rely on a set of 
		macros supplied for this purpose:
		-	#BLOB_SET, which you call to set the value of a single class member of #m_blob.
		-	#BLOB_ARRAY and #BLOB_ARRAY_COPY, which you call to set up BlobArray members of #m_blob.
		-	#BLOB_STRING, which you call to copy a string into a char* class member of #m_blob.
		-	#BLOB_BUILD, which you call to recursively build class members of #m_blob that are themselves blobs that
			need to be built by their own BaseBlobBuilder class. */
	virtual void DoBuild() = 0;
	
public:
	/*! For internal use. Use #BLOB_BUILD instead. */
	void DoBuildChild(BlobBuffer* blobBuffer, T* blob);

	/*! For internal use. Use #BLOB_SET and #BLOB_ARRAY instead. */
	BlobBuffer* m_blobBuffer;

	/*! The blob maintained by this builder. Modify only using the macros listed under DoBuild(). */
	T* m_blob;
};

/*! Use this macro only in implementations of BaseBlobBuilder::DoBuild().
	\pp Call this macro to set a class member of the blob being built to a specified value.
	\param blob			The class member of the blob whose value will be set.
	\param value		The value to set.
	\pp In COUNT mode, this macro does nothing. In WRITE mode, it sets the value provided.
	\ingroup kypathdata_blob */
#define BLOB_SET(blob, value) \
if (this->IsWriteMode()) (blob) = (value)

/*! Use this macro only in implementations of BaseBlobBuilder::DoBuild().
	\pp Call this macro to reserve a BlobArray member of the blob being built.
	\param blobArray	The BlobArray class member of the blob.
	\param count		The number of elements to reserve in \c blobArray.
	\pp In COUNT mode, only the size of the array is counted. In WRITE mode, the array
	is built and the default constructor of each element is called.
	\ingroup kypathdata_blob */
#define BLOB_ARRAY(blobArray, count) \
this->m_blobBuffer->AllocArray(this->IsWriteMode() ? &blobArray : KY_NULL, count)

/*! Use this macro only in implementations of BaseBlobBuilder::DoBuild().
	\pp Call this macro to reserve a BlobArray member of the blob being built, and to copy
	its values from another specified BlobArray.
	\param blobArray	The BlobArray class member of the blob.
	\param src			The source of the values to write to \c blobArray.
	\param count		The number of elements to reserve in \c blobArray.
	\pp In COUNT mode, only the size of the array is counted. In WRITE mode, the array
	is built and the memory contents of \c src are copied to \c blobArray. No constructors are
	called for the elements in the built array.
	\ingroup kypathdata_blob */
#define BLOB_ARRAY_COPY(blobArray, src, count) \
this->m_blobBuffer->AllocAndCopyArray(this->IsWriteMode() ? &blobArray : KY_NULL, count != 0 ? (src) : KY_NULL, count)

/*! Use this macro only in implementations of BaseBlobBuilder::DoBuild().
	\pp Call this macro to copy a string value to a member of the blob being built.
	\param str			The class member of the blob in which to copy the string.
	\param src			The string to copy to \c str.
	\pp In COUNT mode, only the length of the string is counted. In WRITE mode, the memory contents of \c src 
	are copied to \c str.
	\pp This macro is effectively the same as #BLOB_ARRAY_COPY, but automatically calculates the length
	of the array as strlen(src) + 1.
	\ingroup kypathdata_blob */
#define BLOB_STRING(str, src) \
this->m_blobBuffer->AllocAndCopyArray(this->IsWriteMode() ? &str : KY_NULL, src, (KyUInt32)strlen(src) + 1)

/*! Use this macro only in implementations of BaseBlobBuilder::DoBuild().
	\pp Call this macro to build a class member of the blob being built if that class member is itself
	a blob that needs to be built through its own class of BaseBlobBuilder. This automatically invokes the 
	BaseBlobBuilder::Build() method of \c builder in order to set up the \c blob class member.
	\param blob			The class member that needs to be built.
	\param builder		An object that derives from BaseBlobBuilder that will be invoked to build \c blob.
	\ingroup kypathdata_blob */
#define BLOB_BUILD(blob, builder) \
builder.DoBuildChild(this->m_blobBuffer, this->IsWriteMode() ? &blob : KY_NULL)



// --------------------------------- inline implementation ---------------------------------

template<class T>
T* BaseBlobBuilder<T>::Build(BlobHandler<T>& blobHandler)
{
	BlobBuffer blobBuffer;
	m_blobBuffer = &blobBuffer;

	// COUNT
	m_blobBuffer->Alloc<T>();
	DoBuild();

	// ALLOCATE
	m_blobBuffer->SwitchToWriteMode(blobHandler);

	// WRITE
	m_blob = m_blobBuffer->Alloc<T>();
	DoBuild();

	return (T*)m_blob;
}


template<class T>
void BaseBlobBuilder<T>::DoBuildChild(BlobBuffer* blobBuffer, T* blob)
{
	m_blobBuffer = blobBuffer;
	m_blob = blob;
	DoBuild();
}




}


#endif
