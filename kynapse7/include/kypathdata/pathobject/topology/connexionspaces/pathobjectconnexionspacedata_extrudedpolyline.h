/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObject_ConnexionSpace_Data_ExtrudedPolyline_H
#define KyPathData_PathObject_ConnexionSpace_Data_ExtrudedPolyline_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/math/blob/extrudedpolylinedata.h"

namespace Kaim
{

/*
Defines the space to enter the PathObject.
*/
class PathObjectConnexionSpaceData_ExtrudedPolyline
{
public:
	static const char* GetBlobType() { return "PathObjectConnexionSpaceData_ExtrudedPolyline"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	PathObjectConnexionSpaceData_ExtrudedPolyline() {}

	
public:
	ExtrudedPolylineData m_extrudedPolyline;
	Vec3f m_connexionPosition;
};

/*
Implements the endianness swap for all members of PathObjectConnexionSpaceData_ExtrudedPolyline.
*/
inline void SwapEndianness(Endianness::Target e, PathObjectConnexionSpaceData_ExtrudedPolyline& self)
{
	SwapEndianness(e, self.m_extrudedPolyline);
	SwapEndianness(e, self.m_connexionPosition);
}

}

#endif // #ifndef KyPathData_PathObject_ConnexionSpace_Data_ExtrudedPolyline_H
