/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef KYDEQUE_H
#define KYDEQUE_H

#include <kypathdata/containers/staticdeque.h>
#include <kypathdata/containers/poolhandler.h>
#include <kypathdata/generic/cachesize.h>


namespace Kaim
{
	namespace Internal
	{
		/* Deque of objects.
			Allocates by chunk aligned on cache and cache lines.
		 */
		template <typename T, typename PoolHandler>
		struct Deque
		{
			KY_DEFINE_NEW_DELETE_OPERATORS

		public:
			typedef Deque<T, PoolHandler> ContainerType;
			typedef T ValueType;
			typedef ptr_size_type SizeType; /*!< A type that represents the size of this container. */

		private:
			// Returns size of inner structures aligned on cache
			struct SizeOf
			{
				static const SizeType value = sizeof(ValueType);

				static const SizeType chunk_control = sizeof(SizeType)/*size*/ + sizeof(void*)/*first*/;
				static const SizeType node_control = sizeof(void*)/*previous*/ + sizeof(void*)/*next*/; 

				static const SizeType chunk_data = ((node_control + chunk_control + value) > cache_line) ? (((node_control + chunk_control + value)/cache_line)+1)*cache_line - (node_control + chunk_control) : ((cache_line - (node_control + chunk_control) )/value)*value;
				static const SizeType chunk = chunk_control + chunk_data; 

				static const SizeType node_data = chunk; 
				static const SizeType node = node_control + node_data; // aligned on cache line

			};
			
			// Returns count of elements in inner structures
			struct CountOf
			{
				static const SizeType value = SizeOf::chunk_data / SizeOf::value;
			};

		private:
			// Chunk
			typedef StaticDeque<ValueType, CountOf::value > Chunk; 

			// Node
			struct Node
			{
			private:
				KY_DEFINE_NEW_DELETE_OPERATORS

			public:
				typedef typename Chunk::Iterator Iterator;
				typedef typename Chunk::ConstIterator ConstIterator;

				Node() : previous(KY_NULL), next(KY_NULL), chunk()
				{
				}

				/*	Indicates whether or not the node is full. */
				bool IsFull() const
				{
					return chunk.IsFull();
				}

				/*	Indicates whether or not the node is empty. */
				bool IsEmpty() const
				{
					return chunk.IsEmpty();
				}

				/*	Retrieves the total number of element in the node. */
				SizeType Size() const
				{
					/* May optimize
					if (previous != KY_NULL && next != KY_NULL)
						return CountOf::value;
					*/

					return chunk.Size();
				}

				/* Add a element at the back of the node.
					\pre IsFull() == false
					\post IsEmpty() == false */
				void PushBack()
				{
					chunk.PushBack();
				}

				/* Add a element at the back of the node.
					\pre IsFull() == false
					\post IsEmpty() == false */
				void PushBack(const typename ContainerType::ValueType& value)
				{
					chunk.PushBack(value);
				}

				/* Add a element at the front of the node.
					\pre IsFull == false
					\post IsEmpty() == false */
				void PushFront(const typename ContainerType::ValueType& value)
				{
					chunk.PushFront(value);
				}

				/* Removes the last element in the node
					\pre IsEmpty() == false
					\post IsFull() == false 
				*/
				void PopBack()
				{
					chunk.PopBack();
				}

				/* Removes the first element in the node
					\pre IsEmpty() == false
					\post IsFull() == false 
				*/
				void PopFront()
				{
					chunk.PopFront();
				}

				/* Retrieves the first element in the node
					\pre IsEmpty() == false */
				ValueType& Front()
				{
					return chunk.Front();
				}

				/* Retrieves the first element in the node
					\pre IsEmpty() == false */
				const ValueType& Front() const
				{
					return chunk.Front();
				}

				/* Retrieves the last element in the node
					\pre IsEmpty() == false */
				ValueType& Back()
				{
					return chunk.Back();
				}

				/* Retrieves the last element in the node
					\pre IsEmpty() == false */
				const ValueType& Back() const
				{
					return chunk.Back();
				}

				/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
				Iterator Begin()
				{
					return chunk.Begin();
				}

				/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
				Iterator End()
				{
					return chunk.End();
				}

				/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
				Iterator Last()
				{
					return chunk.Last();
				}

				/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
				ConstIterator Begin() const
				{
					return chunk.Begin();
				}

				/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
				ConstIterator End() const
				{
					return chunk.End();
				}

				/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
				ConstIterator Last() const
				{
					return chunk.Last();
				}

				Node* previous;
				Node* next;
			
			private:
				Chunk chunk;
			}; // Node

		public:
			typedef T* PointerType; /*!< A type that represents a pointer to an element in this array. */
			typedef const T* ConstPointerType; /*!< A type that represents a const pointer to an element in this array. */
			typedef T& ReferenceType; /*!< A type that represents a reference to an element in this array. */
			typedef const T& ConstReferenceType; /*!< A type that represents a const reference to an element in this array. */

		// Traits for IteratorImpl
		public:
			struct NoConstTraits
			{
				typedef typename ContainerType::PointerType PointerType;
				typedef typename ContainerType::ReferenceType ReferenceType;
				typedef Node* NodeIteratorType;
				typedef typename Chunk::Iterator ChunkIteratorType;
			};

			struct ConstTraits
			{
				typedef typename ContainerType::ConstPointerType PointerType;
				typedef typename ContainerType::ConstReferenceType ReferenceType;
				typedef const Node* NodeIteratorType;
				typedef typename Chunk::ConstIterator ChunkIteratorType;

			};

		// IteratorImpl
		public:
			/*!	This class represents an iterator that can be used to iterate through the
				elements in a Deque.
				\tparam ContainerType		The type of buffer to be managed by this iterator. Use Deque.
				\tparam Traits		The const qualities of the pointers and references returned by this iterator. Use either
									\c ConstTraits for const pointers and references, or \c NoConstTraits for non-const pointers
									and references. */
			template <typename Traits >
			struct IteratorImpl
			{
			public:		
				typedef typename Traits::PointerType PointerType;
				typedef typename Traits::ReferenceType ReferenceType;
				typedef typename Traits::NodeIteratorType NodeIteratorType;
				typedef typename Traits::ChunkIteratorType ChunkIteratorType;
				typedef ptr_diff_type DifferenceType;
				
			public:	
				const ContainerType* buffer; /*!< The Deque managed by this iterator. */
				NodeIteratorType node;
				ChunkIteratorType chunkIterator; /*!< The type of data stored in the Deque. */

			public:
				/*!	For internal use by Deque. */
				IteratorImpl(const ContainerType* _buff) 
					: buffer(_buff), node(KY_NULL), chunkIterator()
				{
				}

				/*!	For internal use by Deque. */
				IteratorImpl(const IteratorImpl& other) 
					:buffer(other.buffer), node(other.node), chunkIterator(other.chunkIterator)
				{
				}

				/*!	For internal use by Deque. */
				IteratorImpl(const ContainerType* _buff, NodeIteratorType _node, ChunkIteratorType _chunkIterator)
					:buffer(_buff), node(_node), chunkIterator(_chunkIterator)
				{
				}

				/*!	Copies the state of the specified IteratorImpl to this. */
				IteratorImpl& operator = (const IteratorImpl& it) 
				{
					if (this == &it)
						return *this;

					buffer = it.buffer;
					node = it.node;
					chunkIterator = it.chunkIterator;

					return *this;
				}

				/*	Indicates whether or not the iterator is valid. */
				bool IsValid() const
				{
					return node != KY_NULL && chunkIterator.IsValid();
				}

				/*!	Returns the data stored in the Deque element pointed to
					by this IteratorImpl. 
					\pre IsValid() == true
				*/
				ReferenceType operator * ()  const
				{
					return *chunkIterator;
				}

				/*!	Returns the data stored in the Deque element pointed to
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
					++chunkIterator;
					if (chunkIterator == node->End())
					{
						if (node->next != KY_NULL)
						{
							node = node->next;
							chunkIterator = node->Begin();
						}
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
					if (chunkIterator != node->Begin())
					{
						--chunkIterator;
					}
					else
					{
						if (node != buffer->m_first)
						{
							node = node->previous;
							chunkIterator = node->Last();
						}
					}
					return *this;
				}

				/*!	Decrements the IteratorImpl to point to the previous element in the array. 
					\pre IsValid() == true
				*/
				IteratorImpl operator -- (int) 
				{
					IteratorImpl<Traits> tmp = *this;
					--*this;
					return tmp;
				}

				/*!	Tests whether or not the two specified Iterators point to the same data. 
					\pre IsValid() == true
				*/
				bool operator == (const IteratorImpl<Traits>& it) const 
				{
					return node == it.node && chunkIterator == it.chunkIterator;
				}

				/*!	Tests whether or not the two specified Iterators point to different data. 
					\pre IsValid() == true
				*/
				bool operator != (const IteratorImpl<Traits>& it) const 
				{
					return !(*this == it);
				}

				/*!	Sets the IteratorImpl to point to the element at the index \c n greater than
					the chunkIterator index. */
				IteratorImpl& operator += (DifferenceType n) 
				{
					if (n > 0) 
					{
						// distance to next node
						DifferenceType distanceToNextNode = chunkIterator.DistanceTo(node->End()) ;

						if (n >= distanceToNextNode)
						{
							node = node->next;
							n -= distanceToNextNode;

							distanceToNextNode = CountOf::value;
							while (n >= distanceToNextNode)
							{
								node = node->next;
								n -= distanceToNextNode;
							}

							chunkIterator = node->Begin();
						}
						
						chunkIterator += n;
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
					the chunkIterator index. */
				IteratorImpl operator + (DifferenceType n) const 
				{ 
					return IteratorImpl<Traits>(*this) += n; 
				}

				/*!	Sets the IteratorImpl to point to the element at the index \c n less than
					the chunkIterator index. 
					\pre IsValid() == true
				*/
				IteratorImpl& operator -= (DifferenceType n) 
				{
					if (n > 0) 
					{
						// distance to previous node
						DifferenceType distanceToNextNode = 1 + node->Begin().DistanceTo(chunkIterator);

						if (n >= distanceToNextNode)
						{
							node = node->previous;
							n -= distanceToNextNode;

							distanceToNextNode = CountOf::value;
							while (n >= distanceToNextNode)
							{
								node = node->previous;
								n -= distanceToNextNode;
							}

							chunkIterator = node->Last();
						}
							
						chunkIterator -= n;
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
					the chunkIterator index. 
					\pre IsValid() == true
				*/
				IteratorImpl operator - (DifferenceType n) const 
				{ 
					return IteratorImpl<Traits>(*this) -= n; 
				}

				/*!	Retrieves the element at the at the index \c n greater than
					the chunkIterator index. */
				ReferenceType operator [] (SizeType n) const 
				{ 
					return *(*this + n); 
				}
			};// IteratorImpl

			typedef IteratorImpl<NoConstTraits> Iterator; /*!< A type of iterator used to iterate through the elements in this array. */
			typedef IteratorImpl<ConstTraits> ConstIterator; /*!< A const type of iterator used to iterate through the elements in this array. */

		// Pool
		public:
			typedef ChunkBasedPool<Node> PoolType;
		private:
			typedef typename PoolType::Handle Handle;
			typedef typename PoolHandler::template Rebind<Node>::Type PoolHandlerType;

		// Deque
		public:
			/*	Constructor.
				\post IsEmpty() == true
				*/
			explicit Deque(PoolType* pool) : m_size(0), m_first(KY_NULL), m_last(KY_NULL), m_poolHandler(pool)
			{
			}

			/*	Constructor.
				\post IsEmpty() == true
				*/
			explicit Deque(KyUInt16 nbSlotsByChunk) : m_size(0), m_first(KY_NULL), m_last(KY_NULL), m_poolHandler(nbSlotsByChunk)
			{
			}

			/*	Copy Constructor. */
			Deque(const Deque& other) : m_size(0), m_first(KY_NULL), m_last(KY_NULL), m_poolHandler(other.m_poolHandler)
			{
				for (ConstIterator it = other.Begin(); it != other.End(); ++it)
					PushBack(*it);
			}

			Deque& operator=(const Deque& other)
			{
				Clear();
				for (ConstIterator it = other.Begin(); it != other.End(); ++it)
					PushBack(*it);

				return *this;
			}

			/*	Destructor.
				\post IsEmpty() == true
				*/
			~Deque()
			{
				Clear();
			}
	
			/*	Destroys all elements and deallocates the internal memory.
				\post IsEmpty() = true
			*/
			void Clear()
			{
				Node* node = m_first;
				while (node != KY_NULL)
				{
					Node* next = node->next;
					DeAllocateNode(node);
					node = next;
				}
				m_first = KY_NULL;
				m_last = KY_NULL;
				m_size = 0;
			}

			/* Add a element at the back of the container.
			\post IsEmpty() == false */
			KyResult PushBack()
			{
				if (m_last != KY_NULL)
				{
					if (m_last->IsFull())
					{
						Node* node = AllocateNode();
						if (node == KY_NULL)
							return Kaim::Result::OutOfMemory;
						
						m_last->next = node;
						node->previous = m_last;
						m_last = node;
					}
				}
				else
				{
					Node* node = AllocateNode();
					if (node == KY_NULL)
						return Kaim::Result::OutOfMemory;

					m_first = m_last = node;
				}

				m_last->PushBack();
				++m_size;

				return Kaim::Result::Success;
			}

			/* Add a element at the back of the container.
			\post IsEmpty() == false */
			KyResult PushBack(const ValueType& value)
			{
				if (m_last != KY_NULL)
				{
					if (m_last->IsFull())
					{
						Node* node = AllocateNode();
						if (node == KY_NULL)
							return Kaim::Result::OutOfMemory;
						
						m_last->next = node;
						node->previous = m_last;
						m_last = node;
					}
				}
				else
				{
					Node* node = AllocateNode();
					if (node == KY_NULL)
						return Kaim::Result::OutOfMemory;

					m_first = m_last = node;
				}

				m_last->PushBack(value);
				++m_size;

				return Kaim::Result::Success;
			}

			/* Add a element at the front of the container.
			\post IsEmpty() == false */
			KyResult PushFront(const ValueType& value)
			{
				if (m_first != KY_NULL)
				{
					if (m_first->IsFull())
					{
						Node* node = AllocateNode();
						if (node == KY_NULL)
							return Kaim::Result::OutOfMemory;
						
						node->next = m_first;
						m_first->previous = node;
						m_first = node;
					}
				}
				else
				{
					Node* node = AllocateNode();
					if (node == KY_NULL)
						return Kaim::Result::OutOfMemory;

					m_first = m_last = node;
				}

				m_first->PushFront(value);
				++m_size;

				return Kaim::Result::Success;
			}

			/* Removes the last element in the container, if the container is not empty
				\post IsFull() == false */
			void PopBack()
			{
				if (!IsEmpty())
				{
					m_last->PopBack();
					// PopBack node if empty
					if (m_last->IsEmpty())
					{
						Node* previous = m_last->previous;
						DeAllocateNode(m_last);
						m_last = previous;

						if (m_last == KY_NULL)
							m_first = KY_NULL;
					}					
					
					--m_size;
				}
			}

			/* Removes the first element in the container, if the container is not empty
				\post IsFull() == false */
			void PopFront()
			{
				if (!IsEmpty())
				{
					m_first->PopFront();
					// PopFront node if empty
					if (m_first->IsEmpty())
					{
						Node* next = m_first->next;
						DeAllocateNode(m_first);
						m_first = next;

						if (m_first == KY_NULL)
							m_last = KY_NULL;
					}	

					--m_size;
				}
			}

			/*	Retrieves the size of the container. */
			SizeType Size() const
			{
				return m_size;
			}

			/*	Indicates whether or not the container is empty. */
			bool IsEmpty() const
			{
				return m_size == 0;
			}

			/* Retrieves the first element in the container
				\pre IsEmpty() == false */
			ValueType& Front()
			{
				return m_first->Front();
			}

			/* Retrieves the first element in the container
				\pre IsEmpty() == false */
			const ValueType& Front() const
			{
				return m_first->Front();
			}

			/* Retrieves the last element in the container
				\pre IsEmpty() == false */
			ValueType& Back()
			{
				return m_last->Back();
			}

			/* Retrieves the last element in the container
				\pre IsEmpty() == false */
			const ValueType& Back() const
			{
				return m_last->Back();
			}

			/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
			Iterator Begin()
			{
				return IsEmpty() ? Iterator(this) : Iterator(this, m_first, m_first->Begin());
			}

			/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
			Iterator End()
			{
				return IsEmpty() ? Iterator(this) : Iterator(this, m_last, m_last->End());
			}

			/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
			Iterator Last()
			{
				return IsEmpty() ? Iterator(this) : Iterator(this, m_last, m_last->Last());
			}

			/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
			ConstIterator Begin() const
			{
				return IsEmpty() ? ConstIterator(this) : ConstIterator(this, FirstNode(), FirstNode()->Begin());
			}

			/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
			ConstIterator End() const
			{
				return IsEmpty() ? ConstIterator(this) : ConstIterator(this, LastNode(), LastNode()->End());
			}

			/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
			ConstIterator Last() const
			{
				return IsEmpty() ? ConstIterator(this) : ConstIterator(this, LastNode(), LastNode()->Last());
			}

			template <typename OtherIterator>
			KyResult PushBack(OtherIterator _begin, OtherIterator _end)
			{
				for (OtherIterator it = _begin; it !=_end ; ++it)
					PushBack(*it);	

				return Result::Success;
			}

			template <typename OtherIterator>
			KyResult PeekFront(OtherIterator _begin, OtherIterator _end)
			{
				Iterator src = Begin();
				OtherIterator dest = _begin;

				for (; dest != _end; ++dest, ++src)
					*dest = *src;

				return Result::Success;
			}

			template <typename OtherIterator>
			KyResult PopFront(OtherIterator _begin, OtherIterator _end)
			{
				for (OtherIterator it = _begin; it != _end; ++it)
				{
					*it = Front();
					PopFront();
				}

				return Result::Success;
			}

			KyResult operator<<(const ValueType& value)
			{
				return PushBack(value);
			}

			void Swap(ContainerType& other)
			{
				Kaim::Swap(m_size, other.m_size);
				Kaim::Swap(m_first, other.m_first);
				Kaim::Swap(m_last, other.m_last);
				m_poolHandler.Swap(other.m_poolHandler);
			}

			/*	Retrieves the element at #index position in the buffer.
				\pre IsEmpty() == false */
			ValueType& operator[](SizeType index)
			{
				return Begin()[index];
			}

			/*	Retrieves the element at #index position in the buffer.
				\pre IsEmpty() == false */
			const ValueType& operator[](SizeType index) const
			{
				return Begin()[index];
			}
		public:
			PoolType& GetPool() { return m_poolHandler.GetPool(); }
			const PoolType& GetPool() const { return m_poolHandler.GetPool(); }

		private:
			const Node* FirstNode() const
			{
				return m_first;
			}

			const Node* LastNode() const
			{
				return m_last;
			}

		private:
			Node* AllocateNode()
			{
				Handle handle = GetPool().New();
				return &handle.Get();
			}

			void DeAllocateNode(Node* node)
			{
				GetPool().Delete(Handle(node));
			}

		private:
			SizeType m_size;
			Node* m_first;
			Node* m_last;

		protected:
			PoolHandlerType m_poolHandler;
		};

	} // Internal

	/* Deque of objects.
		Allocates by chunk aligned on cache lines.
	 */
	template <typename T>
	class Deque : public Internal::Deque<T, OwnPoolHandler<T> >
	{
	public:
		typedef Internal::Deque<T, OwnPoolHandler<T> > BaseType;

	public:
		Deque(): BaseType(256) {}

		explicit Deque(KyUInt16 nbSlotsByChunk) : BaseType(nbSlotsByChunk) {}

		Deque(const Deque& other) : BaseType(other.GetPool().GetNbSlotsByChunk())
		{
			for (typename BaseType::ConstIterator it = other.Begin(); it != other.End(); ++it)
				BaseType::PushBack(*it);
		}

		Deque& operator=(const Deque& other)
		{
			BaseType::Clear();
			for (typename BaseType::ConstIterator it = other.Begin(); it != other.End(); ++it)
				BaseType::PushBack(*it);

			return *this;
		}
	};

	/* Deque of objects.
		Allocates by chunk aligned on cache lines.
	 */
	template <typename T>
	class SharedPoolDeque : public Internal::Deque<T, SharedPoolHandler<T> >
	{
	public:
		typedef Internal::Deque<T, SharedPoolHandler<T> > BaseType;
		typedef typename BaseType::PoolType PoolType;
		
	public:
		SharedPoolDeque() : BaseType((PoolType*)KY_NULL) {}

		explicit SharedPoolDeque(PoolType* pool) : BaseType(pool) {}

		SharedPoolDeque(const SharedPoolDeque& other) : BaseType(&other.GetPool())
		{
			for (typename BaseType::ConstIterator it = other.Begin(); it != other.End(); ++it)
				BaseType::PushBack(*it);
		}

		SharedPoolDeque& operator=(const SharedPoolDeque& other)
		{
			BaseType::Clear();
			for (typename BaseType::ConstIterator it = other.Begin(); it != other.End(); ++it)
				BaseType::PushBack(*it);

			return *this;
		}

		void SetPool(PoolType* pool) { this->m_poolHandler.SetPool(pool); }
		PoolType* GetSharedPool() const { return &(const_cast<SharedPoolDeque*>(this)->GetPool()); }
	};

} // Kaim

#endif // KYDEQUE_H
