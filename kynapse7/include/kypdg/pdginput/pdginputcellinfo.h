/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgInputCellInfo_H
#define KyPdg_PdgInputCellInfo_H


#include <kypathdata/generic/memory.h>
#include <kypdg/generator/pdgsectorgenerator.h>


namespace Kaim
{

class PdgInputCell;
class PdgSectorGenerator;


class PdgInputCellInfo
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	PdgInputCellInfo() : m_sectorGenerator(KY_NULL), m_inputCell(KY_NULL) {} // default Ctor for Array<PdgInputCellInfo>
	PdgInputCellInfo(PdgSectorGenerator* sectorGenerator, PdgInputCell* inputCell ) :
		m_sectorGenerator(sectorGenerator), m_inputCell(inputCell)
	{}

public:
	PdgSectorGenerator* m_sectorGenerator;
	PdgInputCell* m_inputCell;
};

} // namespace Kaim

#endif
