/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_PdgSectorConfig_H
#define KyPdg_PdgSectorConfig_H


#include <kypathdata/generic/memory.h>
#include <kypathdata/blob/blobfieldarray.h>
#include <kypathdata/containers/kystring.h>
#include "kypathdata/generic/kyguid.h"
#include "kypathdata/navmesh/navmeshtypes.h"


namespace Kaim
{


class PdgSectorConfig
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	PdgSectorConfig(
		KyGuid sectorGuid = KyGuid::s_defaultGuid,
		const char* sectorName = KY_NULL)
	{
		Init(sectorGuid, sectorName);
	}

	void SetInputCellBox(const CellBox& inputCellBox);
	bool IsInputCellBoxSet();


	void Clear();

	/* if sectorGuid==KyGuid::s_defaultGuid it is generated automatically (except on Linux) or loaded from sectorName.PdgInput
		if sectorName==KY_NULL it is generated automatically from sectorGuid or loaded from sectorName.PdgInput.*/
	void Init(
		KyGuid sectorGuid = KyGuid::s_defaultGuid,
		const char* sectorName = KY_NULL);

	void AddMapping(BlobFieldsMapping& mapping) const;

	void AddMapping(BlobFieldsMapping& mapping);

	void ReadBlob(const BlobFieldArray& blobFieldArray);

	void BuildBlob(BlobHandler<BlobFieldArray>& blobFieldArrayHandler) const;

public:
	KyGuid m_sectorGuid;
	KyString m_sectorGuidString;
	KyString m_sectorName;
	// Can't use CellBox directly because of BlobFieldArray mapping
	KyInt32 m_inputCellBoxMinX;
	KyInt32 m_inputCellBoxMinY;
	KyInt32 m_inputCellBoxMaxX;
	KyInt32 m_inputCellBoxMaxY;
};


}


#endif

