/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_GuidCompoundBlobBuilder_H
#define KyPdg_GuidCompoundBlobBuilder_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/blob/baseblobbuilder.h>
#include <kypathdata/generic/guidcompound.h>


namespace Kaim
{


class GuidCompoundBlobBuilder : public BaseBlobBuilder<GuidCompound>
{
public:
	GuidCompoundBlobBuilder(const KyGuid* guids, KyUInt32 guidsCount, KyUInt32 timeStamp) :
		m_guids(guids), m_guidsCount(guidsCount), m_timeStamp(timeStamp) {}

private:
	virtual void DoBuild();

	const KyGuid* m_guids;
	KyUInt32 m_guidsCount;
	KyUInt32 m_timeStamp;
};


}


#endif
