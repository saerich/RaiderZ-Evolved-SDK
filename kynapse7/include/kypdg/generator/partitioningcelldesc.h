/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	

/*! \file
	\ingroup kypdg */

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_PartitioningCellDesc_H
#define KyPdg_PartitioningCellDesc_H

#include "kypathdata/math/box2f.h"
#include "kypathdata/navmesh/navmeshtypes.h"


namespace Kaim
{

/*! Passed by the Generator to a PdgSectorPartitioner in order to describe a PathData cell in the input sector. Based on this information,
	the PdgSectorPartitioner can decide which output sector should contain the cell.
	\ingroup kypdg */
class PartitioningCellDesc
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	CellPos m_cellPos; /*!< The (X,Y) position of this cell within the grid of PathData cells that make up the input sector. */
	Box2f m_kynapseBox2f; /*!< The axis-aligned bounding box around this cell, expressed in the \SDKName coordinate system. */
	Box2f m_clientBox2f; /*!< The axis-aligned bounding box around this cell, expressed in the \gameOrSim engine coordinate system. */
};

}


#endif // KyPdg_PartitioningCellDesc_H
