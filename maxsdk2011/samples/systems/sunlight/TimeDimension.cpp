// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "TimeDimension.h"
#include "autovis.h"

#pragma managed

DimType TimeDimension::DimensionType() 
{
	return DIM_CUSTOM;
}

// Enforce range limits. Out-of-range values are reset to valid limits.
float TimeDimension::Convert(float value)
{
	// Convert seconds to hours.
	if (value < 0.0f)
		return 0.0f;
	else if (value >= SECS_PER_DAY)
		value = SECS_PER_DAY - 1;
	return value/3600.0f;
}

float TimeDimension::UnConvert(float value)
{
	// Convert hours to seconds.
	if (value < 0.0f)
		return 0.0f;
	else if (value >= 24.0f)
		return SECS_PER_DAY - 1;
	return value*3600.0f;
}
