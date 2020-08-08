/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgInputCell_H
#define KyPdg_PdgInputCell_H


#include <kypathdata/blob/endianness.h>
#include <kypathdata/basesystem/coordsystem.h>
#include <kypathdata/blob/blobarray.h>
#include <kypdg/pdginput/inputtriangle.h>
#include <kypdg/pdginput/pdginputterraintypevolume.h>
#include <kypdg/generator/squarepartitiontypes.h>
#include <kypathdata/generic/kyguid.h>


namespace Kaim
{


class PdgInputCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	static const char* GetBlobType() { return "PdgInputCell"; }
	// history: 
	// 0-3 Unknown
	// 4 - Added KyGuid
	static KyUInt32 GetBlobVersion() { return 4; }

public:
	PdgInputCell() {}

public:
	KyGuid m_sectorGuid;
	CellPos m_cellPos;
	KyInt32 m_minAltitude;
	KyInt32 m_maxAltitude;
	BlobArray<InputTriangle> m_triangles;
	BlobArray<PdgInputTerrainTypeVolume> m_volumes;

private:
	PdgInputCell& operator=(const PdgInputCell& inputCell);
	PdgInputCell(const PdgInputCell& inputCell);
};

inline void SwapEndianness(Endianness::Target e, PdgInputCell& self)
{
	//SwapEndianness(e, self.m_sectorGuid); no swap on char[]
	SwapEndianness(e, self.m_cellPos);
	SwapEndianness(e, self.m_minAltitude);
	SwapEndianness(e, self.m_maxAltitude);
	SwapEndianness(e, self.m_triangles);
	SwapEndianness(e, self.m_volumes);
}

} // namespace Kaim

#endif
