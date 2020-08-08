/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	

/*! \file
	\ingroup kypdg */

// primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_InputTriangle_H
#define KyPdg_InputTriangle_H


#include "kypathdata/blob/endianness.h"
#include "kypathdata/math/triangle3i.h"


namespace Kaim
{

typedef KyUInt32 TerrainTypeMask; /*!< Defines a type for a bitmask that reflects a combination of terrain types. \ingroup kypdg */
static const TerrainTypeMask s_nonWalkable_terrainType = 0x00000000; /*!< Defines a non-walkable terrain type. No PathData will be generated for any input triangles tagged with this terrain type. \ingroup kypdg */
static const TerrainTypeMask s_allWalkable_terrainType = 0xFFFFFFFF; /*!< Defines a terrain type that encompasses all other walkable terrain types. \ingroup kypdg */ 


struct InputTriangle
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	InputTriangle()
	{
		m_terrainTypeMask = s_allWalkable_terrainType;
	}

	InputTriangle(const Triangle3i& triangle, TerrainTypeMask terrainTypeMask)
	{
		m_triangle = triangle;
		m_terrainTypeMask = terrainTypeMask;
	}

	Triangle3i m_triangle;
	TerrainTypeMask m_terrainTypeMask;
};

inline void SwapEndianness(Endianness::Target e, InputTriangle& self)
{
	SwapEndianness(e, self.m_triangle);
	SwapEndianness(e, self.m_terrainTypeMask);
}

}


#endif

