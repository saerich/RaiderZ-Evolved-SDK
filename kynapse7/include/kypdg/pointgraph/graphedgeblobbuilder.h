/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyPdg_GraphEdgeBlobBuilder_H
#define KyPdg_GraphEdgeBlobBuilder_H

#include "kypathdata/graph/graphedge.h"
#include "kypathdata/blob/baseblobbuilder.h"


namespace Kaim
{


class PdgGraphEdge;


class GraphEdgeBlobBuilder : public BaseBlobBuilder<GraphEdge>
{
public:
	GraphEdgeBlobBuilder(PdgGraphEdge* pdgGraphEdge) : m_pdgGraphEdge(pdgGraphEdge) {}

private:
	virtual void DoBuild();

	PdgGraphEdge* m_pdgGraphEdge;
};


}

#endif
