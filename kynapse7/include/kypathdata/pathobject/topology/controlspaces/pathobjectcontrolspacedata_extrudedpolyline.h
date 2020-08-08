/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObject_ControlSpace_Data_ExtrudedPolyline_H
#define KyPathData_PathObject_ControlSpace_Data_ExtrudedPolyline_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/math/blob/extrudedpolylinedata.h"

namespace Kaim
{

/*
Defines the space (an extruded polyline) in which the PathObject has the control of the entity.
Serializable structure (Blob) containing data of a pathObject control space, namely:
-An extruded polyline in meters in Kynapse coordinates.
*/
class PathObjectControlSpaceData_ExtrudedPolyline
{
public:
	static const char* GetBlobType() { return "PathObjectControlSpaceData_ExtrudedPolyline"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	PathObjectControlSpaceData_ExtrudedPolyline() {}

	
public:
	ExtrudedPolylineData m_extrudedPolyline;        //!< Extruded polyline representing the control space.
	bool m_canLockEdges;  //!< True if the control space can lock edges
	bool m_isAnObstacle;  //!< True if the control space should be considered as an obstacle for entity not using the PathObject.
	bool m_isASpaceToStartUsingThePathObject;
};

/*
Implements the endianness swap for all members of PathObjectControlSpaceData_ExtrudedPolyline.
*/
inline void SwapEndianness(Endianness::Target e, PathObjectControlSpaceData_ExtrudedPolyline& self)
{
	SwapEndianness(e, self.m_extrudedPolyline);
}

}

#endif // #ifndef KyPathData_PathObject_ControlSpace_Data_ExtrudedPolyline_H
