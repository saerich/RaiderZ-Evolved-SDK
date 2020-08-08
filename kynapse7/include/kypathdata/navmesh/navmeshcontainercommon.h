/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavmeshContainerCommon_H
#define KyPathData_NavmeshContainerCommon_H

namespace Kaim
{

enum ContainerResult
{
	ContainerResult_SUCCESS,
	ContainerResult_FULL_BUFFER,
	ContainerResult_OUT_OF_RANGE,

	ContainerResult_FORCE32 = 0xFFFFFFFF
};

}

#endif // KyPathData_NavmeshContainerCommon_H

