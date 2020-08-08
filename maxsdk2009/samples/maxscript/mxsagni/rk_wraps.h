// \{def_visible_primitive(\([^"]*\)\}+\{\("[^\;]*\)\}
// def_struct_primitive(\1struct, \2\3
#include "buildver.h"

def_visible_primitive			( CreatePreview,		"CreatePreview");
def_visible_primitive_debug_ok	( GetMatLibFileName, 	"GetMatLibFileName");

def_visible_primitive_debug_ok	( SetSysCur, 			"SetSysCur");

def_visible_primitive_debug_ok	( GetSnapState, 		"GetSnapState");
def_visible_primitive_debug_ok	( GetSnapMode, 			"GetSnapMode");
def_visible_primitive_debug_ok	( GetCrossing, 			"GetCrossing");
def_visible_primitive			( SetToolBtnState,		"SetToolBtnState");
def_visible_primitive_debug_ok	( GetCoordCenter, 		"GetCoordCenter");
def_visible_primitive			( SetCoordCenter, 		"SetCoordCenter");

def_visible_primitive			( EnableUndo,			"enableUndo");
def_visible_primitive			( EnableCoordCenter, 	"EnableCoordCenter");
def_visible_primitive_debug_ok	( GetRefCoordSys, 		"GetRefCoordSys");
def_visible_primitive			( SetRefCoordSys, 		"SetRefCoordSys");
def_visible_primitive			( EnableRefCoordSys, 	"EnableRefCoordSys");
def_visible_primitive_debug_ok	( GetNumAxis, 			"GetNumAxis");
def_visible_primitive			( LockAxisTripods, 		"LockAxisTripods");
def_visible_primitive_debug_ok	( AxisTripodLocked, 	"AxisTripodLocked");

def_visible_primitive			( CompleteRedraw, 		"CompleteRedraw");
def_visible_primitive			( ExclusionListDlg, 	"ExclusionListDlg");
def_visible_primitive			( MaterialBrowseDlg, 	"MaterialBrowseDlg");
def_visible_primitive			( HitByNameDlg, 		"HitByNameDlg");
def_visible_primitive			( NodeColorPicker,		"NodeColorPicker");
def_visible_primitive			( FileOpenMatLib, 		"FileOpenMatLib");
#ifndef NO_MATLIB_SAVING // orb 01-09-2002
#ifndef USE_CUSTOM_MATNAV // orb 08-23-2001 removing mtllib saving
def_visible_primitive			( FileSaveMatLib, 		"FileSaveMatLib");
def_visible_primitive			( FileSaveAsMatLib, 	"FileSaveAsMatLib");
#endif
#endif
def_visible_primitive			( LoadDefaultMatLib, 	"LoadDefaultMatLib");

def_visible_primitive			( PushPrompt, 			"PushPrompt");
def_visible_primitive			( PopPrompt, 			"PopPrompt");
def_visible_primitive			( ReplacePrompt, 		"ReplacePrompt");
def_visible_primitive			( DisplayTempPrompt, 	"DisplayTempPrompt");
def_visible_primitive			( RemoveTempPrompt, 	"RemoveTempPrompt");

def_visible_primitive			( DisableStatusXYZ, 	"DisableStatusXYZ");
def_visible_primitive			( EnableStatusXYZ,		"EnableStatusXYZ");
def_visible_primitive			( SetStatusXYZ, 		"SetStatusXYZ");

def_visible_primitive_debug_ok	( GetGridSpacing, 		"GetGridSpacing");
def_visible_primitive_debug_ok	( GetGridMajorLines, 	"GetGridMajorLines");

def_visible_primitive			( EnableShowEndRes, 	"EnableShowEndRes");
def_visible_primitive			( AppendSubSelSet,		"AppendSubSelSet");
def_visible_primitive			( ClearSubSelSets,		"ClearSubSelSets");
def_visible_primitive			( ClearCurSelSet, 		"ClearCurSelSet");
def_visible_primitive			( EnableSubObjSel, 		"EnableSubObjSel");
def_visible_primitive_debug_ok	( IsSubSelEnabled,		"IsSubSelEnabled");

def_visible_primitive_debug_ok	( IsCPEdgeOnInView, 	"IsCPEdgeOnInView");
def_visible_primitive_debug_ok	( NodeInvalRect, 		"NodeInvalRect");

def_visible_primitive			( SetVPortBGColor, 		"SetVPortBGColor");
def_visible_primitive_debug_ok	( GetVPortBGColor,		"GetVPortBGColor");
def_visible_primitive			( SetBkgImageAspect, 	"SetBkgImageAspect");
def_visible_primitive_debug_ok	( GetBkgImageAspect, 	"GetBkgImageAspect");

def_visible_primitive			( SetBkgImageAnimate, 	"SetBkgImageAnimate");
def_visible_primitive_debug_ok	( GetBkgImageAnimate, 	"GetBkgImageAnimate");

#ifndef WEBVERSION	// russom - 04/10/02
def_visible_primitive			( SetBkgFrameRange, 	"SetBkgFrameRange");
def_visible_primitive_debug_ok	( GetBkgRangeVal, 		"GetBkgRangeVal");
#endif // WEBVERSION
def_visible_primitive			( SetBkgORType, 		"SetBkgORType");
def_visible_primitive_debug_ok	( GetBkgORType, 		"GetBkgORType");

#ifndef WEBVERSION	// russom - 04/10/02
def_visible_primitive			( SetBkgStartTime,		"SetBkgStartTime");
def_visible_primitive_debug_ok	( GetBkgStartTime,		"GetBkgStartTime");
def_visible_primitive			( SetBkgSyncFrame,		"SetBkgSyncFrame");
def_visible_primitive_debug_ok	( GetBkgSyncFrame, 		"GetBkgSyncFrame");
def_visible_primitive_debug_ok	( GetBkgFrameNum, 		"GetBkgFrameNum");
#endif // WEBVERSION

def_visible_primitive			( SetUseEnvironmentMap, "SetUseEnvironmentMap");
def_visible_primitive_debug_ok	( GetUseEnvironmentMap, "GetUseEnvironmentMap");

def_visible_primitive			( DeSelectNode, 		"DeSelectNode");
def_visible_primitive			( BoxPickNode, 			"BoxPickNode");
def_visible_primitive			( CirclePickNode, 		"CirclePickNode");
def_visible_primitive			( FencePickNode, 		"FencePickNode");

def_visible_primitive			( FlashNodes, 			"FlashNodes");

#ifndef NO_DRAFT_RENDERER
def_visible_primitive			( SetUseDraftRenderer, "SetUseDraftRenderer");
def_visible_primitive_debug_ok	( GetUseDraftRenderer, "GetUseDraftRenderer");
#endif // NO_DRAFT_RENDERER

def_visible_primitive			( ClearNodeSelection, 	"ClearNodeSelection");
def_visible_primitive			( DisableSceneRedraw, 	"DisableSceneRedraw");
def_visible_primitive			( EnableSceneRedraw, 	"EnableSceneRedraw");
def_visible_primitive			( IsSceneRedrawDisabled,"IsSceneRedrawDisabled");

def_visible_primitive_debug_ok	( GetKeyStepsPos, 		"GetKeyStepsPos");
def_visible_primitive			( SetKeyStepsPos, 		"SetKeyStepsPos");

def_visible_primitive_debug_ok	( GetKeyStepsRot, 		"GetKeyStepsRot");
def_visible_primitive			( SetKeyStepsRot, 		"SetKeyStepsRot");

def_visible_primitive_debug_ok	( GetKeyStepsScale, 	"GetKeyStepsScale");
def_visible_primitive			( SetKeyStepsScale, 	"SetKeyStepsScale");

def_visible_primitive_debug_ok	( GetKeyStepsSelOnly, 	"GetKeyStepsSelOnly");
def_visible_primitive			( SetKeyStepsSelOnly, 	"SetKeyStepsSelOnly");

def_visible_primitive_debug_ok	( GetKeyStepsUseTrans,	"GetKeyStepsUseTrans");
def_visible_primitive			( SetKeyStepsUseTrans,	"SetKeyStepsUseTrans");

def_visible_primitive			( AssignNewName, 			"AssignNewName");
def_visible_primitive			( ForceCompleteRedraw,		"ForceCompleteRedraw");
def_visible_primitive_debug_ok	( GetBackGroundController,	"GetBackGroundController");
def_visible_primitive_debug_ok	( GetBackGround, 			"GetBackGround");
def_visible_primitive_debug_ok	( GetCommandPanelTaskMode, 	"GetCommandPanelTaskMode");
def_visible_primitive_debug_ok	( GetNamedSelSetItem, 		"GetNamedSelSetItem");
def_visible_primitive_debug_ok	( GetNamedSelSetItemCount, 	"GetNamedSelSetItemCount");
def_visible_primitive_debug_ok	( GetNamedSelSetName, 		"GetNamedSelSetName");
def_visible_primitive_debug_ok	( GetNumNamedSelSets, 		"GetNumNamedSelSets");

#ifndef WEBVERSION
def_visible_primitive_debug_ok	( GetRendApertureWidth, 	"GetRendApertureWidth");
def_visible_primitive			( SetRendApertureWidth, 	"SetRendApertureWidth");
#endif
def_visible_primitive_debug_ok	( GetRendImageAspect, 		"GetRendImageAspect");
def_visible_primitive			( NamedSelSetListChanged, 	"NamedSelSetListChanged");
//def_visible_primitive			( NotifyVPDisplayCBChanged,	"NotifyVPDisplayCBChanged");
def_visible_primitive_debug_ok	( OkMtlForScene, 			"OkMtlForScene");
def_visible_primitive			( RescaleWorldUnits, 		"RescaleWorldUnits");
def_visible_primitive			( SetBackGround, 			"SetBackGround");
def_visible_primitive			( SetBackGroundController, 	"SetBackGroundController");
def_visible_primitive			( SetCommandPanelTaskMode, 	"SetCommandPanelTaskMode");
def_visible_primitive			( SetCurNamedSelSet, 		"SetCurNamedSelSet");
def_visible_primitive			( StopCreating, 			"StopCreating");

def_visible_primitive			( loadDllsFromDir,			"loadDllsFromDir");
def_visible_primitive_debug_ok	( SilentMode, 				"SilentMode");
def_visible_primitive			( SetSilentMode, 			"SetSilentMode");
def_visible_primitive_debug_ok	( GetDir, 					"GetDir");
def_visible_primitive			( SetDir, 					"SetDir");
def_visible_primitive_debug_ok  ( ConvertDirIDToInt, 		"ConvertDirIDToInt");

def_visible_primitive			( SelectSaveBitMap, 		"SelectSaveBitMap");

def_visible_primitive_debug_ok	( GetUIColor, 				"GetUIColor");
def_visible_primitive			( SetUIColor, 				"SetUIColor");
def_visible_primitive_debug_ok	( GetDefaultUIColor, 		"GetDefaultUIColor");

def_visible_primitive_debug_ok	( MatchPattern, 			"MatchPattern");
//def_visible_primitive			( CreateArcballDialog,		"CreateArcballDialog");

def_visible_primitive_debug_ok	( GetCVertMode, 			"GetCVertMode");
#ifndef NO_MOTION_BLUR	// russom - 04/10/02
def_visible_primitive_debug_ok	( GetImageBlurMultController, 	"GetImageBlurMultController");
#endif	// NO_MOTION_BLUR
def_visible_primitive_debug_ok	( GetInheritVisibility, 	"GetInheritVisibility");
def_visible_primitive_debug_ok	( GetPosTaskWeight, 		"GetPosTaskWeight");
def_visible_primitive_debug_ok	( GetRenderID, 				"GetRenderID");
def_visible_primitive_debug_ok	( GetRotTaskWeight, 		"GetRotTaskWeight");
def_visible_primitive_debug_ok	( GetShadeCVerts, 			"GetShadeCVerts");
def_visible_primitive_debug_ok	( GetTaskAxisState, 		"GetTaskAxisState");
//def_visible_primitive_debug_ok( GetTaskAxisStateBits, 	"GetTaskAxisStateBits");
def_visible_primitive_debug_ok	( GetTrajectoryON,	 		"GetTrajectoryON");
def_visible_primitive_debug_ok	( IsBoneOnly, 				"IsBoneOnly");
def_visible_primitive			( SetCVertMode, 			"SetCVertMode");
#ifndef NO_MOTION_BLUR	// russom - 04/10/02
def_visible_primitive			( SetImageBlurMultController,"SetImageBlurMultController");
def_visible_primitive			( SetImageBlurMultiplier, 	"SetImageBlurMultiplier");
#endif	// NO_MOTION_BLUR
def_visible_primitive			( SetInheritVisibility, 	"SetInheritVisibility");
#ifndef NO_MOTION_BLUR	// russom - 04/10/02
def_visible_primitive			( SetMotBlur, 				"SetMotBlur");
#endif	// NO_MOTION_BLUR
def_visible_primitive			( SetPosTaskWeight, 		"SetPosTaskWeight");
def_visible_primitive			( SetRenderable, 			"SetRenderable");
def_visible_primitive			( SetRenderID, 				"SetRenderID");
def_visible_primitive			( SetRotTaskWeight, 		"SetRotTaskWeight");
def_visible_primitive			( SetShadeCVerts, 			"SetShadeCVerts");
def_visible_primitive			( SetTaskAxisState, 		"SetTaskAxisState");
def_visible_primitive			( SetTrajectoryON, 			"SetTrajectoryON");
def_visible_primitive_debug_ok	( GetTMController, 			"GetTMController");
def_visible_primitive_debug_ok	( GetMasterController, 	"GetMasterController");
def_visible_primitive_debug_ok	( GetVisController, 		"GetVisController");
def_visible_primitive_debug_ok	( is_group_head,			"isGroupHead");
def_visible_primitive_debug_ok	( is_group_member,			"isGroupMember");
def_visible_primitive_debug_ok	( is_open_group_member,		"isOpenGroupMember");
def_visible_primitive_debug_ok	( is_open_group_head,		"isOpenGroupHead");
def_visible_primitive			( set_group_member,			"setGroupMember");
def_visible_primitive			( set_group_head,			"setGroupHead");
def_visible_primitive			( set_group_open,			"setGroupOpen");
//def_visible_primitive			( set_group_member_open,	"setGroupMemberOpen");

def_visible_primitive			( CreateLockKey, 			"CreateLockKey");
def_visible_primitive			( MirrorIKConstraints,		"MirrorIKConstraints");
def_visible_primitive			( NodeIKParamsChanged, 		"NodeIKParamsChanged");
def_visible_primitive			( OKToBindToNode, 			"OKToBindToNode");

def_visible_primitive_debug_ok	( IntersectRayEx, 			"IntersectRayEx");
def_visible_primitive_debug_ok	( ArbAxis, 					"ArbAxis");
def_visible_primitive_debug_ok	( degToRad, 				"DegToRad");
def_visible_primitive_debug_ok	( radToDeg, 				"radToDeg");
def_visible_primitive			( RAMPlayer, 				"RAMPlayer");

def_visible_primitive_debug_ok	( IsNetServer, 				"IsNetServer");

// Access to atmos/effects active checbox
#ifndef NO_RENDEREFFECTS	// russom - 03/26/02
def_visible_primitive			( fx_set_active,		"setActive");
def_visible_primitive_debug_ok	( fx_is_active,			"isActive");
#endif // NO_RENDEREFFECTS

#ifndef NO_OBJECT_BOOL
// boolean compound object access
def_struct_primitive			( createBooleanObject, 	boolObj, "createBooleanObject");
def_struct_primitive			( setOperandB, 			boolObj, "setOperandB");
def_struct_primitive_debug_ok	( GetOperandSel, 		boolObj, "GetOperandSel");
def_struct_primitive			( SetOperandSel, 		boolObj, "SetOperandSel");
def_struct_primitive_debug_ok	( GetBoolOp, 			boolObj, "GetBoolOp");
def_struct_primitive			( SetBoolOp, 			boolObj, "SetBoolOp");
def_struct_primitive_debug_ok	( GetBoolCutType, 		boolObj, "GetBoolCutType");
def_struct_primitive			( SetBoolCutType, 		boolObj, "SetBoolCutType");
def_struct_primitive_debug_ok	( GetDisplayResult, 	boolObj, "GetDisplayResult");
def_struct_primitive			( SetDisplayResult, 	boolObj, "SetDisplayResult");
def_struct_primitive_debug_ok	( GetShowHiddenOps, 	boolObj, "GetShowHiddenOps");
def_struct_primitive			( SetShowHiddenOps, 	boolObj, "SetShowHiddenOps");
def_struct_primitive_debug_ok	( GetUpdateMode, 		boolObj, "GetUpdateMode");
def_struct_primitive			( SetUpdateMode, 		boolObj, "SetUpdateMode");
def_struct_primitive_debug_ok	( GetOptimize, 			boolObj, "GetOptimize");
def_struct_primitive			( SetOptimize, 			boolObj, "SetOptimize");
#endif

// particle system access
// This functions are not really useful since, the value's set change whenever 
// the time slider is moved
//def_visible_primitive			( SetParticlePos, 			"SetParticlePos");
//def_visible_primitive			( SetParticleVelocity, 		"SetParticleVelocity");
//def_visible_primitive			( SetParticleAge, 			"SetParticleAge");


// trackbar access
def_struct_primitive_debug_ok	( get_trackbar_next_key,	trackbar, "GetNextKeyTime");
def_struct_primitive_debug_ok	( get_trackbar_prev_key,	trackbar, "GetPreviousKeyTime");

// file property access
def_struct_primitive_debug_ok	( fp_num_properties,	fileProperties,	"getNumProperties");
def_struct_primitive_debug_ok	( fp_get_property_name,	fileProperties,	"getPropertyName");
def_struct_primitive_debug_ok	( fp_get_property_val,	fileProperties,	"getPropertyValue");
def_struct_primitive_debug_ok	( fp_get_items,			fileProperties,	"getItems");
def_struct_primitive_debug_ok	( fp_find_property,		fileProperties,	"findProperty");
def_struct_primitive_debug_ok	( fp_add_property,		fileProperties,	"addProperty");
def_struct_primitive_debug_ok	( fp_delete_property,	fileProperties,	"deleteProperty");


//watje
def_visible_primitive			( SetSelectFilter, 			"SetSelectFilter");
def_visible_primitive_debug_ok	( GetSelectFilter, 			"GetSelectFilter");
def_visible_primitive_debug_ok	( GetNumberSelectFilters, 	"GetNumberSelectFilters");

def_visible_primitive			( SetDisplayFilter, 		"SetDisplayFilter");
def_visible_primitive_debug_ok	( GetDisplayFilter, 		"GetDisplayFilter");
def_visible_primitive_debug_ok	( GetNumberDisplayFilters, 	"GetNumberDisplayFilters");

def_visible_primitive_debug_ok	( GetSelectFilterName, 		"GetSelectFilterName");
def_visible_primitive_debug_ok	( GetDisplayFilterName, 	"GetDisplayFilterName");

// LAM: Added these 5/2/01
#ifndef NO_REGIONS
def_visible_primitive			( SetRenderType,	"SetRenderType");
def_visible_primitive_debug_ok	( GetRenderType, 	"GetRenderType");
#endif // NO_REGIONS
