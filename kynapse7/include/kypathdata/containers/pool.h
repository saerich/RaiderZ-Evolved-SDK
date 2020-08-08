/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef KYPOOL_H
#define KYPOOL_H

#include "kypathdata/containers/staticstack.h"
#include "kypathdata/containers/vector.h"

namespace Kaim
{
	/* Pool of objects.
		Allocates by chunk.
	 */
	template <class T, KyUInt32 chunk_size, template<typename> class Allocator >
	struct Pool
	{
		typedef T value_type;

		// Chunk
		struct Chunk : StaticStack<value_type, chunk_size>
		{
			typedef StaticStack<value_type, chunk_size> base_type;

			Chunk() : base_type(), m_allocated(0)
			{
			}

			/* Allocates and default-constructs a new element of type T. */
			value_type* Allocate()
			{
				if (!base_type::IsFull())
				{
					// Default-constructs a new element
					base_type::PushBack();
					++m_allocated;

					return &base_type::Back();
				}

				return KY_NULL;
			}

			void DeAllocate(value_type* /*ptr*/)
			{
				if (m_allocated > 0)
					--m_allocated;

				if (m_allocated == 0)
				{
					Clear();
				}
			}

			void Clear()
			{
				base_type::Clear();

				m_allocated = 0;
			}

			bool Contains(const value_type* ptr) const
			{
				return base_type::Contains((typename base_type::ConstIterator)ptr);
			}

		protected:
			KyUInt32 m_allocated;
		}; // Chunk

		typedef Allocator<Chunk> allocator_type;

		/*	Default constructor.
			\post Capacity() == 0
			*/
		Pool() : chunks()
		{
		}

		/*	Destructor.
			\post Capacity() == 0
			*/
		~Pool()
		{
			Terminate();
		}

		/*	Allocates enough internal memory to hold at least #size elements.
			\post Capacity() >= #size
		*/
		KyResult Reserve(KyUInt32 size)
		{
			return chunks.Reserve(size / chunk_size);
		}

		/*	Deallocates the internal memory.
			\post Capacity() = 0
			*/
		void Terminate()
		{
			return chunks.Terminate();
		}

		/* Retrieves the size of the allocated storage space.*/
		ptr_size_type Capacity() const
		{
			return chunks.Capacity() * chunk_size;
		}

		/* Allocates and default-constructs a new element of type T. */
		value_type* Allocate()
		{
			// Add a chunk  if empty or if the last one is full
			if (chunks.Size() == 0 || chunks.Back().IsFull())
			{
				if (Kaim::Result::Fail(chunks.PushBack()))
					return KY_NULL;
			}

			// Get T* from chunk
			return chunks.Back().Allocate();
		}

		void DeAllocate(value_type* ptr)
		{
			for (typename Kaim::Vector<Chunk, Allocator<Chunk> >::Iterator it = chunks.Begin(); it != chunks.End(); ++it)
			{
				if (it->Contains(ptr))
				{
					it->DeAllocate(ptr);
					if (it->IsEmpty())
					{
						chunks.Erase(it);
					}

					break;
				}
			}
		}

	protected:
		Kaim::Vector<Chunk, Allocator<Chunk> > chunks;
	};
}

#endif // KYPOOL_H
