/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_CellDesc_H
#define KyPdg_CellDesc_H


#include "kypdg/generator/squarepartitiontypes.h"
#include "kypathdata/math/box2f.h"


namespace Kaim
{

class SquarePartition;


class CellDesc
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	void Init(const SquarePartition& partition, const CellPos& cellPos);
	bool operator==(const CellDesc& other) const;

public:
	CellPos m_cellPos;
	PixelBox m_exclusivePixelBox;
	PixelBox m_enlargedPixelBox;
	CoordBox m_exclusiveCoordBox;
	CoordBox m_enlargedCoordBox;
	Box2f m_box2f;
};
inline void SwapEndianness(Endianness::Target e, CellDesc& self)
{
	SwapEndianness(e, self.m_cellPos);
	SwapEndianness(e, self.m_exclusivePixelBox);
	SwapEndianness(e, self.m_enlargedPixelBox);
	SwapEndianness(e, self.m_exclusiveCoordBox);
	SwapEndianness(e, self.m_enlargedCoordBox);
	SwapEndianness(e, self.m_box2f);
}


}


#endif
