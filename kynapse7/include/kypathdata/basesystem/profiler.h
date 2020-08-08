/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_PROFILER_H
#define KY_PROFILER_H

#include <kypathdata/containers/staticstack.h>
#include <kypathdata/containers/pool.h>
#include <kypathdata/basesystem/allocatoradapter.h>
#include <kypathdata/basesystem/threadlocalstorage.h>

#define KAIM_PROFILER_RECORD_POOL Kaim::Pool<Kaim::Profiler::Record, 32, Kaim::allocator_adapter_nolog >

namespace Kaim
{

typedef ThreadLocalStorageWrapper<Profiler*> TlsProfiler;

/*	The Profiler class collects and records performance profiling information for
	code blocks and functions identified using the #KY_PROFILE macro. 
	\pp This class is intended for internal use. */
class Profiler
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*	Represents a single node in the tree of profiling information maintained
		by the Profiler. */
	struct Record
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		typedef KAIM_PROFILER_RECORD_POOL pool_type;

		/*	\label_constructor For internal use. */
		Record();
		
		/*	Instructs the Profiler to begin collecting profiling information for each 
			of the child nodes of this Record. 
			\param expandAll			Specifies whether or not to propagate the expansion to 
										each of the child nodes of this Record and on to their own children. */
		void Expand(bool expandAll = false);
		
		/*	Instructs the Profiler to stop collecting profiling information for each of 
			the child nodes of this Record. Note that this method is always recursive:
			all child nodes are entirely removed from the list of children of this Record, and will be
			unavailable until the next frame. */
		void Collapse();

		/*	For internal use. */
		void Init();
		
		/*	For internal use. */
		KyFloat32 RemoveTimerCost(KyFloat32 timerCost);

		/*	For internal use. */
		Record* AddChild(const char* name);

		/*	For internal use. */
		void PushBack(Record& record);

		/*	For internal use. */
		void Clear();

		/*	For internal use. */
		void SetPool(pool_type& pool_) 
		{
			pool = &pool_;
		}

	public:
		// pool
		pool_type* pool; /*< For internal use. */

		// tree
		Record* parent; /*< For internal use. */
		Record* first; /*< For internal use. */
		Record* last; /*< For internal use. */
		Record* next; /*< For internal use. */

		KyUInt32 size;  /*< For internal use. */ // to help the debugger

		// data
		const char* m_blockName; /*< A descriptive name assigned to this Record in the call to the #KY_PROFILE macro. For code blocks pre-designated with \SDKName, this value is typically the name of the current function. */
		KyFloat32 m_sum; /*< The total time spent in this block of code during the current frame. */
		KyFloat32 m_max; /*< The maximum length of time spent in any single execution of this block of code during the current frame. */
		KyFloat32 m_count; /*< The number of times this code block has been executed during the current frame. */
		bool m_inspectChildren; /*< Indicates whether or not the current Record is expanded. */
	};

	// Profiler

public:	
	static const KyUInt32 record_pool_size = 1024;

public:	
	typedef Record::pool_type record_pool_type;

protected:
	static const KyUInt32 s_stackMaxDepth = 512; /*< The maximum depth of the profiling tree. */
	
protected:
	typedef StaticStack<Record*, Profiler::s_stackMaxDepth> Stack;

public:
	static Profiler& GetInstance()
	{
		TlsProfiler& tlsProfiler = GetTls();
		Profiler* profiler = tlsProfiler.Get();
		if (profiler != KY_NULL)
			return *profiler;

		profiler = KY_NEW_CLASS(Profiler);
		tlsProfiler.Set(profiler);
		return *profiler;
	}

	static void TerminateInstance()
	{
		TlsProfiler& tlsProfiler = GetTls();
		Profiler* profiler = tlsProfiler.Get();
		if (profiler == KY_NULL)
			return;

		TerminateExplicitInstance(profiler);
		tlsProfiler.Set(KY_NULL);
	}

	static void TerminateExplicitInstance(Profiler* instance)
	{
		KY_DELETE_CLASS(instance);
	}

	static TlsProfiler& GetTls()
	{
		static TlsProfiler tlsProfiler;
		return tlsProfiler;
	}

	/*	Indicates that the specified code block has begun its execution. For internal use by the #KY_PROFILE macro.
		\return #KY_TRUE if the profiling was successfully started, or #KY_FALSE otherwise. */
	bool ProfileBegin(const char* blockName);
	
	/*	Indicates that the code block currently being monitored has finished. For internal use by the #KY_PROFILE macro. */
	void ProfileEnd(KyFloat32 t);

	/* For internal use. */
	void StartFrame();
	
	/* For internal use. */
	void EndFrame();

	/* Indicates whether or not the Profiler is ready to begin recording statistics. */
	bool IsReady() const { return m_ready; }

	/* For internal use. */
	bool IsLastInspectChildrenSet() const { return m_stack.Back()->m_inspectChildren; }

protected:
	/*	\label_constructor For internal use. */
	Profiler();

	/*	\label_destructor For internal use. */
	~Profiler();

	Stack m_stack; /*< The current execution stack of code blocks identified for profiling. Do not modify. */
	KyFloat32 m_timerConsumption; /*< The CPU consumption for two calls to Engine::GetElapsedTimeInFrame(). Do not modify. */
	bool m_ready; /*< Indicates whether or not the Profiler has been successfully initialized and is ready for use. Do not modify. */

	record_pool_type pool; /*< The pool of Record objects that make up the profiling tree. Do not modify. */
	Record root; /*< The root node of the profiling tree. */
};

// explicit instantiation to help the debugger
template struct KAIM_PROFILER_RECORD_POOL;

} // namespace Kaim

#endif // KY_PROFILER_H
