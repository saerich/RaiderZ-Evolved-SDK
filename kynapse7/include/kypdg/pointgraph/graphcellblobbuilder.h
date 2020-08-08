/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyPdg_GraphCellBlobBuilder_H
#define KyPdg_GraphCellBlobBuilder_H

#include "kypathdata/graph/graphcell.h"
#include "kypathdata/blob/baseblobbuilder.h"


namespace Kaim
{


class DynamicPointGraphCell;
class PdgGraphVertex;


class GraphCellBlobBuilder : public BaseBlobBuilder<GraphCell>
{
public:
	GraphCellBlobBuilder(const DynamicPointGraphCell* dynPointGraphCell) : m_dynPointGraphCell(dynPointGraphCell) {}

private:
	virtual void DoBuild();
private:
	const DynamicPointGraphCell* m_dynPointGraphCell;
};

} // namespace Kaim

#endif
