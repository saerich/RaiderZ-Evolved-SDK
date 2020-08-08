/**********************************************************************
 *<
	FILE: RendSpline.cpp

	DESCRIPTION:	Renderable Spline Modifier

	CREATED BY:		Alexander Esppeschit Bicalho (discreet)

	HISTORY:		2/1/2004 (Created)

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#include "RendSpline.h"
#include "object.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "macroRec.h"
#include "RealWorldMapUtils.h"

#define RendSpline_CLASS_ID	Class_ID(0x817f4d17, 0x8bc7074a)
#define EDITSPL_CHANNELS (PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO)

#define EPS 0.00000000001f
#define PBLOCK_REF	0
//#define NUM_OLDVERSIONS	0

class RendSplineParamsMapDlgProc;

class RendSpline : public Modifier, public RealWorldMapSizeInterface  {
	friend class RendSplineParamsMapDlgProc;
	protected:
		float mAspect;
		Interval mAspectValidity;
		BOOL  mAspectLock;

		int UseViewOrRenderParams(TimeValue t);
		void OnSetWidthLength(HWND hWnd, int lengthID, int widthID, TimeValue t, BOOL widthSet);
		void OnSetAspect(HWND hWnd, TimeValue t, int widthID, int lengthID);
		void OnAspectLock(HWND hWnd, TimeValue t);
		float GetAspect(TimeValue t, BOOL viewport);
		void CheckAspectLock(TimeValue t);
		//void UpdateAspect(HWND hWnd, TimeValue t);
	public:
		static float nlength;
		static float nangle;
		static int nsides;
		static float vthickness;
		static float vangle;
		static int vsides;
		static RendSplineParamsMapDlgProc* paramDlgProc;
		static HIMAGELIST hLockButton;

		class ParamAccessor;

		// Parameter block
		IParamBlock2	*pblock;	//ref 0

		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

				ChannelMask ChannelsUsed()  { return EDITSPL_CHANNELS; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return EDITSPL_CHANNELS; }
				
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() { return Class_ID(SPLINESHAPE_CLASS_ID,0); }

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);


		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return TRUE;}		
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		BOOL HasUVW();
        BOOL GeneratesUVS();
		void SetGenUVW(BOOL sw);

		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);


		Interval GetValidity(TimeValue t);

		// Automatic texture support
		
		//From Animatable
		Class_ID ClassID() {return RendSpline_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);


		int NumSubs() { return 1; }
		TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }				
		Animatable* SubAnim(int i) { return pblock; }

		// TODO: Maintain the number or references here
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return pblock; }
		void SetReference(int i, RefTargetHandle rtarg) { pblock=(IParamBlock2*)rtarg; }

		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		
		//Constructor/Destructor

		RendSpline(BOOL create);
		~RendSpline();		

        // Get/Set the UsePhyicalScaleUVs flag.
        BOOL GetUsePhysicalScaleUVs();
        void SetUsePhysicalScaleUVs(BOOL flag);
		void UpdateUI();

		//From FPMixinInterface
		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == RWS_INTERFACE) 
				return this; 
			else 
				return FPMixinInterface::GetInterface(id);
		} 
};


static BOOL sInterfaceAdded = FALSE;


class RendSplineClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE)
    {
#ifdef PHYSICAL_SCALE_UVS
		if (!sInterfaceAdded) {
			AddInterface(&gRealWorldMapSizeDesc);
			sInterfaceAdded = TRUE;
		}
#endif
        return new RendSpline(!loading);
    }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return RendSpline_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("RendSpline"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	
};

// Initializations

float			RendSpline::nlength =  1.0f;
float			RendSpline::vthickness = 1.0f;
float			RendSpline::nangle =  0.0f;
float			RendSpline::vangle = 0.0f;
int				RendSpline::nsides = 12;
int				RendSpline::vsides = 12;
RendSplineParamsMapDlgProc* RendSpline::paramDlgProc = NULL;
HIMAGELIST		RendSpline::hLockButton=NULL;


static RendSplineClassDesc RendSplineDesc;
ClassDesc2* GetRendSplineDesc() { return &RendSplineDesc; }

enum { rendspline_params };


//TODO: Add enums for various parameters
enum { 
	rnd_thickness, 
	rnd_viewThickness,
	rnd_sides,
	rnd_viewSides,
	rnd_angle,
	rnd_viewAngle,
	rnd_render, 
	rnd_genuvw,
	rnd_display,
	rnd_useView,
	rnd_ViewportOrRender,
	rnd_v2_symm_or_rect,
	rnd_v2_width,
	rnd_v2_length,
	rnd_v2_aspect_lock,
	rnd_v2_angle2,
	rnd_v2_vpt_symm_or_rect,
	rnd_v2_vpt_width,
	rnd_v2_vpt_length,
	rnd_v2_vpt_aspect_lock,
	rnd_v2_vpt_angle2,
	rnd_v2_threshold,
	rnd_v2_autosmooth,
};

enum {rbViewport, rbRender};
enum {rbSymmetrical, rbRectangular};

class RendSplineParamsMapDlgProc : public ParamMap2UserDlgProc {
	public:
        HWND mhWnd;
		RendSpline *mod;		
		RendSplineParamsMapDlgProc(RendSpline *m) {mod = m;}		
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);	
		void Initialize (HWND hWnd, TimeValue t);
		void Update (HWND hWnd, TimeValue t, ParamID id);
		void Update (HWND hWnd, TimeValue t);
		void Update (TimeValue t, Interval& valid, IParamMap2* pmap)
		{
			Update(pmap->GetHWnd(), t);
		}
		void EnableButtons(HWND hWnd, TimeValue t, BOOL display, BOOL useView);
		void DeleteThis() {
				delete this;
				}
        void UpdateRWS();
	};

class RendSpline::ParamAccessor : public PBAccessor {
public:
	virtual void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);
};

void RendSpline::ParamAccessor::Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
{
	if (owner != NULL && RendSpline::paramDlgProc != NULL)
	{
		RendSpline* mod = static_cast<RendSpline*>(owner);
		IParamBlock2* pb = owner->GetParamBlock(0);
		if (pb != NULL)
		{
			IParamMap2* pm = pb->GetMap(0);
			
			if (pm != NULL)
				RendSpline::paramDlgProc->Update(pm->GetHWnd(), t, id);
		}
	}
}

static RendSpline::ParamAccessor updateUI;

static ParamBlockDesc2 rendspline_param_blk ( rendspline_params, _T("params"),  0, &RendSplineDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	rnd_thickness, 			_T("Thickness"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_THICK, 
		p_default, 		1.0f, 
		p_range, 		0.0f,1000000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_THICKNESS,	IDC_THICKSPIN, 0.1f, 
		p_accessor,		&updateUI,
		end,

	rnd_viewThickness, 			_T("Viewport_Thickness"), 		TYPE_FLOAT, 	0, 	IDS_VIEW_THICK, 
		p_default, 		1.0f, 
		p_range, 		0.0f,1000000.0f, 
//		p_ui, 			TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_THICKNESS,	IDC_THICKSPIN, 0.1f, 
		p_accessor,		&updateUI,
		end,

	rnd_sides, 			_T("Sides"), 		TYPE_INT, 	P_ANIMATABLE, 	IDS_SIDES, 
		p_default, 		12, 
		p_range, 		3,100, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_SIDES,	IDC_SIDESSPIN, SPIN_AUTOSCALE,
		p_accessor,		&updateUI,
		end,

	rnd_viewSides, 			_T("Viewport_Sides"), 		TYPE_INT, 	0, 	IDS_VIEW_SIDES, 
		p_default, 		12, 
		p_range, 		3,100, 
//		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_SIDES,	IDC_SIDESSPIN, SPIN_AUTOSCALE,
		p_accessor,		&updateUI,
		end,

	rnd_angle, 			_T("Angle"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_ANGLE, 
		p_default, 		0.0f, 
		p_range, 		-99999999.0f,99999999.0f,
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_ANGLE,	IDC_ANGLESPIN, 0.1f, 
		p_accessor,		&updateUI,
		end,
		
	rnd_viewAngle, 			_T("Viewport_Angle"), 		TYPE_FLOAT, 	0, 	IDS_VIEW_ANGLE, 
		p_default, 		0.0f, 
		p_range, 		-99999999.0f,99999999.0f, 
//		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_ANGLE,	IDC_ANGLESPIN, 0.1f, 
		p_accessor,		&updateUI,
		end,
		
	rnd_render, 		_T("Renderable"), TYPE_BOOL, P_RESET_DEFAULT,	IDS_RENDERABLE, 	
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_RENDERABLE, 
		p_accessor,		&updateUI,
		end, 

	rnd_genuvw, 		_T("mapcoords"), TYPE_BOOL, P_RESET_DEFAULT,	IDS_GENUVW, 	
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_GENMAPPING, 
		p_accessor,		&updateUI,
		end, 

	rnd_display, 		_T("displayRenderSettings"), TYPE_BOOL, P_RESET_DEFAULT,	IDS_DISPLAY, 	
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_DISPRENDERMESH, 
		p_accessor,		&updateUI,
		end, 

	rnd_useView, 		_T("useViewportSettings"), TYPE_BOOL, P_RESET_DEFAULT,	IDS_VIEW, 	
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_USE_VIEWPORT, 
		p_accessor,		&updateUI,
		end, 
	
	rnd_ViewportOrRender, _T("ViewportOrRender"), TYPE_RADIOBTN_INDEX, 0, IDS_VIEWPORTORRENDERER,
		p_default, rbRender,
		p_ui, 			TYPE_RADIO, 2, IDC_VIEWPORT, IDC_RENDERER,
		p_vals,			rbViewport, rbRender,
		p_range, rbViewport, rbRender,
		p_accessor,		&updateUI,
		end,

	rnd_v2_symm_or_rect, _T("SymmetricalOrRectangular"), TYPE_RADIOBTN_INDEX, 0, IDS_SYMMORRECT,
		p_default,		rbSymmetrical,
		p_ui, 			TYPE_RADIO, 2, IDC_SYMMETRICAL, IDC_RECTANGULAR,
		p_vals,			rbSymmetrical, rbRectangular,
		p_range,		rbSymmetrical, rbRectangular,
		p_accessor,		&updateUI,
		end,

	rnd_v2_width, 			_T("Width"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_WIDTH, 
		p_default, 		2.0f, 
		p_range, 		0.0f,1000000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_WIDTH,	IDC_WIDTHSPIN, 0.1f, 
		p_accessor,		&updateUI,
		end,

	rnd_v2_length, 			_T("Length"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_LENGTH, 
		p_default, 		6.0f, 
		p_range, 		0.0f,1000000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_LENGTH,	IDC_LENGTHSPIN, 0.1f, 
		p_accessor,		&updateUI,
		end,
	
	rnd_v2_angle2, 			_T("Angle2"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_ANGLE2, 
		p_default, 		0.0f, 
		p_range, 		-99999999.0f,99999999.0f,
		p_ui, 			TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_ANGLE2,	IDC_ANGLESPIN2, 0.1f, 
		p_accessor,		&updateUI,
		end,

	rnd_v2_aspect_lock, 	_T("LockAspect"), TYPE_BOOL, P_RESET_DEFAULT,	IDS_LOCK_ASPECT, 	
		p_default, 		FALSE, 
		p_ui, 			TYPE_CHECKBUTTON, IDC_ASPECTLOCK, 
		p_accessor,		&updateUI,
		end, 

	rnd_v2_vpt_symm_or_rect, _T("Viewport_SymmetricalOrRectangular"), TYPE_RADIOBTN_INDEX, 0, IDS_VIEW_SYMMORRECT,
		p_default,		rbSymmetrical,
		p_ui, 			TYPE_RADIO, 2, IDC_VIEW_SYMMETRICAL, IDC_VIEW_RECTANGULAR,
		p_vals,			rbSymmetrical, rbRectangular,
		p_range,		rbSymmetrical, rbRectangular,
		p_accessor,		&updateUI,
		end,

	rnd_v2_vpt_width, 		_T("Viewport_Width"), 		TYPE_FLOAT, 	0, 	IDS_VIEW_WIDTH, 
		p_default, 		2.0f, 
		p_range, 		0.0f,1000000.0f, 
//		p_ui, 			TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_THICKNESS,	IDC_THICKSPIN, 0.1f, 
		p_accessor,		&updateUI,
		end,

	rnd_v2_vpt_length, 		_T("Viewport_Length"), 		TYPE_FLOAT, 	0, 	IDS_VIEW_LENGTH, 
		p_default, 		6.0f, 
		p_range, 		0.0f,1000000.0f, 
//		p_ui, 			TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_THICKNESS,	IDC_THICKSPIN, 0.1f, 
		p_accessor,		&updateUI,
		end,
	
	rnd_v2_vpt_angle2, 			_T("Viewport_Angle2"), 		TYPE_FLOAT, 	0, 	IDS_VIEW_ANGLE2, 
		p_default, 		0.0f, 
		p_range, 		-99999999.0f,99999999.0f, 
//		p_ui, 			TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_THICKNESS,	IDC_THICKSPIN, 0.1f, 
		p_accessor,		&updateUI,
		end,

	rnd_v2_vpt_aspect_lock, 	_T("Viewport_LockAspect"), TYPE_BOOL, P_RESET_DEFAULT,	IDS_VIEW_LOCK_ASPECT, 	
		p_default, 		FALSE, 
		//p_ui, 			TYPE_CHECKBUTTON, IDC_ASPECTLOCK, 
		p_accessor,		&updateUI,
		end, 


	rnd_v2_autosmooth, 		_T("Autosmooth"), TYPE_BOOL, P_RESET_DEFAULT,	IDS_AUTOSMOOTH, 	
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_AUTOSMOOTH, 
		p_accessor,		&updateUI,
		end, 

		rnd_v2_threshold, _T("Threshold"), TYPE_ANGLE, P_RESET_DEFAULT|P_ANIMATABLE, IDS_THRESHOLD,
		p_default, 2*PI/9.0f,
		p_range, 0.0f, 180.0f,	
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_THRESHOLD, IDC_THRESHOLDSPIN, 0.1,
		p_accessor,		&updateUI,
		end,

	end
	);




IObjParam *RendSpline::ip			= NULL;

void RendSplineParamsMapDlgProc::UpdateRWS()
{
    BOOL usePhysUVs = mod->GetUsePhysicalScaleUVs();
    CheckDlgButton(mhWnd, IDC_REAL_WORLD_MAP_SIZE, usePhysUVs);
    EnableWindow(GetDlgItem(mhWnd, IDC_REAL_WORLD_MAP_SIZE), mod->GeneratesUVS());
}

INT_PTR RendSplineParamsMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
{

switch (msg) {
	case WM_INITDIALOG:
        mhWnd = hWnd;
		Initialize(hWnd,t);
		Update(hWnd,t);
        UpdateRWS();
		break;
	case CC_SPINNER_CHANGE:			
		{
		int id = LOWORD(wParam);
		ISpinnerControl *iSpin = (ISpinnerControl*)lParam;
		BOOL isViewport = (mod->UseViewOrRenderParams(t) == rbViewport);
		//if (!theHold.Holding()) theHold.Begin();
		//BOOL redraw = TRUE;
		switch ( id ) {
			case IDC_ASPECTSPIN:
			{
			if (isViewport)
					mod->OnSetAspect(hWnd,t, rnd_v2_vpt_width, rnd_v2_vpt_length);
			else
					mod->OnSetAspect(hWnd,t, rnd_v2_width, rnd_v2_length );
					
			}
			}
		return TRUE;
		}

   case WM_COMMAND:
       switch (LOWORD(wParam)) {
       case IDC_GENMAPPING:
           UpdateRWS();
		break;

       case IDC_REAL_WORLD_MAP_SIZE: {
           BOOL check = IsDlgButtonChecked(hWnd, IDC_REAL_WORLD_MAP_SIZE);
		theHold.Begin();
           mod->SetUsePhysicalScaleUVs(check);
           theHold.Accept(GetString(IDS_PARAM_CHANGE));
       }
     }
     break;

}
return FALSE;
}

void RendSplineParamsMapDlgProc::Initialize(HWND hWnd, TimeValue t)
{
	if (hWnd == NULL) return;
	else
	{
		ISpinnerControl *iSpin = GetISpinner(GetDlgItem(hWnd, IDC_ASPECTSPIN));
		iSpin->LinkToEdit(GetDlgItem(hWnd, IDC_ASPECT), EDITTYPE_FLOAT);
		iSpin->SetLimits(0.0f,99999999.0f);
		iSpin->SetAutoScale(TRUE);
		ReleaseISpinner(iSpin);

		if (RendSpline::hLockButton == NULL) {
			HBITMAP hBitmap, hMask;
			RendSpline::hLockButton = ImageList_Create(16, 15, TRUE, 2, 0);
			hBitmap = LoadBitmap(RendSplineDesc.HInstance(), MAKEINTRESOURCE(IDB_LOCK));
			hMask   = LoadBitmap(RendSplineDesc.HInstance(), MAKEINTRESOURCE(IDB_LOCKMASK));
			ImageList_Add(RendSpline::hLockButton,hBitmap,hMask);
			DeleteObject(hBitmap);
			DeleteObject(hMask);
		}

		ICustButton *lockAspect = GetICustButton(GetDlgItem(hWnd,IDC_ASPECTLOCK));
		lockAspect->SetImage(RendSpline::hLockButton,0,0,1,1,16,15);
		lockAspect->SetType(CBT_CHECK);
		ReleaseICustButton(lockAspect);

		CheckRadioButton(hWnd, IDC_VIEWPORT, IDC_RENDERER, IDC_RENDERER);

        BOOL usePhysUVs = mod->GetUsePhysicalScaleUVs();
        CheckDlgButton(hWnd, IDC_REAL_WORLD_MAP_SIZE, usePhysUVs);
	}

}
void RendSplineParamsMapDlgProc::EnableButtons(HWND hWnd, TimeValue t, BOOL display, BOOL useView)
{
	// Enable/Disable buttons acording to the UI
	int vp_or_render;
	if (display)
	{
		EnableWindow(GetDlgItem(hWnd, IDC_USE_VIEWPORT), TRUE);
		if (useView)
		{
			EnableWindow(GetDlgItem(hWnd, IDC_VIEWPORT), TRUE);
			// but don't change the state of the radio buttons
		}
		else
		{
			// disable viewport radio; set radio to renderer
			mod->pblock->GetValue(rnd_ViewportOrRender,t,vp_or_render, FOREVER);
			
			if(vp_or_render != rbRender)
			{
			mod->pblock->SetValue(rnd_ViewportOrRender,t,rbRender, FALSE);
			}

			EnableWindow(GetDlgItem(hWnd, IDC_VIEWPORT), FALSE);
		}
	}
	else
	{
		// disable use viewport settings and viewport radio; set radio to renderer
		EnableWindow(GetDlgItem(hWnd, IDC_USE_VIEWPORT), FALSE);
		mod->pblock->GetValue(rnd_ViewportOrRender,t,vp_or_render, FOREVER);
		if(vp_or_render != rbRender)
		{
		mod->pblock->SetValue(rnd_ViewportOrRender,t,rbRender, FALSE);
		}
		EnableWindow(GetDlgItem(hWnd, IDC_VIEWPORT), FALSE);
	}
}
void RendSplineParamsMapDlgProc::Update(HWND hWnd, TimeValue t, ParamID id)
{
	if (hWnd != NULL && mod != NULL)
	{
		Interval valid;
		switch (id)
		{
			case rnd_ViewportOrRender:
			{
				if(mod->UseViewOrRenderParams(t)== rbViewport) 
				{
					mod->pblock->GetMap()->ReplaceParam(rnd_thickness,rnd_viewThickness);
					mod->pblock->GetMap()->ReplaceParam(rnd_sides,rnd_viewSides);
					mod->pblock->GetMap()->ReplaceParam(rnd_angle,rnd_viewAngle);
					
					mod->pblock->GetMap()->ReplaceParam(rnd_v2_width,rnd_v2_vpt_width);
					mod->pblock->GetMap()->ReplaceParam(rnd_v2_length,rnd_v2_vpt_length);
					mod->pblock->GetMap()->ReplaceParam(rnd_v2_angle2,rnd_v2_vpt_angle2);
					mod->pblock->GetMap()->ReplaceParam(rnd_v2_aspect_lock,rnd_v2_vpt_aspect_lock);
					
					// NS: 10/21/04 This is kind of a hack. We need to have 2 Radio Button Sets in the dialog. 
					// One for Renderer and one for Viewport settings. Depening on which set we're editing 
					// We need to hide the other set. Unfortunately ReplaceParam doesn't work with Radio Buttons					
					//mod->pblock->GetMap()->ReplaceParam(rnd_v2_symm_or_rect,rnd_v2_vpt_symm_or_rect);

					ShowWindow(GetDlgItem(hWnd,IDC_SYMMETRICAL),SW_HIDE);
					ShowWindow(GetDlgItem(hWnd,IDC_RECTANGULAR),SW_HIDE);

					ShowWindow(GetDlgItem(hWnd,IDC_VIEW_SYMMETRICAL),SW_SHOW);
					ShowWindow(GetDlgItem(hWnd,IDC_VIEW_RECTANGULAR),SW_SHOW);

				}
				else
				{
					mod->pblock->GetMap()->ReplaceParam(rnd_thickness,rnd_thickness);
					mod->pblock->GetMap()->ReplaceParam(rnd_sides,rnd_sides);
					mod->pblock->GetMap()->ReplaceParam(rnd_angle,rnd_angle);
					
					mod->pblock->GetMap()->ReplaceParam(rnd_v2_width,rnd_v2_width);
					mod->pblock->GetMap()->ReplaceParam(rnd_v2_length,rnd_v2_length);
					mod->pblock->GetMap()->ReplaceParam(rnd_v2_angle2,rnd_v2_angle2);
					mod->pblock->GetMap()->ReplaceParam(rnd_v2_aspect_lock,rnd_v2_aspect_lock);
					
					// NS: 10/21/04 This is kind of a hack. We need to have 2 Radio Button Sets in the dialog. 
					// One for Renderer and one for Viewport settings. Depening on which set we're editing 
					// We need to hide the other set. Unfortunately ReplaceParam doesn't work with Radio Buttons					
					//mod->pblock->GetMap()->ReplaceParam(rnd_v2_vpt_symm_or_rect,rnd_v2_symm_or_rect);

					ShowWindow(GetDlgItem(hWnd,IDC_VIEW_SYMMETRICAL),SW_HIDE);
					ShowWindow(GetDlgItem(hWnd,IDC_VIEW_RECTANGULAR),SW_HIDE);

					ShowWindow(GetDlgItem(hWnd,IDC_SYMMETRICAL),SW_SHOW);
					ShowWindow(GetDlgItem(hWnd,IDC_RECTANGULAR),SW_SHOW);
					

				}
				break;
			}				
			case rnd_display:
			case rnd_useView:		
			{
				BOOL display, useView;
				mod->pblock->GetValue(rnd_display,t,display, valid); // sets value
				mod->pblock->GetValue(rnd_useView,t,useView, valid); // sets value
				EnableButtons(hWnd, t, display, useView);
			} break;
			case rnd_v2_symm_or_rect:
			case rnd_v2_vpt_symm_or_rect:
			{
				int rectangular;
				BOOL aspectLock;
				BOOL viewport = (mod->UseViewOrRenderParams(t) == rbViewport);
				if(viewport)
			{

					mod->pblock->GetValue(rnd_v2_vpt_symm_or_rect,t,rectangular,valid);
					mod->pblock->GetValue(rnd_v2_vpt_aspect_lock,t,aspectLock,valid);
				}
				else
			{
					mod->pblock->GetValue(rnd_v2_symm_or_rect,t,rectangular,valid);
					mod->pblock->GetValue(rnd_v2_aspect_lock,t,aspectLock,valid);
				}
				BOOL sym = (rectangular == rbSymmetrical);
				mod->pblock->GetMap()->Enable(rnd_thickness,sym);
				mod->pblock->GetMap()->Enable(rnd_sides,sym);
				mod->pblock->GetMap()->Enable(rnd_angle,sym);

				mod->pblock->GetMap()->Enable(rnd_v2_width,!sym);
				mod->pblock->GetMap()->Enable(rnd_v2_length,!sym);
				mod->pblock->GetMap()->Enable(rnd_v2_angle2,!sym);

				if(!sym)
					mod->CheckAspectLock(t);
				else
					mod->pblock->GetMap()->Enable(rnd_v2_aspect_lock,FALSE);

				EnableWindow(GetDlgItem(hWnd,IDC_THICKTEXT), sym);
				EnableWindow(GetDlgItem(hWnd,IDC_SIDESTEXT), sym);
				EnableWindow(GetDlgItem(hWnd,IDC_ANGLETEXT), sym);

				EnableWindow(GetDlgItem(hWnd,IDC_WIDTHTEXT), !sym);
				EnableWindow(GetDlgItem(hWnd,IDC_LENGTHTEXT), !sym);
				EnableWindow(GetDlgItem(hWnd,IDC_ANGLE2TEXT), !sym);
				EnableWindow(GetDlgItem(hWnd,IDC_ASPECTTEXT), !sym);

				ISpinnerControl *iSpin = GetISpinner(GetDlgItem(hWnd, IDC_ASPECTSPIN));
				iSpin->Enable(!sym);
				if(!aspectLock)
			{
					mod->mAspect = mod->GetAspect(t,viewport);
					iSpin->SetValue(mod->mAspect,FALSE);
				}
				else
				{
					if(!mod->mAspectValidity.InInterval(t))
					{
						mod->mAspect = mod->GetAspect(t,viewport);
						iSpin->SetValue(mod->mAspect,FALSE);
					}
				}
				ReleaseISpinner(iSpin);


			} break;
			//case rnd_v2_width:
			//	mod->OnSetWidthLength(hWnd, rnd_v2_length, rnd_v2_width, t, TRUE);
			//	break;
			//case rnd_v2_vpt_width:
			//	mod->OnSetWidthLength(hWnd, rnd_v2_vpt_length, rnd_v2_vpt_width, t, TRUE);
			//	break;
			//case rnd_v2_length:
			//	mod->OnSetWidthLength(hWnd, rnd_v2_length, rnd_v2_width, t, FALSE);
			//	break;
			//case rnd_v2_vpt_length:
			//	mod->OnSetWidthLength(hWnd, rnd_v2_vpt_length, rnd_v2_vpt_width, t, FALSE);
			//	break;
			case rnd_v2_aspect_lock:
			case rnd_v2_vpt_aspect_lock:
				mod->OnAspectLock(hWnd,t);
				break;

		}
	}
}

void RendSplineParamsMapDlgProc::Update(HWND hWnd, TimeValue t)
{
	//Update(hWnd, t, rnd_viewThickness);
	//Update(hWnd, t, rnd_sides);
	//Update(hWnd, t, rnd_viewSides);
	//Update(hWnd, t, rnd_angle);
	//Update(hWnd, t, rnd_viewAngle);
	//Update(hWnd, t, rnd_render);
	//Update(hWnd, t, rnd_genuvw);
	
	Update(hWnd, t, rnd_display);
	Update(hWnd, t, rnd_useView);
	Update(hWnd, t, rnd_ViewportOrRender);
	Update(hWnd, t, rnd_v2_symm_or_rect);
	Update(hWnd, t, rnd_v2_aspect_lock);
	//mod->UpdateAspect(hWnd,t);
}

	
//--- RendSpline -------------------------------------------------------
RendSpline::RendSpline(BOOL create)
{
	mAspectValidity = FOREVER;
	pblock = NULL;
	RendSplineDesc.MakeAutoParamBlocks(this);
#ifdef PHYSICAL_SCALE_UVS
    if (create)
        if (!GetPhysicalScaleUVsDisabled())
            SetUsePhysicalScaleUVs(true);
#endif
}

RendSpline::~RendSpline()
{

}


/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval RendSpline::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	Interval valid = GetValidity(t);	
	return valid;
}


/*************************************************************************************************
*
	Between NotifyPreCollapse and NotifyPostCollapse, Modify is
	called by the system.  NotifyPreCollapse can be used to save any plugin dependant data e.g.
	LocalModData
*
\*************************************************************************************************/

void RendSpline::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
	//TODO:  Perform any Pre Stack Collapse methods here
}



/*************************************************************************************************
*
	NotifyPostCollapse can be used to apply the modifier back onto to the stack, copying over the
	stored data from the temporary storage.  To reapply the modifier the following code can be 
	used

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID)
	{
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	}
	else
		derob = (IDerivedObject*) bo;

	// Add ourselves to the top of the stack
	derob->AddModifier(this,NULL,derob->NumModifiers());

*
\*************************************************************************************************/

void RendSpline::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{
	//TODO: Perform any Post Stack collapse methods here.

}


/*************************************************************************************************
*
	ModifyObject will do all the work in a full modifier
    This includes casting objects to their correct form, doing modifications
	changing their parameters, etc
*
\************************************************************************************************/

void RendSpline::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	Interval valid = GetValidity(t);
	// and intersect it with the channels we use as input (see ChannelsUsed)
	valid &= os->obj->ChannelValidity(t,TOPO_CHAN_NUM);
	valid &= os->obj->ChannelValidity(t,GEOM_CHAN_NUM);

//	float nlength;
	
	BOOL doRender = false, doDisplay = false, doUVW = false, useView = false, autosmooth = false, aspectLock = false, vAspectLock;
	float threshold, width, length, angle2, vwidth, vlength, vangle2;
	int rectangular, vRectangular;
	pblock->GetValue(rnd_thickness,t,nlength,valid);
	pblock->GetValue(rnd_sides,t,nsides,valid);
	pblock->GetValue(rnd_angle,t,nangle,valid);
	pblock->GetValue(rnd_viewThickness,t,vthickness,valid);
	pblock->GetValue(rnd_viewSides,t,vsides,valid);
	pblock->GetValue(rnd_viewAngle,t,vangle,valid);
	pblock->GetValue(rnd_render,t,doRender,valid);
	pblock->GetValue(rnd_display,t,doDisplay,valid);
	pblock->GetValue(rnd_genuvw,t,doUVW,valid);
	pblock->GetValue(rnd_useView,t,useView,valid);
	pblock->GetValue(rnd_v2_symm_or_rect,t,rectangular,valid);
	pblock->GetValue(rnd_v2_length,t,length,valid);
	pblock->GetValue(rnd_v2_width,t,width,valid);
	pblock->GetValue(rnd_v2_angle2,t,angle2,valid);
	pblock->GetValue(rnd_v2_aspect_lock,t,aspectLock,valid);
	pblock->GetValue(rnd_v2_vpt_symm_or_rect,t,vRectangular,valid);
	pblock->GetValue(rnd_v2_vpt_length,t,vlength,valid);
	pblock->GetValue(rnd_v2_vpt_width,t,vwidth,valid);
	pblock->GetValue(rnd_v2_vpt_angle2,t,vangle2,valid);
	pblock->GetValue(rnd_v2_vpt_aspect_lock,t,vAspectLock,valid);
	pblock->GetValue(rnd_v2_autosmooth,t,autosmooth,valid);
	pblock->GetValue(rnd_v2_threshold,t,threshold,valid);
	BOOL usePhysUVs = GetUsePhysicalScaleUVs();
	
	mAspectLock = aspectLock;

	theHold.Suspend(); // need to suspend Undo
	SplineShape *shape = (SplineShape *)os->obj; //->ConvertToType(t,splineShapeClassID);
	
	shape->SetRenderable(doRender);
		shape->SetDispRenderMesh(doDisplay);
	if(doRender || doDisplay)
		shape->SetGenUVW(doUVW);

	nlength = nlength < 0.0f ? 0.0f : (nlength > 1000000.0f ? 1000000.0f : nlength);
	vthickness = vthickness < 0.0f ? 0.0f : (vthickness > 1000000.0f ? 1000000.0f : vthickness);

	nsides = nsides < 0 ? 0 : (nsides > 100? 100 : nsides);
	vsides = vsides < 0 ? 0 : (vsides > 100? 100 : vsides);

	shape->SetThickness(t,nlength);
	shape->SetSides(t,nsides);
	shape->SetAngle(t,nangle);

	shape->SetViewportAngle(vangle);
	shape->SetViewportSides(vsides);
	shape->SetViewportThickness(vthickness);

	shape->SetUseViewport(useView);

	shape->SetUsePhysicalScaleUVs(usePhysUVs);

	IShapeRectRenderParams *rparams = (IShapeRectRenderParams *)shape->GetProperty(SHAPE_RECT_RENDERPARAMS_PROPID);
	if(rparams)
	{
		rparams->SetAutosmooth(t,autosmooth);
		rparams->SetAutosmoothThreshold(t, threshold);
		rparams->SetRectangular(t,rectangular == rbRectangular);

		rparams->SetAngle2(t,angle2);
		rparams->SetLength(t,length);
		rparams->SetWidth(t,width);

		rparams->SetVPTRectangular(t,vRectangular == rbRectangular);
		rparams->SetVPTAngle2(t,vangle2);
		rparams->SetVPTLength(t,vlength);
		rparams->SetVPTWidth(t,vwidth);
	}

	shape->shape.UpdateSels();	
	shape->shape.InvalidateGeomCache();
	theHold.Resume(); // it's now safe to resume Undo


	os->obj->SetChannelValidity(TOPO_CHAN_NUM, valid);
	os->obj->SetChannelValidity(GEOM_CHAN_NUM, valid);
	os->obj->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
	os->obj->SetChannelValidity(MTL_CHAN_NUM, valid);
	os->obj->SetChannelValidity(SELECT_CHAN_NUM, valid);
	os->obj->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
	os->obj->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

	os->obj = shape;

	//os->obj->UnlockObject();


}


void RendSpline::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;

	ip->EnableShowEndResult(FALSE);

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	RendSplineDesc.BeginEditParams(ip, this, flags, prev);

	// pointcache_param_blk.SetUserDlgProc(new PointCacheParamsMapDlgProc(this));

	paramDlgProc = new RendSplineParamsMapDlgProc(this);
	rendspline_param_blk.SetUserDlgProc(paramDlgProc);

}

void RendSpline::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	RendSplineDesc.EndEditParams(ip, this, flags, next);

	TimeValue t = ip->GetTime();

	paramDlgProc = NULL;
	ip->EnableShowEndResult(TRUE);


	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;
}



Interval RendSpline::GetValidity(TimeValue t)
{
	float f;
	int i;
	BOOL b;
	Interval valid = FOREVER;

	// Start our interval at forever...
	// Intersect each parameters interval to narrow it down.
	pblock->GetValue(rnd_thickness,t,f,valid);
	pblock->GetValue(rnd_sides,t,i,valid);
	pblock->GetValue(rnd_angle,t,f,valid);
	pblock->GetValue(rnd_viewThickness,t,f,valid);
	pblock->GetValue(rnd_viewSides,t,i,valid);
	pblock->GetValue(rnd_viewAngle,t,f,valid);
	pblock->GetValue(rnd_render,t,b,valid);
	pblock->GetValue(rnd_display,t,b,valid);
	pblock->GetValue(rnd_genuvw,t,b,valid);
	pblock->GetValue(rnd_useView,t,b,valid);
	pblock->GetValue(rnd_v2_autosmooth,t,b,valid);
	pblock->GetValue(rnd_v2_threshold,t,f,valid);
	return valid;
}



RefTargetHandle RendSpline::Clone(RemapDir& remap)
{
	RendSpline* newmod = new RendSpline(false);	
	newmod->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock));
	BaseClone(this, newmod, remap);
    newmod->SetUsePhysicalScaleUVs(GetUsePhysicalScaleUVs());
	return(newmod);
}


//From ReferenceMaker 
RefResult RendSpline::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	if(hTarget == pblock && message == REFMSG_CHANGE)
	{
		ParamID changing_param = pblock->LastNotifyParamID();
		IParamMap2 *map = pblock->GetMap();
		if(!map)
			return REF_SUCCEED;

		switch(changing_param)
		{
			case rnd_v2_width:
				OnSetWidthLength(map->GetHWnd(), rnd_v2_length, rnd_v2_width, GetCOREInterface()->GetTime(), TRUE);
				break;
			case rnd_v2_vpt_width:
				OnSetWidthLength(map->GetHWnd(), rnd_v2_vpt_length, rnd_v2_vpt_width, GetCOREInterface()->GetTime(), TRUE);
				break;
			case rnd_v2_length:
				OnSetWidthLength(map->GetHWnd(), rnd_v2_length, rnd_v2_width, GetCOREInterface()->GetTime(), FALSE);
				break;
			case rnd_v2_vpt_length:
				OnSetWidthLength(map->GetHWnd(), rnd_v2_vpt_length, rnd_v2_vpt_width, GetCOREInterface()->GetTime(), FALSE);
				break;
		}
	}

	return REF_SUCCEED;
}

/****************************************************************************************
*
 	NotifyInputChanged is called each time the input object is changed in some way
 	We can find out how it was changed by checking partID and message
*
\****************************************************************************************/

void RendSpline::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL RendSpline::HasUVW() 
{ 
	BOOL doUVW, doRender, doDisplay;
	pblock->GetValue(rnd_render,0.0f,doRender,FOREVER); // check if it's renderable
	pblock->GetValue(rnd_display,0.0f,doDisplay,FOREVER); // check if it's Displayed
	pblock->GetValue(rnd_genuvw,0.0f,doUVW,FOREVER); // check if UVW is on
	return (doUVW && (doRender|| doDisplay)); // if it's renderable and UVW is on, returns true
}

BOOL RendSpline::GeneratesUVS() 
{ 
	BOOL doUVW;
	pblock->GetValue(rnd_genuvw,0.0f,doUVW,FOREVER); // check if UVW is on
	return doUVW; // if it's renderable and UVW is on, returns true
}

void RendSpline::UpdateUI()
{
	if (ip == NULL)
		return;
    if (paramDlgProc != NULL)
        paramDlgProc->UpdateRWS();
}

BOOL RendSpline::GetUsePhysicalScaleUVs()
{
    return ::GetUsePhysicalScaleUVs(this);
}


void RendSpline::SetUsePhysicalScaleUVs(BOOL flag)
{
    BOOL curState = GetUsePhysicalScaleUVs();
    if (curState == flag)
        return;
    if (theHold.Holding())
        theHold.Put(new RealWorldScaleRecord<RendSpline>(this, curState));
    ::SetUsePhysicalScaleUVs(this, flag);
    if (pblock != NULL)
		pblock->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	UpdateUI();
	macroRec->SetProperty(this, _T("realWorldMapSize"), mr_bool, flag);
}

void RendSpline::SetGenUVW(BOOL sw) 
{  

	if (sw==HasUVW()) return;
	else
	{
		BOOL doRender, doDisplay;
		pblock->GetValue(rnd_render,0.0f,doRender,FOREVER); // check if it's renderable
		pblock->GetValue(rnd_display,0.0f,doDisplay,FOREVER); // check if it's renderable
		if (doRender || doDisplay)
			pblock->SetValue(rnd_genuvw,0.0f,sw,0); // sets UVW ON if needed
	}
	UpdateUI();
	//TODO: Set the plugin internal value to sw				
}

void RendSpline::CheckAspectLock(TimeValue t)
{
	int lengthID, widthID, aspectlockID;
	float length, width, aspect = 1.0f;
	BOOL aspectlocked = FALSE;

	if(UseViewOrRenderParams(t) == rbViewport)
{
		widthID = rnd_v2_vpt_width;
		lengthID = rnd_v2_vpt_length;
		aspectlockID = rnd_v2_vpt_aspect_lock;
	}
	else
	{
		widthID = rnd_v2_width;
		lengthID = rnd_v2_length;
		aspectlockID = rnd_v2_aspect_lock;
	}
	pblock->GetValue(lengthID,t,length,FOREVER);
	pblock->GetValue(widthID,t,width,FOREVER);
	pblock->GetValue(aspectlockID,t,aspectlocked,FOREVER);	
	
	IParamMap2 *map = pblock->GetMap(0);
	
	if(map)
	{
		ISpinnerControl *iSpin = GetISpinner(GetDlgItem(map->GetHWnd(), IDC_ASPECTSPIN));
		aspect = iSpin->GetFVal();
}

	pblock->GetMap()->Enable(aspectlockID,((length != 0.0f && width != 0.0f && aspect != 0.0f) || aspectlocked));
}
//void RendSpline::UpdateAspect(HWND hWnd, TimeValue t)
//{
//	
//	//ISpinnerControl *iSpin = GetISpinner(GetDlgItem(hWnd, IDC_ASPECTSPIN));
//	//
//	//iSpin->SetValue(this->GetAspect((UseViewOrRenderParams(t)== rbViewport)),FALSE);
//	//ReleaseISpinner(iSpin);
//}
void RendSpline::OnSetWidthLength(HWND hWnd, int lengthID, int widthID, TimeValue t, BOOL widthSet)
{
	static BOOL reenter = FALSE;
	if(reenter)
		return;
	reenter = TRUE;
	ISpinnerControl *iSpin = GetISpinner(GetDlgItem(hWnd, IDC_ASPECTSPIN));
	BOOL aspectlocked;
	float length, width;

	if(UseViewOrRenderParams(t) == rbViewport)
		pblock->GetValue(rnd_v2_vpt_aspect_lock,t,aspectlocked,FOREVER);
	else
		pblock->GetValue(rnd_v2_aspect_lock,t,aspectlocked,FOREVER);

	pblock->GetValue(lengthID,t,length,FOREVER);
	pblock->GetValue(widthID,t,width,FOREVER);

	
	if(!aspectlocked)
	{
		CheckAspectLock(t);

		if(width == 0.0f)
		{
			width = EPS;
			//iSpin->SetValue(0.0f, FALSE);
			//iSpin->SetIndeterminate(TRUE);
		}
		else
			iSpin->SetValue(length/width, FALSE);
			
	}
	else
	{
		if(widthSet)
		{
			pblock->SetValue(lengthID,t,mAspect*width);
		}
		else
		{
			if(mAspect == 0.0f)
				mAspect = EPS;
				//pblock->SetValue(widthID,t,0.0f);
			pblock->SetValue(widthID,t,length/mAspect);
		}
	}

	ReleaseISpinner(iSpin);
	reenter = FALSE;
}

void RendSpline::OnSetAspect(HWND hWnd, TimeValue t, int widthID, int lengthID)
{
	CheckAspectLock(t);

	ISpinnerControl *iSpin = GetISpinner(GetDlgItem(hWnd, IDC_ASPECTSPIN));
	mAspect = iSpin->GetFVal();
	//if(mAspect < EPS)
	//{
	//	mAspect = EPS;
	//	iSpin->SetValue(mAspect,FALSE);
//}
	float width;

	pblock->GetValue(widthID,t,width,FOREVER);
	pblock->SetValue(lengthID,t,mAspect*width);

	ReleaseISpinner(iSpin);
}

void RendSpline::OnAspectLock(HWND hWnd, TimeValue t)
{
	BOOL aspectLock;

	if(UseViewOrRenderParams(t) == rbViewport)
		pblock->GetValue(rnd_v2_vpt_aspect_lock,t,aspectLock,FOREVER);
	else
		pblock->GetValue(rnd_v2_aspect_lock,t,aspectLock,FOREVER);

	ISpinnerControl *iSpin = GetISpinner(GetDlgItem(hWnd, IDC_ASPECTSPIN));

	if(aspectLock)
	{	
		if(!mAspectLock)
			mAspect = GetAspect(t,(UseViewOrRenderParams(t)== rbViewport));

		iSpin->SetValue(mAspect,FALSE);
		iSpin->Disable();
		mAspectLock = aspectLock;
	}
	
	ReleaseISpinner(iSpin);
}

int RendSpline::UseViewOrRenderParams(TimeValue t)
{
	int whichval;
	pblock->GetValue(rnd_ViewportOrRender,t,whichval, FOREVER);
	return whichval;
}

float RendSpline::GetAspect(TimeValue t, BOOL viewport)
{
	float length;
	float width;
	int lengthIdx, widthIdx;
	if(viewport)
	{
		lengthIdx = rnd_v2_vpt_length;
		widthIdx = rnd_v2_vpt_width;
	}
	else
	{
		lengthIdx = rnd_v2_length;
		widthIdx = rnd_v2_width;
	}
	mAspectValidity = FOREVER;
	pblock->GetValue(lengthIdx,t,length,mAspectValidity);
	pblock->GetValue(widthIdx,t,width,mAspectValidity);
	if(width == 0)
		return 0;
	return length/width;
}
