/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BaseBlobHandler_H
#define KyPathData_BaseBlobHandler_H


#include <kypathdata/generic/memory.h>


namespace Kaim
{

/*! BaseBlobHandler provides an abstract base class derived by BlobHandler. It gathers the non-templated methods'
	needed by the BlobHandler. You should not use this class directly; use its methods only through BlobHandler.
	\pp See also the description for the \ref kypathdata_blob module.
	\ingroup kypathdata_blob */
class BaseBlobHandler
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_baseconstructor For internal use. */
	BaseBlobHandler() : m_blob(KY_NULL), m_blobSize(0) {}

	/*! \label_baseconstructor For internal use. */
	BaseBlobHandler(BaseBlobHandler& other)
	{
		operator=(other);
	}

	/*! Copies the state of the specified BaseBlobHandler to this object. For internal use. */
	BaseBlobHandler& operator=(BaseBlobHandler& other)
	{
		m_blob = other.m_blob;
		m_blobSize = other.m_blobSize;
		other.m_blob = KY_NULL;
		other.m_blobSize = 0;
		return *this;
	}

	/*! Abandons ownership over the blob maintained by this object, but does not de-allocate memory. 
		For internal use. */
	void AbandonOwnership()
	{
		m_blobSize = 0;
		m_blob = KY_NULL;
	}

	/*! \label_clear */
	void Clear()
	{
		m_blobSize = 0;
		KY_FREE(m_blob);
		m_blob = KY_NULL;
	}

	/*! \label_destructor For internal use. */
	~BaseBlobHandler()
	{
		KY_FREE(m_blob);
	}

public:
	void* m_blob; /*!< The blob handled by this object. */
	KyUInt32 m_blobSize; /*!< The size of #m_blob in memory. */
};


}


#endif

