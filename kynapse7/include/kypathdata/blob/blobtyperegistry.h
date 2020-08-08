/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobTypeRegistry_H
#define KyPathData_BlobTypeRegistry_H


#include <kypathdata/generic/types.h>
#include <kypathdata/containers/array.h>
#include <kypathdata/blob/endianness.h>


namespace Kaim
{


/*
*/
class BaseBlobTypeDescriptor : public VirtualMemObject
{
public:
	virtual const char* GetBlobType() = 0;
	virtual KyUInt32 GetBlobVersion() = 0;
	virtual void DoSwapEndianness(Endianness::Target e, void* blob) = 0;
	virtual BaseBlobTypeDescriptor* Clone() = 0; 
};


/*
*/
template<class T>
class BlobTypeDescriptor : public BaseBlobTypeDescriptor
{
public:
	virtual const char* GetBlobType() { return T::GetBlobType();    }
	virtual KyUInt32 GetBlobVersion() { return T::GetBlobVersion(); }
	virtual void DoSwapEndianness(Endianness::Target e, void* blob) { SwapEndianness(e, *((T*)blob)); }
	virtual BaseBlobTypeDescriptor* Clone() { return KY_NEW_CLASS(BlobTypeDescriptor<T>); }
};


/*
*/
class BlobTypeRegistry : public VirtualMemObject
{
public:
	void Clear()
	{
		for (KyUInt32 i = 0; i < m_descriptors.GetCount(); ++i)
			KY_DELETE_CLASS(m_descriptors[i]);
		m_descriptors.Clear();
	}

	virtual ~BlobTypeRegistry()
	{
		Clear();
	}

	template<class T>
	void AddBlobType()
	{
		BlobTypeDescriptor<T>* descriptor = KY_NEW_CLASS(BlobTypeDescriptor<T>);
		m_descriptors.PushBack(descriptor);
	}

	void AddBlobTypeRegistry(const BlobTypeRegistry& other)
	{
		for (KyUInt32 i = 0; i < other.m_descriptors.GetCount(); ++i)
			m_descriptors.PushBack(other.m_descriptors[i]->Clone());
	}

	BaseBlobTypeDescriptor* GetBlobTypeDescriptor(const char* blobType) const
	{
		for (KyUInt32 i = 0; i < m_descriptors.GetCount(); ++i)
		{
			BaseBlobTypeDescriptor* descriptor = m_descriptors[i];
			if (strcmp(descriptor->GetBlobType(), blobType) == 0)
				return descriptor;
		}

		return KY_NULL;
	}

public:
	Array<BaseBlobTypeDescriptor*> m_descriptors;
};


}

#endif

