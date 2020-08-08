#ifndef __POINTCACHEMANAGER_H__
#define __POINTCACHEMANAGER_H__

#include "Max.h"

class PointCache;

// Holds an entire cache file in memory and tracks how many things are referencing it
class CacheBuffer
{
public:
	CacheBuffer(const TSTR& filename);
	~CacheBuffer();

	int			m_refCount;
	Point3*		m_buffer;
	TSTR		m_filename;
	int			m_byteSize;
};

// Handles loading caches into CacheBuffers... directly twiddles PointCache internals (suck)
class PointCacheManager
{
public:
	PointCacheManager();
	~PointCacheManager();

	void	PreLoadPointCache(PointCache* pcMod);
	void	UnLoadPointCache(PointCache* pcMod);

	int		GetMemoryUsed();

private:
	bool GetCacheIndex(const TSTR& filename, int& index);

	Tab<CacheBuffer*>	m_cacheBuffers;
};

extern PointCacheManager& GetPointCacheManager();

#endif // __POINTCACHEMANAGER_H__
