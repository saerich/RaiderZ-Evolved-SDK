/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: NONE
#ifndef KyPdg_NavRasterCellPainter_H
#define KyPdg_NavRasterCellPainter_H


#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/common/growinglistpool.h"
#include "kypdg/common/growingsmallbuffer.h"
#include "kypdg/navraster/navrastercell.h"
#include "kypdg/generator/celldesc.h"
#include <kypdg/common/stlvector.h>


namespace Kaim
{

class DynamicNavRasterCell;
class NavRasterWorkingColumnElement;

class NavRasterCellPainter
{
public: 
	typedef KyUInt32 Color; //needed by inline utility functions
private:
	struct FloodFillNode{ 
		FloodFillNode(NavRasterColumnElement* _ptr,PixelPos _pos)
		{
			ptr = _ptr;
			pos = _pos;
		}
		NavRasterColumnElement* ptr; 
		PixelPos pos;
	};
	typedef StlVector<StlVector<FloodFillNode> > FloodFillQueue;

	enum ForbiddenMergeDetectionMode
	{
		DO_CHECK_BEVELS, 
		DO_NOT_CHECK_BEVELS
	};
	
public: //keep it simple
	NavRasterCellPainter(): m_useTerrainTypes(true), m_useSubdivision(false), m_subdivisionStep(20) { }

	//returns the number of colors used during painting
	KyUInt32 Paint(DynamicNavRasterCell& navRaster); 
	const Array<KyUInt32>& GetTerrainTypeTable() const { return m_terrainTypeTable; }
	void ToggleTerrainTypeSegmentation(bool status) { m_useTerrainTypes = status; }
	void ToggleSubdivision(bool status) { m_useSubdivision = status; }

	void SetSubdivisionStep(KyUInt32 subdivisionStep) { m_subdivisionStep = subdivisionStep; }

	KyUInt32 GetNbColorsUsed() const { return m_colorUsed; }

private:
	
	bool ConstrainedFloodFill(DynamicNavRasterCell& navRaster, PixelPos& pos, KyUInt32 floor, Color color, PixelBox& bbox);
	
	void ColorBorder(DynamicNavRasterCell& navRaster);
	
	void MergeCornerColors(DynamicNavRasterCell& navRaster);
	void MergeBevelsCW(DynamicNavRasterCell& navRaster);
	void MergeBevelsCCW(DynamicNavRasterCell& navRaster, bool* forbiddenPairs);

	void FloodFillFromBorders(DynamicNavRasterCell& navRaster);

	void DetectForbiddenBorderMerges(DynamicNavRasterCell& navRaster, bool *forbiddenPairsTable, ForbiddenMergeDetectionMode mode = DO_CHECK_BEVELS);
	void BleedOnBorders(DynamicNavRasterCell& navRaster);

private:
	Color m_colorUsed;
	FloodFillQueue m_floodQueue;
	Array<KyUInt32> m_terrainTypeTable;
	bool m_useTerrainTypes;
	bool m_useSubdivision;
	KyUInt32 m_subdivisionStep;
};
}

#endif
