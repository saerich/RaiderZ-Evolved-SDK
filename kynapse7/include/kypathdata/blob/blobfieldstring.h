/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobFieldString_H
#define KyPathData_BlobFieldString_H


#include <kypathdata/generic/types.h>
#include <kypathdata/blob/blobarray.h>
#include <kypathdata/blob/baseblobbuilder.h>
#include <kypathdata/containers/kystring.h>


namespace Kaim
{


class BlobFieldString
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BlobArray<char> m_name;
	BlobArray<char> m_value;
};

inline void SwapEndianness(Endianness::Target e, BlobFieldString& self)
{
	SwapEndianness(e, self.m_name);
	SwapEndianness(e, self.m_value);
}


class BlobFieldStringMapping
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BlobFieldStringMapping() : m_name(KY_NULL), m_basicString(KY_NULL) {}

	BlobFieldStringMapping(const char* name, KyString* basicString) :
		m_name(name), m_basicString(basicString)
	{}

public:
	const char* m_name;
	KyString* m_basicString;
};


class BlobFieldStringBuilder : public BaseBlobBuilder<BlobFieldString>
{
public:
	BlobFieldStringBuilder(BlobFieldStringMapping& mapping) : m_mapping(&mapping) {}

private:
	virtual void DoBuild()
	{
		BLOB_STRING(m_blob->m_name, m_mapping->m_name);
		BLOB_STRING(m_blob->m_value, m_mapping->m_basicString->CStr());
	}

	BlobFieldStringMapping* m_mapping;
};



}


#endif

