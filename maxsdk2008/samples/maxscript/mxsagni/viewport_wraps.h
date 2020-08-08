/* ViewExp Class */
def_struct_primitive			( VP_SetType,			viewport,		"SetType");
def_struct_primitive_debug_ok	( VP_GetType,			viewport,		"GetType");
def_struct_primitive			( VP_SetTM,				viewport,		"SetTM");
def_struct_primitive_debug_ok	( VP_GetTM,				viewport,		"GetTM");
def_struct_primitive			( VP_SetCamera,			viewport,		"SetCamera");
def_struct_primitive_debug_ok	( VP_GetCamera,			viewport,		"GetCamera");
def_struct_primitive			( VP_SetLayout,			viewport,		"SetLayout");
def_struct_primitive_debug_ok	( VP_GetLayout,			viewport,		"GetLayout");

def_struct_primitive			( VP_SetRenderLevel,	viewport,		"SetRenderLevel");
def_struct_primitive_debug_ok	( VP_GetRenderLevel,	viewport,		"GetRenderLevel");

def_struct_primitive			( VP_SetShowEdgeFaces,	viewport,		"SetShowEdgeFaces");
def_struct_primitive_debug_ok	( VP_GetShowEdgeFaces,	viewport,		"GetShowEdgeFaces");

def_struct_primitive			( VP_SetTransparencyLevel, viewport,	"SetTransparencyLevel");
def_struct_primitive_debug_ok	( VP_GetTransparencyLevel, viewport,	"GetTransparencyLevel");

// LAM: Added these
#ifndef NO_REGIONS
def_struct_primitive			( VP_SetRegionRect,		viewport,		"SetRegionRect");
def_struct_primitive_debug_ok	( VP_GetRegionRect,		viewport,		"GetRegionRect");
def_struct_primitive			( VP_SetBlowupRect,		viewport,		"SetBlowupRect");
def_struct_primitive_debug_ok	( VP_GetBlowupRect,		viewport,		"GetBlowupRect");
#endif // NO_REGIONS

def_struct_primitive			( VP_SetGridVisibility,	viewport,		"SetGridVisibility");
def_struct_primitive_debug_ok	( VP_GetGridVisibility,	viewport,		"GetGridVisibility");
def_struct_primitive			( VP_Getviewportdib,	viewport,		"getViewportDib");

// RK: Added this
def_struct_primitive			( VP_ResetAllViews,		viewport,		"resetAllViews");
//watje
def_struct_primitive			( VP_ZoomToBounds,		viewport,		"ZoomToBounds");

// David Cunnigham: 
def_struct_primitive_debug_ok	( VP_CanSetToViewport,	viewport,		"CanSetToViewport");

def_struct_primitive_debug_ok	( VP_IsEnabled,			viewport,		"IsEnabled");

def_struct_primitive_debug_ok	( VP_GetFOV,			viewport,		"GetFOV");
def_struct_primitive_debug_ok	( VP_SetFOV,			viewport,		"SetFOV");
def_struct_primitive_debug_ok	( VP_Pan,				viewport,		"Pan");
def_struct_primitive_debug_ok	( VP_Zoom,				viewport,		"Zoom");
def_struct_primitive_debug_ok	( VP_ZoomPerspective,  viewport,       "ZoomPerspective");
def_struct_primitive_debug_ok	( VP_GetFocalDistance,  viewport,       "GetFocalDistance");
def_struct_primitive_debug_ok	( VP_SetFocalDistance,  viewport,       "SetFocalDistance");

def_struct_primitive_debug_ok	( VP_Rotate,			viewport,		"Rotate");
def_struct_primitive_debug_ok	( VP_GetScreenScaleFactor,	viewport,		"GetScreenScaleFactor");

def_struct_primitive_debug_ok	( VP_IsPerspView,		viewport,		"IsPerspView");

def_struct_primitive_debug_ok	( VP_IsWire,			viewport,		"IsWire");

def_struct_primitive_debug_ok	( VP_GetFPS,			viewport,		"GetFPS");


def_struct_primitive_debug_ok	( VP_GetClipScale,			viewport,		"GetClipScale");
def_struct_primitive_debug_ok	( VP_SetClipScale,			viewport,		"SetClipScale");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegFPS,			viewport,		"GetAdaptiveDegGoalFPS");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegFPS,			viewport,		"SetAdaptiveDegGoalFPS");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegDisplayModeCurrent,			viewport,		"GetAdaptiveDegDisplayModeCurrent");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegDisplayModeCurrent,			viewport,		"SetAdaptiveDegDisplayModeCurrent");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegDisplayModeFastShaded,			viewport,		"GetAdaptiveDegDisplayModeFastShaded");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegDisplayModeFastShaded,			viewport,		"SetAdaptiveDegDisplayModeFastShaded");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegDisplayModeWire,			viewport,		"GetAdaptiveDegDisplayModeWire");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegDisplayModeWire,			viewport,		"SetAdaptiveDegDisplayModeWire");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegDisplayModeBox,			viewport,		"GetAdaptiveDegDisplayModeBox");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegDisplayModeBox,			viewport,		"SetAdaptiveDegDisplayModeBox");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegDisplayModePoint,			viewport,		"GetAdaptiveDegDisplayModePoint");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegDisplayModePoint,			viewport,		"SetAdaptiveDegDisplayModePoint");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegDisplayModeHide,			viewport,		"GetAdaptiveDegDisplayModeHide");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegDisplayModeHide,			viewport,		"SetAdaptiveDegDisplayModeHide");


def_struct_primitive_debug_ok	( VP_GetAdaptiveDegDrawBackface,			viewport,		"GetAdaptiveDegDrawBackfaces");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegDrawBackface,			viewport,		"SetAdaptiveDegDrawBackfaces");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegNeverDegradeSelected,			viewport,		"GetAdaptiveDegNeverDegradeSelected");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegNeverDegradeSelected,			viewport,		"SetAdaptiveDegNeverDegradeSelected");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegDegradeLight,			viewport,		"GetAdaptiveDegDegradeLight");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegDegradeLight,			viewport,		"SetAdaptiveDegDegradeLight");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegNeverRedrawAfterDegrade,			viewport,		"GetAdaptiveDegNeverRedrawAfterDegrade");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegNeverRedrawAfterDegrade,			viewport,		"SetAdaptiveDegNeverRedrawAfterDegrade");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegCameraDistancePriority,			viewport,		"GetAdaptiveDegCameraDistancePriority");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegCameraDistancePriority,			viewport,		"SetAdaptiveDegCameraDistancePriority");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegScreenSizePriority,			viewport,		"GetAdaptiveDegScreenSizePriority");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegScreenSizePriority,			viewport,		"SetAdaptiveDegScreenSizePriority");

def_struct_primitive_debug_ok	( VP_GetAdaptiveDegMinSize,			viewport,		"GetAdaptiveDegMinSize");
def_struct_primitive_debug_ok	( VP_SetAdaptiveDegMinSize,			viewport,		"SetAdaptiveDegMinSize");

