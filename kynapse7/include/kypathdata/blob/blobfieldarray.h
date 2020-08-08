/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobFieldArray_H
#define KyPathData_BlobFieldArray_H


#include <kypathdata/generic/types.h>
#include <kypathdata/blob/blobarray.h>
#include <kypathdata/blob/baseblobbuilder.h>
#include <kypathdata/blob/blobfield32.h>
#include <kypathdata/blob/blobfieldstring.h>
#include <kypathdata/containers/array.h>
#include <kypathdata/containers/kystring.h>


namespace Kaim
{


/* Blob that is a name and any numer of blobfields */
class BlobFieldArray
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	static const char* GetBlobType() { return "BlobFieldArray"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	BlobArray<char> m_name;
	BlobArray<BlobField32> m_field32s;
	BlobArray<BlobFieldString> m_fieldStrings;
};

inline void SwapEndianness(Endianness::Target e, BlobFieldArray& self)
{
	SwapEndianness(e, self.m_name);
	SwapEndianness(e, self.m_field32s);
	SwapEndianness(e, self.m_fieldStrings);
}


class BlobFieldsMapping
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	void AddString(const char* name, KyString& value) { m_fieldStringMaps.PushBack(BlobFieldStringMapping(name, &value)); }

	void AddUInt32(const char* name, KyUInt32& value)   { m_field32Maps.PushBack(BlobField32Mapping(BlobField32::Type_KyUInt32 , name, (KyUInt32*)&value)); }
	void AddInt32(const char* name, KyInt32& value)     { m_field32Maps.PushBack(BlobField32Mapping(BlobField32::Type_KyInt32  , name, (KyUInt32*)&value)); }
	void AddFloat32(const char* name, KyFloat32& value) { m_field32Maps.PushBack(BlobField32Mapping(BlobField32::Type_KyFloat32, name, (KyUInt32*)&value)); }

	KyResult ReadFromBlobFieldArray(const BlobFieldArray& blobFieldArray);

public:
	Array<BlobField32Mapping> m_field32Maps;
	Array<BlobFieldStringMapping> m_fieldStringMaps;
};


class BlobFieldArrayBuilder : public BaseBlobBuilder<BlobFieldArray>
{
public:
	BlobFieldArrayBuilder(const char* name, BlobFieldsMapping& mapping) : m_name(name), m_mapping(&mapping) {}

private:
	virtual void DoBuild();

public:
	const char* m_name;
	BlobFieldsMapping* m_mapping;
};


}


#endif

