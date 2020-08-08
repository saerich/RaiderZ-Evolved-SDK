/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_SquarePartition_H
#define KyPdg_SquarePartition_H


#include "kypathdata/blob/endianness.h"
#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/generator/squarepartitionutils.h"


namespace Kaim
{

/*
                                /\                              [---------------[ cellSize = 16  
                                |                               [-------[ pixelSize = 8          
                                |                                                                
                                |                                                                
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |       #       |       % pixel | pixel # pixel | pixel %       |       #       |       %
#       |       #       |       %  0,3  |  1,3  #  2,3  |  3,3  %       |       #       |       %
#-------+-------#-------+-------%---cell(1,0)---#---cell(1,1)---%---cell(2,1)---#---cell(3,1)---%
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |       #       |       % pixel | pixel # pixel | pixel %       |       #       |       %
#       |       #       |       %  0,2  |  1,2  #  2,2  |  3,2  %       |               |       %
################################%###############################%###############################%
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |       #       |       % pixel | pixel # pixel | pixel %       |       #       |       %
#       |       #       |       %  0,1  |  1,1  #  2,1  |  3,1  %       |       #       |       %
#-------+-------#-------+-------%---cell(0,0)---#---cell(1,0)---%---cell(2,0)---#---cell(3,0)---%
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |       #       |       % pixel | pixel # pixel | pixel %       |       #       |       %
#       |       #       |       %  0,0  |  1,0  #  2,0  |  3,0  %       |       #       |       %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%------> X
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |       #       |       %       |       #       |       %       |       #       |       %
#-------+-------#-------+-------%-------+-------#-------+-------%-------+-------#-------+-------%
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |               |       %       |               |       %       |               |       %
################################%###############################%###############################%       |               |       %       |               |       %       |               |       %
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |       #       |       %       |       #       |       %       |       #       |       %
#-------+-------#-------+-------%-------+-------#-------+-------%-------+-------#-------+-------%
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |       #       |       %       |       #       |       %       |       #       |       %
#       |       #       |       %       |       #       |       %       |       #       |       %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


---------------------------- ZOOMING ON (0,0) ----------------------------
                        Y/\
                         |
        X=-8            X=0             X=8             X=16
         |               |               |               |
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + pixel + + | + + pixel + + | + + pixel + + | + +
 + + + + | + +(-1,1) + + | + + (0,1) + + | + + (1,1) + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
---------+---------------+---------------+---------------+-------Y=8
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + pixel + + | + + pixel + + | + + pixel + + | + +
 + + + + | + +(-1,0) + + | + + (0,0) + + | + + (1,0) + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
---------+---------------+---------------+---------------+-------Y=0---> X
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + pixel + + | + + pixel + + | + + pixel + + | + +
 + + + + | + +(-1,-1)+ + | + + (0,-1)+ + | + + (1,-1)+ + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
---------+---------------+---------------+---------------+-------Y=-8
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
 + + + + | + + + + + + + | + + + + + + + | + + + + + + + | + +
*/
class SquarePartition
{
	typedef SquarePartitionUtils Utils; // alias for readability

public:
	SquarePartition()
	{
		m_pixelSize = 0;
		m_cellSize = 0;
		m_nbPixelsOnCellSide = 0;
		m_pixelEnlargement = 0;
		m_coordEnlargement = 0;
	}

public:
	bool IsValid() const
	{
		return (m_pixelSize > 0) && (m_nbPixelsOnCellSide > 0) && (m_coordEnlargement >= 0);
	}

	// -------------------- Coord PixelCoord CoordPos PixelPos --------------------
	// Coord -> PixelCoord
	PixelCoord LowPixelCoord (Coord coord) const { return Utils::LowSquare (m_pixelSize, coord); }
	PixelCoord HighPixelCoord(Coord coord) const { return Utils::HighSquare(m_pixelSize, coord); }

	// PixelCoord -> Coord
	Coord MinCoordOfPixel(PixelCoord pixelCoord) const { return Utils::MinCoord(m_pixelSize, pixelCoord); }
	Coord MaxCoordOfPixel(PixelCoord pixelCoord) const { return Utils::MaxCoord(m_pixelSize, pixelCoord); }

	// CoordPos -> PixelPos
	void LowPixelPos (const CoordPos& coordPos, PixelPos& pixelPos) const { return Utils::LowSquare (m_pixelSize, coordPos, pixelPos); }
	void HighPixelPos(const CoordPos& coordPos, PixelPos& pixelPos) const { return Utils::HighSquare(m_pixelSize, coordPos, pixelPos); }

	// PixelPos -> CoordPos
	void MinCoordPosOfPixel(const PixelPos& pixelPos, CoordPos& coordPos) const { Utils::MinCoordPos(m_pixelSize, pixelPos, coordPos); }
	void MaxCoordPosOfPixel(const PixelPos& pixelPos, CoordPos& coordPos) const { Utils::MaxCoordPos(m_pixelSize, pixelPos, coordPos); }


	// ------------------------- PixelPos CellPos -------------------------
	// PixelPos -> CellPos
	void PixelPos_To_CellPos(const PixelPos& pixelPos, CellPos& cellPos) const { Utils::BigSquarePos(m_nbPixelsOnCellSide, pixelPos, cellPos); }

	// ---------------- CoordBox <-> { PixelBox, CellBox } ----------------
	// PixelBox <-> CoordBox
	void CoordBox_To_PixelBox(const CoordBox& coordBox, PixelBox& pixelBox) const { Utils::SquareBox(m_pixelSize, coordBox, pixelBox); }
	void PixelBox_To_CoordBox(const PixelBox& pixelBox, CoordBox& coordBox) const { Utils::GetCoordBox(m_pixelSize, pixelBox, coordBox); }

	// CellBox <-> CoordBox
	void CoordBox_To_CellBox(const CoordBox& coordBox, CellBox& cellBox) const { Utils::SquareBox(m_cellSize, coordBox, cellBox); }
	void CellBox_To_CoordBox(const CellBox& cellBox, CoordBox& coordBox) const { Utils::GetCoordBox(m_cellSize, cellBox, coordBox); }

	// --------------------- PixelBox <-> CellBox  ---------------------
	// PixelBox <-> CellBox
	void PixelBox_To_CellBox(const PixelBox& pixelBox, CellBox& cellBox) const { Utils::BigSquareBox(m_nbPixelsOnCellSide, pixelBox, cellBox); }
	void CellBox_To_PixelBox(const CellBox& cellBox, PixelBox& pixelBox) const { Utils::SmallSquareBox(m_nbPixelsOnCellSide, cellBox, pixelBox); }

public:
	Coord m_pixelSize;
	Coord m_cellSize;
	PixelCoord m_nbPixelsOnCellSide;

	Coord m_coordEnlargement;
	Coord m_pixelEnlargement;
};

inline void SwapEndianness(Endianness::Target e, SquarePartition& self)
{
	SwapEndianness(e, self.m_pixelSize);
	SwapEndianness(e, self.m_cellSize);
	SwapEndianness(e, self.m_nbPixelsOnCellSide);
	SwapEndianness(e, self.m_coordEnlargement);
	SwapEndianness(e, self.m_pixelEnlargement);
}


}


#endif

