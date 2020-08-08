/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

#ifndef KYPOOLHANDLER_H
#define KYPOOLHANDLER_H

#include <kypathdata/containers/chunkbasedpool.h>

namespace Kaim
{
	template <typename T>
	class OwnPoolHandler
	{
		typedef ChunkBasedPool<T> PoolType;

	public:
		template <class U> struct Rebind
		{
			typedef OwnPoolHandler<U> Type;
		};

	public:
		explicit OwnPoolHandler(KyUInt16 size = 256) : m_pool(size) {}
		PoolType& GetPool() { return m_pool; }
		const PoolType& GetPool() const { return m_pool; }
		void Swap(OwnPoolHandler& other)
		{
			m_pool.Swap(other.m_pool);
		}

	protected:
		PoolType m_pool;
	};

	template <typename T>
	class SharedPoolHandler
	{
		typedef ChunkBasedPool<T> PoolType;

	public:
		template <class U> struct Rebind
		{
			typedef SharedPoolHandler<U> Type;
		};

	public:
		explicit SharedPoolHandler(PoolType* pool) : m_pool(pool) {}

		void SetPool(PoolType* pool) { m_pool = pool; }
		PoolType& GetPool() { return *m_pool; }
		const PoolType& GetPool() const { return *m_pool; }
		void Swap(SharedPoolHandler& other)
		{
			Kaim::Swap(m_pool, other.m_pool);
		}

	protected:
		PoolType* m_pool;
	};
}

#endif // KYPOOLHANDLER_H
