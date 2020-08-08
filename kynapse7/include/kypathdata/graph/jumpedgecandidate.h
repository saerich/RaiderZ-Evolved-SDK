/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPathData_JumpEdgeCandidate_H
#define KyPathData_JumpEdgeCandidate_H

#include <kypathdata/math/vec3f.h>

namespace Kaim
{


class JumpEdgeCandidate
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*	Default constructor. */
	JumpEdgeCandidate() {}

	static const char* GetBlobType() { return "JumpEdgeCandidate"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	Vec3f m_startPos;
	Vec3f m_endPos;
};


inline void SwapEndianness(Endianness::Target e, JumpEdgeCandidate& self)
{
	SwapEndianness(e, self.m_startPos);
	SwapEndianness(e, self.m_endPos);
}

} // namespace Kaim


#endif // KyPathData_JumpEdgeCandidate_H

