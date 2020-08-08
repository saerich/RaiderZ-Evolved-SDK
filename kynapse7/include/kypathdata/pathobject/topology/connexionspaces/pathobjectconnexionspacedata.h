/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObject_ConnexionSpace_Data_H
#define KyPathData_PathObject_ConnexionSpace_Data_H


#include <kypathdata/pathobject/pathobjectdatatypes.h>
#include <kypathdata/math/vec3f.h>

namespace Kaim
{

/*
Defines the space to enter the PathObject as its minimal representation: the position of the connexion.
Serializable structure (Blob) containing data of a pathObject connection space, namely:
-A 3D position.
*/
class PathObjectConnexionSpaceData
{
public:
	static const char* GetBlobType() { return "PathObjectConnexionSpaceData"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	PathObjectConnexionSpaceData() {}

	
public:
	Vec3f m_position;              //!< Position of the connexion.
};

/*
Implements the endianness swap for all members of PathObjectConnexionSpaceData.
*/
inline void SwapEndianness(Endianness::Target e, PathObjectConnexionSpaceData& self)
{
	SwapEndianness(e, self.m_position);
}

}

#endif // #ifndef KyPathData_PathObject_ConnexionSpace_Data_H
