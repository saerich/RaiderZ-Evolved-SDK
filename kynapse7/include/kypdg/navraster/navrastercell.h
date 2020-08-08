/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_NavRasterCell_H
#define KyPdg_NavRasterCell_H


#include "kypathdata/blob/endianness.h"
#include <kypathdata/basesystem/coordsystem.h>
#include "kypdg/generator/squarepartition.h"
#include "kypathdata/blob/blobarray.h"
#include "kypathdata/math/triangle3i.h"
#include "kypdg/generator/celldesc.h"
#include "kypathdata/containers/stringstream.h"


namespace Kaim
{

class NavRasterColumnElement
{
public:
	typedef KyUInt32 ElementColor;
	typedef KyUInt32 TerrainType;
	static const KyUInt32 s_navRaster_noConnection; /*= KyUInt32MAXVAL*/
	static const ElementColor s_navRaster_noColor; /*= KyUInt32MAXVAL*/
	static const ElementColor s_navRaster_irrelevantColor; /*= KyUInt32MAXVAL - 7*/ //to be compliant with boundary colors

public:
	
	inline KyUInt32 GetNeighborIndex(KyUInt32 connectionIdx) const { return m_connections[connectionIdx].neighborIndex;}
	inline KyUInt32 GetEastNeighborIndex() const { return m_connections[0].neighborIndex;}
	inline KyUInt32 GetNorthNeighborIndex() const { return m_connections[1].neighborIndex;}
	inline KyUInt32 GetWestNeighborIndex() const { return m_connections[2].neighborIndex;}
	inline KyUInt32 GetSouthNeighborIndex() const { return m_connections[3].neighborIndex;}

	class Connection 
	{
	public: 
		Connection() { neighborIndex = s_navRaster_noConnection; }
		KyUInt32 neighborIndex; 
	};
	

public:

	NavRasterColumnElement& operator=(const NavRasterColumnElement& other) 
	{
		this->m_altitude = other.m_altitude;
		this->m_color = other.m_color;
		this->m_terrainType = other.m_terrainType;
		for (KyUInt32 i = 0 ; i < 4; ++i)
		{
			this->m_connections[i].neighborIndex = other.m_connections[i].neighborIndex;
		}
		return *this;
	}

	NavRasterColumnElement()
	{
		for (KyInt8 i = 0 ; i < 4 ; ++i)
		{
			m_connections[i].neighborIndex = s_navRaster_noConnection;
		}
		m_altitude = 0;
		m_color = s_navRaster_noColor;
		m_terrainType = 0;
	}

	inline KyFloat32 GetAltitude() const { return m_altitude;}

	KyFloat32 m_altitude;
	ElementColor m_color;
	TerrainType m_terrainType;

	// connections are indexed as follow:
	//  +-----------+
	//  |     1     |
	//  |           |
	//  |2    +    0|
	//  |           |
	//  |     3     |
	//  +-----------+

	//0 corresponds to East (+1;0) and index increases CCW
	//Stores level index of neighbor position if reachable, 0xFFFFFFFF otherwise
	Connection m_connections[4];
};

inline void SwapEndianness(Endianness::Target e, NavRasterColumnElement& self)
{
	SwapEndianness(e, self.m_altitude);
	SwapEndianness(e, self.m_color);
	SwapEndianness(e, self.m_terrainType);
	SwapEndianness(e, self.m_connections[0].neighborIndex);
	SwapEndianness(e, self.m_connections[1].neighborIndex);
	SwapEndianness(e, self.m_connections[2].neighborIndex);
	SwapEndianness(e, self.m_connections[3].neighborIndex);
}

//In the corners of the exclusive pixel box, a single color might not
//be enough to describe the pixel and ensure consistency across borders
//For these pixels, we can require bevel edges
//We therefore use a particular element with an extra "half" color
//we arbitrarily decide that m_color correspond the left part of outgoing
//bevel half edge whereas m_secondColor correspond to the right part
//
//  ___NW__   _NE__
//  | \ 2 |  | 1 / |
//	|  \  |  |  /  |   1: m_color
//	|_1_\_|  |_/_2_|   2: m_secondColor
//  __SW__    __SE_
//	| 2 / |  | \ 1 |
//	|  /  |  |  \  |
//	|_/_1_|  |_2_\_|

class NavRasterCornerColumnElement : public NavRasterColumnElement
{
public: 
	NavRasterCornerColumnElement() : NavRasterColumnElement()
	{
		m_secondColor = s_navRaster_noColor;
	}

	bool AreColorsConsistent() { return m_secondColor == m_color ;}
	ElementColor GetNorthColor() { return m_color ;}
	ElementColor GetSouthColor() { return m_secondColor ;}

public: 
	ElementColor m_secondColor;
};

inline void SwapEndianness(Endianness::Target e, NavRasterCornerColumnElement& self)
{
	SwapEndianness(e, self.m_altitude);
	SwapEndianness(e, self.m_color);
	SwapEndianness(e, self.m_terrainType);
	SwapEndianness(e, self.m_secondColor);
	SwapEndianness(e, self.m_connections[0].neighborIndex);
	SwapEndianness(e, self.m_connections[1].neighborIndex);
	SwapEndianness(e, self.m_connections[2].neighborIndex);
	SwapEndianness(e, self.m_connections[3].neighborIndex);
}


class NavRasterColumn
{
public:
	NavRasterColumn() {}

public:
	BlobArray<NavRasterColumnElement> m_navRasterPoints;

private:
	NavRasterColumn& operator=(const NavRasterColumn& navRasterColumn);
	NavRasterColumn(const NavRasterColumn& navRasterColumn);
};
inline void SwapEndianness(Endianness::Target e, NavRasterColumn& self)
{
	SwapEndianness(e, self.m_navRasterPoints);
}

class NavRasterCornerColumn
{
public:
	NavRasterCornerColumn() {}

public:
	BlobArray<NavRasterCornerColumnElement> m_navRasterCornerPoints;

private:
	NavRasterCornerColumn& operator=(const NavRasterCornerColumn& navRasterColumn);
	NavRasterCornerColumn(const NavRasterCornerColumn& navRasterColumn);
};
inline void SwapEndianness(Endianness::Target e, NavRasterCornerColumn& self)
{
	SwapEndianness(e, self.m_navRasterCornerPoints);
}


class NavRasterCell
{
public:
	static const char* GetBlobType() { return "NavRasterCell"; }
	//version history: 
	// 0: initial version
	// 1: added corners elements (skipped header update...) 
	// 2: added TerrainType
	static KyUInt32 GetBlobVersion() { return 2; }

public:
	enum CornerColumnIndexes
	{
		NORTH_WEST_CORNER = 0,
		NORTH_EAST_CORNER = 1,
		SOUTH_EAST_CORNER = 2,
		SOUTH_WEST_CORNER = 3
	};

	CoordSystem m_coordSystem;
	SquarePartition m_partition;
	CellDesc m_cellDesc;
	BlobArray<NavRasterColumn> m_columns; // row major
	NavRasterCornerColumn m_cornerColumns[4];
};
inline void SwapEndianness(Endianness::Target e, NavRasterCell& self)
{
	SwapEndianness(e, self.m_coordSystem);
	SwapEndianness(e, self.m_partition);
	SwapEndianness(e, self.m_cellDesc);
	SwapEndianness(e, self.m_columns);
	SwapEndianness(e, self.m_cornerColumns[NavRasterCell::NORTH_WEST_CORNER]);
	SwapEndianness(e, self.m_cornerColumns[NavRasterCell::NORTH_EAST_CORNER]);
	SwapEndianness(e, self.m_cornerColumns[NavRasterCell::SOUTH_EAST_CORNER]);
	SwapEndianness(e, self.m_cornerColumns[NavRasterCell::SOUTH_WEST_CORNER]);
}

}


#endif

