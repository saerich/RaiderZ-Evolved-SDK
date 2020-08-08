/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavMeshManager_H
#define KyPathData_NavMeshManager_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/navtriangleptr.h"
#include "kypathdata/navmesh/navhalfedgeptr.h"
#include "kypathdata/navmesh/navvertexptr.h"
#include "kypathdata/navmesh/navmeshstitchdatamanager.h"
#include "kypathdata/navmesh/stitchednavmesh.h"
#include "kypathdata/navmesh/navcellgrid.h"
#include "kypathdata/navmesh/navcell.h"
#include "kypathdata/navmesh/navmesh.h"
#include "kypathdata/generic/activeguids.h"
#include "kypathdata/navmesh/navmeshmanagervisualrepresentation.h"

#include "kypathdata/math/box2f.h"
#include "kypathdata/math/box2i.h"

#include "kypathdata/kycrt/math.h"

namespace Kaim
{



enum NavMeshManagerRC
{
	NavMeshManager_SUCCESS = 0,
	NavMeshManager_INVALID_NAVMESH,
	NavMeshManager_INVALID_NAVCELL,
	NavMeshManager_NAVMESH_ALREADY_LOADED,
	NavMeshManager_NAVCELL_ALREADY_LOADED,
	NavMeshManager_NAVMESH_NOT_LOADED,
	NavMeshManager_NAVCELL_NOT_LOADED,
	NavMeshManager_NAVMESH_OVERLAP_ALREADY_LOADED_DATA,
	NavMeshManager_ERROR,
	NavMeshManagerRC_FORCE32 = 0xFFFFFFFF
};




/*!	This class is a runtime container for the \NavMeshes that represent the individual sectors in your terrain. 
	It provides a single point of access to the current set of \NavMeshes loaded into memory. 
	\pp Each Database owned by the World creates and initializes one instance of the NavMeshManager class, accessible through
	Database::GetNavMeshManager(). Any time you add PathData to a Database, the NavMesh contained in the PathData is added
	transparently to the NavMeshManager. The NavMeshManager stitches these separate \NavMeshes together transparently at
	runtime by creating links between their adjacent triangles.
	\pp To interact with the \NavMeshes that you load into the NavMeshManager, you should use an instance of NavMeshQueryServer:
	typically the NavMeshQueryServerWrapper accessible through Database::GetNavMeshQueryServer(). 
	\ingroup kyruntime_graph */
class NavMeshManager
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor For internal use. */
	NavMeshManager();
	/*! \label_destructor For internal use. */
	~NavMeshManager();

	/*! For internal use. */
	void Clear();

public: // --- VisualGeometry
	/*! Sets the instance of IVisualGeometry to which this object will send its triangles for rendering when
		ComputeVisualGeometry() is called. */
	void SetVisualGeometry(IVisualGeometry* geometry) { m_visualRepresentation.m_geometry = geometry; }

	/*! Retrieves the instance of IVisualGeometry to which this object sends its triangles for rendering, 
		or #KY_NULL if none has been set. */
	IVisualGeometry* GetVisualGeometry() { return m_visualRepresentation.m_geometry; }

	/*! Retrieves the instance of the NavMeshManagerVisualRepresentation maintained by this object, which describes 
		the way its data is expressed as triangles for rendering. */
	NavMeshManagerVisualRepresentation& GetVisualRepresentation() { return m_visualRepresentation; }

	/*! Sends to the IVisualGeometry a set of triangles that express the data maintained by this object for rendering.
		See :USERGUIDE:"Rendering PathData". */
	void ComputeVisualGeometry() { m_visualRepresentation.ComputeGeometry(); }

public: // --- Adding / Removing NavMeshes and NavCells

	/*! For internal use. */
	NavMeshReference* AddNavMesh(const NavMesh& navMesh); //<! insert a NavMesh. If StartMultipleInsertion() has not been called before, the navCellGrid will be reseted
	/*! For internal use. */
	NavMeshManagerRC RemoveNavMesh(const NavMesh& navMesh); //<! insert a NavMesh. If StartMultipleRemoval() has not been called before, the navCellGrid will be reseted

	/*! For internal use. */
	void StartMultipleInsertion(); //<! function to tell the NavMeshManager to not Reset the navCellGrid before the call of EndMultipleInsertion() Can be used to insert several navMeshes/navCells
	/*! For internal use. */
	void EndMultipleInsertion();   //<! function to tell the NavMeshManager the insertion is done, it can reset the navCellGrid

	/*! For internal use. */
	void StartMultipleRemoval(); //<! function to tell the NavMeshManager to not Reset the navCellGrid before the call of EndMultipleRemoval(). Can be used to remove several navMeshes/navCells
	/*! For internal use. */
	void EndMultipleRemoval();   //<! function to tell the NavMeshManager the insertion is done, it can reset the navCellGrid

	/*! Indicates the number of \NavMeshes that have been added to the NavMeshManager. */
	KyUInt32 GetNumberOfInsertedNavMesh() const { return m_stitchDataManager.m_numberOfStitchedNavMesh; }

	/*! Retrieves the NavMesh with the specified index within the list of \NavMeshes added to this NavMeshManager. */
	const NavMesh& GetInsertedNavMesh(KyUInt32 idx) const
	{
		KY_FUNCTION("NavMeshManager::GetInsertedNavMesh");
		KY_ASSERTK(idx < m_stitchDataManager.m_numberOfStitchedNavMesh);
		return m_stitchDataManager.m_bufferOfStitchedNavMesh[idx]->m_navMesh;
	}

	/*! Retrieves the CellBox around all \NavCells added to this NavMeshManager. */
	KY_FORCE_INLINE const CellBox& GetCellBox() const { return m_navCellGrid.m_cellBox; }

	/*! Retrieves the length and width of each NavCell added to this NavMeshManager. */
	KY_FORCE_INLINE KyFloat32 GetCellSize() const { return m_cellSize; }

	/*! Retrieves the radius of the character that the \NavMeshes added to this NavMeshManager were generated for. */
	KY_FORCE_INLINE KyFloat32 GetGenerationRadius() const { return m_generationRadius; }

	/*! Retrieves the height of the character that the \NavMeshes added to this NavMeshManager were generated for. */
	KY_FORCE_INLINE KyFloat32 GetGenerationHeight() const { return m_generationHeight; }

public: // --- Ask information about presence and stitching

	/*! For internal use. */
	const StitchedNavCell* GetStitchedCell(const CellPos& cellPos) const { return m_navCellGrid.GetStitchedCell(cellPos); }

	/*! For internal use. */
	StitchedNavCell* GetStitchedCell(const CellPos& cellPos) { return m_navCellGrid.GetStitchedCell(cellPos); }

	/*! For internal use. */
	const StitchedNavCell* GetStitchedCell(const Vec3f& pos) const
	{
		const CellPos cellPos(ComputeNavCellPos(pos));
		return m_navCellGrid.GetStitchedCell(cellPos);
	}

	/*! Indicates whether the NavCell at the specified position within the grid has been loaded. */
	bool IsCellAvailiable(const CellPos& cellPos) const { return m_navCellGrid.IsCellAvailiable(cellPos); }

	/*! Retrieves the NavCell at the specified position within the grid. */
	const NavCell* GetNavCell(const CellPos& cellPos) const { return m_navCellGrid.GetNavCell(cellPos); }

public:

	/*! Retrieves the position within the grid of the NavCell that is responsible for the specified (X,Y) coordinates. */
	KY_FORCE_INLINE CellPos ComputeNavCellPos(const Vec2f& pos) const
	{
		KY_FUNCTION("NavMeshManager::ComputeNavCellPos");
		KY_ASSERT(m_cellSize != KyFloat32MAXVAL, ("You should insert at least a valid navCell first"));

		// Ceilf - 1 on x and Floorf on y to be IsInsideTriangle compatible
		return m_navCellGrid.ComputeNavCellPos(pos, (KyInt32) Ceilf((pos.x * m_cellSizeInv)) - 1, (KyInt32) Floorf((pos.y * m_cellSizeInv)));
	}
	/*! Retrieves the position within the grid of the NavCell that is responsible for the specified (X,Y) coordinates.
		\param pos				The coordinates for which to retrieve the NavCell.
		\param[out] cellPos		The position within the grid of the NavCell responsible for \c pos. */
	KY_FORCE_INLINE void ComputeNavCellPos(const Vec2f& pos, CellPos& cellPos) const
	{
		KY_FUNCTION("NavMeshManager::ComputeNavCellPos");
		KY_ASSERT(m_cellSize != KyFloat32MAXVAL, ("You should insert at least a valid navCell first"));

		// Ceilf - 1 on x and Floorf on y to be IsInsideTriangle Floorf
		m_navCellGrid.ComputeNavCellPos(pos, (KyInt32) Ceilf((pos.x * m_cellSizeInv)) - 1, (KyInt32) Floorf((pos.y * m_cellSizeInv)), cellPos);
	}

		/*! Retrieves the position within the grid of the NavCell that is responsible for the specified coordinates. */
	KY_FORCE_INLINE CellPos ComputeNavCellPos(const Vec3f& pos) const { return ComputeNavCellPos(Vec2f(pos.x, pos.y)); }

	/*! Retrieves the position within the grid of the NavCell that is responsible for the specified coordinates.
		\param pos				The coordinates for which to retrieve the NavCell.
		\param[out] cellPos		The position within the grid of the NavCell responsible for \c pos. */
	KY_FORCE_INLINE void ComputeNavCellPos(const Vec3f& pos, CellPos& cellPos) const
	{
		ComputeNavCellPos(Vec2f(pos.x, pos.y), cellPos);
	}

	/*! Indicates whether or not the specified NavMesh was created with the same generation parameters
		as the \NavMeshes that have already been loaded into this NavMeshManager. */
	bool IsCompatibleWith(const NavMesh& navMesh) const;

public:
	/*! @{ */
	/*! For internal use. Do not modify. */
	NavMeshStitchDataManager m_stitchDataManager; 
	NavMeshReferenceManager m_navMeshReferenceManager;
	NavCellGrid m_navCellGrid;

	KyFloat32 m_generationRadius;
	KyFloat32 m_generationHeight;
	KyFloat32 m_cellSize;
	KyFloat32 m_cellSizeInv;

	ActiveGuids m_activatedGuids;

	NavMeshManagerVisualRepresentation m_visualRepresentation;
	/*! @} */

private:
	KyUInt32 m_numberOfMeshToUpdate;
	CellBox m_cellBox;

	typedef KyUInt32 InsertionMode;
	static const InsertionMode SINGLE_INSERTION = 0;
	static const InsertionMode MULTIPLE_INSERTION = 1;
	InsertionMode m_insertionMode;

	typedef KyUInt32 RemovalMode;
	static const RemovalMode SINGLE_REMOVAL = 0;
	static const RemovalMode MULTIPLE_REMOVAL = 1;
	RemovalMode m_removalMode;
};



}


#endif //KyPathData_NavMeshManager_H

