/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObject_ControlSpace_Data_ExtrudedRect_H
#define KyPathData_PathObject_ControlSpace_Data_ExtrudedRect_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/math/blob/extrudedrectangledata.h"

namespace Kaim
{

/*
Defines the space (an extruded rectangle) in which the PathObject has the control of the entity.
Serializable structure (Blob) containing data of a pathObject control space, namely:
-An extruded rectangle in meters in Kynapse coordinates.
*/
class PathObjectControlSpaceData_ExtrudedRectangle
{
public:
	static const char* GetBlobType() { return "PathObjectControlSpaceData_ExtrudedRectangle"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	PathObjectControlSpaceData_ExtrudedRectangle() {}

	
public:
	ExtrudedRectangleData m_extrudedRectangle;        //!< Extruded rectangle representing the control space.
	bool m_canLockEdges;  //!< True if the control space can lock edges
	bool m_isAnObstacle;  //!< True if the control space should be considered as an obstacle for entity not using the PathObject.
	bool m_isASpaceToStartUsingThePathObject;
};

/*
Implements the endianness swap for all members of PathObjectControlSpaceData_ExtrudedRectangle.
*/
inline void SwapEndianness(Endianness::Target e, PathObjectControlSpaceData_ExtrudedRectangle& self)
{
	SwapEndianness(e, self.m_extrudedRectangle);
}

}

#endif // #ifndef KyPathData_PathObject_ControlSpace_Data_ExtrudedRect_H
