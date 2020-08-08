/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_DynamicNavRasterCell_H
#define KyPdg_DynamicNavRasterCell_H


#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/common/growinglistpool.h"
#include "kypdg/common/growingsmallbuffer.h"
#include "kypdg/raster/dynamicrastercell.h"
#include "kypdg/navraster/navrastercell.h"
#include "kypdg/generator/celldesc.h"
#include "kypdg/navraster/navrastercellpainter.h"
#include "kypdg/common/performancetickcount.h"
#include <kypdg/common/stlvector.h>

namespace Kaim
{

//forward decl
class PdgSystem;
class BlobChunkBuilder;

class NavRasterWorkingColumnElement
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	inline KyUInt32 GetEastNeighborIndex() const { return m_connections[0].neighborIndex;}
	inline KyUInt32 GetNorthNeighborIndex() const { return m_connections[1].neighborIndex;}
	inline KyUInt32 GetWestNeighborIndex() const { return m_connections[2].neighborIndex;}
	inline KyUInt32 GetSouthNeighborIndex() const { return m_connections[3].neighborIndex;}

	enum ConnectionTag 
	{
		NO_TAG,
		HOLE, 
		WALL,
		STEP,
		BORDER,
		Dummy_FORCE32 = 0xFFFFFFFF
	};

	class Connection 
	{
	public: 
		Connection() {tag = NO_TAG; neighborPtr = KY_NULL; }
		KyUInt32 neighborIndex; 
		KyUInt32 tag; 
		NavRasterWorkingColumnElement* neighborPtr;
	};

public:

	NavRasterWorkingColumnElement& operator=(const NavRasterWorkingColumnElement& other) 
	{
		this->m_altitude = other.m_altitude;
		this->m_distanceFromHole = other.m_distanceFromHole;
		this->m_distanceFromWall = other.m_distanceFromWall;
		this->m_floor = other.m_floor;
		this->m_terrainType = other.m_terrainType;
		for (KyUInt32 i = 0 ; i < 4; ++i)
		{
			this->m_connections[i].neighborPtr = other.m_connections[i].neighborPtr;
			this->m_connections[i].tag = other.m_connections[i].tag;
		}
		return *this;
	}

	NavRasterWorkingColumnElement()
	{
		Clear();
	}
	
	void Clear()
	{
		for (KyInt8 i = 0 ; i < 4 ; ++i)
		{
			m_connections[i].neighborPtr = KY_NULL;
			m_connections[i].tag = NO_TAG;
		}
		m_altitude = -KyFloat32MAXVAL;
		m_distanceFromWall = KyFloat32MAXVAL;
		m_distanceFromHole = KyFloat32MAXVAL;
		m_floor = -1;
		m_terrainType = 0;
	}

	KyFloat32 m_altitude;
	KyFloat32 m_distanceFromWall;
	KyFloat32 m_distanceFromHole;
	KyInt32 m_floor;
	KyUInt32 m_terrainType;

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

class DynamicNavRasterCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	struct NavRasterFinalColumn 
	{ 
		NavRasterFinalColumn()
		{
			count = 0; elements = KY_NULL;
		}
		KyUInt32 count; 
		NavRasterColumnElement* elements; 
	};
	struct NavRasterFinalCornerColumn 
	{ 
		NavRasterFinalCornerColumn()
		{
			count = 0; elements = KY_NULL;
		}
		KyUInt32 count; 
		NavRasterCornerColumnElement* elements; 
	};
	struct NavRasterWorkingColumn 
	{ 
		NavRasterWorkingColumn()
		{
			count = 0; elements = KY_NULL;
		}
		KyUInt32 count;
		NavRasterWorkingColumnElement* elements; 
	};

public:
	enum CornerColumnIndexes
	{
		NORTH_WEST_CORNER = 0,
		NORTH_EAST_CORNER = 1,
		SOUTH_EAST_CORNER = 2,
		SOUTH_WEST_CORNER = 3
	};

	DynamicNavRasterCell(PdgSystem* sys, const DynamicRasterCell* inputRasterCell, const CellDesc& cellDesc);
	~DynamicNavRasterCell();

	void CreateNavRaster();

	NavRasterFinalColumn GetFinalColumnFromLocal(const PixelPos& localPixelPos) const;
	NavRasterFinalColumn GetFinalColumn(const PixelPos& pixelPos) const;

	/* Write NavRaster file file if specified in the Generator RunOptions, for debug purpose. */
	KyResult WriteIntermediateFile();

	bool IsEmpty() const { return m_isEmpty; }

	inline KyInt32 GetWorkingNeighborColumnIndex(const PixelPos& pixelPos, KyInt32 dir);
	inline KyInt32 GetFinalNeighborColumnIndex(const PixelPos& pixelPos, KyInt32 dir);

	inline NavRasterColumnElement* GetFinalNeighbor(const PixelPos& pos, const NavRasterColumnElement& pixel, KyInt32 dir);
	//inline NavRasterWorkingColumnElement* GetWorkingNeighbor(const PixelPos& pos, const NavRasterWorkingColumnElement& pixel, KyInt32 dir);
	
	inline NavRasterWorkingColumn* GetWorkingNeighborColumn(const PixelPos& pos, KyInt32 dir);
	inline NavRasterFinalColumn* GetFinalNeighborColumn(const PixelPos& pos, KyInt32 dir);

	inline NavRasterWorkingColumnElement* GetWorkingNeighbor_fast(const PixelPos& pos, const NavRasterWorkingColumnElement& pixel, KyInt32 dir);
	inline const NavRasterWorkingColumnElement* GetWorkingNeighbor_fast(const PixelPos& pos, const NavRasterWorkingColumnElement& pixel, KyInt32 dir) const;


private:
	void BuildRaster();
	void TagConnections();
	void ErodePixels();

	void MakeFinalColumns();

	void EnforceSymetry();
	void ComputeFourNeighborDistanceMap();
	void ComputeEightNeighborDistanceMap();

	void ReleaseWorkingColumns()
	{
		//swap trick (see: http://stackoverflow.com/questions/253157/how-to-downsize-stdvector)
		StlVector<NavRasterWorkingColumn>().swap(m_workingColumns);
		m_workingColumnsPool.Release();
	}

public:
	PdgSystem* m_sys;
	CellDesc m_cellDesc;
	KyInt32 m_intEntityRadius;
	KyUInt32 m_highestFloor;

public:

	const DynamicRasterCell* m_inputRasterCell;

	PixelBox m_enlargedPixelBox;
	PixelBox m_navPixelBox;

	//coloring
	NavRasterCellPainter m_painter;

	// final columns
	StlVector<NavRasterFinalColumn> m_finalColumns;
	NavRasterFinalCornerColumn m_finalCornerColumns[4];
	GrowingSmallBufferPool m_finalColumnsPool;
	GrowingSmallBufferPool m_finalCornerColumnsPool;

	StlVector<NavRasterWorkingColumn> m_workingColumns;
	GrowingSmallBufferPool m_workingColumnsPool;

	bool m_isEmpty;

	PerformanceTickCount m_coloringTickCountStart;
	PerformanceTickCount m_coloringTickCountEnd;

	friend class DynamicNavRasterCellBlobBuilder;
};

inline DynamicNavRasterCell::NavRasterFinalColumn DynamicNavRasterCell::GetFinalColumnFromLocal(const PixelPos& localPixelPos) const
{
	KyInt32 idx = m_navPixelBox.GetRowMajorIndexFromLocalPos(localPixelPos);
	return m_finalColumns[idx];
}

inline DynamicNavRasterCell::NavRasterFinalColumn DynamicNavRasterCell::GetFinalColumn(const PixelPos& pixelPos) const
{
	KyInt32 idx = m_navPixelBox.GetRowMajorIndex(pixelPos);
	return m_finalColumns[idx];
}

inline KyInt32 DynamicNavRasterCell::GetWorkingNeighborColumnIndex(const PixelPos& pixelPos, KyInt32 dir)
{
	PixelPos neighborPos;
	switch(dir)
	{
	case 0: neighborPos = pixelPos.NeighborEast();
		break;
	case 1: neighborPos = pixelPos.NeighborNorth();
		break;
	case 2: neighborPos = pixelPos.NeighborWest();
		break;
	case 3: neighborPos = pixelPos.NeighborSouth();
		break;
	default: return - 1;
	}

	if (m_enlargedPixelBox.IsInside(neighborPos))
	{
		return m_enlargedPixelBox.GetRowMajorIndex(neighborPos);
	}
	return -1;
}

inline KyInt32 DynamicNavRasterCell::GetFinalNeighborColumnIndex(const PixelPos& pixelPos, KyInt32 dir)
{
	PixelPos neighborPos;
	switch(dir)
	{
	case 0: neighborPos = pixelPos.NeighborEast();
		break;
	case 1: neighborPos = pixelPos.NeighborNorth();
		break;
	case 2: neighborPos = pixelPos.NeighborWest();
		break;
	case 3: neighborPos = pixelPos.NeighborSouth();
		break;
	default: return - 1;
	}

	if (m_navPixelBox.IsInside(neighborPos))
	{
		return m_navPixelBox.GetRowMajorIndex(neighborPos);
	}
	return -1;
}

inline NavRasterColumnElement* DynamicNavRasterCell::GetFinalNeighbor(const PixelPos& pos, const NavRasterColumnElement& pixel, KyInt32 dir)
{
	PixelPos neighborPos;
	KyUInt32 neighborIndexInColumn;
	switch(dir)
	{
	case 0: 
		neighborPos = pos.NeighborEast();
		neighborIndexInColumn = pixel.GetEastNeighborIndex();
		break;
	case 1: 
		neighborPos = pos.NeighborNorth();
		neighborIndexInColumn = pixel.GetNorthNeighborIndex();
		break;
	case 2: 
		neighborPos = pos.NeighborWest();
		neighborIndexInColumn = pixel.GetWestNeighborIndex();
		break;
	case 3:
		neighborPos = pos.NeighborSouth();
		neighborIndexInColumn = pixel.GetSouthNeighborIndex();
		break;
	default: return KY_NULL;
	}

	if (m_navPixelBox.IsInside(neighborPos))
	{
		KyInt32 neighborColumnIndex = GetFinalNeighborColumnIndex(pos,dir);
		if (neighborColumnIndex < 0)
			return KY_NULL;
		NavRasterFinalColumn& neighborColumn = m_finalColumns[neighborColumnIndex];
		if (neighborColumn.count > neighborIndexInColumn)
		{
			return &m_finalColumns[neighborColumnIndex].elements[neighborIndexInColumn];
		}
	}
	return KY_NULL;
}

inline NavRasterWorkingColumnElement* DynamicNavRasterCell::GetWorkingNeighbor_fast(const PixelPos& /*pos*/, const NavRasterWorkingColumnElement& pixel, KyInt32 dir)
{
	return pixel.m_connections[dir].neighborPtr;
}

inline const NavRasterWorkingColumnElement* DynamicNavRasterCell::GetWorkingNeighbor_fast(const PixelPos& /*pos*/, const NavRasterWorkingColumnElement& pixel, KyInt32 dir) const
{
	return pixel.m_connections[dir].neighborPtr;
}

inline DynamicNavRasterCell::NavRasterWorkingColumn* DynamicNavRasterCell::GetWorkingNeighborColumn(const PixelPos& pos, KyInt32 dir)
{
	PixelPos neighborPos;
	switch(dir)
	{
	case 0: 
		neighborPos = pos.NeighborEast();
		break;
	case 1: 
		neighborPos = pos.NeighborNorth();
		break;
	case 2: 
		neighborPos = pos.NeighborWest();
		break;
	case 3:
		neighborPos = pos.NeighborSouth();
		break;
	default: return KY_NULL;
	}

	if (m_enlargedPixelBox.IsInside(neighborPos))
	{
		KyInt32 neighborColumnIndex = GetWorkingNeighborColumnIndex(pos,dir);
		if (neighborColumnIndex < 0)
			return KY_NULL;
		NavRasterWorkingColumn& neighborColumn = m_workingColumns[neighborColumnIndex];
		return &neighborColumn;
	}
	return KY_NULL;

}

inline DynamicNavRasterCell::NavRasterFinalColumn* DynamicNavRasterCell::GetFinalNeighborColumn(const PixelPos& pos, KyInt32 dir)
{
	PixelPos neighborPos;
	switch(dir)
	{
	case 0: 
		neighborPos = pos.NeighborEast();
		break;
	case 1: 
		neighborPos = pos.NeighborNorth();
		break;
	case 2: 
		neighborPos = pos.NeighborWest();
		break;
	case 3:
		neighborPos = pos.NeighborSouth();
		break;
	default: return KY_NULL;
	}

	if (m_navPixelBox.IsInside(neighborPos))
	{
		KyInt32 neighborColumnIndex = GetFinalNeighborColumnIndex(pos,dir);
		if (neighborColumnIndex < 0)
			return KY_NULL;
		NavRasterFinalColumn& neighborColumn = m_finalColumns[neighborColumnIndex];
		return &neighborColumn;
	}
	return KY_NULL;
}

}

#endif
