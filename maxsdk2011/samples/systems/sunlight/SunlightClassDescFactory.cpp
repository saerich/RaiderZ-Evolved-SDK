// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "SunlightClassDescFactory.h"
#include "SunMasterClassDesc.h"
#include "DayMasterClassDesc.h"
#include "SlaveControlPosClassDesc.h"
#include "SlaveControlFloatClassDesc.h"
#include "DaySlaveControlPosClassDesc.h"
#include "DaySlaveControlFloatClassDesc.h"
#include "CompassRoseObjClassDesc.h"
#include "NatLightAssemblyClassDesc.h"

#pragma managed
using namespace ManagedServices;
using namespace System::Collections::Generic;


namespace SunlightSystem
{


IEnumerable<ClassDescWrapper^>^ SunlightClassDescFactory::CreateClassDescs()
{
	return gcnew cli::array<ClassDescWrapper^> {
		gcnew ClassDescOwner(new SunMasterClassDesc()),
		gcnew ClassDescOwner(new DayMasterClassDesc()),
		gcnew ClassDescOwner(new SlaveControlPosClassDesc()),
		gcnew ClassDescOwner(new SlaveControlFloatClassDesc()),
		gcnew ClassDescOwner(new DaySlaveControlPosClassDesc()),
		gcnew ClassDescOwner(new DaySlaveControlFloatClassDesc()),
		gcnew ClassDescOwner(new CompassRoseObjClassDesc()),
		gcnew ClassDescReference(NatLightAssemblyClassDesc::GetInstance())
	};
}


}

