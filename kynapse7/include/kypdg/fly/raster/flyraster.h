/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




// primary contact: BEHE - secondary contact: NONE
#ifndef KyFlyPdg_FlyRaster_H
#define KyFlyPdg_FlyRaster_H

#include "kypathdata/math/interval.h"
#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/raster/dynamicrastercell.h"
#include "kypdg/raster/rasterpoint.h"
#include "kypdg/generator/celldesc.h"
#include "kypdg/fly/generator/flycell.h"
#include "kypdg/pointgraph/dynamicpointgraphtypes.h"

namespace Kaim
{
class PdgSystem;
class FlyRaster;
class FlyRasterBlobBuilder;
class DynamicPointGraphCell;
class ICollisionInterface;
class PdgGraphVertex;
class PdgPointGraph;

struct CountedInterval
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	CountedInterval() : m_interval(-KyFloat32MAXVAL, KyFloat32MAXVAL), m_count(0) {}
	CountedInterval(KyFloat32 low, KyFloat32 high) : m_interval(low, high), m_count(1) {}

	Interval m_interval;
	KyUInt32 m_count;
};
inline void SwapEndianness(Endianness::Target e, CountedInterval& self)
{
	SwapEndianness(e, self.m_interval);
	SwapEndianness(e, self.m_count);
}

struct ValidatedInterval
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	ValidatedInterval() : m_interval(-KyFloat32MAXVAL, KyFloat32MAXVAL), m_valid(KY_TRUE) {}
	ValidatedInterval(KyFloat32 low, KyFloat32 high) : m_interval(low, high), m_valid(KY_TRUE) {}

	Interval m_interval;
	KyBool m_valid;
};
inline void SwapEndianness(Endianness::Target e, ValidatedInterval& self)
{
	SwapEndianness(e, self.m_interval);
	SwapEndianness(e, self.m_valid);
}

enum SplitPlane
{
	SPLIT_X,
	SPLIT_Y,
	SPLIT_Z
};

//-
class FlyRasterNode
{
	KY_DEFINE_NEW_DELETE_OPERATORS
	friend class FlyRaster;
	friend class FlyRasterBlobBuilder;
public:
	FlyRasterNode(FlyRaster* flyRaster) :
		m_flyRaster(flyRaster), m_convexKernel(KY_NULL), m_vertex(KY_NULL),
		m_childLow(KY_NULL), m_childHigh(KY_NULL)
	{}
	~FlyRasterNode();
	bool Process(); // return false if fully inside an obstacle


	const FlyRasterNode* GetChildLow() const { return m_childLow; }
	const FlyRasterNode* GetChildHigh() const { return m_childHigh; }
	const PixelBox& GetHoriBounds() const { return m_horiBounds; }
protected:
	void Process_Fill_Empty();
	void Process_Fill_FullyIncludedVerticalInterval();
	void Process_Fill_Partial();
	void Process_Child();
	void SplitAlongX(PixelCoord xSplit);
	void SplitAlongY(PixelCoord ySplit);
	void SplitAlongZ(KyFloat32 zSplit);
protected:
	// processing sub-parts
	enum FillState
	{
		Fill_Empty = 0,
		Fill_Full,
		Fill_FullyIncludedVerticalInterval,
		Fill_Partial
	};
	FillState CheckNodeFillState() const;
	PixelCoord FindBestXSplitter() const; // split is right before the returned column
	PixelCoord FindBestYSplitter() const;
	KyFloat32 FindBestZSplitter() const;
	bool ComputeConvexKernel();
	bool TryAndConnectTo(const FlyRasterNode* other);

	static bool TryAndConnectTo_TopBottom(const FlyRasterNode& top, const FlyRasterNode& bottom, bool& createDirectLink, bool& createIntermediatePt, Vec3f& intermediatePt);

	bool HasLateralIntermediatePoint(KyInt8 boundaryCoord, PixelCoord thisBound, KyFloat32 exactBound,
										const FlyRasterNode* other, PixelCoord otherBound,
										PixelCoord minLateral, PixelCoord maxLateral,
										KyFloat32 sqrPixelSize, Vec3f& resultPt) const;
	bool HasVerticalIntermediatePoint(KyFloat32 bound,
										PixelCoord minX, PixelCoord maxX,
										PixelCoord minY, PixelCoord maxY,
										Vec3f& resultPt) const;

	FillState CheckColumnFillState(const PixelPos& columnCoord) const;
	KyFloat32 FindNeighborHeightFloor(KyFloat32 floorRefHeight, KyFloat32 ceilRefHeight, PixelCoord x, PixelCoord y) const;
	KyFloat32 FindNeighborHeightCeil(KyFloat32 ceilRefHeight, KyFloat32 floorRefHeight, PixelCoord x, PixelCoord y) const;

	bool IsVertexInBox(PdgGraphVertex& vertex) const;
	void ConnectOwnVertexToSeedVerticeInBox();

protected:
	FlyRaster* m_flyRaster;
	PixelBox m_horiBounds; // inclusive
	KyFloat32 m_bottom;
	KyFloat32 m_top;
	ValidatedInterval* m_convexKernel; // row major
	RasterPoint m_point;
	PdgGraphVertex* m_vertex;

	SplitPlane m_splitPlane;
	FlyRasterNode* m_childLow;
	FlyRasterNode* m_childHigh;
};

class FlyRaster
{
	KY_DEFINE_NEW_DELETE_OPERATORS
	friend class FlyRasterNode;
	friend class FlyRasterBlobBuilder;
public:
	FlyRaster(PdgSystem& sys, const DynamicRasterCell& inputRasterCell, DynamicPointGraphCell& graph, const ICollisionInterface& collisionSet, const FlyCellDesc& cellDesc);
	~FlyRaster();

	//! Step 1 - Create the axis-aligned BSP partition of the cell based on the input raster.
	KyResult CreateFlyRaster();

	//! Step 2 - Create the (dense) graph by connecting the BSP nodes and the input seed graph.
	//  As a post-process, all vertice get overconnected
	KyResult CreateGraph();

	//! Step 3 - Add edges between all eligible vertice to ease the future simplification process.
	//  A vertex pair (A,B) is eligible if Dist(A,B) < maxDistPixelRatio*pixelsize,
	//  and if it does not already have a connection through one other vertex with a cumulated
	//  length of less than maxConnectionDistRatio*dist(A,B)
	KyResult OverConnectGraph(KyInt32 maxDistPixelRatio = 8, KyFloat32 maxConnectionDistRatio = 1.5f);

	//! Optional - dump results (flyraster and graph)
	KyResult WriteFiles();

	// internal helpers - should be somewhere else?
	Vec2f CenterPixelToMeter(PixelPos pixPos) const;
private:
	bool IntegrityTest() const;

private:
	PdgSystem* m_sys;
	FlyCellDesc m_flyCellDesc;

	const DynamicRasterCell* m_inputRasterCell;
	const ICollisionInterface* m_collisionSet;

	FlyRasterNode* m_root;
	DynamicPointGraphCell* m_graph;
	StlVector<PdgGraphVertex*> m_seedPoints;
};

} // namespace Kaim

#endif
