/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavMeshToConnectedComponents_H
#define KyPathData_NavMeshToConnectedComponents_H

#include "kypathdata/navmesh/navmesh.h"
#include "kypathdata/navmesh/navfloorptr.h"

namespace Kaim
{

class NavMeshManager;

class NavMeshToConnectedComponents
{
	KY_DEFINE_NEW_DELETE_OPERATORS
private:
	NavMeshToConnectedComponents();

public:
	static KyUInt32 ComputeConnectedComponents(const NavMeshManager& navMeshManager);
};


}


#endif //KyPathData_NavMeshToConnectedComponents_H

