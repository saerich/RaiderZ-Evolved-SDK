/**********************************************************************
 *<
	FILE: MXSAgni.h

	DESCRIPTION: Main header file for MXSAgni.dlx

	CREATED BY: Ravi Karra, 1998

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/


#ifndef __MXSAGNI__H__
#define __MXSAGNI__H__

#include "meshdelta.h"
#include "MAXObj.h"
#include "resource.h"

struct paramType
{
	int	id; 
	Value* val;
};

// following defined in rk_wraps.cpp
extern void	DebugPrint(CharStream* out, const Matrix3& m3);
extern void	DebugPrint(CharStream* out, const Point3& p3);
extern void	DebugPrint(CharStream* out, const Point2& p2);

// following defined in common_funcs.cpp
extern int		GetID(paramType params[], int count, Value* val, int def_val=-1);
extern Value*	GetName(paramType params[], int count, int id, Value* def_val=NULL);
extern void		ValueToBitArray(Value* inval, BitArray &theBitArray, int maxSize, TCHAR* errmsg=_T(""), int selTypesAllowed=0);
extern void		ValueToBitArrayM(Value* inval, BitArray &theBitArray, int maxSize, TCHAR* errmsg, int selTypesAllowed, Mesh* mesh);
extern void		ValueToBitArrayP(Value* inval, BitArray &theBitArray, int maxSize, TCHAR* errmsg, int selTypesAllowed, MNMesh* poly);

extern INode*	_get_valid_node(MAXNode* _node, TCHAR* errmsg); // see #define get_valid_node below
extern BezierShape*		set_up_spline_access(INode* node, int index);
extern Mesh*	_get_meshForValue(Value* value, int accessType, ReferenceTarget** owner, TCHAR* fn);
extern Mesh*	getWorldStateMesh(INode* node, BOOL renderMesh);
extern Mesh*	_get_worldMeshForValue(Value* value, TCHAR* fn, BOOL renderMesh);
extern MNMesh*	_get_polyForValue(Value* value, int accessType, ReferenceTarget** owner, TCHAR* fn);
extern IMeshSelectData*	get_IMeshSelData_for_mod(Object* obj, Modifier*& target_mod, int mod_index);
extern MeshDeltaUser*	get_MeshDeltaUser_for_mod(Object* obj, Modifier*& target_mod, int mod_index);
extern void world_to_current_coordsys(Point3& p, int mode=0);
extern void world_from_current_coordsys(Point3& p, int mode=0);

#define get_meshForValue(_value, _accessType, _owner, _fn) _get_meshForValue(_value, _accessType, _owner, #_fn##)
#define get_worldMeshForValue(_value, _fn, _renderMesh) _get_worldMeshForValue(_value, #_fn##, _renderMesh)
#define get_polyForValue(_value, _accessType, _owner, _fn) _get_polyForValue(_value, _accessType, _owner, #_fn##)

// valid values for parameter selTypesAllowed to ValueToBitArray, ValueToBitArrayM
#define MESH_VERTSEL_ALLOWED 1
#define MESH_FACESEL_ALLOWED 2
#define MESH_EDGESEL_ALLOWED 3

// valid values for parameter selTypesAllowed to ValueToBitArrayP
#define POLY_VERTS 1
#define POLY_FACES 2
#define POLY_EDGES 3

#define	AUTOEDGE_SETCLEAR	0
#define	AUTOEDGE_SET		1
#define	AUTOEDGE_CLEAR		2

#define EDITTRIOBJ_CLASSID GetEditTriObjDesc()->ClassID()
#define TRIOBJ_CLASSID Class_ID(TRIOBJ_CLASS_ID, 0)

struct IndexedPoint3SortVal {int index; Point3 val;};
extern int IndexedPoint3SortCmp(const void *elem1, const void *elem2 );

// following def also in common_funcs.cpp for use by ValueToBitArray()
// following only valid for integer and float range checking
#define range_check(_val, _lowerLimit, _upperLimit, _desc)					\
	if (_val < _lowerLimit || _val > _upperLimit) {							\
		TCHAR buf[256];														\
		TCHAR buf2[128];													\
		strcpy(buf,_desc);													\
		strcat(buf,_T(" < "));												\
		_snprintf(buf2, 128, _T("%g"), (double)EPS(_lowerLimit));			\
		strcat(buf,buf2);													\
		strcat(buf,_T(" or > "));											\
		_snprintf(buf2, 128, _T("%g"), (double)EPS(_upperLimit));			\
		strcat(buf,buf2);													\
		strcat(buf,_T(": "));												\
		_snprintf(buf2, 128, _T("%g"), (double)EPS(_val));					\
		strcat(buf,buf2);													\
		throw RuntimeError (buf);											\
	}

inline	BOOL StrEqual(TCHAR* str1, TCHAR* str2) { 
			return (_tcsicmp(str1, str2) == 0); }

#define redraw_views()																\
			MAXScript_interface->RedrawViews(MAXScript_interface->GetTime())

#define get_valid_node(_node, _fn) _get_valid_node((MAXNode*)_node, #_fn##" requires a node")

#define elements(array)			(sizeof(array)/sizeof((array)[0]))
#define def_not_implemented()	throw RuntimeError (GetString(IDS_RK_FN_NOT_YET_IMPLEMENTED))

// following are for methods defined in W2K, but not NT
typedef BOOL (WINAPI *transparentBlt)(HDC hdcDest,int nXOriginDest,int nYOriginDest,
     int nWidthDest,int hHeightDest,HDC hdcSrc,int nXOriginSrc,int nYOriginSrc,
	 int nWidthSrc,int nHeightSrc,UINT crTransparent);

typedef BOOL (WINAPI *alphaBlend)(HDC hdcDest,int nXOriginDest,int nYOriginDest,
     int nWidthDest,int hHeightDest,HDC hdcSrc,int nXOriginSrc,int nYOriginSrc,
     int nWidthSrc,int nHeightSrc,BLENDFUNCTION blendFunction);

#endif //__MXSAGNI__H__

