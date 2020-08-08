/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobAggregate_H
#define KyPathData_BlobAggregate_H

#include <kypathdata/blob/endianness.h>
#include <kypathdata/blob/blobtyperegistry.h>
#include <kypathdata/blob/blobhandler.h>
#include <kypathdata/containers/array.h>

namespace Kaim
{

/* Data at the beginning a BlobAggregate file */
class BlobAggregateFileHeader
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	static const char* MagicString() { return "BlobAggFile"; }

public:
	BlobAggregateFileHeader();
	void InitForWrite(Endianness::Type endianness, KyUInt32 blobCount);
	KyResult CheckAndFixEndianness(bool& isEndiannessSwap);

public:
	char m_magicString[12]; // BlobAggFile0
	KyUInt32 m_endianness;
	KyUInt32 m_blobCount;
};


/* Data just before Blob in a BlobAggregate file */
class BlobAggregateBlobHeader
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BlobAggregateBlobHeader();
	void Clear();
	KyResult Init(KyUInt32 blobSize, const char* blobType, KyUInt32 blobVersion);
	void SwapEndianness();
	template<class T> bool Isa() const { return strcmp(m_blobType, T::GetBlobType()) == 0; }

public:
	char m_blobType[24];
	KyUInt32 m_blobVersion;
	KyUInt32 m_blobSize;
};


/* In a BlobAggregate object, contains a Blob and its description */
class BlobAggregateBlobDesc
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BlobAggregateBlobDesc();
	BlobAggregateBlobDesc(const BlobAggregateBlobHeader& blobHeader, void* blob);
	void Clear();
	const char* GetBlobType() const { return m_blobHeader.m_blobType;    }
	KyUInt32 GetBlobVersion() const { return m_blobHeader.m_blobVersion; }
	KyUInt32 GetBlobSize()    const { return m_blobHeader.m_blobSize;    }
	void* GetBlob()           const { return m_blob;                     }
	template<class T> bool Isa() const { return m_blobHeader.Isa<T>(); }
	template<class T> T* DynamicCastBlob() const { return Isa<T>() ? (T*)m_blob : KY_NULL; }

public:
	BlobAggregateBlobHeader m_blobHeader;
	void* m_blob;
};


/* BlobAggregate allows to save/load any number of blobs to/from a single file.
	File Memory layout is:
	BlobAggregateFileHeader BlobAggregateBlobHeader_0 Blob_0 BlobAggregateBlobHeader_1 Blob_1 ... */
class BlobAggregate
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	enum AcquireOwnership { ACQUIRE_OWNERSHIP, DO_NOT_ACQUIRE_OWNERSHIP };

public:
	BlobAggregate(const BlobTypeRegistry* blobTypeRegistry = KY_NULL) : m_blobTypeRegistry(blobTypeRegistry) {}

	~BlobAggregate() { Clear(); }

	void Clear();

	void SetBlobRegistry(const BlobTypeRegistry& blobTypeRegistry) { m_blobTypeRegistry = &blobTypeRegistry; }
	const BlobTypeRegistry& GetBlobRegistry() { return *m_blobTypeRegistry; }
	const BlobTypeRegistry& GetBlobRegistry() const { return *m_blobTypeRegistry; }

	const BlobTypeRegistry* GetBlobRegistryPtr() { return m_blobTypeRegistry; }
	const BlobTypeRegistry* GetBlobRegistryPtr() const { return m_blobTypeRegistry; }

	template<class T> KyResult AddBlob(BlobHandler<T>& blobHandler);

	template<class T> KyResult AddBlobAndGetOwership(BlobHandler<T>& blobHandler);

	template<class T> KyResult AddBlob(T* blob, KyUInt32 blobSize);

	KyResult AddBlob(void* blob, KyUInt32 blobSize, const char* blobType, KyUInt32 blobVersion);

	KyResult AddBlobAndGetOwership(void* blob, KyUInt32 blobSize, const char* blobType, KyUInt32 blobVersion);

	KyUInt32 ComputeByteSize() const;

	KyResult Save(const char* fileName, Endianness::Type endianness = Endianness::KY_BIG_ENDIAN);
	KyResult Save(KyFilePtr file, Endianness::Type endianness = Endianness::KY_BIG_ENDIAN);
	KyResult SaveToMemory(void* memory, Endianness::Type endianness = Endianness::KY_BIG_ENDIAN);

	KyResult Load(const char* fileName);
	KyResult Load(KyFilePtr file);
	KyResult LoadFromMemory(void* memory);

	KyUInt32 GetBlobDescCount() const { return m_blobDescs.GetCount(); }
	const BlobAggregateBlobDesc& GetBlobDesc(KyUInt32 index) const { return m_blobDescs[index]; }

private:
	const BlobTypeRegistry* m_blobTypeRegistry;
	Kaim::Array<BlobAggregateBlobDesc> m_blobDescs;
	Kaim::Array<void*> m_ownedBlobs;
};


template<class T>
KyResult BlobAggregate::AddBlob(BlobHandler<T>& blobHandler)
{
	if (m_blobTypeRegistry == KY_NULL)
		return KY_ERROR;
	return AddBlob((void*)blobHandler.Blob(), blobHandler.BlobSize(), T::GetBlobType(), T::GetBlobVersion());
}


template<class T>
KyResult BlobAggregate::AddBlobAndGetOwership(BlobHandler<T>& blobHandler)
{
	if (m_blobTypeRegistry == KY_NULL)
		return KY_ERROR;

	const KyResult resAdded = AddBlob((void*)blobHandler.Blob(), blobHandler.BlobSize(), T::GetBlobType(), T::GetBlobVersion());
	if (Result::Fail(resAdded))
		return resAdded;

	m_ownedBlobs.PushBack((void*)blobHandler.Blob());
	blobHandler.AbandonOwnership();
	return KY_SUCCESS;
}


template<class T>
KyResult BlobAggregate::AddBlob(T* blob, KyUInt32 blobSize)
{
	if (m_blobTypeRegistry == KY_NULL)
		return KY_ERROR;
	return AddBlob((void*)blob, blobSize, T::GetBlobType(), T::GetBlobVersion());
}


//-------------------------------------------------------------------------------------------------------------------
//
//                                    BlobAggregateLoader and some derivations (TODO put these classes in a separate file)
//
//-------------------------------------------------------------------------------------------------------------------

// Abstract BlobAggregate loader that typically allows derived classes to load
// - from a file or a memory buffer
// - to any destination (not necessarily a BlobAggregate)
class BlobAggregateLoader : public VirtualMemObject
{
public:
	BlobAggregateLoader(const BlobTypeRegistry* blobTypeRegistry = KY_NULL) : m_blobTypeRegistry(blobTypeRegistry) {}
	virtual ~BlobAggregateLoader() {}
	void SetBlobRegistry(const BlobTypeRegistry& blobTypeRegistry) { m_blobTypeRegistry = &blobTypeRegistry; }

protected:
	KyResult Load();

public:
	virtual void* ReadBytes(void* dest, KyUInt32 bytesToRead, KyUInt32& bytesRead) = 0;
	virtual KyResult IgnoreBytes(KyUInt32 byteCount) = 0;
	virtual void* AllocateBytes(KyUInt32 byteCount) = 0;
	virtual KyResult OnBlobLoaded(const BlobAggregateBlobHeader& blobHeader, void* blob) = 0;
	virtual bool DoLoadBlob(const BlobAggregateBlobHeader& /*blobHeader*/) { return true; } // load all blobs by default

public:
	const BlobTypeRegistry* m_blobTypeRegistry;
};


// Abstract BlobAggregate loader that reads from a file and allocates with KY_MALLOC. OnBlobLoaded is still virtual.
class BlobAggregateLoader_FromFile : public BlobAggregateLoader
{
public:
	BlobAggregateLoader_FromFile(const BlobTypeRegistry* blobTypeRegistry = KY_NULL) : BlobAggregateLoader(blobTypeRegistry), m_file(KY_NULL) {}
	KyResult LoadFromFile(const char* fileName);
	KyResult LoadFromFile(KyFilePtr file);

public:
	virtual void* ReadBytes(void* dest, KyUInt32 bytesToRead, KyUInt32& bytesRead) { bytesRead = Fread(dest, 1, bytesToRead, m_file); return dest; }
	virtual KyResult IgnoreBytes(KyUInt32 byteCount) { return Fseek(m_file, byteCount, SEEK_CUR) == 0 ? KY_SUCCESS : KY_ERROR; }
	virtual void* AllocateBytes(KyUInt32 byteCount) { return KY_MALLOC(char, byteCount); }

public:
	KyFilePtr m_file;
};


// Concrete BlobAggregate loader that reads from a file and allocates with KY_MALLOC. Each loaded blob is maintained by the blobAggregate passed in constructor.
class BlobAggregateLoader_FromFile_ToAggregate : public BlobAggregateLoader_FromFile
{
public:
	BlobAggregateLoader_FromFile_ToAggregate(BlobAggregate* blobAggregate) : m_blobAggregate(blobAggregate) { SetBlobRegistry(blobAggregate->GetBlobRegistry()); }

	virtual KyResult OnBlobLoaded(const BlobAggregateBlobHeader& blobHeader, void* blob)
	{
		return m_blobAggregate->AddBlobAndGetOwership(blob, blobHeader.m_blobSize, blobHeader.m_blobType, blobHeader.m_blobVersion);
	}

public:
	BlobAggregate* m_blobAggregate;
};


// Abstract BlobAggregate loader that reads from a memory buffer and uses the memory buffer as the blob. OnBlobLoaded is still virtual.
// Any object that stores blob pointers (without memcpying) by implementing OnBlobLoaded(const BlobAggregateBlobHeader& blobHeader, void* blob)
// has to make sure the memory buffer passed to LoadFromMemory(void* memory) has a longe life cycle than the object.
class BlobAggregateLoader_FromMemory : public BlobAggregateLoader
{
public:
	BlobAggregateLoader_FromMemory(const BlobTypeRegistry* blobTypeRegistry = KY_NULL) : BlobAggregateLoader(blobTypeRegistry), m_memory(KY_NULL) {}
	KyResult LoadFromMemory(void* memory);

public:
	virtual void* ReadBytes(void* /*dest*/, KyUInt32 bytesToRead, KyUInt32& bytesRead) { void* prevMemory = m_memory; m_memory += bytesToRead; bytesRead = bytesToRead; return prevMemory; }
	virtual KyResult IgnoreBytes(KyUInt32 byteCount) { m_memory += byteCount; return KY_SUCCESS; }
	virtual void* AllocateBytes(KyUInt32 /*byteCount*/) { return (void*)m_memory; }

public:
	char* m_memory;
};


// Concrete BlobAggregate loader that reads from a memory buffer and uses the memory buffer as the blob.
// The caller has to make sure that memory buffer passed to LoadFromMemory(void* memory) has a longer lifecycle than the blobAggregate passed to the constructor
class BlobAggregateLoader_FromMemory_ToAggregate : public BlobAggregateLoader_FromMemory
{
public:
	BlobAggregateLoader_FromMemory_ToAggregate(BlobAggregate* blobAggregate) : m_blobAggregate(blobAggregate) { SetBlobRegistry(blobAggregate->GetBlobRegistry()); }

	virtual KyResult OnBlobLoaded(const BlobAggregateBlobHeader& blobHeader, void* blob)
	{
		return m_blobAggregate->AddBlob(blob, blobHeader.m_blobSize, blobHeader.m_blobType, blobHeader.m_blobVersion);
	}

public:
	BlobAggregate* m_blobAggregate;
};




//-------------------------------------------------------------------------------------------------------------------
//
//                                    BlobAggregateBufferWriter and some derivations (TODO put these classes in a separate file)
//
//-------------------------------------------------------------------------------------------------------------------


// Abstract BlobAggregate saver that typically allows derived classes to save a file or a memory buffer
class BlobAggregateSaver : public VirtualMemObject
{
public:
	BlobAggregateSaver(const BlobAggregate* blobAggregate) :
		m_blobAggregate(blobAggregate), m_saveEndianness(Endianness::KY_BIG_ENDIAN) {}

	virtual ~BlobAggregateSaver() {}

	KyUInt32 ComputeByteSize() { return Write(COUNT_MODE); }

private:
	virtual KyUInt32 WriteBytes(void* /*src*/, KyUInt32 /*bytesToWrite*/) { return 0; }

protected:
	enum Mode { COUNT_MODE, WRITE_MODE };
	KyUInt32 Write(Mode mode);
	KyUInt32 WriteBlob(const BlobAggregateBlobDesc& blobDesc, Mode mode);

public:
	const BlobAggregate* m_blobAggregate;
	Endianness::Type m_saveEndianness;
};


class BlobAggregateSaver_ToFile : public BlobAggregateSaver
{
public:
	BlobAggregateSaver_ToFile(const BlobAggregate* blobAggregate) :
		BlobAggregateSaver(blobAggregate), m_file(KY_NULL) {}

	KyResult SaveToFile(KyFilePtr file, Endianness::Type saveEndianness = Endianness::KY_BIG_ENDIAN);

private:
	virtual KyUInt32 WriteBytes(void* src, KyUInt32 bytesToWrite) { return Fwrite(src, 1, bytesToWrite, m_file); }

private:
	KyFilePtr m_file;
};


class BlobAggregateSaver_ToMemory : public BlobAggregateSaver
{
public:
	BlobAggregateSaver_ToMemory(const BlobAggregate* blobAggregate) :
		BlobAggregateSaver(blobAggregate), m_memory(KY_NULL) {}

	KyResult SaveToMemory(void* memory, Endianness::Type saveEndianness = Endianness::KY_BIG_ENDIAN);

private:
	virtual KyUInt32 WriteBytes(void* src, KyUInt32 bytesToWrite) { memcpy(m_memory, src, bytesToWrite); m_memory += bytesToWrite; return bytesToWrite; }

private:
	char* m_memory; // destination memory buffer
};


} // namespace Kaim


#endif // KyPathData_BlobAggregate_H

