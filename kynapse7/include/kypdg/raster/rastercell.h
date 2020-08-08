/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_RasterCell_H
#define KyPdg_RasterCell_H


#include "kypathdata/blob/endianness.h"
#include <kypathdata/basesystem/coordsystem.h>
#include "kypdg/generator/squarepartition.h"
#include "kypdg/generator/celldesc.h"
#include "kypathdata/blob/blobarray.h"
#include "kypathdata/containers/stringstream.h"


namespace Kaim
{


class RasterColumn
{
public:
	RasterColumn() {}
public:
	BlobArray<KyFloat32> m_altitudes;
	BlobArray<KyUInt32> m_terrainTypes;

private:
	RasterColumn& operator=(const RasterColumn& rasterColumn);
	RasterColumn(const RasterColumn& rasterColumn);
};
inline void SwapEndianness(Endianness::Target e, RasterColumn& self)
{
	SwapEndianness(e, self.m_altitudes);
	SwapEndianness(e, self.m_terrainTypes);
}


class RasterCell
{
public:
	static const char* GetBlobType() { return "RasterCell"; }
	//version history: 
	// 0: original version
	// 1: Added colors ! 
	static KyUInt32 GetBlobVersion() { return 1; }

public:
	RasterCell() {}

public:
	CoordSystem m_coordSystem;
	SquarePartition m_partition;
	CellDesc m_cellDesc;
	BlobArray<RasterColumn> m_columns; // row major

private:
	RasterCell& operator=(const RasterCell& rasterCell);
	RasterCell(const RasterCell& rasterCell);
};
inline void SwapEndianness(Endianness::Target e, RasterCell& self)
{
	SwapEndianness(e, self.m_coordSystem);
	SwapEndianness(e, self.m_partition);
	SwapEndianness(e, self.m_cellDesc);
	SwapEndianness(e, self.m_columns);
}


}


#endif

