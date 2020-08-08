#include "mxs_units.h"

#include "MAXScrpt.h"
#include "Numbers.h"
#include "Strings.h"
#include "MaxObj.h"

#include "resource.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "MXSAgni.h"
#include "agnidefs.h"

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

//  =================================================
//  MAX Unit methods
//  =================================================
// maxscript syntax

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

Value* 
formatValue_cf(Value** arg_list, int count)
{
	check_arg_count(formatValue, 1, count);
	TCHAR* outstring = FormatUniverseValue(arg_list[0]->to_float());
	if (outstring[0] == '\0') 
		throw RuntimeError (GetString(IDS_UNABLE_TO_FORMAT_VALUE), arg_list[0]);
	return new String(outstring);
}


Value* 
decodeValue_cf(Value** arg_list, int count)
{
	check_arg_count(decodeValue, 1, count);
	BOOL valid;
	float outval = DecodeUniverseValue(arg_list[0]->to_string(),&valid);
	if (!valid) 
		throw RuntimeError (GetString(IDS_UNABLE_TO_DECODE_VALUE), arg_list[0]);
	return Float::intern(outval);
}

Value*
getUnitDisplayType()
{
	def_displayunit_types();
	int type = GetUnitDisplayType();
	return GetName(displayunitTypes, elements(displayunitTypes), type, &undefined);
}

Value*
setUnitDisplayType(Value* val)
{
	def_displayunit_types();
	int type = GetID(displayunitTypes, elements(displayunitTypes), val);
	SetUnitDisplayType(type);
	return val;
}

#ifndef USE_HARDCODED_SYSTEM_UNIT

Value*
getUnitSystemType()
{
	def_systemunit_types();
	int type;
	float scale;
	GetMasterUnitInfo(&type, &scale);
	return GetName(systemunitTypes, elements(systemunitTypes), type, &undefined);
}

Value*
setUnitSystemType(Value* val)
{
	def_systemunit_types();
	int type;
	float scale;
	GetMasterUnitInfo(&type, &scale);
	type = GetID(systemunitTypes, elements(systemunitTypes), val);
	SetMasterUnitInfo(type, scale);
	return val;
}

Value*
getUnitSystemScale()
{
	int type;
	float scale;
	GetMasterUnitInfo(&type, &scale);
	return Float::intern(scale);
}

Value*
setUnitSystemScale(Value* val)
{
	int type;
	float scale;
	GetMasterUnitInfo(&type, &scale);
	SetMasterUnitInfo(type, val->to_float());
	return val;
}
#endif // USE_HARDCODED_SYSTEM_UNIT 

Value*
getMetricDisplay()
{
	def_metricunit_types();
	DispInfo info;
	GetUnitDisplayInfo(&info);
	return GetName(metricunitTypes, elements(metricunitTypes), info.metricDisp, &undefined);
}

Value*
setMetricDisplay(Value* val)
{
	def_metricunit_types();
	DispInfo info;
	GetUnitDisplayInfo(&info);
	info.metricDisp= GetID(metricunitTypes, elements(metricunitTypes), val);
	SetUnitDisplayInfo(&info);
	return val;
}

Value*
getUSDisplay()
{
	def_usunit_types();
	DispInfo info;
	GetUnitDisplayInfo(&info);
	return GetName(usunitTypes, elements(usunitTypes), info.usDisp, &undefined);
}

Value*
setUSDisplay(Value* val)
{
	def_usunit_types();
	DispInfo info;
	GetUnitDisplayInfo(&info);
	info.usDisp= GetID(usunitTypes, elements(usunitTypes), val);
	SetUnitDisplayInfo(&info);
	return val;
}

Value*
getUSFrac()
{
	def_usfrac_types();
	DispInfo info;
	GetUnitDisplayInfo(&info);
	return GetName(usfracTypes, elements(usfracTypes), info.usFrac, &undefined);
}

Value*
setUSFrac(Value* val)
{
	def_usfrac_types();
	DispInfo info;
	GetUnitDisplayInfo(&info);
	info.usFrac= GetID(usfracTypes, elements(usfracTypes), val);
	SetUnitDisplayInfo(&info);
	return val;
}

Value*
getCustomName()
{
	DispInfo info;
	GetUnitDisplayInfo(&info);
	return new String(info.customName);
}

Value*
setCustomName(Value* val)
{
	DispInfo info;
	GetUnitDisplayInfo(&info);
	info.customName= val->to_string();
	SetUnitDisplayInfo(&info);
	return val;
}

Value*
getCustomValue()
{
	DispInfo info;
	GetUnitDisplayInfo(&info);
	return Float::intern(info.customValue);
}

Value*
setCustomValue(Value* val)
{
	DispInfo info;
	GetUnitDisplayInfo(&info);
	info.customValue= val->to_float();
	SetUnitDisplayInfo(&info);
	return val;
}

Value*
getCustomUnit()
{
	def_systemunit_types();
	DispInfo info;
	GetUnitDisplayInfo(&info);
	return GetName(systemunitTypes, elements(systemunitTypes), info.customUnit, &undefined);
}

Value*
setCustomUnit(Value* val)
{
	def_systemunit_types();
	DispInfo info;
	GetUnitDisplayInfo(&info);
	info.customUnit= GetID(systemunitTypes, elements(systemunitTypes), val);
	SetUnitDisplayInfo(&info);
	return val;
}