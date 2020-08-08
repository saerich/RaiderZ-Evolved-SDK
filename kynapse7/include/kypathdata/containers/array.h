/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	



#ifndef KY_ARRAY_H
#define KY_ARRAY_H

#include <kypathdata/generic/memory.h>

/*!	\file
	\ingroup kypathdata_containers */

namespace Kaim
{
//////////////////////////////////////////////////////////////////////////////
// Array
//////////////////////////////////////////////////////////////////////////////

/*!	Array represents an array of elements of a single type.
	\tparam T			The type of object to be stored in this array.
	\ingroup kypathdata_containers */
template <typename T>
class Array
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	typedef T ValueType;
	typedef T* Iterator;
	typedef const T* ConstIterator;
	typedef KyUInt32 SizeType;

public:
	/*! Used in calls to Resize(), to specify how the array should react when the new capacity
		specified for the array is different from its current capacity. */
	enum eSizeAccuracy {
		/*! Forces the array to the specified size. If the current number of elements in the array is larger than the new size, the array
			is truncated, discarding the elements at the end of the array. */
		EXACT_SIZE,
		/*! If the current number of elements in the array is larger than the new size, the capacity of the array is reduced to match its
			current number of elements. This preserves all elements in the array. If the current number of elements in the array is less than
			the new size, the capacity of the array is doubled until it exceeds the new capacity. */
		MIN_SIZE,
		eSizeAccuracy_DUMMY_ = KYFORCEENUMSIZEINT
	};

public:
	/*! \label_constructor. */
	inline Array();

	/*! \label_destructor. */
	inline ~Array();

	/*! \label_constructor Copies the state of the specified Array to the newly created object. */
	inline Array(const Array&);

	/*! Copies the state of the specified Array to this object. */
	inline Array& operator=(const Array&);

	/*! Adds the specified element at the end of the array.
		If the capacity of the array is thereby exceeded,
		the array is doubled in size to hold the new element. */
	inline void PushBack(const ValueType& t);

	/*! Remove the last element of the array 
		\pre	The array must not be empty. */
	inline void PopBack();

	/*! Insert the specified element at given position of the array.
		If the capacity of the array is thereby exceeded,
		the array is doubled in size to hold the new element.
		return iterator on the new inserted element.
	*/
	inline Iterator Insert(const ValueType& t, Iterator pos);

	/*! Retrieves the element at the specified index within the array. */
	inline const ValueType& operator[](SizeType i) const;

	/*! Retrieves the element at the specified index within the array. */
	inline ValueType& operator[](SizeType i);

	/*! Removes all elements from the array, but does not free the memory allocated. */
	inline void Clear();

	/*! Removes all elements from the array and resizes the array to a maximum capacity of zero,
		freeing the memory that is no longer needed. */
	inline void ForceClear();

	/*! Reserves memory to store the specified number of elements in the array.
	If \c size is less than the current capacity of the array, the array is left unchanged. */
	inline KyResult Reserve(SizeType size);

	/*!	Resizes the array to the specified capacity.
		\param newSize			The desired capacity for the array. 
		\param accuracy			An element from the #eSizeAccuracy enumeration that determines how the method should treat conflicts.
								See the enumeration for details. */
	inline KyResult Resize(SizeType newSize, eSizeAccuracy accuracy = MIN_SIZE);

	/*!	Invokes Resize(KyUInt32, eSizeAccuracy) to resize the array, and fills any new elements added to the array
		with the item passed in the \c t parameter. */
	inline KyResult Resize(SizeType newSize, const T& t, eSizeAccuracy accuracy = MIN_SIZE);

	/*! Returns the number of elements currently in the array. */
	inline SizeType GetCount() const;

	/*! Returns the current capacity of the array: i.e. the maximum number of elements it may hold. */
	inline SizeType GetCapacity() const;

	/*! Returns the first element in the array. */
	ConstIterator Begin() const { return &m_pool[0]; }
	/*! Returns the first element in the array. */
	Iterator Begin() { return &m_pool[0]; }

	/*! Returns the last element in the array. */
	ConstIterator End() const { return &m_pool[0] + m_count; }
	/*! Returns the last element in the array. */
	Iterator End() { return &m_pool[0] + m_count; }

private:
	inline T* AllocatePool(SizeType capacity);
	inline void DeallocatePool(T* pool);
	inline KyResult ReallocatePool(SizeType capacity);

private:
	T* m_pool;
	SizeType m_capacity;
	SizeType m_count;
};

	////////////////////////////////////////////////////////////////////////////////
	// Array<KyBool>
	////////////////////////////////////////////////////////////////////////////////

	/*!	This class is a specialization of Array<T> optimized for storing items of the KyBool type. 
		It can be used exactly the same way as a Array of any other type.
		\ingroup kypathdata_containers */
	template <>
	class Array<KyBool>: public VirtualMemObject
	{
	public:
		typedef KyBool ValueType;
	public:
		enum eSizeAccuracy
		{
			EXACT_SIZE,
			MIN_SIZE,
			eSizeAccuracy_DUMMY_ = KYFORCEENUMSIZEINT
		};

		class BoolReference
		{
		public:
			inline BoolReference();
			inline BoolReference(KyUInt8* byte, KyUInt8 offset);
			inline BoolReference(const BoolReference& r);
			inline BoolReference& operator=(const BoolReference& r);
			inline BoolReference& operator=(KyBool val);
			inline operator KyBool() const;
		private:
			KyUInt8* m_byte;
			KyUInt8 m_offset;
		};

	public:
		inline Array();
		inline ~Array();
		inline Array(const Array&);
		inline Array& operator=(const Array&);

		inline void PushBack(KyBool val);
		inline const BoolReference operator[](KyUInt32 i) const;
		inline BoolReference operator[](KyUInt32 i);
		inline void Clear();
		inline void ForceClear();
		inline void Reserve(KyUInt32 size);
		inline void Resize(KyUInt32 newSize, eSizeAccuracy accuracy = MIN_SIZE);
		inline void Resize(KyUInt32 newSize, KyBool val, eSizeAccuracy accuracy = MIN_SIZE);

		inline KyUInt32 GetCount() const;
		inline KyUInt32 GetCapacity() const;

	private:

		KyUInt8* m_pool;
		KyUInt32 m_capacity;
		KyUInt32 m_count;
		KyUInt32 m_lastByte;
		KyUInt32 m_lastOffset;
	};


} // namespace Kaim

#include <kypathdata/containers/array.inl>

#endif //KY_ARRAY_H
