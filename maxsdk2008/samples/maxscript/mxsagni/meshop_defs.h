#ifndef __MESHOP_DEFS_H__
#define __MESHOP_DEFS_H__

// ============================================================================

typedef struct{ meshUIParam	id; Value* val; } meshUIparamType;

meshUIParam 
GetID(meshUIparamType params[], int count, Value* val)
{
	for(int p = 0; p < count; p++)
		if(val == params[p].val)
			return params[p].id;

	// Throw an error showing valid args incase an invalid arg is passed
	TCHAR buf[2056]; buf[0] = '\0';
	
	_tcscat(buf, GetString(IDS_FUNC_NEED_ARG_OF_TYPE));	
	for (int i=0; i < count; i++) 
	{
		_tcscat(buf, params[i].val->to_string());
		if (i < count-1) _tcscat(buf, _T(" | #"));
	}	
	_tcscat(buf, GetString(IDS_FUNC_GOT_ARG_OF_TYPE));
	throw RuntimeError(buf, val); 
}

Value* 
GetName(meshUIparamType params[], int count, meshUIParam id, Value* def_val=NULL)
{
	for(int p = 0; p < count; p++)
		if(id == params[p].id)
			return params[p].val;
	if (def_val) return def_val;
	return NULL;
}

#define def_MeshDeltaUserParam_types()				\
	meshUIparamType MeshDeltaUserParamTypes[] = {	\
		{ MuiSelByVert,		n_selByVert },			\
		{ MuiIgBack,		n_igBack },				\
		{ MuiIgnoreVis,		n_ignoreVis },			\
		{ MuiSoftSel,		n_softSel },			\
		{ MuiSSUseEDist,	n_sSUseEDist },			\
		{ MuiSSEDist,		n_sSEDist },			\
		{ MuiSSBack,		n_sSBack },				\
		{ MuiWeldBoxSize,	n_weldBoxSize },		\
		{ MuiExtrudeType,	n_extrudeType },		\
		{ MuiShowVNormals,	n_showVNormals },		\
		{ MuiShowFNormals,	n_showFNormals },		\
		{ MuiPolyThresh,	n_polyThresh },			\
		{ MuiFalloff,		n_falloff },			\
		{ MuiPinch,			n_pinch },				\
		{ MuiBubble,		n_bubble },				\
		{ MuiWeldDist,		n_weldDist },			\
		{ MuiNormalSize,	n_normalSize },			\
		{ MuiDeleteIsolatedVerts, n_deleteIsolatedverts },	\
	};

#endif //__MESHOP_DEFS_H__














