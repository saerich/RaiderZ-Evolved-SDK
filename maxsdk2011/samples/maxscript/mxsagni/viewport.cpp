/**********************************************************************
*<
FILE: viewports.cpp

DESCRIPTION: 

CREATED BY: Simon Feltman

HISTORY: Created 9 December 1998

*>	Copyright (c) 1998, All Rights Reserved.
**********************************************************************/

#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MXSAgni.h"
#include <trig.h>
#include "bitmaps.h"

#include "resource.h"

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
#	include "viewport_wraps.h"

#include "agnidefs.h"

extern Bitmap* CreateBitmapFromBInfo(void **ptr, const int cx, const int cy);

// ============================================================================
static int vpt_lookup[] = {
	MAXCOM_VPT_LEFT,
	MAXCOM_VPT_RIGHT,
	MAXCOM_VPT_TOP,
	MAXCOM_VPT_BOTTOM,
	MAXCOM_VPT_FRONT,
	MAXCOM_VPT_BACK,
	MAXCOM_VPT_ISO_USER,
	MAXCOM_VPT_PERSP_USER,
	MAXCOM_VPT_CAMERA,
	MAXCOM_VPT_GRID,
	-1, // VIEW_NONE
	MAXCOM_VPT_TRACK,
	MAXCOM_VPT_PERSP_USER,
	MAXCOM_VPT_SPOT,
	MAXCOM_VPT_SHAPE,
};


// ============================================================================
Value* VP_SetType_cf(Value **arg_list, int count)
{
	check_arg_count(SetType, 1, count);
	def_view_types();
	// AF -- (04/30/02) Removed the Trackview in a viewport feature ECO #831
#ifndef TRACKVIEW_IN_A_VIEWPORT
	if (arg_list[0] == n_view_track)
		return &false_value;
#endif //TRACKVIEW_IN_A_VIEWPORT
	int cmd = vpt_lookup[GetID(viewTypes, elements(viewTypes), arg_list[0], -1)];
	if(cmd >= 0) {
		MAXScript_interface->ExecuteMAXCommand(cmd);
		return &true_value;
	}

	return &false_value;
}


// ============================================================================
Value* VP_GetType_cf(Value **arg_list, int count)
{
	check_arg_count_with_keys(GetType, 0, count);
	def_view_types();

	Value *indexValue;
	int index = (int_key_arg(index, indexValue, 0))-1;

	if (index<0) index = MAXScript_interface7->getActiveViewportIndex();
	if (index<0) return &undefined;

	ViewExp *vpt = MAXScript_interface7->getViewport(index);
	int type = vpt->GetViewType();
	Value *vptType = GetName(viewTypes, elements(viewTypes), vpt->GetViewType(), &undefined);
	MAXScript_interface->ReleaseViewport(vpt);

	return vptType;
}

// ============================================================================
Value* VP_IsPerspView_cf(Value **arg_list, int count)
{
	check_arg_count(IsPerspView, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	BOOL isPerspView = vpt->IsPerspView();
	MAXScript_interface->ReleaseViewport(vpt);

	return bool_result(isPerspView);
}

// ============================================================================
Value* VP_GetFPS_cf(Value **arg_list, int count)
{
	check_arg_count(GetFPS, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;

	ViewExp *vp = MAXScript_interface->GetActiveViewport();
	float fps = 0.0f;

	if (vp)
	{
		ViewExp10* vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));

		if (vp10) 
		{
			fps = vp10->GetViewportFPS();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}

	return Float::intern(fps);
}





Value* VP_GetClipScale_cf(Value **arg_list, int count)
{
	check_arg_count(GetClipScale, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);

	float clipScale = 0.0f;
	if (vp)
	{
		ViewExp10* vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		if (vp10)
			clipScale = vp10->GetViewportClipScale();
		MAXScript_interface->ReleaseViewport(vp);
	}

	return Float::intern(clipScale);
}

Value* VP_SetClipScale_cf(Value **arg_list, int count)
{
	// viewport.setfov <float>
	check_arg_count(SetClipScale, 1, count);
	float clipScale = arg_list[0]->to_float();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			// SetFOV only allows changes to a viewport if VIEW_PERSP_USER or VIEW_CAMERA.
			vp10->SetViewportClipScale(clipScale);
			result = &true_value;
		}		
		MAXScript_interface->ReleaseViewport(vp);
	}

	
	return result;
}


// ============================================================================
Value* VP_SetTM_cf(Value **arg_list, int count)
{
	check_arg_count(SetTM, 1, count);

	BOOL result = FALSE;
	Matrix3 tm = arg_list[0]->to_matrix3();

	ViewExp* vpt = MAXScript_interface->GetActiveViewport();
	if(vpt)
	{
		result = vpt->SetAffineTM(tm);
		MAXScript_interface->ReleaseViewport(vpt);
	}

	if(result)
	{
		needs_complete_redraw_set();
		return &true_value;
	}

	return &false_value;
}


// ============================================================================
Value* VP_GetTM_cf(Value **arg_list, int count)
{
	check_arg_count(GetTM, 0, count);

	Matrix3 tm;
	tm.IdentityMatrix();

	ViewExp* vpt = MAXScript_interface->GetActiveViewport();
	if(vpt)
	{
		vpt->GetAffineTM(tm);
		MAXScript_interface->ReleaseViewport(vpt);
	}

	return new Matrix3Value(tm);
}


// ============================================================================
Value* VP_SetCamera_cf(Value **arg_list, int count)
{
	check_arg_count(SetCamera, 1, count);

	INode *camnode = arg_list[0]->to_node();
	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	if(vpt && camnode)
	{
		ObjectState os = camnode->EvalWorldState(MAXScript_interface->GetTime());
		if(os.obj && os.obj->SuperClassID() == CAMERA_CLASS_ID)
		{
			vpt->SetViewCamera(camnode);
			needs_complete_redraw_set();
		}
		else if(os.obj && os.obj->SuperClassID() == LIGHT_CLASS_ID) // LAM - 9/5/01
		{
			LightState ls;
			Interval iv;

			((LightObject *)os.obj)->EvalLightState(MAXScript_interface->GetTime(),iv,&ls);
			if (ls.type == SPOT_LGT || ls.type == DIRECT_LGT)
			{
				vpt->SetViewSpot(camnode);
				needs_complete_redraw_set();
			}
		}
	}
	if (vpt)
		MAXScript_interface->ReleaseViewport(vpt);

	return &ok;
}


//  =================================================

Value*
VP_CanSetToViewport_cf(Value** arg_list, int count) 
{
	check_arg_count(canSetToViewport, 1, count);
	Value* ret = &false_value;

	INode *camnode = arg_list[0]->to_node();
	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	if(vpt && camnode)
	{
		ObjectState os = camnode->EvalWorldState(MAXScript_interface->GetTime());
		if(os.obj && os.obj->SuperClassID() == CAMERA_CLASS_ID)
		{
			ret = &true_value;
		}
		else if(os.obj && os.obj->SuperClassID() == LIGHT_CLASS_ID) // LAM - 9/5/01
		{
			LightState ls;
			Interval iv;

			((LightObject *)os.obj)->EvalLightState(MAXScript_interface->GetTime(),iv,&ls);
			if (ls.type == SPOT_LGT || ls.type == DIRECT_LGT)
			{
				ret = &true_value;
			}
		}
	}

	if (vpt)
		MAXScript_interface->ReleaseViewport(vpt);

	return ret;
}


// ============================================================================
Value* VP_GetCamera_cf(Value **arg_list, int count)
{
	check_arg_count_with_keys(GetCamera, 0, count);

	Value *indexValue;
	int index = (int_key_arg(index, indexValue, 0))-1;

	if (index<0) index = MAXScript_interface7->getActiveViewportIndex();
	if (index<0) return &undefined;

	INode *camnode = NULL;
	ViewExp *vpt = MAXScript_interface7->getViewport(index);
	if(vpt)
	{
		camnode = vpt->GetViewCamera();
		if (camnode == NULL)	// LAM - 9/5/01
			camnode = vpt->GetViewSpot();
		MAXScript_interface->ReleaseViewport(vpt);
	}

	if(camnode) return MAXNode::intern(camnode);
	return &undefined;
}


// ============================================================================
Value* VP_SetLayout_cf(Value **arg_list, int count)
{
	check_arg_count(SetLayout, 1, count);
	def_vp_layouts();

	int layoutType = GetID(vpLayouts, elements(vpLayouts), arg_list[0], -1);
	if(layoutType != -1)
	{
		MAXScript_interface->SetViewportLayout(layoutType);
		needs_redraw_set();
	}

	return &ok;
}


// ============================================================================
Value* VP_GetLayout_cf(Value **arg_list, int count)
{
	check_arg_count(GetLayout, 0, count);
	def_vp_layouts();

	Value *layoutKey = GetName(vpLayouts,
		elements(vpLayouts),
		MAXScript_interface->GetViewportLayout(),
		&undefined);
	return layoutKey;
}

// ============================================================================
Value* VP_SetRenderLevel_cf(Value **arg_list, int count)
{
	check_arg_count(SetRenderLevel, 1, count);
	def_vp_renderlevel();

	int renderLevel = GetID(vpRenderLevel, elements(vpRenderLevel), arg_list[0], -1);
	if(renderLevel != -1)
	{
		MAXScript_interface7->SetActiveViewportRenderLevel(renderLevel);
		needs_redraw_set();
	}

	return &ok;
}


// ============================================================================
Value* VP_GetRenderLevel_cf(Value **arg_list, int count)
{
	check_arg_count(GetRenderLevel, 0, count);
	def_vp_renderlevel();

	Value *renderLevelKey = GetName(vpRenderLevel,
		elements(vpRenderLevel),
		MAXScript_interface7->GetActiveViewportRenderLevel(),
		&undefined);
	return renderLevelKey;
}

// ============================================================================
Value* VP_SetShowEdgeFaces_cf(Value **arg_list, int count)
{
	check_arg_count(SetShowEdgeFaces, 1, count);

	BOOL show = arg_list[0]->to_bool();

	MAXScript_interface7->SetActiveViewportShowEdgeFaces(show);
	needs_redraw_set();

	return &ok;
}


// ============================================================================
Value* VP_GetShowEdgeFaces_cf(Value **arg_list, int count)
{
	check_arg_count(GetShowEdgeFaces, 0, count);

	BOOL show = MAXScript_interface7->GetActiveViewportShowEdgeFaces();

	return bool_result(show);
}

// ============================================================================
Value* VP_SetTransparencyLevel_cf(Value **arg_list, int count)
{
	check_arg_count(SetTransparencyLevel, 1, count);

	int level = arg_list[0]->to_int()-1;

	MAXScript_interface7->SetActiveViewportTransparencyLevel(level);
	needs_redraw_set();

	return &ok;
}


// ============================================================================
Value* VP_GetTransparencyLevel_cf(Value **arg_list, int count)
{
	check_arg_count(GetTransparencyLevel, 0, count);

	int level = MAXScript_interface7->GetActiveViewportTransparencyLevel()+1;

	return Integer::intern(level);
}


// RK: Start

Value* 
VP_SetActiveViewport(Value *val)
{
	int index = val->to_int();
	if (index < 1 || index > MAXScript_interface7->getNumViewports())
		throw RuntimeError(GetString(IDS_RK_ACTIVEVIEWPORT_INDEX_OUT_OF_RANGE));
	MAXScript_interface7->setActiveViewport(index-1);
	return val;
}

Value* 
VP_GetActiveViewport()
{
	return Integer::intern(MAXScript_interface7->getActiveViewportIndex()+1);
}

Value* VP_SetNumViews(Value* val)
{
	throw RuntimeError (GetString(IDS_RK_CANNOT_SET_NUMVIEWS));
	return NULL;
}

Value* 
VP_GetNumViews()
{
	return Integer::intern(MAXScript_interface7->getNumViewports());
}

Value* VP_ResetAllViews_cf(Value **arg_list, int count)
{
	check_arg_count(resetAllViews, 0, count);
	MAXScript_interface7->resetAllViews();
	return &ok;
}

// RK: End

Value* 
VP_ZoomToBounds_cf(Value **arg_list, int count)
{
	check_arg_count(ZoomToBounds, 3, count);
	BOOL all = arg_list[0]->to_bool();
	Point3 a = arg_list[1]->to_point3();
	Point3 b = arg_list[2]->to_point3();
	Box3 box(a,b);
	MAXScript_interface->ZoomToBounds(all,box);

	return &ok;
}

// LAM: Start

#ifndef NO_REGIONS
Value* 
VP_SetRegionRect_cf(Value **arg_list, int count)
{
	check_arg_count_with_keys(SetRegionRect, 2, count);
	int index = arg_list[0]->to_int();
	if (index < 1 || index > MAXScript_interface7->getNumViewports())
		throw RuntimeError(GetString(IDS_RK_ACTIVEVIEWPORT_INDEX_OUT_OF_RANGE));
	Rect region = arg_list[1]->to_box2();
	Value* tmp;
	BOOL byPixel = bool_key_arg(byPixel, tmp, TRUE); 
	if (byPixel) 
		MAXScript_interface7->SetRegionRect2(index-1,region);
	else
		MAXScript_interface7->SetRegionRect(index-1,region);
	return &ok;
}

Value* 
VP_GetRegionRect_cf(Value **arg_list, int count)
{
	check_arg_count_with_keys(GetRegionRect, 1, count);
	int index = arg_list[0]->to_int();
	if (index < 1 || index > MAXScript_interface7->getNumViewports())
		throw RuntimeError(GetString(IDS_RK_ACTIVEVIEWPORT_INDEX_OUT_OF_RANGE));
	Rect region;
	Value* tmp;
	BOOL byPixel = bool_key_arg(byPixel, tmp, TRUE); 
	if (byPixel) 
		region = MAXScript_interface7->GetRegionRect2(index-1);
	else
		region = MAXScript_interface7->GetRegionRect(index-1);
	return new Box2Value(region);
}

Value* 
VP_SetBlowupRect_cf(Value **arg_list, int count)
{
	check_arg_count_with_keys(SetBlowupRect, 2, count);
	int index = arg_list[0]->to_int();
	if (index < 1 || index > MAXScript_interface7->getNumViewports())
		throw RuntimeError(GetString(IDS_RK_ACTIVEVIEWPORT_INDEX_OUT_OF_RANGE));
	Rect region = arg_list[1]->to_box2();
	Value* tmp;
	BOOL byPixel = bool_key_arg(byPixel, tmp, TRUE); 
	if (byPixel) 
		MAXScript_interface7->SetBlowupRect2(index-1,region);
	else
		MAXScript_interface7->SetBlowupRect(index-1,region);
	return &ok;
}

Value* 
VP_GetBlowupRect_cf(Value **arg_list, int count)
{
	check_arg_count_with_keys(GetBlowupRect, 1, count);
	int index = arg_list[0]->to_int();
	if (index < 1 || index > MAXScript_interface7->getNumViewports())
		throw RuntimeError(GetString(IDS_RK_ACTIVEVIEWPORT_INDEX_OUT_OF_RANGE));
	Rect region;
	Value* tmp;
	BOOL byPixel = bool_key_arg(byPixel, tmp, TRUE); 
	if (byPixel) 
		region = MAXScript_interface7->GetBlowupRect2(index-1);
	else
		region = MAXScript_interface7->GetBlowupRect(index-1);
	return new Box2Value(region);
}
#endif //NO_REGIONS

Value* 
VP_SetGridVisibility_cf(Value **arg_list, int count)
{
	check_arg_count(setGridVisibility, 2, count);
	BOOL state = arg_list[1]->to_bool();
	if (arg_list[0] == n_all)
	{
		for (int index = 0; index < MAXScript_interface7->getNumViewports(); index++)
			MAXScript_interface7->SetViewportGridVisible(index,state);
	}
	else
	{
		int index = arg_list[0]->to_int();
		if (index < 1 || index > MAXScript_interface7->getNumViewports())
			throw RuntimeError(GetString(IDS_RK_ACTIVEVIEWPORT_INDEX_OUT_OF_RANGE));
		MAXScript_interface7->SetViewportGridVisible(index-1,state);
	}
	needs_redraw_set();
	return &ok;
}

Value* 
VP_GetGridVisibility_cf(Value **arg_list, int count)
{
	check_arg_count(getGridVisibility, 1, count);
	int index = arg_list[0]->to_int();
	if (index < 1 || index > MAXScript_interface7->getNumViewports())
		throw RuntimeError(GetString(IDS_RK_ACTIVEVIEWPORT_INDEX_OUT_OF_RANGE));
	return bool_result (MAXScript_interface7->GetViewportGridVisible(index-1));
}

// LAM: End

/*
// ============================================================================
Value* VP_IsActive_cf(Value **arg_list, int count)
{
check_arg_count(IsActive, 0, count);

ViewExp *vpt = MAXScript_interface->GetActiveViewport();
BOOL isActive = vpt->IsActive();
MAXScript_interface->ReleaseViewport(vpt);

return (isActive) ? &true_value : &false_value;
}
*/

// ============================================================================
Value* VP_IsEnabled_cf(Value **arg_list, int count)
{
	check_arg_count(IsEnabled, 0, count);

	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	BOOL isEnabled = vpt->IsEnabled();
	MAXScript_interface->ReleaseViewport(vpt);

	return (isEnabled) ? &true_value : &false_value;
}


// ============================================================================
Value* VP_GetFOV_cf(Value **arg_list, int count)
{
	// viewport.getfov()
	check_arg_count(VP_GetFOV, 0, count);

	ViewExp* vpt = MAXScript_interface->GetActiveViewport();
	float fov = vpt->GetFOV() * (180.f/PI);
	MAXScript_interface->ReleaseViewport(vpt);

	return Float::intern(fov);
}

Value* VP_SetFOV_cf(Value **arg_list, int count)
{
	// viewport.setfov <float>
	check_arg_count(VP_SetFOV, 1, count);
	float fov = arg_list[0]->to_float();
	MXS_range_check(fov, 0.0f, 360.0f, "Wanted a value between");
	Value* result = &false_value;
	// Field of View is in Degrees. Can only be between zero and 360 degrees.
	if (fov >= 0.00f && fov <= 360.0f)
	{
		ViewExp* vp = MAXScript_interface->GetActiveViewport();
		DbgAssert(NULL != vp);
		if (NULL != vp)
		{
			HWND hwnd = vp->GetHWnd();
			ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
			DbgAssert(NULL != vp10);
			if (NULL != vp10 && hwnd != NULL)
			{
				// SetFOV only allows changes to a viewport if VIEW_PERSP_USER or VIEW_CAMERA.
				float radiansFOV = DegToRad(fov);
				BOOL setFOVResult = vp10->SetFOV(radiansFOV); // Only a ViewExp10 class has this SetFOV method.
				if (setFOVResult) 
				{
					result = &true_value;
				}
			}
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	
	return result;
}

Value* VP_Pan_cf(Value **arg_list, int count)
{
	// viewport.pan <x> <y>
	check_arg_count(VP_Pan, 2, count);
	float x = arg_list[0]->to_float();
	float y = arg_list[1]->to_float();
	
	Value* returnResult = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10)
		{
			Point2 pt(x, y);
			vp10->Pan(pt);
			returnResult = &true_value;
		}
	}
	MAXScript_interface->ReleaseViewport(vp);
	return returnResult;
}

Value* VP_Zoom_cf(Value **arg_list, int count)
{
	// <boolean> viewport.zoom <float>
	// Equivalent to the Zoom Region tool in the UI.
	// This is also equivalent to what the system calls when the interactive Zoom tool
	// is used on an orthographic or Isometric User viewport.
	check_arg_count(VP_Zoom, 1, count);
	float zoomfactor = arg_list[0]->to_float();
	Value* returnResult = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10)
		{
			vp10->Zoom(zoomfactor);
			returnResult = &true_value;
		}
	}
	MAXScript_interface->ReleaseViewport(vp);
	return returnResult;
}

Value* VP_ZoomPerspective_cf(Value **arg_list, int count)
{
	// <boolean> viewport.zoomPerspective <float>
	// Exactly Equivalent to the Interactive zoom tool in the UI. This method only works for
	// Perspective views. In the UI when a user zooms in an orthographic or Isometric User view, 
	// internally, a switch is performed on the viewport type, and instead a call to ViewExp::Zoom is made.
	// Doesn't work for camera views. Implemented here to show users how the code
	// works internally.
	// Returns true if successsful or false otherwise.
	check_arg_count(VP_Zoom, 1, count);
	float dist = arg_list[0]->to_float();
	Value* returnResult = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10)
		{
			ULONG viewportType = vp10->GetViewType();
			if (vp10->IsPerspView() && viewportType != VIEW_CAMERA)
			{
				Matrix3 stm;
				vp10->GetAffineTM(stm);
				float diff = vp->GetFocalDist() - dist;
				stm.Translate(Point3(0.0f, 0.0f, diff));
				vp10->SetAffineTM(stm);
				vp10->SetFocalDistance(dist);
				returnResult = &true_value;
			}
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	
	return returnResult;
}

Value* VP_SetFocalDistance_cf(Value **arg_list, int count)
{
	// <boolean> viewport.setfocaldistance <float>
	// Returns true if successsful or false otherwise.
	check_arg_count(VP_Zoom, 1, count);
	float zoomfactor = arg_list[0]->to_float();
	Value* returnResult = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10)
		{
			ULONG viewportType = vp10->GetViewType();
			if (vp10->IsPerspView() && viewportType != VIEW_CAMERA)
			{
				vp10->SetFocalDistance(zoomfactor);
				returnResult = &true_value;
			}
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	
	return returnResult;
}

Value* VP_GetFocalDistance_cf(Value **arg_list, int count)
{
	// Set Focal Distance is equivalent to the Interactive zoom in the UI
	// viewport.getfocaldistance()
	// returns the set focal distance, or undefined otherwise.
	check_arg_count(VP_Zoom, 0, count);
	float focalDistance = 0.0f;
	Value* returnResult = &undefined;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10)
		{
			focalDistance = vp10->GetFocalDist();
			returnResult = Float::intern(focalDistance);
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	
	return returnResult;
}

Value* VP_Rotate_cf(Value **arg_list, int count)
{
	// <boolean> viewport.rotate <quat> [center: point2]
	check_arg_count_with_keys(VP_Rotate, 1, count);

	Quat quat = arg_list[0]->to_quat();
	
	Value* returnResult = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10)
		{
			Value* vCenterPt = key_arg_or_default(center, &unsupplied);
			if (vCenterPt == &unsupplied)
			{
				vp10->Rotate(quat);
			}
			else
			{
				Point3 pt3 =  vCenterPt->to_point3();
				vp10->Rotate(quat, pt3);
			}
			returnResult = &true_value;
		}
	}
	MAXScript_interface->ReleaseViewport(vp);
	return returnResult;
}

// ============================================================================
Value* VP_GetScreenScaleFactor_cf(Value **arg_list, int count)
{
	check_arg_count(VP_GetScreenScaleFactor, 1, count);

	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	float screenScaleFactor = vpt->GetScreenScaleFactor(arg_list[0]->to_point3());
	MAXScript_interface->ReleaseViewport(vpt);

	return Float::intern(screenScaleFactor);
}



// ============================================================================
Value* VP_IsWire_cf(Value **arg_list, int count)
{
	check_arg_count(IsWire, 0, count);

	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	BOOL isWire = vpt->IsWire();
	MAXScript_interface->ReleaseViewport(vpt);

	return (isWire) ? &true_value : &false_value;
}
/*
Value* 
VP_SetGridSize(Value *val)
{
float size = val->to_float();
ViewExp *vpt = MAXScript_interface->GetActiveViewport();
vpt->SetGridSize(size);
MAXScript_interface->ReleaseViewport(vpt);
needs_redraw_set();
return val;
}

Value* 
VP_GetGridSize()
{
ViewExp *vpt = MAXScript_interface->GetActiveViewport();
float size = vpt->GetGridSize();
MAXScript_interface->ReleaseViewport(vpt);
return Float::intern(size);
}
*/
Value* 
VP_SetBkgImageDsp(Value *val)
{
	BOOL on = val->to_bool();
	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	vpt->setBkgImageDsp(on);
	MAXScript_interface->ReleaseViewport(vpt);
	needs_redraw_set();
	return val;
}

Value* 
VP_GetBkgImageDsp()
{
	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	BOOL on = vpt->getBkgImageDsp();
	MAXScript_interface->ReleaseViewport(vpt);
	return bool_result(on);
}

Value* VP_Getviewportdib_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getViewportDib, 0, count);
	ViewExp* vpt = MAXScript_interface->GetActiveViewport();
	DbgAssert(vpt != NULL);
	if (vpt == NULL) return &undefined;

	GraphicsWindow *gw = vpt->getGW();

	int dibSize;
	gw->getDIB(NULL, &dibSize);
	BITMAPINFO *bmi = (BITMAPINFO *)malloc(dibSize);
	BITMAPINFOHEADER *bmih = (BITMAPINFOHEADER *)bmi;
	gw->getDIB(bmi, &dibSize);

	HWND gw_hwnd = gw->getHWnd();
	HDC hdc = GetDC(gw_hwnd);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdc,bmih->biWidth,bmih->biHeight);

	HDC hdcMem = CreateCompatibleDC(hdc);
	HGDIOBJ hOldB = SelectObject(hdcMem,hBitmap);
	BitBlt(hdcMem, 0, 0, bmih->biWidth, bmih->biHeight, hdc, 0, 0, SRCCOPY);

	BYTE *buf = new BYTE[bmih->biWidth * bmih->biHeight * (bmih->biBitCount / 8) + sizeof(BITMAPINFOHEADER)];

	if (buf)
	{
		if (!GetDIBits(hdcMem, hBitmap, 0, bmih->biHeight, &buf[sizeof(BITMAPINFOHEADER)], bmi, DIB_RGB_COLORS))
		{
			delete [] buf;
			free(bmi);
			DeleteDC(hdcMem);
			ReleaseDC(gw_hwnd, hdc);
			MAXScript_interface->ReleaseViewport(vpt);
			return &undefined;
		}			
		memcpy(buf, bmih, sizeof(BITMAPINFOHEADER));
	}

	Bitmap *map = CreateBitmapFromBInfo((void**)&buf, bmih->biWidth, bmih->biHeight);
	delete [] buf;
	free(bmi);
	DeleteDC(hdcMem);
	ReleaseDC(gw_hwnd, hdc);
	one_typed_value_local(MAXBitMap* mbm);
	vl.mbm = new MAXBitMap ();
	vl.mbm->bi.CopyImageInfo(&map->Storage()->bi);
	vl.mbm->bi.SetFirstFrame(0);
	vl.mbm->bi.SetLastFrame(0);
	vl.mbm->bi.SetName("");
	vl.mbm->bi.SetDevice("");
	if (vl.mbm->bi.Type() > BMM_TRUE_64)
		vl.mbm->bi.SetType(BMM_TRUE_64);
	vl.mbm->bm = map;
	MAXScript_interface->ReleaseViewport(vpt);
	return_value(vl.mbm);
}

Value* VP_GetAdaptiveDegFPS_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegFPS, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	ViewExp *vpt = MAXScript_interface->GetActiveViewport();

	float fps = vpt->GetFPS();

	MAXScript_interface->ReleaseViewport(vpt);

	return Float::intern(fps);
}


Value* VP_SetAdaptiveDegFPS_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegFPS, 1, count);
	float fps = arg_list[0]->to_float();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegGoalFPS(fps);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}


Value* VP_GetAdaptiveDegDisplayModeCurrent_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegDisplayModeCurrent, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	BOOL mode = FALSE;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			mode = vp10->GetAdaptiveDegDisplayModeCurrent();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return bool_result(mode);
}


Value* VP_SetAdaptiveDegDisplayModeCurrent_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegDisplayModeCurrent, 1, count);
	BOOL mode = arg_list[0]->to_bool();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegDisplayModeCurrent(mode);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}


Value* VP_GetAdaptiveDegDisplayModeFastShaded_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegDisplayModeFastShaded, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	BOOL mode = FALSE;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			mode = vp10->GetAdaptiveDegDisplayModeFastShaded();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return bool_result(mode);
}


Value* VP_SetAdaptiveDegDisplayModeFastShaded_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegDisplayModeFastShaded, 1, count);
	BOOL mode = arg_list[0]->to_bool();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegDisplayModeFastShaded(mode);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}


Value* VP_GetAdaptiveDegDisplayModeWire_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegDisplayModeWire, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	BOOL mode = FALSE;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			mode = vp10->GetAdaptiveDegDisplayModeWire();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return bool_result(mode);
}


Value* VP_SetAdaptiveDegDisplayModeWire_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegDisplayModeWire, 1, count);
	BOOL mode = arg_list[0]->to_bool();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegDisplayModeWire(mode);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}


Value* VP_GetAdaptiveDegDisplayModeBox_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegDisplayModeBox, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	BOOL mode = FALSE;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			mode = vp10->GetAdaptiveDegDisplayModeBox();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return bool_result(mode);
}


Value* VP_SetAdaptiveDegDisplayModeBox_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegDisplayModeBox, 1, count);
	BOOL mode = arg_list[0]->to_bool();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegDisplayModeBox(mode);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}

Value* VP_GetAdaptiveDegDisplayModePoint_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegDisplayModePoint, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	BOOL mode = FALSE;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			mode = vp10->GetAdaptiveDegDisplayModePoint();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return bool_result(mode);
}


Value* VP_SetAdaptiveDegDisplayModePoint_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegDisplayModePoint, 1, count);
	BOOL mode = arg_list[0]->to_bool();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegDisplayModePoint(mode);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}

Value* VP_GetAdaptiveDegDisplayModeHide_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegDisplayModeHide, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	BOOL mode = FALSE;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			mode = vp10->GetAdaptiveDegDisplayModeHide();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return bool_result(mode);
}


Value* VP_SetAdaptiveDegDisplayModeHide_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegDisplayModeHide, 1, count);
	BOOL mode = arg_list[0]->to_bool();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegDisplayModeHide(mode);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}

 
Value* VP_GetAdaptiveDegDrawBackface_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegDrawBackface, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	BOOL mode = FALSE;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			mode = vp10->GetAdaptiveDegDrawBackface();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return bool_result(mode);
}


Value* VP_SetAdaptiveDegDrawBackface_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegDrawBackface, 1, count);
	BOOL draw = arg_list[0]->to_bool();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegDrawBackface(draw);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}


Value* VP_GetAdaptiveDegNeverDegradeSelected_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegNeverDegradeSelected, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	BOOL mode = FALSE;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			mode = vp10->GetAdaptiveDegNeverDegradeSelected();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return bool_result(mode);
}


Value* VP_SetAdaptiveDegNeverDegradeSelected_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegNeverDegradeSelected, 1, count);
	BOOL degrade = arg_list[0]->to_bool();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegNeverDegradeSelected(degrade);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}

Value* VP_GetAdaptiveDegDegradeLight_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegDegradeLight, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	BOOL mode = FALSE;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			mode = vp10->GetAdaptiveDegDegradeLight();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return bool_result(mode);
}


Value* VP_SetAdaptiveDegDegradeLight_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegDegradeLight, 1, count);
	BOOL degrade = arg_list[0]->to_bool();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegDegradeLight(degrade);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}

Value* VP_GetAdaptiveDegNeverRedrawAfterDegrade_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegNeverRedrawAfterDegrade, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	BOOL mode = FALSE;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			mode = vp10->GetAdaptiveDegNeverRedrawAfterDegrade();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return bool_result(mode);
}


Value* VP_SetAdaptiveDegNeverRedrawAfterDegrade_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegNeverRedrawAfterDegrade, 1, count);
	BOOL degrade = arg_list[0]->to_bool();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegNeverRedrawAfterDegrade(degrade);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}

Value* VP_GetAdaptiveDegCameraDistancePriority_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegCameraDistancePriority, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;

	float dist = 0.0f;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			dist = vp10->GetAdaptiveDegCameraDistancePriority();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return Float::intern(dist);
}


Value* VP_SetAdaptiveDegCameraDistancePriority_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegCameraDistancePriority, 1, count);
	float dist = arg_list[0]->to_float();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegCameraDistancePriority(dist);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}

Value* VP_GetAdaptiveDegScreenSizePriority_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegScreenSizePriority, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;
	
	float size = 0.0f;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			size = vp10->GetAdaptiveDegScreenSizePriority();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return Float::intern(size);
}


Value* VP_SetAdaptiveDegScreenSizePriority_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegScreenSizePriority, 1, count);
	float size = arg_list[0]->to_float();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegScreenSizePriority(size);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}

Value* VP_GetAdaptiveDegMinSize_cf(Value **arg_list, int count)
{
	check_arg_count(GetAdaptiveDegMinSize, 0, count);

	if (MAXScript_interface7->getActiveViewportIndex() < 0) return &undefined;

	int size = 0;
	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			size = vp10->GetAdaptiveDegMinSize();
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return Integer::intern(size);
}


Value* VP_SetAdaptiveDegMinSize_cf(Value **arg_list, int count)
{
	check_arg_count(SetAdaptiveDegMinSize, 1, count);
	int size = arg_list[0]->to_int();

	Value* result = &false_value;

	ViewExp* vp = MAXScript_interface->GetActiveViewport();
	DbgAssert(NULL != vp);
	if (NULL != vp)
	{
		HWND hwnd = vp->GetHWnd();
		ViewExp10* vp10 = vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		DbgAssert(NULL != vp10);
		if (NULL != vp10 && hwnd != NULL)
		{
			vp10->SetAdaptiveDegMinSize(size);
			result = &true_value;
		}
		MAXScript_interface->ReleaseViewport(vp);
	}
	return result;
}

Value* VP_GetID_cf( Value **arg_list, int count )
{
	check_arg_count(GetID, 1, count);
	int index = (arg_list[0]->to_int() - 1);
	
	if (index<0) index = MAXScript_interface7->getActiveViewportIndex();
	if (index<0) return &undefined;

	int viewID = -1;
	ViewExp *vpt = MAXScript_interface7->getViewport(index);
	if(vpt)
	{
		// Increment by 1, for consistency with other functions.
		// Note the function Interface11::GetRendViewIndex() returns an ID value, not an index.
		// But maxscript's get_rend_viewIndex() adds one to the value, treating it as an index.
		// The ID is actually the same as the index except in certain cases, like when a viewport is maximized.
		viewID = vpt->Execute( ViewExp::kEXECUTE_GET_VIEWPORT_ID ) + 1;
		MAXScript_interface->ReleaseViewport(vpt);
	}

	if(viewID>=0) return Integer::intern(viewID);
	return &undefined;
}

/* --------------------- plug-in init --------------------------------- */
// this is called by the dlx initializer, register the global vars here
void viewport_init()
{
#include "viewport_glbls.h"
}



