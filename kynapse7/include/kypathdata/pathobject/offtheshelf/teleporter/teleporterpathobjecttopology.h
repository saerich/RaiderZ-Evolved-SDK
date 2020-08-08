/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: None
#ifndef KYPATHDATA_TELEPORTER_PATHOBJECT_TOPOLOGY_H
#define KYPATHDATA_TELEPORTER_PATHOBJECT_TOPOLOGY_H

#include "kypathdata/pathobject/topology/dynamicpathobjecttopology.h"
#include "kypathdata/pathobject/topology/connexionspaces/dynamicpathobjectconnexionspace_cylinder.h"

namespace Kaim
{

/*	TeleporterPathObjectTopology describes the topology of a basic teleporter pathobject.
	It contains:
	-An entrance connexion space.
	-An exit connexion space.
*/
class TeleporterPathObjectTopology : public DynamicPathObjectTopology
{
public:
	TeleporterPathObjectTopology(
		Kaim::Vec3f entrance,
		Kaim::Vec3f exit,
		KyFloat32 height,
		KyFloat32 halfWidth,
		bool bidirectional);

	~TeleporterPathObjectTopology() {}

protected:
	Kaim::DynamicPathObjectConnexionSpace_Cylinder m_entrance;
	Kaim::DynamicPathObjectConnexionSpace_Cylinder m_exit;
};

} // namespace Kaim

#endif // KYPATHDATA_TELEPORTER_PATHOBJECT_TOPOLOGY_H
