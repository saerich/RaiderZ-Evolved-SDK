/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObjectData_H
#define KyPathData_PathObjectData_H


#include "kypathdata/blob/blobarray.h"
#include <kypathdata/basesystem/coordsystem.h>
#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/topology/pathobjecttopologydata.h"
#include "kypathdata/pathobject/info/pathobjectinfodata.h"

namespace Kaim
{

/*
Serializable structure (Blob) containing all basic data of a pathObject, namely:
-The unique ID of the pathObject
-The Type of the PathObject:
   eg: ladder, door, lift,...
-PathObject Connexion spaces: defines spaces where the PathObject must be connected to the PointGraphCell. 
   eg: Connexion space 1 on one side of a door, connexion space 2 on the other side.
-PathObject Connexion links: defines the links between connexion spaces.
   eg: For a lift: link1 = [ [ConnexionSpaceFloor1, ConnexionSpaceFloor2], bidirectional ]
                   link2 = [ [ConnexionSpaceFloor1, ConnexionSpaceFloor3], bidirectional ]
                   link3 = [ [ConnexionSpaceFloor2, ConnexionSpaceFloor3], bidirectional ]
-PathObject Control spaces: areas covered by the PathObject
   eg: For a door, the door volume + a flag forbidding other graph edges to go intersect this volume.
*/
class PathObjectData
{
public:
	static const char* GetBlobType() { return "PathObjectData"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	PathObjectData() {}
	
public:
	PathObjectInfoData m_info;
	PathObjectTopologyData m_topology;

private:
	PathObjectData& operator=(const PathObjectData& pathObjectData);
	PathObjectData(const PathObjectData& pathObjectData);
};

/*
Implements the endianness swap for all members of PathObjectData.
*/
inline void SwapEndianness(Endianness::Target e, PathObjectData& self)
{
	SwapEndianness(e, self.m_info);
	SwapEndianness(e, self.m_topology);
}

}

#endif // #ifndef KyPathData_PathObjectData_H
