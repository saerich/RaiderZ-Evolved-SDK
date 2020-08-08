/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef KyPathData_StaticStack_H
#define KyPathData_StaticStack_H

#include "kypathdata/generic/typetraits.h"

namespace Kaim
{
template <typename T>
struct AbstractStack
{
public:	
	typedef T ValueType;
	typedef T* PointerType; /*!< A type that represents a pointer to an element in this array. */
	typedef const T* ConstPointerType; /*!< A type that represents a const pointer to an element in this array. */
	typedef T& ReferenceType; /*!< A type that represents a reference to an element in this array. */
	typedef const T& ConstReferenceType; /*!< A type that represents a const reference to an element in this array. */
	typedef KyInt32 DifferenceType;
	typedef KyUInt32 SizeType; /*!< A type that represents the size of each element in this array. */
	typedef T* Iterator; /*!< A type of iterator used to iterate through the elements in this array. */
	typedef const T* ConstIterator; /*!< A const type of iterator used to iterate through the elements in this array. */

public:
	/*	Constructor.
		\post IsEmpty() == true */
	AbstractStack(SizeType _capacity, PointerType _buffer);

	/* Raw copy of data, without reallocating the buffer.
	\pre Capacity() >= other.Capacity() */
	void Copy(const AbstractStack& other);

public:	
	/* Add a element at the end of the buffer, if the buffer is not full.
		\post [Capacity()>0] IsEmpty() == false */
	void PushBack(const ValueType& value);

	/* Add a default-constructed element at the end of the buffer, if the buffer is not full.
		\post [Capacity()>0] IsEmpty() == false */
	void PushBack();

	/* Retrieves the last element in the buffer
		\pre IsEmpty() == false */
	ValueType& Back();

	/* Retrieves the last element in the buffer
		\pre IsEmpty() == false */
	const ValueType& Back() const;

	/* Removes the last element in the buffer
		\pre IsEmpty() == false
		\post [Capacity()>0] IsFull() == false */
	void PopBack();

	/* Removes all elements in the container.
		\post IsEmpty() == true		*/
	void Clear();

	/* Retrieves the first element in the container
		\pre IsEmpty() == false */
	ValueType& Front();

	/* Retrieves the first element in the container
		\pre IsEmpty() == false */
	const ValueType& Front() const;

	/*	Indicates whether or not the buffer is full. */
	bool IsFull() const;

	/*	Indicates whether or not the buffer is empty. */
	bool IsEmpty() const;

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
	/*	Retrieves the current size of the container. */
	SizeType Size() const;

	/*	Retrieves the total capacity of the container. */
	SizeType Capacity() const;

	/*	Retrieves the element at #index position in the container.
		\pre IsEmpty() == false */
	ValueType& operator[](SizeType index);

	/*	Retrieves the element at #index position in the container.
		\pre IsEmpty() == false */
	const ValueType& operator[](SizeType index) const;

	/*	Indicates whether or not the container contains an element pointed by the specified iterator. */
	bool Contains(ConstIterator it) const;

protected:
	PointerType Buffer();

	ConstPointerType Buffer() const;

protected:
	SizeType m_capacity; // keep it non-const non-static to access it after Capacity is casted away 
	SizeType m_size;
	T* m_buffer;
}; // AbstractStack

template <class T, KyUInt32 N>
struct StaticStack : AbstractStack<T>
{
public:	
	typedef T ValueType;
	typedef T* PointerType; /*!< A type that represents a pointer to an element in this array. */
	typedef const T* ConstPointerType; /*!< A type that represents a const pointer to an element in this array. */
	typedef T& ReferenceType; /*!< A type that represents a reference to an element in this array. */
	typedef const T& ConstReferenceType; /*!< A type that represents a const reference to an element in this array. */
	typedef KyInt32 DifferenceType;
	typedef KyUInt32 SizeType; /*!< A type that represents the size of each element in this array. */
	typedef T* Iterator; /*!< A type of iterator used to iterate through the elements in this array. */
	typedef const T* ConstIterator; /*!< A const type of iterator used to iterate through the elements in this array. */

public:	
	/*	Default constructor.
		\post IsEmpty() == true */
	StaticStack();

	/*	Destructor.
		\post IsEmpty() == true */
	~StaticStack();

private:
	char m_store[N*sizeof(T)];
}; // StaticStack


/*	Constructor.
	\post IsEmpty() == true */
template <typename T>
AbstractStack<T>::AbstractStack(typename AbstractStack<T>::SizeType _capacity, typename AbstractStack<T>::PointerType _buffer) : m_capacity(_capacity), m_size(0),m_buffer(_buffer)
{
}

/* Raw copy of data, without reallocating the buffer.
\pre Capacity() >= other.Capacity() */
template <typename T>
void AbstractStack<T>::Copy(const AbstractStack& other)
{
	Clear();	

	// raw copy
	memcpy(m_buffer, other.m_buffer, sizeof(T)*other.m_size);
	m_size = other.m_size;
}

/* Add a element at the end of the buffer, if the buffer is not full.
	\post [Capacity()>0] IsEmpty() == false */
template <typename T>
void AbstractStack<T>::PushBack(const typename AbstractStack<T>::ValueType& value)
{
	if (!IsFull())
	{
		CallDefaultConstructor(m_buffer + m_size);
		Buffer()[m_size] = value;
		++m_size;
	}
}

/* Add a default-constructed element at the end of the buffer, if the buffer is not full.
	\post [Capacity()>0] IsEmpty() == false */
template <typename T>
void AbstractStack<T>::PushBack()
{
	if (!IsFull())
	{
		CallDefaultConstructor(m_buffer + m_size);
		++m_size;
	}
}

/* Retrieves the last element in the buffer
	\pre IsEmpty() == false */
template <typename T>
typename AbstractStack<T>::ValueType& AbstractStack<T>::Back()
{
	return m_buffer[Size()-1];
}

/* Retrieves the last element in the buffer
	\pre IsEmpty() == false */
template <typename T>
const typename AbstractStack<T>::ValueType& AbstractStack<T>::Back() const
{
	return m_buffer[Size()-1];
}

/* Removes the last element in the buffer
	\pre IsEmpty() == false
	\post [Capacity()>0] IsFull() == false */
template <typename T>
void AbstractStack<T>::PopBack()
{
	if (!IsEmpty())
	{
		CallDestructor(&Back());
		--m_size;
	}
}

/* Removes all elements in the container.
	\post IsEmpty() == true		*/
template <typename T>
void AbstractStack<T>::Clear()
{
	Iterator end = End();
	for (Iterator it = Begin(); it != end; ++it)
	{
		CallDestructor(&(*it));
	}

	m_size = 0;
}

/* Retrieves the first element in the container
	\pre IsEmpty() == false */
template <typename T>
typename AbstractStack<T>::ValueType& AbstractStack<T>::Front()
{
	return *m_buffer;
}

/* Retrieves the first element in the container
	\pre IsEmpty() == false */
template <typename T>
const typename AbstractStack<T>::ValueType& AbstractStack<T>::Front() const
{
	return *m_buffer;
}

/*	Indicates whether or not the buffer is full. */
template <typename T>
bool AbstractStack<T>::IsFull() const
{
	return m_size == m_capacity;
}

/*	Indicates whether or not the buffer is empty. */
template <typename T>
bool AbstractStack<T>::IsEmpty() const
{
	return m_size == 0;
}

/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
template <typename T>
typename AbstractStack<T>::Iterator AbstractStack<T>::Begin()
{
	return  m_buffer;
}

/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
template <typename T>
typename AbstractStack<T>::Iterator AbstractStack<T>::End()
{
	return  m_buffer + m_size;
}

/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
template <typename T>
typename AbstractStack<T>::Iterator AbstractStack<T>::Last()
{
	return  m_buffer + m_size - 1;
}

/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
template <typename T>
typename AbstractStack<T>::ConstIterator AbstractStack<T>::Begin() const
{
	return  Buffer();
}

/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
template <typename T>
typename AbstractStack<T>::ConstIterator AbstractStack<T>::End() const
{
	return  Buffer() + m_size;
}

/*!	Returns an iterator that points to the last element in the container, or 0 if the container is empty. */
template <typename T>
typename AbstractStack<T>::ConstIterator AbstractStack<T>::Last() const
{
	return  Buffer() + m_size - 1;
}

/*	Retrieves the current size of the container. */
template <typename T>
typename AbstractStack<T>::SizeType AbstractStack<T>::Size() const
{
	return m_size;
}

/*	Retrieves the total capacity of the container. */
template <typename T>
typename AbstractStack<T>::SizeType AbstractStack<T>::Capacity() const
{
	return m_capacity;
}

/*	Retrieves the element at #index position in the container.
	\pre IsEmpty() == false */
template <typename T>
typename AbstractStack<T>::ValueType& AbstractStack<T>::operator[](typename AbstractStack<T>::SizeType index)
{
	return m_buffer[index];
}

/*	Retrieves the element at #index position in the container.
	\pre IsEmpty() == false */
template <typename T>
const typename AbstractStack<T>::ValueType& AbstractStack<T>::operator[](typename AbstractStack<T>::SizeType index) const
{
	return m_buffer[index];
}

/*	Indicates whether or not the container contains an element pointed by the specified iterator. */
template <typename T>
bool AbstractStack<T>::Contains(typename AbstractStack<T>::ConstIterator it) const
{
	return it >= Begin() && it < End();
}

template <typename T>
typename AbstractStack<T>::PointerType AbstractStack<T>::Buffer()
{
	return m_buffer;
}

template <typename T>
typename AbstractStack<T>::ConstPointerType AbstractStack<T>::Buffer() const
{
	return m_buffer;
}

/*	Default constructor.
	\post IsEmpty() == true */
template <class T, KyUInt32 N>
StaticStack<T, N>::StaticStack() : AbstractStack<T>(N, reinterpret_cast<T*>(&m_store))
{
}

/*	Destructor.
	\post IsEmpty() == true */
template <class T, KyUInt32 N>
StaticStack<T, N>::~StaticStack()
{
	AbstractStack<T>::Clear();
}

} // Kaim

#endif // KyPathData_StaticStack_H
