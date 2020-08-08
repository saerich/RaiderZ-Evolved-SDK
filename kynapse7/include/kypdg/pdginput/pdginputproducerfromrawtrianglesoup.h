/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgInputProducerFromRawTriangleSoup_H
#define KyPdg_PdgInputProducerFromRawTriangleSoup_H


#include <kypathdata/generic/types.h>
#include <kypdg/pdginput/pdginputproducer.h>
#include "kypathdata/containers/kystring.h"


namespace Kaim
{

class ClientInputConsumer;


class PdgInputProducerFromRawTriangleSoup : public PdgInputProducer
{
public:
	PdgInputProducerFromRawTriangleSoup(PdgSystem* sys = KY_NULL) : m_sys(sys) {}

	void SetFileName(const char* fileName) { m_fileName = fileName; }

	virtual KyResult Produce(ClientInputConsumer& inputConsumer);

public:
	KyString m_fileName;
	PdgSystem* m_sys;
};


}


#endif
