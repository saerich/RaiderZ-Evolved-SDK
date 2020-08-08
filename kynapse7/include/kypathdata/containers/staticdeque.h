/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

#ifndef KYSTATICDEQUE_H
#define KYSTATICDEQUE_H

#include "kypathdata/generic/typetraits.h"
#include <cstddef>

namespace Kaim
{
	/* Deque of objects.
		Allocates by chunk.
	 */
	template <class T, KyUInt32 Capacity>
	struct StaticDeque
	{
	public:	
		typedef T ValueType;
		typedef T* PointerType; /*!< A type that represents a pointer to an element in this array. */
		typedef const T* ConstPointerType; /*!< A type that represents a const pointer to an element in this array. */
		typedef T& ReferenceType; /*!< A type that represents a reference to an element in this array. */
		typedef const T& ConstReferenceType; /*!< A type that represents a const reference to an element in this array. */
		typedef ptr_diff_type DifferenceType;
		typedef ptr_size_type SizeType; /*!< A type that represents the size of each element in this array. */

	// Traits for IteratorImpl
	public:	
		struct NoConstTraits
		{
			typedef StaticDeque<T, Capacity> ContainerType;
			typedef typename ContainerType::PointerType PointerType;
			typedef typename ContainerType::ReferenceType ReferenceType;
		};

		struct ConstTraits
		{
			typedef StaticDeque<T, Capacity> ContainerType;
			typedef typename ContainerType::ConstPointerType PointerType;
			typedef typename ContainerType::ConstReferenceType ReferenceType;
		};

	public:	
		/*!	This class represents an iterator that can be used to iterate through the
			elements in a StaticDeque.
			\tparam ContainerType		The type of buffer to be managed by this iterator. Use StaticDeque.
			\tparam Traits		The const qualities of the pointers and references returned by this iterator. Use either
								\c ConstTraits for const pointers and references, or \c NoConstTraits for non-const pointers
								and references. */
		template <typename Traits >
		struct IteratorImpl
		{
		public:		
			typedef StaticDeque<T, Capacity> ContainerType;
			typedef typename Traits::PointerType PointerType;
			typedef typename Traits::ReferenceType ReferenceType;
			
		public:	
			const ContainerType* buffer; /*!< The StaticDeque managed by this iterator. */
			PointerType current; /*!< The type of data stored in the container. */

		public:
			/*!	For internal use by StaticDeque. 
				\post IsValid() == false
			*/
			IteratorImpl() 
				: buffer(KY_NULL), current(KY_NULL)
			{
			}

			/*!	For internal use by StaticDeque. */
			IteratorImpl(const IteratorImpl& other) 
				:buffer(other.buffer), current(other.current)
			{
			}

			/*!	For internal use by StaticDeque. */
			IteratorImpl(const ContainerType* buff, const PointerType p)
				:buffer(buff), current(p)
			{
			}

			/*!	Copies the state of the specified IteratorImpl to this. */
			IteratorImpl& operator = (const IteratorImpl& it) 
			{
				if (this == &it)
					return *this;

				buffer = it.buffer;
				current = it.current;

				return *this;
			}

			/*	Indicates whether or not the iterator is valid. */
			bool IsValid() const
			{
				return current != KY_NULL;
			}

			/*!	Returns the data stored in the container element pointed to
				by this IteratorImpl. 
				\pre IsValid() == true
			*/
			ReferenceType operator * ()  const
			{
				return *current;
			}

			/*!	Returns the data stored in the container element pointed to
				by this IteratorImpl. 
				\pre IsValid() == true
			*/
			PointerType operator -> ()  const
			{ 
				return &(operator*()); 
			}

			/*!	Increments the IteratorImpl to point to the next element in the array. 
				\pre IsValid() == true
			*/
			IteratorImpl& operator ++ () 
			{
				if (current != buffer->End().current)
				{
					if (current == buffer->Last().current)
						current = (PointerType)buffer->End().current;
					else
						buffer->Increment(current);
				}

				return *this;
			}

			/*!	Increments the IteratorImpl to point to the next element in the array.
				\pre IsValid() == true
			*/
			IteratorImpl operator ++ (int) 
			{
				IteratorImpl<Traits> tmp = *this;
				++*this;
				return tmp;
			}

			/*!	Decrements the IteratorImpl to point to the previous element in the array.
				\pre IsValid() == true
			*/
			IteratorImpl& operator -- () 
			{
				if (current != buffer->Begin().current)
					buffer->Decrement(current);

				return *this;
			}

			/*!	Decrements the IteratorImpl to point to the previous element in the array. */
			IteratorImpl operator -- (int) 
			{
				IteratorImpl<Traits> tmp = *this;
				--*this;
				return tmp;
			}

			/*!	Sets the IteratorImpl to point to the element at the index \c n greater than
				the current index.				
				\pre IsValid() == true
			*/
			IteratorImpl& operator += (DifferenceType n) 
			{
				if (n > 0) 
				{
					current = buffer->Add(current, n);
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
				the current index.
				\pre IsValid() == true
			*/
			IteratorImpl operator + (DifferenceType n) const 
			{ 
				return IteratorImpl<Traits>(*this) += n; 
			}

			/*!	Sets the IteratorImpl to point to the element at the index \c n less than
				the current index.
				\pre IsValid() == true
			*/
			IteratorImpl& operator -= (DifferenceType n) 
			{
				if (n > 0) 
				{
					current = buffer->Sub(current, n);
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
				the current index.
				\pre IsValid() == true
			*/
			IteratorImpl operator - (DifferenceType n) const 
			{ 
				return IteratorImpl<Traits>(*this) -= n; 
			}

			/*!	Retrieves the element at the at the index \c n greater than
				the current index.
				\pre IsValid() == true
			*/
			ReferenceType operator [] (DifferenceType n) const 
			{ 
				return *(*this + n); 
			}

			/*!	Tests whether or not the two specified Iterators point to the same data. */
			bool operator == (const IteratorImpl<Traits>& it) const 
			{
				return current == it.current;
			}

			/*!	Tests whether or not the two specified Iterators point to different data. */
			bool operator != (const IteratorImpl<Traits>& it) const 
			{
				return current != it.current;
			}

			/* Returns the relative distance to another iterator
				\pre IsValid() == true
			*/
			DifferenceType DistanceTo(const IteratorImpl<Traits>& it) const 
			{ 
				return buffer->Distance(current, it.current);
			}
		}; // IteratorImpl

	public:	
		typedef IteratorImpl<NoConstTraits> Iterator; /*!< A type of iterator used to iterate through the elements in this array. */
		typedef IteratorImpl<ConstTraits> ConstIterator; /*!< A const type of iterator used to iterate through the elements in this array. */

	public:	
		/*	Default constructor.
			\post IsEmpty() == true */
		StaticDeque() : size(0), first(Buffer())
		{
		}

		StaticDeque(const StaticDeque& other) : size(0), first(Buffer())
		{
			for (ConstIterator it = other.Begin(); it != other.End(); ++it)
				PushBack(*it);
		}

		const StaticDeque& operator=(const StaticDeque& other)
		{
			Clear();
			for (ConstIterator it = other.Begin(); it != other.End(); ++it)
				PushBack(*it);

			return *this;
		}

		/*	Destructor.
			\post IsEmpty() == true */
		~StaticDeque()
		{
			Clear();
		}

		/* Add a element at the end of the container, if the container is not full.
			\post [capacity>0] IsEmpty() == false */
		void PushBack(const ValueType& value)
		{
			if (!IsFull())
			{
				CallDefaultConstructor(Add(first, size));
				*Add(first, size++) = value;
			}
		}

		/* Add a default-constructed element at the end of the container, if the container is not full.
			\post [capacity>0] IsEmpty() == false */
		void PushBack()
		{
			if (!IsFull())
			{
				CallDefaultConstructor(Add(first, size++));
			}
		}

		/* Retrieves the last element in the container
			\pre IsEmpty() == false */
		ValueType& Back()
		{
			return *Add(first, size - 1);
		}

		/* Retrieves the last element in the container
			\pre IsEmpty() == false */
		const ValueType& Back() const
		{
			return *Add(first, size - 1);
		}

		/* Removes the last element in the container, if the container is not empty
			\post [capacity>0] IsFull() == false */
		void PopBack()
		{
			if (!IsEmpty())
			{
				CallDestructor(&Back());
				--size;
			}
		}

		/* Add a element at the front of the container, if the container is not full.
			\post [capacity>0] IsEmpty() == false */
		void PushFront(const ValueType& value)
		{
			if (!IsFull())
			{
				Decrement(first);
				CallDefaultConstructor(first);
				*first = value;
				++size;
			}
		}

		/* Add a default-constructed element at the front of the container, if the container is not full.
			\post [capacity>0] IsEmpty() == false */
		void PushFront()
		{
			if (!IsFull())
			{
				Decrement(first);
				CallDefaultConstructor(first);
				++size;
			}
		}

		/* Retrieves the first element in the container
			\pre IsEmpty() == false */
		ValueType& Front()
		{
			return *first;
		}

		/* Retrieves the first element in the container
			\pre IsEmpty() == false */
		const ValueType& Front() const
		{
			return *first;
		}

		/* Removes the first element in the container, if the container is not empty
			\post [capacity>0] IsFull() == false */
		void PopFront()
		{
			if (!IsEmpty())
			{
				CallDestructor(&Front());
				Increment(first);
				--size;
			}
		}

		/* Destroys all elements int he container
			\post IsEmpty() == true
		*/
		void Clear()
		{
			for (Iterator it = Begin(); it != End(); ++it)
			{
				CallDestructor(&(*it));
			}

			first = Buffer();
			size = 0;
		}

		/*	Indicates whether or not the container is full. */
		bool IsFull() const
		{
			return size == capacity;
		}

		/*	Indicates whether or not the container is empty. */
		bool IsEmpty() const
		{
			return size == 0;
		}

		/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
		Iterator Begin()
		{
			return Iterator(this, IsEmpty() ? KY_NULL : first);
		}

		/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
		Iterator End()
		{
			return Iterator(this, IsEmpty() ? KY_NULL : first + size);
		}

		/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
		Iterator Last()
		{
			return Iterator(this, IsEmpty() ? KY_NULL : Add(first, size - 1));
		}

		/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
		ConstIterator Begin() const
		{
			ConstPointerType _first(IsEmpty() ? KY_NULL : first);
			return ConstIterator(this, _first);
		}

		/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
		ConstIterator End() const
		{
			ConstPointerType _end(IsEmpty() ? KY_NULL : first + size);
			return ConstIterator(this, _end);
		}

		/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
		ConstIterator Last() const
		{
			ConstPointerType _last(IsEmpty() ? KY_NULL : Add(first, size - 1));
			return ConstIterator(this, _last);
		}

		/*	Retrieves the size of the container. */
		SizeType Size() const
		{
			return size;
		}

		/*	Retrieves the element at #index position in the container.
			\pre IsEmpty() == false */
		ValueType& operator[](SizeType index)
		{
			return *Add(first, index);
		}

		/*	Retrieves the element at #index position in the container.
			\pre IsEmpty() == false */
		const ValueType& operator[](SizeType index) const
		{
			return *Add(first, index);
		}

	public:
		static const SizeType capacity = Capacity;

	private:
		PointerType Buffer()
		{
			return reinterpret_cast<PointerType>(buffer);
		}

		ConstPointerType Buffer() const
		{
			return reinterpret_cast<ConstPointerType>(const_cast<char*>(buffer));
		}

		template <class Pointer>
		SizeType IndexOf(Pointer right) const 
		{
			// special case : right before first
			return (SizeType)((right < first) ? right - first + capacity : right - first);
		}

		// Increment the Pointer.
		template <class Pointer>
		void Increment(Pointer& p) const
		{
			(p == (Buffer() + capacity - 1)) ? p = (Pointer)Buffer() : ++p;
		}

		// Decrement the Pointer.
		template <class Pointer>
		void Decrement(Pointer& p) const
		{
			(p == Buffer()) ? p = (Pointer)(Buffer() + capacity - 1) : --p;
		}

		// Add <code>n</code> to the PointerType.
		template <class Pointer>
		Pointer Add(Pointer p, SizeType n) const
		{
			// special case :(p + n) after end
			return Pointer((IndexOf(p) + n >= size) ? first + size : (p + (n < (SizeType)((Buffer() + capacity) - p) ? n : n - capacity)));
		}

		// Subtract <code>n</code> from the PointerType.
		template <class Pointer>
		Pointer Sub(Pointer p, SizeType n) const
		{
			// special case :(p - n) before begin
			return Pointer((IndexOf(p) <  n) ? first : (p - (n > (SizeType)(p - Buffer()) ? n - capacity : n)));
		}

		/* Returns the relative distance between two iterators
		*/
		template <class Pointer>
		DifferenceType Distance(Pointer left, Pointer right) const 
		{ 
			return (left == first && right == first + size) ? (DifferenceType)Size() : ((right == first && left == first + size) ? -(DifferenceType)Size() : IndexOf(right) - IndexOf(left));
		}

	private:
		SizeType size;
		PointerType first;
		char buffer[capacity*sizeof(T)];
	};
}

#endif // KYSTATICDEQUE_H
