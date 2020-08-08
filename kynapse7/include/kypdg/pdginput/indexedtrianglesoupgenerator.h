/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPdg_IndexedTriangleSoupGenerator_H
#define KyPdg_IndexedTriangleSoupGenerator_H



#include "kypdg/generator/pdgsystem.h"

#include "kypathdata/containers/kystring.h"

namespace Kaim
{
class IndexedTriangleSoup;

/*

   O______________________________
   |        /|        /|        /|
   |      /  |      /  |      /  |
   |    /    |    /    |    /    |
   |  /      |  /      |  /      |
   |/________|/________|/________|
   |        /|        /|        /|
   |      /  |      /  |      /  |
   |    /    |    /    |    /    |
   |  /      |  /      |  /      |
   |/________|/________|/________|
Y  |        /|        /|        /|
^  |      /  |      /  |      /  |
|  |    /    |    /    |    /    |
|  |  /      |  /      |  /      |
|  |/________|/________|/________|
|
-------> X
*/
class IndexedTriangleSoupGenerator
{
public:
	IndexedTriangleSoupGenerator(KyUInt32 nbRows, KyUInt32 nbCols, KyUInt32 nbFloors, Vec3f origin, KyFloat32 rowHeight, KyFloat32 rowWidth, KyFloat32 floorLength, KyFloat32 coefX, KyFloat32 coefY, bool noise = false, bool holes = false) :
	m_nbRows(nbRows), m_nbCols(nbCols), m_nbFloors(nbFloors), m_rowLength(rowHeight), m_colLength(rowWidth), m_floorLength(floorLength), m_origin(origin), m_coefX(coefX), m_coefY(coefY), m_noise(noise), m_holes(holes) {}

	IndexedTriangleSoupGenerator(KyUInt32 nbRows, KyUInt32 nbCols, KyUInt32 nbFloors, Vec3f origin, KyFloat32 rowHeight, KyFloat32 rowWidth, KyFloat32 floorLength, bool noise = false, bool holes = false) :
	m_nbRows(nbRows), m_nbCols(nbCols), m_nbFloors(nbFloors), m_rowLength(rowHeight), m_colLength(rowWidth), m_floorLength(floorLength), m_origin(origin), m_coefX(0.f), m_coefY(0.f), m_noise(noise), m_holes(holes) {}

	KyResult Generate(IndexedTriangleSoup& indexedTriangle);

	static void Create2JoinedQuads(IndexedTriangleSoup& testMap);
	static void Create2SeparatedQuads(IndexedTriangleSoup& testMap);
	static void Create2JoinedQuadsAnd1Floor(IndexedTriangleSoup& testMap);
	static void CreateFloorsQuads(IndexedTriangleSoup& testMap, KyUInt32 nbFloors, KyFloat32 floorsHeight);

private:
	KyUInt32 GetVertexIdx(KyInt32 gridx, KyInt32  gridy, KyInt32  gridz);
	void GetTriangle1(KyInt32 casex, KyInt32 casey, KyInt32 casez, KyUInt32* vertexIndices);
	void GetTriangle2(KyInt32 casex, KyInt32 casey, KyInt32 casez, KyUInt32* vertexIndices);
	bool RemoveTriangle(KyUInt32 x, KyUInt32 y, KyUInt32 xmax, KyUInt32 ymax);

private:
	KyUInt32 m_nbRows;
	KyUInt32 m_nbCols;
	KyUInt32 m_nbFloors;
	KyFloat32 m_rowLength;
	KyFloat32 m_colLength;
	KyFloat32 m_floorLength;
	Vec3f m_origin;
	// coeffecient for the altitude
	// z = gridz * m_floorLength + m_coefX * gridx * m_colLength + m_coefY * gridy * m_rowLength
	KyFloat32 m_coefX;
	KyFloat32 m_coefY;
	bool m_noise;
	bool m_holes;
};

}


#endif //KyPdg_IndexedTriangleSoupGenerator_H


