/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_bridge */

#ifndef KSMEMORYBRIDGE_H
#define KSMEMORYBRIDGE_H

#include <kypathdata/generic/imemorybridge.h>

namespace Kaim
{
	/*!	This class is the default implementation of the IMemoryBridge interface created by the Bridge at its
		initialization.
		\pp This implementation invokes the standard C++ memory management functions, such as \c malloc
		and \c free. You can use this implementation as is, or write your own implementation of
		ILogBridge and set your own implementation in your Bridge.
		\pp See also :USERGUIDE:"Customizing Bridge Interfaces" and :USERGUIDE:"Initializing \SDKName".
		\ingroup kypathdata_bridge */
	class DefaultMemoryBridge : public Kaim::IMemoryBridge
	{
	public:
		/*!	\label_baseconstructor */
		DefaultMemoryBridge();

		/*!	\label_virtualdestructor */
		virtual ~DefaultMemoryBridge();

	public:
		/*!	\copydoc IMemoryBridge::Allocate()
			\pp This implementation of IMemoryBridge::Allocate() invokes \c malloc(). */
		virtual void* Allocate(KyUInt32 size);

		/*!	\copydoc IMemoryBridge::Free()
			\pp This implementation of IMemoryBridge::Free() invokes \c free(). */
		virtual void Free(void* ptr);

	public:
		/*	\copydoc IMemoryBridge::Reallocate()
			\pp This implementation of IMemoryBridge::Reallocate() invokes \c realloc(). */
		//virtual void* Reallocate(void* ptr, KyUInt32 size);

	public:
		/*!	\copydoc IMemoryBridge::AllocateAligned()
			\pp This implementation of IMemoryBridge::AllocateAligned() invokes \c memalign()
			on PlayStation 3, and \c _aligned_malloc() on all other platforms. */
		virtual void* AllocateAligned(KyUInt32 alignement, KyUInt32 size);

		/*!	\copydoc IMemoryBridge::FreeAligned()
			\pp This implementation of IMemoryBridge::FreeAligned() invokes \c free()
			on PlayStation 3, and \c _aligned_free() on all other platforms. */
		virtual void FreeAligned(void* ptr);

	public:
		/*!	\copydoc IMemoryBridge::IsMemoryValid()
			\pp This implementation of IMemoryBridge::IsMemoryValid() returns \c true in all cases. */
		virtual bool IsMemoryValid(void* /*address*/, KyUInt32 /*length*/) const
		{
			return true;
		}
	};

} // namespace Kaim


#endif // KSMEMORYBRIDGE_H
