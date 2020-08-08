
/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// Primary contact: LASI - secondary contact: NONE
#ifndef KyPathData_AdditionalGraphBlobBuilder_H
#define KyPathData_AdditionalGraphBlobBuilder_H

#include "kypathdata/graph/graph.h"
#include "kypathdata/containers/array.h"
#include "kypathdata/graph/additionalgraph.h"

namespace Kaim
{

class DynamicAdditionalGraph;
class DynamicAdditionalGraphVertex;
class DynamicAdditionalGraphEdge;

class AdditionalGraphBlobBuilder_ToClassicGraph: public BaseBlobBuilder<Graph>
{
public:
	AdditionalGraphBlobBuilder_ToClassicGraph(const DynamicAdditionalGraph* additionalGraph): m_graph(additionalGraph) {};

private:
	virtual void DoBuild();
	void ComputeBuffers();
	void ComputeBBox();

private:
	const DynamicAdditionalGraph* m_graph;
	Array<DynamicAdditionalGraphVertex*> m_vertexBuffer;
	Array<DynamicAdditionalGraphEdge*> m_edgeBuffer;
	Box3f m_boundingBox;

};

class AdditionalGraphBlobBuilder : public BaseBlobBuilder<AdditionalGraph>
{
public:
	AdditionalGraphBlobBuilder(const DynamicAdditionalGraph* additionalGraph): m_graph(additionalGraph), m_graphbuilder(additionalGraph) {};

private:
	virtual void DoBuild();
	void ComputeBuffers();

private:
	const DynamicAdditionalGraph* m_graph;
	Array<DynamicAdditionalGraphVertex*> m_vertexBuffer;
	Array<DynamicAdditionalGraphEdge*> m_edgeBuffer;
	AdditionalGraphBlobBuilder_ToClassicGraph m_graphbuilder;

};


}

#endif // KyPathData_AdditionalGraphBlobBuilder_H
