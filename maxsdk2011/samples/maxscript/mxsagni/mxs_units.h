//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Maxscript code for accessing system units.
// AUTHOR: Larry Minton
// DATE: 
// August 7 2008, factored out of avg_DLX.cpp to this file. Chris Johnson
//**************************************************************************/
#pragma once

#include "maxscrpt.h"

//  =================================================
//  MAX Unit methods
//  =================================================
//  Maxscript syntax:
/* 
	units.DisplayType
	units.SystemType
	units.SystemScale
	units.MetricType
	units.USType
	units.USFrac
	units.CustomName
	units.CustomValue
	units.CustomUnit
*/

Value* formatValue_cf(Value** arg_list, int count);
Value* decodeValue_cf(Value** arg_list, int count);
Value* getUnitDisplayType();
Value* setUnitDisplayType(Value* val);

#ifndef USE_HARDCODED_SYSTEM_UNIT
Value* getUnitSystemType();
Value* setUnitSystemType(Value* val);

Value* getUnitSystemScale();
Value* setUnitSystemScale(Value* val);
#endif // USE_HARDCODED_SYSTEM_UNIT 

Value* getMetricDisplay();
Value* setMetricDisplay(Value* val);

Value* getUSDisplay();
Value* setUSDisplay(Value* val);

Value* getUSFrac();
Value* setUSFrac(Value* val);

Value* getCustomName();
Value* setCustomName(Value* val);

Value* getCustomValue();
Value* setCustomValue(Value* val);

Value* getCustomUnit();
Value* setCustomUnit(Value* val);