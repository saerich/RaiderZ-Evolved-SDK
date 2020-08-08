/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypdg */

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_PdgSectorPartitioner_H
#define KyPdg_PdgSectorPartitioner_H

#include "kypathdata/generic/kyguid.h"
#include "kypathdata/containers/kystring.h"
#include "kypdg/generator/partitioningcelldesc.h"
#include "kypathdata/containers/array.h"

namespace Kaim
{


class PdgSystem;
class PdgGuidCompound;

/* For internal use. */
struct PartitionerGeneratedSectorDesc : public VirtualMemObject
{
	PartitionerGeneratedSectorDesc() {} // default Ctor 
	PartitionerGeneratedSectorDesc(const KyString& name, const KyGuid& guid )
		: m_name(name), m_guid(guid) {};
	KyString m_name;
	KyGuid m_guid;
};

namespace Result
{
	/*! Can be returned by PdgSectorPartitioner::Dispatch() in order to leave a cell in the original input sector. \ingroup kypdg */
	static const KyResult PdgSectorPartitioner_SkipDispatch = 1;
}
/*!	This class provides an abstract base interface for an object that the Generator can invoke in order to partition
	an input sector automatically into multiple smaller output sectors. If you want to partition an area of your terrain
	automatically, you can provide an object that derives from this class in the PdgSector constructor.
	\pp This class cannot be used as-is. You must use a class that derives from PdgSectorPartitioner and that implements
	at a minimum the virtual Dispatch() method. \SDKName provides a default implementation, FixedSizeSectorPartitioner,
	which sub-divides the input sector into a regular grid of output sectors. You can also create your own implementation of
	PdgSectorPartitioner in order to partition an input sector according to a different logic.
	\pp See also :USERGUIDE:"Sectorizing the Terrain".
	\ingroup kypdg */
class PdgSectorPartitioner : public VirtualMemObject
{
public:

	/*! \label_baseconstructor */
	PdgSectorPartitioner() {}

	/*!	The Generator calls this method iteratively, once for each cell in the PathData generated for the input sector.
		This method is expected to determine which output sector or sectors should contain that cell, and to return to the
		Generator the GUIDs of those output sectors. 
		\pp The GUIDs of the output sectors are up to you to create. If this method dispatches a cell to the GUID of an existing
		sector, the cell is added to that sector. If this method dispatches a cell to a GUID that does not match any existing sectors,
		the Generator makes a new sector that contains only that cell, and calls AssignName() to generate a unique descriptive name
		for the new sector. See the PdgGuidCompound and KyGuid classes.
		\param[in] PartitioningCellDesc&		Contains information about the location and bounding box of the cell, which the
												Dispatch() method can use in order to decide in which output sector the cell belongs.
		\param[out] PdgGuidCompound&			Contains a compound of the GUIDs of all output sectors that should contain the cell
												described in the PartitioningCellDesc.
		\retval #KY_ERROR						The method was unable to dispatch the cell to any output sector.
		\retval #KY_SUCCESS						The method completed its computations successfully, and updated the output parameter to
												indicate the output sectors in which the cell should be placed.
		\retval Result::PdgSectorPartitioner_SkipDispatch	Indicates that the cell should be kept in the original input sector, and not
												dispatched to any other output sectors.
		\pp No default implementation is provided; derived classes must implement this method. */
	virtual KyResult Dispatch(const PartitioningCellDesc& , PdgGuidCompound& ) = 0;

	/*!	The Generator calls this method once for each new output sector the Dispatch() identifies. This method is expected
		to provide a unique name for the output sector based on its GUID. This unique name will be appended to the name of the
		original input sector when naming output files generated for the sector. For example, if the original input sector was
		named "sector1", and the AssignName() method assigns the name "subsectorA" to an output sector, the .PathData file generated 
		for the output sector will be named "sector1_subsectorA.PathData".
		\param[in] guid			The GUID of an output sector created by the Dispatch() method.
		\param[out] name		A unique name for the output sector identified by \c guid.
		\return a #KyResult that indicates the success or failure of the computation.
		\pp The default implementation simply converts the GUID to a string. */
	virtual KyResult AssignName(const KyGuid& guid, KyString& name)
	{
		char nameArray[37];
		guid.ToString(nameArray);
		name = nameArray;
		return KY_SUCCESS;
	}

	// store the name of the sub sectors we will create using this partitioner. 
	// automatically called by the PdgSectorGenerator when saving Pathdata. 
	// can be used to keep track of the generated pathdata
	/*! @{ */
	/*! For internal use. */
	void AddGeneratedSectorDesc(const PartitionerGeneratedSectorDesc& desc) { m_generatedSubSectorDescs.PushBack(desc); }
	void ClearGeneratedSectorDescs() { m_generatedSubSectorDescs.Clear(); }
	Array<PartitionerGeneratedSectorDesc>& GetGeneratedSectorDescs() { return m_generatedSubSectorDescs; }
	/*! @} */

protected: 
	/*! For internal use. */
	Array<PartitionerGeneratedSectorDesc> m_generatedSubSectorDescs;
};

}


#endif // KyPdg_PdgSectorPartitioner_H

