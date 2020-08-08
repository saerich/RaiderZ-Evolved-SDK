/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPathData_PathdataIndex_H
#define KyPathData_PathdataIndex_H

#include "kypathdata/pathdata/sectordescriptor.h"

namespace Kaim
{

class PathDataIndex
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	static const char* GetBlobType() { return "PathDataIndex"; }
	// history: 
	// 0 - Initial version
	// 1 - Name changed from PathDataIndex to PathDataIndex
	static KyUInt32 GetBlobVersion() { return 1; }

public:
	PathDataIndex() {}

public:
	BlobArray<SectorDescriptor> m_sectorDescriptors;

private:
	PathDataIndex& operator=(const PathDataIndex& inputCell);
	PathDataIndex(const PathDataIndex& inputCell);
};

inline void SwapEndianness(Endianness::Target e, PathDataIndex& self)
{
	SwapEndianness(e, self.m_sectorDescriptors);
}

} // namespace Kaim

#endif
