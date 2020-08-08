/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypdg */

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_FixedSizeSectorPartitioner_H
#define KyPdg_FixedSizeSectorPartitioner_H

#include "kypdg/generator/pdgsectorpartitioner.h"
#include "kypdg/generator/pdgsystem.h"
#include "kypdg/generator/celldesc.h"
#include "kypdg/pdginput/dynamicgrid.h"
#include "kypdg/generator/partitioningcelldesc.h"
#include "kypathdata/containers/stringstream.h"


namespace Kaim
{

class PdgSystem;
class PdgGuidCompound;
class FixedSizeSectorPartitioner_GuidGrid;

/*!	This implementation of PdgSectorPartitioner creates a regular horizontal grid of output sectors, each of which has a
	width and length equal to the value set in the constructor. Each time the Generator passes a cell to the Dispatch()
	method, the cell is placed in the output sector that covers the extents of that cell's bounding box. If a cell overlaps
	multiple output sectors, it is placed in all of the sectors that it overlaps.
	\pp This class uses the KyGuid::Generate() method to create GUIDs for the output sectors. Since this method is only
	implemented on Windows platforms, this class can only be used on Windows.
	\ingroup kypdg */
class FixedSizeSectorPartitioner : public PdgSectorPartitioner
{
public:
	typedef FixedSizeSectorPartitioner_GuidGrid GuidGrid;

public:
	/*! \label_constructor
		\param meterSectorSize		The width and length of each output sector. */
	FixedSizeSectorPartitioner(KyFloat32 meterSectorSize);

	/*! \label_virtualdestructor */
	virtual ~FixedSizeSectorPartitioner();

	/*! @copydoc PdgSectorPartitioner::Dispatch()
		For details on this implementation, see the FixedSizeSectorPartitioner class description. */
	virtual KyResult Dispatch(const PartitioningCellDesc& desc, PdgGuidCompound& guids);

	/*! @copydoc PdgSectorPartitioner::AssignName()
		This implementation assigns a name based on the (X,Y) position of the sector within the partitioning grid
		created by the Dispatch() method. */
	virtual KyResult AssignName(const KyGuid& guid, KyString& name);

protected: 
	// defines the step of the sector grid. Given in meters (kynapse internal units)
	KyFloat32 m_meterSectorSize;

	// maps grid pos <=> guid
	GuidGrid* m_guidGrid;
};

}


#endif // KyPdg_FixedSizeSectorPartitioner_H

