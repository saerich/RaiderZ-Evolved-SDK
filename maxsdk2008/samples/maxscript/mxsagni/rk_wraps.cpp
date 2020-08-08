/**********************************************************************
 *<
	FILE: rk_tclasses.cpp

	DESCRIPTION: Contains the Implementation of the various types and the root class

	CREATED BY: Ravi Karra

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "MAXScrpt.h"
#include "Numbers.h"
#include "Strings.h"

#include "MAXMats.h"
#include "ColorVal.h"

#include "resource.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "MXSAgni.h"
#include "ExtClass.h"

#include "defextfn.h"
#	include "namedefs.h"

#include "definsfn.h"
#	include	"rk_wraps.h"

#include "agnidefs.h"

void 
DebugPrint(CharStream* out, const Matrix3& m3)
{
	for (int rn = 0; rn < 4; rn++)
	{
		Point3 p3 = m3.GetRow(rn);
		out->printf(_T("| %.2f\t %.2f\t %.2f |\n"), 
					p3.x, p3.y, p3.z );
	}
}

void 
DebugPrint(CharStream* out, const Point3& p3)
{
	out->printf(_T("[ %.2f, %.2f, %.2f ]\n"), 
					p3.x, p3.y, p3.z );
}

void 
DebugPrint(CharStream* out, const Point2& p2)
{
	out->printf(_T("[ %.2f, %.2f]\n"), 
					p2.x, p2.y);
}

/*--------------------------------- TestInterface-----------------------------------*/


Value*
CreatePreview_cf(Value** arg_list, int count)
{
	check_arg_count(CreatePreview, 0, count);
	MAXScript_interface->CreatePreview();
	return &ok;
}

Value*  
GetMatLibFileName_cf(Value** arg_list, int count)
{
	check_arg_count(GetMatLibFileName, 0, count); 
	return new String(MAXScript_interface->GetMatLibFileName()); 	
}

Value* 
SetSysCur_cf(Value** arg_list, int count)
{
	check_arg_count(SetSysCur, 1, count);
	def_cursor_types();
	SetCursor(MAXScript_interface->GetSysCursor(
		GetID(cursorTypes, elements(cursorTypes), arg_list[0])));
	return &ok;
}

//More
Value*  
GetSnapState_cf(Value** arg_list, int count)
{
	check_arg_count(GetSnapState, 0, count); 
	return MAXScript_interface->GetSnapState() ? &true_value : &false_value; 
}

Value*  
GetSnapMode_cf(Value** arg_list, int count)
{
	check_arg_count(GetSnapMode, 0, count); 
	return new String(MAXScript_interface->GetSnapMode() ? _T("ABSOLUTE") : _T("RELATIVE"));
}

Value*  
GetCrossing_cf(Value** arg_list, int count)
{
	check_arg_count(GetCrossing, 0, count); 
	return MAXScript_interface->GetCrossing() ? &true_value : &false_value; 
}

Value* 
SetToolBtnState_cf(Value** arg_list, int count)
{
	check_arg_count(SetToolBtnState, 2, count);
	def_toolbtn_types();
	MAXScript_interface->SetToolButtonState(
			GetID(toolbtnTypes, elements(toolbtnTypes), arg_list[0]), 
			arg_list[1]->to_bool());
	return &ok;
}

//More

Value* 
GetCoordCenter_cf(Value** arg_list, int count)
{
	check_arg_count(GetCoordCenter, 0, count);
	def_origin_types();	
	int coord = MAXScript_interface->GetCoordCenter();
	return GetName(originTypes, elements(originTypes), coord);	
}

Value* 
SetCoordCenter_cf(Value** arg_list, int count)
{
	check_arg_count(SetCoordCenter, 1, count);
	def_origin_types();
	MAXScript_interface->SetCoordCenter(
		GetID(originTypes, elements(originTypes), arg_list[0]));	
	return &ok;
}

Value* 
EnableUndo_cf(Value** arg_list, int count)
{
	check_arg_count(EnableUndo, 1, count); 
	MAXScript_interface->EnableUndo(arg_list[0]->to_bool()); 
	return arg_list[0];
}

Value* 
EnableCoordCenter_cf(Value** arg_list, int count)
{
	check_arg_count(EnableCoordCenter, 1, count); 
	MAXScript_interface->EnableCoordCenter(arg_list[0]->to_bool()); 
	return arg_list[0];
}

Value* 
GetRefCoordSys_cf(Value** arg_list, int count)
{
	check_arg_count(GetRefCoordSys, 0, count);
	def_coord_types();
	int coord = MAXScript_interface->GetRefCoordSys();
	return GetName(coordTypes, elements(coordTypes), coord);	
}

Value* 
SetRefCoordSys_cf(Value** arg_list, int count)
{
	check_arg_count(SetRefCoordSys, 1, count);
	def_coord_types();	
	MAXScript_interface->SetRefCoordSys(
		GetID(coordTypes, elements(coordTypes), arg_list[0]));	
	return 	arg_list[0];	
}

Value*  
EnableRefCoordSys_cf(Value** arg_list, int count)
{
	check_arg_count(EnableRefCoordSys, 1, count); 
	MAXScript_interface->EnableRefCoordSys(arg_list[0]->to_bool());
	return arg_list[0];
}

Value*
GetNumAxis_cf(Value** arg_list, int count)
{
	check_arg_count(GetNumAxis, 0, count);
	def_axis_types();
	return GetName(axisTypes, elements(axisTypes), MAXScript_interface->GetNumAxis());
}

Value*  
LockAxisTripods_cf(Value** arg_list, int count)
{
	check_arg_count(LockAxisTripods, 1, count); 
	MAXScript_interface->LockAxisTripods(arg_list[0]->to_bool()); 
	needs_redraw_set();
	return arg_list[0];
}

Value*  
AxisTripodLocked_cf(Value** arg_list, int count)
{
	check_arg_count(AxisTripodLocked, 0, count); 
	return MAXScript_interface->AxisTripodLocked() ? &true_value : &false_value; 
}

Value* 
CompleteRedraw_cf(Value** arg_list, int count)
{
	check_arg_count(CompleteRedraw, 0, count);
	MAXScript_interface->ForceCompleteRedraw(); 
	return &ok;
}

Value* 
ExclusionListDlg_cf(Value** arg_list, int count)
{
	check_arg_count(ExclusionListDlg, 0, count);

	ExclList nl; // DS 8/31/00 - change from NameTab
	MAXScript_interface->DoExclusionListDialog(&nl);
	
	one_typed_value_local(Array* result); 
	vl.result = new Array(nl.Count());
	for(int n = 0; n < nl.Count(); n++)
		vl.result->append(MAXNode::intern(nl[n]));
	return_value(vl.result);
}

Value* 
MaterialBrowseDlg_cf(Value** arg_list, int count)
{
	//MaterialBrowseDlg() [#matsOnly | #mapsOnly | #incNone | #instanceOnly]
	def_browse_types();
	BOOL newMat = FALSE, cancel = FALSE;	
	DWORD flags=0;
	for (int i=0; i < count; i++)
	  flags |= GetID(browseTypes, elements(browseTypes), arg_list[i]);
	MtlBase *mbase = MAXScript_interface->DoMaterialBrowseDlg(
		MAXScript_interface->GetMAXHWnd(), flags, newMat, cancel);
	
	if(!cancel) 
	{
		return MAXClass::make_wrapper_for(mbase);
/*		if (mbase->SuperClassID() == TEXMAP_CLASS_ID)
			return MAXTexture::intern((Texmap*)mbase);
		else if (mbase->SuperClassID() == MATERIAL_CLASS_ID)
			return MAXMaterial::intern((Mtl*)mbase);
*/
	}
	return &undefined;	
}

Value*  
HitByNameDlg_cf(Value** arg_list, int count)
{
	check_arg_count(HitByNameDlg, 0, count); 
	return MAXScript_interface->DoHitByNameDialog() ? &true_value : &false_value; 
}

Value* 
NodeColorPicker_cf(Value** arg_list, int count)
{
	check_arg_count(NodeColorPicker, 0, count);
	DWORD col;
	if (MAXScript_interface->NodeColorPicker(MAXScript_interface->GetMAXHWnd(), col))
		return new ColorValue(col);
	return &undefined;	
}

Value*  
FileOpenMatLib_cf(Value** arg_list, int count)
{
	check_arg_count(FileOpenMatLib, 0, count); 
	HoldSuspend hs (theHold.Holding()); // LAM - 2/22/01
	MAXScript_interface->FileOpenMatLib(MAXScript_interface->GetMAXHWnd());
	hs.Resume();
	return &ok;	
}

#ifndef NO_MATLIB_SAVING // orb 01-09-2002
#ifndef USE_CUSTOM_MATNAV // orb 08-23-2001 removing matlib saving
Value*  
FileSaveMatLib_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(FileSaveMatLib, 0, count); 
	HoldSuspend hs; // LAM - 2/22/01
	Value* v;
	BOOL quiet = bool_key_arg(quiet, v, MAXScript_interface->GetQuietMode());
	TempQuietMode tmpQuiet (quiet);

	MAXScript_interface->FileSaveMatLib(MAXScript_interface->GetMAXHWnd()); 

	return &ok;
}

Value*  
FileSaveAsMatLib_cf(Value** arg_list, int count)
{
	check_arg_count(FileSaveAsMatLib, 0, count); 
	HoldSuspend hs (theHold.Holding()); // LAM - 2/22/01
	MAXScript_interface->FileSaveAsMatLib(MAXScript_interface->GetMAXHWnd()); 
	hs.Resume();
	return &ok;
}
#endif // USE_CUSTOM_MATNAV
#endif // NO_MATLIB_SAVING 

Value*  
LoadDefaultMatLib_cf(Value** arg_list, int count)
{
	check_arg_count(LoadDefaultMatLib, 0, count); 
	HoldSuspend hs (theHold.Holding()); // LAM - 2/22/01
	MAXScript_interface->LoadDefaultMatLib(); 
	hs.Resume();
	return &ok;
}


//Status Panel Prompt Methods:
Value*  
PushPrompt_cf(Value** arg_list, int count)
{
	check_arg_count(PushPrompt, 1, count); 
	MAXScript_interface->PushPrompt(arg_list[0]->to_string()); 
	return &ok;
}

Value*  
PopPrompt_cf(Value** arg_list, int count)
{
	check_arg_count(PopPrompt, 0, count); 
	MAXScript_interface->PopPrompt(); 
	return &ok;
}

Value*  
ReplacePrompt_cf(Value** arg_list, int count)
{
	check_arg_count(ReplacePrompt, 1, count); 
	MAXScript_interface->ReplacePrompt(arg_list[0]->to_string()); 
	return &ok;
}

Value*  
DisplayTempPrompt_cf(Value** arg_list, int count)
{
	check_arg_count(DisplayTempPrompt, 2, count); 
	MAXScript_interface->DisplayTempPrompt(
		arg_list[0]->to_string(), 
		arg_list[1]->to_int()); 
	return &ok;
}  

Value*  
RemoveTempPrompt_cf(Value** arg_list, int count)
{
	check_arg_count(RemoveTempPrompt, 0, count); 
	MAXScript_interface->RemoveTempPrompt(); 
	return &ok;
}

//Progress bars are handled by Scripter

Value*  
DisableStatusXYZ_cf(Value** arg_list, int count)
{
	check_arg_count(DisableStatusXYZ, 0, count); 
	MAXScript_interface->DisableStatusXYZ(); 
	return &ok;
}

Value*  
EnableStatusXYZ_cf(Value** arg_list, int count)
{
	check_arg_count(EnableStatusXYZ, 0, count); 
	MAXScript_interface->EnableStatusXYZ(); 
	return &ok;
}

Value*
SetStatusXYZ_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(SetStatusXYZ, 2, count); 
	def_status_types();
	MAXScript_interface->SetStatusXYZ(
		arg_list[1]->to_point3(), 
		GetID(statusTypes, elements(statusTypes), arg_list[0]));
	return &ok;	
}

//SetStatusAng_cf(Value** arg_list, int count){ MAXScript_interface->SetStatusAng(); }

Value*  
GetGridSpacing_cf(Value** arg_list, int count)
{
	check_arg_count(GetGridSpacing, 0, count); 
	return Float::intern(MAXScript_interface->GetGridSpacing()); 	
}

Value*  
GetGridMajorLines_cf(Value** arg_list, int count)
{
	check_arg_count(GetGridMajorLines, 0, count); 
	return Integer::intern(MAXScript_interface->GetGridMajorLines()); 	
}

//Modifier related methods
Value*  
EnableShowEndRes_cf(Value** arg_list, int count)
{
	check_arg_count(EnableShowEndRes, 1, count); 
	MAXScript_interface->EnableShowEndResult(arg_list[0]->to_bool()); 
	return arg_list[0];
}

//Later -> Value* RegSubObjectTypes_cf(Value** arg_list, int count)

Value*  
AppendSubSelSet_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(AppendSubSelSet, 1, count); 
	MAXScript_interface->AppendSubObjectNamedSelSet(arg_list[0]->to_string()); 
	return &ok;
}

Value*  
ClearSubSelSets_cf(Value** arg_list, int count)
{
	check_arg_count(ClearSubSelSets, 0, count); 
	MAXScript_interface->ClearSubObjectNamedSelSets(); 
	return &ok;
}

Value*  
ClearCurSelSet_cf(Value** arg_list, int count)
{
	check_arg_count(ClearCurSelSet, 0, count);
	MAXScript_interface->ClearCurNamedSelSet(); 
	return &ok;
}

Value*  
EnableSubObjSel_cf(Value** arg_list, int count)
{
	check_arg_count(EnableSubObjSel, 1, count); 
	MAXScript_interface->EnableSubObjectSelection(arg_list[0]->to_bool()); 
	return arg_list[0];
}

Value*  
IsSubSelEnabled_cf(Value** arg_list, int count)
{
	check_arg_count(IsSubSelEnabled, 0, count);  
	return MAXScript_interface->IsSubObjectSelectionEnabled() ? &true_value : &false_value;	
}

Value*  
IsCPEdgeOnInView_cf(Value** arg_list, int count)
{
	check_arg_count(IsCPEdgeOnInView, 0, count);  
	return MAXScript_interface->IsCPEdgeOnInView() ? &true_value : &false_value; 
}

Value* 
NodeInvalRect_cf(Value** arg_list, int count)
{
	check_arg_count(NodeInvalRect, 1, count);
	INode* node = arg_list[0]->to_node();
	InvalidateNodeRect(node,MAXScript_time());
	return &ok;
}

//More directory stufff

Value* 
SetVPortBGColor_cf(Value** arg_list, int count)
{
	check_arg_count(SetVPortBGColor, 1, count);
	MAXScript_interface->SetViewportBGColor(arg_list[0]->to_point3()/255.0f); 
	needs_redraw_set();
	return arg_list[0];
}

Value* 
GetVPortBGColor_cf(Value** arg_list, int count)
{
	check_arg_count(GetVPortBGColor, 0, count); 
	return new ColorValue(MAXScript_interface->GetViewportBGColor() * 255.0f ); 	
}

Value*
SetBkgImageAspect_cf(Value** arg_list, int count)
{
	check_arg_count(SetBkgImageAspect, 1, count);
	def_bkgaspect_types();
	MAXScript_interface->setBkgImageAspect(
		GetID(bkgaspectTypes, elements(bkgaspectTypes), arg_list[0]));
	return &ok;	
}

Value*
GetBkgImageAspect_cf(Value** arg_list, int count)
{
	check_arg_count(GetBkgImageAspect, 0, count);
	def_bkgaspect_types();	
	int bi  = MAXScript_interface->getBkgImageAspect();
	return GetName(bkgaspectTypes, elements(bkgaspectTypes), bi);	
}

Value*
SetBkgImageAnimate_cf(Value** arg_list, int count)
{
	check_arg_count(SetBkgImageAnimate, 1, count);
	MAXScript_interface->setBkgImageAnimate(arg_list[0]->to_bool());
	return arg_list[0];
}

Value*
GetBkgImageAnimate_cf(Value** arg_list, int count)
{
	check_arg_count(GetBkgImageAnimate, 0, count);
	return MAXScript_interface->getBkgImageAnimate() ? &true_value : &false_value;	
}

Value*
SetBkgFrameRange_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(SetBkgFrameRange, 1, count);
	Point3 p = arg_list[0]->to_point3(); 
	MAXScript_interface->setBkgFrameRange((int)p.x, (int)p.y, (int)p.z);
	return &ok;
}

Value*
GetBkgRangeVal_cf(Value** arg_list, int count)
{
	check_arg_count(GetBkgRangeVal, 0, count);
	return new Point2Value(
		MAXScript_interface->getBkgFrameRangeVal(VIEWPORT_BKG_START),
		MAXScript_interface->getBkgFrameRangeVal(VIEWPORT_BKG_END));
}

Value*
SetBkgORType_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(SetBkgORType, 2, count);
	def_bkgor_types();	
	MAXScript_interface->setBkgORType(arg_list[0]->to_int(), 
		GetID(bkgorTypes, elements(bkgorTypes), arg_list[1]));
	return &ok;	
}

Value*
GetBkgORType_cf(Value** arg_list, int count)
{
	check_arg_count(GetBkgORType, 1, count);
	def_bkgor_types();
	int val = MAXScript_interface->getBkgORType(arg_list[0]->to_int());
	return GetName(bkgorTypes, elements(bkgorTypes), val);
}

Value*
SetBkgStartTime_cf(Value** arg_list, int count)
{
	check_arg_count(SetBkgStartTime, 1, count);
	MAXScript_interface->setBkgStartTime(arg_list[0]->to_timevalue());
	return arg_list[0];
}

Value*
GetBkgStartTime_cf(Value** arg_list, int count)
{
	check_arg_count(GetBkgStartTime, 0, count);
	return MSTime::intern(MAXScript_interface->getBkgStartTime());
}

Value*
SetBkgSyncFrame_cf(Value** arg_list, int count)
{
	check_arg_count(SetBkgSyncFrame, 1, count);
	MAXScript_interface->setBkgSyncFrame(arg_list[0]->to_int());
	return arg_list[0];	
}

Value*
GetBkgSyncFrame_cf(Value** arg_list, int count)
{
	check_arg_count(GetBkgSyncFrame, 0, count);
	return Integer::intern(MAXScript_interface->getBkgSyncFrame());	
}

Value*
GetBkgFrameNum_cf(Value** arg_list, int count)
{
	check_arg_count(GetBkgFrameNum, 1, count);
	return Integer::intern(MAXScript_interface->getBkgFrameNum(arg_list[0]->to_timevalue()));
}

Value*
SetUseEnvironmentMap_cf(Value** arg_list, int count)
{
	check_arg_count(SetUseEnvironmentMap, 1, count);
	MAXScript_interface->SetUseEnvironmentMap(arg_list[0]->to_bool());
	return arg_list[0];
}

Value*
GetUseEnvironmentMap_cf(Value** arg_list, int count)
{
	check_arg_count(GetUseEnvironmentMap, 0, count);
	return MAXScript_interface->GetUseEnvironmentMap() ? &true_value : &false_value;
}

//----------------2.0 & Later
Value*
DeSelectNode_cf(Value** arg_list, int count)
{
	check_arg_count(DeSelectNode, 1, count);	
	MAXScript_interface->DeSelectNode(arg_list[0]->to_node());
	return &ok;
}

static Array* get_hit_nodes(Interface *MAXScript_interface, ViewExp *vpt)
{
	one_typed_value_local(Array* result); 
	int count = vpt->HitCount();
	vl.result = new Array(count);
	for(int i = 0; i < count; i++)
		vl.result->append(MAXNode::intern(vpt->GetHit(i))); 
	needs_redraw_set();
	MAXScript_interface->ReleaseViewport(vpt);
	return_value(vl.result);
}

Value*
BoxPickNode_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(BoxPickNode, 1, count);
	Box2 b = arg_list[0]->to_box2();
	IPoint2 points[] = {IPoint2(b.left, b.bottom), IPoint2(b.right, b.top)};
	BOOL doCrossing = key_arg_or_default(crossing, &true_value)->to_bool();
	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	MAXScript_interface->BoxPickNode(
		vpt,
		points,
		doCrossing);
	return get_hit_nodes(MAXScript_interface, vpt);	
}

Value*
CirclePickNode_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(CirclePickNode, 1, count);		
	Box2 b = arg_list[0]->to_box2();
	IPoint2 points[] = {IPoint2(b.left, b.bottom), IPoint2(b.right, b.top)};
	BOOL doCrossing = key_arg_or_default(crossing, &true_value)->to_bool();
	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	MAXScript_interface->CirclePickNode(
		vpt,
		points,
		doCrossing);	
	return get_hit_nodes(MAXScript_interface, vpt);	
}

Value*
FencePickNode_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(FencePickNode, 1, count);	
	type_check(arg_list[0], Array, "FencePickNode");	
	Array* pts =  (Array*)arg_list[0];	
	IPoint2 *points = new IPoint2[pts->size+1];
	BOOL doCrossing = key_arg_or_default(crossing, &true_value)->to_bool();
	int i; 
	for(i=0; i < pts->size; i++)
		points[i] = to_ipoint2(pts->data[i]);
	// Add the end of list signal
	points[i] = IPoint2(-100,-100);
	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	MAXScript_interface->FencePickNode(
		vpt,
		points,
		doCrossing);	
	delete [] points;
	return get_hit_nodes(MAXScript_interface, vpt);	
}

Value*
FlashNodes_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(FlashNodes, 1, count);
	INodeTab flash;
	type_check(arg_list[0], Array, "FlashNodes");	
	Array* nodes =  (Array*)arg_list[0];
	for (int i=0; i<nodes->size; i++) {
		INode *node = nodes->data[i]->to_node();
		flash.Append(1, &node,10);
	}
	MAXScript_interface->FlashNodes(&flash);
	return &ok;
}

Value*
SetUseDraftRenderer_cf(Value** arg_list, int count)
{
	check_arg_count(SetUseDraftRenderer, 1, count);
	MAXScript_interface->SetUseDraftRenderer(arg_list[0]->to_bool());
	return &ok;
}

Value*
GetUseDraftRenderer_cf(Value** arg_list, int count)
{
	check_arg_count(GetUseDraftRenderer, 0, count);
	return MAXScript_interface->GetUseDraftRenderer() ? &true_value : &false_value;
}

Value*
ClearNodeSelection_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(ClearNodeSelection, 0, count);
	MAXScript_interface->ClearNodeSelection(key_arg_or_default(redraw, &true_value)->to_bool());
	return &ok;	
}

Value*
DisableSceneRedraw_cf(Value** arg_list, int count)
{
	check_arg_count(DisableSceneRedraw, 0, count);
	MAXScript_interface->DisableSceneRedraw();
	return &ok;
}

Value*
EnableSceneRedraw_cf(Value** arg_list, int count)
{
	check_arg_count(EnableSceneRedraw, 0, count);
	MAXScript_interface->EnableSceneRedraw();
	return &ok;
}

Value*
IsSceneRedrawDisabled_cf(Value** arg_list, int count)
{
	check_arg_count(IsSceneRedrawDisabled, 0, count);
	return MAXScript_interface->IsSceneRedrawDisabled() ? &true_value : &false_value;
	return &ok;
}

Value*
GetKeyStepsPos_cf(Value** arg_list, int count)
{
	check_arg_count(GetKeyStepsPos, 0, count);
	return MAXScript_interface->GetKeyStepsPos() ? &true_value : &false_value;
}

Value*
SetKeyStepsPos_cf(Value** arg_list, int count)
{
	check_arg_count(SetKeyStepsPos, 1, count);
	MAXScript_interface->SetKeyStepsPos(arg_list[0]->to_bool());
	return arg_list[0];
}

Value*
GetKeyStepsRot_cf(Value** arg_list, int count)
{
	check_arg_count(GetKeyStepsRot, 0, count);
	return MAXScript_interface->GetKeyStepsRot() ? &true_value : &false_value;
}

Value*
SetKeyStepsRot_cf(Value** arg_list, int count)
{
	check_arg_count(SetKeyStepsRot, 1, count);
	MAXScript_interface->SetKeyStepsRot(arg_list[0]->to_bool());
	return arg_list[0];
}

Value*
GetKeyStepsScale_cf(Value** arg_list, int count)
{
	check_arg_count(GetKeyStepsScale, 0, count);
	return MAXScript_interface->GetKeyStepsScale() ? &true_value : &false_value;
}

Value*
SetKeyStepsScale_cf(Value** arg_list, int count)
{
	check_arg_count(SetKeyStepsScale, 1, count);
	MAXScript_interface->SetKeyStepsScale(arg_list[0]->to_bool());
	return arg_list[0];	
}

Value*
GetKeyStepsSelOnly_cf(Value** arg_list, int count)
{
	check_arg_count(GetKeyStepsSelOnly, 0, count);
	return MAXScript_interface->GetKeyStepsSelOnly() ? &true_value : &false_value;
}

Value*
SetKeyStepsSelOnly_cf(Value** arg_list, int count)
{
	check_arg_count(SetKeyStepsSelOnly, 1, count);
	MAXScript_interface->SetKeyStepsSelOnly(arg_list[0]->to_bool());
	return arg_list[0];	
}

Value*
GetKeyStepsUseTrans_cf(Value** arg_list, int count)
{
	check_arg_count(GetKeyStepsUseTrans, 0, count);
	return MAXScript_interface->GetKeyStepsUseTrans() ? &true_value : &false_value;
}

Value*
SetKeyStepsUseTrans_cf(Value** arg_list, int count)
{
	check_arg_count(SetKeyStepsUseTrans, 1, count);
	MAXScript_interface->SetKeyStepsUseTrans(arg_list[0]->to_bool());
	return arg_list[0];
}

Value*
AssignNewName_cf(Value** arg_list, int count)
{
	check_arg_count(AssignNewName, 1, count);
	if (arg_list[0]->is_kind_of(class_tag(MAXMaterial)))
		MAXScript_interface->AssignNewName(arg_list[0]->to_mtl());
	else if (arg_list[0]->tag == class_tag(MAXTexture))
		MAXScript_interface->AssignNewName(arg_list[0]->to_texmap());
	else
		throw TypeError (GetString(IDS_ASSIGNNEWNAME_NEEDS_MAT_TEX), arg_list[0]);
	return &ok;
}

Value*
ForceCompleteRedraw_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(ForceCompleteRedraw, 0, count);
	MAXScript_interface->ForceCompleteRedraw(key_arg_or_default(doDisabled, &true_value)->to_bool());
	return &ok;
}

Value*
GetBackGroundController_cf(Value** arg_list, int count)
{
	check_arg_count(GetBackGroundController, 0, count);
	return MAXControl::intern(MAXScript_interface->GetBackGroundController());
}

Value*
GetCommandPanelTaskMode_cf(Value** arg_list, int count)
{
	check_arg_count(GetCommandPanelTaskMode, 0, count);	
	def_cpanel_types();
	return GetName(cpanelTypes, elements(cpanelTypes), 
		MAXScript_interface->GetCommandPanelTaskMode());
}

Value*
GetNamedSelSetItem_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(GetNamedSelSetItem, 2, count);
	int set_ind = arg_list[0]->to_int();
	int item_ind = arg_list[1]->to_int();
	
	if (set_ind < 1 || set_ind > MAXScript_interface->GetNumNamedSelSets())
		throw RuntimeError(GetString(IDS_RK_GETNAMEDSELSETITEM_SET_INDEX_OUT_OF_RANGE));		
	if (item_ind < 1 || item_ind > MAXScript_interface->GetNamedSelSetItemCount(set_ind-1)) 
		throw RuntimeError(GetString(IDS_RK_GETNAMEDSELSETITEM_ITEM_INDEX_OUT_OF_RANGE));
	return MAXNode::intern(MAXScript_interface->GetNamedSelSetItem(set_ind-1, item_ind-1));
}

Value*
GetNamedSelSetItemCount_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(GetNamedSelSetItemCount, 1, count);
	int set_ind = arg_list[0]->to_int();
	if (set_ind < 1 || set_ind > MAXScript_interface->GetNumNamedSelSets())
		throw RuntimeError(GetString(IDS_RK_GETNAMEDSELSETITEMCOUNT_SET_INDEX_OUT_OF_RANGE));		
	return Integer::intern(MAXScript_interface->GetNamedSelSetItemCount(set_ind-1));
}

Value*
GetNamedSelSetName_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(GetNamedSelSetName, 1, count);
	int set_ind = arg_list[0]->to_int();
	if (set_ind < 1 || set_ind > MAXScript_interface->GetNumNamedSelSets())
		throw RuntimeError(GetString(IDS_RK_GETNAMEDSELSETNAME_SET_INDEX_OUT_OF_RANGE));			
	return new String(MAXScript_interface->GetNamedSelSetName(set_ind-1));	
}

Value*
GetNumNamedSelSets_cf(Value** arg_list, int count)
{
	check_arg_count(GetNumNamedSelSets, 0, count);
	return Integer::intern(MAXScript_interface->GetNumNamedSelSets());	
}

Value*
GetRendApertureWidth_cf(Value** arg_list, int count)
{
	check_arg_count(GetRendApertureWidth, 0, count);
	return Float::intern(MAXScript_interface->GetRendApertureWidth());
}

Value*
SetRendApertureWidth_cf(Value** arg_list, int count)
{
	check_arg_count(SetRendApertureWidth, 1, count);
	MAXScript_interface->SetRendApertureWidth(arg_list[0]->to_float());
	return arg_list[0];
}

Value*
GetRendImageAspect_cf(Value** arg_list, int count)
{
	check_arg_count(GetRendImageAspect, 0, count);
	return Float::intern(MAXScript_interface->GetRendImageAspect());
}

Value*
NamedSelSetListChanged_cf(Value** arg_list, int count)
{
	check_arg_count(NamedSelSetListChanged, 0, count);
	MAXScript_interface->NamedSelSetListChanged();
	return &ok;
}

Value*
OkMtlForScene_cf(Value** arg_list, int count)
{
	check_arg_count(OkMtlForScene, 1, count);
	return MAXScript_interface->OkMtlForScene( arg_list[0]->to_mtlbase() ) ? &true_value : &false_value;	
}

Value*
RescaleWorldUnits_cf(Value** arg_list, int count)
{
	if (count < 1)
		throw ArgCountError (_T("rescaleWorldUnits "), 1, count);
	MAXScript_interface->RescaleWorldUnits(
		arg_list[0]->to_float(), 
		arg_list[count-1] == n_selOnly );
	needs_complete_redraw_set();
	return &ok;
}

Value*
SetBackGround_cf(Value** arg_list, int count)
{
	if (count < 1 || count > 2)
		throw ArgCountError (_T("SetBackGround "), 2, count);
	Point3 color;
	TimeValue time;
	if (count == 2)
	{	color = arg_list[1]->to_point3()/255.0f;
		time = arg_list[0]->to_timevalue();
	}
	else
	{
		color = arg_list[0]->to_point3()/255.0f;
		time = MAXScript_time();
	}
	MAXScript_interface->SetBackGround(time, color);
	return &ok;
}

Value*
GetBackGround_cf(Value** arg_list, int count)
{
	if (count > 1)
		throw ArgCountError (_T("GetBackGround "), 1, count);
	TimeValue time;
	if (count == 1)
		time = arg_list[0]->to_timevalue();
	else
		time = MAXScript_time();
	Interval valid  = FOREVER;
	return new ColorValue(MAXScript_interface->GetBackGround(time, valid) * 255.0f );
}

Value*
SetBackGroundController_cf(Value** arg_list, int count)
{
	check_arg_count(SetBackGroundController, 1, count);
	MAXScript_interface->SetBackGroundController(arg_list[0]->to_controller());
	return &ok;
}

Value*
SetCommandPanelTaskMode_cf(Value** arg_list, int count)
{
	Value *mode;
	if (count == 1) 
		mode = arg_list[0];
	else
	{
		check_arg_count_with_keys(SetCommandPanelTaskMode, 0, count);
		mode = key_arg(mode);
	}
	def_cpanel_types();	
	int imode = GetID(cpanelTypes, elements(cpanelTypes), mode);
	MAXScript_interface->SetCommandPanelTaskMode(imode);
	return &ok;
}

Value*
SetCurNamedSelSet_cf(Value** arg_list, int count)
{
	check_arg_count(SetCurNamedSelSet, 1, count);
	MAXScript_interface->SetCurNamedSelSet(arg_list[0]->to_string());
	return &ok;
}

Value*
StopCreating_cf(Value** arg_list, int count)
{
	check_arg_count(StopCreating, 0, count);
	MAXScript_interface->StopCreating();
	return &ok;
}

/*-----------------------------------TestBitmapManager-------------------------------------------*/
Value* 
SilentMode_cf(Value** arg_list, int count)
{
	check_arg_count(SilentMode, 0, count);
	return TheManager->SilentMode() ? &true_value : &false_value;	
}

Value* 
SetSilentMode_cf(Value** arg_list, int count)
{
	check_arg_count(SetSilentMode, 1, count);
	return TheManager->SetSilentMode(arg_list[0]->to_bool()) ? &true_value : &false_value;
}

Value* 
ConvertDirIDToInt_cf(Value** arg_list, int count)
{
	check_arg_count(ConvertDirIDToInt, 1, count);
	def_dir_types();
	int dir = GetID(dirTypes, elements(dirTypes), arg_list[0]);		
	return Integer::intern(dir);
}

Value* 
GetDir_cf(Value** arg_list, int count)
{
	check_arg_count(GetDir, 1, count);
	def_dir_types();
	int dir = GetID(dirTypes, elements(dirTypes), arg_list[0]);		
	return new String(TheManager->GetDir(dir));	
}

Value* 
SetDir_cf(Value** arg_list, int count)
{
	check_arg_count(GetDir, 2, count);
	def_dir_types();
	int dir = GetID(dirTypes, elements(dirTypes), arg_list[0]);
	// russom - 05/31/01
	// Limit string size
#ifdef USE_GMAX_SECURITY
	TCHAR *szPath = arg_list[1]->to_string();
	if( szPath ) {
		if( _tcslen( szPath ) > MAX_PATH )
			szPath[MAX_PATH] = _T('\0');
	}
	BOOL res = MAXScript_interface7->SetDir(dir, expand_file_name(szPath));
#else
	BOOL res = MAXScript_interface7->SetDir(dir, arg_list[1]->to_filename());
#endif
	return res ? &true_value : &false_value;	
}

Value*
SelectSaveBitMap_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(SelectSaveBitMap, 0, count);
	BitmapInfo bi;
	Value* cap = key_arg(caption);
	Value* filename = key_arg(filename);
	if (filename != &unsupplied)
		bi.SetName(filename->to_filename());
	TCHAR*  cap_str = (cap != &unsupplied) ? cap->to_string() : NULL ;
	if (TheManager->SelectFileOutput(&bi, GetActiveWindow(), cap_str))
		return new String((TCHAR*)bi.Name());
	else
		return &undefined;
}

Value*
loadDllsFromDir_cf(Value** arg_list, int count)
{
	check_arg_count(loadDllsFromDir, 2, count);
	TSTR  dir = arg_list[0]->to_filename();
	if (dir.last(_T('\\')) != dir.Length()-1 && dir.last(_T('/')) != dir.Length()-1)
		dir += TSTR(_T("\\"));
	int res = MAXScript_interface->GetDllDir().LoadDllsFromDir(
					dir , arg_list[1]->to_string());
	if (res != 0) {
		MAXScript_interface7->ReBuildSuperList();
		return &true_value;
	}
	return &false_value; 
}

/*-----------------------------------TestINode------------------------------------------*/
Value* 
GetCVertMode_cf(Value** arg_list, int count)
{
	check_arg_count(GetCVertMode, 1, count);
	INode *node = arg_list[0]->to_node();
	return node->GetCVertMode() ? &true_value : &false_value;
}

Value* 
GetImageBlurMultController_cf(Value** arg_list, int count)
{
	check_arg_count(GetImageBlurMultController, 1, count);
	INode *node = arg_list[0]->to_node();
	return MAXClass::make_wrapper_for(node->GetImageBlurMultController());
}


Value* 
GetInheritVisibility_cf(Value** arg_list, int count)
{
	check_arg_count(GetInheritVisibility, 1, count);
	INode *node = arg_list[0]->to_node();
	return node->GetInheritVisibility() ? &true_value : &false_value;
}

Value* 
GetPosTaskWeight_cf(Value** arg_list, int count)
{
	check_arg_count(GetPosTaskWeight, 1, count);
	INode *node = arg_list[0]->to_node();
	return Float::intern(node->GetPosTaskWeight());
}

Value* 
GetRenderID_cf(Value** arg_list, int count)
{
	check_arg_count(GetRenderID, 1, count);	
	INode *node = arg_list[0]->to_node();
	return Integer::intern(node->GetRenderID());	
}

Value* 
GetRotTaskWeight_cf(Value** arg_list, int count)
{
	check_arg_count(GetRotTaskWeight, 1, count);
	INode *node = arg_list[0]->to_node();
	return Float::intern(node->GetRotTaskWeight());	
}

Value* 
GetShadeCVerts_cf(Value** arg_list, int count)
{
	check_arg_count(GetShadeCVerts, 1, count);		
	INode *node = arg_list[0]->to_node();
	return node->GetShadeCVerts() ? &true_value : &false_value;		
}

Value* 
GetTaskAxisState_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(GetTaskAxisState, 3, count);		
	INode *node = arg_list[0]->to_node();
	return node->GetTaskAxisState(
		arg_list[1]->to_int(), 
		arg_list[2]->to_int()) ?
		&true_value : &false_value;
}

Value* 
GetTaskAxisStateBits_cf(Value** arg_list, int count)
{
	check_arg_count(GetTaskAxisStateBits, 1, count);		
	INode *node = arg_list[0]->to_node();
	return node->GetTaskAxisStateBits() ? &true_value : &false_value;
	
}

Value* 
GetTrajectoryON_cf(Value** arg_list, int count)
{
	check_arg_count(GetTrajectoryON, 1, count);		
	INode *node = arg_list[0]->to_node();
	return node->GetTrajectoryON() ? &true_value : &false_value;	
}

Value* 
IsBoneOnly_cf(Value** arg_list, int count)
{
	check_arg_count(IsBoneOnly, 1, count);
	INode *node = arg_list[0]->to_node();
	return node->IsBoneOnly() ? &true_value : &false_value;
}


Value* 
SetCVertMode_cf(Value** arg_list, int count)
{
	check_arg_count(SetCVertMode, 2, count);
	INode *node = arg_list[0]->to_node();
	node->SetCVertMode(arg_list[1]->to_bool());
	return &ok;
}

Value* 
SetImageBlurMultController_cf(Value** arg_list, int count)
{
	check_arg_count(SetImageBlurMultController, 2, count);		
	INode *node = arg_list[0]->to_node();
	node->SetImageBlurMultController(arg_list[1]->to_controller());
	return &ok;
}

Value* 
SetImageBlurMultiplier_cf(Value** arg_list, int count)
{
	check_arg_count(SetImageBlurMultiplier, 3, count);
	INode *node = arg_list[0]->to_node();
	node->SetImageBlurMultiplier(arg_list[1]->to_timevalue(), arg_list[2]->to_float());
	return &ok;
}

Value* 
SetInheritVisibility_cf(Value** arg_list, int count)
{
	check_arg_count(SetInheritVisibility, 2, count);		
	INode *node = arg_list[0]->to_node();
	node->SetInheritVisibility(arg_list[1]->to_bool());
	return arg_list[1];
}

Value* 
SetMotBlur_cf(Value** arg_list, int count)
{
	check_arg_count(SetMotBlur, 2, count);			
	INode *node = arg_list[0]->to_node();
	node->SetMotBlur(arg_list[1]->to_int());
	return arg_list[1];
}

Value* 
SetPosTaskWeight_cf(Value** arg_list, int count)
{
	check_arg_count(SetPosTaskWeight, 2, count);		
	INode *node = arg_list[0]->to_node();
	node->SetPosTaskWeight(arg_list[1]->to_float());
	if (node->Selected()) MAXScript_interface->SelectNode(node, 0);
	return arg_list[1];
}

Value* 
SetRenderable_cf(Value** arg_list, int count)
{
	check_arg_count(SetRenderable, 2, count);		
	INode *node = arg_list[0]->to_node();
	node->SetRenderable(arg_list[1]->to_bool());
	if (node->Selected()) MAXScript_interface->SelectNode(node, 0);
	return arg_list[1];
}

Value* 
SetRenderID_cf(Value** arg_list, int count)
{
	check_arg_count(SetRenderID, 2, count);		
	INode *node = arg_list[0]->to_node();
	node->SetRenderID(arg_list[1]->to_int());
	if (node->Selected()) MAXScript_interface->SelectNode(node, 0);
	return arg_list[1];
}

Value* 
SetRotTaskWeight_cf(Value** arg_list, int count)
{
	check_arg_count(SetRotTaskWeight, 2, count);		
	INode *node = arg_list[0]->to_node();
	node->SetRotTaskWeight(arg_list[1]->to_float());
	if (node->Selected()) MAXScript_interface->SelectNode(node, 0);
	return arg_list[1];
}

Value* 
SetShadeCVerts_cf(Value** arg_list, int count)
{
	check_arg_count(SetShadeCVerts, 2, count);		
	INode *node = arg_list[0]->to_node();
	node->SetShadeCVerts(arg_list[1]->to_bool());
	if (node->Selected()) MAXScript_interface->SelectNode(node, 0);
	return arg_list[1];
}

Value* 
SetTaskAxisState_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(SetTaskAxisState, 4, count);		
	INode *node = arg_list[0]->to_node();
	node->SetTaskAxisState(
		arg_list[1]->to_int(),
		arg_list[2]->to_int(),
		arg_list[3]->to_bool()
		);
	if (node->Selected()) MAXScript_interface->SelectNode(node, 0);
	return &ok;
}

Value* 
SetTrajectoryON_cf(Value** arg_list, int count)
{
	check_arg_count(SetTrajectoryON, 2, count);		
	INode *node = arg_list[0]->to_node();
	node->SetTrajectoryON(arg_list[1]->to_bool());
	needs_complete_redraw_set();
	return arg_list[1];
}

Value*
GetTMController_cf(Value** arg_list, int count)
{
	check_arg_count(GetTMController, 1, count);		
	INode *node = arg_list[0]->to_node();
	return MAXControl::intern(node->GetTMController());
}

Value*
GetMasterController_cf(Value** arg_list, int count)
{
	check_arg_count(GetMasterController, 1, count);		
	INode *node = arg_list[0]->to_node();
	Control* tmControl = node->GetTMController();
	if (tmControl) {
		return MAXRefTarg::intern(static_cast<ReferenceTarget*>(tmControl->GetInterface(I_MASTER)));
	}
	return &undefined;
}

Value*
GetVisController_cf(Value** arg_list, int count)
{
	check_arg_count(GetVisController, 1, count);
	Control* ctrl = arg_list[0]->to_node()->GetVisController();
	if (ctrl)
		return MAXControl::intern(ctrl);
	return &undefined;
}

// Groups related functions
Value*
is_group_head_cf(Value** arg_list, int count)
{
	check_arg_count(isGroupHead, 1, count);		
	INode* node = get_valid_node(arg_list[0], isGroupHead);
	return node->IsGroupHead() ? &true_value : &false_value;
}

Value*
is_group_member_cf(Value** arg_list, int count)
{
	check_arg_count(isGroupMember, 1, count);	
	INode* node = get_valid_node(arg_list[0], isGroupMember);
	return node->IsGroupMember() ? &true_value : &false_value;
}

Value*
is_open_group_member_cf(Value** arg_list, int count)
{
	check_arg_count(isOpenGroupMember, 1, count);	
	INode* node = get_valid_node(arg_list[0], isOpenGroupMember);
	return node->IsOpenGroupMember() ? &true_value : &false_value;	
}

Value*
is_open_group_head_cf(Value** arg_list, int count)
{
	check_arg_count(isOpenGroupHead, 1, count);	
	INode* node = get_valid_node(arg_list[0], isOpenGroupHead);
	return node->IsOpenGroupHead() ? &true_value : &false_value;	
}

Value*
set_group_member_cf(Value** arg_list, int count)
{
	check_arg_count(setGroupMember, 2, count);	
	INode* node = get_valid_node(arg_list[0], setGroupMember);
	if (CanChangeGroupFlags(node))
	{
		node->SetGroupMember(arg_list[1]->to_bool());
		needs_redraw_set();
	}
	return arg_list[1];
}

Value*
set_group_head_cf(Value** arg_list, int count)
{
	check_arg_count(setGroupHead, 2, count);	
	INode* node = get_valid_node(arg_list[0], setGroupHead);	
	if (CanChangeGroupFlags(node))
	{
		node->SetGroupHead(arg_list[1]->to_bool());
		needs_redraw_set();
	}
	return arg_list[1];
}

Value*
set_group_open_cf(Value** arg_list, int count)
{
	check_arg_count(setGroupHeadOpen, 2, count);
	INode* node = get_valid_node(arg_list[0], setGroupHeadOpen);	
	BOOL val = arg_list[1]->to_bool();
	if (!node->IsGroupHead())
		throw RuntimeError(GetString(IDS_NOT_VALID_GROUP_HEAD), arg_list[0]);
#if 0
	node->SetGroupHeadOpen(val);
	// Now open all the group members
	for (int i=0; i < node->NumberOfChildren(); i++)
	{
		node->GetChildNode(i)->SetGroupMemberOpen(val);
		InvalidateNodeRect(node->GetChildNode(i),MAXScript_time())
	}
	InvalidateNodeRect(node,MAXScript_time());
#else
	INodeTab nodes;
	nodes.Append(1,&node,5);
	for (int i=0; i < node->NumberOfChildren(); i++) 
	{
		INode *c = node->GetChildNode(i);
		nodes.Append(1,&c,5);
	}
	if (val) 
	{
		MAXScript_interface->OpenGroup(&nodes,FALSE);		
		// To Update the group box
		BOOL sel = node->Selected();
		MAXScript_interface->SelectNode(node,FALSE);
		if (!sel) MAXScript_interface->DeSelectNode(node);
	}
	else 
	{
		MAXScript_interface->CloseGroup(&nodes,FALSE);	
		InvalidateNodeRect(node,MAXScript_time());
	}
#endif	
	needs_redraw_set();
	return arg_list[1];
}

/*
Value*
set_group_member_open_cf(Value** arg_list, int count)
{
	check_arg_count(setGroupMemberOpen, 2, count);
	get_valid_node(arg_list[0], setGroupMemberOpen);
	node->SetGroupMemberOpen(arg_list[1]->to_bool());
	needs_redraw_set();
	return arg_list[1];
}


Value*
CanInstanceController_cf(Value** arg_list, int count)
{
	check_arg_count(CanInstanceController, 1, count);
	Control *c = arg_list[0]->to_controller();	
	return c->CanInstanceController() ? &true_value : &false_value;	
}

Value*
CloneControl_cf(Value** arg_list, int count)
{
	check_arg_count(CloneControl, 1, count);
	Control *c = arg_list[0]->to_controller(); //Assuming c is a boolControl
	if(!c) return &undefined;
	RemapDir *rmdir = NewRemapDir();
	Control *cc = NewDefaultBoolController();	
	c->CloneControl(cc, *rmdir);
	rmdir->DeleteThis();
	return cc;
}
*/

/*Value*
GetXController_cf(Value** arg_list, int count)
{
	check_arg_count(GetXController, 1, count);
	Control *c = arg_list[0]->to_controller();
	if(!c) return &undefined; 	
	return MAXControl::intern(c->GetXController());
}

Value*
GetYController_cf(Value** arg_list, int count)
{
	check_arg_count(GetYController, 1, count);
	Control *c = arg_list[0]->to_controller();
	if(!c) return &undefined; 
	return MAXControl::intern(c->GetYController());
}

Value*
GetZController_cf(Value** arg_list, int count)
{
	check_arg_count(GetZController, 1, count);
	Control *c = arg_list[0]->to_controller();
	if(!c) return &undefined; 
	return MAXControl::intern(c->GetZController());
}*/

Value*
CreateLockKey_cf(Value** arg_list, int count)
{
	check_arg_count(CreateLockKey, 3, count);
	Control *c = arg_list[0]->to_controller();
	if(!c) return &undefined;	
	return c->CreateLockKey(arg_list[1]->to_timevalue(), 
		arg_list[2]->to_int()) ? 
		&true_value : &false_value;	
}

Value*
MirrorIKConstraints_cf(Value** arg_list, int count)
{
	check_arg_count(MirrorIKConstraints, 3, count);
	INode *node = arg_list[0]->to_node();	
	node->GetTMController()->MirrorIKConstraints(
		arg_list[1]->to_int(), arg_list[2]->to_int());
	return &ok;
}

Value*
NodeIKParamsChanged_cf(Value** arg_list, int count)
{
	check_arg_count(NodeIKParamsChanged, 1, count);
	INode *node = arg_list[0]->to_node();	
	node->GetTMController()->NodeIKParamsChanged();
	return &ok;
}

Value*
OKToBindToNode_cf(Value** arg_list, int count)
{
	check_arg_count(OKToBindToNode, 2, count);
	INode *node = arg_list[0]->to_node();	
	return node->GetTMController()->OKToBindToNode(arg_list[1]->to_node()) ? 
		&true_value : &false_value;	
}

Value* 
GetUIColor_cf(Value** arg_list, int count)
{
	check_arg_count(GetUIColor, 1, count);
	//Has 57 types
	return new Point3Value(GetUIColor(arg_list[0]->to_int()));	
}

Value* 
SetUIColor_cf(Value** arg_list, int count)
{
	check_arg_count(SetUIColor, 2, count);
	//Has 57 types
	int which = arg_list[0]->to_int();
	Point3 clr = arg_list[1]->to_point3();
	((IColorManager*)GetCOREInterface(COLOR_MGR_INTERFACE))->SetOldUIColor(which, &clr);
	return arg_list[1];
}

Value* 
GetDefaultUIColor_cf(Value** arg_list, int count)
{
	check_arg_count(GetDefaultUIColor, 1, count);
	//Has 57 types
	return new Point3Value(GetDefaultUIColor(arg_list[0]->to_int()));
}

static BOOL MatchString(TCHAR *s, TCHAR *p, BOOL ignoreCase)
{
	int i = 0;

	if(p[0] == _T('*')) {
		while(s[i]) {
			if(MatchString(s+i, p+1, ignoreCase))
				return TRUE;
			i++;
		}
		if(s[i] == p[1])
			return TRUE;
		return FALSE;
	}
	while( s[i] && p[i] && ((s[i] == p[i]) || (p[i] == _T('?')) || 
			(ignoreCase && (_totupper(s[i]) == _totupper(p[i]))) ) )
		i++;
	if(s[i] == p[i] || (ignoreCase && (_totupper(s[i]) == _totupper(p[i]))))
		return TRUE;
	if(p[i] == _T('*'))
		return MatchString(s+i, p+i, ignoreCase);
	return FALSE;
}

Value*
MatchPattern_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(MatchPattern, 1, count);
/*
	return MatchPattern(
		TSTR(arg_list[0]->to_string()),
		TSTR(key_arg(pattern)->to_string()),
		key_arg_or_default(ignoreCase, &true_value)->to_bool()) ?
		&true_value : &false_value;
*/
	BOOL res = MatchString(
		arg_list[0]->to_string(),
		key_arg(pattern)->to_string(),
		key_arg_or_default(ignoreCase, &true_value)->to_bool());
	return res ? &true_value : &false_value;
}

Value*
IntersectRayEx_cf(Value** arg_list, int count)
{
	float		at;
	Point3		pt, normal, bary;
	Ray			ray, os_ray;
	GeomObject*	obj;	
	DWORD		fi;

	check_arg_count(intersectRayEx, 2, count);
	INode* node = get_valid_node(arg_list[0], intersectRayEx);

	// Get the object from the node
	ObjectState os = node->EvalWorldState(MAXScript_time());
	if (os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
		obj = (GeomObject*)os.obj;
		if (obj->ClassID() != GetEditTriObjDesc()->ClassID() &&
			obj->ClassID() != Class_ID(TRIOBJ_CLASS_ID, 0))		
			return &undefined;
	}
	else
		return &undefined;
	
	// Back transform the ray into object space.		
	Matrix3 obtm  = node->GetObjectTM(MAXScript_time());
	Matrix3 iobtm = Inverse(obtm);
	ray = arg_list[1]->to_ray();
	os_ray.p   = iobtm * ray.p;
	os_ray.dir = VectorTransform(iobtm, ray.dir);
	
	// See if we hit the object
	if (((TriObject*)obj)->GetMesh().IntersectRay(os_ray, at, normal, fi, bary))
	{
		// Calculate the hit point
		pt = os_ray.p + os_ray.dir * at;

		// transform back into world space & build result ray
		pt = pt * obtm;
		normal = Normalize(VectorTransform(obtm, normal));
		
		// Return the array containing the intersection details 
		one_typed_value_local(Array* result);
		vl.result = new Array(3);
		vl.result->append(new RayValue (pt, normal));
		vl.result->append(Integer::intern (fi+1));
		vl.result->append(new Point3Value (bary));
		return_value (vl.result); // LAM - 5/18/01 - was return vl.result
	}
	else
		return &undefined;
}

Value*
ArbAxis_cf(Value** arg_list, int count)
{
	check_arg_count(ArbAxis, 1, count);	
	Matrix3 mat(TRUE);
	ArbAxis(Normalize(arg_list[0]->to_point3()), mat);
	return new Matrix3Value(mat);
}

Value*
degToRad_cf(Value** arg_list, int count)
{
	check_arg_count(DegToRad, 1, count);
	return Float::intern(DegToRad(arg_list[0]->to_float()));

}

Value*
radToDeg_cf(Value** arg_list, int count)
{
	check_arg_count(RadToDeg, 1, count);
	return Float::intern(RadToDeg(arg_list[0]->to_float()));
}

Value*
RAMPlayer_cf(Value** arg_list, int count)
{
	//RAMPlayer <file_name1> <file_name2>
	check_arg_count(RAMPlayer, 2, count);
	MAXScript_interface->RAMPlayer(MAXScript_interface->GetMAXHWnd(),
		arg_list[0]->to_filename(), arg_list[1]->to_filename());
	return &ok;
}

// RK 5/12/99, Commenting as Shiva does not use C-Dilla
/*Value*
IsEmergencyLicense_cf(Value** arg_list, int count)
{
	check_arg_count(IsEmergencyLicense, 0, count);
	return MAXScript_interface->IsEmergencyLicense() ? &true_value : &false_value;
}

Value*
IsNetworkLicense_cf(Value** arg_list, int count)
{
	check_arg_count(IsNetworkLicense, 0, count);
	return MAXScript_interface->IsNetworkLicense() ? &true_value : &false_value;
}

Value*
IsTrialLicense_cf(Value** arg_list, int count)
{
	check_arg_count(IsTrialLicense, 0, count);
	return MAXScript_interface->IsTrialLicense() ? &true_value : &false_value;
}
*/

Value*
IsNetServer_cf(Value** arg_list, int count)
{
	return MAXScript_interface->IsNetServer() ? &true_value : &false_value;
}

/*Value*
SetNetServer_cf(Value** arg_list, int count)
{
	MAXScript_interface->SetNetServer(); 
	return &ok;
}*/


// Access to atmos/effects active checbox
Value*
fx_set_active_cf(Value** arg_list, int count)
{
	check_arg_count(setActive, 2, count);
	Value *val = arg_list[0];
	BOOL active = arg_list[1]->to_bool();	
	if (val->tag != class_tag(MAXAtmospheric) && val->tag != class_tag(MAXEffect))
 		throw RuntimeError(GetString(IDS_WANTED_ATMOS_EFFECT), val);
	ReferenceTarget *obj = ((MAXWrapper*)val)->get_max_object();
	if (!active) obj->SetAFlag(A_ATMOS_DISABLED);
	else obj->ClearAFlag(A_ATMOS_DISABLED);
	obj->NotifyDependents(FOREVER, PART_ALL | PART_MXS_PROPCHANGE, REFMSG_CHANGE);	
	obj->NotifyDependents(FOREVER, PART_ALL, REFMSG_SFX_CHANGE); // LAM - defect 265455	
	return &ok;
}

Value*
fx_is_active_cf(Value** arg_list, int count)
{
	check_arg_count(isActive, 1, count);
	Value *val = arg_list[0];
	if (val->tag != class_tag(MAXAtmospheric) && val->tag != class_tag(MAXEffect))
		throw RuntimeError(GetString(IDS_WANTED_ATMOS_EFFECT), val);
	ReferenceTarget *obj = ((MAXWrapper*)val)->get_max_object();
	return obj->TestAFlag(A_ATMOS_DISABLED) ? &false_value : &true_value; 
}

#ifndef NO_OBJECT_BOOL
/*---------------------- Boolean object access --------------------------------------*/

static IBoolObject*
setup_bool_access(Value* val)
{
	type_check(val, MAXNode, "Boolean Object access");
	INode* node = val->to_node();
	deletion_check(((MAXNode*)val));	
	
	Object* obj = node->GetObjectRef();
	if (obj->ClassID() != NEWBOOL_CLASS_ID/*Class_ID(BOOLOBJ_CLASS_ID, 0)*/)
		throw RuntimeError (GetString(IDS_RK_INVALID_BOOLEAN_OBJECT), val);
	return (IBoolObject*)obj;
}

class BoolRestore : public RestoreObj
{
public:
	INode *node;
	BoolRestore(INode *n) { node = n; }
	void Restore(int isUndo) { if (node->Selected()) MAXScript_interface->SelectNode(node, FALSE); }
	void Redo() { if (node->Selected()) MAXScript_interface->SelectNode(node, FALSE); }
};

Value*
createBooleanObject_cf(Value** arg_list, int count)
{		
	// createBooleanObject <operand_A> [<operand_B>] [<add_method>] [<mat_method]
	if (count < 1 || (count > 1 && count != 4))
		throw ArgCountError (_T("createBooleanObject "), 1, count);	
	INode* nodeA = arg_list[0]->to_node();
	deletion_check(((MAXNode*)arg_list[0]));
	if (theHold.Holding()) theHold.Put(new BoolRestore(nodeA));
	ObjectState os = nodeA->GetObjectRef()->Eval(MAXScript_time());
	if (os.obj->IsParticleSystem() || !(os.obj->IsDeformable() || os.obj->CanConvertToType(defObjectClassID)))
		throw RuntimeError (GetString(IDS_RK_CREATEBOOLEANOBJECT_SOURCE_OBJECT), arg_list[0]);
	
	// make the new boolean obj and add source obj as operand A
	IBoolObject* obj = (IBoolObject*)MAXScript_interface->CreateInstance(
		GEOMOBJECT_CLASS_ID, NEWBOOL_CLASS_ID/*Class_ID(BOOLOBJ_CLASS_ID,0)*/);		
	if (!obj) throw RuntimeError(GetString(IDS_RK_BOOLEAN_OBJECT_NOT_CREATABLE));
	obj->SetOperandA (MAXScript_time(), nodeA);	
	nodeA->SetObjectRef(obj);
	
	// Set operandB if supplied
	if (count>1)	
	{			
		INode* nodeB = arg_list[1]->to_node();
		deletion_check(((MAXNode*)arg_list[1]));
		
		/*// The following has to be done to prevent crashing in SetOperandB()
		BOOL sel = nodeA->Selected();		
		if (!sel) MAXScript_interface->SelectNode(nodeA,FALSE);		
		int mode = MAXScript_interface->GetCommandPanelTaskMode();
		MAXScript_interface->SetCommandPanelTaskMode(TASK_MODE_MODIFY);*/

		obj->SetOperandB (MAXScript_time(), nodeB, nodeA,
			arg_list[2]->to_int()-1, arg_list[3]->to_int()-1);
		
		//MAXScript_interface->SetCommandPanelTaskMode(mode);
		//if (!sel) MAXScript_interface->DeSelectNode(nodeA);
	}	
	
	// Update the command panel if selected	
	if(nodeA->Selected()) MAXScript_interface->SelectNode(nodeA, FALSE);	
	nodeA->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	needs_redraw_set();
	return arg_list[0];
}

Value*
setOperandB_cf(Value** arg_list, int count)
{
	// setOperandB <operand_A> <operand_B> <add_method> <mat_method>
	check_arg_count(setOperandB, 4, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	INode* nodeB = arg_list[1]->to_node();
	deletion_check(((MAXNode*)arg_list[1]));
	obj->SetOperandB (MAXScript_time(), nodeB, arg_list[0]->to_node(), 
		arg_list[2]->to_int()-1, arg_list[3]->to_int()-1);
	needs_redraw_set();
	return &ok;
}

Value*
GetOperandSel_cf(Value** arg_list, int count)
{
	check_arg_count(GetOperandSel, 2, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	return obj->GetOperandSel(arg_list[1]->to_int()-1) ? 
		&true_value : &false_value;	
}	

Value*
SetOperandSel_cf(Value** arg_list, int count)
{
	check_arg_count(SetOperandSel, 3, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	obj->SetOperandSel(arg_list[1]->to_int()-1, arg_list[2]->to_bool());
	needs_redraw_set();
	return arg_list[1];
}

Value*
GetBoolOp_cf(Value** arg_list, int count)
{
	check_arg_count(GetBoolOp, 1, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	return Integer::intern(obj->GetBoolOp()+1);
}

Value*
SetBoolOp_cf(Value** arg_list, int count)
{
	check_arg_count(setOperandB, 2, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	obj->SetBoolOp(arg_list[1]->to_int()-1);
	needs_redraw_set();
	return arg_list[1];
}

Value*
GetBoolCutType_cf(Value** arg_list, int count)
{
	check_arg_count(GetBoolCutType, 1, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	return Integer::intern(obj->GetBoolCutType()+1);	
}

Value*
SetBoolCutType_cf(Value** arg_list, int count)
{
	check_arg_count(SetBoolCutType, 2, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	obj->SetBoolCutType(arg_list[1]->to_int()-1);
	needs_redraw_set();
	return arg_list[1];
}

Value*
GetDisplayResult_cf(Value** arg_list, int count)
{
	check_arg_count(GetDisplayResult, 1, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	return obj->GetDisplayResult() ? &true_value : &false_value;
}

Value*
SetDisplayResult_cf(Value** arg_list, int count)
{
	check_arg_count(SetDisplayResult, 2, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	obj->SetDisplayResult(arg_list[1]->to_bool());
	needs_redraw_set();
	return arg_list[1];
}

Value*
GetShowHiddenOps_cf(Value** arg_list, int count)
{
	check_arg_count(GetShowHiddenOps, 1, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	return obj->GetShowHiddenOps() ? &true_value : &false_value;
}

Value*
SetShowHiddenOps_cf(Value** arg_list, int count)
{
	check_arg_count(setOperandB, 2, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	obj->SetShowHiddenOps(arg_list[1]->to_bool());
	needs_redraw_set();
	return arg_list[1];
}

Value*
GetUpdateMode_cf(Value** arg_list, int count)
{
	check_arg_count(GetUpdateMode, 1, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	return Integer::intern(obj->GetUpdateMode()+1);
}

Value*
SetUpdateMode_cf(Value** arg_list, int count)
{
	check_arg_count(SetUpdateMode, 2, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	int mode = arg_list[1]->to_int()-1;
	if (mode==0) {
		obj->SetUpdateMode(BOOLUPDATE_ALWAYS);
		needs_redraw_set();
	}
	else if (mode==1) obj->SetUpdateMode(BOOLUPDATE_RENDER);
	else if (mode==2) obj->SetUpdateMode(BOOLUPDATE_MANUAL);	
	return arg_list[1];
}

Value*
GetOptimize_cf(Value** arg_list, int count)
{
	check_arg_count(GetOptimize, 1, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	return obj->GetOptimize() ? &true_value : &false_value;
}

Value*
SetOptimize_cf(Value** arg_list, int count)
{
	check_arg_count(SetOptimize, 2, count);
	IBoolObject* obj = setup_bool_access(arg_list[0]);
	obj->SetOptimize(arg_list[1]->to_bool());	
	needs_redraw_set();
	return arg_list[1];
}
#endif // NO_OBJECT_BOOL

// particle system access
static SimpleParticle*
setup_indexed_particle(TCHAR* op, int& index, Value** arg_list, int count)
{
	SimpleParticle* sp;

	if (count != 3)
		throw ArgCountError (op, 3, count);

	Value* val = arg_list[0];
	type_check(val, MAXNode, "Particle Object access");
	INode* node = val->to_node();
	deletion_check(((MAXNode*)val));	

	// pick up base obj & make sure its a particle system & check particle index if given
	ObjectState os = node->EvalWorldState(MAXScript_time());
	if (os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID ||
		(sp = (SimpleParticle*)os.obj->GetInterface(I_SIMPLEPARTICLEOBJ)) == NULL)
			throw RuntimeError (GetString(IDS_SIMPLEPARTICLE_OPERATION_ON_NONSIMPLEPARTICLESYSTEM), os.obj->GetObjectName());

	index = arg_list[1]->to_int() - 1;	// get & check particle index, cvt from 1-based to 0-based
	if (index < 0 || index >= sp->parts.Count())
		throw RuntimeError (GetString(IDS_PARTICLE_INDEX_OUT_OF_RANGE), arg_list[1]);

	TimeValue t;
	if ((t = MAXScript_time()) != sp->tvalid)
		sp->UpdateParticles(t, node);	// rebuild particle array if not currently at this time

	//return sp->parts.Alive(index) ? &sp->parts : NULL;  // return partsys only if indexed particle is alive
	
	return sp;
}

/*Value*
SetParticlePos_cf(Value** arg_list, int count)
{
	int index;	
	SimpleParticle* obj = setup_indexed_particle(_T("particlePos"), index, arg_list, count);
	obj->SetParticlePosition(MAXScript_time(), index, arg_list[2]->to_point3());
	obj->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	return arg_list[2];
}

Value*
SetParticleVelocity_cf(Value** arg_list, int count)
{
	int index;	
	SimpleParticle* obj = setup_indexed_particle(_T("particleVelocity"), index, arg_list, count);
	obj->SetParticleVelocity(MAXScript_time(), index, arg_list[2]->to_point3());
	obj->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	return arg_list[2];
}

Value*
SetParticleAge_cf(Value** arg_list, int count)
{
	int index;	
	SimpleParticle* obj = setup_indexed_particle(_T("particleAge"), index, arg_list, count);
	obj->SetParticleAge(MAXScript_time(), index, arg_list[2]->to_timevalue());
	obj->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	return arg_list[2];
}*/


// trackbar access

Value*
set_trackbar_visible(Value* val)
{
	MAXScript_interface->GetTrackBar()->SetVisible(val->to_bool());
	return val;
}

Value*
get_trackbar_visible()
{
	return MAXScript_interface->GetTrackBar()->IsVisible() ? &true_value : &false_value;	
}

Value*
set_trackbar_filter(Value* val)
{
	def_tbar_filter_types();
	MAXScript_interface->GetTrackBar()->SetFilter(
		GetID(tbarFiltTypes, elements(tbarFiltTypes), val));
	return val;
}

Value*
get_trackbar_filter()
{
	def_tbar_filter_types();
	return GetName(tbarFiltTypes, elements(tbarFiltTypes), 
		MAXScript_interface->GetTrackBar()->GetFilter());
}

Value*
get_trackbar_next_key_cf(Value** arg_list, int count)
{
	check_arg_count(GetNextKeyTime, 0, count);
	TimeValue t = MAXScript_interface->GetTrackBar()->GetNextKey(MAXScript_time(),TRUE);	
	return (t==MAXScript_time()) ? &undefined : MSTime::intern(t);
}

Value*
get_trackbar_prev_key_cf(Value** arg_list, int count)
{
	check_arg_count(GetPreviousKeyTime, 0, count);
	TimeValue t = MAXScript_interface->GetTrackBar()->GetNextKey(MAXScript_time(), FALSE);
	return (t==MAXScript_time()) ? &undefined : MSTime::intern(t);
}


// file property access

// Document SummaryInfo properties from PropertyTest.h
#define PID_MANAGER				0x0000000E
#define PID_COMPANY				0x0000000F
#define PID_CATEGORY			0x00000002
#define PID_HEADINGPAIR			0x0000000C
#define PID_DOCPARTS			0x0000000D


// string maps for File Properties->Summary page fields
struct prop_type { int id; TCHAR* name; };
static prop_type props[] = {
	{ PIDSI_TITLE,		_T("Title") },
	{ PIDSI_SUBJECT,	_T("Subject") },
	{ PIDSI_AUTHOR,		_T("Author") },
	{ PID_MANAGER,		_T("Manager") },
	{ PID_COMPANY,		_T("Company") },
	{ PID_CATEGORY,		_T("Category") },
	{ PIDSI_KEYWORDS,	_T("Keywords") },
	{ PIDSI_COMMENTS,	_T("Comments") },
	{ PID_HEADINGPAIR,	_T("Headers") }	
};

// from App/PropertyEditor.cpp
static BOOL ParseDateFromString(char* pValue, FILETIME* ft)
	{
	char	szDateSeparator[2];
	char	szDateFormat[2];

	// Get Date Separator
	GetLocaleInfo(	LOCALE_USER_DEFAULT,	// locale identifier
					LOCALE_SDATE,			// type of information
					szDateSeparator,		// address of buffer for information
					2						// size of buffer
					);

	// Get Date Format
	GetLocaleInfo(	LOCALE_USER_DEFAULT,	// locale identifier
					LOCALE_IDATE,			// type of information
					szDateFormat,			// address of buffer for information
					2						// size of buffer
					);

	char*	p = pValue;
	char*	pCs = pValue;
	char	szComponent[80];
	char	szNextChar[2];
	int		nDate[3];
	int		component = 0;

	strcpy(szComponent, "");
	szNextChar[1] = '\0';
	do {
		if (*p == szDateSeparator[0]) {
			nDate[component] = atoi(szComponent);
			strcpy(szComponent, "");
			component++;
			p++;
			pCs = p;
			}
		else {
			szNextChar[0] = *p;
			strcat(szComponent, szNextChar);
			p++;
			}
		} while (*p && (component != 3));

	if (component != 2)
		return FALSE;

	if (strlen(szComponent) > 0) {
		nDate[component] = atoi(szComponent);
		}

	int dateFormat = atoi(szDateFormat);
	// 0 - Month-Day-Year 
	// 1 - Day-Month-Year 
	// 2 - Year-Month-Day 

	int dayIdx;
	int monthIdx;
	int yearIdx;

	switch (dateFormat) {
		case 0:
			dayIdx = 1;
			monthIdx = 0;
			yearIdx = 2;
			break;
		case 1:
			dayIdx = 0;
			monthIdx = 1;
			yearIdx = 2;
			break;
		case 2:
			dayIdx = 2;
			monthIdx = 1;
			yearIdx = 0;
			break;
		}

	if (nDate[yearIdx] < 28) {
		nDate[yearIdx] += 2000;
		}
	else if (nDate[yearIdx] < 100) {
		nDate[yearIdx] += 1900;
		}

	SYSTEMTIME sysTime;
	sysTime.wYear = nDate[yearIdx];
	sysTime.wMonth = nDate[monthIdx];
	sysTime.wDayOfWeek = 0;	// Ignored
	sysTime.wDay = nDate[dayIdx];;
	sysTime.wHour = 12;
	sysTime.wMinute = 0;
	sysTime.wSecond = 0;
	sysTime.wMilliseconds = 0; 

	return SystemTimeToFileTime(&sysTime, ft);
	}


Value*
fp_num_properties_cf(Value** arg_list, int count)
{
	// fileProperties.getNumProperties <set_name>
	check_arg_count(getNumProperties, 1, count);
	def_propertyset_types();
	int set = GetID(propertysetTypes, elements(propertysetTypes), arg_list[0]);
	int num = MAXScript_interface->GetNumProperties(set);
	if (set == PROPSET_DOCSUMMARYINFO) num--;// Don't count document parts
	return Integer::intern(num);
}

Value*
fp_get_property_name_cf(Value** arg_list, int count)
{
	// fileProperties.getPropertyName <set_name> <index>
	check_arg_count(getProperty, 2, count);
	def_propertyset_types();
	int set	= GetID(propertysetTypes, elements(propertysetTypes), arg_list[0]);
	int index =  arg_list[1]->to_int()-1;
	
	int num = MAXScript_interface->GetNumProperties(set);	
	if (set == PROPSET_DOCSUMMARYINFO) num--;// Don't count document parts
	
	if (index < 0 || index > num-1)
		throw RuntimeError(GetString(IDS_RK_FILEPROPS_GETPROPERTY_INDEX_OUT_OF_RANGE));
	
	const PROPSPEC* ps = MAXScript_interface->GetPropertySpec(set, index);
	
	if (ps->ulKind==PRSPEC_LPWSTR)
		return new String(TSTR(ps->lpwstr));	
	else
	{
		for(int i=0; i < elements(props); i++)
			if(props[i].id == ps->propid) 
				return new String(props[i].name);
	}
	return &undefined;	
}

Value*
fp_get_property_val_cf(Value** arg_list, int count)
{
	// fileProperties.getPropertyValue <set_name> <index>
	check_arg_count(getPropertyValue, 2, count);
	def_propertyset_types();
	int set = GetID(propertysetTypes, elements(propertysetTypes), arg_list[0]);
	int index =  arg_list[1]->to_int()-1;
	
	int num = MAXScript_interface->GetNumProperties(set);	
	if (set == PROPSET_DOCSUMMARYINFO) num--;// Don't count document parts

	if (index < 0 || index > num-1)
		throw RuntimeError(GetString(IDS_RK_FILEPROPS_GETPROPERTYVALUE_INDEX_OUT_OF_RANGE));

	const PROPVARIANT* pProp = MAXScript_interface->GetPropertyVariant(set, index);
	const PROPSPEC* pSpec = MAXScript_interface->GetPropertySpec(set, index);
	
	if (!pProp || !pSpec) return &undefined;
	
	if (pSpec->propid == PID_HEADINGPAIR) // Contents page
	{
		const CAPROPVARIANT *pHeading = &pProp->capropvar;		
		
		// Return an array of arrays of all the items in contents page
		one_typed_value_local(Array* result); 
		vl.result = new Array(pHeading->cElems/2);
		for (UINT i=0; i<pHeading->cElems; i+=2) 
			vl.result->append(new String(TSTR(pHeading->pElems[i].pszVal))); 
		return_value(vl.result);		
	}
	
	switch(pProp->vt) {
		case VT_LPWSTR: return new String(TSTR(pProp->pwszVal));
		case VT_LPSTR:	return new String(TSTR(pProp->pszVal));
		case VT_I4:		return Integer::intern(pProp->lVal);
		case VT_I8:    return Integer64::intern(pProp->hVal.QuadPart);
		case VT_INT:   return Integer::intern(pProp->intVal);
		case VT_INT_PTR:
#if !defined( WIN64 )
			return IntegerPtr::intern(pProp->intVal);
#else
			return IntegerPtr::intern(pProp->hVal.QuadPart);
#endif
		case VT_R4:		return Float::intern(pProp->fltVal);
		case VT_R8:		return Double::intern(pProp->dblVal);			
		case VT_BOOL:	return pProp->boolVal ? &true_value : &false_value;
		case VT_FILETIME:
			SYSTEMTIME sysTime;
			TCHAR time_str[64];			
			FileTimeToSystemTime(&pProp->filetime, &sysTime);
			GetDateFormat(LOCALE_SYSTEM_DEFAULT,
						  DATE_SHORTDATE,
						  &sysTime,
						  NULL,
						  time_str,
						  sizeof(time_str));
			return new String(time_str);
	}
	return &undefined;
}

Value*
fp_get_items_cf(Value** arg_list, int count)
{
	// fileProperties.getItems <header_name>
	check_arg_count(getItems, 1, count);
	PROPSPEC spec;
	spec.ulKind = PRSPEC_PROPID;
	spec.propid = PID_HEADINGPAIR;
	int idx;
	Interface *ip = MAXScript_interface;

	const PROPVARIANT*	pPropHeading;	
	const PROPVARIANT*	pPropDocPart;	

	if ((idx = ip->FindProperty(PROPSET_DOCSUMMARYINFO, &spec)) != -1) {
		if ((pPropHeading = ip->GetPropertyVariant(PROPSET_DOCSUMMARYINFO, idx)) != NULL) {
			
			spec.propid = PID_DOCPARTS;							
			if ((idx = ip->FindProperty(PROPSET_DOCSUMMARYINFO, &spec)) != -1) {
				if ((pPropDocPart = ip->GetPropertyVariant(PROPSET_DOCSUMMARYINFO, idx)) != NULL) {

					const CAPROPVARIANT*	pHeading = &pPropHeading->capropvar;
					int nDocPart = 0;
					for (UINT i=0; i<pHeading->cElems; i+=2) {
						
						int ndocs = pHeading->pElems[i+1].lVal;						
						
						if(_tcsicmp(TSTR(pHeading->pElems[i].pszVal), arg_list[0]->to_string()))
						{
							nDocPart += ndocs;
							continue;
						}
						
						// Return and array of docparts once the heading is found
						const CALPSTR*	pDocPart = &pPropDocPart->calpstr;			
						one_typed_value_local(Array* result);
						vl.result = new Array(ndocs);
						for (int j=0; j<ndocs; j++) {
							vl.result->append(new String(TSTR(pDocPart->pElems[nDocPart]))); 
							nDocPart++;
						}
						return_value(vl.result);
					}
				}
			}
		}
	}
	return &undefined;
}

Value*
fp_find_property_cf(Value** arg_list, int count)
{
	// fileProperties.findProperty <set_name> <prop_name>
	check_arg_count(findProperty, 2, count);
	def_propertyset_types();
	
	int set = GetID(propertysetTypes, elements(propertysetTypes), arg_list[0]);
	TCHAR *pname = arg_list[1]->to_string();
	PROPSPEC spec;	
	WStr wsz(pname);

	if(set==PROPSET_SUMMARYINFO || set==PROPSET_DOCSUMMARYINFO) 
	{
		spec.ulKind = PRSPEC_PROPID;
		bool found = false;
		for(int i=0; i < elements(props); i++)
			if(_tcsicmp(pname, props[i].name)==0) {
				spec.propid = props[i].id;
				found=true;
				break;
			}

		if (!found) return Integer::intern(0); // return 0 if not found
	}
	else
	{
		spec.ulKind = PRSPEC_LPWSTR;
		spec.lpwstr = wsz;
	}	
	int ind = MAXScript_interface->FindProperty(set, &spec);	
	return Integer::intern(ind+1);
}

Value*
fp_add_property_cf(Value** arg_list, int count)
{
	// fileProperties.addProperty <set_name> <prop_name> <prop_value> [#date]
	BOOL date = FALSE;	
	if (count==4) {
		if(arg_list[3]==n_date)
			date = TRUE;
		else
			check_arg_count(addProperty, 3, count);
	}
	else
		check_arg_count(addProperty, 3, count);
	
	def_propertyset_types();	
	int set = GetID(propertysetTypes, elements(propertysetTypes), arg_list[0]);
	TCHAR *pname = arg_list[1]->to_string();
	Value *val = arg_list[2];
	WStr wszName(pname);
	TSTR szVal;

	PROPVARIANT	var;
	PROPSPEC	spec;	

	if(set==PROPSET_SUMMARYINFO || set==PROPSET_DOCSUMMARYINFO) // if adding a summary page property
	{
		spec.ulKind = PRSPEC_PROPID;
		bool found = false;
		for(int i=0; i < elements(props); i++)
			if(_tcsicmp(pname, props[i].name)==0) {
				spec.propid = props[i].id;
				found = true;
				break;
			}
		if (!found || spec.propid==PID_HEADINGPAIR)
			throw RuntimeError(GetString(IDS_RK_INVALID_FILEPROP_SUMMARY_PROPERTY), arg_list[1]); 
	}
	else
	{
		spec.ulKind = PRSPEC_LPWSTR;
		spec.lpwstr = wszName;
	}
 	// Add the property variant based on the argument type passed
	if(is_string(val)) 
	{		
		if(date)
		{
			FILETIME ft;
			if (ParseDateFromString(val->to_string(), &ft)) {
				var.vt = VT_FILETIME;
				var.filetime.dwLowDateTime = ft.dwLowDateTime;
				var.filetime.dwHighDateTime = ft.dwHighDateTime;
				}
			else throw RuntimeError(GetString(IDS_RK_INVALID_FILEPROP_DATE_FORMAT), val);
		}
		else 
		{
			var.vt = VT_LPSTR;
			var.pszVal = (szVal=val->to_string());			
		}

	}
	else if(is_integer(val))
	{
		var.vt = VT_I4;
		var.lVal = val->to_int();
	}
	else if(is_integer64(val))
	{
		var.vt = VT_I8;
		var.hVal.QuadPart = val->to_int64();
	}
	else if(is_integerptr(val))
	{
		var.vt = VT_INT_PTR;
#if !defined( WIN64 )
		V_INT(&var) = val->to_intptr();
#else
		var.hVal.QuadPart = val->to_intptr();
#endif
	}
	else if(is_float(val))
	{
		var.vt = VT_R4;
		var.fltVal = val->to_float();
	}
	else if(is_double(val))
	{
		var.vt = VT_R8;
		var.dblVal = val->to_double();
	}
	else if(is_bool(val))
	{
		var.vt = VT_BOOL;
		var.boolVal = val->to_bool();
	}
	else
		throw RuntimeError(GetString(IDS_RK_INVALID_FILEPROP_VALUE_TYPE), val);

	MAXScript_interface->AddProperty(set, &spec, &var);
	return &ok;
}

Value*
fp_delete_property_cf(Value** arg_list, int count)
{
	// fileProperties.deleteProperty <set> <prop_name>
	check_arg_count(DeleteProperty, 2, count);
	def_propertyset_types();
	int set = GetID(propertysetTypes, elements(propertysetTypes), arg_list[0]);
	
	TCHAR *pname = arg_list[1]->to_string();
	WStr wsz(pname);
	PROPSPEC spec;

	if(set==PROPSET_SUMMARYINFO || set==PROPSET_DOCSUMMARYINFO) 
	{
		spec.ulKind = PRSPEC_PROPID;
		bool found = false;
		for(int i=0; i < elements(props); i++)
			if(_tcsicmp(pname, props[i].name)==0) {
				spec.propid = props[i].id;
				found=true;
				break;
			}
		if (!found || spec.propid==PID_HEADINGPAIR)
			throw RuntimeError(GetString(IDS_RK_INVALID_FILEPROP_SUMMARY_PROPERTY), arg_list[1]); 
	}
	else
	{		
		spec.ulKind = PRSPEC_LPWSTR;
		spec.lpwstr = wsz;
	}
	MAXScript_interface->DeleteProperty(set,&spec);
	return &ok;
}

/* --------------------- plug-in init --------------------------------- */
// this is called by the dlx initializer, register the global vars here
void rk_init()
{
	#include "rk_glbls.h"
}


//watje

Value* 
SetSelectFilter_cf(Value** arg_list, int count)
{
	check_arg_count(SetSelectFilter, 1, count);
//	def_coord_types();	
	int c = arg_list[0]->to_int()-1;
	if ( (c >=0 ) && (c < MAXScript_interface->GetNumberSelectFilters()))
		MAXScript_interface->SetSelectFilter(c);	
	return 	&ok;	
}

//watje

Value* 
GetSelectFilter_cf(Value** arg_list, int count)
{
	check_arg_count(GetSelectFilter, 0, count);
//	def_coord_types();	
//	int c = arg_list[0]->to_int()-1;
	int c = MAXScript_interface->GetSelectFilter();	
	return 	Integer::intern(c+1);	
}

Value* 
GetNumberSelectFilters_cf(Value** arg_list, int count)
{
	check_arg_count(GetNumberSelectFiltera, 0, count);
//	def_coord_types();	
//	int c = arg_list[0]->to_int()-1;
	int c = MAXScript_interface->GetNumberSelectFilters();	
	return 	Integer::intern(c);	
}

//watje

Value* 
SetDisplayFilter_cf(Value** arg_list, int count)
{
	check_arg_count(SetSelectFilter, 2, count);
//	def_coord_types();	
	int c = arg_list[0]->to_int()-1;
	BOOL on = arg_list[1]->to_bool();
	if ( (c >=0 ) && (c < MAXScript_interface->GetNumberDisplayFilters()))
		MAXScript_interface->SetDisplayFilter(c,on);	
	return 	&ok;	
}

//watje

Value* 
GetDisplayFilter_cf(Value** arg_list, int count)
{
	check_arg_count(GetSelectFilter, 1, count);
//	def_coord_types();	
	int c = arg_list[0]->to_int()-1;
	BOOL on = MAXScript_interface->GetDisplayFilter(c);	
	return on ? &true_value : &false_value;	
}

Value* 
GetNumberDisplayFilters_cf(Value** arg_list, int count)
{
	check_arg_count(GetNumberSelectFiltera, 0, count);
//	def_coord_types();	
//	int c = arg_list[0]->to_int()-1;
	int c = MAXScript_interface->GetNumberDisplayFilters();	
	return 	Integer::intern(c);	
}


Value* 
GetDisplayFilterName_cf(Value** arg_list, int count)
{
	check_arg_count(GetDisplayFilterName, 1, count);
//	def_coord_types();	
	int c = arg_list[0]->to_int()-1;
	if ( (c <0 ) || (c >= MAXScript_interface->GetNumberDisplayFilters()) )
		{
		return &undefined;	
		}
	else
		{
		TCHAR *name = MAXScript_interface->GetDisplayFilterName(c);	
		return new String(name);	
		}
}

Value* 
GetSelectFilterName_cf(Value** arg_list, int count)
{
	check_arg_count(GetDisplayFilterName, 1, count);
//	def_coord_types();	
	int c = arg_list[0]->to_int()-1;
	if ( (c <0 ) || (c >= MAXScript_interface->GetNumberSelectFilters()))
		{
		return &undefined;	
		}
	else
		{
		TCHAR *name = MAXScript_interface->GetSelectFilterName(c);	
		return new String(name);	
		}
}

// LAM: Added these 5/2/01
#ifndef NO_REGIONS
Value* 
SetRenderType_cf(Value **arg_list, int count)
{
	check_arg_count(SetRenderType, 1, count);
	def_max_render_types();
	int rtype = GetID(renderTypes, elements(renderTypes), arg_list[0]);
	MAXScript_interface7->SetRenderType(rtype);
	return &ok;
}

Value* 
GetRenderType_cf(Value **arg_list, int count)
{
	check_arg_count(GetRenderType, 0, count);
	int rtype = MAXScript_interface7->GetRenderType();
	def_max_render_types();
	return GetName(renderTypes, elements(renderTypes), rtype, &undefined);
}
#endif // NO_REGIONS

