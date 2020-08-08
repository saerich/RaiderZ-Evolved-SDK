/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: None
#ifndef KYPATHDATA_JUMP_PATHOBJECT_TOPOLOGY_H
#define KYPATHDATA_JUMP_PATHOBJECT_TOPOLOGY_H

#include "kypathdata/pathobject/topology/dynamicpathobjecttopology.h"
#include "kypathdata/pathobject/topology/connexionspaces/dynamicpathobjectconnexionspace_cylinder.h"
#include "kypathdata/pathobject/topology/controlspaces/dynamicpathobjectcontrolspace_extrudedrectangle.h"
#include "kypathdata/pathobject/topology/controlspaces/dynamicpathobjectcontrolspace_box.h"

namespace Kaim
{

/*	JumpPathObjectTopology describes the topology of a basic jump pathobject.
	It contains:
	-An entrance connexion space.
	-An exit connexion space.
	-A control space describing the jump trajectory.
*/
class JumpPathObjectTopology : public DynamicPathObjectTopology
{
public:
	JumpPathObjectTopology(
		Kaim::Vec3f entrance,
		Kaim::Vec3f exit,
		KyFloat32 height,
		KyFloat32 halfWidth,
		KyFloat32 jumpHeight,
		bool bidirectional);

	~JumpPathObjectTopology() {}

public:
	Kaim::DynamicPathObjectConnexionSpace_Cylinder m_entrance;
	Kaim::DynamicPathObjectConnexionSpace_Cylinder m_exit;
	Kaim::DynamicPathObjectControlSpace_ExtrudedRectangle m_controlSpace;
};

} // namespace Kaim

#endif // KYPATHDATA_JUMP_PATHOBJECT_TOPOLOGY_H
