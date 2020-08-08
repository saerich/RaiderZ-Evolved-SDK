/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERBYTECONVERTER_H
#define KYDEBUGGERBYTECONVERTER_H

#include <kyruntime/debugger/debuggertypes.h>

namespace Kaim
{
	namespace Debugger
	{
		template <typename T>
		union byte_converter
		{
			T value;
			ByteType bytes[sizeof(T)];

			ByteType* begin()
			{
				return bytes;
			}

			ByteType* end()
			{
				return bytes + sizeof(T);
			}
		};
	}
}

#endif // KYDEBUGGERBYTECONVERTER_H
