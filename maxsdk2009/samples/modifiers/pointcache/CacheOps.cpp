#include "PointCacheMods.h"

#include "maxscrpt\Maxscrpt.h"

#include "maxscrpt\listener.h"

#include "maxscrpt\Strings.h"
#include "maxscrpt\arrays.h"
#include "maxscrpt\3DMath.h"
#include "maxscrpt\Numbers.h"
#include "maxscrpt\definsfn.h"


#define get_cache_mod()															\
	Modifier* mod = arg_list[0]->to_modifier();									\
	Class_ID id = mod->ClassID();												\
	if ( id != POINTCACHEOSM_CLASS_ID && id != POINTCACHEWSM_CLASS_ID )			\
		throw RuntimeError(GetString(IDS_NOTCACHEERROR), arg_list[0]);			\
	PointCache* pmod = (PointCache*)mod;

def_struct_primitive( cacheOps_recordCache,		cacheOps, "RecordCache" );
def_struct_primitive( cacheOps_enableBelow,		cacheOps, "EnableBelow" );
def_struct_primitive( cacheOps_disableBelow,	cacheOps, "DisableBelow" );
def_struct_primitive( cacheOps_unload,			cacheOps, "Unload" );
def_struct_primitive( cacheOps_reload,			cacheOps, "Reload" );

def_struct_primitive( pointCacheMan_getMemoryUsed,	pointCacheMan, "GetMemoryUsed" );

// Set a forced channel # for loading Autodesk Cache files
def_struct_primitive( cacheOps_setChannel,		cacheOps, "SetChannel" );

// Clear forced channel #
def_struct_primitive( cacheOps_clearChannel,	cacheOps, "ClearChannel" );

/*
def_struct_primitive( cacheOps_createCache,		cacheOps, "CreateCache" );
def_struct_primitive( cacheOps_openCache,		cacheOps, "OpenCache" );
def_struct_primitive( cacheOps_closeCache,		cacheOps, "CloseCache" );
def_struct_primitive( cacheOps_isCacheOpen,		cacheOps, "IsCacheOpen" );
def_struct_primitive( cacheOps_recordPoints,	cacheOps, "RecordPoints" );
def_struct_primitive( cacheOps_recordObject,	cacheOps, "RecordObject" );
def_struct_primitive( cacheOps_createCache,		cacheOps, "GetNumSamples" );
def_struct_primitive( cacheOps_createCache,		cacheOps, "SeekSample" );
def_struct_primitive( cacheOps_createCache,		cacheOps, "GetSample" );
*/

Value* cacheOps_recordCache_cf(Value** arg_list, int count)
{
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	// NOTE: re-disable exception handling
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	check_arg_count(recordCache, 1, count);
	get_cache_mod();

	bool canceled;
	bool res = pmod->RecordToFile(pmod->m_hWnd,canceled);
	
	return (res ? &true_value : &false_value);
}

Value* cacheOps_enableBelow_cf(Value** arg_list, int count)
{
	check_arg_count(enableBelow, 1, count);
	get_cache_mod();
	if ( !pmod->m_iCore ) throw RuntimeError(GetString(IDS_CACHENOTSELECTED), arg_list[0]);

	pmod->EnableModsBelow(TRUE);

	return &ok;
}

Value* cacheOps_disableBelow_cf(Value** arg_list, int count)
{
	check_arg_count(disableBelow, 1, count);
	get_cache_mod();
	if ( !pmod->m_iCore ) throw RuntimeError(GetString(IDS_CACHENOTSELECTED), arg_list[0]);

	pmod->EnableModsBelow(FALSE);

	return &ok;
}

Value* cacheOps_unload_cf(Value** arg_list, int count)
{
	check_arg_count(unload, 1, count);
	get_cache_mod();
	if ( !pmod->m_iCore ) throw RuntimeError(GetString(IDS_CACHENOTSELECTED), arg_list[0]);

	pmod->Unload();

	return &ok;
}

Value* cacheOps_reload_cf(Value** arg_list, int count)
{
	check_arg_count(reload, 1, count);
	get_cache_mod();
	if ( !pmod->m_iCore ) throw RuntimeError(GetString(IDS_CACHENOTSELECTED), arg_list[0]);

	pmod->Reload();

	return &ok;
}

Value* cacheOps_setChannel_cf(Value** arg_list, int count)
{
	check_arg_count(setChannel, 2, count);
	get_cache_mod();
	if ( !pmod->m_iCore ) throw RuntimeError(GetString(IDS_CACHENOTSELECTED), arg_list[0]);

	pmod->SetForcedChannelNumber(arg_list[1]->to_int());

	return &ok;
}

Value* cacheOps_clearChannel_cf(Value** arg_list, int count)
{
	check_arg_count(clearChannel, 1, count);
	get_cache_mod();
	if ( !pmod->m_iCore ) throw RuntimeError(GetString(IDS_CACHENOTSELECTED), arg_list[0]);

	pmod->ClearForcedChannelNumber();

	return &ok;
}


Value* pointCacheMan_getMemoryUsed_cf(Value** arg_list, int count)
{
	check_arg_count(getMemoryUsed, 0, count);

	int memUsed = GetPointCacheManager().GetMemoryUsed();

	return Integer::intern(memUsed);
}
