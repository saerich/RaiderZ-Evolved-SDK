/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_SeedGraphEdgeData_H
#define KyPathData_SeedGraphEdgeData_H


#include "kypathdata/seedgraph/seedgraphdatatypes.h"

namespace Kaim
{

/*
Serializable structure (Blob) containing all basic data of a seedGraph edge, namely:
-A start vertex.
-A end vertex.
-A flag saying if the edge is bidirectionnal.
*/
class SeedGraphEdgeData
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	SeedGraphEdgeData() {}

public:
	SeedGraphVertexDataIdx m_startVertexIdx;   //< Start vertex.
	SeedGraphVertexDataIdx m_endVertexIdx;     //< End vertex.
	KyBool m_bidirectional;                    //< True if edge is bidirectional, else wrong.
};

/*
Implements the endianness swap for all members of SeedGraphEdgeData.
*/
inline void SwapEndianness(Endianness::Target e, SeedGraphEdgeData& self)
{
	SwapEndianness(e, self.m_startVertexIdx);
	SwapEndianness(e, self.m_endVertexIdx);
	SwapEndianness(e, self.m_bidirectional);
}

}

#endif // #ifndef KyPathData_SeedGraphEdgeData_H
