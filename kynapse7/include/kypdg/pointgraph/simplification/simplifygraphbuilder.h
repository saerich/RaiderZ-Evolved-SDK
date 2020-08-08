/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* This computer source code and related instructions and comments are the
* unpublished confidential and proprietary information of Autodesk, Inc. and
* are protected under applicable copyright and trade secret law. They may
* not be disclosed to, copied or used by any third party without
* the prior written consent of Autodesk, Inc.
*/

#ifndef KyPdg_SimplifyGraphBuilder_H
#define KyPdg_SimplifyGraphBuilder_H

#include "kypdg/pointgraph/dynamicpointgraphbuilder.h"

namespace Kaim
{

class CoverageInfo;
class DynamicNavFloor;
class PdgPointGraph;

class SimplifyGraphBuilder : public IDynamicPointGraphBuilder
{
public:
	SimplifyGraphBuilder(const PdgSystem* sys);

	virtual KyResult BuildPointGraphFromDynamicNavCell(DynamicNavCell& navCell, DynamicPointGraphCell& graph);
protected:
	void BuildPointGraphFromDynamicNavFloor(const DynamicNavCell& navCell, const DynamicNavFloor& navFloor, PdgPointGraph& pdgPointGraph, DynamicNavCell& triangulation);
	KyResult ComputeRawJumpCandidates(const DynamicNavCell& navcell, DynamicPointGraphCell& graph) const;
};

} // namespace Kaim

#endif // KyPdg_SimplifyGraphBuilder_H
