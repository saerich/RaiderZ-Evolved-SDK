/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgInputCellBlobBuilder_H
#define KyPdg_PdgInputCellBlobBuilder_H


#include <kypathdata/blob/baseblobbuilder.h>
#include <kypdg/pdginput/pdginputcell.h>


namespace Kaim
{


class DynamicPdgInputCell;


class PdgInputCellBlobBuilder : public BaseBlobBuilder<PdgInputCell>
{
public:
	PdgInputCellBlobBuilder(const DynamicPdgInputCell& dynamicPdgInputCell) { m_dyn = &dynamicPdgInputCell; }

private:
	virtual void DoBuild();

	const DynamicPdgInputCell* m_dyn;
};


}


#endif
