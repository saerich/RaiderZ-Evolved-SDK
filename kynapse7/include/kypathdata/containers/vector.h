/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KYVECTOR_H
#define KYVECTOR_H

#include "kypathdata/generic/typetraits.h"
#include "kypathdata/generic/algorithm.h"
#include "kypathdata/basesystem/allocatoradapter.h"

namespace Kaim
{
	// Do not construct or destroy objects
	template <class T, class Allocator = Kaim::allocator_adapter_nolog<T> >
	struct Vector
	{
		typedef T ValueType;
		typedef T* Iterator;
		typedef T* PointerType;
		typedef const T* ConstIterator;
		typedef ptr_size_type SizeType;

		/*	Default constructor.
			\post Size() == 0
			\post Capacity() == 0
			*/
		Vector() : m_buffer(0), m_size(0), m_capacity(0)
		{
		}

		/*	Destructor.
			\post Size() == 0
			\post Capacity() == 0
			*/
		~Vector()
		{
			Terminate();
		}

		/*	Push a default-constructed element at the end of the vector.
			\post Capacity() >= Size()
			*/
		KyResult PushBack()
		{
			KyResult error = Resize(Size() + 1);
			if (Result::Fail(error))
				return error;
	
			CallDefaultConstructor(m_buffer + Size() - 1);

			return Kaim::Result::Success;
		}

		/*	Push a element at the end of the vector.
			\post Capacity() >= Size()
			*/
		KyResult PushBack(const ValueType& value)
		{
			KyResult error = Resize(Size() + 1);
			if (Result::Fail(error))
				return error;
	
			m_buffer[Size() - 1] = value;

			return Kaim::Result::Success;
		}

		template <typename OtherIterator>
		KyResult PushBack(OtherIterator _begin, OtherIterator _end)
		{
			const SizeType count = _end - _begin;

			KyResult error = Resize(Size() + count);
			if (Result::Fail(error))
				return error;

			Kaim::Copy(_begin, _end, End() - count);

			return Kaim::Result::Success;
		}

		void Erase(Iterator it)
		{
			if (it >= Begin() && it < End())
			{
				Kaim::Copy(it + 1, End(), it);
				Resize(Size() - 1);
			}
		}

		void EraseRange(Iterator _First, Iterator _Last)
		{
			if (_First >= Begin() && _Last <= End() && _Last > _First)
			{
				Kaim::Copy(_Last, End(), _First);
				Resize(Size() - KyUInt32(_Last - _First));
			}
		}

		/* Retrieves the last element in the stack
			\pre Size() > 0 */
		ValueType& Back()
		{
			return m_buffer[Size() - 1];
		}

		/* Retrieves the last element in the stack
			\pre Size() > 0 */
		const ValueType& Back() const
		{
			return m_buffer[Size() - 1];
		}

		/* Calls destructor on each element in a range of elements in the vector container.*/
		void Destroy(PointerType _First, PointerType _Last)
		{
			Kaim::DestroyRange(_First, _Last, m_allocator);
		}

		/*	Deallocates the internal memory of the vector.
			\post Size() == 0
			\post Capacity() == 0
			*/
		void Terminate()
		{
			m_allocator.deallocate(m_buffer, m_capacity);

			m_buffer = 0;
			m_capacity = 0;
			m_size = 0;
		}

		/*	Removes all elements from the vector.
			\post Size() == 0
			*/
		void Clear()
		{
			Resize(0);
		}

		/*	Allocates enough internal memory to hold at least #count elements.
			\post Capacity() >= #count
			\post Capacity() >= Size()
		*/
		KyResult Reserve(SizeType count)
		{	
			// Grow only
			if (count > m_capacity)
			{
				ValueType* buffer = m_allocator.allocate(count);
				if (buffer == 0)
					return Result::OutOfMemory;

				if (m_buffer != 0)
				{
					Kaim::Copy(m_buffer, m_buffer + m_size, buffer);

					m_allocator.deallocate(m_buffer, m_capacity);
				}

				m_buffer = buffer;
				m_capacity = count;
			}

			return Kaim::Result::Success;
		}

		/*	Resizes the vector to contain sz elements.
			\post Size() = #count
			\post Capacity() >= Size()
		*/
		KyResult Resize(SizeType count)
		{
			if (Capacity() < count)
			{
				// Add 50%
				KyResult error = Reserve(count + count / 2);
				if (Result::Fail(error))
					return error;
			}
			m_size = count;

			return Kaim::Result::Success;
		}

		/*	Retrieves an iterator on first element. */
		Iterator Begin()
		{
			return m_buffer;
		}

		/*	Retrieves an iterator on first element. */
		ConstIterator Begin() const
		{
			return m_buffer;
		}

		/*	Retrieves an iterator after last element. */
		Iterator End()
		{
			return m_buffer + Size();
		}

		/*	Retrieves an iterator after last element. */
		ConstIterator End() const
		{
			return m_buffer + Size();
		}

		/*	Retrieves the size of the vector container. */
		SizeType Size() const
		{
			return m_size;
		}

		/* Retrieves the size of the allocated storage space for the elements of the vector container.*/
		SizeType Capacity() const
		{
			return m_capacity;
		}

		bool IsEmpty() const
		{
			return m_size == 0;
		}

	protected:
		ValueType* m_buffer;
		SizeType m_size;
		SizeType m_capacity;
		Allocator m_allocator;
	};

} // namespace Kaim

#endif // KYVECTOR_H
