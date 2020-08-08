/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETIPENDPOINT_H
#define KYDEBUGGERNETIPENDPOINT_H

#include <kypathdata/containers/kystring.h>

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			struct IPEndPoint
			{
				typedef KyString string_type;

				string_type m_ip;
				KyUInt16 m_port;
			};
		}
	}
}

#endif // KYDEBUGGERNETIPENDPOINT_H
