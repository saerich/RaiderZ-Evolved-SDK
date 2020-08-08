/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobFileHeader_H
#define KyPathData_BlobFileHeader_H


#include <kypathdata/blob/endianness.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{

/*
BlobFileHeader is the header information that is stored in a file just before the Blob buffer.
It allows to describe
- the endianness
- the blob type, version, size

BlobFileHeader memory layout
    m_magicString (12)
    |            m_endianness (4)
    |            |    m_blobType (24)
    |            |    |                        m_blobVersion UINT (4)
    |            |    |                        |    m_blobSize UINT (4)
    |            |    |                        |    |    blob
    |            |    |                        |    |    |
    BlobFile0000 #### MyBlobType00000000000000 #### #### ###########...
*/
class BlobFileHeader
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	static const char* MagicString() { return "BlobFile"; }

	BlobFileHeader();

	/* Sets the whole structure to zeros */
	void Clear();

	/* Copy memoryBuffer to this and fix the endianness if needed.
		Typically called when loading a Blob from a buffer or a file. */
	KyResult InitFromBuffer(void* memoryBuffer, bool& isEndiannessSwap);

	/* Typically called when saving a Blob to a buffer or a file */
	KyResult Init(KyUInt32 blobSize, const char* blobType, KyUInt32 blobVersion);

	/* GetEndianness() works even if the endianness of the structure is swapped */
	Endianness::Type GetEndianness() const;

	/* Swap endianness in place.
		After SwapEndianness() is called, all subsequent calls to BlobFileHeader are invalid !
		Swaps also m_endianness from little to big or big to little */
	void SwapEndianness();

public:
	char m_magicString[12]; // BlobFile0000
	KyUInt32 m_endianness;
	char m_blobType[24];
	KyUInt32 m_blobVersion;
	KyUInt32 m_blobSize;
};


}


#endif
