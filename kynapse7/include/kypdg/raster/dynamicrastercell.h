/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_DynamicRasterCell_H
#define KyPdg_DynamicRasterCell_H


#include <kypdg/generator/squarepartitiontypes.h>
#include <kypdg/common/boxofarrays.h>
#include <kypdg/raster/dynamicrastercolumn.h>
#include <kypdg/generator/celldesc.h>
#include <kypdg/generator/pdgglobalconfig.h>


namespace Kaim
{

class PdgSystem;
class SingleStagePixel;
class PdgInputCell;
template <typename T> 
class Array;

enum DynamicRasterCellMergeStatus
{
	DynamicRasterCellMergeStatus_MergeEnabled,
	DynamicRasterCellMergeStatus_MergeDisabled
};

class DynamicRasterCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	
	DynamicRasterCell(const PdgGlobalConfig& config, KyInt32 pixelSize, const CellDesc& cellDesc, Array<const PdgInputCell*>* inputCells, const PixelBox& pixelBox)
	{
		Setup(config, pixelSize, cellDesc, inputCells, pixelBox);
	}
	DynamicRasterCell(const PdgGlobalConfig& config, KyInt32 pixelSize, const CellDesc& cellDesc, Array<const PdgInputCell*>* inputCells)
	{
		Setup(config, pixelSize, cellDesc, inputCells, cellDesc.m_enlargedPixelBox);
	}

	void Setup(const PdgGlobalConfig& config, KyInt32 pixelSize, const CellDesc& cellDesc, Array<const PdgInputCell*>* inputCells, const PixelBox& pixelBox);

	/* To be called before the fill of dynamicColumns: GetDynamicColumn().PushPixel(). */
	void BeginColumns();

	/* To be called between BeginColumns() and EndColumns(). */
	void PushPixel(const PixelPos& pixelPos, const SingleStagePixel& pixel, KyUInt32 color);

	/* To be called between BeginColumns() and EndColumns(). */
	void PushPixelToLocal(const PixelPos& localPixelPos, const SingleStagePixel& pixel, KyUInt32 color);

	/* To be called between BeginColumns() and EndColumns(). */
	void PushPixel(KyInt32 rowMajorIdx, const SingleStagePixel& pixel, KyUInt32 color);

	/*
	To be called after the fill of dynamicColumns: GetDynamicColumn().PushPixel().
	Copy dynamicColumns to finalColumns and release dynamicColumns.
	If mergeStatus is DynamicRasterCellMergeStatus_MergeEnabled, merge vertically the obstacles which are
	less than entity height from one another.
	*/
	void EndColumns(DynamicRasterCellMergeStatus mergeStatus);

	/* Once EndColumns() has been called, read FinalColumns. */
	const BoxOfArrays<KyFloat32>::Column& GetFinalColumnFromLocal(const PixelPos& localPixelPos) const;

	/* Once EndColumns() has been called, read FinalColumns. */
	const BoxOfArrays<KyFloat32>::Column& GetFinalColumn(const PixelPos& pixelPos) const;

	/* Once EndColumns() has been called, read FinalColumns. */
	const BoxOfArrays<KyUInt32>::Column& GetFinalTerrainTypeColumnFromLocal(const PixelPos& localPixelPos) const;

	/* Once EndColumns() has been called, read FinalColumns. */
	const BoxOfArrays<KyUInt32>::Column& GetFinalTerrainTypeColumn(const PixelPos& pixelPos) const;

	/* Write Raster file and Raster 3d obj file if specified in the generatorConfig, for debug purpose. */
	KyResult WriteFiles(PdgSystem* system) const;

	bool IsEmpty() const { return m_isEmpty; }
	
	const PixelBox& GetPixelBox() const { return m_pixelBox; }
	BoxOfArrays<KyFloat32>& GetFinalColumns() { return m_finalColumns; };
	const BoxOfArrays<KyFloat32>& GetFinalColumns() const { return m_finalColumns; };

	BoxOfArrays<KyUInt32>& GetFinalColorColumns() { return m_finalTerrainTypesColumns; };
	const BoxOfArrays<KyUInt32>& GetFinalColorColumns() const { return m_finalTerrainTypesColumns; };

	/* Copies the final columns from  another raster. Fails if celldesc is not the same. */
	KyResult Copy(const DynamicRasterCell& from);

	KyInt32 GetPixelSize() const { return m_pixelSize; }

private:
	void MakeFinalColumns(DynamicRasterCellMergeStatus mergeStatus);
	void ReleaseDynamicColumns();

private:
	KyInt32 m_pixelSize;
	PdgGlobalConfig m_config;
	CellDesc m_cellDesc;
	PixelBox m_pixelBox; // = m_cellDesc.m_enlargedPixelBox
	Array<const PdgInputCell*>* m_inputCells;

	// dynamic columns
	DynamicRasterColumnPool m_dynamicColumns;

	// final columns
	BoxOfArrays<KyFloat32> m_finalColumns;

	// final colors
	BoxOfArrays<KyUInt32> m_finalTerrainTypesColumns;
	
	bool m_isEmpty;

private:
	DynamicRasterCell(const DynamicRasterCell&);
	DynamicRasterCell& operator=(const DynamicRasterCell&);

friend class DynamicRasterCellBlobBuilder;
friend class FlyCell;
};



// --------------------------------- inline implementation ---------------------------------

inline void DynamicRasterCell::PushPixelToLocal(const PixelPos& localPixelPos, const SingleStagePixel& pixel, KyUInt32 color)
{
	KyInt32 idx = m_pixelBox.GetRowMajorIndexFromLocalPos(localPixelPos);
	PushPixel(idx, pixel, color);
}

inline void DynamicRasterCell::PushPixel(const PixelPos& pixelPos, const SingleStagePixel& pixel, KyUInt32 color)
{
	KyInt32 idx = m_pixelBox.GetRowMajorIndex(pixelPos);
	PushPixel(idx, pixel, color);
}

inline const BoxOfArrays<KyFloat32>::Column& DynamicRasterCell::GetFinalColumnFromLocal(const PixelPos& localPixelPos) const
{
	return m_finalColumns.GetColumn(localPixelPos.x, localPixelPos.y);
}

inline const BoxOfArrays<KyFloat32>::Column& DynamicRasterCell::GetFinalColumn(const PixelPos& pixelPos) const
{
	KyInt32 idx = m_pixelBox.GetRowMajorIndex(pixelPos);
	return m_finalColumns.GetColumn(idx);
}

inline const BoxOfArrays<KyUInt32>::Column& DynamicRasterCell::GetFinalTerrainTypeColumnFromLocal(const PixelPos& localPixelPos) const
{
	return m_finalTerrainTypesColumns.GetColumn(localPixelPos.x, localPixelPos.y);
}

inline const BoxOfArrays<KyUInt32>::Column& DynamicRasterCell::GetFinalTerrainTypeColumn(const PixelPos& pixelPos) const
{
	KyInt32 idx = m_pixelBox.GetRowMajorIndex(pixelPos);
	return m_finalTerrainTypesColumns.GetColumn(idx);
}


}


#endif
