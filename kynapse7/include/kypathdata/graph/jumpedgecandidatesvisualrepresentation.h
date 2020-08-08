/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// ---------- Primary contact: LASI - secondary contact: NOBODY
#ifndef KyPathData_JumpEdgeCandidateVisualRepresentation_H
#define KyPathData_JumpEdgeCandidateVisualRepresentation_H


#include "kypathdata/graph/jumpedgecandidatearray.h"
#include "kypathdata/visualsystem/visualrepresentation.h"
#include "kypathdata/visualsystem/visualcolor.h"

namespace Kaim
{


class GraphCell;


class JumpCandidatesVisualRepresentation : public VisualRepresentation
{
public:
	JumpCandidatesVisualRepresentation(const JumpEdgeCandidateArray* candidates) : VisualRepresentation(candidates) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);
};


}

#endif

