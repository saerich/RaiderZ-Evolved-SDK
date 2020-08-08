/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERLOCALMEMORYREADER_H
#define KYDEBUGGERLOCALMEMORYREADER_H

#include <kyruntime/debugger/debuggertypes.h>
#include <kyruntime/debugger/debuggererrorvalues.h>
#include <kypathdata/generic/algorithm.h>

namespace Kaim
{
	namespace Debugger
	{
		struct LocalMemoryReader
		{
			template <class OtherIterator> 
			KyResult Bytes(ptr_type address, ptr_size_type length, OtherIterator result) const
			{
				if (IsPointerValid(address, length))
				{
					Kaim::Copy((ByteType*)address, (ByteType*)address + length, result);
				}
				else
				{
					Kaim::Fill(result, result + length, '?');
				}

				return Kaim::Result::Success;
			}

			bool IsPointerValid(ptr_type address, ptr_size_type length) const;
		};
	}
}

#endif // KYDEBUGGERLOCALMEMORYREADER_H
