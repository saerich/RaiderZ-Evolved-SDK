/**********************************************************************
 *<
	FILE: testonly.cpp

	DESCRIPTION: The file contains test only functions. It should be 
				 removed from the shipping version of Max 
	CREATED BY: Ravi Karra

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifdef INC_TESTONLY

//#include "pch.h"
#include "MAXScrpt.h"
#include "Numbers.h"
#include "MAXObj.h"
#include "Strings.h"

#include "resource.h"

#include "exprlib.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "MXSAgni.h"

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
#	include	"testonly.h"

#include "agnidefs.h"
#include "meshadj.h"
#include <maxheapdirect.h>

#define TESTAPP_CLASS_ID Class_ID(0x2c7c0cb4, 0x4d16287b)

void HandleLoadError(int status, Expr expr);
void DisplayExprResult(Expr expr, float *ans);
void HandleEvalError(int status, Expr expr);

Value* 
RenderFrame_cf(Value** arg_list, int count){ 
	int res;
	
	// Create a blank bitmap
	static Bitmap *bm = NULL;
	
	if (!bm) {
		BitmapInfo bi;		
		bi.SetWidth(320);
		bi.SetHeight(200);
		bi.SetType(BMM_TRUE_64);
		bi.SetFlags(MAP_HAS_ALPHA);
		bi.SetAspect(1.0f);
		bm = CreateBitmapFromBitmapInfo(bi);
		}

	// Get the active viewport to render
	ViewExp *view = MAXScript_interface->GetActiveViewport();
	
	// Display the bitmap
	bm->Display(_T("Test"));

	// Open up the renderer, render a frame and close it.
	res = MAXScript_interface->OpenCurRenderer(NULL,view);
	res = MAXScript_interface->CurRendererRenderFrame(
		MAXScript_time(),bm);
	MAXScript_interface->CloseCurRenderer();	

	// We're done with the viewport.
	MAXScript_interface->ReleaseViewport(view);
	return &ok;
	
}

Value* 
TestNode_cf(Value** arg_list, int count)
{
	check_arg_count(TestNode, 1, count);
	INode * node = arg_list[0]->to_node();
	CharStream* out = thread_local(current_stdout); 
	out->printf(_T("root node: %s\t numChildren: %i\n"),
		node->GetName(),
		node->NumberOfChildren());
	
	INode * pNode = node->GetParentNode();
	
	if(!pNode)
		out->printf(_T("No Parent Node\n"));
	
	for(int i = 0; i < node->NumberOfChildren(); i++){
		INode *cNode = node->GetChildNode(i);
		
		out->printf(_T("Child Node: %s\t NumChildren: %i\n"),
			cNode->GetName(),
			cNode->NumberOfChildren());
		}
	return &ok;
	
}

Value* 
TestTMAxis_cf(Value** arg_list, int count)
{
	check_arg_count(TestTMAxis, 1, count);
	TimeValue t =MAXScript_time();
	Matrix3 tm ;
	
	INode *node = arg_list[0]->to_node();
	//tm=selNode->GetObjectTM(t);
	tm=node->GetNodeTM(t);
	AngAxis aa(Point3(1,0,0),0.0f);
	aa.angle = DegToRad(90);
	node->Rotate(t, tm,aa);
	redraw_views();
	return &ok;
}


Value* 
TSetScale_cf(Value** arg_list, int count)
{
	check_arg_count(TSetScale, 1, count);
	TimeValue tv = MAXScript_time();
	INode *newNode = arg_list[0]->to_node();
	Matrix3 m3 = newNode->GetNodeTM(tv);
	Point3 p = Point3(2,1,1); 
	
	//m3.SetScale(p); BUG:fails because there is no such function in Matrix3.cpp
    m3.Scale(p);
	newNode->SetNodeTM(tv,m3);

	needs_redraw_set();
	return &ok;
}

Value* 
TestMatrix3_cf(Value** arg_list, int count)
{
	check_arg_count(TestMatrix3, 1, count);
	Matrix3 nm3, om3;
	INode *node;
	node = arg_list[0]->to_node();
	
	CharStream* out = thread_local(current_stdout); 
	out->printf(_T("Node :%s\n"), node->GetName());
	
	nm3 = node->GetNodeTM(MAXScript_time());
	om3 = node->GetObjectTM(MAXScript_time());		
	
	out->printf(_T("\nNode TM BEFORE \n"));		
	DebugPrint(out, nm3);
	
	out->printf(_T("\nObject TM BEFORE \n"));
	DebugPrint(out, om3);
	
	nm3.Translate(Point3(10,10,10));		
	node->SetNodeTM(MAXScript_time(),nm3);

	out->printf(_T("\nNode TM AFTER \n"));
	DebugPrint(out, nm3);
		
	om3 = node->GetObjectTM(MAXScript_time());
	out->printf(_T("\nObject TM AFTER \n"));
	DebugPrint(out, om3);				
	needs_redraw_set();
	return &ok;	
}


Value* 
ViewLocalRot_cf(Value** arg_list, int count) 
{
	check_arg_count(ViewLocalRot, 1, count);
	Interval valid = FOREVER;
	Matrix3 tmat;
	INode *node;
	
	CharStream* out = thread_local(current_stdout); 
	node = arg_list[0]->to_node();
	out->printf(_T("Node :%s\n"), node->GetName());		
	
	Control *c = node->GetTMController();
	tmat.IdentityMatrix();
	c->GetValue(MAXScript_time(), &tmat, valid, CTRL_RELATIVE);

	Quat q(tmat);
	float ang[3];
	QuatToEuler(q, ang);

	out->printf(_T("Local Rotation\n"));
	out->printf(_T("XRot=%.1f, YRot=%.1f, ZRot=%.1f"), 
		RadToDeg(ang[0]), RadToDeg(ang[1]), RadToDeg(ang[2]));
	return &ok;	
}


Value* 
ViewWorldRot_cf(Value** arg_list, int count) 
{
	check_arg_count(ViewLocalRot, 1, count);

	Interval valid = FOREVER;
	Matrix3 tmat;
	INode *node;

	CharStream* out = thread_local(current_stdout); 
	node = arg_list[0]->to_node();
	
	Control *c = node->GetTMController();
	tmat = node->GetParentTM(MAXScript_time());
	c->GetValue(MAXScript_time(), &tmat, valid, CTRL_RELATIVE);

	Quat q(tmat);
	float ang[3];
	QuatToEuler(q, ang);

	out->printf(_T("World Rotation\n"));
	out->printf(_T("XRot=%.1f, YRot=%.1f, ZRot=%.1f\n"), 
		RadToDeg(ang[0]), RadToDeg(ang[1]), RadToDeg(ang[2]));
	return &ok;
}

Value* 
GetSpaceWarp_cf(Value** arg_list, int count)
{
	check_arg_count(ViewLocalRot, 1, count);
	ReferenceTarget* n = arg_list[0]->to_node();
	ReferenceTarget* ref;
	int nRefs = n->NumRefs();

	for (int j = 0; j < nRefs; j++)
	{
		ref = n->GetReference(j);		
		if(!ref) continue;
		if(ref->SuperClassID()== WSM_CLASS_ID)
			return MAXClass::make_wrapper_for(ref);			
	}
	return &undefined;
}

//This function is a corrected function of NumCycles
//Delete this in future
inline int NCycles(Interval i,TimeValue t)
	{
	int dur = i.Duration();
	if (dur<=0) return 1;
	if (t<i.Start()) {
		return (abs(t+1-i.Start())/dur)+1;
	} else 
	if (t>i.End()) {
		return (abs(t-i.End())/dur)+1;
	} else {
		return 0;
		}
	}


Value* 
NumCycles_cf(Value** arg_list, int count)
{
	check_arg_count(NumCycles, 2, count); 
	return Integer::intern(NumCycles(arg_list[0]->to_interval(), 
		arg_list[1]->to_timevalue()));
}

Value* 
NewBitmapInfo_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(NewBitmapInfo, 1, count);
	def_bmp_types();
	//Bitmap *bmap;
	BitmapInfo *bi = new BitmapInfo();
	
	bi->SetType(GetID(bmpTypes, elements(bmpTypes), arg_list[0]));	
	bi->SetWidth(arg_list[0]->to_int());
	bi->SetHeight(arg_list[1]->to_int());
	bi->SetName(key_arg(file)->to_string());

//NC	return new BitmapInfoValue(bi);
	return &undefined;
}

Value* 
UndoRedoMultipleSelect_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(UndoRedoMultipleSelect, 0, count);
	theHold.Begin();
	MAXScript_interface->SelectNode(key_arg(node1)->to_node());
	MAXScript_interface->SelectNode(key_arg(node2)->to_node(), 0);
	TSTR undostr; undostr.printf("Multi Select");
	theHold.Accept(undostr);
	return &ok;	

}

Value*
TestExpr_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(TestExpr, 0, count);
	// Declare an expression instance and variable storage
	Expr expr;
	float sRegs[2];  // Must be at least getVarCount(SCALAR_VAR);
	Point3 vRegs[2]; // Must be at least getVarCount(VECTOR_VAR);
	float ans[3];
	int status;

	// Define a few expressions
	char* e0 = key_arg(expr)->to_string(); //"2+2";
	char e1[]	= "2.0 * pi * radius";
	char e2[] = "[1,1,0] + axis";
	char e3[] = "[sin(90.0), sin(radToDeg(0.5*pi)), axis.z]";
	char e4[] = "2+2*!@#$%"; // Bad expression

// Define variables
	int radiusReg = expr.defVar(SCALAR_VAR, _T("radius"));
	int axisReg = expr.defVar(VECTOR_VAR, _T("axis"));
	// Set the variable values
	sRegs[radiusReg] = 50.0f;
	vRegs[axisReg] = Point3(0.0f, 0.0f, 1.0f);
	// Get the number of each we have defined so far
	int sCount = expr.getVarCount(SCALAR_VAR);
	int vCount = expr.getVarCount(VECTOR_VAR);

	// Load and evaluate expression "e0"
	if (status = expr.load(e0)) 
		HandleLoadError(status, expr);

	else 
	{
		status = expr.eval(ans, sCount, sRegs, vCount, vRegs);
		if (status != EXPR_NORMAL) 
			HandleEvalError(status, expr);
		else
			DisplayExprResult(expr, ans);
	}
	// Load and evaluate expression "e1"
	if (status = expr.load(e1)) 
		HandleLoadError(status, expr);
	else 
	{
		status = expr.eval(ans, sCount, sRegs, vCount, vRegs);
		if (status != EXPR_NORMAL) 
			HandleEvalError(status, expr);
		else
			DisplayExprResult(expr, ans);

	}
	// Load and evaluate expression "e2"
	if (status = expr.load(e2)) 
		HandleLoadError(status, expr);
	else 
	{
		status = expr.eval(ans, sCount, sRegs, vCount, vRegs);
		if (status != EXPR_NORMAL) 
			HandleEvalError(status, expr);
		else
			DisplayExprResult(expr, ans);
	}
	// Load and evaluate expression "e3"
	if (status = expr.load(e3)) 
		HandleLoadError(status, expr);
	else 
	{
		status = expr.eval(ans, sCount, sRegs, vCount, vRegs);

		if (status != EXPR_NORMAL) 
			HandleEvalError(status, expr);
		else
			DisplayExprResult(expr, ans);
	}
	// Load and evaluate expression "e4"
	if (status = expr.load(e4)) 
		HandleLoadError(status, expr);
	else 
	{
		status = expr.eval(ans, sCount, sRegs, vCount, vRegs);
		if (status != EXPR_NORMAL) 
			HandleEvalError(status, expr);
		else
			DisplayExprResult(expr, ans);
	}
	return &ok;
}

// Display the expression and the result
void
DisplayExprResult(Expr expr, float *ans) 
{

	TCHAR msg[128];

	if (expr.getExprType()== SCALAR_EXPR) 
	{
		_stprintf(msg, _T("Answer to \"%s\" is %.1f"), 
			expr.getExprStr(), *ans);
	}
	else 
	{
		_stprintf(msg, _T("Answer to \"%s\" is [%.1f, %.1f, %.1f]"), 
			expr.getExprStr(), ans[0], ans[1], ans[2]);
	}
	CharStream* out = thread_local(current_stdout); 
	out->printf("Expression Result: %s\n", msg);
}

// Display the load error message
void
HandleLoadError(int status, Expr expr) 
{
	CharStream* out = thread_local(current_stdout); 	
	
	if(status == EXPR_INST_OVERFLOW) 
		out->printf(_T("Inst stack overflow: %s"), expr.getProgressStr());
	else if (status == EXPR_UNKNOWN_TOKEN) 
		out->printf(_T("Unknown token: %s"), expr.getProgressStr());
	else 
		out->printf(_T("Cannot parse \"%s\". Error begins at last char of: %s"), 
			expr.getExprStr(), expr.getProgressStr());
}

// Display the evaluation error message	
void
HandleEvalError(int status, Expr expr) 
{
	thread_local(current_stdout)->printf(
		_T("Can't parse expression \"%s\""), expr.getExprStr());
}


Value* 
PutSceneAppData_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(PutSceneAppData, 2, count);
	Animatable *anim = MAXScript_interface->GetScenePointer();
	
	int size = (int)((_tcslen(arg_list[1]->to_string()) + 1) * sizeof(TCHAR));
	TCHAR *buf = (TCHAR*)MAX_malloc(size);
	
	_tcscpy(buf, arg_list[1]->to_string());
	
	anim->RemoveAppDataChunk(
		TESTAPP_CLASS_ID, 
		UTILITY_CLASS_ID, 
		arg_list[0]->to_int());
	
	anim->AddAppDataChunk(
		TESTAPP_CLASS_ID, 
		UTILITY_CLASS_ID, 
		arg_list[0]->to_int(),
		(DWORD)size, buf);
	return &ok;	
}

Value* 
GetSceneAppData_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(GetSceneAppData, 1, count);
	Animatable *anim = MAXScript_interface->GetScenePointer();
	AppDataChunk *ad = 
		anim->GetAppDataChunk(
			TESTAPP_CLASS_ID, 
			UTILITY_CLASS_ID, 
			arg_list[0]->to_int());
	
	if(ad) return new String((TCHAR*)ad->data);
	return &undefined;	
}

Value*
GetSubAnims_cf(Value** arg_list, int count)
{
	check_arg_count(GetSubAnims, 1, count);
	INode *node = arg_list[0]->to_node();
	Mtl *pMtl;
	CharStream* out = thread_local(current_stdout); 
	for(int i = 0; i < node->NumSubs(); i++)
	{
		pMtl = node->GetMtl();
		out->printf(
			_T("index: %i\tSubAnim: %s\tname: %s\n"), 
			i, 
			node->SubAnim(i)?_T("true"):_T("false"),
			(i==4)?(pMtl?node->SubAnimName(i):_T("---")):node->SubAnimName(i)	
			);
	}
	return &ok;
}

Value*
BitmapWrite_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(BitmapWrite, 2, count);
	BitmapInfo obi, cbi;
	Bitmap *omap, *cmap;
	BMMRES status;

	obi.SetName(arg_list[0]->to_string());
	
	omap = TheManager->Load(&obi, &status);
	
	if(status != BMMRES_SUCCESS)
	{
		throw RuntimeError("Error reading bitmap");
		
	}
	
	cbi = obi;
	cbi.SetName(arg_list[1]->to_string());
	cbi.SetType(BMM_TRUE_32);

	cmap = CreateBitmapFromBitmapInfo(cbi);
	cmap->CopyImage(omap, COPY_IMAGE_RESIZE_HI_QUALITY, 0);	

	cmap->OpenOutput(&cbi);
	cmap->Write(&cbi);
	cmap->Close(&cbi);

	cmap->Display("The copied Bitmap");		
	return &ok;
}


// NEW CLASS for bmi
Value*
Create_cf(Value** arg_list, int count)
{
	check_arg_count(Create, 1, count);
//NC	return TheManager->Create(arg_list[0]->to_bmi());
	return &undefined;
}

Value*
Load_cf(Value** arg_list, int count)
{
	check_arg_count(Load, 1, count);	
/*	
	def_bmmerror_types();
	BMMRES status;
	Bitmap *bMap;
	
	BitmapInfo bi;
//NC	bMap = TheManager->Load(arg_list[0]->to_bmi(), &status);
	bMap = TheManager->Load(&bi, &status);
	if (status != BMMRES_SUCCESS) 
		return new GetName(bmmerrorTypes, status);
	
	return new MAXBitMap(bi, bMap);
	*/
	return &ok;
}


class NullView: public View
{
public:
	Point2 ViewToScreen(Point3 p) { return 		Point2(p.x,p.y); }
	NullView() { worldToView.IdentityMatrix(); 		screenW = 0.0f; screenH = 480.0f; }
};

Value*
GetRenderMeshBug_cf(Value** arg_list, int count)
{
	check_arg_count(Load, 1, count);	
	INode *node=get_valid_node(arg_list[0], GetRenderMeshBug);
	
	NullView nullView;
	ObjectState os = node->EvalWorldState(MAXScript_interface->GetTime());

	if (os.obj == NULL) return &ok;
	if (os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID) return &ok;

	int needDel;
	Mesh* cMeshPtr = ((GeomObject*)os.obj)->GetRenderMesh(
		MAXScript_interface->GetTime(), node, nullView,needDel);

	Matrix3 tm(TRUE);
	cMeshPtr->ApplyUVWMap(MAP_SPHERICAL,1.0f, 1.0f,1.0f,0,0,0,0,tm);

	if(needDel)
		delete cMeshPtr;
	return &ok;
}

Value*
AddNewNamedSelSet_cf(Value** arg_list, int count)
{
	check_arg_count(AddNewNamedSelSet, 0, count);	
	INodeTab ntab;
	for (int i=0; i < MAXScript_interface->GetSelNodeCount(); i++) {
		INode *node = MAXScript_interface->GetSelNode(i);
		ntab.Append(1, &node, 5);
	}
	MAXScript_interface->AddNewNamedSelSet(ntab, TSTR(_T("C:\\something\\directory\\file.ext")));
	return &ok;
}


Value*
SnapAngle_cf(Value** arg_list, int count)
{
	//float snapangle <float> [fastsnap:boolean] [forcesnap:boolean]
	//snapangle 5.0 fastsnap:true forcesnap:false

	check_arg_count_with_keys (SnapAngle, 1, count);


	Value* fastsnap = key_arg_or_default (fastsnap, &true_value);
	Value* forcesnap = key_arg_or_default (forcesnap, &false_value);
	
	return Float::intern(MAXScript_interface->SnapAngle (arg_list[0]->to_float(), fastsnap->to_bool(), forcesnap->to_bool()) );
}


Value*
GetMatID_cf(Value** arg_list, int count)
{
	int the_curve;
	int the_piece;

	check_arg_count (GetMatID, 3, count);
	
	INode* node = arg_list[0]->to_node();
	ObjectState os = node->EvalWorldState(MAXScript_time());
	Object* refobj = os.obj;

	if (refobj->SuperClassID() == SHAPE_CLASS_ID)
	{
		ShapeObject *theshape = (ShapeObject *)refobj;
	
		the_curve = arg_list[1]->to_int();
		the_piece = arg_list[2]->to_int();
		if (theshape->NumberOfCurves() >= the_curve && the_curve > 0)
		{
			if (theshape->NumberOfPieces(MAXScript_time(), the_curve-1) >= the_piece && the_piece > 0)
			{
				return Integer::intern ( (theshape->GetMatID (MAXScript_time(), the_curve-1, the_piece-1) + 1 ));
			}
		}

		throw RuntimeError(_T("Verify parameters for the curve and piece\n"));
	}

	return &undefined;
}


Value*
SelectionDistance_cf (Value** arg_list, int count)
{
	Object *obj;
	check_arg_count (SelectionDistance, 1, count);

	INode* node = arg_list[0]->to_node();
	
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
		
	/* get the associated mesh and return the average center of the selected vertices */
	Mesh &themesh = ((TriObject *) obj)->GetMesh();
	float *thedistance = (float *)malloc (sizeof (float) * themesh.getNumVerts());
	SelectionDistance (themesh, thedistance);
	free (thedistance);
	return &ok;
}



Value*
CheckForRenderAbort_cf(Value** arg_list, int count)
{
	//bool checkforrenderabort

	check_arg_count (CheckForRenderAbort, 0, count);

	return MAXScript_interface->CheckForRenderAbort () ? &true_value : &false_value;

}


Value*
GetCommandStackSize_cf(Value** arg_list, int count)
{
	check_arg_count (GetCommandStackSize, 0, count);
	return Integer::intern(MAXScript_interface->GetCommandStackSize());
}


Value*
AddGridToScene_cf(Value** arg_list, int count)
{
	check_arg_count (AddGridtoScene, 1, count);
	MAXScript_interface->AddGridToScene (arg_list[0]->to_node());
	return &ok;
}

Value*
GetImportZoomExtents_cf(Value** arg_list, int count)
{
	check_arg_count (GetImportZoomExtent, 0, count);
	return MAXScript_interface->GetImportZoomExtents() ? &true_value : &false_value;
}

Value*
GetSavingVersion_cf (Value** arg_list, int count)
{
	check_arg_count (GetSavingVersion, 0, count);

	return Integer::intern (GetSavingVersion());

}

// Get-Set functions for lights
extern Value*  get_light_atmosOpacity(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid);
extern void    set_light_atmosOpacity(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val);
extern Value*  get_light_atmosColAmt(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid);
extern void    set_light_atmosColAmt(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val);
extern Value*  get_light_decayRadius(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid);
extern void    set_light_decayRadius(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val);
extern Value*  get_light_shadowColor(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid);
extern void    set_light_shadowColor(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val);
extern Value*  get_light_setShadMult(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid);
extern void    set_light_getShadMult(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val);

/*Value*  
get_light_atmosOpacity(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)
{
	return Float::intern(100.0f * ((GenLight*)obj)->GetAtmosOpacity(t, valid));
}
void    
set_light_atmosOpacity(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)
{
	((GenLight*)obj)->SetAtmosOpacity(t, val->to_float()/100.0f);
}

Value*  
get_light_atmosColAmt(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)
{
	return Float::intern(100.0f * ((GenLight*)obj)->GetAtmosColAmt(t, valid));
}
void    
set_light_atmosColAmt(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)
{
	((GenLight*)obj)->SetAtmosColAmt(t, val->to_float()/100.0f);
}

Value*  
get_light_decayRadius(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)
{
	return Float::intern(((GenLight*)obj)->GetDecayRadius(t, valid));
}
void    
set_light_decayRadius(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)
{
	((GenLight*)obj)->SetDecayRadius(t, val->to_float());
}

Value*  
get_light_shadowColor(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)
{
	return new Point3Value(((GenLight*)obj)->GetShadColor(t, valid) * 255.0f);
}
void    
set_light_shadowColor(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)
{
	((GenLight*)obj)->SetShadColor(t, val->to_point3() / 255.0f);
}

Value*  
get_light_setShadMult(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)
{
	return Float::intern(((GenLight*)obj)->GetShadMult(t, valid));
}
void    
set_light_getShadMult(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)
{
	((GenLight*)obj)->SetShadMult(t, val->to_float());
}*/

/* --------------------- plug-in init --------------------------------- */

#endif //INC_TESTONLY
