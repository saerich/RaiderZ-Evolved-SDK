// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#pragma once
#pragma managed(push, on)
#using <ManagedServices.dll>

namespace SunlightSystem
{

public ref class SunlightClassDescFactory : ManagedServices::ClassDescFactory
{
public:
	virtual System::Collections::Generic::IEnumerable<ManagedServices::ClassDescWrapper^>^
		CreateClassDescs();
};


}

#pragma managed(pop)
