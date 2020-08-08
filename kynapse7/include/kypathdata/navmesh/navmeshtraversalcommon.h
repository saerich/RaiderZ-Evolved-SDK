/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavmeshTraversalCommon_H
#define KyPathData_NavmeshTraversalCommon_H


namespace Kaim
{

enum TraversalResult
{
	TraversalResult_DONE,
	TraversalResult_INVALID_MEMORY_FOR_CLOSED_NODES,
	TraversalResult_INVALID_MEMORY_FOR_OPEN_NODES,
	TraversalResult_INVALID_MEMORY_FOR_VISITED_NODES,
	TraversalResult_LACK_OF_MEMORY_FOR_CLOSED_NODES,
	TraversalResult_LACK_OF_MEMORY_FOR_OPEN_NODES,
	TraversalResult_LACK_OF_MEMORY_FOR_VISITED_NODES,
	TraversalResult_ERROR,

	TraversalResult_FORCE32 = 0xFFFFFFFF
};

}

#endif // KyPathData_NavmeshTraversalCommon_H

