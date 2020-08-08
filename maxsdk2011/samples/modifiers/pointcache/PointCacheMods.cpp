/**********************************************************************
 *<
	FILE: PointCacheMods.cpp

	DESCRIPTION:	ClassDesc, ParamBlocks, and DlgProcs for OSM and WSM

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "PointCacheMods.h"
#include "AssetManagement\IAssetAccessor.h"
#include "IPathConfigMgr.h"
#include "assetmanagement\AssetType.h"
#include "IFileResolutionManager.h"
#include "AssetManagement/iassetmanager.h"

using namespace MaxSDK::AssetManagement;

///////////////////////////////////////////////////////////////////////////////////////////////////

class PointCacheOSMClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()						{ return 1; }
	void *			Create(BOOL loading = FALSE)	{ return new PointCacheOSM(); }
	const TCHAR *	ClassName()						{ return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID()					{ return OSM_CLASS_ID; }
	Class_ID		ClassID()						{ return POINTCACHEOSM_CLASS_ID; }
	const TCHAR* 	Category()						{ return GetString(IDS_CATEGORY); }
	const TCHAR*	InternalName()					{ return _T("PointCache2"); }
	HINSTANCE		HInstance()						{ return hInstance; }
};

class PointCacheWSMClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()						{ return 1; }
	void *			Create(BOOL loading = FALSE)	{ return new PointCacheWSM(); }
	const TCHAR *	ClassName()						{ return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID()					{ return WSM_CLASS_ID; }
	Class_ID		ClassID()						{ return POINTCACHEWSM_CLASS_ID; }
	const TCHAR* 	Category()						{ return GetString(IDS_CATEGORY); }
	const TCHAR*	InternalName()					{ return _T("PointCache2WSM"); }
	HINSTANCE		HInstance()						{ return hInstance; }
};


///////////////////////////////////////////////////////////////////////////////////////////////////


static PointCacheOSMClassDesc PointCacheOSMDesc;
ClassDesc2* GetPointCacheOSMDesc() { return &PointCacheOSMDesc; }

static PointCacheWSMClassDesc PointCacheWSMDesc;
ClassDesc2* GetPointCacheWSMDesc() { return &PointCacheWSMDesc; }



///////////////////////////////////////////////////////////////////////////////////////////////////////////
//New pointcache still has to have these interfaces
static FPInterfaceDesc pointcache_interface(
    POINTCACHE_INTERFACE, _T("pointCache"), 0, &PointCacheOSMDesc, FP_MIXIN,
		pointcache_record, _T("record"), 0, TYPE_VOID, 0, 0,
		pointcache_setcache, _T("setCache"), 0, TYPE_VOID, 0, 0,
		pointcache_enablemods, _T("enableMods"), 0, TYPE_VOID, 0, 0,

		pointcache_disablemods, _T("disableMods"), 0, TYPE_VOID, 0, 0,


      end
      );

static FPInterfaceDesc pointcachewsm_interface(
    POINTCACHEWSM_INTERFACE, _T("pointCacheWSM"), 0, &PointCacheWSMDesc, FP_MIXIN,
		pointcache_record, _T("record"), 0, TYPE_VOID, 0, 0,
		pointcache_setcache, _T("setCache"), 0, TYPE_VOID, 0, 0,
		pointcache_enablemods, _T("enableMods"), 0, TYPE_VOID, 0, 0,

		pointcache_disablemods, _T("disableMods"), 0, TYPE_VOID, 0, 0,


      end
      );



FPInterfaceDesc* PointCacheOSM::GetDesc()
{
	 return &pointcache_interface;
}

FPInterfaceDesc* PointCacheWSM::GetDesc()
{
	 return &pointcachewsm_interface;
}


void PointCacheOSM::fnRecord()
{
	bool canceled;
	RecordToFile(m_hWnd,canceled);
	UpdateUI();
}
void PointCacheOSM::fnSetCache()
{
	OpenFilePrompt(m_hWnd);
	UpdateUI();

}
void PointCacheOSM::fnEnableMods()
{
	EnableModsBelow(TRUE);

}
void PointCacheOSM::fnDisableMods()
{
	EnableModsBelow(FALSE);
}


void PointCacheWSM::fnRecord()
{
	bool canceled;
	RecordToFile(m_hWnd,canceled);
	UpdateUI();
}
void PointCacheWSM::fnSetCache()
{
	OpenFilePrompt(m_hWnd);
	UpdateUI();

}
void PointCacheWSM::fnEnableMods()
{
	EnableModsBelow(TRUE);

}
void PointCacheWSM::fnDisableMods()
{
	EnableModsBelow(FALSE);
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////


class PointCachePBAccessor : public PBAccessor
{
public:
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
		PointCache* pointCache = (PointCache*)owner;

		switch (id)
		{
			case pc_preload_cache:
			{
				if (pointCache->m_modifierVersion >= 3)
				{
					v.i = pointCache->m_pblock->GetInt(pc_load_type) == load_preload;
				}
				break;
			}
			case pc_load_type:
			{
				if (v.i < load_stream) v.i = load_stream;
				if (v.i > load_preload) v.i = load_preload;
				break;
			}
			case pc_load_type_slave:
			{
				if (v.i < load_persample) v.i = load_persample;
				if (v.i > load_preload) v.i = load_preload;
				break;
			}
		}
	}

	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		PointCache* pointCache = (PointCache*)owner;

		switch (id)
		{
			case pc_cache_file:
			{
				
				macroRecorder->Disable();

				pointCache->CloseCacheFile(true);

				AssetUser asset = pointCache->m_pblock->GetAssetUser(pc_cache_file);
				pointCache->SetFileType(asset.GetFileName());
				pointCache->m_hCacheFile = pointCache->OpenCacheForRead(asset);
				if (pointCache->IsCacheFileOpen())
				{
					pointCache->m_doLoad = true;
					if (pointCache->ReadHeader())
					{
						float st = pointCache->m_cacheStartFrame;
						float sr = pointCache->m_cacheSampleRate;
						float et = st + ((pointCache->m_cacheNumSamples-1) / (1.0f/sr));

						pointCache->m_pblock->SetValue(pc_record_start,0,st);
						pointCache->m_pblock->SetValue(pc_record_end,0,et);
						pointCache->m_pblock->SetValue(pc_sample_rate,0,sr);
						pointCache->m_pblock->SetValue(pc_playback_start,0,st);
						pointCache->m_pblock->SetValue(pc_playback_end,0,et);
						pointCache->m_pblock->SetValue(pc_file_count, 0, pointCache->m_fileCount);
					}
				}
				pointCache->CloseCacheFile(true);

				macroRecorder->Enable();

				pointCache->UpdateUI();
				
				break;
			}
			case pc_record_start:
			{
				macroRecorder->Disable();
				pointCache->m_pblock->SetValue(pc_playback_start,0,v.f);
				macroRecorder->Enable();

				pointCache->UpdateUI();
				break;
			}
			case pc_record_end:
			{
				macroRecorder->Disable();
				pointCache->m_pblock->SetValue(pc_playback_end,0,v.f);
				macroRecorder->Enable();

				pointCache->UpdateUI();
				break;
			}
			case pc_playback_before:
			case pc_playback_after:
			{
				if (v.i < 0) v.i = 0;
				if (v.i > 2) v.i = 2;
				pointCache->UpdateUI();
				break;
			}
			case pc_preload_cache:
			{
				if (v.i)
				{
					pointCache->m_pblock->SetValue(pc_load_type, 0, load_preload);
				} else {
					pointCache->m_pblock->SetValue(pc_load_type, 0, load_stream);
				}
				break;
			}
			case pc_force_unc_path:
			{
				pointCache->m_pblock->CallSet(pc_cache_file);
				break;
			}
			case pc_load_type:
			{
				pointCache->Reload();
				break;
			}
			case pc_file_count:
			{
				if (v.i < PointCache::eOneFile) v.i = PointCache::eOneFile;
				if (v.i > PointCache::eOneFilePerFrame) v.i = PointCache::eOneFilePerFrame;
				break;
			}
		}
	}
};

static PointCachePBAccessor pointCachePBAccessor;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define ENABLE_PLAYBACK_SET(CONTROL_ID, DO_ENABLE)						\
	EnableWindow(GetDlgItem(thishWnd, CONTROL_ID##_LABEL), DO_ENABLE);	\
	EnableSpinner(thishWnd, CONTROL_ID##_EDIT, DO_ENABLE);				\
	EnableSpinner(thishWnd, CONTROL_ID##_SPIN, DO_ENABLE);

class PointCacheParamsMapDlgProc : public ParamMap2UserDlgProc
{
public:
	PointCache *m;
	HWND thishWnd;
	PointCacheParamsMapDlgProc(PointCache *mod) {m = mod; thishWnd = NULL;}
	void Update(TimeValue t);
	INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { delete this; }

	void EnableSpinner(HWND hWnd, int spinID, BOOL enable);
};

void PointCacheParamsMapDlgProc::EnableSpinner(HWND hWnd, int spinID, BOOL enable)
{
	ISpinnerControl* sp = GetISpinner(GetDlgItem(hWnd,spinID));
	(enable) ? sp->Enable() : sp->Disable();
	ReleaseISpinner(sp);
}

void PointCacheParamsMapDlgProc::Update(TimeValue t)
{
	if (thishWnd)
	{
		//cache file label
		m->UpdateCacheInfoStatus( thishWnd );

		// Memory used label
		TSTR str;
		str.printf(GetString(IDS_MEMORY_USED_LABEL), float(GetPointCacheManager().GetMemoryUsed()) / 1048576.0f);
		SetWindowText(GetDlgItem(thishWnd, IDC_MEMORY_USED_LABEL), str);

		//enable/disable playback controls
		int playbackType = m->m_pblock->GetInt(pc_playback_type, 0);
		switch (playbackType)
		{
			case playback_original:
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_START, FALSE)
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_END, FALSE)
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_FRAME, FALSE)
				EnableWindow(GetDlgItem(thishWnd, IDC_CLAMP_GRAPH), FALSE);
				break;
			case playback_start:
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_START, TRUE)
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_END, FALSE)
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_FRAME, FALSE)
				EnableWindow(GetDlgItem(thishWnd, IDC_CLAMP_GRAPH), FALSE);
				break;
			case playback_range:
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_START, TRUE)
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_END, TRUE)
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_FRAME, FALSE)
				EnableWindow(GetDlgItem(thishWnd, IDC_CLAMP_GRAPH), FALSE);
				break;
			case playback_graph:
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_START, FALSE)
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_END, FALSE)
				ENABLE_PLAYBACK_SET(IDC_PLAYBACK_FRAME, TRUE)
				EnableWindow(GetDlgItem(thishWnd, IDC_CLAMP_GRAPH), TRUE);
				break;
		}
	}

	BOOL bIsPTS = (m->m_fileType == PointCache::ePTS);
	BOOL bIsPC2 = (m->m_fileType == PointCache::ePC2);
	BOOL bIsMC  = (m->m_fileType == PointCache::eMC);

	EnableWindow(GetDlgItem(thishWnd,IDC_RECORD), bIsPC2||bIsMC );
	EnableWindow(GetDlgItem(thishWnd,IDC_APPLY_TO_WHOLE_OBJECT), bIsPC2||bIsMC);
	EnableWindow(GetDlgItem(thishWnd,IDC_PLAYBACK_TYPE), bIsPC2||bIsMC);
	EnableWindow(GetDlgItem(thishWnd,IDC_LOAD_TYPE_SLAVE), bIsPC2);
	EnableWindow(GetDlgItem(thishWnd,IDC_LOAD_TYPE), bIsPC2);
	EnableWindow(GetDlgItem(thishWnd,IDC_NEW), !bIsPTS);

	EnableWindow(GetDlgItem(thishWnd,IDC_MEMORY_USED_LABEL),bIsPC2);

	EnableWindow(GetDlgItem(thishWnd,IDC_SAVE_EVERY_EDIT),bIsMC);
	EnableWindow(GetDlgItem(thishWnd,IDC_SAVE_EVERY_SPIN),bIsMC);
	EnableWindow(GetDlgItem(thishWnd,IDC_SAVE_EVERY_TEXT),bIsMC);
	EnableWindow(GetDlgItem(thishWnd,IDC_ONEFILEPERFRAME),bIsMC);
}

INT_PTR PointCacheParamsMapDlgProc::DlgProc
(
	TimeValue t, IParamMap2* map, HWND hWnd,
	UINT msg, WPARAM wParam,LPARAM lParam
) {
	thishWnd = hWnd;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			m->m_hWnd = hWnd;

			ISpinnerControl* sp = GetISpinner(GetDlgItem(hWnd,IDC_SAMPLE_RATE_SPIN));
			sp->SetResetValue(1.0f);
			ReleaseISpinner(sp);

			m->UpdateUI();
			Update(t);

			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_NEW:
				{
					m->SaveFilePrompt(hWnd);
					break;
				}
				case IDC_LOAD:
				{
					m->OpenFilePrompt(hWnd);
					break;
				}
				case IDC_BROWSE:
				{
					const TSTR& str = m->m_pblock->GetAssetUser(pc_cache_file).GetFileName();

					if (m->IsValidFName(str))
					{
						TSTR path;
						SplitPathFile(str, &path, NULL);

						HINSTANCE res = ShellExecute( hWnd, "open", path.data(), NULL, NULL, SW_SHOWNORMAL );
					}

					break;
				}
				case IDC_UNLOAD:
				{
					m->Unload();
					break;
				}
				case IDC_RELOAD:
				{
					m->Reload();
					break;
				}
				case IDC_RECORD:
				{
					bool canceled;
					if (!m->RecordToFile(hWnd,canceled)&&canceled==false) //we failed but not because we canceled.
					{
						m->OpenCacheForRead(m->m_pblock->GetAssetUser(pc_cache_file));
						MessageBox(hWnd, GetString(IDS_ERROR_RECORDING), GetString(IDS_CLASS_NAME), MB_OK);
					}
					m->UpdateUI();
					break;
				}
				case IDC_ENABLE:
				{
					m->EnableModsBelow(TRUE);
					break;
				}
				case IDC_DISABLE:
				{
					m->EnableModsBelow(FALSE);
					break;
				}
				case IDC_APPLYTOSPLINE:
				{
					m->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					m->UpdateUI();
					break;
				}
				case IDC_PLAYBACK_TYPE:
				{
					Update(t);
					break;
				}
			}
			break;
		}
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////



// flagrant abuse of preprocessor
#define PBLOCK_DEFINITION																								\
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,																			\
	IDD_POINTCACHE, IDS_PARAMS, 0, 0, NULL,																				\
	pc_cache_file, 		_T("filename"), 	TYPE_FILENAME, 	P_RESET_DEFAULT,	IDS_FILENAME,							\
	p_assetTypeID,	kAnimationAsset,																		\
	/*	p_ui,			TYPE_EDITBOX,		IDC_FILENAME,*/																\
		p_accessor,		&pointCachePBAccessor,																			\
		end,																											\
	pc_record_start, 	_T("recordStart"), 	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_RECORD_START,						\
		p_default, 		0.0f,																							\
		p_range, 		-999999999.0f, 999999999.0f,																	\
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_RECORD_START_EDIT,	IDC_RECORD_START_SPIN, 1.0f,		\
		p_accessor,		&pointCachePBAccessor,																			\
		end,																											\
	pc_record_end,		_T("recordEnd"), 	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_RECORD_END,							\
		p_default, 		0.0f,																							\
		p_range, 		-999999999.0f, 999999999.0f,																	\
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_RECORD_END_EDIT,	IDC_RECORD_END_SPIN, 1.0f,			\
		p_accessor,		&pointCachePBAccessor,																			\
		end,																											\
	pc_sample_rate,		_T("sampleRate"), 	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_SAMPLE_RATE,						\
		p_default, 		1.0f,																							\
		p_range, 		0.001f, 999.0f,																					\
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_SAMPLE_RATE_EDIT,	IDC_SAMPLE_RATE_SPIN, 1.0f,			\
		end,																											\
	pc_strength, 		_T("strength"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_PW_STRENGTH,		\
		p_default, 		1.0f,																							\
		p_range, 		-10.0f,10.0f,																					\
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_STRENGTH_EDIT,	IDC_STRENGTH_SPIN, 0.01f,				\
		end,																											\
	pc_relative, 		_T("relativeOffset"), 	TYPE_BOOL, 		P_RESET_DEFAULT,	IDS_RELATIVE,						\
		p_default,		FALSE,																							\
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_RELATIVE_CHECK,														\
		end,																											\
	pc_playback_type,	_T("playbackType"), 	TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PLAYBACK_TYPE,						\
		p_default, 		0,																								\
		p_range, 		0,3,																							\
		p_ui, 			TYPE_INTLISTBOX, IDC_PLAYBACK_TYPE, 4,	IDS_PLAYBACKTYPE_ORIGINAL,								\
																IDS_PLAYBACKTYPE_START,									\
																IDS_PLAYBACKTYPE_RANGE,									\
																IDS_PLAYBACKTYPE_GRAPH,									\
		end,																											\
	pc_playback_start,	_T("playbackStart"), TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_PLAYBACK_START,						\
		p_default, 		0.0f,																							\
		p_range, 		-999999999.0f, 999999999.0f,																	\
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_PLAYBACK_START_EDIT,	IDC_PLAYBACK_START_SPIN, 1.0f,	\
		end,																											\
	pc_playback_end,	_T("playbackEnd"),	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_PLAYBACK_END,						\
		p_default, 		0.0f,																							\
		p_range, 		-999999999.0f, 999999999.0f,																	\
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_PLAYBACK_END_EDIT,	IDC_PLAYBACK_END_SPIN, 1.0f,		\
		end,																											\
	pc_playback_before,	_T("playbackORTbefore"),	TYPE_INT,		P_RESET_DEFAULT,	IDS_PLAYBACK_BEFORE,			\
		p_default,		ort_hold,																						\
		p_range,		0, 2,																							\
		/*p_ui,			TYPE_INTLISTBOX, IDC_ORT_TYPE_BEFORE, 0,*/														\
		p_accessor,		&pointCachePBAccessor,																			\
		end,																											\
	pc_playback_after,	_T("playbackORTafter"),		TYPE_INT,		P_RESET_DEFAULT,	IDS_PLAYBACK_AFTER,				\
		p_default,		ort_hold,																						\
		p_range,		0, 2,																							\
		/*p_ui,			TYPE_INTLISTBOX, IDC_ORT_TYPE_AFTER, 0,*/														\
		p_accessor,		&pointCachePBAccessor,																			\
		end,																											\
	pc_playback_frame,	_T("playbackFrame"), TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_PLAYBACK_FRAME,			\
		p_default, 		0.0f,																							\
		p_range, 		-9999999.0f, 9999999.0f,																		\
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_PLAYBACK_FRAME_EDIT,	IDC_PLAYBACK_FRAME_SPIN, 1.0f,	\
		end,																											\
	pc_interp_type,	_T("interpolationType"), 	TYPE_INT, 	P_RESET_DEFAULT, 	IDS_INTERP_TYPE,						\
		p_default, 		0,																								\
		p_range, 		0,1,																							\
		/*p_ui, 			TYPE_RADIO,		2,	IDC_INTERP_LINEAR,	IDC_INTERP_CUBIC,*/									\
		end,																											\
	pc_apply_to_spline, _T("applyMeshToSpline"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_APPLYTOSPLINE,					\
		p_default,		FALSE,																							\
		/*p_ui,			TYPE_SINGLECHEKBOX,	IDC_APPLYTOSPLINE,*/														\
		end,																											\
	pc_preload_cache, _T("preLoadCache"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_PRELOADCACHE,							\
		p_default,		FALSE,																							\
		p_ui,			TYPE_SINGLECHEKBOX,	IDC_PRELOADCACHE,															\
		p_accessor,		&pointCachePBAccessor,																			\
		end,																											\
	pc_clamp_graph, _T("clampGraph"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_CLAMP_GRAPH,								\
		p_default,		FALSE,																							\
		p_ui,			TYPE_SINGLECHEKBOX,	IDC_CLAMP_GRAPH,															\
		end,																											\
	/*no longer used, but left in for old files*/																		\
	pc_force_unc_path, _T("forceUncPath"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_FORCE_UNC_PATH,							\
		p_default,		TRUE,																							\
		p_accessor,		&pointCachePBAccessor,																			\
	/*gone	p_ui,			TYPE_SINGLECHEKBOX,	IDC_FORCE_UNC_PATH,*/														\
		end,																											\
	pc_load_type, _T("loadType"), TYPE_INT, P_RESET_DEFAULT, IDS_LOAD_TYPE,												\
		p_default, 		load_stream,																					\
		p_range, 		load_stream, load_preload,																		\
		p_accessor,		&pointCachePBAccessor,																			\
		p_ui,			TYPE_INTLISTBOX, IDC_LOAD_TYPE, 3,	IDS_LOADTYPE_STREAM,										\
															IDS_LOADTYPE_PERSAMPLE,										\
															IDS_LOADTYPE_PRELOAD,										\
		p_vals,			load_stream,																					\
						load_persample,																					\
						load_preload,																					\
		end,																											\
	pc_apply_to_whole_object, _T("applyToWholeObject"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_APPLY_TO_WHOLE_OBJECT,		\
		p_default,		TRUE,																							\
		p_ui,			TYPE_SINGLECHEKBOX,	IDC_APPLY_TO_WHOLE_OBJECT,													\
		end,																											\
	pc_load_type_slave, _T("loadTypeSlave"), TYPE_INT, P_RESET_DEFAULT, IDS_LOAD_TYPE_SLAVE,							\
		p_default, 		load_persample,																					\
		p_range, 		load_persample, load_preload,																	\
		p_accessor,		&pointCachePBAccessor,																			\
		p_ui,			TYPE_INTLISTBOX, IDC_LOAD_TYPE_SLAVE, 2,	IDS_LOADTYPE_PERSAMPLE,								\
																	IDS_LOADTYPE_PRELOAD,								\
		p_vals,			load_persample,																					\
						load_preload,																					\
		end,																											\
	pc_file_count,	_T("fileCount"), TYPE_INT, P_RESET_DEFAULT, IDS_FILE_COUNT,											\
		p_default, PointCache::eOneFile,																				\
		p_ui, TYPE_RADIO, 2, IDC_ONEFILE, IDC_ONEFILEPERFRAME,															\
		p_accessor,		&pointCachePBAccessor,																			\
		end,																											\
	end

static ParamBlockDesc2 pointcache_param_blk
(
	pointcache_params, _T("params"),  0, &PointCacheOSMDesc,
	PBLOCK_DEFINITION
);

static ParamBlockDesc2 pointcachewsm_param_blk
(
	pointcache_params, _T("params"),  0, &PointCacheWSMDesc,
	PBLOCK_DEFINITION
);

// PointCacheOSM //////////////////////////////////////////////////////////////////////////////////

PointCacheOSM::PointCacheOSM()
{
	dlgProc = NULL;
	PointCacheOSMDesc.MakeAutoParamBlocks(this);

	Interval iv = GetCOREInterface()->GetAnimRange();
	int tpf = GetTicksPerFrame();
	float startFrame = float(iv.Start()) / float(tpf);
	float endFrame = float(iv.End()) / float(tpf);
	m_pblock->SetValue(pc_record_start,0,startFrame);
	m_pblock->SetValue(pc_record_end,0,endFrame);
	m_pblock->SetValue(pc_playback_start,0,startFrame);
	m_pblock->SetValue(pc_playback_end,0,endFrame);
}

SClass_ID PointCacheOSM::SuperClassID()
{
	return OSM_CLASS_ID;
}

Class_ID PointCacheOSM::ClassID()
{
	return POINTCACHEOSM_CLASS_ID;
}

void PointCacheOSM::BeginEditParams(IObjParam* iCore, ULONG flags, Animatable* prev)
{
	m_iCore = iCore;
	PointCacheOSMDesc.BeginEditParams(iCore, this, flags, prev);
	dlgProc = new PointCacheParamsMapDlgProc(this);
	pointcache_param_blk.SetUserDlgProc(dlgProc);
}

void PointCacheOSM::EndEditParams(IObjParam* iCore, ULONG flags, Animatable* next)
{
	PointCacheOSMDesc.EndEditParams(iCore, this, flags, next);
	m_iCore = NULL;
	m_hWnd = NULL;
	dlgProc = NULL;
}

TCHAR* PointCacheOSM::GetObjectName()
{
	return GetString(IDS_CLASS_NAME);
}

RefTargetHandle PointCacheOSM::Clone(RemapDir& remap)
{
	PointCacheOSM* newmod = new PointCacheOSM();
	newmod->ReplaceReference(0,remap.CloneRef(m_pblock));

	CloneCache( (PointCache*)newmod );

#if MAX_RELEASE > 3100
	BaseClone(this, newmod, remap);
#endif
	return(newmod);
}

void PointCacheOSM::SetFile(const AssetUser& filename)
{
	if(dlgProc&&dlgProc->thishWnd)
	{
		//cache file label
		AssetUser currFile = m_pblock->GetAssetUser(pc_cache_file);
		TSTR path, file, ext;
		SplitFilename(currFile.GetFileName(), &path, &file, &ext);
		TSTR name;
		if (file.isNull()) file = GetString(IDS_NONE);
		else
			name = file+ext;
		HWND hTextWnd = GetDlgItem(dlgProc->thishWnd,IDC_CACHEFILE_LABEL);
		SetWindowText(hTextWnd,file);

		ICustStatus *iStatus = GetICustStatus(GetDlgItem(dlgProc->thishWnd,IDC_FILENAME));
	    iStatus->SetText(name);
		TSTR fullpath;
		if (currFile.GetFullFilePath(fullpath)) 
		{
			iStatus->SetTooltip(TRUE,fullpath);
		}
		else
			iStatus->SetTooltip(FALSE,GetString(IDS_NONE));
		ReleaseICustStatus(iStatus);
	}
}

// PointCacheWSM //////////////////////////////////////////////////////////////////////////////////

PointCacheWSM::PointCacheWSM()
{
	dlgProc = NULL;
	PointCacheWSMDesc.MakeAutoParamBlocks(this);

	Interval iv = GetCOREInterface()->GetAnimRange();
	int tpf = GetTicksPerFrame();
	float startFrame = float(iv.Start()) / float(tpf);
	float endFrame = float(iv.End()) / float(tpf);
	m_pblock->SetValue(pc_record_start,0,startFrame);
	m_pblock->SetValue(pc_record_end,0,endFrame);
	m_pblock->SetValue(pc_playback_start,0,startFrame);
	m_pblock->SetValue(pc_playback_end,0,endFrame);
}

SClass_ID PointCacheWSM::SuperClassID()
{
	return WSM_CLASS_ID;
}

Class_ID PointCacheWSM::ClassID()
{
	return POINTCACHEWSM_CLASS_ID;
}

void PointCacheWSM::BeginEditParams(IObjParam* iCore, ULONG flags, Animatable* prev)
{
	m_iCore = iCore;
	PointCacheWSMDesc.BeginEditParams(iCore, this, flags, prev);
	dlgProc = new PointCacheParamsMapDlgProc(this);
	pointcachewsm_param_blk.SetUserDlgProc(dlgProc);
}

void PointCacheWSM::EndEditParams(IObjParam* iCore, ULONG flags, Animatable* next)
{
	dlgProc = NULL;
	PointCacheWSMDesc.EndEditParams(iCore, this, flags, next);
	m_iCore = NULL;
	m_hWnd = NULL;
}

TCHAR* PointCacheWSM::GetObjectName()
{
	return GetString(IDS_POINTCACHE_NAMEBINDING);
}

RefTargetHandle PointCacheWSM::Clone(RemapDir& remap)
{
	PointCacheWSM* newmod = new PointCacheWSM();
	newmod->ReplaceReference(0,remap.CloneRef(m_pblock));

	CloneCache( (PointCache*)newmod );

#if MAX_RELEASE > 3100
	BaseClone(this, newmod, remap);
#endif
	return(newmod);
}

void PointCacheWSM::SetFile(const AssetUser& file)
{
	if(dlgProc&&dlgProc->thishWnd)
	{
		//cache file label
		TSTR path, filename, ext;
		SplitFilename(file.GetFileName(), &path, &filename, &ext);
		TSTR name;
		if (filename.isNull()) filename = GetString(IDS_NONE);
		else
			name = filename+ext;
		HWND hTextWnd = GetDlgItem(dlgProc->thishWnd,IDC_CACHEFILE_LABEL);
		SetWindowText(hTextWnd,filename);

		ICustStatus *iStatus = GetICustStatus(GetDlgItem(dlgProc->thishWnd,IDC_FILENAME));
	    iStatus->SetText(name);
		TSTR fullpath;
		if (file.GetFullFilePath(fullpath)) 
		{
			iStatus->SetTooltip(TRUE,fullpath);
		}
		else
			iStatus->SetTooltip(FALSE,GetString(IDS_NONE));
		ReleaseICustStatus(iStatus);
	}
}


void PointCache::SetCacheFile(const AssetUser& file)
{
	m_pblock->SetValue(pc_cache_file,0,file);
	SetFileType(file.GetFileName());
}

void PointCache::SetRecordStart(float f)
{
	m_pblock->SetValue(pc_record_start,0,f);
}

void PointCache::SetRecordEnd(float f)
{
	m_pblock->SetValue(pc_record_end,0,f);
}

void PointCache::SetPlaybackStart(float f)
{
	m_pblock->SetValue(pc_playback_start,0,f);
}

void PointCache::SetPlaybackEnd(float f)
{
	m_pblock->SetValue(pc_playback_end,0,f);
}

void PointCache::SetSampleRate(float f)
{
	m_pblock->SetValue(pc_sample_rate,0,f);
}

void PointCache::SetStrength(float f)
{
	m_pblock->SetValue(pc_strength,0,f);
}

void PointCache::SetRelative(BOOL b)
{
	m_pblock->SetValue(pc_relative,0,b);
}


//this function finds all of the nodes that the OLDPointCacheBase is on and replaces it with the new PointCache modifier


class PCEnumProc : public DependentEnumProc
{
public:
	Tab<INode *> Nodes;
	int  proc(ReferenceMaker *rmaker);
};

int PCEnumProc::proc(ReferenceMaker *rmaker) 
{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
	{
		Nodes.Append(1, (INode **)&rmaker);  
		return DEP_ENUM_SKIP;
	}

	return DEP_ENUM_CONTINUE;
}
void ReplaceOldPointCache(OLDPointCacheBase *obj)
{
	theHold.Suspend(); //don't really need to UNDO this so disable.. makes it faster too.
	PCEnumProc dep;              
	obj->DoEnumDependents(&dep);
	Interface7 *ip = GetCOREInterface7();
	int modStackIdx;
	int derivedObjIdx;
	bool isInScene = false;
	PointCache* PC = NULL;
	if(obj->SuperClassID()==OSM_CLASS_ID)
	{
		PC = new PointCacheOSM();
	}
	else
	{
		PC = new PointCacheWSM();
	}
	for (int i = 0; i < dep.Nodes.Count(); i++)
	{
		INode *node = dep.Nodes[i];
		if (node)
		{
			//may have more than one instance of the modifier on the object.. so we do it until it's NULL
			while(ip->FindModifier(*node,*obj,modStackIdx,derivedObjIdx)!=NULL)
			{
				AssetUser file = obj->GetCacheFile();
				if(file.GetId()!=kInvalidId)
				{
					MaxSDK::Util::Path path;
					if (file.GetFullFilePath(path)) 
					{
						IPathConfigMgr::GetPathConfigMgr()->NormalizePathAccordingToSettings(path);
						PC->SetCacheFile(IAssetManager::GetInstance()->GetAsset(path,kAnimationAsset));
					}
				}

				PC->SetStrength(obj->GetStrength());
				PC->SetRecordStart(obj->GetStartTime());
				PC->SetRecordEnd(obj->GetEndTime());
				PC->SetPlaybackStart(obj->GetStartTime());
				PC->SetPlaybackEnd(obj->GetEndTime());
				PC->SetRelative(obj->GetRelative());
				PC->SetSampleRate(1.0f/(float)obj->GetSamples());

				//note that the modStackIdx is the index taking into account both WSMs and OSMs. The SDK has
				//been modified to handle this correctly in AddModifier. Defect 798844 MZ.
				ip->DeleteModifier(*node,modStackIdx);
				ip->AddModifier(*node,*PC,modStackIdx);
				isInScene = true;
			}
		}
	}
	if(isInScene==false)
		delete PC;

	theHold.Resume();
}
