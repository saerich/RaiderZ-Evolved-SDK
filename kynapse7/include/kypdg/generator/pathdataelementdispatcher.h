/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/



// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_PathdataElementDispatcher_H
#define KyPdg_PathdataElementDispatcher_H

#include "kypathdata/generic/guidcompound.h"
#include "kypdg/common/stlmap.h"
#include "kypdg/generator/partitioningcelldesc.h"
#include "kypdg/generator/pdgsectorgenerator.h"

namespace Kaim
{

class PathDataElementDispatcher
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	PathDataElementDispatcher(PdgSectorGenerator* generator, PdgPathDataElement* inputElem) :
	m_generator(generator), m_inputElement(inputElem)
	{
	}

	~PathDataElementDispatcher();	

public:
	KyResult Dispatch();
	bool HasChildren() const { return !m_children.empty(); } 
	const StlMap<PdgGuidCompound,PdgPathDataElement*>& Children() const { return m_children; } 


private: 
	void CellPosToPartitionCellDesc(const CellPos& pos, PartitioningCellDesc& desc);

private:
	PdgSectorGenerator* m_generator;
	PdgPathDataElement* m_inputElement;
	StlMap<PdgGuidCompound,PdgPathDataElement*> m_children;
};

} // namespace Kaim

#endif

