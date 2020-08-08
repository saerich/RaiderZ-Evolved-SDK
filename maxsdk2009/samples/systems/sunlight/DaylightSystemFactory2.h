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
#ifndef SUNCREATIONIMP2_H
#define SUNCREATIONIMP2_H

#include "IDaylightSystem.h"
#include "maxtypes.h"
#include "paramtype.h"
#include "Noncopyable.h"

class DaylightSystemFactory2 : public IDaylightSystemFactory2, public MaxSDK::Util::Noncopyable 
{
public:

	#pragma warning(push)
	#pragma warning(disable:4793)
	DECLARE_DESCRIPTOR_NDC(DaylightSystemFactory2); 
	#pragma warning(pop)

	// Function IDs for function publishing
	enum 
	{ 
		eCreateDaylightSystem, 
	};

	BEGIN_FUNCTION_MAP
		FN_2(eCreateDaylightSystem, TYPE_INTERFACE, fpCreate, TYPE_CLASS, TYPE_CLASS);
	END_FUNCTION_MAP

	// From IDaylightSystemFactory2
	virtual INode* Create(
		IDaylightSystem2*& pDaylight,
		const Class_ID* sunClassID = NULL, 
		const Class_ID* skyClassID = NULL
		);

	// Singleton
	static DaylightSystemFactory2& GetInstance();

private:
	static DaylightSystemFactory2 mTheDaylightSystemFactory;
	static float kDefaultCompassDiameter;
	static float kDefaultOrbitalScale;

	// Function published variant of IDaylightSystemFactory2::Create
	FPInterface* fpCreate(ClassDesc* sunCD, ClassDesc* skyCD);
};

#endif