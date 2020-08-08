/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_generic */

#ifndef KYIMEMORYBRIDGE_H
#define KYIMEMORYBRIDGE_H

#include "kypathdata/generic/types.h"

namespace Kaim
{
	/*!	Defines an interface used by \SDKName components to allocate and de-allocate memory.
		\pp The Bridge maintains an instance of a class that implements this interface, making the interface available to other
		\SDKName components. By default, the Bridge uses an instance of the DefaultMemoryBridge class. You can also write
		your own implementation of IMemoryBridge, and assign an instance of your class to the Bridge.
		\pp See :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_generic */
	class IMemoryBridge
	{
	private:
		static IMemoryBridge* s_instance;

	public:
		/*!	Retrieves the instance of the IMemoryBridge currently maintained by the Bridge. */
		static IMemoryBridge* GetInstance() { return s_instance; }
		
		/*!	For internal use. Use Bridge::ReplaceMemoryBridge() instead. */
		static void SetInstance(IMemoryBridge* instance) { s_instance = instance; }

	public:
		/*!	Allocates memory.
			\param size				The number of bytes to allocate.
			\return A void pointer to the allocated buffer, or NULL if there is insufficient memory available. */
		virtual void* Allocate(KyUInt32 size) = 0;

		/*!	Deallocates or frees a previously allocated memory buffer.
			\param ptr				The memory block that will be freed. */
		virtual void Free(void* ptr) = 0;

	public:
		/*!	\label_virtualdestructor */
		virtual ~IMemoryBridge() {}

	public:
		/*!	Allocates a memory block that is aligned to the specified boundary.
			\param alignement		The alignment value, which must be an integer power of 2.
			\param size				The size of the memory block to allocate.
			\return A void pointer to the memory buffer allocated, or 0 if insufficient memory
					is available. */
		virtual void* AllocateAligned(KyUInt32 alignement, KyUInt32 size) = 0;

		/*!	Frees the specified memory block, which must have been allocated through a call to AllocateAligned(). */
		virtual void FreeAligned(void* ptr) = 0;

	public:
		/*!	Indicates whether or not the calling process has read access to the specified range of memory.
			\param address			The first byte of the memory block.
			\param length			The size of the memory block, in bytes.
			\return \c true if the calling process has read access to all bytes in the specified memory range, or
					\c false otherwise. */
		virtual	bool IsMemoryValid(void* address, KyUInt32 length)  const = 0;
	};

} // namespace Kaim


#endif // KYIMEMORYBRIDGE_H
