/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_RawTriangleSoup_H
#define KyPdg_RawTriangleSoup_H


#include "kypathdata/blob/blobarray.h"
#include "kypathdata/math/vec3f.h"


namespace Kaim
{


class RawTriangleSoup
{
public:
	static const char* GetBlobType() { return "RawTriangleSoup"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	RawTriangleSoup() {}

public:
	BlobArray<Vec3f> m_vertices;
	BlobArray<KyUInt32> m_indices;

private:
	RawTriangleSoup& operator=(const RawTriangleSoup& rawTriangleSoup);
	RawTriangleSoup(const RawTriangleSoup& rawTriangleSoup);
};
inline void SwapEndianness(Endianness::Target e, RawTriangleSoup& self)
{
	SwapEndianness(e, self.m_vertices);
	SwapEndianness(e, self.m_indices);
}


}


#endif
