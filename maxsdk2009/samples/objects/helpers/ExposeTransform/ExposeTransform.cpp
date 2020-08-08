/**********************************************************************
 *<
	FILE: ExposeTransform.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: Michael Zyracki

	HISTORY: 

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/
#include "ExposeTransform.h"
#include "ExposeControllers.h"
#include "euler.h"
#include "notify.h"
#include "INodeValidity.h"
#include "modstack.h"

#define AXIS_LENGTH 20.0f
#define ZFACT (float).005;
#define RadToDegDbl	(180.0f / 3.141592f)


void AxisViewportRect(ViewExp *vpt, const Matrix3 &tm, float length, Rect *rect);
void DrawAxis(ViewExp *vpt, const Matrix3 &tm, float length, BOOL screenSize);
Box3 GetAxisBox(ViewExp *vpt, const Matrix3 &tm,float length,int resetTM);



static ExposeTransformClassDesc ExposeTransformDesc;
ClassDesc2* GetExposeTransformDesc() { return &ExposeTransformDesc; }

//used by the UI for setting up the euler dropdown
#define NUM_SUPPORTED_EULERS		6
#define NUM_SUPPORTED_EULERS_MINUSONE	5
static int eulerIDs[] = {
    IDS_EULERTYPE0,IDS_EULERTYPE1,IDS_EULERTYPE2,
    IDS_EULERTYPE3,IDS_EULERTYPE4,IDS_EULERTYPE5,
    IDS_EULERTYPE6,IDS_EULERTYPE7,IDS_EULERTYPE8};

IObjParam *ExposeTransform::ip = NULL;
ExposeTransform *ExposeTransform::editOb = NULL;
// block IDs
enum { exposetransform_expose,exposetransform_info,exposetransform_display,exposetransform_output };
//expose params 
//NOTE if you change these guys then you must reset the min/max defines
//since they depend upon them
enum { 
	pb_localeuler,pb_localeulerx,pb_localeulery,pb_localeulerz,
	pb_worldeuler,pb_worldeulerx,pb_worldeulery,pb_worldeulerz,
	pb_localposition,pb_localpositionx,	pb_localpositiony,pb_localpositionz,
	pb_worldposition,pb_worldpositionx, pb_worldpositiony,pb_worldpositionz,
	pb_distance,
	pb_worldboundingbox_size,pb_worldboundingbox_width,pb_worldboundingbox_length,pb_worldboundingbox_height,
	pb_angle

};
#define pb_expose_min	pb_localeuler
#define pb_expose_max	pb_angle

//info params
enum {
	pb_expose_node,
	pb_reference_node,
	pb_stripnuscale,
	pb_x_order, pb_y_order,pb_z_order,
	pb_use_parent_as_reference,
	pb_use_time_offset,
	pb_time_offset
};

//output params
enum {pb_display_exposed};

//enums for time choice
enum {time_current,time_track};


//display enums
enum { 
	pb_pointobj_size, pb_pointobj_centermarker, pb_pointobj_axistripod, 
	pb_pointobj_cross, pb_pointobj_box, pb_pointobj_screensize, pb_pointobj_drawontop };


//post load callback to set up the controls with the exposetransform pointer
class ExposeTransformPLCB : public PostLoadCallback 
{
public:
	ExposeTransform *eT;
	ExposeTransformPLCB(ExposeTransform* e)
	{ 
		eT = e;
	}
	void proc(ILoad *iload)
	{
		//set up all of the pblock params..
		for(int i = pb_expose_min;i<= pb_expose_max;++i)
		{
			BaseExposeControl * control = (BaseExposeControl*) eT->pblock_expose->GetController(i,0);
			if(control)
			{
				//check for bad  eulers..
				if((i==pb_localeuler||i==pb_worldeuler)&&control->ClassID()!=EulerExposeController_CLASS_ID)
				{
					eT->CreateController(i);
				}
				else
				{
					control->exposeTransform = eT;
					control->paramID = i;
				}
			}
			else //from an older version ..we need to create the paramter.
			{
				eT->CreateController(i);
			}
		}
		delete this;
	}

};


class ExposeTMPBAccessor : public PBAccessor
{
public:
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
		if(owner)
		{
			ExposeTransform *eT = (ExposeTransform*)owner;
			BaseExposeControl * control;
			switch (id)
			{
				//the bad ones.
				case pb_localeuler:
					//localeuler
					control = (BaseExposeControl*) eT->pblock_expose->GetController(pb_localeulerx,0);
					if(control)
					{
						control->GetValue(t,&(v.p->x),FOREVER);
					}
					control = (BaseExposeControl*) eT->pblock_expose->GetController(pb_localeulery,0);
					if(control)
					{
						control->GetValue(t,&(v.p->y),FOREVER);
					}
					control = (BaseExposeControl*) eT->pblock_expose->GetController(pb_localeulerz,0);
					if(control)
					{
						control->GetValue(t,&(v.p->z),FOREVER);
					}
					break;
				case pb_worldeuler:
					//worldeuler
					control = (BaseExposeControl*) eT->pblock_expose->GetController(pb_worldeulerx,0);
					if(control)
					{
						control->GetValue(t,&(v.p->x),FOREVER);
					}
					control = (BaseExposeControl*) eT->pblock_expose->GetController(pb_worldeulery,0);
					if(control)
					{
						control->GetValue(t,&(v.p->y),FOREVER);
					}
					control = (BaseExposeControl*) eT->pblock_expose->GetController(pb_worldeulerz,0);
					if(control)
					{
						control->GetValue(t,&(v.p->z),FOREVER);
					}
					break;
			
			}
		}
	}

	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		//no sets allowed!
	}

	BOOL KeyFrameAtTime(ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if(owner)
		{
			ExposeTransform *eT = (ExposeTransform*)owner;
			BaseExposeControl * control;
			switch (id)
			{
				case pb_localeuler:
				case pb_worldeuler:
					//currently this really doesn't do anything since we the expose controller isn't keyable, but may change(probably not).
					control = (BaseExposeControl*) eT->pblock_expose->GetController(id,0);
					if(control)
					{
						return control->IsKeyAtTime(t,0);
					}
					break;
			}
		}
		return FALSE;
	}
		
};

static ExposeTMPBAccessor exposePBAccessor;




static ParamBlockDesc2 exposetransform_expose_blk (
	exposetransform_expose, _T("params"), 0, &ExposeTransformDesc, 
	P_AUTO_CONSTRUCT,PBLOCK_EXPOSE,
	// params
	pb_localeuler, 		_T("localEuler"), 		TYPE_POINT3, 	P_READ_ONLY|P_READ_SECOND_FLAG_VALUE|P_ANIMATABLE|P_TRANSIENT,
		P_USE_ACCESSOR_ONLY,IDS_LOCALEULER, 
		p_accessor,		&exposePBAccessor,	
		end,
	pb_localeulerx, 	_T("localEulerX"), 		TYPE_ANGLE, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_LOCALEULERX, 
		end,
	pb_localeulery, 	_T("localEulerY"), 		TYPE_ANGLE, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_LOCALEULERY, 
		end,
	pb_localeulerz, 	_T("localEulerZ"), 		TYPE_ANGLE, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_LOCALEULERZ, 
		end,
	pb_worldeuler, 		_T("worldEuler"), 		TYPE_POINT3, 	P_READ_ONLY|P_READ_SECOND_FLAG_VALUE|P_ANIMATABLE|P_TRANSIENT,
	P_USE_ACCESSOR_ONLY,IDS_WORLDEULER, 
		p_accessor,		&exposePBAccessor,	
		end,
	pb_worldeulerx, 	_T("worldEulerX"), 		TYPE_ANGLE, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDEULERX, 
		end,
	pb_worldeulery, 	_T("worldEulerY"), 		TYPE_ANGLE, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDEULERY, 
		end,
	pb_worldeulerz, 	_T("worldEulerZ"), 		TYPE_ANGLE, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDEULERZ, 
		end,
	pb_localposition, 	_T("localPosition"), 	TYPE_POINT3, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_LOCALPOSITION, 
		end,
	pb_localpositionx, 	_T("localPositionX"), 	TYPE_WORLD, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_LOCALPOSITIONX, 
		end,
	pb_localpositiony, 	_T("localpositionY"), 	TYPE_WORLD, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_LOCALPOSITIONY, 
		end,
	pb_localpositionz, 	_T("localPositionZ"), 	TYPE_WORLD, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_LOCALPOSITIONZ, 
		end,
	pb_worldposition, 	_T("worldPosition"), 	TYPE_POINT3, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDPOSITION, 
		end,
	pb_worldpositionx, 	_T("worldPositionX"), 	TYPE_WORLD, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDPOSITIONX, 
		end,
	pb_worldpositiony, 	_T("worldPositionY"), 	TYPE_WORLD, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDPOSITIONY, 
		end,
	pb_worldpositionz, 	_T("worldPositionZ"), 	TYPE_WORLD, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDPOSITIONZ, 
		end,
	pb_distance, 	_T("distance"),				 TYPE_FLOAT,	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_DISTANCE, 
		end,
	pb_worldboundingbox_size, 	_T("worldBoundingBoxSize"), TYPE_POINT3, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDBOUNDINGBOX_SIZE, 
		end,
	pb_worldboundingbox_width, 	_T("worldBoundingBoxWidth"), TYPE_FLOAT, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDBOUNDINGBOX_WIDTH, 
		end,
	pb_worldboundingbox_length,	_T("worldBoundingBoxLength"), TYPE_FLOAT, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDBOUNDINGBOX_LENGTH, 
		end,
	pb_worldboundingbox_height,	_T("worldBoundingBoxHeight"), TYPE_FLOAT, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_WORLDBOUNDINGBOX_HEIGHT, 
		end,
	pb_angle, 	_T("angle"), TYPE_ANGLE, 	P_READ_ONLY|P_ANIMATABLE|P_TRANSIENT, 	IDS_ANGLE, 
		end,
	end
	);

//for the info UI
class ExposeTransformInfoPBAccessor : public PBAccessor
{ 
	public:
	void Set(PB2Value &v,ReferenceMaker *owner,ParamID id,int tabIndex,TimeValue t)
	{
		ExposeTransform* p = (ExposeTransform*)owner;
		switch (id)
		{
		case pb_use_time_offset:
			p->InitTimeOffset();
			p->RecalcData();
			p->InitOutputUI(GetCOREInterface()->GetTime());
			break;
		case pb_expose_node:
		case pb_use_parent_as_reference:
			p->InitRefTarg();
			p->RecalcData();
			p->InitOutputUI(GetCOREInterface()->GetTime());
			break;
		}
	}
};
static ExposeTransformInfoPBAccessor exposetransforminfo_accessor;


class InfoTransformDlgProcType : public ParamMap2UserDlgProc 
{
	public:
		INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			ExposeTransform *exposeTransform = (ExposeTransform*)map->GetParamBlock()->GetOwner();
			switch (msg) 
			{
	           case WM_INITDIALOG:
                    exposeTransform->InitInfoUI();
					break;
		      case WM_COMMAND:
				    exposeTransform->WMInfoCommand(LOWORD(wParam), HIWORD(wParam), hWnd,(HWND)lParam);
			        break;          

			}
			return FALSE;
		}
		void DeleteThis() 
		{}
};
static InfoTransformDlgProcType ExposeTransformInfoProc;

static ParamBlockDesc2 exposetransform_info_blk( 
	
	exposetransform_info, _T("ExposeTransformInfo"),  0, &ExposeTransformDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, PBLOCK_INFO,

	//rollout
	IDD_INFO, IDS_INFO_PARAMS, 0, 0, & ExposeTransformInfoProc,

	// params
	pb_expose_node, 	_T("exposeNode"), TYPE_INODE,  0,	IDS_EXTRACT_NODE,
		p_ui, 			TYPE_PICKNODEBUTTON, IDC_EXTRACT_NODE, 
//		p_validator,    &ExposeValidator,
		p_prompt,    	IDS_EXTRACT_NODE,
		p_accessor,		&exposetransforminfo_accessor,
		end,
	pb_reference_node, 	_T("localReferenceNode"), TYPE_INODE,  0,	IDS_REFERENCE_NODE,
		p_ui, 			TYPE_PICKNODEBUTTON, IDC_REFERENCE_NODE, 
	//	p_validator,    &BipValidator,
		p_prompt,    	IDS_REFERENCE_NODE,
	//	p_accessor,		&DelegBipAcc,
		end,
	pb_use_parent_as_reference,		_T("useParent"),	TYPE_BOOL, P_RESET_DEFAULT, IDS_USE_PARENT_REF, 
		p_default, 		TRUE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_USE_PARENT_REF, 
		p_accessor,		&exposetransforminfo_accessor,
		end,	
	pb_stripnuscale,	_T("stripNUScale"),	TYPE_BOOL, P_RESET_DEFAULT, IDS_STRIPNUSCALE, 
		p_default, 		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_STRIPNUSCALE, 
		end,	

	pb_x_order,	_T("eulerXOrder"),	TYPE_INT, 0, IDS_XORDER, 
		p_default, 		1,	
		p_range, 		1, NUM_SUPPORTED_EULERS, 
		end,
	pb_y_order,	_T("eulerYOrder"),	TYPE_INT, 0, IDS_YORDER, 
		p_default, 		1,	
		p_range, 		1, NUM_SUPPORTED_EULERS, 
		end,
	pb_z_order,	_T("eulerZOrder"),	TYPE_INT, 0, IDS_ZORDER, 
		p_default, 		1,	
		p_range, 		1, NUM_SUPPORTED_EULERS, 
		end,
	pb_use_time_offset,	_T("useTimeOffset"), TYPE_BOOL, P_RESET_DEFAULT, IDS_USE_TIME_OFFSET,
		p_default, 		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_USE_TIME_OFFSET, 
		p_accessor,		&exposetransforminfo_accessor,
		end,
	pb_time_offset, _T("timeOffset"), TYPE_TIMEVALUE,P_ANIMATABLE,IDS_TIME_OFFSET,
		p_default,	0,
		p_range,   -99999999, 99999999,
		p_ui,		TYPE_SPINNER,EDITTYPE_TIME,IDC_TIME_OFFSET_EDIT,IDC_TIME_OFFSET_SPIN,1.0f,
		end,
	end
	);

static ParamBlockDesc2 exposetransform_display_blk( 
	
	exposetransform_display, _T("ExposeTransformDisplay"),  0, &ExposeTransformDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, PBLOCK_DISPLAY,

	//rollout
	IDD_DISPLAY, IDS_DISPLAY_PARAMS, 0, 0, NULL,

	// params
	pb_pointobj_size, _T("size"), TYPE_WORLD, P_ANIMATABLE, IDS_POINT_SIZE,
		p_default, 		20.0,	
		p_ms_default,	20.0,
		p_range, 		0.0f, float(1.0E30), 
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_POINT_SIZE, IDC_POINT_SIZESPIN, SPIN_AUTOSCALE, 
		end, 


	pb_pointobj_centermarker, 	_T("centermarker"), TYPE_BOOL, P_ANIMATABLE, IDS_POINT_CENTERMARKER,
		p_default, 			FALSE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_POINT_MARKER, 
		end, 

	pb_pointobj_axistripod, 	_T("axistripod"), TYPE_BOOL, P_ANIMATABLE, IDS_POINT_AXISTRIPOD,
		p_default, 			FALSE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_POINT_AXIS, 
		end, 

	pb_pointobj_cross, 		_T("cross"), TYPE_BOOL, P_ANIMATABLE, IDS_POINT_CROSS,
		p_default, 			TRUE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_POINT_CROSS, 
		end, 

	pb_pointobj_box, 			_T("box"), TYPE_BOOL, P_ANIMATABLE, IDS_POINT_BOX,
		p_default, 			FALSE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_POINT_BOX, 
		end, 

	pb_pointobj_screensize,	_T("constantscreensize"), TYPE_BOOL, P_ANIMATABLE, IDS_POINT_SCREENSIZE,
		p_default, 			FALSE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_POINT_SCREENSIZE,
		end, 

	pb_pointobj_drawontop,	    _T("drawontop"),       TYPE_BOOL, P_ANIMATABLE, IDS_POINT_DRAWONTOP,
		p_default, 			FALSE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_POINT_DRAWONTOP,
		end, 
		
	end
	);





//for the output UI
class ExposeTransformOutputPBAccessor : public PBAccessor
{ 
	public:
	void Set(PB2Value &v,ReferenceMaker *owner,ParamID id,int tabIndex,TimeValue t)
	{
		ExposeTransform* p = (ExposeTransform*)owner;
		switch (id)
		{
		case pb_display_exposed:
			p->InitOutputUI(GetCOREInterface()->GetTime());
			break;
		}
	}
};
static ExposeTransformOutputPBAccessor exposetransformoutput_accessor;

class ExposeTransformOutputDlgProcType : public ParamMap2UserDlgProc 
{
	public:
		INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			ExposeTransform *exposeTransform = (ExposeTransform*)map->GetParamBlock()->GetOwner();
			switch (msg) 
			{
	           case WM_INITDIALOG:
                    exposeTransform->InitOutputUI(GetCOREInterface()->GetTime());
					break;
		      case WM_COMMAND:
				    exposeTransform->WMOutputCommand(LOWORD(wParam), HIWORD(wParam), hWnd,(HWND)lParam);
			        break;          
			   case WM_DESTROY:
				   exposeTransform->DeleteCustomEdits();
				   exposeTransform->DeleteCustomButtons();
					break;
			   case WM_PAINT:
				   if(exposeTransform->valid==FALSE)
				   {
                       exposeTransform->InitOutputUI(GetCOREInterface()->GetTime());
				   }
				   break;

			}
			return FALSE;
		}
		void DeleteThis() 
		{}
};
static ExposeTransformOutputDlgProcType ExposeTransformOutputProc;

static ParamBlockDesc2 exposetransform_output_blk (
	exposetransform_output, _T("otuput"), 0, &ExposeTransformDesc, 
	P_AUTO_CONSTRUCT+P_AUTO_UI,PBLOCK_OUTPUT,
	//rollout
	IDD_EXPOSE, IDS_EXPOSED_VALUES, 0, 0, & ExposeTransformOutputProc,
	// params
	pb_display_exposed,	_T("displayExposedVals"),	TYPE_BOOL, P_RESET_DEFAULT, IDS_DISPLAY_EXPOSED, 
		p_default, 		TRUE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_DISPLAY_EXPOSED_VALUES, 
		p_accessor,		&exposetransformoutput_accessor,
		end,	
	end
	);





ExposeTransform::ExposeTransform()
{	
	pblock_expose = pblock_info = pblock_display = pblock_output = NULL;
	ExposeTransformDesc.MakeAutoParamBlocks(this);
	suspendRecalc = FALSE;
	suspendSnap = FALSE;
	valid = FALSE;
	SetAFlag(A_OBJ_CREATING);

	//set up the pblock with the new controls
	Control *c;
	suspendRecalc= TRUE;
	//localeuler
	c = new EulerExposeControl(this,pb_localeuler);
	pblock_expose->SetController(pb_localeuler, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_localeulerx);
	pblock_expose->SetController(pb_localeulerx, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_localeulery);
	pblock_expose->SetController(pb_localeulery, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_localeulerz);
	pblock_expose->SetController(pb_localeulerz, 0, c, FALSE);
	//worldeuler
	c = new EulerExposeControl(this,pb_worldeuler);
	pblock_expose->SetController(pb_worldeuler, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_worldeulerx);
	pblock_expose->SetController(pb_worldeulerx, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_worldeulery);
	pblock_expose->SetController(pb_worldeulery, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_worldeulerz);
	pblock_expose->SetController(pb_worldeulerz, 0, c, FALSE);
	//localposition
	c = new Point3ExposeControl(this,pb_localposition);
	pblock_expose->SetController(pb_localposition, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_localpositionx);
	pblock_expose->SetController(pb_localpositionx, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_localpositiony);
	pblock_expose->SetController(pb_localpositiony, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_localpositionz);
	pblock_expose->SetController(pb_localpositionz, 0, c, FALSE);
	//worldposition
	c = new Point3ExposeControl(this,pb_worldposition);
	pblock_expose->SetController(pb_worldposition, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_worldpositionx);
	pblock_expose->SetController(pb_worldpositionx, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_worldpositiony);
	pblock_expose->SetController(pb_worldpositiony, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_worldpositionz);
	pblock_expose->SetController(pb_worldpositionz, 0, c, FALSE);
	//distance
	c = new FloatExposeControl(this,pb_distance);
	pblock_expose->SetController(pb_distance, 0, c, FALSE);
	//worldboundingbox
	c = new Point3ExposeControl(this,pb_worldboundingbox_size);
	pblock_expose->SetController(pb_worldboundingbox_size, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_worldboundingbox_width);
	pblock_expose->SetController(pb_worldboundingbox_width, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_worldboundingbox_height);
	pblock_expose->SetController(pb_worldboundingbox_height, 0, c, FALSE);
	c = new FloatExposeControl(this,pb_worldboundingbox_length);
	pblock_expose->SetController(pb_worldboundingbox_length, 0, c, FALSE);
	//angle
	c = new FloatExposeControl(this,pb_angle);
	pblock_expose->SetController(pb_angle,0,c,FALSE);
	suspendRecalc= FALSE;

	//custom edits
	localEulerXEdit = NULL;	localEulerYEdit = NULL;	localEulerZEdit = NULL;
	worldEulerXEdit = NULL;	worldEulerYEdit = NULL;	worldEulerZEdit = NULL;
	localPositionXEdit = NULL;	localPositionYEdit = NULL;	localPositionZEdit = NULL;
	worldPositionXEdit = NULL;	worldPositionYEdit = NULL;	worldPositionZEdit = NULL;
	boundingBoxLengthEdit = NULL; boundingBoxWidthEdit = NULL; boundingBoxHeightEdit = NULL;
	distanceEdit =NULL;
	angleEdit = NULL;
	//custom buttons
	localEulerXButton = NULL;	localEulerYButton = NULL;	localEulerZButton = NULL;
	worldEulerXButton = NULL;	worldEulerYButton = NULL;	worldEulerZButton = NULL;
	localPositionXButton = NULL;	localPositionYButton = NULL;	localPositionZButton = NULL;
	worldPositionXButton = NULL;	worldPositionYButton = NULL;	worldPositionZButton = NULL;
	boundingBoxLengthButton = NULL; boundingBoxWidthButton = NULL; boundingBoxHeightButton = NULL;
	distanceButton =NULL;
	angleButton = NULL;
	

	GetCOREInterface()->RegisterTimeChangeCallback(this);
}

ExposeTransform::~ExposeTransform()
{
	GetCOREInterface()->UnRegisterTimeChangeCallback(this);
	DeleteAllRefsFromMe();
}

IOResult ExposeTransform::Load(ILoad *iload)
{
	IOResult res = IO_OK;
	ExposeTransformPLCB* plcb = new ExposeTransformPLCB(this);
	iload->RegisterPostLoadCallback(plcb);
	return res;
}
IOResult ExposeTransform::Save(ISave *isave)
{
/*	if(GetReferenceNode()&&GetReferenceNode()->IsRootNode())
	{
		pblock_info->SetValue(pb_reference_node,0,(INode*)NULL);
	}
	*/
	return IO_OK;
}

void ExposeTransform::CreateController(int where)
{
	if(pblock_expose==NULL)
		return;
	BOOL wasSuspendRecalc = suspendRecalc;
	suspendRecalc= TRUE;
	Control *c;

	switch(where)
	{
	case pb_localeuler:
		//localeuler
		c = new EulerExposeControl(this,pb_localeuler);
		pblock_expose->SetController(pb_localeuler, 0, c, FALSE);
		break;
	case pb_localeulerx:
        c = new FloatExposeControl(this,pb_localeulerx);
		pblock_expose->SetController(pb_localeulerx, 0, c, FALSE);
		break;
	case pb_localeulery:
        c = new FloatExposeControl(this,pb_localeulery);
		pblock_expose->SetController(pb_localeulery, 0, c, FALSE);
		break;
	case pb_localeulerz:
        c = new FloatExposeControl(this,pb_localeulerz);
		pblock_expose->SetController(pb_localeulerz, 0, c, FALSE);
		break;
	case pb_worldeuler:
        //worldeuler
		c = new EulerExposeControl(this,pb_worldeuler);
		pblock_expose->SetController(pb_worldeuler, 0, c, FALSE);
		break;
	case pb_worldeulerx:
        c = new FloatExposeControl(this,pb_worldeulerx);
		pblock_expose->SetController(pb_worldeulerx, 0, c, FALSE);
		break;
	case pb_worldeulery:
		c = new FloatExposeControl(this,pb_worldeulery);
		pblock_expose->SetController(pb_worldeulery, 0, c, FALSE);
		break;
	case pb_worldeulerz:
		c = new FloatExposeControl(this,pb_worldeulerz);
		pblock_expose->SetController(pb_worldeulerz, 0, c, FALSE);
		break;
	case pb_localposition:
		//localposition
		c = new Point3ExposeControl(this,pb_localposition);
		pblock_expose->SetController(pb_localposition, 0, c, FALSE);
		break;
	case pb_localpositionx:
		c = new FloatExposeControl(this,pb_localpositionx);
		pblock_expose->SetController(pb_localpositionx, 0, c, FALSE);
		break;
	case pb_localpositiony:
		c = new FloatExposeControl(this,pb_localpositiony);
		pblock_expose->SetController(pb_localpositiony, 0, c, FALSE);
		break;
	case pb_localpositionz:
		c = new FloatExposeControl(this,pb_localpositionz);
		pblock_expose->SetController(pb_localpositionz, 0, c, FALSE);
		break;
	case pb_worldposition:
		//worldposition
		c = new Point3ExposeControl(this,pb_worldposition);
		pblock_expose->SetController(pb_worldposition, 0, c, FALSE);
		break;
	case pb_worldpositionx:
		c = new FloatExposeControl(this,pb_worldpositionx);
		pblock_expose->SetController(pb_worldpositionx, 0, c, FALSE);
		break;
	case pb_worldpositiony:
		c = new FloatExposeControl(this,pb_worldpositiony);
		pblock_expose->SetController(pb_worldpositiony, 0, c, FALSE);
		break;
	case pb_worldpositionz:
		c = new FloatExposeControl(this,pb_worldpositionz);
		pblock_expose->SetController(pb_worldpositionz, 0, c, FALSE);
		break;
	case pb_distance:
		//distance
		c = new FloatExposeControl(this,pb_distance);
		pblock_expose->SetController(pb_distance, 0, c, FALSE);
		break;
	case pb_worldboundingbox_size:
		//worldboundingbox
		c = new Point3ExposeControl(this,pb_worldboundingbox_size);
		pblock_expose->SetController(pb_worldboundingbox_size, 0, c, FALSE);
		break;
	case pb_worldboundingbox_width:
		c = new FloatExposeControl(this,pb_worldboundingbox_width);
		pblock_expose->SetController(pb_worldboundingbox_width, 0, c, FALSE);
		break;
	case pb_worldboundingbox_height:
		c = new FloatExposeControl(this,pb_worldboundingbox_height);
		pblock_expose->SetController(pb_worldboundingbox_height, 0, c, FALSE);
		break;
	case pb_worldboundingbox_length:
		c = new FloatExposeControl(this,pb_worldboundingbox_length);
		pblock_expose->SetController(pb_worldboundingbox_length, 0, c, FALSE);
		break;
	case pb_angle:
		//angle
		c = new FloatExposeControl(this,pb_angle);
		pblock_expose->SetController(pb_angle,0,c,FALSE);
		break;

	};
	suspendRecalc = wasSuspendRecalc;
}
void ExposeTransform::SetReferenceNodeNULL()
{
	if(pblock_info==NULL)
		return;
	pblock_info->SetValue(pb_reference_node,0,(INode*)NULL);
}

Point3 ExposeTransform::GetExposedPoint3Value(TimeValue t,int paramID,Control *) //control not used yet
{
	if(pblock_info==NULL||pblock_expose==NULL||GetExposeNode()==NULL)
		return Point3(0,0,0);


	BOOL wasSuspendRecalc = suspendRecalc;
	suspendRecalc = TRUE;
	BOOL useTime;
	pblock_info->GetValue(pb_use_time_offset,0,useTime,FOREVER);
	TimeValue offset=0;
	if(useTime==TRUE)
		pblock_info->GetValue(pb_time_offset,t,offset,FOREVER);
	offset/= GetTicksPerFrame();
	t+=offset;

	Point3 val(0,0,0);
	switch(paramID)
	{
	case pb_localposition:
		val = CalculateLocalPosition(t);
		break;
	case pb_worldposition:
		val = CalculateWorldPosition(t);
		break;;
	case pb_worldboundingbox_size:
		val = CalculateWorldBoundingBoxSize(t);
		break;
	}
	suspendRecalc = wasSuspendRecalc;
	return val;
}


Quat ExposeTransform::GetExposedEulerValue(TimeValue t,int paramID,Control *) //control not used yet
{
	if(pblock_info==NULL||pblock_expose==NULL||GetExposeNode()==NULL)
		return Quat();

	BOOL wasSuspendRecalc = suspendRecalc;
	suspendRecalc = TRUE;
	BOOL useTime;
	pblock_info->GetValue(pb_use_time_offset,0,useTime,FOREVER);
	TimeValue offset=0;
	if(useTime==TRUE)
		pblock_info->GetValue(pb_time_offset,t,offset,FOREVER);
	offset/= GetTicksPerFrame();
	t+=offset;

	Point3 val(0,0,0);
	switch(paramID)
	{
	case pb_localeuler:
		val = CalculateLocalEuler(t);
		break;
	case pb_worldeuler:
		val = CalculateWorldEuler(t);
		break;
	}
	Quat qval;
	EulerToQuat(val,qval);
	suspendRecalc = wasSuspendRecalc;
	return qval;
}
float ExposeTransform::GetExposedFloatValue(TimeValue t, int paramID,Control *c) //control not used yet
{
	if(pblock_info==NULL||pblock_expose==NULL||GetExposeNode()==NULL)
		return 0.0f;

	BOOL wasSuspendRecalc = suspendRecalc;
	suspendRecalc = TRUE;
	BOOL useTime;
	pblock_info->GetValue(pb_use_time_offset,0,useTime,FOREVER);
	TimeValue offset=0;
	if(useTime==TRUE)
		pblock_info->GetValue(pb_time_offset,t,offset,FOREVER);
	offset/= GetTicksPerFrame();

	t+=offset;

	Point3 val;
	float v;
	switch(paramID)
	{
	case pb_localeulerx:
		val = CalculateLocalEuler(t);
		suspendRecalc = wasSuspendRecalc;
		return val.x;
	case pb_localeulery:
		val = CalculateLocalEuler(t);
		suspendRecalc = wasSuspendRecalc;
		return val.y;
	case pb_localeulerz:
		val = CalculateLocalEuler(t);
		suspendRecalc = wasSuspendRecalc;
		return val.z;
	case pb_worldeulerx:
		val = CalculateWorldEuler(t);
		suspendRecalc = wasSuspendRecalc;
		return val.x;
	case pb_worldeulery:
		val = CalculateWorldEuler(t);
		suspendRecalc = wasSuspendRecalc;
		return val.y;
	case pb_worldeulerz:
		val = CalculateWorldEuler(t);
		suspendRecalc = wasSuspendRecalc;
		return val.z;

	case pb_localpositionx:
		val = CalculateLocalPosition(t);
		suspendRecalc = wasSuspendRecalc;
		return val.x;
	case pb_localpositiony:
		val = CalculateLocalPosition(t);
		suspendRecalc = wasSuspendRecalc;
		return val.y;
	case pb_localpositionz:
		val = CalculateLocalPosition(t);
		suspendRecalc = wasSuspendRecalc;
		return val.z;
	case pb_worldpositionx:
		val = CalculateWorldPosition(t);
		suspendRecalc = wasSuspendRecalc;
		return val.x;
	case pb_worldpositiony:
		val = CalculateWorldPosition(t);
		suspendRecalc = wasSuspendRecalc;
		return val.y;
	case pb_worldpositionz:
		val = CalculateWorldPosition(t);
		suspendRecalc = wasSuspendRecalc;
		return val.z;
	case pb_distance:
		v = CalculateDistance(t);
		suspendRecalc = wasSuspendRecalc;
		return v;
	case pb_angle:
		v = CalculateAngle(t);
		suspendRecalc = wasSuspendRecalc;
		return v;
	case pb_worldboundingbox_width:
		val = CalculateWorldBoundingBoxSize(t);
		suspendRecalc = wasSuspendRecalc;
		return val.x;
	case pb_worldboundingbox_length:
		val = CalculateWorldBoundingBoxSize(t);
		suspendRecalc = wasSuspendRecalc;
		return val.y;
	case pb_worldboundingbox_height:
		val = CalculateWorldBoundingBoxSize(t);
		suspendRecalc = wasSuspendRecalc;
		return val.z;
	}
	return 0.0;
}


void ExposeTransform::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
{	
	this->ip = ip;
	editOb   = this;
	ExposeTransformDesc.BeginEditParams(ip, this, flags, prev);	


}
		
void ExposeTransform::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{	
	editOb   = NULL;
	this->ip = NULL;
	ExposeTransformDesc.EndEditParams(ip, this, flags, next);
	ClearAFlag(A_OBJ_CREATING);

	
}

INode *ExposeTransform::GetExposeNode()
{
	if(pblock_info==NULL)
		return NULL;
	INode *val;
	pblock_info->GetValue(pb_expose_node, 0, val, FOREVER);
	return val;
}

void ExposeTransform::SetExposeNode(INode *node)
{
	if(pblock_info==NULL)
		return;
	pblock_info->SetValue(pb_expose_node,0,node);
}

INode *ExposeTransform::GetReferenceNode()
{
	if(pblock_info==NULL)
		return NULL;


	BOOL val;
	pblock_info->GetValue(pb_use_parent_as_reference, 0, val, FOREVER);

	if(val==TRUE)
	{
		INode *node= GetExposeNode();
		if(node)
			return node->GetParentNode();
		else
			return NULL;
	}
	else
	{
		INode *val=NULL;
		pblock_info->GetValue(pb_reference_node, 0, val, FOREVER);
		return val;
	}

}

void ExposeTransform::InitRefTarg()
{
	if(pblock_info==NULL)
		return;

   if (pblock_info->GetMap())
	{
		HWND hDlg = pblock_info->GetMap()->GetHWnd();
		if (!hDlg) return;
		//ISpinnerControl *spinner;
		BOOL val;
		pblock_info->GetValue(pb_use_parent_as_reference, 0, val, FOREVER);
		if(val)
		{	INode *node = GetExposeNode();
			pblock_info->GetMap()->Enable(pb_reference_node, FALSE);
			if(node&&node->GetParentNode())
			{
				if(node->GetParentNode()->IsRootNode()==FALSE)
					pblock_info->SetValue(pb_reference_node,0,node->GetParentNode());
			}
		}
		else
		{
			pblock_info->GetMap()->Enable(pb_reference_node, TRUE);
		}
	}
}
void ExposeTransform::InitTimeOffset()
{
	if(pblock_info==NULL)
		return;

	if(pblock_info->GetMap())
	{
		HWND hDlg = pblock_info->GetMap()->GetHWnd();
		if (!hDlg) return;
		//ISpinnerControl *spinner;
		BOOL val;
		pblock_info->GetValue(pb_use_time_offset, 0, val, FOREVER);
		if(val)
		{
			pblock_info->GetMap()->Enable(pb_time_offset, TRUE);
		}
		else
		{
			pblock_info->GetMap()->Enable(pb_time_offset, FALSE);
		}
	}
}

void ExposeTransform::InitEulers()
{
	if(pblock_info==NULL)
		return;

	if (pblock_info->GetMap())
	{
		HWND hDlg = pblock_info->GetMap()->GetHWnd();
		if (!hDlg) return;
	    int val,i;

		SendDlgItemMessage(hDlg, IDC_EULER_ORDER_X, CB_RESETCONTENT, 0, 0);
	    for (i=0; i<NUM_SUPPORTED_EULERS; i++) {
		    SendDlgItemMessage(hDlg,IDC_EULER_ORDER_X, CB_ADDSTRING, 0,
			    (LPARAM)GetString(eulerIDs[i]));
        }
		pblock_info->GetValue(pb_x_order, 0, val, FOREVER);
		--val; //to go from 1 to 0 based
		SendDlgItemMessage(hDlg, IDC_EULER_ORDER_X, CB_SETCURSEL,val, 0);

		SendDlgItemMessage(hDlg, IDC_EULER_ORDER_Y, CB_RESETCONTENT, 0, 0);
	    for (i=0; i<NUM_SUPPORTED_EULERS; i++) {
		    SendDlgItemMessage(hDlg,IDC_EULER_ORDER_Y, CB_ADDSTRING, 0,
			    (LPARAM)GetString(eulerIDs[i]));
        }
		pblock_info->GetValue(pb_y_order, 0, val, FOREVER);
		--val; //to go from 1 to 0 based
		SendDlgItemMessage(hDlg, IDC_EULER_ORDER_Y, CB_SETCURSEL,val, 0);

		SendDlgItemMessage(hDlg, IDC_EULER_ORDER_Z, CB_RESETCONTENT, 0, 0);
	    for (i=0; i<NUM_SUPPORTED_EULERS; i++) {
		    SendDlgItemMessage(hDlg,IDC_EULER_ORDER_Z, CB_ADDSTRING, 0,
			    (LPARAM)GetString(eulerIDs[i]));
        }
		pblock_info->GetValue(pb_z_order, 0, val, FOREVER);
		--val; //to go from 1 to 0 based
		SendDlgItemMessage(hDlg, IDC_EULER_ORDER_Z, CB_SETCURSEL,val, 0);



	}
}

void ExposeTransform::InitInfoUI()
{
	BOOL wasSuspendRecalc = suspendRecalc;
	suspendRecalc = TRUE;
	InitRefTarg();
	InitEulers();
	InitTimeOffset();
	suspendRecalc = wasSuspendRecalc;
}

void ExposeTransform::WMInfoCommand(int id, int notify, HWND hWnd,HWND hCtrl)
{
	if(pblock_info==NULL)
		return;

	switch (id)
	{
        case IDC_EULER_ORDER_X:
            if (notify==CBN_SELCHANGE)
			{
                int res = SendDlgItemMessage(hWnd, IDC_EULER_ORDER_X,
                                             CB_GETCURSEL, 0, 0);
                if (res!=CB_ERR)
				{
					++res;//to go from 0 to 1 based
					pblock_info->SetValue(pb_x_order,0,res);
				}
            }
            break; 
        case IDC_EULER_ORDER_Y:
            if (notify==CBN_SELCHANGE)
			{
                int res = SendDlgItemMessage(hWnd, IDC_EULER_ORDER_Y,
                                             CB_GETCURSEL, 0, 0);
                if (res!=CB_ERR)
				{
					++res;//to go from 0 to 1 based
					pblock_info->SetValue(pb_y_order,0,res);
				}
            }
            break;          
        case IDC_EULER_ORDER_Z:
            if (notify==CBN_SELCHANGE)
			{
                int res = SendDlgItemMessage(hWnd, IDC_EULER_ORDER_Z,
                                             CB_GETCURSEL, 0, 0);
                if (res!=CB_ERR)
				{
					++res; //to go from 0 to 1 based
					pblock_info->SetValue(pb_z_order,0,res);
				}
            }
            break;          
	}
}

void ExposeTransform::TimeChanged(TimeValue t)
{
	InitOutputUI(t);
}

void ExposeTransform::InitOutputUI(TimeValue t)
{
	BOOL wasSuspendRecalc = suspendRecalc;
	suspendRecalc = TRUE;
	InitOutput(t);
	suspendRecalc = wasSuspendRecalc;
}

void ExposeTransform::WMOutputCommand(int id, int notify, HWND hWnd,HWND hCtrl)
{
    switch (id)
	{

	case IDC_LE_X:
		SetUpClipBoard(TSTR("localEulerX"));
		break;
	case IDC_LE_Y:
		SetUpClipBoard(TSTR("localEulerY"));
		break;
	case IDC_LE_Z:
		SetUpClipBoard(TSTR("localEulerZ"));
		break;
	case IDC_WE_X:
		SetUpClipBoard(TSTR("worldEulerX"));
		break;
	case IDC_WE_Y:
		SetUpClipBoard(TSTR("worldEulerY"));
		break;
	case IDC_WE_Z:
		SetUpClipBoard(TSTR("worldEulerZ"));
		break;
	case IDC_LP_X:
		SetUpClipBoard(TSTR("localPositionX"));
		break;
	case IDC_LP_Y:
		SetUpClipBoard(TSTR("localPositionY"));
		break;
	case IDC_LP_Z:
		SetUpClipBoard(TSTR("localPositionZ"));
		break;
	case IDC_WP_X:
		SetUpClipBoard(TSTR("worldPositionX"));
		break;
	case IDC_WP_Y:
		SetUpClipBoard(TSTR("worldPositionY"));
		break;
	case IDC_WP_Z:
		SetUpClipBoard(TSTR("worldPositionZ"));
		break;
	case IDC_B_L:
		SetUpClipBoard(TSTR("worldBoundingBoxLength"));
		break;
	case IDC_B_W:
		SetUpClipBoard(TSTR("worldBoundingBoxWidth"));
		break;
	case IDC_B_H:
		SetUpClipBoard(TSTR("worldBoundingBoxHeight"));
		break;
	case IDC_D:
		SetUpClipBoard(TSTR("distance"));
		break;

	case IDC_A:
		SetUpClipBoard(TSTR("angle"));
		break;
	
	}

}


void Copy(char * buf,DWORD MAXSIZE)
{
     HANDLE handle;
     char * str;          //This has the data that is to be copied to the clipboard
     if(OpenClipboard(NULL))
     {
          EmptyClipboard();

          handle = GlobalAlloc(GMEM_MOVEABLE,MAXSIZE+1);
          if(handle)
          {
               str = (LPTSTR)GlobalLock(handle); 
               memcpy(str,buf,MAXSIZE);
               GlobalUnlock(handle); 
               SetClipboardData(CF_TEXT, handle); 
          }
          CloseClipboard();
     }
     delete buf;
}


void ExposeTransform::SetUpClipBoard(TSTR &localName)
{
	if(OpenClipboard(NULL))
	{
		EmptyClipboard();
		INode *myNode = GetMyNode();
		if(myNode)
		{
			//allocate handle mem
			HANDLE handle = GlobalAlloc(GMEM_MOVEABLE,256); //real big just in case
			if(handle)
			{
				TSTR t("$");
				TSTR final = TSTR("$") + TSTR(myNode->GetName()) + TSTR(".")+ localName;
				//set up the handle
				char *str = (LPTSTR) GlobalLock(handle);
				memcpy(str,final.data(),255);
				GlobalUnlock(handle);			
				SetClipboardData(CF_TEXT,handle);
			}
		}
		CloseClipboard();
	}

}

void ExposeTransform::InitOutput(TimeValue t)
{
	if(pblock_output==NULL)
		return;

	if(pblock_output->GetMap())
	{
		HWND hDlg = pblock_output->GetMap()->GetHWnd();
		if(!hDlg) return;
		//first we create the edits if they don't exists..
		if(localEulerXEdit==NULL) //assume one for all
		{
			localEulerXEdit = GetICustEdit(GetDlgItem(hDlg, IDC_LOCAL_EULER_X));
			localEulerXEdit->Enable();
			localEulerYEdit = GetICustEdit(GetDlgItem(hDlg, IDC_LOCAL_EULER_Y));
			localEulerYEdit->Enable();
			localEulerZEdit = GetICustEdit(GetDlgItem(hDlg, IDC_LOCAL_EULER_Z));
			localEulerZEdit->Enable();
			worldEulerXEdit = GetICustEdit(GetDlgItem(hDlg, IDC_WORLD_EULER_X));
			worldEulerXEdit->Enable();
			worldEulerYEdit = GetICustEdit(GetDlgItem(hDlg, IDC_WORLD_EULER_Y));
			worldEulerYEdit->Enable();
			worldEulerZEdit = GetICustEdit(GetDlgItem(hDlg, IDC_WORLD_EULER_Z));
			worldEulerZEdit->Enable();
			localPositionXEdit = GetICustEdit(GetDlgItem(hDlg, IDC_LOCAL_POSITION_X));
			localPositionXEdit->Enable();
			localPositionYEdit = GetICustEdit(GetDlgItem(hDlg, IDC_LOCAL_POSITION_Y));
			localPositionYEdit->Enable();
			localPositionZEdit = GetICustEdit(GetDlgItem(hDlg, IDC_LOCAL_POSITION_Z));
			localPositionZEdit->Enable();
			worldPositionXEdit = GetICustEdit(GetDlgItem(hDlg, IDC_WORLD_POSITION_X));
			worldPositionXEdit->Enable();
			worldPositionYEdit = GetICustEdit(GetDlgItem(hDlg, IDC_WORLD_POSITION_Y));
			worldPositionYEdit->Enable();
			worldPositionZEdit = GetICustEdit(GetDlgItem(hDlg, IDC_WORLD_POSITION_Z));
			worldPositionZEdit->Enable();
			boundingBoxLengthEdit = GetICustEdit(GetDlgItem(hDlg, IDC_BOUNDING_BOX_LENGTH));
			boundingBoxLengthEdit->Enable();
			boundingBoxWidthEdit = GetICustEdit(GetDlgItem(hDlg, IDC_BOUNDING_BOX_WIDTH));
			boundingBoxWidthEdit->Enable();
			boundingBoxHeightEdit = GetICustEdit(GetDlgItem(hDlg, IDC_BOUNDING_BOX_HEIGHT));
			boundingBoxHeightEdit->Enable();
			distanceEdit = GetICustEdit(GetDlgItem(hDlg, IDC_DISTANCE));
			distanceEdit->Enable();
			angleEdit = GetICustEdit(GetDlgItem(hDlg, IDC_ANGLE));
			angleEdit->Enable();
			//buttons
			TCHAR text[4] = {"M"};
			localEulerXButton = GetICustButton(GetDlgItem(hDlg, IDC_LE_X));
			localEulerXButton->SetType(CBT_PUSH);
			localEulerXButton->SetText(text);
			localEulerXButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			localEulerYButton = GetICustButton(GetDlgItem(hDlg, IDC_LE_Y));
			localEulerYButton->SetType(CBT_PUSH);
			localEulerYButton->SetText(text);
			localEulerYButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			localEulerZButton = GetICustButton(GetDlgItem(hDlg, IDC_LE_Z));
			localEulerZButton->SetType(CBT_PUSH);
			localEulerZButton->SetText(text);
			localEulerZButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			worldEulerXButton = GetICustButton(GetDlgItem(hDlg, IDC_WE_X));
			worldEulerXButton->SetType(CBT_PUSH);
			worldEulerXButton->SetText(text);
			worldEulerXButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			worldEulerYButton = GetICustButton(GetDlgItem(hDlg, IDC_WE_Y));
			worldEulerYButton->SetType(CBT_PUSH);
			worldEulerYButton->SetText(text);
			worldEulerYButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			worldEulerZButton = GetICustButton(GetDlgItem(hDlg, IDC_WE_Z));
			worldEulerZButton->SetType(CBT_PUSH);
			worldEulerZButton->SetText(text);
			worldEulerZButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			localPositionXButton = GetICustButton(GetDlgItem(hDlg, IDC_LP_X));
			localPositionXButton->SetType(CBT_PUSH);
			localPositionXButton->SetText(text);
			localPositionXButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			localPositionYButton = GetICustButton(GetDlgItem(hDlg, IDC_LP_Y));
			localPositionYButton->SetType(CBT_PUSH);
			localPositionYButton->SetText(text);
			localPositionYButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			localPositionZButton = GetICustButton(GetDlgItem(hDlg, IDC_LP_Z));
			localPositionZButton->SetType(CBT_PUSH);
			localPositionZButton->SetText(text);
			localPositionZButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			worldPositionXButton = GetICustButton(GetDlgItem(hDlg, IDC_WP_X));
			worldPositionXButton->SetType(CBT_PUSH);
			worldPositionXButton->SetText(text);
			worldPositionXButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			worldPositionYButton = GetICustButton(GetDlgItem(hDlg, IDC_WP_Y));
			worldPositionYButton->SetType(CBT_PUSH);
			worldPositionYButton->SetText(text);			
			worldPositionYButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			worldPositionZButton = GetICustButton(GetDlgItem(hDlg, IDC_WP_Z));
			worldPositionZButton->SetType(CBT_PUSH);
			worldPositionZButton->SetText(text);			
			worldPositionZButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			boundingBoxLengthButton = GetICustButton(GetDlgItem(hDlg, IDC_B_L));
			boundingBoxLengthButton->SetType(CBT_PUSH);
			boundingBoxLengthButton->SetText(text);		
			boundingBoxLengthButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			boundingBoxWidthButton = GetICustButton(GetDlgItem(hDlg, IDC_B_W));
			boundingBoxWidthButton->SetType(CBT_PUSH);
			boundingBoxWidthButton->SetText(text);					
			boundingBoxWidthButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			boundingBoxHeightButton = GetICustButton(GetDlgItem(hDlg, IDC_B_H));
			boundingBoxHeightButton->SetType(CBT_PUSH);
			boundingBoxHeightButton->SetText(text);		
			boundingBoxHeightButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			
			distanceButton = GetICustButton(GetDlgItem(hDlg, IDC_D));
			distanceButton->SetType(CBT_PUSH);
			distanceButton->SetText(text);	
			distanceButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
			angleButton = GetICustButton(GetDlgItem(hDlg, IDC_A));
			angleButton->SetType(CBT_PUSH);
			angleButton->SetText(text);		
			angleButton->SetTooltip(TRUE,(LPSTR)GetString(IDS_SCRIPTBUTTON));
		}

		BOOL val;
		pblock_output->GetValue(pb_display_exposed,0,val,FOREVER);
		if(val&&GetExposeNode())
		{
			SetExposeValues(t);
		}
		else
		{
			SetZeroExposeValues(); //disable the controls and zero the value
		}

	}
}

void ExposeTransform::SetExposeValues(TimeValue t)
{
	if(pblock_info==NULL)
		return;

	localEulerXEdit->Enable();
	localEulerYEdit->Enable();
	localEulerZEdit->Enable();
	worldEulerXEdit->Enable();
	worldEulerYEdit->Enable();
	worldEulerZEdit->Enable();
	localPositionXEdit->Enable();
	localPositionYEdit->Enable();
	localPositionZEdit->Enable();
	worldPositionXEdit->Enable();
	worldPositionYEdit->Enable();
	worldPositionZEdit->Enable();
	boundingBoxLengthEdit->Enable();
	boundingBoxHeightEdit->Enable();
	boundingBoxWidthEdit->Enable();
	distanceEdit->Enable();
	angleEdit->Enable();

	BOOL wasSuspendRecalc = suspendRecalc;
	suspendRecalc = TRUE;

	BOOL useTime = FALSE;
	pblock_info->GetValue(pb_use_time_offset,0,useTime,FOREVER);
	TimeValue offset=0;
	if(useTime==TRUE)
		pblock_info->GetValue(pb_time_offset,t,offset,FOREVER);
	offset/= GetTicksPerFrame();
	t+=offset;

	Point3 p; float f;

	p =  CalculateLocalEuler(t);
	localEulerXEdit->SetText(p.x*RadToDegDbl);
	localEulerYEdit->SetText(p.y*RadToDegDbl);
	localEulerZEdit->SetText(p.z*RadToDegDbl);

	p =  CalculateWorldEuler(t);
	worldEulerXEdit->SetText(p.x*RadToDegDbl);
	worldEulerYEdit->SetText(p.y*RadToDegDbl);
	worldEulerZEdit->SetText(p.z*RadToDegDbl);

	p =  CalculateLocalPosition(t);
	localPositionXEdit->SetText(p.x);
	localPositionYEdit->SetText(p.y);
	localPositionZEdit->SetText(p.z);

	p =  CalculateWorldPosition(t);
	worldPositionXEdit->SetText(p.x);
	worldPositionYEdit->SetText(p.y);
	worldPositionZEdit->SetText(p.z);

	p =  CalculateWorldBoundingBoxSize(t);
	boundingBoxWidthEdit->SetText(p.x);
	boundingBoxLengthEdit->SetText(p.y);
	boundingBoxHeightEdit->SetText(p.z);

	f= CalculateDistance(t);
	distanceEdit->SetText(f);

	f = CalculateAngle(t);
	angleEdit->SetText(f*RadToDegDbl);

	suspendRecalc = wasSuspendRecalc;

}

void ExposeTransform::SetZeroExposeValues()
{
	BOOL wasSuspendRecalc = suspendRecalc;
	suspendRecalc = TRUE;

	TCHAR val[2] = {""};
	localEulerXEdit->Enable(FALSE);
	localEulerXEdit->SetText(val);
	localEulerYEdit->Enable(FALSE);
	localEulerYEdit->SetText(val);
	localEulerZEdit->Enable(FALSE);
	localEulerZEdit->SetText(val);

	worldEulerXEdit->Enable(FALSE);
	worldEulerXEdit->SetText(val);
	worldEulerYEdit->Enable(FALSE);
	worldEulerYEdit->SetText(val);
	worldEulerZEdit->Enable(FALSE);
	worldEulerZEdit->SetText(val);

	localPositionXEdit->Enable(FALSE);
	localPositionXEdit->SetText(val);
	localPositionYEdit->Enable(FALSE);
	localPositionYEdit->SetText(val);
	localPositionZEdit->Enable(FALSE);
	localPositionZEdit->SetText(val);

	worldPositionXEdit->Enable(FALSE);
	worldPositionXEdit->SetText(val);
	worldPositionYEdit->Enable(FALSE);
	worldPositionYEdit->SetText(val);
	worldPositionZEdit->Enable(FALSE);
	worldPositionZEdit->SetText(val);

	boundingBoxWidthEdit->Enable(FALSE);
	boundingBoxWidthEdit->SetText(val);
	boundingBoxLengthEdit->Enable(FALSE);
	boundingBoxLengthEdit->SetText(val);
	boundingBoxHeightEdit->Enable(FALSE);
	boundingBoxHeightEdit->SetText(val);

	distanceEdit->Enable(FALSE);
	distanceEdit->SetText(val);

	angleEdit->Enable(FALSE);
	angleEdit->SetText(val);	

	suspendRecalc = wasSuspendRecalc;

}
void ExposeTransform::DeleteCustomEdits()
{
	if(localEulerXEdit){ ReleaseICustEdit(localEulerXEdit);	localEulerXEdit = NULL;	}
	if(localEulerYEdit){ ReleaseICustEdit(localEulerYEdit);	localEulerYEdit = NULL;	}
	if(localEulerZEdit){ ReleaseICustEdit(localEulerZEdit);	localEulerZEdit = NULL;	}
	if(worldEulerXEdit){ ReleaseICustEdit(worldEulerXEdit);	worldEulerXEdit = NULL;	}
	if(worldEulerYEdit){ ReleaseICustEdit(worldEulerYEdit);	worldEulerYEdit = NULL;	}
	if(worldEulerZEdit){ ReleaseICustEdit(worldEulerZEdit);	worldEulerZEdit = NULL;	}
	if(localPositionXEdit){ ReleaseICustEdit(localPositionXEdit);	localPositionXEdit = NULL;	}
	if(localPositionYEdit){ ReleaseICustEdit(localPositionYEdit);	localPositionYEdit = NULL;	}
	if(localPositionZEdit){ ReleaseICustEdit(localPositionZEdit);	localPositionZEdit = NULL;	}
	if(worldPositionXEdit){ ReleaseICustEdit(worldPositionXEdit);	worldPositionXEdit = NULL;	}
	if(worldPositionYEdit){ ReleaseICustEdit(worldPositionYEdit);	worldPositionYEdit = NULL;	}
	if(worldPositionZEdit){ ReleaseICustEdit(worldPositionZEdit);	worldPositionZEdit = NULL;	}
	if(boundingBoxLengthEdit){ ReleaseICustEdit(boundingBoxLengthEdit);	boundingBoxLengthEdit = NULL;	}
	if(boundingBoxWidthEdit){ ReleaseICustEdit(boundingBoxWidthEdit);	boundingBoxWidthEdit = NULL;	}
	if(boundingBoxHeightEdit){ ReleaseICustEdit(boundingBoxHeightEdit);	boundingBoxHeightEdit = NULL;	}
	if(distanceEdit){ ReleaseICustEdit(distanceEdit); distanceEdit = NULL;}
	if(angleEdit){ ReleaseICustEdit(angleEdit); angleEdit = NULL;}
}

void ExposeTransform::DeleteCustomButtons()
{
	if(localEulerXButton){ ReleaseICustButton(localEulerXButton);	localEulerXButton = NULL;	}
	if(localEulerYButton){ ReleaseICustButton(localEulerYButton);	localEulerYButton = NULL;	}
	if(localEulerZButton){ ReleaseICustButton(localEulerZButton);	localEulerZButton = NULL;	}
	if(worldEulerXButton){ ReleaseICustButton(worldEulerXButton);	worldEulerXButton = NULL;	}
	if(worldEulerYButton){ ReleaseICustButton(worldEulerYButton);	worldEulerYButton = NULL;	}
	if(worldEulerZButton){ ReleaseICustButton(worldEulerZButton);	worldEulerZButton = NULL;	}
	if(localPositionXButton){ ReleaseICustButton(localPositionXButton);	localPositionXButton = NULL;	}
	if(localPositionYButton){ ReleaseICustButton(localPositionYButton);	localPositionYButton = NULL;	}
	if(localPositionZButton){ ReleaseICustButton(localPositionZButton);	localPositionZButton = NULL;	}
	if(worldPositionXButton){ ReleaseICustButton(worldPositionXButton);	worldPositionXButton = NULL;	}
	if(worldPositionYButton){ ReleaseICustButton(worldPositionYButton);	worldPositionYButton = NULL;	}
	if(worldPositionZButton){ ReleaseICustButton(worldPositionZButton);	worldPositionZButton = NULL;	}
	if(boundingBoxLengthButton){ ReleaseICustButton(boundingBoxLengthButton);	boundingBoxLengthButton = NULL;	}
	if(boundingBoxWidthButton){ ReleaseICustButton(boundingBoxWidthButton);	boundingBoxWidthButton = NULL;	}
	if(boundingBoxHeightButton){ ReleaseICustButton(boundingBoxHeightButton);	boundingBoxHeightButton = NULL;	}
	if(distanceButton){ ReleaseICustButton(distanceButton); distanceButton = NULL;}
	if(angleButton){ ReleaseICustButton(angleButton); angleButton = NULL;}
}


void ExposeTransform::RecalcData()
{
	if(pblock_expose==NULL)
		return;
	INode *node = this->GetMyNode();
	if(node==NULL)
		return;

	valid = TRUE;
	INodeValidity *nodeValidity = (INodeValidity *) node->GetInterface(NODEVALIDITY_INTERFACE);
	
	if(suspendRecalc==FALSE&&((nodeValidity->GetTMValid()==NEVER)==FALSE))
	{
		suspendRecalc = TRUE; //don't get into any loops here
	//	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

		for(int i = pb_expose_min;i<=pb_expose_max;++i)
		{
			Control *cTest = pblock_expose->GetController(i,0);
			if(cTest!=NULL)
			{
                ((BaseExposeControl *)cTest)->ivalid.SetEmpty();
                cTest->NotifyDependents(FOREVER,PART_TM,REFMSG_CHANGE);
			}
		}
		
		suspendRecalc = FALSE;
	}

}



//helper function with these order functions
static float GetEulerValue(float val[3],int which, int order)
{
	switch(order)
	{
	case EULERTYPE_XYZ:
		if(which==0)
			return val[0];
		else if(which==1)
			return val[1];
		else if(which==2)
			return val[2];
		break;
	case EULERTYPE_XZY:
		if(which==0)
			return val[0];
		else if(which==1)
			return val[2];
		else if(which==2)
			return val[1];
		break;
	case EULERTYPE_YZX:
		if(which==0)
			return val[2];
		else if(which==1)
			return val[0];
		else if(which==2)
			return val[1];
		break;
	case EULERTYPE_YXZ:
		if(which==0)
			return val[1];
		else if(which==1)
			return val[0];
		else if(which==2)
			return val[2];
		break;
	case EULERTYPE_ZXY:
		if(which==0)
			return val[1];
		else if(which==1)
			return val[2];
		else if(which==2)
			return val[0];
		break;
	case EULERTYPE_ZYX:
		if(which==0)
			return val[2];
		else if(which==1)
			return val[1];
		else if(which==2)
			return val[0];
		break;
	}
	return 0.0f;
}

static void CalculateEulerFromOrders(Point3 &val,Quat &q,int xorder,int yorder,int zorder)
{
	float valx[3],valy[3],valz[3];//each of the values for the three ..note only calculate them if different as opt

	QuatToEuler(q, valx, xorder);
	val[0] = GetEulerValue(valx,0,xorder);
	if(yorder==xorder)
		val[1] = GetEulerValue(valx,1,xorder);
	else
	{
		QuatToEuler(q,valy,yorder);
		val[1] = GetEulerValue(valy,1,yorder);
	}
	//now handle z
	if(zorder==xorder)
		val[2] = GetEulerValue(valx,2,xorder);
	else if(zorder==yorder)
		val[2] = GetEulerValue(valy,2,yorder);
	else
	{
		QuatToEuler(q,valz,zorder);
		val[2] = GetEulerValue(valz,2,zorder);
	}
}

Point3 ExposeTransform::CalculateLocalEuler(TimeValue t)
{

	Point3 val(0,0,0);
	if(GetExposeNode())
	{
		BOOL noScale;
		pblock_info->GetValue(pb_stripnuscale,t,noScale,FOREVER);

		Matrix3 tm = GetExposeNode()->GetNodeTM(t);
		if(noScale)
			tm.NoScale();
		Matrix3 ptm(1);
		if(GetReferenceNode())
		{
			ptm =GetReferenceNode()->GetNodeTM(t);
			if(noScale)
				ptm.NoScale();
			tm = tm *Inverse(ptm);
		}
		Quat q(tm);
		
		int xorder,yorder,zorder;
		//get the 3 orders
		pblock_info->GetValue(pb_x_order,t,xorder,FOREVER);
		pblock_info->GetValue(pb_y_order,t,yorder,FOREVER);
		pblock_info->GetValue(pb_z_order,t,zorder,FOREVER);
		--xorder;--yorder;--zorder;
		CalculateEulerFromOrders(val,q,xorder,yorder,zorder);
	}
	return val;
}

Point3 ExposeTransform::CalculateWorldEuler(TimeValue t)
{

	Point3 val(0,0,0);
	if(GetExposeNode())
	{
		Matrix3 tm = GetExposeNode()->GetNodeTM(t);
		BOOL noScale;
		pblock_info->GetValue(pb_stripnuscale,t,noScale,FOREVER);
		if(noScale)
			tm.NoScale();

		Quat q(tm);

		int xorder,yorder,zorder;
		//get the 3 orders
		pblock_info->GetValue(pb_x_order,t,xorder,FOREVER);
		pblock_info->GetValue(pb_y_order,t,yorder,FOREVER);
		pblock_info->GetValue(pb_z_order,t,zorder,FOREVER);
		--xorder;--yorder;--zorder;
		CalculateEulerFromOrders(val,q,xorder,yorder,zorder);

	}
	return val;

}
Point3 ExposeTransform::CalculateLocalPosition(TimeValue t)
{
	
	Point3 val(0,0,0);
	if(GetExposeNode())
	{
		Matrix3 tm = GetExposeNode()->GetNodeTM(t);
		Matrix3 ptm(1);
		if(GetReferenceNode())
		{
			ptm =GetReferenceNode()->GetNodeTM(t);
		}
		tm.NoScale();
		ptm.NoScale();
		tm = tm *Inverse(ptm);
		val = tm.GetTrans();
	}
	return val;

}
Point3 ExposeTransform::CalculateWorldPosition(TimeValue t)
{
	Point3 val(0,0,0);
	if(GetExposeNode())
	{
		Matrix3 tm = GetExposeNode()->GetNodeTM(t);
		val = tm.GetTrans();
	}
	return val;

}
Point3 ExposeTransform::CalculateWorldBoundingBoxSize(TimeValue t)
{
	Point3 val(0,0,0);
	if(GetExposeNode())
	{
		Object *obj = GetExposeNode()->EvalWorldState(t).obj;
		Box3 boundingBox;
		obj->GetDeformBBox(t, boundingBox);
		val = boundingBox.Max()- boundingBox.Min();
	}
	return val;
}

float ExposeTransform::CalculateDistance(TimeValue t)
{
	Point3 val = CalculateLocalPosition(t);
	return sqrtf(val.x*val.x + val.y*val.y + val.z*val.z);
}


class EnumProc : public DependentEnumProc 
{
  public :
	INodeTab Nodes;              
	virtual int proc(ReferenceMaker *rmaker); 
};
int EnumProc::proc(ReferenceMaker *rmaker) 
{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
	{
		Nodes.Append(1, (INode **)&rmaker);  
		return DEP_ENUM_SKIP;
	}

	return DEP_ENUM_CONTINUE;
}

class ExposeTransformGeomPipelineEnumProc : public GeomPipelineEnumProc
{
public:  
	ExposeTransformGeomPipelineEnumProc(Object& obj) : mFound(false), mObject(obj) {}
	PipeEnumResult proc(ReferenceTarget *obj, IDerivedObject *derObj, int index);
	bool mFound;
	Object& mObject;

protected:
	ExposeTransformGeomPipelineEnumProc(); // disallowed
	ExposeTransformGeomPipelineEnumProc(ExposeTransformGeomPipelineEnumProc& rhs); // disallowed
	ExposeTransformGeomPipelineEnumProc& operator=(const ExposeTransformGeomPipelineEnumProc& rhs); // disallowed
};

PipeEnumResult ExposeTransformGeomPipelineEnumProc::proc(
	ReferenceTarget *obj, 
	IDerivedObject *derObj, 
	int index)
{
	if (obj != NULL && obj == &mObject) 
	{
		mFound = true;
		return PIPE_ENUM_STOP;
	}
	return PIPE_ENUM_CONTINUE;
}

INode * ExposeTransform::GetMyNode()
{
	EnumProc dep;              
	DoEnumDependents(&dep);
	for (int i = 0; i < dep.Nodes.Count(); i++) 
	{
		INode *node = dep.Nodes[i];
		Object* obj = node->GetObjectRef();
		ExposeTransformGeomPipelineEnumProc pipeEnumProc(*this);
		EnumGeomPipeline(&pipeEnumProc, obj);
		if (pipeEnumProc.mFound)
		{
			return node;
		}
	}
	return NULL;
}

float ExposeTransform::CalculateAngle(TimeValue t)
{
	INode *myNode = GetMyNode();
	if(myNode&&GetExposeNode()&&GetReferenceNode())
	{
		Point3 myPos = myNode->GetNodeTM(t).GetTrans();
		Point3 firstPos = GetExposeNode()->GetNodeTM(t).GetTrans();
		Point3 secondPos = GetReferenceNode()->GetNodeTM(t).GetTrans();
		float lastAngle= 0.0f;
		Point3 vec1 = firstPos - myPos;
		Point3 vec2 = secondPos -myPos;
		float len1 = Length(vec1);
		float len2 = Length(vec2);
		if(len1 > 0.00001f && len2 > 0.00001f) {
			double cosAng = (double)DotProd(vec1, vec2) / (double)(len1 * len2);
			if(fabs(cosAng) <= 0.999999)	// beyond float accuracy!
				lastAngle = acos(cosAng);// * RadToDegDbl;
			else
				lastAngle = 3.141592f;
		}
		return lastAngle;
	}
	return 0.0f;
}

class PointHelpObjCreateCallBack: public CreateMouseCallBack {
	ExposeTransform *ob;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(ExposeTransform *obj) { ob = obj; }
	};

int PointHelpObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {	

	#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _3D_CREATE
			vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		#else
			vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
		#endif
	}
	#endif

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0: {

				// Find the node and plug in the wire color
				ULONG handle;
				ob->NotifyDependents(FOREVER, (PartID)&handle, REFMSG_GET_NODE_HANDLE);
				INode *node;
				node = GetCOREInterface()->GetINodeByHandle(handle);
				if (node) {
					Point3 color = GetUIColor(COLOR_POINT_OBJ);
					node->SetWireColor(RGB(color.x*255.0f, color.y*255.0f, color.z*255.0f));
					}

				ob->suspendSnap = TRUE;
				#ifdef _3D_CREATE	
					mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_3D));
				#else	
					mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
				#endif				
				break;
				}

			case 1:
				#ifdef _3D_CREATE	
					mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_3D));
				#else	
					mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
				#endif
				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					return 0;
					}
				break;			
			}
	} else 
	if (msg == MOUSE_ABORT) {		
		return CREATE_ABORT;
		}
	return 1;
	}

static PointHelpObjCreateCallBack pointHelpCreateCB;

CreateMouseCallBack* ExposeTransform::GetCreateMouseCallBack() {
	pointHelpCreateCB.SetObj(this);
	return(&pointHelpCreateCB);
	}

void ExposeTransform::SetExtendedDisplay(int flags)
	{
	extDispFlags = flags;
	}


void ExposeTransform::GetLocalBoundBox(
		TimeValue t, INode* inode, ViewExp* vpt, Box3& box ) 
	{
	Matrix3 tm = inode->GetObjectTM(t);	
	
	float size;
	int screenSize;
	pblock_display->GetValue(pb_pointobj_size, t, size, FOREVER);
	pblock_display->GetValue(pb_pointobj_screensize, t, screenSize, FOREVER);

	float zoom = 1.0f;
	if (screenSize) {
		zoom = vpt->GetScreenScaleFactor(tm.GetTrans())*ZFACT;
		}
	if (zoom==0.0f) zoom = 1.0f;

	size *= zoom;
	box =  Box3(Point3(0,0,0), Point3(0,0,0));
	box += Point3(size*0.5f,  0.0f, 0.0f);
	box += Point3( 0.0f, size*0.5f, 0.0f);
	box += Point3( 0.0f, 0.0f, size*0.5f);
	box += Point3(-size*0.5f,   0.0f,  0.0f);
	box += Point3(  0.0f, -size*0.5f,  0.0f);
	box += Point3(  0.0f,  0.0f, -size*0.5f);

	//JH 6/18/03
	//This looks odd but I'm being conservative an only excluding it for
	//the case I care about which is when computing group boxes
	//	box.EnlargeBy(10.0f/zoom);
	if(!(extDispFlags & EXT_DISP_GROUP_EXT)) 
		box.EnlargeBy(10.0f/zoom);

	}

void ExposeTransform::GetWorldBoundBox(
		TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
	{
	Matrix3 tm;
	tm = inode->GetObjectTM(t);
	Box3 lbox;

	GetLocalBoundBox(t, inode, vpt, lbox);
	box = Box3(tm.GetTrans(), tm.GetTrans());
	for (int i=0; i<8; i++) {
		box += lbox * tm;
		}
	
	}


void ExposeTransform::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
	{
	if(suspendSnap)
		return;

	Matrix3 tm = inode->GetObjectTM(t);	
	GraphicsWindow *gw = vpt->getGW();	
	gw->setTransform(tm);

	Matrix3 invPlane = Inverse(snap->plane);

	// Make sure the vertex priority is active and at least as important as the best snap so far
	if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) {
		Point2 fp = Point2((float)p->x, (float)p->y);
		Point2 screen2;
		IPoint3 pt3;

		Point3 thePoint(0,0,0);
		// If constrained to the plane, make sure this point is in it!
		if(snap->snapType == SNAP_2D || snap->flags & SNAP_IN_PLANE) {
			Point3 test = thePoint * tm * invPlane;
			if(fabs(test.z) > 0.0001)	// Is it in the plane (within reason)?
				return;
			}
		gw->wTransPoint(&thePoint,&pt3);
		screen2.x = (float)pt3.x;
		screen2.y = (float)pt3.y;

		// Are we within the snap radius?
		int len = (int)Length(screen2 - fp);
		if(len <= snap->strength) {
			// Is this priority better than the best so far?
			if(snap->vertPriority < snap->priority) {
				snap->priority = snap->vertPriority;
				snap->bestWorld = thePoint * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
				}
			else
			if(len < snap->bestDist) {
				snap->priority = snap->vertPriority;
				snap->bestWorld = thePoint * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
				}
			}
		}
	}




int ExposeTransform::DrawAndHit(TimeValue t, INode *inode, ViewExp *vpt)
	{
	float size;
	int centerMarker, axisTripod, cross, box, screenSize, drawOnTop;

	Color color(inode->GetWireColor());	

	Interval ivalid = FOREVER;
	pblock_display->GetValue(pb_pointobj_size, t,         size, ivalid);
	pblock_display->GetValue(pb_pointobj_centermarker, t, centerMarker, ivalid);
	pblock_display->GetValue(pb_pointobj_axistripod, t,   axisTripod, ivalid);
	pblock_display->GetValue(pb_pointobj_cross, t,        cross, ivalid);
	pblock_display->GetValue(pb_pointobj_box, t,          box, ivalid);
	pblock_display->GetValue(pb_pointobj_screensize, t,   screenSize, ivalid);
	pblock_display->GetValue(pb_pointobj_drawontop, t,    drawOnTop, ivalid);
	
	Matrix3 tm(1);
	Point3 pt(0,0,0);
	Point3 pts[5];

	vpt->getGW()->setTransform(tm);	
	tm = inode->GetObjectTM(t);

	int limits = vpt->getGW()->getRndLimits();
	if (drawOnTop) vpt->getGW()->setRndLimits(limits & ~GW_Z_BUFFER);

	if (inode->Selected()) {
		vpt->getGW()->setColor( TEXT_COLOR, GetUIColor(COLOR_SELECTION) );
		vpt->getGW()->setColor( LINE_COLOR, GetUIColor(COLOR_SELECTION) );
	} else if (!inode->IsFrozen() && !inode->Dependent()) {
		//vpt->getGW()->setColor( TEXT_COLOR, GetUIColor(COLOR_POINT_AXES) );
		//vpt->getGW()->setColor( LINE_COLOR, GetUIColor(COLOR_POINT_AXES) );		
		vpt->getGW()->setColor( TEXT_COLOR, color);
		vpt->getGW()->setColor( LINE_COLOR, color);
		}	

	if (axisTripod) {
		DrawAxis(vpt, tm, size, screenSize);
		}
	
	size *= 0.5f;

	float zoom = vpt->GetScreenScaleFactor(tm.GetTrans())*ZFACT;
	if (screenSize) {
		tm.Scale(Point3(zoom,zoom,zoom));
		}

	vpt->getGW()->setTransform(tm);

	if (!inode->IsFrozen() && !inode->Dependent() && !inode->Selected()) {
		//vpt->getGW()->setColor(LINE_COLOR, GetUIColor(COLOR_POINT_OBJ));
		vpt->getGW()->setColor( LINE_COLOR, color);
		}

	if (centerMarker) {		
		vpt->getGW()->marker(&pt,X_MRKR);
		}

	if (cross) {
		// X
		pts[0] = Point3(-size, 0.0f, 0.0f); pts[1] = Point3(size, 0.0f, 0.0f);
		vpt->getGW()->polyline(2, pts, NULL, NULL, FALSE, NULL);

		// Y
		pts[0] = Point3(0.0f, -size, 0.0f); pts[1] = Point3(0.0f, size, 0.0f);
		vpt->getGW()->polyline(2, pts, NULL, NULL, FALSE, NULL);

		// Z
		pts[0] = Point3(0.0f, 0.0f, -size); pts[1] = Point3(0.0f, 0.0f, size);
		vpt->getGW()->polyline(2, pts, NULL, NULL, FALSE, NULL);
		}

	if (box) {

		// Make the box half the size
		size = size * 0.5f;

		// Bottom
		pts[0] = Point3(-size, -size, -size); 
		pts[1] = Point3(-size,  size, -size);
		pts[2] = Point3( size,  size, -size);
		pts[3] = Point3( size, -size, -size);
		vpt->getGW()->polyline(4, pts, NULL, NULL, TRUE, NULL);

		// Top
		pts[0] = Point3(-size, -size,  size); 
		pts[1] = Point3(-size,  size,  size);
		pts[2] = Point3( size,  size,  size);
		pts[3] = Point3( size, -size,  size);
		vpt->getGW()->polyline(4, pts, NULL, NULL, TRUE, NULL);

		// Sides
		pts[0] = Point3(-size, -size, -size); 
		pts[1] = Point3(-size, -size,  size);
		vpt->getGW()->polyline(2, pts, NULL, NULL, FALSE, NULL);

		pts[0] = Point3(-size,  size, -size); 
		pts[1] = Point3(-size,  size,  size);
		vpt->getGW()->polyline(2, pts, NULL, NULL, FALSE, NULL);

		pts[0] = Point3( size,  size, -size); 
		pts[1] = Point3( size,  size,  size);
		vpt->getGW()->polyline(2, pts, NULL, NULL, FALSE, NULL);

		pts[0] = Point3( size, -size, -size); 
		pts[1] = Point3( size, -size,  size);
		vpt->getGW()->polyline(2, pts, NULL, NULL, FALSE, NULL);
		}

	vpt->getGW()->setRndLimits(limits);

	return 1;
	}

int ExposeTransform::HitTest(
		TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) 
	{
	Matrix3 tm(1);	
	HitRegion hitRegion;
	DWORD	savedLimits;
	Point3 pt(0,0,0);

	vpt->getGW()->setTransform(tm);
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = gw->getMaterial();

   	tm = inode->GetObjectTM(t);		
	MakeHitRegion(hitRegion, type, crossing, 4, p);

	gw->setRndLimits(((savedLimits = gw->getRndLimits())|GW_PICK)&~GW_ILLUM);
	gw->setHitRegion(&hitRegion);
	gw->clearHitCode();

	DrawAndHit(t, inode, vpt);



	gw->setRndLimits(savedLimits);
	
	// CAL-08/27/03: This doesn't make sense. It shouldn't do this. (Defect #468271)
	// This will always select this helper when there's an intersection on the bounding box and the selection window.
	// TODO: There's still a problem with window selection. We need to check if it hits all components in DrawAndHit.
	/*
	if((hitRegion.type != POINT_RGN) && !hitRegion.crossing)
		return TRUE;
	*/

	return gw->checkHitCode();
	}



int ExposeTransform::Display(
		TimeValue t, INode* inode, ViewExp *vpt, int flags) 
	{
	DrawAndHit(t, inode, vpt);


	return(0);
	}



//
// Reference Managment:
//

// This is only called if the object MAKES references to other things.
RefResult ExposeTransform::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID, RefMessage message ) 
    {
	/*	char str[64];
		sprintf(str,"%x = message, %d = target %x = partID\n", message,hTarget,partID);
		OutputDebugString(str);
*/
	
	switch (message) {
		case REFMSG_CHANGE:
			{
			if (editOb==this) 
			{
				valid = FALSE;
				InvalidateUI();
				if ((hTarget== pblock_info&&partID==PART_TM)||hTarget==pblock_expose)
					RecalcData();
			}
			else if ((hTarget== pblock_info&&partID==PART_TM)||hTarget==pblock_expose)
			{
				RecalcData();
				InitOutputUI(GetCOREInterface()->GetTime());
			}
			break;
			}
		}
	return(REF_SUCCEED);
	}

void ExposeTransform::InvalidateUI()
	{
	exposetransform_display_blk.InvalidateUI(pblock_display->LastNotifyParamID());
	exposetransform_info_blk.InvalidateUI(pblock_info->LastNotifyParamID());
	exposetransform_output_blk.InvalidateUI(pblock_output->LastNotifyParamID());
	}

Interval ExposeTransform::ObjectValidity(TimeValue t)
	{
	float size;
	int centerMarker, axisTripod, cross, box, screenSize, drawOnTop;

	Interval ivalid = FOREVER;
	pblock_display->GetValue(pb_pointobj_size, t,         size, ivalid);
	pblock_display->GetValue(pb_pointobj_centermarker, t, centerMarker, ivalid);
	pblock_display->GetValue(pb_pointobj_axistripod, t,   axisTripod, ivalid);
	pblock_display->GetValue(pb_pointobj_cross, t,        cross, ivalid);
	pblock_display->GetValue(pb_pointobj_box, t,          box, ivalid);
	pblock_display->GetValue(pb_pointobj_screensize, t,   screenSize, ivalid);
	pblock_display->GetValue(pb_pointobj_drawontop, t,    drawOnTop, ivalid);

	return ivalid;
	}

ObjectState ExposeTransform::Eval(TimeValue t)
	{
	return ObjectState(this);
	}

RefTargetHandle ExposeTransform::Clone(RemapDir& remap) 
	{
	ExposeTransform* newob = new ExposeTransform();	
	newob->ReplaceReference(PBLOCK_EXPOSE, remap.CloneRef(pblock_expose));
	newob->ReplaceReference(PBLOCK_INFO, remap.CloneRef(pblock_info));
	newob->ReplaceReference(PBLOCK_DISPLAY, remap.CloneRef(pblock_display));
	newob->ReplaceReference(PBLOCK_OUTPUT, remap.CloneRef(pblock_output));
	BaseClone(this, newob, remap);
	for(int i = pb_expose_min;i<= pb_expose_max;++i)
	{
		BaseExposeControl * control = (BaseExposeControl*) newob->pblock_expose->GetController(i,0);
		if(control)
		{
			control->exposeTransform = newob;
			control->paramID = i;
		}
		else //from an older version ..we need to create the paramter.
		{
			newob->CreateController(i);
		}
	}
	return(newob);
	}




/*--------------------------------------------------------------------*/
// 
// Stole this from scene.cpp
// Probably couldn't hurt to make an API...
//
//


void Text( ViewExp *vpt, TCHAR *str, Point3 &pt )
	{	
	vpt->getGW()->text( &pt, str );	
	}

static void DrawAnAxis( ViewExp *vpt, Point3 axis )
	{
	Point3 v1, v2, v[3];	
	v1 = axis * (float)0.9;
	if ( axis.x != 0.0 || axis.y != 0.0 ) {
		v2 = Point3( axis.y, -axis.x, axis.z ) * (float)0.1;
	} else {
		v2 = Point3( axis.x, axis.z, -axis.y ) * (float)0.1;
		}
	
	v[0] = Point3(0.0,0.0,0.0);
	v[1] = axis;
	vpt->getGW()->polyline( 2, v, NULL, NULL, FALSE, NULL );	
	v[0] = axis;
	v[1] = v1+v2;
	vpt->getGW()->polyline( 2, v, NULL, NULL, FALSE, NULL );
	v[0] = axis;
	v[1] = v1-v2;
	vpt->getGW()->polyline( 2, v, NULL, NULL, FALSE, NULL );
	}


void DrawAxis(ViewExp *vpt, const Matrix3 &tm, float length, BOOL screenSize)
	{
	Matrix3 tmn = tm;
	float zoom;

	// Get width of viewport in world units:  --DS
	zoom = vpt->GetScreenScaleFactor(tmn.GetTrans())*ZFACT;
	
	if (screenSize) {
		tmn.Scale( Point3(zoom,zoom,zoom) );
		}
	vpt->getGW()->setTransform( tmn );		

	Text( vpt, _T("x"), Point3(length,0.0f,0.0f) ); 
	DrawAnAxis( vpt, Point3(length,0.0f,0.0f) );	
	
	Text( vpt, _T("y"), Point3(0.0f,length,0.0f) ); 
	DrawAnAxis( vpt, Point3(0.0f,length,0.0f) );	
	
	Text( vpt, _T("z"), Point3(0.0f,0.0f,length) ); 
	DrawAnAxis( vpt, Point3(0.0f,0.0f,length) );
	}



//--- RB 7/17/2000: the code below seems to be unused ---------------------------------------------------


Box3 GetAxisBox(ViewExp *vpt, const Matrix3 &tm,float length,int resetTM)
	{
	Matrix3 tmn = tm;
	Box3 box;
	float zoom;

	// Get width of viewport in world units:  --DS
	zoom = vpt->GetScreenScaleFactor(tmn.GetTrans())*ZFACT;
	if (zoom==0.0f) zoom = 1.0f;
//	tmn.Scale(Point3(zoom,zoom,zoom));
	length *= zoom;
	if(resetTM)
		tmn.IdentityMatrix();

	box += Point3(0.0f,0.0f,0.0f) * tmn;
	box += Point3(length,0.0f,0.0f) * tmn;
	box += Point3(0.0f,length,0.0f) * tmn;
	box += Point3(0.0f,0.0f,length) * tmn;	
	box += Point3(-length/5.f,0.0f,0.0f) * tmn;
	box += Point3(0.0f,-length/5.f,0.0f) * tmn;
	box += Point3(0.0f,0.0f,-length/5.0f) * tmn;
	box.EnlargeBy(10.0f/zoom);
	return box;
	}


inline void EnlargeRectIPoint3( RECT *rect, IPoint3& pt )
	{
	if ( pt.x < rect->left )   rect->left   = pt.x;
	if ( pt.x > rect->right )  rect->right  = pt.x;
	if ( pt.y < rect->top )    rect->top    = pt.y;
	if ( pt.y > rect->bottom ) rect->bottom = pt.y;
	}

// This is a guess - need to find real w/h.
#define FONT_HEIGHT	11
#define FONT_WIDTH  9	


static void AxisRect( GraphicsWindow *gw, Point3 axis, Rect *rect )
	{
	Point3 v1, v2, v;	
	IPoint3 iv;
	v1 = axis * (float)0.9;
	if ( axis.x != 0.0 || axis.y != 0.0 ) {
		v2 = Point3( axis.y, -axis.x, axis.z ) * (float)0.1;
	} else {
		v2 = Point3( axis.x, axis.z, -axis.y ) * (float)0.1;
		}
	v = axis;
	gw->wTransPoint( &v, &iv );
	EnlargeRectIPoint3( rect, iv);

	iv.x += FONT_WIDTH;
	iv.y -= FONT_HEIGHT;
	EnlargeRectIPoint3( rect, iv);

	v = v1+v2;
	gw->wTransPoint( &v, &iv );
	EnlargeRectIPoint3( rect, iv);
	v = v1-v2;
	gw->wTransPoint( &v, &iv );
	EnlargeRectIPoint3( rect, iv);
	}


void AxisViewportRect(ViewExp *vpt, const Matrix3 &tm, float length, Rect *rect)
	{
	Matrix3 tmn = tm;
	float zoom;
	IPoint3 wpt;
	Point3 pt;
	GraphicsWindow *gw = vpt->getGW();

	// Get width of viewport in world units:  --DS
	zoom = vpt->GetScreenScaleFactor(tmn.GetTrans())*ZFACT;
	
	tmn.Scale( Point3(zoom,zoom,zoom) );
	gw->setTransform( tmn );	
	pt = Point3(0.0f, 0.0f, 0.0f);
	gw->wTransPoint( &pt, &wpt );
	rect->left = rect->right  = wpt.x;
	rect->top  = rect->bottom = wpt.y;

	AxisRect( gw, Point3(length,0.0f,0.0f),rect );	
	AxisRect( gw, Point3(0.0f,length,0.0f),rect );	
	AxisRect( gw, Point3(0.0f,0.0f,length),rect );	

	rect->right  += 2;
	rect->bottom += 2;
	rect->left   -= 2;
	rect->top    -= 2;
	}



