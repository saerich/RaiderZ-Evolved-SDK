/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KYMEMORYLOG_H
#define KYMEMORYLOG_H

#include "kypathdata/generic/memory.h"
#include "kypathdata/basesystem/memorytree.h"
#include "kypathdata/basesystem/threadlocalstorage.h"

namespace Kaim
{
	typedef ThreadLocalStorageWrapper<MemoryLog*> TlsMemoryLog;

	class Archive;

	/* Maintains the log of memory allocations and de-allocations. For internal use. */
	struct MemoryLog
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		static MemoryLog& GetInstance()
		{
			TlsMemoryLog& tlsMemoryLog = GetTls();
			MemoryLog* memoryLog = tlsMemoryLog.Get();
			if (memoryLog != KY_NULL)
				return *memoryLog;

			allocator_adapter_nolog<MemoryLog> allocator;
			memoryLog = allocator.allocate(1); // MemoryLog allocation is not logged
			::new(memoryLog) MemoryLog();
			memoryLog->Initialize();
			tlsMemoryLog.Set(memoryLog);
			return *memoryLog;
		}

		static void TerminateInstance()
		{
			TlsMemoryLog& tlsMemoryLog = GetTls();
			MemoryLog* memoryLog = tlsMemoryLog.Get();
			if (memoryLog == KY_NULL)
				return;

			TerminateExplicitInstance(memoryLog);
			tlsMemoryLog.Set(KY_NULL);
		}

		static void TerminateExplicitInstance(MemoryLog* instance)
		{
			allocator_adapter_nolog<MemoryLog> allocator;
			instance->~MemoryLog();
			allocator.deallocate(instance, 1);
		}

		static TlsMemoryLog& GetTls()
		{
			static TlsMemoryLog tlsMemoryLog;
			return tlsMemoryLog;
		}

		KyResult Initialize();
		void Terminate();

		bool IsInitialized() const
		{
			return m_tree.IsInitialized();
		}

		KyResult SetAutoDump();
		KyResult SetAutoDump(const char* filename);

		KyResult Dump(const char* filename) const;

		void Write(Kaim::Archive& archive) const;

		MemoryTree& GetTree()
		{
			return m_tree;
		}

		const MemoryTree& GetTree() const
		{
			return m_tree;
		}

		/*! Retrieves the current callstack of \SDKName functions. */
		CallStack& GetCallStack()
		{
			return m_callStack;
		}

	protected:
		MemoryLog()
			: m_tree(), m_fileName(KY_NULL)
		{
		}

		~MemoryLog()
		{
			Terminate();
		}

		CallStack m_callStack;
		MemoryTree m_tree;
		char* m_fileName;
	};
}

#define KY_LOG_MEMORY(module) Kaim::ScopedCall _scopedcall((Kaim::GetBaseSystem().IsMemoryLogEnabled() ? &Kaim::MemoryLog::GetInstance().GetCallStack() : KY_NULL), Kaim::Call(module, __FILE__, __LINE__));


#endif // KYMEMORYLOG_H
