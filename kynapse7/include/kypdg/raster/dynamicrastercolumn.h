/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_DynamicRasterColumn_H
#define KyPdg_DynamicRasterColumn_H


#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/common/boxoflists.h"
#include "kypdg/pdginput/inputtriangle.h"


namespace Kaim
{


class DynamicRasterPixel
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	DynamicRasterPixel()
	{
		Set(0.0f, -1, s_allWalkable_terrainType);
	}

	void Set(KyFloat32 altitude, KyInt32 pairing, KyInt32 terrainType)
	{
		m_altitude = altitude;
		m_pairing = pairing;
		m_terrainType = terrainType;
	}

public:
	KyFloat32 m_altitude;
	KyInt32 m_pairing;
	//LASI: correspond to the TerrainType associated to the inputTriangle
	//MSB #1 is reserved for slope tagging
	KyUInt32 m_terrainType;
};
typedef BoxOfLists<DynamicRasterPixel, 4> DynamicRasterColumnPool;
typedef DynamicRasterColumnPool::Column DynamicRasterColumn;


}


#endif
