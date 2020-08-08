/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_AdditionalStitchedGraph_H
#define KyPathData_AdditionalStitchedGraph_H

#include "kyruntime/graph/stitchedgraph.h"
#include "kypathdata/graph/additionalgraph.h"

namespace Kaim
{

/* For internal use. */
class AdditionalStitchedGraph : public StitchedGraph
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	AdditionalStitchedGraph& operator=(const AdditionalStitchedGraph&);

friend class GraphStitchDataManager;
	AdditionalStitchedGraph(const Graph& graph) : StitchedGraph(graph), m_additionalGraph(KY_NULL) {}
	AdditionalStitchedGraph() : StitchedGraph(), m_additionalGraph(KY_NULL) {}


public:
	const AdditionalGraph* m_additionalGraph;
};


}

#endif //KyPathData_AdditionalStitchedGraph_H

