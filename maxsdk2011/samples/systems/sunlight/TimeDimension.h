// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#pragma once
#include "NativeInclude.h"

#pragma managed(push, on)

class TimeDimension : public ParamDimension 
{
public:
	DimType DimensionType();
	
	// Enforce range limits. Out-of-range values are reset to valid limits.
	float Convert(float value);
	
	float UnConvert(float value);
};

#pragma managed(pop)
