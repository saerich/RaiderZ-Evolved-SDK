/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_DynamicRasterColumnToAltitudes_H
#define KyPdg_DynamicRasterColumnToAltitudes_H


#include "kypdg/raster/dynamicrastercolumn.h"


namespace Kaim
{


class DynamicRasterColumnToAltitudes
{
public:
	DynamicRasterColumnToAltitudes(KyFloat32 meterMergeDistance, KyUInt32 elementsMaxCount = 10000);

	~DynamicRasterColumnToAltitudes();

	void ColumnToAltitudes(const DynamicRasterColumn* column, KyFloat32 terrainTypeMergeTolerance);

	KyUInt32 GetAltitudesCount() { return m_altitudesCount; }

	KyFloat32* GetAltitudes() { return (m_altitudesCount != 0) ? m_altitudes : KY_NULL; }

	KyUInt32 GetTerrainTypesCount() { return m_terrainTypesCount; }

	KyUInt32* GetTerrainTypes() { return (m_terrainTypesCount != 0) ? m_terrainTypes : KY_NULL; }

private:
	void AddAltitude(KyFloat32 altitude)
	{
		assert(m_altitudesCount < m_altitudesMaxCount);
		m_altitudes[m_altitudesCount] = altitude;
		++m_altitudesCount;
	}

	void AddTerrainType(KyUInt32 terrainType)
	{
		assert(m_terrainTypesCount < m_terrainTypesMaxCount);
		m_terrainTypes[m_terrainTypesCount] = terrainType;
		++m_terrainTypesCount;
	}

private:
	KyFloat32 m_meterMergeDistance;

	KyUInt32 m_sortedElementsMaxCount;
	KyUInt32 m_sortedElementsCount;
	DynamicRasterPixel* m_sortedElements;

	KyUInt32 m_altitudesMaxCount;
	KyUInt32 m_altitudesCount;
	KyFloat32* m_altitudes;

	KyUInt32 m_terrainTypesMaxCount;
	KyUInt32 m_terrainTypesCount;
	KyUInt32* m_terrainTypes;
};


}


#endif
