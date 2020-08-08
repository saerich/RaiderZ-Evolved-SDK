/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: LAPA
#ifndef KYDEBUGGERVECTOR3D_H
#define KYDEBUGGERVECTOR3D_H

#include <kyruntime/debugger/debuggernetmessage.h>
			
namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			template <>
			KyResult operator<<(Kaim::Debugger::Net::Message& message, const Kaim::Vec3f& value);
		}
	}
}
#endif // KYDEBUGGERVECTOR3D_H
