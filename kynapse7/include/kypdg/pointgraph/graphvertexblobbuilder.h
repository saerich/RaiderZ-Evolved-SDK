/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyPdg_GraphVertexBlobBuilder_H
#define KyPdg_GraphVertexBlobBuilder_H

#include "kypathdata/graph/graphvertex.h"
#include "kypathdata/blob/baseblobbuilder.h"


namespace Kaim
{


class PdgPointGraph;
class PdgGraphVertex;
class PdgGraphEdge;


class GraphVertexBlobBuilder : public BaseBlobBuilder<GraphVertex>
{
public:
	GraphVertexBlobBuilder(const PdgPointGraph* pdgPointGraph) :
		m_pdgPointGraph(pdgPointGraph), m_pdgGraphVertex(KY_NULL) {}

	void SetPdgGraphVertex(const PdgGraphVertex* pdgGraphVertex) { m_pdgGraphVertex = pdgGraphVertex; }

private:
	virtual void DoBuild();

private:
	const PdgPointGraph* m_pdgPointGraph;
	const PdgGraphVertex* m_pdgGraphVertex;
};


}

#endif
