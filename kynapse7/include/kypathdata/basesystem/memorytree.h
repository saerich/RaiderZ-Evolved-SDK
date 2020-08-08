/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef KYMEMORYTREE_H
#define KYMEMORYTREE_H

#include "kypathdata/containers/pool.h"

#include "kypathdata/basesystem/archive.h"
#include "kypathdata/basesystem/allocatoradapter.h"

#define KAIM_MEMORYTREE_NODE_POOL Kaim::Pool<Kaim::MemoryTree::Node, 32, Kaim::allocator_adapter_nolog >

namespace Kaim
{
	/* For internal use by the memory logging system. */
	struct MemoryTree
	{
		/* For internal use by the memory logging system. */
		struct Record : Call
		{
			ptr_size_type size;
			ptr_size_type maxSize;
			KyUInt32 allocCount;
			KyUInt32 deallocCount;

			KyFloat32 timestamp;

			Record()
				:Call(), size(0), maxSize(0), allocCount(0), deallocCount(0), timestamp(0.f)
			{
			}

			Record(const Call& call_) 
				: Call(call_), size(0), maxSize(0), allocCount(0), deallocCount(0), timestamp(0.f)
			{
			}

			bool operator==(const Call& other)
			{
				return static_cast<Call>(*this) == other;
			}

			void Reset()
			{
				size = 0;
				maxSize = 0;
				allocCount = 0;
				deallocCount = 0;

				timestamp = 0.f;
			}

			void Allocate(ptr_size_type size_, KyFloat32 timestamp_)
			{
				size += size_;
				maxSize = size > maxSize ? size : maxSize;
				++allocCount;
				timestamp = timestamp_;
			}

			void DeAllocate(ptr_size_type size_, KyFloat32 timestamp_)
			{
				size -= size_;
				++deallocCount;
				timestamp = timestamp_;
			}

			void Write(Kaim::Archive& archive) const
			{
				// Call
				archive << MakeNamedValue("name", name);
				archive << MakeNamedValue("file", file);
				archive << MakeNamedValue("line", line);
				//Record
				archive << MakeNamedValue("size", size);
				archive << MakeNamedValue("maxSize", maxSize);
				archive << MakeNamedValue("allocCount", allocCount);
				archive << MakeNamedValue("deallocCount", deallocCount);

				archive << MakeNamedValue("timestamp", timestamp);
			}
		};

		/* For internal use by the memory logging system. */
		struct Node
		{
			typedef KAIM_MEMORYTREE_NODE_POOL pool_type;

		public:	
			Node()
				: pool(KY_NULL), parent(KY_NULL), first(KY_NULL), last(KY_NULL), next(KY_NULL), size(0)
			{
			}

			void Initialize(pool_type& pool_, const Call& call_) 
			{
				pool = &pool_;
				record = call_;
			}

			void Terminate()
			{
				pool = KY_NULL;
			}

			bool IsInitialized() const
			{
				return pool != KY_NULL;
			}

			Node* AddNode(const Kaim::CallStack& callstack, const Kaim::Call& context, KyUInt32 level = 0)
			{
				if (level < callstack.Size())
				{
					// Get/Create node at this level
					Node* node = AddNode(callstack[level]);

					if (node == KY_NULL)
						return KY_NULL;
					
					// Insert in child node
					return node->AddNode(callstack, context, level+1);
				}
				else
				{
					// Get/Create leaf
					return AddNode(context);
				}
			}

			Node* AddNode(const Kaim::Call& context)
			{
				// Search an existing child node
				Node* node = Find(context);

				// Create a new child node if necessary
				if (node == KY_NULL)
				{
					// Create a new child node
					node = pool->Allocate();
					if (node == KY_NULL)
						return KY_NULL;

					node->Initialize(*pool, context);

					PushBack(*node);
				}

				return node;
			}


			void PushBack(Node& node)
			{
				node.parent = this;

				if (first == KY_NULL)
					first = &node;

				if (last != KY_NULL)
					last->next = &node;

				last = &node;

				++size;
			}

			void Write(Kaim::Archive& archive) const
			{
				archive << MakeNamedValue("record", record);

				for (Node* node = first; node != KY_NULL; node = node->next)
				{
					archive << MakeNamedValue("child", *node);
				}
			}

			void Clear()
			{
				parent = KY_NULL;
				first = KY_NULL;
				last = KY_NULL;
				next = KY_NULL;

				size = 0;
			}

			void Allocate(ptr_size_type size_, KyFloat32 timestamp_)
			{
				record.Allocate(size_, timestamp_);

				if (parent != KY_NULL)
					parent->Allocate(size_, timestamp_);
			}

			void DeAllocate(ptr_size_type size_, KyFloat32 timestamp_)
			{
				record.DeAllocate(size_, timestamp_);

				if (parent != KY_NULL)
					parent->DeAllocate(size_, timestamp_);
			}

		protected:
			Node* Find(const Call& call_) const
			{
				for (Node* node = first; node != KY_NULL; node = node->next)
				{
					if (node->record == call_)
						return node;
				}

				return KY_NULL;
			}

		protected:
			// pool
			pool_type* pool;

			// tree
			Node* parent;
			Node* first;
			Node* last;
			Node* next;

			KyUInt32 size; // to help the debugger

			// data
			Record record;

		}; // Node


	// MemoryTree
	public:	
		static const KyUInt32 node_pool_size = 1024;

	public:	
		typedef Node::pool_type node_pool_type;

	public:	
		 MemoryTree() 
			: pool()
		{
		}

		KyResult Initialize()
		{
			Terminate();

			pool.Reserve(node_pool_size);
			root.Initialize(pool, Call("root", 0, 0));

			return Result::Success;
		}

		void Terminate()
		{
			if (IsInitialized())
			{
				root.Terminate();
				pool.Terminate();
			}
		}

		bool IsInitialized() const
		{
			return pool.Capacity() > 0;
		}

		Node* Allocate(ptr_size_type size_, const Kaim::CallStack& callstack, const Kaim::Call& context, KyFloat32 timestamp_)
		{
			// Create or Get node
			Node* node = root.AddNode(callstack, context);

			if (node  != KY_NULL)
			{
				// Update node
				node->Allocate(size_, timestamp_);
			}

			return node;
		}

		void DeAllocate(Node* node, ptr_size_type size_, KyFloat32 timestamp_)
		{
			if (node != KY_NULL)
			{
				// Upate node
				node->DeAllocate(size_, timestamp_);
			}
		}

		void Write(Kaim::Archive& archive) const
		{
			archive << MakeNamedValue("root", root);
		}

	protected:
		node_pool_type pool;
		Node root;
	};
}

// explicit instantiation to help the debugger
template struct KAIM_MEMORYTREE_NODE_POOL;

#endif // KYMEMORYTREE_H
