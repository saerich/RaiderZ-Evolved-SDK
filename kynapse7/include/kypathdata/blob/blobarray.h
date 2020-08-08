/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_BlobArray_H
#define KyPathData_BlobArray_H


#include <kypathdata/blob/endianness.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{

/*! A BlobArray is a type of array that is compatible with the blob serialization framework. It is the
	only way to store a variable array of objects in a serializable blob.
	\tparam T			The type of object this array will hold. This type of array may not contain any pointers.
						It must contain only base types, other blob classes, or other BlobArray objects. Each type 
						of object stored in a BlobArray must have its own SwapEndianness() implementation.
	\pp See also the description for the \ref kypathdata_blob module.
\section blobarraydebug Debugging BlobArrays
To allow the debugger to display BlobArray content,
add these lines to "Microsoft Visual Studio 9.0\Common7\Packages\Debugger\autoexp.dat"
just before the [hresult] section at the very end of the file:
<tt>
;------------------------------------------------------------------------------
;  Kaim::BlobArray
;------------------------------------------------------------------------------
Kaim::BlobArray<*>{
	children
	(
		#array
		(
			expr : (($T1*) &(((char*)&($e.m_offset))[$e.m_offset]) )[$i],
			size : $e.m_count
		)
	)

	preview
	(
		#(
			"[",
			$e.m_count,
			"](",
			#array
			(
				expr : (($T1*) &(((char*)&($e.m_offset))[$e.m_offset]) )[$i],
				size : $e.m_count
			),
			")"
		)
	)
}
</tt>
*/
template <class T>
class BlobArray
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor */
	BlobArray() : m_count(0), m_offset(0) {}

	/*! Retrieves a const pointer to the data stored in this array. */
	inline const T* Values() const { return (const T*)((char*)&m_offset + m_offset); }

	/*! Retrieves a pointer to the data stored in this array. */
	inline T* Values() { return (T*)((char*)&m_offset + m_offset); }

public:
	KyUInt32 m_count; /*!< The number of elements in this BlobArray. Set by #BLOB_ARRAY and #BLOB_ARRAY_COPY during BaseBlobBuilder::DoBuild(). Do not modify. */
	KyInt32 m_offset; /*!< Set by #BLOB_ARRAY and #BLOB_ARRAY_COPY during BaseBlobBuilder::DoBuild(). Do not modify. */
};



// --------------------------------- inline implementation ---------------------------------

template <class T>
void SwapEndianness(Endianness::Target e, BlobArray<T>& self)
{
	if (e == Endianness::SYSTEM_ENDIANNESS)
	{
		// use count and values once they have been swapped
		SwapEndianness(e, self.m_count);
		SwapEndianness(e, self.m_offset);

		T* values = self.Values();
		for (KyUInt32 i = 0; i < self.m_count; ++i)
			SwapEndianness(e, values[i]);
	}
	else
	{
		// use count and values before they are swapped
		KyUInt32 count = self.m_count;
		T* values = self.Values();

		SwapEndianness(e, self.m_count);
		SwapEndianness(e, self.m_offset);
		
		for (KyUInt32 i = 0; i < count; ++i)
			SwapEndianness(e, values[i]);
	}
}


}


#endif

