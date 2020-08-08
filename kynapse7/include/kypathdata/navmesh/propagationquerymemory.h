/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_PropagationQueryMemory_H
#define KyPathData_PropagationQueryMemory_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"

namespace Kaim
{

/*! An instance of this class is used by the classes that conduct queries against the NavMesh, in order to
	determine whether or not to keep a list of the NavMesh triangles crossed by those queries, and to access
	that list of triangles. 
	\pp By default, the list of crossed triangles is not kept. To record the list, you must set the 
	#rememberCrossedTriangleInServerBuffer value to \c true before you carry out your query. The
	NavMeshQueryServer will record the crossed triangles in a memory buffer that it pre-allocates for
	the purpose. The NavMeshQueryServer updates #m_crossedTriangles to point to this memory buffer, and
	updates #m_crossedTrianglesCount to indicate the number of triangles it writes to the buffer.
	\pp Note that the memory buffer containing the triangles will be overwritten during the next query
	that you make against the NavMeshQueryServer.
	\pp This class is used by DiskCanGoQuery, DiskCastQuery, DiskCollisionQuery, DiskExpansionQuery,
	RayCanGoQuery, RayCastQuery, SegmentCanGoQuery and SegmentCastQuery.
	\ingroup kypathdata_navmesh */
class PropagationQueryMemory
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor For internal use. */
	PropagationQueryMemory() :  rememberCrossedTriangleInServerBuffer(false), m_crossedTriangles(KY_NULL), m_crossedTrianglesCount(0) {}

	// debug or rendering
	/*! Indicates whether or not the NavMeshQueryServer should record the list of NavMesh triangles crossed by the
		query that owns this object. */
	bool rememberCrossedTriangleInServerBuffer;

	/*! When #rememberCrossedTriangleInServerBuffer is set to \c true, this pointer is updated by the NavMeshQueryServer to
		indicate the location of the memory buffer in which the triangles are written. Note that the contents of this buffer
		will be overwritten during the next query that you make against the NavMeshQueryServer. */
	NavTrianglePtr* m_crossedTriangles;

	/*! When #rememberCrossedTriangleInServerBuffer is set to \c true, this value is updated by the NavMeshQueryServer to
		indicate the number of triangles written to the #m_crossedTriangles buffer. */
	KyUInt32 m_crossedTrianglesCount;
};


}


#endif //KyPathData_SegmentCastVisitor_H

