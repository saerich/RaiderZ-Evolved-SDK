/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

#ifndef KyPathData_Stack_H
#define KyPathData_Stack_H

#include <kypathdata/containers/staticstack.h>
#include <kypathdata/containers/array.h>
#include <kypathdata/containers/list.h>
#include <kypathdata/containers/fixedsizebufferpool.h>

#include <assert.h>

namespace Kaim
{
/* Stack of objects.
 */
template <typename T>
class Stack
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	typedef Stack<T> ContainerType;
	typedef T ValueType;
	typedef KyUInt32 SizeType; /*!< A type that represents the size of this container. */

private:
	// A linked-node in the container
	struct Node
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		Node(SizeType capacity, T* buffer);
		~Node();

		/* Raw copy of data, without reallocating the buffer.
		\pre Capacity() >= other.Capacity() */
		void Copy(const Node& other);
	
	public:
		/* Removes all elements in the container.
			\post IsEmpty() == true		*/
		void Clear();

	public:
		typedef AbstractStack<T> StaticStackType;
		typedef typename StaticStackType::Iterator Iterator;
		typedef typename StaticStackType::ConstIterator ConstIterator;

	public:
		/*	Indicates whether or not the node is full. */
		bool IsFull() const;

		/*	Indicates whether or not the node is empty. */
		bool IsEmpty() const;

		/*	Retrieves the number of element in the node. */
		SizeType Size() const;

		/*	Retrieves the max number of element in the node. */
		SizeType Capacity() const;

		/* Add a element at the back of the node.
			\pre IsFull() == false
			\post IsEmpty() == false */
		void PushBack(const typename ContainerType::ValueType& value);

		/* Add a element at the front of the node.
			\pre IsFull == false
			\post IsEmpty() == false */
		void PushFront(const typename ContainerType::ValueType& value);

		/* Removes the last element in the node
			\pre IsEmpty() == false
			\post IsFull() == false 
		*/
		void PopBack();

		/* Removes the first element in the node
			\pre IsEmpty() == false
			\post IsFull() == false 
		*/
		void PopFront();

		/* Retrieves the first element in the node
			\pre IsEmpty() == false */
		ValueType& Front();

		/* Retrieves the first element in the node
			\pre IsEmpty() == false */
		const ValueType& Front() const;

		/* Retrieves the last element in the node
			\pre IsEmpty() == false */
		ValueType& Back();

		/* Retrieves the last element in the node
			\pre IsEmpty() == false */
		const ValueType& Back() const;

		/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
		Iterator Begin();

		/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
		Iterator End();

		/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
		Iterator Last();

		/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
		ConstIterator Begin() const;

		/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
		ConstIterator End() const;

		/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
		ConstIterator Last() const;
	
	public:
		/*	Retrieves the size in memory of (node + buffer) required to contain the given capacity. */
		static SizeType SizeOf(SizeType nodeCapacity);

		/*	Retrieves the capacity available for a given size in memory of (node + buffer). */
		static SizeType Capacity(SizeType size);
	public:
		Node* m_previous;
		Node* m_next;

	private:
		StaticStackType m_stack;
	}; // Node

public:
	/* Pool of nodes of different capacity.
	*/
	struct Pool
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		typedef FixedSizeBufferPool NodePoolType;
		typedef Array<NodePoolType*> NodePoolListType;
		typedef typename NodePoolListType::ConstIterator ConstIterator;
		typedef typename NodePoolListType::Iterator Iterator;

	public:
		/*	Destructor.
			\post IsEmpty() == true
		*/
		~Pool();

		/* Add a pool of nodes of the specified capacity.
			\post IsEmpty() == false 
			\retval Result::OutOfMemory		Indicates that there is not enough memory to allocate pool of nodes.
			\retval Result::Success			Indicates that the operation is successfull.
		*/
		KyResult AddNodePool(SizeType nbElementsByNode, KyUInt16 nbNodesByChunk);

		/*	Indicates whether or not the pool is empty. */
		bool IsEmpty() const;

		/* Finds a pool of nodes whose capacity is strictly greater than \c nodeCapacity.
			\retval KY_NULL Indicates that the pool of nodes is not found.
		*/
		NodePoolType* FindPoolOfGreaterCapacity(SizeType nodeCapacity);

		/* Finds a pool of nodes whose capacity is equal to \c nodeCapacity.
			\retval KY_NULL Indicates that the pool of nodes is not found.
		*/
		NodePoolType* FindPoolOfCapacity(SizeType nodeCapacity);

		/* Returns the first pool of node in the pool.
			\pre IsEmpty() == false
		*/
		NodePoolType* First();

		/* Returns the last pool of node in the pool.
			\pre IsEmpty() == false
		*/
		NodePoolType* Last();

	private:
		NodePoolListType m_nodePools;
	};

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
		typedef Node* ContainerIteratorType;
		typedef typename Node::Iterator NodeIteratorType;
	};

	struct ConstTraits
	{
		typedef typename ContainerType::ConstPointerType PointerType;
		typedef typename ContainerType::ConstReferenceType ReferenceType;
		typedef const Node* ContainerIteratorType;
		typedef typename Node::ConstIterator NodeIteratorType;

	};

// IteratorImpl
public:
	/*!	This class represents an iterator that can be used to iterate through the
		elements in a Stack.
		\tparam ContainerType		The type of container to be managed by this iterator. Use Stack.
		\tparam Traits		The const qualities of the pointers and references returned by this iterator. Use either
							\c ConstTraits for const pointers and references, or \c NoConstTraits for non-const pointers
							and references. */
	template <typename Traits >
	struct IteratorImpl
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:		
		typedef typename Traits::PointerType PointerType;
		typedef typename Traits::ReferenceType ReferenceType;
		typedef typename Traits::ContainerIteratorType ContainerIteratorType;
		typedef typename Traits::NodeIteratorType NodeIteratorType;
		typedef KyInt32 DifferenceType;
		
	public:	
		const ContainerType* container; /*!< The Container managed by this iterator. */
		ContainerIteratorType containerIterator;
		NodeIteratorType nodeIterator; /*!< The type of data stored in the Stack. */

	public:
		/*!	For internal use by Stack. */
		IteratorImpl(const ContainerType* _buff);

		/*!	For internal use by Stack. */
		IteratorImpl(const typename Stack<T>::template IteratorImpl<Traits>& other);

		/*!	For internal use by Stack. */
		IteratorImpl(const ContainerType* _buff, ContainerIteratorType _containerIterator, NodeIteratorType _nodeIterator);

		/*!	Copies the state of the specified IteratorImpl to this. */
		typename Stack<T>::template IteratorImpl<Traits>& operator = (const typename Stack<T>::template IteratorImpl<Traits>& it);

		/*	Indicates whether or not the iterator is valid. */
		bool IsValid() const;

		/*!	Returns the data stored in the Stack element pointed to
			by this IteratorImpl. 
			\pre IsValid() == true
		*/
		ReferenceType operator * ()  const;

		/*!	Returns the data stored in the Stack element pointed to
			by this IteratorImpl. 
			\pre IsValid() == true
		*/
		PointerType operator -> ()  const;

		/*!	Increments the IteratorImpl to point to the m_next element in the array. 
			\pre IsValid() == true
		*/
		typename Stack<T>::template IteratorImpl<Traits>& operator ++ ();

		/*!	Increments the IteratorImpl to point to the m_next element in the array. 
			\pre IsValid() == true
		*/
		typename Stack<T>::template IteratorImpl<Traits> operator ++ (int);

		/*!	Decrements the IteratorImpl to point to the m_previous element in the array. 
			\pre IsValid() == true
		*/
		typename Stack<T>::template IteratorImpl<Traits>& operator -- ();

		/*!	Decrements the IteratorImpl to point to the m_previous element in the array. 
			\pre IsValid() == true
		*/
		typename Stack<T>::template IteratorImpl<Traits> operator -- (int);

		/*!	Tests whether or not the two specified Iterators point to the same data. 
			\pre IsValid() == true
		*/
		bool operator == (const typename Stack<T>::template IteratorImpl<Traits>& it) const;

		/*!	Tests whether or not the two specified Iterators point to different data. 
			\pre IsValid() == true
		*/
		bool operator != (const typename Stack<T>::template IteratorImpl<Traits>& it) const;
	};// IteratorImpl

// Iterator
public:
	typedef IteratorImpl<NoConstTraits> Iterator; /*!< A type of iterator used to iterate through the elements in this array. */
	typedef IteratorImpl<ConstTraits> ConstIterator; /*!< A const type of iterator used to iterate through the elements in this array. */


// PoolType
public:
	typedef Pool PoolType;
private:
	typedef typename Pool::NodePoolType NodePoolType;

// Stack
public:
	/*	Constructor.
		\post IsEmpty() == true
		*/
	Stack();

	/*	Constructor.
		\post IsEmpty() == true
		*/
	explicit Stack(PoolType* pool);

	/*	Destructor.
		\post IsEmpty() == true
		*/
	~Stack();

	/*	Indicates whether or not the container has a pool to allocate/deallocate memory. */
	bool HasPool() const;

	/* Retrieves the pool used by the container to allocate/deallocate memory. */
	PoolType* GetPool();

	/* Retrieves the pool used by the container to allocate/deallocate memory. */
	const PoolType* GetPool() const;

	/*	Set the pool used by the container to allocate/deallocate memory. 
		Clears the container first. 
	*/
	void SetPool(PoolType* pool);

	/*	Destroys all elements and deallocates the internal memory.
		\post IsEmpty() = true
	*/
	void Clear();

	/* Add a element at the back of the container.
	\pre HasPool() == true
	\post IsEmpty() == false */
	KyResult PushBack(const ValueType& value);

	/* Removes the last element in the container. 
	\pre HasPool() == true */
	void PopBack();

	/*	Retrieves the size of the container. */
	SizeType Size() const;

	/*	Indicates whether or not the container is empty. */
	bool IsEmpty() const;

	/* Retrieves the first element in the container
		\pre IsEmpty() == false */
	ValueType& Front();

	/* Retrieves the first element in the container
		\pre IsEmpty() == false */
	const ValueType& Front() const;

	/* Retrieves the last element in the container
		\pre IsEmpty() == false */
	ValueType& Back();

	/* Retrieves the last element in the container
		\pre IsEmpty() == false */
	const ValueType& Back() const;

	/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
	Iterator Begin();

	/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
	Iterator End();

	/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
	Iterator Last();

	/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
	ConstIterator Begin() const;

	/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
	ConstIterator End() const;

	/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
	ConstIterator Last() const;

	/*!	Returns an iterator that points to the first element whose value is [value]. */
	Iterator FindFirst(const ValueType& value);

	/*!	Returns an iterator that points to the last element whose value is [value]. */
	Iterator FindLast(const ValueType& value);

private:
	SizeType m_size;
	Node* m_first;
	Node* m_last;

private:
	/* Returns the first node in the container.	*/
	const Node* FirstNode() const;

	/* Returns the last node in the container.	*/
	const Node* LastNode() const;

private:
	/* \pre HasPool() == true */
	NodePoolType* FindNodePool();

	/* \pre HasPool() == true */
	Node* AllocateNode();

	/* \pre HasPool() == true */
	void DeAllocateNode(Node* node);

private:
	PoolType* m_pool;

private:
	Stack(const Stack&); // No implementation.
	const Stack& operator=(const Stack&); // No implementation.
};

} // Kaim

/* 
Implementation
*/

namespace Kaim
{
// Node

template <typename T>
Stack<T>::Node::Node(typename Stack<T>::SizeType capacity, T* buffer) : m_previous(KY_NULL), m_next(KY_NULL), m_stack(capacity, buffer)
{
}

template <typename T>
Stack<T>::Node::~Node()
{
	Clear();
}

template <typename T>
void Stack<T>::Node::Copy(const typename Stack<T>::Node& other)
{
	Clear();

	m_previous = other.m_previous;
	m_next = other.m_next;
	m_stack.Copy(other.m_stack);
}

template <typename T>
void Stack<T>::Node::Clear()
{
	m_stack.Clear();
	m_previous = KY_NULL;
	m_next = KY_NULL;
}

template <typename T>
bool Stack<T>::Node::IsFull() const
{
	return m_stack.IsFull();
}

template <typename T>
bool Stack<T>::Node::IsEmpty() const
{
	return m_stack.IsEmpty();
}

template <typename T>
typename Stack<T>::SizeType Stack<T>::Node::Size() const
{
	return m_stack.Size();
}

template <typename T>
typename Stack<T>::SizeType Stack<T>::Node::Capacity() const
{
	return m_stack.Capacity();
}

template <typename T>
void Stack<T>::Node::PushBack(const typename Stack<T>::ValueType& value)
{
	m_stack.PushBack(value);
}

template <typename T>
void Stack<T>::Node::PushFront(const typename Stack<T>::ValueType& value)
{
	m_stack.PushFront(value);
}

template <typename T>
void Stack<T>::Node::PopBack()
{
	m_stack.PopBack();
}

template <typename T>
void Stack<T>::Node::PopFront()
{
	m_stack.PopFront();
}

template <typename T>
typename Stack<T>::ValueType& Stack<T>::Node::Front()
{
	return m_stack.Front();
}

template <typename T>
const typename Stack<T>::ValueType& Stack<T>::Node::Front() const
{
	return m_stack.Front();
}

template <typename T>
typename Stack<T>::ValueType& Stack<T>::Node::Back()
{
	return m_stack.Back();
}

template <typename T>
const typename Stack<T>::ValueType& Stack<T>::Node::Back() const
{
	return m_stack.Back();
}

template <typename T>
typename Stack<T>::Node::Iterator Stack<T>::Node::Begin()
{
	return m_stack.Begin();
}

template <typename T>
typename Stack<T>::Node::Iterator Stack<T>::Node::End()
{
	return m_stack.End();
}

template <typename T>
typename Stack<T>::Node::Iterator Stack<T>::Node::Last()
{
	return m_stack.Last();
}

template <typename T>
typename Stack<T>::Node::ConstIterator Stack<T>::Node::Begin() const
{
	return m_stack.Begin();
}

template <typename T>
typename Stack<T>::Node::ConstIterator Stack<T>::Node::End() const
{
	return m_stack.End();
}

template <typename T>
typename Stack<T>::Node::ConstIterator Stack<T>::Node::Last() const
{
	return m_stack.Last();
}

template <typename T>
typename Stack<T>::SizeType Stack<T>::Node::SizeOf(typename Stack<T>::SizeType nbNodesByChunk)
{
	return sizeof(Node) + sizeof(T) * nbNodesByChunk;
}

template <typename T>
typename Stack<T>::SizeType Stack<T>::Node::Capacity(typename Stack<T>::SizeType size)
{
	return (size - sizeof(Node))/sizeof(T);
}

// Pool

template <typename T>
Stack<T>::Pool::~Pool()
{
	for (KyUInt32 i = 0; i < m_nodePools.GetCount(); ++i)
	{
		KY_DELETE_CLASS(m_nodePools[i]);
	}
}

template <typename T>
KyResult Stack<T>::Pool::AddNodePool(typename Stack<T>::SizeType nbElementsByNode, KyUInt16 nbNodesByChunk)
{
	const SizeType sizeOfNode = Node::SizeOf(nbElementsByNode);

	Iterator it = m_nodePools.Begin();

	for (; it != m_nodePools.End(); ++it)
	{
		NodePoolType* nodePool  = *it;

		if (nodePool->GetSizeOfBuffer() == sizeOfNode)
			return Kaim::Result::Success;

		if (nodePool->GetSizeOfBuffer() > sizeOfNode)
			break;
	}

	NodePoolType* nodePool = KY_NEW_CLASS(NodePoolType(sizeOfNode, nbNodesByChunk));
	if (nodePool == KY_NULL)
		return Kaim::Result::OutOfMemory;

	m_nodePools.Insert(nodePool, it);
	return Kaim::Result::Success;
}

template <typename T>
bool Stack<T>::Pool::IsEmpty() const
{
	return m_nodePools.GetCount() == 0;
}

template <typename T>
typename Stack<T>::Pool::NodePoolType* Stack<T>::Pool::FindPoolOfGreaterCapacity(typename Stack<T>::SizeType nodeCapacity)
{
	const SizeType sizeOfNode = Node::SizeOf(nodeCapacity);

	for (ConstIterator it = m_nodePools.Begin(); it != m_nodePools.End(); ++it)
	{
		NodePoolType* nodePool  = *it;

		if (nodePool->GetSizeOfBuffer() > sizeOfNode)
		{
			return *it;
		}
	}

	return KY_NULL;
}

template <typename T>
typename Stack<T>::Pool::NodePoolType* Stack<T>::Pool::FindPoolOfCapacity(typename Stack<T>::SizeType nodeCapacity)
{
	const SizeType sizeOfNode = Node::SizeOf(nodeCapacity);

	for (ConstIterator it = m_nodePools.Begin(); it != m_nodePools.End(); ++it)
	{
		NodePoolType* nodePool  = *it;

		if (nodePool->GetSizeOfBuffer() == sizeOfNode)
		{
			return *it;
		}
	}

	return KY_NULL;
}

template <typename T>
typename Stack<T>::Pool::NodePoolType* Stack<T>::Pool::First()
{
	return *m_nodePools.Begin();
}

template <typename T>
typename Stack<T>::Pool::NodePoolType* Stack<T>::Pool::Last()
{
	return m_nodePools[m_nodePools.GetCount() - 1];
}

// IteratorImpl
template <typename T> template <typename Traits >
Stack<T>::IteratorImpl<Traits>::IteratorImpl(const Stack<T>* _buff) :
	container(_buff), containerIterator(KY_NULL), nodeIterator()
{
}

template <typename T> template <typename Traits >
Stack<T>::IteratorImpl<Traits>::IteratorImpl(const typename Stack<T>::template IteratorImpl<Traits>& other) 
	:container(other.container), containerIterator(other.containerIterator), nodeIterator(other.nodeIterator)
{
}

template <typename T> template <typename Traits >
Stack<T>::IteratorImpl<Traits>::IteratorImpl(const Stack<T>* _buff, typename Stack<T>::template IteratorImpl<Traits>::ContainerIteratorType _containerIterator, typename Stack<T>::template IteratorImpl<Traits>::NodeIteratorType _nodeIterator)
	:container(_buff), containerIterator(_containerIterator), nodeIterator(_nodeIterator)
{
}

template <typename T> template <typename Traits >
typename Stack<T>::template IteratorImpl<Traits>& Stack<T>::IteratorImpl<Traits>::operator = (const typename Stack<T>::template IteratorImpl<Traits>& it) 
{
	if (this == &it)
		return *this;

	container = it.container;
	containerIterator = it.containerIterator;
	nodeIterator = it.nodeIterator;

	return *this;
}

template <typename T> template <typename Traits >
bool Stack<T>::IteratorImpl<Traits>::IsValid() const
{
	return containerIterator != KY_NULL && nodeIterator.IsValid();
}

template <typename T> template <typename Traits >
typename Stack<T>::template IteratorImpl<Traits>::ReferenceType Stack<T>::IteratorImpl<Traits>::operator * ()  const
{
	return *nodeIterator;
}

template <typename T> template <typename Traits >
typename Stack<T>::template IteratorImpl<Traits>::PointerType Stack<T>::IteratorImpl<Traits>::operator -> ()  const
{ 
	return &(operator*()); 
}

template <typename T> template <typename Traits >
typename Stack<T>::template IteratorImpl<Traits>& Stack<T>::IteratorImpl<Traits>::operator ++ () 
{
	++nodeIterator;
	if (nodeIterator == containerIterator->End())
	{
		if (containerIterator->m_next != KY_NULL)
		{
			containerIterator = containerIterator->m_next;
			nodeIterator = containerIterator->Begin();
		}
	}
	return *this;
}

template <typename T> template <typename Traits >
typename Stack<T>::template IteratorImpl<Traits> Stack<T>::IteratorImpl<Traits>::operator ++ (int) 
{
	IteratorImpl<Traits> tmp = *this;
	++*this;
	return tmp;
}

template <typename T> template <typename Traits >
typename Stack<T>::template IteratorImpl<Traits>& Stack<T>::IteratorImpl<Traits>::operator -- () 
{
	if (nodeIterator != containerIterator->Begin())
	{
		--nodeIterator;
	}
	else
	{
		if (containerIterator != container->m_first)
		{
			containerIterator = containerIterator->m_previous;
			nodeIterator = containerIterator->Last();
		}
	}
	return *this;
}

template <typename T> template <typename Traits >
typename Stack<T>::template IteratorImpl<Traits> Stack<T>::IteratorImpl<Traits>::operator -- (int) 
{
	IteratorImpl<Traits> tmp = *this;
	--*this;
	return tmp;
}

template <typename T> template <typename Traits >
bool Stack<T>::IteratorImpl<Traits>::operator == (const typename Stack<T>::template IteratorImpl<Traits>& it) const 
{
	return containerIterator == it.containerIterator && nodeIterator == it.nodeIterator;
}

template <typename T> template <typename Traits >
bool Stack<T>::IteratorImpl<Traits>::operator != (const typename Stack<T>::template IteratorImpl<Traits>& it) const 
{
	return !(*this == it);
}

// Stack
template <typename T>
Stack<T>::Stack() : m_size(0), m_first(KY_NULL), m_last(KY_NULL), m_pool(KY_NULL)
{
}

template <typename T>
Stack<T>::Stack(typename Stack<T>::PoolType* pool) : m_size(0), m_first(KY_NULL), m_last(KY_NULL), m_pool(pool)
{
}

template <typename T>
Stack<T>::~Stack()
{
	Clear();
}

template <typename T>
bool Stack<T>::HasPool() const
{
	return m_pool != KY_NULL;
}

template <typename T>
typename Stack<T>::PoolType* Stack<T>::GetPool() 
{ 
	return m_pool; 
}

template <typename T>
const typename Stack<T>::PoolType* Stack<T>::GetPool() const 
{ 
	return m_pool; 
} 

template <typename T>
void Stack<T>::SetPool(typename Stack<T>::PoolType* pool) 
{ 
	Clear();

	m_pool = pool; 
}

template <typename T>
void Stack<T>::Clear()
{
	if (HasPool() == true)
	{
		Node* node = m_first;
		while (node != KY_NULL)
		{
			Node* next = node->m_next;
			DeAllocateNode(node);
			node = next;
		}
		m_first = KY_NULL;
		m_last = KY_NULL;
		m_size = 0;
	}
}

template <typename T>
KyResult Stack<T>::PushBack(const typename Stack<T>::ValueType& value)
{
	if (m_last != KY_NULL)
	{
		if (m_last->IsFull())
		{
			Node* node = AllocateNode();
			if (node == KY_NULL)
				return Kaim::Result::OutOfMemory;
			
			if (node->Capacity() > m_last->Capacity())
			{
				// Replace Node
				node->Copy(*m_last);
				DeAllocateNode(m_last);

				m_first = m_last = node;
			}
			else
			{
				// Add node
				m_last->m_next = node;
				node->m_previous = m_last;
				m_last = node;
			}
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

template <typename T>
void Stack<T>::PopBack()
{
	if (!IsEmpty())
	{
		m_last->PopBack();
		// PopBack node if empty
		if (m_last->IsEmpty())
		{
			Node* previous = m_last->m_previous;
			DeAllocateNode(m_last);
			m_last = previous;

			if (m_last == KY_NULL)
				m_first = KY_NULL;
			else
				m_last->m_next = KY_NULL;
		}					
		
		--m_size;
	}
}

template <typename T>
typename Stack<T>::SizeType Stack<T>::Size() const
{
	return m_size;
}

template <typename T>
bool Stack<T>::IsEmpty() const
{
	return m_size == 0;
}

template <typename T>
typename Stack<T>::ValueType& Stack<T>::Front()
{
	return m_first->Front();
}

template <typename T>
const typename Stack<T>::ValueType& Stack<T>::Front() const
{
	return m_first->Front();
}

template <typename T>
typename Stack<T>::ValueType& Stack<T>::Back()
{
	return m_last->Back();
}

template <typename T>
const typename Stack<T>::ValueType& Stack<T>::Back() const
{
	return m_last->Back();
}

template <typename T>
typename Stack<T>::Iterator Stack<T>::Begin()
{
	return IsEmpty() ? Iterator(this) : Iterator(this, m_first, m_first->Begin());
}

template <typename T>
typename Stack<T>::Iterator Stack<T>::End()
{
	return IsEmpty() ? Iterator(this) : Iterator(this, m_last, m_last->End());
}

template <typename T>
typename Stack<T>::Iterator Stack<T>::Last()
{
	return IsEmpty() ? Iterator(this) : Iterator(this, m_last, m_last->Last());
}

template <typename T>
typename Stack<T>::ConstIterator Stack<T>::Begin() const
{
	return IsEmpty() ? ConstIterator(this) : ConstIterator(this, FirstNode(), FirstNode()->Begin());
}

template <typename T>
typename Stack<T>::ConstIterator Stack<T>::End() const
{
	return IsEmpty() ? ConstIterator(this) : ConstIterator(this, LastNode(), LastNode()->End());
}

template <typename T>
typename Stack<T>::ConstIterator Stack<T>::Last() const
{
	return IsEmpty() ? ConstIterator(this) : ConstIterator(this, LastNode(), LastNode()->Last());
}

template <typename T>
typename Stack<T>::Iterator Stack<T>::FindFirst(const typename Stack<T>::ValueType& value) 
{
	typename Stack<T>::Iterator end = End();

	for (Iterator it = Begin(); it != end; ++it)
	{
		if (*it == value)
			return it;
	}

	return end;
}

template <typename T>
typename Stack<T>::Iterator Stack<T>::FindLast(const typename Stack<T>::ValueType& value) 
{
	if (IsEmpty() == false)
	{
		typename Stack<T>::Iterator begin = Begin();

		for (Iterator it = Last(); it != begin; --it)
		{
			if (*it == value)
				return it;
		}

		if (value == *begin)
			return begin;
	}

	return End();
}

template <typename T>
const typename Stack<T>::Node* Stack<T>::FirstNode() const
{
	return m_first;
}

template <typename T>
const typename Stack<T>::Node* Stack<T>::LastNode() const
{
	return m_last;
}

template <typename T>
typename Stack<T>::NodePoolType* Stack<T>::FindNodePool()
{
	typename PoolType::NodePoolType* nodePool = KY_NULL;

	if (m_last != KY_NULL)
	{
		nodePool = GetPool()->FindPoolOfGreaterCapacity(m_last->Capacity());

		if (nodePool == KY_NULL)
			nodePool = GetPool()->Last();
	}
	else
	{
		nodePool = GetPool()->First();
	}

	return nodePool;
}

template <typename T>
typename Stack<T>::Node* Stack<T>::AllocateNode()
{
	typename PoolType::NodePoolType* nodePool = FindNodePool();

	assert(nodePool != KY_NULL);

	if (nodePool == KY_NULL)
		return KY_NULL;

	void* buffer = nodePool->Allocate();
	if (buffer == KY_NULL)
		return KY_NULL;

	return new(buffer) Node(Node::Capacity(nodePool->GetSizeOfBuffer()), (T*)((unsigned char*)buffer + sizeof(Node)));
}

template <typename T>
void Stack<T>::DeAllocateNode(typename Stack<T>::Node* node)
{
	assert(node != KY_NULL);

	typename PoolType::NodePoolType* nodePool = GetPool()->FindPoolOfCapacity(node->Capacity());

	assert(nodePool != KY_NULL);

	if (nodePool != KY_NULL)
	{
		node->~Node();
		nodePool->Deallocate(node);
	}
}

} // Kaim

#endif // KyPathData_Stack_H
