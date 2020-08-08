/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_BITARRAY_H
#define KY_BITARRAY_H

#include <kypathdata/generic/memory.h>


namespace Kaim
{
/*! This class represents an array of KyUInt32 that can store integer values on fewer than 32 bits. 
	It can allow you to conserve memory when handling a large number of very small integers, all of 
	whose values are always less than a certain predefined value. For example, if all your integers 
	are less than 16 in value, they can be stored using 4 bits for each integer rather than 32. 
	\pp This class is used internally by the ConcreteBodyInfo class to manage date stamps for BodyInfo objects, 
	but you are free to use it in your own code as well. 
	\pp After you create an instance of this class, you must call Resize() to size the capacity of the 
	BitArray to the desired number of elements.
	\ingroup kyruntime_core */
class BitArray
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor
		\param bits		The number of bits reserved for each integer you add to the BitArray.
						This value cannot be greater than 32. */
	explicit BitArray(KyUInt32 bits);

	/*! \label_destructor */
	~BitArray();

	/*! Retrieves the integer at the specified index within the BitArray. The \c i value must be less than the maximum size of the BitArray. */
	KyUInt32 Get(KyUInt32 i) const;
	/*! Sets the value of the integer at the specified index within the BitArray. The \c i value must be less than the maximum size of the BitArray. */
	void Set(KyUInt32 i, const KyUInt32& value);

	/*! Enlarges the capacity of the BitArray to handle the specified number of elements. If the BitArray already
		contains more elements than the specified value, the size of the BitArray is not changed. */
	void Resize(KyUInt32 newSize);

	/*! Retrieves the current capacity of the BitArray. */
	KyUInt32 GetCount() const { return m_count; }

private:
	BitArray(const BitArray& rhs); // No implementation
	BitArray& operator=(const BitArray& rhs); // No implementation

private:
	KyUInt8* m_data;
	KyUInt32 m_bits;
	KyUInt32 m_count;
};

} //namespace Kaim

#endif // KY_BITARRAY_H
