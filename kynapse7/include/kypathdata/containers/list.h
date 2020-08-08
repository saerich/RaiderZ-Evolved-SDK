/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_containers */

#ifndef KY_LIST_H
#define KY_LIST_H

#include <kypathdata/generic/types.h>
#include <kypathdata/containers/chunkbasedpool.h>

namespace Kaim
{

namespace InternalList
{
template <typename T, typename PoolHandler> class List;


template <typename T, typename PoolHandler>
class Cell
{
	typedef typename ChunkBasedPool<Cell>::Handle Handle;
	KY_DEFINE_NEW_DELETE_OPERATORS
	friend class List<T, PoolHandler>;
	friend class List<T, PoolHandler>::ConstIterator;
public:
	Cell() {}
	explicit Cell(const T& data) : m_data(data) {}
	const T& GetData() const { return m_data; }
	T& GetData() { return m_data; }
	Handle GetNext() const { return m_next; }
	Handle GetPrevious() const { return m_previous; }
protected:
	const Cell& operator = (const Cell&); // No implementation.
protected:
	T m_data;
	Handle m_previous;
	Handle m_next;
};


template <typename T, typename PoolHandler>
class List
{
protected:
	KY_DEFINE_NEW_DELETE_OPERATORS

	typedef ChunkBasedPool<Cell<T, PoolHandler> > Pool;
	typedef typename Pool::Handle Handle;

public:
	class ConstIterator
	{
		friend class List;
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		explicit ConstIterator() {}
		explicit ConstIterator(const Handle& handle) : m_handle(handle) {}
		ConstIterator(const ConstIterator& i) : m_handle(i.m_handle) {}

		KY_FORCE_INLINE const ConstIterator& operator = (const ConstIterator& i) { m_handle = i.m_handle; return *this; }

		KY_FORCE_INLINE void Invalidate() { m_handle.Invalidate(); }
		KY_FORCE_INLINE bool IsValid() const { return m_handle.IsValid(); }

		KY_FORCE_INLINE bool operator == (const ConstIterator& i) const { return m_handle == i.m_handle; }
		KY_FORCE_INLINE bool operator != (const ConstIterator& i) const { return !(*this == i); }

		KY_FORCE_INLINE const T& operator*() const { return m_handle.Get().GetData(); }
		KY_FORCE_INLINE const T* operator->() const { return &m_handle.Get().GetData(); }

		KY_FORCE_INLINE ConstIterator& operator++() { m_handle = m_handle.Get().GetNext(); return *this; }
		KY_FORCE_INLINE ConstIterator operator++(int) { ConstIterator old = *this; ++*this; return old; }
		KY_FORCE_INLINE ConstIterator& operator--() { m_handle = m_handle.Get().GetPrevious(); return *this; }
		KY_FORCE_INLINE ConstIterator operator--(int) { ConstIterator old = *this; --*this; return old; }

		KY_FORCE_INLINE ConstIterator GetPrevious() const { if (!IsValid()) return *this; ConstIterator res = *this; return --res; }
		KY_FORCE_INLINE ConstIterator GetNext() const { if (!IsValid()) return *this; ConstIterator res = *this; return ++res; }

	protected:
		void SetNextIfRevelant(const Handle& handle) { if (m_handle.IsValid()) m_handle.Get().m_next = handle; }
		void SetPreviousIfRevelant(const Handle& handle) { if (m_handle.IsValid()) m_handle.Get().m_previous = handle; }
	protected:
		Handle m_handle;
	};

	class Iterator : public ConstIterator
	{
	public:
		explicit Iterator() {}
		explicit Iterator(const Handle& handle) : ConstIterator(handle) {}

		KY_FORCE_INLINE T& operator*() const { return const_cast<Handle&>(this->m_handle).Get().GetData(); }
		KY_FORCE_INLINE T& operator*() { return this->m_handle.Get().GetData(); }
		KY_FORCE_INLINE T* operator->() const { return &const_cast<Handle&>(this->m_handle).Get().GetData(); }
		KY_FORCE_INLINE T* operator->() { return &this->m_handle.Get().GetData(); }

		KY_FORCE_INLINE Iterator GetPrevious() { if (!this->IsValid()) return *this; Iterator res = *this; --res; return res; }
		KY_FORCE_INLINE Iterator GetNext() { if (!this->IsValid()) return *this; Iterator res = *this; ++res; return res; }
	};

protected:
	explicit List(Pool* pool) : m_poolHandler(pool), m_count(0)
	{
	}

	explicit List(KyUInt16 nbSlotsByChunk) : m_poolHandler(nbSlotsByChunk), m_count(0)
	{
	}

	~List()
	{
		Clear();
	}
public:
	/*! Returns the number of elements currently stored in the list. */
	KyUInt32 GetCount() const { return m_count; }

	bool IsEmpty() const { return m_count == 0; }

	/*! Insert default T before nextCellIt (default append to the end of the list). */
	Iterator Insert(Iterator nextCellIt = Iterator())
	{
		Handle handle = GetPool().New();

		return InternalInsert(handle, nextCellIt);
	}

	/*! Insert T before nextCellIt (default append to the end of the list). */
	Iterator Insert(const T& data, Iterator nextCellIt = Iterator())
	{
		Handle handle = GetPool().New(Cell<T, PoolHandler>(data));

		return InternalInsert(handle, nextCellIt);
	}

	Iterator PushBack() { return Insert(); }
	Iterator PushBack(const T& data) { return Insert(data); }

	Iterator PushFront() { return Insert(GetFirst()); }
	Iterator PushFront(const T& data) { return Insert(data, GetFirst()); }

	Iterator Erase(Iterator it)
	{
		Iterator nextIt = it.GetNext();
		Iterator prevIt = it.GetPrevious();
		const Iterator lastIt = m_last;
		const Iterator firstIt = m_first;

		nextIt.SetPreviousIfRevelant(prevIt.m_handle);
		prevIt.SetNextIfRevelant(nextIt.m_handle);

		if (it == firstIt)
			m_first = nextIt;
		if (it == lastIt)
			m_last = prevIt;

		GetPool().Delete(it.m_handle);
		--m_count;
		return nextIt;
	}

	void Clear()
	{
		while (GetLast().IsValid())
		{
			Iterator last(GetLast());

			Erase(last);
		}
	}

	ConstIterator Find(const T& data) const
	{
		for (ConstIterator it = GetFirst(); it.IsValid(); ++it)
		{
			if (*it == data)
				return it;
		}
		return Iterator();
	}

	Iterator Find(const T& data)
	{
		for (Iterator it = GetFirst(); it.IsValid(); ++it)
		{
			if (*it == data)
				return it;
		}
		return Iterator();
	}

	template<typename Predicate>
	void Remove_If(Predicate p)
	{
		for (Iterator it = GetFirst(); it.IsValid(); )
		{
			if (p(*it))
			{
				it = Erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	KyResult RemoveFirstOccurrence(const T& data)
	{
		Iterator it = Find(data);

		if (it.IsValid())
		{
			Erase(it);
			return KY_SUCCESS;
		}
		return KY_ERROR;
	}

	void RemoveAllOccurrences(const T& data)
	{
		for (Iterator it = GetFirst(); it.IsValid(); )
		{
			if (*it != data)
			{
				++it;
			}
			else
			{
				it = Erase(it);
			}
		}
	}

	/*! Returns an iterator that points to the first element in the list. */
	ConstIterator GetFirst() const { return m_first; }
	/*! Returns an iterator that points to the first element in the list. */
	Iterator GetFirst() { return m_first; }

	/*! Returns an iterator that points to the last element in the list. */
	ConstIterator GetLast() const { return m_last; }
	/*! Returns an iterator that points to the last element in the list. */
	Iterator GetLast() { return m_last; }

protected:
	Iterator InternalInsert(Handle handle, Iterator nextCellIt)
	{
		if (!handle.IsValid()) // No more memory
		{
			return Iterator();
		}
		++m_count;
		Cell<T, PoolHandler>& newCell = handle.Get();
		Iterator prevCellIt = nextCellIt.GetPrevious();

		if (!nextCellIt.IsValid())
		{
			prevCellIt = m_last;
			m_last = Iterator(handle);
		}
		if (nextCellIt == m_first)
		{
			m_first = Iterator(handle);
		}
		newCell.m_next = nextCellIt.m_handle;
		newCell.m_previous = prevCellIt.m_handle;
		nextCellIt.SetPreviousIfRevelant(handle);
		prevCellIt.SetNextIfRevelant(handle);
		return Iterator(handle);
	}

	ChunkBasedPool<Cell<T, PoolHandler> >& GetPool() { return m_poolHandler.GetPool(); }
private:
	List(const List&); // No implementation.
	const List& operator=(const List&); // No implementation.
protected:
	PoolHandler m_poolHandler;
	Iterator m_first; /*!< The first data cell. */
	Iterator m_last; /*!< The last data cell. */
	KyUInt32 m_count; /*!< The number of used data cells. */
};

template <typename T>
class OwnPoolHandler
{
public:
	explicit OwnPoolHandler(KyUInt16 size = 256) : m_pool(size) {}
	ChunkBasedPool<Cell<T, OwnPoolHandler<T> > >& GetPool() { return m_pool; }
protected:
	ChunkBasedPool<Cell<T, OwnPoolHandler<T> > > m_pool;
};

template <typename T>
class SharedPoolHandler
{
	typedef ChunkBasedPool<Cell<T, SharedPoolHandler<T> > > Pool;
public:
	explicit SharedPoolHandler(Pool* pool) : m_pool(pool) {}

	void SetPool(Pool* pool) { m_pool = pool; }
	Pool& GetPool() { return *m_pool; }
protected:
	Pool* m_pool;
};

} // InternalList



/*! List represents an ordered list of elements of a single type.
	\tparam T  The type of object to be stored in this list.
	\pp This class allocates memory by chunk when needed (In Insert() method).
		All allocated memory is freed when the List is destroyed.
	\section iterating Iterating through the list
	To iterate through the elements of the list in ascending order:
	\code
	typedef Kaim::List<MyType> List
	for (List::ConstIterator it = myList.GetFirst(); it.isValid(); ++it)
	{
		// You can use the * operator to retrieve the instance of your data type currently pointed to by the Iterator:
		const MyType& element = (*it);

		// ...
	} \endcode
	\ingroup kypathdata_containers */
template <typename T>
class List : public InternalList::List<T, InternalList::OwnPoolHandler<T> >
{
	typedef InternalList::List<T, InternalList::OwnPoolHandler<T> > baseType;
public:
	explicit List(KyUInt16 nbSlotsByChunk = 256) : baseType(nbSlotsByChunk) {}
private:
	List(const List&); // No implementation.
	const List& operator=(const List&); // No implementation.
};


/*! SharedPoolList represents an ordered list of elements of a single type, drawn as needed from
	a pre-allocated pool that you provide.
	\tparam T  The type of object to be stored in this list.
	\pp \section construction Constructing the list
	In order to use this class, you must create an instance of SharedPoolList::Pool.
	You must then set the SharedPoolList to use this pool of objects, either in its constructor or by
	calling SharedPoolList::SetPool().
	\pp For example:
	\code
	typedef SharedPoolList<MyType> SharedList
	SharedList::Pool myPool;
	SharedList myList(&myPool);
	// or myList.SetPool(&myPool);
	\endcode
	\pp Note that each instance of Pool should only be assigned to one SharedPoolList,
		and that each instance of SharedPoolList should only be assigned a Pool once.
	\pp \section iterating Iterating through the list
	To iterate through the elements of the list in ascending order:
	\code
	for (SharedList::ConstIterator it = myList.GetFirst(); it.isValid(); ++it)
	{
		// You can use the * operator to retrieve the instance of your data type currently pointed to by the Iterator:
		const MyType& element = (*it);

		// ...
	} \endcode
	\ingroup kypathdata_containers */
template <typename T>
class SharedPoolList : public InternalList::List<T, InternalList::SharedPoolHandler<T> >
{
public:
	typedef InternalList::List<T, InternalList::SharedPoolHandler<T> > baseType;
	typedef ChunkBasedPool<InternalList::Cell<T, InternalList::SharedPoolHandler<T> > > Pool;

	explicit SharedPoolList(Pool* pool = KY_NULL) : baseType(pool) {}

	void SetPool(Pool* pool) { this->m_poolHandler.SetPool(pool); }
	Pool* GetSharedPool() const { return &(const_cast<SharedPoolList*>(this)->m_poolHandler.GetPool()); }
private:
	SharedPoolList(const SharedPoolList&); // No implementation.
	const SharedPoolList& operator=(const SharedPoolList&); // No implementation.
};

} // namespace Kaim

#endif // KY_LIST_H
