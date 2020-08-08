/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*!	\file
	\ingroup kyruntime_mesh */

// primary contact: GUAL - secondary contact: LAPA
#ifndef KY_POLYGONMERGER_GRID_H
#define KY_POLYGONMERGER_GRID_H

#include <kypathdata/generic/basetypes.h>
#include <kypathdata/generic/memory.h>
#include <kypathdata/math/vec3f.h>


namespace Kaim
{

enum PlgMrg_Grid_RasterType {
	GP_RASTER_FREE = 0,
	GP_RASTER_EDGE = 1,
	GP_RASTER_DOUBLE = 2,
	GP_RASTER_INTERSECTION = 3,
	GP_RASTER_FORCE_32 = 0xFFFFFFFF
};

enum PlgMrg_Grid_ContourType {
	GP_CONTOUR_FREE = 0,
	GP_CONTOUR_ON = 1,
	GP_CONTOUR_IN = 2,
	GP_CONTOUR_OUT = 3,
	GP_CONTOUR_FORCE_32 = 0xFFFFFFFF
};


class PlgMrg_Grid_Pixel
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	// data
	// < byte >< byte >< byte >< byte >
	// ####...........edge............>
	// ||||
	// ||ContourType
	// ||
	// RasterType

	#define KY_GP_EDGE_MASK      0x0FFFFFFF

	#define KY_GP_CONTOUR_MASK   0x30000000
	#define KY_GP_CONTOUR_OFFSET 28

	#define KY_GP_RASTER_MASK    0xC0000000
	#define KY_GP_RASTER_OFFSET  30

public:
	inline KyUInt32 GetEdge()
	{
		return data & KY_GP_EDGE_MASK;
	}
	inline void SetEdge(KyUInt32 edge)
	{
		data &= ~KY_GP_EDGE_MASK; // data = data & 11110000 = ????0000
		data |= edge;             // data = ????0000 | 0000xxxx
	}

	inline PlgMrg_Grid_RasterType GetRasterType()
	{
		return (PlgMrg_Grid_RasterType)(data >> KY_GP_RASTER_OFFSET);
	}
	inline void SetRasterType(PlgMrg_Grid_RasterType rasterType)
	{
		data &= ~KY_GP_RASTER_MASK;                              // data = data & 00111111 = 00??????
		data |= (((KyUInt32)rasterType) << KY_GP_RASTER_OFFSET); // data = 00?????? | xx000000
	}

	PlgMrg_Grid_ContourType GetContourType()
	{
		return (PlgMrg_Grid_ContourType)( (data & KY_GP_CONTOUR_MASK) >> KY_GP_CONTOUR_OFFSET );
	}
	void SetContourType(PlgMrg_Grid_ContourType contourType)
	{
		data &= ~KY_GP_CONTOUR_MASK;                               // data = data & 11001111 = ??00????
		data |= (((KyUInt32)contourType) << KY_GP_CONTOUR_OFFSET); // data = ??00???? | 00xx0000
	}

public:
	KyUInt32 data;
};


class PlgMrg_Grid_Contour
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	PlgMrg_Grid_Contour() {}

	PlgMrg_Grid_Contour(KyUInt32 first, KyUInt32 last) : m_first(first), m_last(last) {}

	KyUInt32 Next(KyUInt32 idx) const { return (idx == m_last) ? m_first : idx + 1; }

	KyUInt32 Prev(KyUInt32 idx) const { return (idx == m_first) ? m_last : idx - 1; }

public:
	KyUInt32 m_first;
	KyUInt32 m_last;
};


/*!	This class merges together a set of input polylines into a set of output contours by projecting the input polylines
	into a regular grid of pixels.
	\pp You should not need to interact with the PlgMrg_Grid_Merger class directly. This class is typically used only indirectly 
	through the LpfPreMerger_Grid class of pre-merger modifier, which you can configure the LpfManager to use for merging 
	obstacle outlines into \LpfPreAggregates. 
	\ingroup kyruntime_mesh */
class PlgMrg_Grid_Merger
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	enum Status
	{
		GMS_Idle = 0,
		GMS_ReadyForInputs,
		GMS_ReadyForMerge,
		GMS_Merging,
		GMS_MergeCompleted,
		GMS_Error_Inputs_CountourMaxReached,
		GMS_Error_Inputs_VertexMaxReached,
		GMS_Error_Merger_Internal,
		GMS_Error_Merger_DegeneratedContour,
		GMS_Error_Outputs_CountourMaxReached,
		GMS_Error_Outputs_VertexMaxReached,
		GMS_DUNMMY__ = KYFORCEENUMSIZEINT
	};

	PlgMrg_Grid_Merger();

	virtual ~PlgMrg_Grid_Merger();

	// deallocate memory
	void Clear();

	// allocate grid memory
	KyBool Init(KyFloat32 size, KyFloat32 defaultPrecision, KyUInt32 nbVertsMax, KyUInt32 nbContoursMax);

	// if precision = 0.0f, the best possible precision will be used
	// the precision used will never be less than minimumPrecision
	void SetupBeforeAddInputs(KyFloat32 precision = 0.0f);

	// add in CCW order
	KyBool AddInputVertex(const Vec3f& pos);

	// to call when all vertices of an input contour has been added
	KyBool FinalizeInputContour();

	// return KY_TRUE when computation succeeded and is over, KY_FALSE otherwise
	KyBool Merge();

	// get the results
	KyBool GetOutputVerts(KyUInt32 p, Vec3f& position);

	void Reset() {m_status = GMS_Idle;}

public:
	void CalculateBBoxAndPrecision();

	PlgMrg_Grid_RasterType CalculateSnappedPoints(
		const Vec3f& A, const Vec3f& B, const Vec3f& C,
		KyInt32& ix_1, KyInt32& iz_1,
		KyInt32& ix_2, KyInt32& iz_2);

	void Raster(
		KyInt32 A_ix, KyInt32 A_iz, PlgMrg_Grid_RasterType A_type,
		KyInt32 B_ix, KyInt32 B_iz, PlgMrg_Grid_RasterType B_type,
		KyUInt32 idx);

	void Plot(KyInt32 x, KyInt32 z, KyUInt32 edge, PlgMrg_Grid_RasterType type);

	KyBool BuildContour(KyInt32 first_x, KyInt32 first_z);

	KyBool AddOutputVertex(KyInt32 x, KyInt32 z, PlgMrg_Grid_RasterType type, KyUInt32 edge, bool offsetVertex);
	void OffsetPreviousOutputVertex();
	KyBool CheckGeomSmoothing(const Vec3f& edgeEnd);

	KyBool FinalizeOutputContour();

	inline PlgMrg_Grid_Pixel* GetPixel(KyInt32 ix, KyInt32 iz) { return &m_pixels[ix * m_nbPixels + iz]; }

	inline KyInt32 GetInt_X(KyFloat32 x) { return (KyInt32)((x - m_origin_X) * m_invPrecision); }
	inline KyInt32 GetInt_Y(KyFloat32 y) { return (KyInt32)((y - m_origin_Y) * m_invPrecision); }

	inline KyFloat32 Get_X(KyInt32 ix) { return m_origin_X + ix * m_precision; }
	inline KyFloat32 Get_Y(KyInt32 iy) { return m_origin_Y + iy * m_precision; }

public:
	// status
	Status m_status;

	// init
	KyFloat32 m_defaultPrecision;
	PlgMrg_Grid_Pixel* m_pixels;
	KyInt32 m_nbPixels;

	// merge setup
	KyFloat32 m_origin_X;
	KyFloat32 m_origin_Y;
	KyInt32 m_end_X;
	KyInt32 m_end_Y;
	KyFloat32 m_precision;
	KyFloat32 m_invPrecision;

	// fill inputs tmp data
	KyUInt32 m_inputVertexIdx;

	// fill outputs tmp data
	KyUInt32 m_outputVertexIdx;

	// outputs tmp smoothing data
	KyUInt32 m_geomSmooth_startIdx;
	KyUInt32 m_geomSmooth_endIdx;

	// vertices
	KyUInt32 m_nbInputVertsMax;
	KyUInt32 m_nbInputVerts;
	Vec3f* m_inputVerts;

	// input contours
	KyUInt32 m_nbInputContoursMax;
	KyUInt32 m_nbInputContours;
	PlgMrg_Grid_Contour* m_inputContours;

	// output vertices
	KyUInt32 m_nbOutputVertsMax;
	KyUInt32 m_nbOutputVerts;
	Vec3f* m_outputVerts;

	// output contours
	KyUInt32 m_nbOutputContoursMax;
	KyUInt32 m_nbOutputContours;
	PlgMrg_Grid_Contour* m_outputContours;

	// perf analysis
	KyFloat32 m_bboxTime;
	KyFloat32 m_rasterTime;
	KyFloat32 m_buildContourTime;
	KyFloat32 m_clearGridTime;
	KyFloat32 m_mergeTime;

	// output contour construction
	bool m_outputVertex_offsetPreviousVertex; // used to postpone the Vertex offset position computation, since we need the second edge to do it
};

}

#endif // KY_POLYGONMERGER_GRID_H
