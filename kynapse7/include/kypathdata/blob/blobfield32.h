/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobField32_H
#define KyPathData_BlobField32_H


#include <kypathdata/generic/types.h>
#include <kypathdata/blob/blobarray.h>
#include <kypathdata/blob/baseblobbuilder.h>


namespace Kaim
{


class BlobField32
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	static const KyUInt32 Type_Unset     = 0;
	static const KyUInt32 Type_KyUInt32  = 1;
	static const KyUInt32 Type_KyInt32   = 2;
	static const KyUInt32 Type_KyFloat32 = 3;

public:
	BlobField32() : m_type(Type_Unset), m_value(0) {}

public:
	BlobArray<char> m_name;
	KyUInt32 m_type;
	KyUInt32 m_value;
};

inline void SwapEndianness(Endianness::Target e, BlobField32& self)
{
	SwapEndianness(e, self.m_name);
	SwapEndianness(e, self.m_type);
	SwapEndianness(e, self.m_value);
}


class BlobField32Mapping
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BlobField32Mapping() : m_type(0), m_name(KY_NULL), m_value(KY_NULL) {}

	BlobField32Mapping(KyUInt32 type, const char* name, void* value) :
		m_type(type), m_name(name), m_value((KyUInt32*)value)
	{}

public:
	KyUInt32 m_type;
	const char* m_name;
	KyUInt32* m_value;
};


class BlobField32Builder : public BaseBlobBuilder<BlobField32>
{
public:
	BlobField32Builder(BlobField32Mapping& mapping) : m_mapping(&mapping) {}

private:
	virtual void DoBuild()
	{
		BLOB_SET(m_blob->m_type, m_mapping->m_type);
		BLOB_STRING(m_blob->m_name, m_mapping->m_name);
		BLOB_SET(m_blob->m_value, *m_mapping->m_value);
	}

	BlobField32Mapping* m_mapping;
};


}


#endif

