// from avg_dlx
#include "avg_maxver.h" // defines MAGMA 

define_system_global( "listener",			get_listener,			NULL);
define_system_global( "macroRecorder",		get_macroRecorder,		NULL);
define_system_global( "superclasses",		get_superclasses,		NULL);
#ifndef MAGMA
define_system_global( "avguard_dlx_ver",	get_avguard_dlx_ver,	NULL);
#endif // MAGMA

define_struct_global ("fileName",	"WAVsound", getSoundFileName,	setSoundFileName);
define_struct_global ("start",		"WAVsound", getSoundStartTime,	setSoundStartTime);
define_struct_global ("end",		"WAVsound", getSoundEndTime,	NULL);
define_struct_global ("mute",		"WAVsound", getSoundMute,		setSoundMute);
define_struct_global ("isPlaying",	"WAVsound", getSoundIsPlaying,	NULL);

define_struct_global ("playbackSpeed",	"timeConfiguration", getPlaybackSpeed,	setPlaybackSpeed);

define_struct_global ("mode",			"mouse",	getMouseMode,			NULL);
define_struct_global ("pos",			"mouse",	getMouseMAXPos,			NULL);
define_struct_global ("screenpos",		"mouse",	getMouseScreenPos,		NULL);
define_struct_global ("buttonStates",	"mouse",	getMouseButtonStates,	NULL);
define_struct_global ("inAbort",		"mouse",	getInMouseAbort,		setInMouseAbort);

define_system_global ("rendTimeType",	get_RendTimeType,	set_RendTimeType);
define_system_global ("rendStart",		get_RendStart,		set_RendStart);
define_system_global ("rendEnd",		get_RendEnd,		set_RendEnd);
define_system_global ("rendNThFrame",	get_RendNThFrame,	set_RendNThFrame);
define_system_global ("rendHidden",		get_RendHidden,		set_RendHidden);
define_system_global ("rendForce2Side",	get_RendForce2Side,	set_RendForce2Side);

define_system_global ("rendSaveFile",	get_RendSaveFile,	set_RendSaveFile);

#ifndef WEBVERSION
define_system_global ("rendShowVFB",	get_RendShowVFB,	set_RendShowVFB);
define_system_global ("rendUseDevice",	get_RendUseDevice,	set_RendUseDevice);
define_system_global ("rendUseNet",		get_RendUseNet,		set_RendUseNet);
define_system_global ("rendFieldRender",get_RendFieldRender,set_RendFieldRender);
define_system_global ("rendColorCheck",	get_RendColorCheck,	set_RendColorCheck);
define_system_global ("rendSuperBlack",	get_RendSuperBlack,	set_RendSuperBlack);
define_system_global ("rendSuperBlackThresh",	get_RendSuperBlackThresh,	set_RendSuperBlackThresh);
define_system_global ("rendAtmosphere",	get_RendAtmosphere,	set_RendAtmosphere);
 
define_system_global ("rendDitherTrue",	get_RendDitherTrue,	set_RendDitherTrue);
define_system_global ("rendDither256",	get_RendDither256,	set_RendDither256);
define_system_global ("rendMultiThread",get_RendMultiThread,set_RendMultiThread);
define_system_global ("rendNThSerial",	get_RendNThSerial,	set_RendNThSerial);
define_system_global ("rendVidCorrectMethod",	get_RendVidCorrectMethod,	set_RendVidCorrectMethod);
 
define_system_global ("rendFieldOrder",	get_RendFieldOrder,	set_RendFieldOrder);
define_system_global ("rendNTSC_PAL",	get_RendNTSC_PAL,	set_RendNTSC_PAL);
define_system_global ("rendLockImageAspectRatio",	get_LockImageAspRatio,	set_LockImageAspRatio);
define_system_global ("rendImageAspectRatio",	get_ImageAspRatio,	set_ImageAspRatio);
define_system_global ("rendLockPixelAspectRatio",	get_LockPixelAspRatio,	set_LockPixelAspRatio);
define_system_global ("rendPixelAspectRatio",	get_PixelAspRatio,	set_PixelAspRatio);
define_system_global ("rendSimplifyAreaLights",  get_RendSimplifyAreaLights,      set_RendSimplifyAreaLights);
#endif // WEBVERSION
define_system_global ("rendFileNumberBase",		get_RendFileNumberBase,		set_RendFileNumberBase);
define_system_global ("rendPickupFrames",		get_RendPickupFrames,		set_RendPickupFrames);

define_struct_global ("CommandMode",	"toolMode", getCommandMode,		setCommandMode);
define_struct_global ("AxisConstraints","toolMode", getAxisConstraints,	setAxisConstraints);

define_struct_global ("DisplayType",	"units", getUnitDisplayType,	setUnitDisplayType);
#ifndef USE_HARDCODED_SYSTEM_UNIT
define_struct_global ("SystemType",		"units", getUnitSystemType,		setUnitSystemType);
define_struct_global ("SystemScale",	"units", getUnitSystemScale,	setUnitSystemScale);
#endif
define_struct_global ("MetricType",		"units", getMetricDisplay,		setMetricDisplay);
define_struct_global ("USType",			"units", getUSDisplay,			setUSDisplay);
define_struct_global ("USFrac",			"units", getUSFrac,				setUSFrac);

#ifndef WEBVERSION
define_struct_global ("CustomName",		"units", getCustomName,			setCustomName);
define_struct_global ("CustomValue",	"units", getCustomValue,		setCustomValue);
define_struct_global ("CustomUnit",		"units", getCustomUnit,			setCustomUnit);
#endif //WEBVERSION

define_struct_global ("enabled",	"autoBackup",	getAutoBackupEnabled,	setAutoBackupEnabled);
define_struct_global ("time",		"autoBackup",	getAutoBackupTime,		setAutoBackupTime);

define_system_global( "productAppID",	get_productAppID,			NULL);

define_struct_global ("current",	"renderers", get_CurrentRenderer,		set_CurrentRenderer);
define_struct_global ("production",	"renderers", get_ProductionRenderer,	set_ProductionRenderer);
define_struct_global ("medit",		"renderers", get_MEditRenderer,			set_MEditRenderer);
#ifndef NO_DRAFT_RENDERER
define_struct_global ("draft",		"renderers", get_DraftRenderer,			set_DraftRenderer);
#endif // NO_DRAFT_RENDERER
#if !defined(NO_ACTIVESHADE)
define_struct_global ("activeShade","renderers", get_ReshadeRenderer,		set_ReshadeRenderer);
#endif // !NO_ACTIVESHADE
define_struct_global ("medit_locked","renderers", get_MEditRendererLocked,	set_MEditRendererLocked);

define_struct_global ("dualPlane",		"gw",		get_VptUseDualPlanes,	set_VptUseDualPlanes);

define_system_global( "manipulateMode",	getManipulateMode,			setManipulateMode);

define_struct_global ( "coordSysNode",			"toolMode",				get_coordsys_node,			set_coordsys_node);

define_struct_global ("geometry",	"hideByCategory",	getHideByCategory_geometry,		setHideByCategory_geometry);
define_struct_global ("shapes",		"hideByCategory",	getHideByCategory_shapes,		setHideByCategory_shapes);
define_struct_global ("lights",		"hideByCategory",	getHideByCategory_lights,		setHideByCategory_lights);
define_struct_global ("cameras",	"hideByCategory",	getHideByCategory_cameras,		setHideByCategory_cameras);
define_struct_global ("helpers",	"hideByCategory",	getHideByCategory_helpers,		setHideByCategory_helpers);
define_struct_global ("spacewarps",	"hideByCategory",	getHideByCategory_spacewarps,	setHideByCategory_spacewarps);
define_struct_global ("particles",	"hideByCategory",	getHideByCategory_particles,	setHideByCategory_particles);
define_struct_global ("bones",		"hideByCategory",	getHideByCategory_bones,		setHideByCategory_bones);

define_struct_global ("hilite",				"snapMode",	getSnapHilite,		setSnapHilite);
define_struct_global ("markSize",			"snapMode",	getSnapMarkSize,	setSnapMarkSize);
define_struct_global ("toFrozen",			"snapMode",	getSnapToFrozen,	setSnapToFrozen);
define_struct_global ("axisConstraint",		"snapMode",	getSnapAxisConstraint,	setSnapAxisConstraint);
define_struct_global ("display",			"snapMode",	getSnapDisplay,		setSnapDisplay);
define_struct_global ("strength",			"snapMode",	getSnapStrength,	setSnapStrength);
define_struct_global ("hit",				"snapMode",	getSnapHit,			setSnapHit);
define_struct_global ("node",				"snapMode",	getSnapNode,		setSnapNode);
define_struct_global ("flags",				"snapMode",	getSnapFlags,		setSnapFlags);
define_struct_global ("hitPoint",			"snapMode",	getHitPoint,		setHitPoint);
define_struct_global ("worldHitpoint",		"snapMode",	getWorldHitpoint,	setWorldHitpoint);
define_struct_global ("screenHitPoint",		"snapMode",	getScreenHitpoint,	setScreenHitpoint);
define_struct_global ("OKForRelativeSnap",	"snapMode",	getSnapOKForRelativeSnap,	setSnapOKForRelativeSnap);
define_struct_global ("refPoint",			"snapMode",	getSnapRefPoint,	setSnapRefPoint);
define_struct_global ("topRefPoint",		"snapMode",	getSnapTopRefPoint,	setSnapTopRefPoint);
define_struct_global ("numOSnaps",			"snapMode",	getNumOSnaps,		setNumOSnaps);
define_struct_global ("snapRadius",			"snapMode",	getSnapRadius,		setSnapRadius);
define_struct_global ("snapPreviewRadius",	"snapMode",	getSnapPreviewRadius,		setSnapPreviewRadius);
define_struct_global ("displayRubberBand",	"snapMode",	getDisplaySnapRubberBand,	setDisplaySnapRubberBand);
define_struct_global ("useAxisCenterAsStartSnapPoint",	"snapMode",	getUseAxisCenterAsStartSnapPoint,	setUseAxisCenterAsStartSnapPoint);

define_system_global ( "timeDisplayMode",							get_TimeDisplayMode,			set_TimeDisplayMode);

define_struct_global ("tension",			"TCBDefaultParams",		getTCBDefaultParam_t,			setTCBDefaultParam_t);
define_struct_global ("continuity",			"TCBDefaultParams",		getTCBDefaultParam_c,			setTCBDefaultParam_c);
define_struct_global ("bias",				"TCBDefaultParams",		getTCBDefaultParam_b,			setTCBDefaultParam_b);
define_struct_global ("easeTo",				"TCBDefaultParams",		getTCBDefaultParam_easeIn,		setTCBDefaultParam_easeIn);
define_struct_global ("easeFrom",			"TCBDefaultParams",		getTCBDefaultParam_easeOut,		setTCBDefaultParam_easeOut);

define_struct_global ("inTangentType",		"BezierDefaultParams",	getBezierTangentTypeIn,			setBezierTangentTypeIn);
define_struct_global ("outTangentType",		"BezierDefaultParams",	getBezierTangentTypeOut,		setBezierTangentTypeOut);

//EOF
