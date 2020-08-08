#include "PointCacheManager.h"
#include "PointCache.h"
#include "AssetManagement/iassetmanager.h"

using namespace MaxSDK::AssetManagement;

///////////////////////////////////////////////////////////////////////////////////////////////////

CacheBuffer::CacheBuffer(const AssetUser& file)
{
	m_fileAsset	= file;
	m_refCount	= 0;
	m_buffer	= NULL;
	m_byteSize	= 0;
}

CacheBuffer::~CacheBuffer()
{
	DbgAssert(m_refCount == 0);
	delete[] m_buffer;
	m_buffer = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

PointCacheManager::PointCacheManager()
{
}

PointCacheManager::~PointCacheManager()
{
	DbgAssert(m_cacheBuffers.Count() == 0);
	int bufCount = m_cacheBuffers.Count();
	for (int i=(bufCount-1); i>=0; --i)
	{
		delete m_cacheBuffers[i];
		m_cacheBuffers.Delete(i, 1);
	}
}

void PointCacheManager::PreLoadPointCache(PointCache* pcMod)
{
	AssetUser file = pcMod->m_pblock->GetAssetUser(pc_cache_file, 0);
	int cacheIndex = 0;
	if (GetCacheIndex(file, cacheIndex))
	{
		// found pre-loaded, so use it
		m_cacheBuffers[cacheIndex]->m_refCount++;
		pcMod->m_preloadBuffer = m_cacheBuffers[cacheIndex]->m_buffer;
	} else {
		// didn't find anything, so make a new buffer and load cache
		CacheBuffer* cacheBuffer = new CacheBuffer(file);
		cacheBuffer->m_buffer = new Point3[pcMod->m_cacheNumPoints * pcMod->m_cacheNumSamples];

		if (cacheBuffer->m_buffer)
		{
			// buffer allocated, now load the whole damn thing
			int offset = pcMod->ComputeOffset(0);
			SetFilePointer(pcMod->m_hCacheFile, offset, NULL, FILE_BEGIN);
			DWORD numRead;
			DWORD numBytes = sizeof(Point3)*pcMod->m_cacheNumPoints*pcMod->m_cacheNumSamples;
			ReadFile(pcMod->m_hCacheFile, cacheBuffer->m_buffer, numBytes, &numRead, NULL);

			if (numRead != numBytes)
			{
				// something went wrong, just bail on pre-loading
				pcMod->FreeBuffers();
				return;
			}

			cacheBuffer->m_byteSize = (int)numBytes;
			cacheBuffer->m_refCount++;

			m_cacheBuffers.Append(1, &cacheBuffer);

			pcMod->m_preloadBuffer = cacheBuffer->m_buffer;
		}
	}
}

void PointCacheManager::UnLoadPointCache(PointCache* pcMod)
{
	if (pcMod->m_pblock)
	{
		AssetUser file = pcMod->m_pblock->GetAssetUser(pc_cache_file, 0);
		int cacheIndex = 0;
		if (GetCacheIndex(file, cacheIndex))
		{
			// decrement usage
			m_cacheBuffers[cacheIndex]->m_refCount--;

			// and delete buffer if not referenced anymore
			if (m_cacheBuffers[cacheIndex]->m_refCount == 0)
			{
				delete m_cacheBuffers[cacheIndex];
				m_cacheBuffers.Delete(cacheIndex, 1);
			}
		}

		pcMod->m_firstSample =
		pcMod->m_sampleA = pcMod->m_sampleB =
		pcMod->m_samplea = pcMod->m_sampleb =
		pcMod->m_preloadBuffer = NULL;
	}
}

bool PointCacheManager::GetCacheIndex(const AssetUser& file, int& index)
{
	int searchIndex = 0;
	while (searchIndex != m_cacheBuffers.Count())
	{
		if (m_cacheBuffers[searchIndex]->m_fileAsset.GetId() == file.GetId()) break;
		++searchIndex;
	}

	if (searchIndex == m_cacheBuffers.Count())
	{
		return false;
	} else {
		index = searchIndex;
		return true;
	}
}

int PointCacheManager::GetMemoryUsed()
{
	int memUsed = 0;

	for (int i=0; i<m_cacheBuffers.Count(); i++)
	{
		memUsed += m_cacheBuffers[i]->m_byteSize;
	}

	return memUsed;
}

PointCacheManager& GetPointCacheManager()
{
	static PointCacheManager pcMan;
	return pcMan;
}
