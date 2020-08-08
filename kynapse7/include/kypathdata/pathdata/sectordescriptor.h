/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_SectorDescriptor_H
#define KyPdg_SectorDescriptor_H

#include "kypathdata/generic/kyguid.h"
#include "kypathdata/blob/blobarray.h"


namespace Kaim
{

class SectorDescriptor
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	static const char* GetBlobType() { return "SectorDescriptor"; }
	// history: 
	// 0 - Initial version
	static KyUInt32 GetBlobVersion() { return 1; }

public:
	SectorDescriptor() {}

public:
	BlobArray<char> m_sectorFilename;
	KyGuid m_sectorGuid;

private:
	SectorDescriptor& operator=(const SectorDescriptor& desc);
	SectorDescriptor(const SectorDescriptor& desc);
};

inline void SwapEndianness(Endianness::Target e, SectorDescriptor& self)
{
	SwapEndianness(e, self.m_sectorFilename);
	SwapEndianness(e, self.m_sectorGuid);
}

} // namespace Kaim

#endif
