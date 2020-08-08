/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: ? - secondary contact: NOBODY
#ifndef KYSTRING_H
#define KYSTRING_H

#include "kypathdata/generic/types.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/generic/algorithm.h"

#include "kypathdata/basesystem/logger.h"

#include <cstring>

namespace Kaim
{

class String
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	typedef ptr_size_type SizeType;
	typedef ptr_diff_type DifferenceType;
	typedef char ValueType;
	typedef char& ReferenceType;
	typedef const char& ConstReferenceType;
	typedef char* Iterator;
	typedef const char* ConstIterator;

public:
	/*	Creates an empty string. 
		\post IsEmpty() == true	*/
	String();

	/*	Copy constructor.	*/
	String(const String& right);

	/*	Constructor.	*/
	String(const ValueType* value);

	/*	Constructor.	*/
	String(const String& right, SizeType index, SizeType length);

	/*	Constructor.	*/
	template <typename OtherIterator>
	String(OtherIterator start, OtherIterator end);

	/*	Destructor.
		\post IsEmpty() == true	*/
	~String();

	/* Assignment operator.  */
	String& operator=(const String& right);

	/* Assign a null-terminated character array to a string.  */
	String& operator=(const ValueType* buffer);
public:
	/* 	Swaps the contents of two strings. */
	void Swap(String& other);

	/*	Indicates whether or not the container is empty. */
	bool IsEmpty() const;

	/*	Assigns a string.	*/
	KyResult Assign(const String& right);

	/*	Assigns a substring.	*/
	KyResult Assign(const String& right, SizeType index, SizeType size);

	/*	Assigns the first size characters of a null-terminated array of characters..	*/
	KyResult Assign(const ValueType* buffer, SizeType size);

	/*	Assigns a null-terminated array of characters.	*/
	KyResult Assign(const ValueType* buffer);

	/*	Assigns a range.	*/
	template <typename OtherIterator>
	KyResult Assign(OtherIterator start, OtherIterator end);

	/*	Appends a string.	*/
	KyResult Append(const String& right);

	/*	Appends a substring.	*/
	KyResult Append(const String& right, SizeType index, SizeType size);

	/*	Appends the first size characters of a null-terminated array of characters..	*/
	KyResult Append(const ValueType* buffer, const SizeType size);

	/*	Appends a null-terminated array of characters.	*/
	KyResult Append(const ValueType* buffer);

	/*	Appends a range.	*/
	template <typename OtherIterator>
	KyResult Append(OtherIterator start, OtherIterator end);

	/* \post Size() == 0 && Capacity() == 0*/
	void Terminate();

	/* Erases the entire container. 
	\post Size() == 0 && Capacity() >= Size()*/
	void Clear();

	/* \post Capacity() >= count && Capacity() >= Size()*/
	KyResult Reserve(SizeType count);

	/* Erases the entire container and deallocates memeory.
	\post Size() == count && Capacity() >= Size()*/
	KyResult Resize(SizeType count);

	/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
	Iterator Begin();

	/*!	Returns an iterator that points to the first element in the container, or 0 if the container is empty. */
	ConstIterator Begin() const;

	/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
	Iterator End();

	/*!	Returns an iterator that points after the last element in the container, or 0 if the container is empty. */
	ConstIterator End() const;

	/*	Retrieves the size of the container. */
	SizeType Size() const;

	/*	Retrieves the max number of element in the container. */
	SizeType Capacity() const;

	/* Returns a pointer to a null-terminated array of characters representing the string's contents. */ 
	const ValueType* CStr() const;

	KyInt32 Compare(const String& right) const;

	KyInt32 Compare(const ValueType* right) const;

	String& operator+=(const String& right);

	String& operator+=(const char* right);

	ReferenceType operator[](SizeType start);

	ConstReferenceType operator[](SizeType start) const;

public:
	/*  Find the last (highest position) element of the controlled sequence that matches any of the elements in [str]. */
	SizeType FindLastOf(const ValueType* str, SizeType index, SizeType count) const;

	/*  Find the last (highest position) element of the controlled sequence that matches any of the elements in [str]. */
	SizeType FindLastOf(const ValueType* str, SizeType index = npos) const;

	/*  Find the last (highest position) element of the controlled sequence that matches any of the elements in [str]. */
	SizeType FindLastOf(ValueType elem, SizeType index = npos) const;

	/*  Find the last (highest position) element of the controlled sequence that not matches any of the elements in [str]. */
	SizeType FindLastNotOf(const ValueType* str, SizeType index, SizeType count) const;

	/*  Find the last (highest position) element of the controlled sequence that not matches any of the elements in [str]. */
	SizeType FindLastNotOf(const ValueType* str, SizeType index = npos) const;

	/*  Find the last (highest position) element of the controlled sequence that not matches any of the elements in [str]. */
	SizeType FindLastNotOf(ValueType elem, SizeType index = npos) const;

	/* find the first (lowest beginning position) subsequence in the controlled sequence that matches [value]. */
	SizeType Find(ValueType value, SizeType index = 0) const;

public:
	static const SizeType npos = SizeType(-1);

public:
	/* Determines the lowest N in the range [0, count) for which eq(first[N], value) is true. If successful, it returns first + N. Otherwise, it returns KY_NULL. */
	static const ValueType* Find(const ValueType* first, SizeType count, ValueType value);

private:
	void Eos(const SizeType size);

private:
	ValueType* m_buffer;
	SizeType m_size;
	SizeType m_capacity;
};

template <typename OtherIterator>
String::String(OtherIterator start, OtherIterator end)
	:m_buffer(0), m_size(0), m_capacity(0)
{
	Assign(start, end);
}

template <typename OtherIterator>
KyResult String::Assign(OtherIterator start, OtherIterator end)
{
	return Assign(start, (end - start));
}

template <typename OtherIterator>
KyResult String::Append(OtherIterator start, OtherIterator end)
{
	return Append(start, (end - start));
}

inline bool operator==(const String& left, const String& right)
{
	return (left.Compare(right) == 0);
}

inline bool operator==(const char* left, const String& right)
{
	return (right.Compare(left) == 0);
}

inline bool operator==(const String& left, const char* right)
{
	return (left.Compare(right) == 0);
}

inline bool operator!=(const String& left, const String& right)
{
	return (left.Compare(right) != 0);
}

inline bool operator!=(const char* left, const String& right)
{
	return (right.Compare(left) != 0);
}

inline bool operator!=(const String& left, const char* right)
{
	return (left.Compare(right) != 0);
}

inline bool operator<(const String& left, const String& right)
{
	return (left.Compare(right) < 0);
}

inline bool operator<(const char* left, const String& right)
{
	return (right.Compare(left) > 0);
}

inline bool operator<(const String& left, const char* right)
{
	return (left.Compare(right) < 0);
}

inline String operator+(const String& left, const String& right)
{
	return (String(left)+=right);
}

inline String operator+(const char* left, const String& right)
{
	return (String(left)+=right);
}

inline String operator+(const String& left, const char* right)
{
	return (String(left)+=right);
}

template<> 
inline void Swap(String& left, String& right)
{
	left.Swap(right);
}

inline size_t StrLen(const char* value)
{
	return (value != KY_NULL) ? strlen(value) : 0;
}

}

typedef Kaim::String KyString;

#endif // KYSTRING_H
