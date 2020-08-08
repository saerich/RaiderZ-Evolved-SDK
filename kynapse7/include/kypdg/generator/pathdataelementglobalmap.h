/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgPathDataElementMap_H
#define KyPdg_PdgPathDataElementMap_H


#include <kypdg/generator/pdgpathdataelement.h>
#include <kypathdata/containers/array.h>


namespace Kaim
{

class PdgPathDataElementMap_Implementation;


class PdgPathDataElementMap
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	PdgPathDataElementMap();
	~PdgPathDataElementMap();

	void Clear();

	KyResult Add(PdgPathDataElement& pathDataElement);

	PdgPathDataElement* Get(const PdgGuidCompound& pdgGuidCompound) const;

	const Array<PdgPathDataElement*>& GetArray() const;

private:
	PdgPathDataElementMap(const PdgPathDataElementMap&);
	PdgPathDataElementMap& operator=(const PdgPathDataElementMap&);
	
private:
	PdgPathDataElementMap_Implementation* m_impl;
};


}


#endif

