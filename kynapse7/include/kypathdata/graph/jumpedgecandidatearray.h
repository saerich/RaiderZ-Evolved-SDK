/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPathData_JumpEdgeCandidateArray_H
#define KyPathData_JumpEdgeCandidateArray_H

#include <kypathdata/graph/jumpedgecandidate.h>
#include <kypathdata/blob/blobarray.h>
#include <kypathdata/generic/guidcompound.h>


namespace Kaim
{

class JumpEdgeCandidateArray
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	static const char* GetBlobType() { return "JumpEdgeCandidateArray"; }

	// version history: 
	// -----------------
	// 0- Initial version.
	// 1- m_guidCompound added
	static KyUInt32 GetBlobVersion() { return 1; }

public:
	GuidCompound m_guidCompound;
	BlobArray<JumpEdgeCandidate> m_candidates;
};


inline void SwapEndianness(Endianness::Target e, JumpEdgeCandidateArray& self)
{
	SwapEndianness(e, self.m_guidCompound);
	SwapEndianness(e, self.m_candidates);
}

} // namespace Kaim


#endif // KyPathData_JumpEdgeCandidateArray_H

