/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\brief Contains a collection of commonly used algorithms for sorting, comparison, etc.
	\ingroup kypathdata_generic */

#ifndef KYALGORITHM_H
#define KYALGORITHM_H

#include <string.h> // memcpy
#include <cstring>
#include "kypathdata/generic/types.h"

namespace Kaim
{
	// simple copy
	template<class InputIterator, class OutputIterator>
	inline OutputIterator Copy(InputIterator first, InputIterator last, OutputIterator dest)
	{
		for (; first != last; ++dest, ++first)
			*dest = *first;
		return (dest);
	}

	template<>
	inline char* Copy(const char* first, const char* last, char* dest)
	{
		memcpy(dest, first, last - first);

		return (dest);
	}

	// simple fill
	template<class InputIterator, class T>
	inline void Fill(InputIterator first, InputIterator last, const T& value)
	{
		for (; first != last; ++first)
			*first = value;
	}

	inline void Fill(char* first, char* last, KyInt32 value)
	{
		memset(first, value, last - first);
	}

	inline void Fill(signed char* first, signed char* last, KyInt32 value)
	{
		memset( first, value, last - first);
	}

	inline void Fill(unsigned char* first, unsigned char* last, KyInt32 value)
	{
		memset(first, value, last - first);
	}

	template <typename Iterator, typename Pred>
	Iterator Find_If(Iterator begin, Iterator end, Pred pred)
	{
		for (Iterator it = begin; it != end; ++it)
		{
			if (pred(*it))
				return it;
		}
		return end;
	}

	// simple swap
	template<class T> 
	inline void Swap(T& left, T& right)
	{
		if (&left != &right)
		{	
			T temp = left;

			left = right;
			right = temp;
		}
	}

	// simple range destroy
	template<class _Alloc> inline
	void DestroyRange(typename _Alloc::pointer first, typename _Alloc::pointer last, _Alloc& _Al)
	{	// destroy [first, last), arbitrary type
		for (; first != last; ++first)
			_Al.destroy(first);
	}

	template <typename T1, typename T2 = T1>
	class DefaultLess
	{
	public:
		bool operator () (const T1& lhs, const T2& rhs) const
		{
			return lhs < rhs;
		}
	};

	template <typename T, typename KEY, typename COMPARE>
	T* LowerBound(T* begin, T* end, const KEY& key, const COMPARE& lessCmp)
	{
		T* a = begin;
		T* b = end;
		T* c;

		for (c = a + (b - a) / 2; a != b; c = a + (b - a) / 2)
		{
			if (lessCmp(*c, key))
				a = c + 1;
			else
				b = c;
		}
		return c;
	}

	template <typename T, typename KEY>
	T* LowerBound(T* begin, T* end, KEY& key)
	{
		DefaultLess<T, KEY> lessCmp;

		return LowerBound(begin, end, key, lessCmp);
	}

} // namespace Kaim

#endif // KYALGORITHM_H
