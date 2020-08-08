/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_core */

// primary contact: ? - secondary contact: NOBODY
#ifndef KY_CIRCULARARRAY_H
#define KY_CIRCULARARRAY_H

#include "kypathdata/generic/types.h"
#include "kypathdata/generic/algorithm.h"
#include "kypathdata/basesystem/allocatoradapter.h"

namespace Kaim
{
namespace details
{
	template <typename Buff>
	struct NoConstTraits
	{
		typedef typename Buff::PointerType PointerType;
		typedef typename Buff::ReferenceType ReferenceType;
	};

	template <typename Buff>
	struct ConstTraits
	{
		typedef typename Buff::ConstPointerType PointerType;
		typedef typename Buff::ConstReferenceType ReferenceType;
	};

	/*!	This class represents an iterator that can be used to iterate through the
		elements in a CircularArray.
		\tparam Buff		The type of buffer to be managed by this iterator. Use CircularArray.
		\tparam Traits		The const qualities of the pointers and references returned by this iterator. Use either
							\c ConstTraits for const pointers and references, or \c NoConstTraits for non-const pointers
							and references. */
	template <typename Buff, typename Traits >
	struct IteratorImpl
	{
	public:		
		typedef typename Traits::PointerType PointerType;
		typedef typename Traits::ReferenceType ReferenceType;
		typedef KyInt32 DifferenceType;
		
	public:	
		const Buff* m_buff; /*!< The CircularArray managed by this iterator. */
		PointerType m_it; /*!< The type of data stored in the CircularArray. */

	public:
		/*!	For internal use by CircularArray. */
		IteratorImpl() 
			: m_buff(KY_NULL), m_it(KY_NULL)
		{
		}

		/*!	For internal use by CircularArray. */
		IteratorImpl(const IteratorImpl& other) 
			:m_buff(other.m_buff), m_it(other.m_it)
		{
		}

		/*!	For internal use by CircularArray. */
		IteratorImpl(const Buff* buff, const PointerType p)
			:m_buff(buff), m_it(p)
		{
		}

		/*!	Copies the state of the specified IteratorImpl to this. */
		IteratorImpl& operator = (const IteratorImpl& it) 
		{
			if (this == &it)
				return *this;

			m_buff = it.m_buff;
			m_it = it.m_it;

			return *this;
		}

		/*!	Returns the data stored in the CircularArray element pointed to
			by this IteratorImpl. */
		ReferenceType operator * ()  const
		{
			return *m_it;
		}

		/*!	Returns the data stored in the CircularArray element pointed to
			by this IteratorImpl. */
		PointerType operator -> ()  const
		{ 
			return &(operator*()); 
		}

		/*!	Increments the IteratorImpl to point to the next element in the array. */
		IteratorImpl& operator ++ () 
		{
			m_buff->Increment(m_it);

			if (m_it == m_buff->m_last)
				m_it = 0;

			return *this;
		}

		/*!	Increments the IteratorImpl to point to the next element in the array. */
		IteratorImpl operator ++ (int) 
		{
			IteratorImpl<Buff, Traits> tmp = *this;
			++*this;
			return tmp;
		}

		/*!	Decrements the IteratorImpl to point to the previous element in the array. */
		IteratorImpl& operator -- () 
		{
			if (m_it == 0)
				m_it = m_buff->m_last;

			m_buff->Decrement(m_it);

			return *this;
		}

		/*!	Decrements the IteratorImpl to point to the previous element in the array. */
		IteratorImpl operator -- (int) 
		{
			IteratorImpl<Buff, Traits> tmp = *this;
			--*this;
			return tmp;
		}

		/*!	Sets the IteratorImpl to point to the element at the index \c n greater than
			the current index. */
		IteratorImpl& operator += (DifferenceType n) 
		{
			if (n > 0) 
			{
				m_it = m_buff->Add(m_it, n);

				if (m_it == m_buff->m_last)
					m_it = 0;
			} 
			else
			{
				if (n < 0) 
				{
					*this -= -n;
				}
			}
			return *this;
		}

		/*!	Sets the IteratorImpl to point to the element at the index \c n greater than
			the current index. */
		IteratorImpl operator + (DifferenceType n) const 
		{ 
			return IteratorImpl<Buff, Traits>(*this) += n; 
		}

		/*!	Sets the IteratorImpl to point to the element at the index \c n less than
			the current index. */
		IteratorImpl& operator -= (DifferenceType n) 
		{
			if (n > 0) 
			{
				m_it = m_buff->Sub(m_it == 0 ? m_buff->m_last : m_it, n);
			} 
			else 
			{
				if (n < 0) 
				{
					*this += -n;
				}
			}
			return *this;
		}

		/*!	Sets the IteratorImpl to point to the element at the index \c n less than
			the current index. */
		IteratorImpl operator - (DifferenceType n) const 
		{ 
			return IteratorImpl<Buff, Traits>(*this) -= n; 
		}

		/*!	Retrieves the element at the at the index \c n greater than
			the current index. */
		ReferenceType operator [] (DifferenceType n) const 
		{ 
			return *(*this + n); 
		}

		/*!	Tests whether or not the two specified Iterators point to the same data. */
		bool operator == (const IteratorImpl<Buff, Traits>& it) const 
		{
			return m_it == it.m_it;
		}

		/*!	Tests whether or not the two specified Iterators point to different data. */
		bool operator != (const IteratorImpl<Buff, Traits>& it) const 
		{
			return m_it != it.m_it;
		}
	};

} // namespace details

/*!	This class represents an array of a fixed size. When the array is full and a new element
	is added, the oldest element is removed. This class is used internally by the remote
	debugging framework, but you are also free to use it in your own code if you wish.
	\tparam T			The type of the elements in the array.
	\ingroup kyruntime_core */
template <typename T, class Allocator = Kaim::allocator_adapter_nolog<T> >
class CircularArray 
{
public:
	typedef T ValueType;
	typedef T* PointerType; /*!< A type that represents a pointer to an element in this array. */
	typedef const T* ConstPointerType; /*!< A type that represents a const pointer to an element in this array. */
	typedef T& ReferenceType; /*!< A type that represents a reference to an element in this array. */
	typedef const T& ConstReferenceType; /*!< A type that represents a const reference to an element in this array. */
	typedef KyUInt32 SizeType; /*!< A type that represents the size of each element in this array. */
	typedef KyUInt32 CapacityType; /*!< A type used to store the capacity of this array. */
	typedef details::IteratorImpl< CircularArray<T>, details::NoConstTraits< CircularArray<T> > > Iterator; /*!< A type of iterator used to iterate through the elements in this array. */
	typedef details::IteratorImpl< CircularArray<T>, details::ConstTraits< CircularArray<T> > > ConstIterator; /*!< A const type of iterator used to iterate through the elements in this array. */

public:
	template <typename Buff, typename Traits> friend struct details::IteratorImpl;

public:
	/*!	Default constructor. */
	CircularArray() : m_buffer(0), m_size(0), m_capacity(0), m_first(KY_NULL), m_last(KY_NULL)
	{
	}

	/*!	Adds a new element to the end of the CircularArray. If the CircularArray is already at its maximum
		capacity, the oldest element is removed. */
	void PushBack(ConstReferenceType t)
	{
		if (m_capacity == 0)
		{
			Reserve(4);
		}
		if (IsFull())
		{
			*m_last = t;
			Increment(m_last);
			m_first = m_last;
		}
		else
		{
			*m_last = t;
			Increment(m_last);
			++m_size;
		}
	}

	ConstReferenceType operator[](SizeType index) const
	{
		return *Add(m_first, index);
	}

	ReferenceType operator[](SizeType index)
	{
		return *Add(m_first, index);
	}

	void Clear()
	{
		Resize(0);

		m_first = m_buffer;
		m_last = m_buffer;
	}

	void Terminate()
	{
		if (m_buffer != KY_NULL)
		{
			m_allocator.deallocate(m_buffer, m_capacity);
			m_buffer = KY_NULL;
		}
		m_first = KY_NULL;
		m_last = KY_NULL;
	}

	KyResult Reserve(CapacityType size)
	{
		// Grow or shrink !
		if (size != m_capacity)
		{
			const SizeType countMax = (m_size<size? m_size:size);

			T* buffer = KY_NULL;

			if (size != 0)
			{
				buffer = m_allocator.allocate(size);
				if (buffer == 0)
					return Result::OutOfMemory;

				for (SizeType i=0; i<countMax; ++i)
				{
					// order elements
					buffer[i] = *Add(m_first, i);
				}
			}

			if (m_buffer != KY_NULL)
			{
				m_allocator.deallocate(m_buffer, m_capacity);
				m_buffer = KY_NULL;
			}

			m_capacity = size;
			m_buffer = buffer;

			if (m_size > m_capacity)
				m_size = m_capacity;

			m_first = m_buffer;
			m_last = Add(m_first, m_size);
		}

		return Result::Success;
	}

	
	KyResult Resize(SizeType count)
	{
		if (Capacity() < count)
		{
			KyResult error = Reserve(count + count / 2);
			if (Result::Fail(error))
				return error;
		}

		m_size = count;
		m_last = Add(m_first, m_size);

		return Result::Success;
	}

	

	/*!	Indicates whether or not the CircularArray is at its maximum capacity. */
	KyBool IsFull() const
	{
		return (m_size == m_capacity);
	}

	/*!	Indicates whether or not the CircularArray is empty, containing no data elements. */
	KyBool IsEmpty() const
	{
		return (Size() == 0);
	}

	/*!	Returns an iterator that points to the first element in the CircularArray, or 0 if the CircularArray is empty. */
	Iterator Begin()
	{
		return Iterator(this, IsEmpty() ? KY_NULL : m_first);
	}

	/*!	Returns an iterator that points to the last element in the CircularArray, or 0 if the CircularArray is empty. */
	Iterator End()
	{
		return Iterator(this, 0);
	}

	/*!	Returns an iterator that points to the first element in the CircularArray, or 0 if the CircularArray is empty. */
	ConstIterator Begin() const
	{
		ConstPointerType first(IsEmpty() ? KY_NULL : m_first);
		return ConstIterator(this, first);
	}

	/*!	Returns an iterator that points to the last element in the CircularArray, or 0 if the CircularArray is empty. */
	ConstIterator End() const
	{
		return ConstIterator(this, ConstPointerType(KY_NULL));
	}

	/*!	Retrieves the size of the vector container. */
	SizeType Size() const
	{
		return m_size;
	}

	/*! Retrieves the size of the allocated storage space for the elements of the vector container.*/
	SizeType Capacity() const
	{
		return m_capacity;
	}

protected:
	// Increment the pointer.
	template <class Pointer>
	void Increment(Pointer& p) const
	{
		if (++p == (m_buffer + m_capacity))
			p = m_buffer;
	}

	// Decrement the pointer.
	template <class Pointer>
	void Decrement(Pointer& p) const
	{
		if (p == m_buffer)
			p = (m_buffer + m_capacity);
		--p;
	}

	// Add <code>n</code> to the pointer.
	template <class Pointer>
	Pointer Add(Pointer p, SizeType n) const
	{
		return Pointer(p + (n < (SizeType)((m_buffer + m_capacity) - p) ? n : n - m_capacity));
	}

	// Subtract <code>n</code> from the pointer.
	template <class Pointer>
	Pointer Sub(Pointer p, SizeType n) const
	{
		return Pointer(p - (n > (p - m_buffer) ? n - m_capacity : n));
	}

protected:
	ValueType* m_buffer;
	SizeType m_size;
	SizeType m_capacity;
	Allocator m_allocator;

	PointerType m_first;
	PointerType m_last;
};

} // namespace Kaim

#endif // KY_CIRCULARARRAY_H
