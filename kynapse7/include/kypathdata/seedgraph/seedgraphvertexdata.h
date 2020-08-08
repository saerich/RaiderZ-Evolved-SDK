/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_SeedGraphVertexData_H
#define KyPathData_SeedGraphVertexData_H


#include "kypathdata/seedgraph/seedgraphdatatypes.h"
#include "kypathdata/math/vec3f.h"

namespace Kaim
{

/*
Serializable structure (Blob) containing all basic data of a seedGraph vertex, namely:
-A position.
*/
class SeedGraphVertexData
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	SeedGraphVertexData() {}

public:
	Vec3f m_position;   //< Position.
};

/*
Implements the endianness swap for all members of SeedGraphVertexData.
*/
inline void SwapEndianness(Endianness::Target e, SeedGraphVertexData& self)
{
	SwapEndianness(e, self.m_position);
}

}

#endif // #ifndef KyPathData_SeedGraphVertexData_H
