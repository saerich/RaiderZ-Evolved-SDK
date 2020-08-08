/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// Primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_GraphBlobBuilder_H
#define KyPdg_GraphBlobBuilder_H


#include "kypathdata/graph/graph.h"
#include "kypathdata/blob/baseblobbuilder.h"
#include "kypdg/common/stlvector.h"


namespace Kaim
{

class PdgSystem;
class DynamicPointGraphCell;
class PdgGuidCompound;


class GraphBlobBuilder : public BaseBlobBuilder<Graph>
{
public:
	GraphBlobBuilder(PdgSystem* sys, const PdgGuidCompound* guidCompound, const CellBox& cellBox, const StlVector<const DynamicPointGraphCell*>* dynPointGraphCells, Graph::GraphFlags flags = Graph::GRAPH_NO_FLAG);

private:
	virtual void DoBuild();

private:
	PdgSystem* m_sys;
	const PdgGuidCompound* m_guidCompound;
	CellBox m_cellBox;
	const StlVector<const DynamicPointGraphCell*>* m_dynPointGraphCells;
	Graph::GraphFlags m_flags;
};


}

#endif
