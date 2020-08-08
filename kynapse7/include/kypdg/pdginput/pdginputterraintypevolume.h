/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_PdgInputTerrainTypeVolume_H
#define KyPdg_PdgInputTerrainTypeVolume_H

#include <kypathdata/math/blob/extrudedpolylinedata.h>


namespace Kaim
{

/* use PdgInputTerrainTypeVolumeBlobBuilder to build a PdgInputTerrainTypeVolume */
class PdgInputTerrainTypeVolume
{
public:
	static const char* GetBlobType() { return "InputTerrainTypeVolume"; }
	// version history
	// 0: initial
	// 1: changed BlobType from PdgInputTerrainTypeVolume (> 24 char) to InputTerrainTypeVolume
	static KyUInt32 GetBlobVersion() { return 1; }
	bool operator ==(const PdgInputTerrainTypeVolume& other) const
	{
		return m_terrainTypeMask == other.m_terrainTypeMask && m_volume == other.m_volume;
	}

public:
	ExtrudedPolylineData m_volume;
	KyUInt32 m_terrainTypeMask;
	KyUInt32 m_index;
};

inline void SwapEndianness(Endianness::Target e, PdgInputTerrainTypeVolume& self)
{
	SwapEndianness(e, self.m_volume);
	SwapEndianness(e, self.m_terrainTypeMask);
	SwapEndianness(e, self.m_index);
}

// Create an alias for Volumes in Client Coordinates

/* use ClientInputTerrainTypeVolumeBlobBuilder to build a ClientInputTerrainTypeVolume */
class ClientInputTerrainTypeVolume
{
public:
	static const char* GetBlobType() { return "ClientTerrainTypeVolume"; }
	// version history
	// 0: initial
	static KyUInt32 GetBlobVersion() { return 0; }
	bool operator ==(const ClientInputTerrainTypeVolume& other) const
	{
		return m_terrainTypeMask == other.m_terrainTypeMask && m_volume == other.m_volume;
	}

public:
	ExtrudedPolylineData m_volume;
	KyUInt32 m_terrainTypeMask;
	KyUInt32 m_index;
};

inline void SwapEndianness(Endianness::Target e, ClientInputTerrainTypeVolume& self)
{
	SwapEndianness(e, self.m_volume);
	SwapEndianness(e, self.m_terrainTypeMask);
	SwapEndianness(e, self.m_index);
}

} //namespace Kaim

#endif
