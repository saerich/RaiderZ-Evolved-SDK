/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PointGraphEdge_H
#define KyPathData_PointGraphEdge_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/blob/endianness.h"
#include "kypdg/pointgraph/pointgraphtypes.h"

namespace Kaim
{

/*
Serializable structure (Blob) containing all basic data of a graph edge, namely:
-The index of the start vertex in the PointGraphCell.
-The index of the end vertex in the PointGraphCell.
*/
class PointGraphEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	PointGraphEdge() {}

	GraphVertexIdx GetStartVertexIdx() const { return m_startVertexIdx; }
	GraphVertexIdx GetEndVertexIdx() const { return m_endVertexIdx; }

public :
	GraphVertexIdx m_startVertexIdx;   //< Index of the start vertex.
	GraphVertexIdx m_endVertexIdx;     //< Index of the end vertex.
};

/*
Implements the endianness swap for all members of PointGraphEdge.
*/
inline void SwapEndianness(Endianness::Target e, PointGraphEdge& self)
{
	SwapEndianness(e, self.m_startVertexIdx);
	SwapEndianness(e, self.m_endVertexIdx);
}
}

#endif // #ifndef KyPathData_PointGraphEdge_H
