/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObject_ConnexionSpace_Data_Cylinder_H
#define KyPathData_PathObject_ConnexionSpace_Data_Cylinder_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"

namespace Kaim
{

/*
Defines the space to enter the PathObject as a vertical cylinder.
Serializable structure (Blob) containing data of a pathObject connection space, namely:
-A 3D position.
-A radius in meters.
-A height in meters.
*/
class PathObjectConnexionSpaceData_Cylinder
{
public:
	static const char* GetBlobType() { return "PathObjectConnexionSpaceData_Cylinder"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	PathObjectConnexionSpaceData_Cylinder() {}

	
public:
	Vec3f m_position;              //!< Position of the connexion space.
	KyFloat32 m_height;            //!< Height of the connexion space.
	KyFloat32 m_radius;            //!< Radius of the connexion space.
};

/*
Implements the endianness swap for all members of PathObjectConnexionSpaceData_Cylinder.
*/
inline void SwapEndianness(Endianness::Target e, PathObjectConnexionSpaceData_Cylinder& self)
{
	SwapEndianness(e, self.m_position);
	SwapEndianness(e, self.m_height);
	SwapEndianness(e, self.m_radius);
}

}

#endif // #ifndef KyPathData_PathObject_ConnexionSpace_Data_Cylinder_H
