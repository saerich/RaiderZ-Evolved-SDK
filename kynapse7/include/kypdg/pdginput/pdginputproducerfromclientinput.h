/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_PdgInputProducerFromClientInput_H
#define KyPdg_PdgInputProducerFromClientInput_H

#include "kypathdata/containers/kystring.h"
#include "kypdg/pdginput/pdginputproducer.h"
#include "kypdg/pdginput/clientinputconsumer.h"


namespace Kaim
{

class ClientInputByString;

/* Used through Generator::AddSectorFromClientInput() */
class PdgInputProducerFromClientInput : public PdgInputProducer
{
public:
	virtual ~PdgInputProducerFromClientInput();

	ClientInput* AddClientInputFileName(const char* clientInputFileName);

	virtual KyResult Produce(ClientInputConsumer& inputConsumer);

public:
	struct Entry
	{
		Entry() : m_clientInput(KY_NULL) {}
		KyString m_fileName;
		ClientInput* m_clientInput;
	};
	Array<Entry> m_clientInputs;
};


}


#endif // KyPdg_PdgInputProducerFromClientInput_H

