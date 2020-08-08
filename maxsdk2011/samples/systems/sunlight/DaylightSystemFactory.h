//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#ifndef SUNCREATIONIMP_H
#define SUNCREATIONIMP_H

#pragma unmanaged
#include "NativeInclude.h"

#pragma managed

class DaylightSystemFactory : public IDaylightSystemFactory, public MaxSDK::Util::Noncopyable 
{
public:

	#pragma warning(push)
	#pragma warning(disable:4793)
	DECLARE_DESCRIPTOR_NDC(DaylightSystemFactory); 
	#pragma warning(pop)

	INode* Create(IDaylightSystem*& pDaylight);
	
	static void RegisterInstance();
	static void Destroy();

private:
	// The single instance of this class
	static DaylightSystemFactory* sTheDaylightSystemFactory;
};

#endif
