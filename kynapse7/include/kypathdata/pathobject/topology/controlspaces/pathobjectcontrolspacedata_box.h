/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObject_ControlSpace_Data_Box_H
#define KyPathData_PathObject_ControlSpace_Data_Box_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/math/box3f.h"

namespace Kaim
{

/*
Defines the space (a box) in which the PathObject has the control of the entity.
Serializable structure (Blob) containing data of a pathObject control space, namely:
-A bbox in meters.
*/
class PathObjectControlSpaceData_Box
{
public:
	static const char* GetBlobType() { return "PathObjectControlSpaceData_Box"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	PathObjectControlSpaceData_Box() {}

	
public:
	Box3f m_box;        //!< Box of the control space.
	bool m_canLockEdges;  //!< True if the control space can lock edges
	bool m_isAnObstacle;  //!< True if the control space should be considered as an obstacle for entity not using the PathObject.
	bool m_isASpaceToStartUsingThePathObject;
};

/*
Implements the endianness swap for all members of PathObjectControlSpaceData_Box.
*/
inline void SwapEndianness(Endianness::Target e, PathObjectControlSpaceData_Box& self)
{
	SwapEndianness(e, self.m_box);
}

}

#endif // #ifndef KyPathData_PathObject_ControlSpace_Data_Box_H
