/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETARRAY_H
#define KYDEBUGGERNETARRAY_H

#include <kyruntime/debugger/debuggertypes.h>
#include <kyruntime/debugger/debuggererrorutils.h>
#include <kyruntime/debugger/debuggererrorvalues.h>
#include <kypathdata/containers/vector.h>
#include <kyruntime/debugger/debuggernetbyteconverter.h>

#include <kypathdata/containers/kystring.h>

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			struct NetArray : public Kaim::Vector<Kaim::Debugger::ByteType>
			{
			};

			template <typename T>
			KyResult operator<<(Kaim::Debugger::Net::NetArray& buffer, const T& value)
			{	
				Kaim::Debugger::Net::net_byte_converter<T> converter(value);

				return buffer.PushBack( converter.Begin(), converter.End());
			}

			template <>
			KyResult operator<<(Kaim::Debugger::Net::NetArray& buffer, const KyString& value);
		}
	}
}




#endif // KYDEBUGGERNETARRAY_H
