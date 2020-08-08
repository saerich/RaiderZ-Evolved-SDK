/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_JumpCandidateBlobBuilder_H
#define KyPdg_JumpCandidateBlobBuilder_H

#include "kypathdata/generic/memory.h"
#include "kypathdata/blob/baseblobbuilder.h"
#include "kypathdata/graph/jumpedgecandidatearray.h"
#include "kypdg/common/stlvector.h"
#include "kypdg/pointgraph/pdgpointgraph.h"

namespace Kaim
{

class PdgGuidCompound;


class JumpCandidateBlobBuilder : public BaseBlobBuilder<JumpEdgeCandidateArray>
{
public:
	JumpCandidateBlobBuilder(PdgGuidCompound* pdgGuidCompound, StlVector<PdgPointGraph::AsymmetricCandidate>* candidates) :
		m_pdgGuidCompound(pdgGuidCompound), m_candidates(candidates)
	{}

private:
	virtual void DoBuild();

	PdgGuidCompound* m_pdgGuidCompound;
	StlVector<PdgPointGraph::AsymmetricCandidate>* m_candidates;
};


}


#endif //KyPdg_JumpCandidateBlobBuilder_H
