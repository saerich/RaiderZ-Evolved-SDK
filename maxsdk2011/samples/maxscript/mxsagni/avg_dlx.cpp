// NOTE: Everything in here except for getSegLengths_cf and possibly subdivideSegment_cf 
// OK for public consumption. These methods use code plagerized from Spline3D.

// Note: Integration: fix up include path to triobjed.h

/*	Adds new primitives/globals
 *  Larry Minton, 1999
 */

#include "avg_maxver.h"

#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "Strings.h"
#include "Arrays.h"
#include <hashtab.h>
#include "MAXMats.h"
#include "MAXclses.h"
#include "ColorVal.h"
#include "Parser.h"
#include "Listener.h"
#include "Structs.h"
#include "MSPlugin.h"
#include "mscustattrib.h"
#include "funcs.h"
#include "thunks.h"
#include "bitmaps.h"
#include "mszippackage.h"
#include <trig.h>
#include "resource.h"
//#include "avg_MSPlugIn.h"

#include "SPLSHAPE.h"
#include "modstack.h"
#include "interactiverender.h"
#include "IMtlRender_Compatibility.h"
#include "WSMClasses.h"
#include "hsv.h"

#include "iLayerManager.h"
#include "ILayerProperties.h"
#include "ILayer.h"
#include <soundobj.h>
#include <AnimEnum.h>
#include "INodeExposure.h"
#include "IPathConfigMgr.h"
#include "assetmanagement\AssetType.h"
#include "IFileResolutionManager.h"



#include "MXSAgni.h"
#include <maxscript\macros\define_instantiation_functions.h>
#include "avg_dlx.h"
#include "agnidefs.h"

#include <locale.h>
#include <Wininet.h>
#include <SHLWAPI.H>

#include "ICustAttribContainer.h"
#include "CustAttrib.h"

#include "xref\iXrefItem.h"
#include "3dsmaxport.h"
#include "PerformanceTools.h"

#include "AssetManagement/iassetmanager.h"

using namespace MaxSDK::AssetManagement;

// ============================================================================
#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
	#include "lam_wraps.h"

//  =================================================
//  Spline methods
//  =================================================

#define NUMARCSTEPS 100

Value*
getSegLengths_cf(Value** arg_list, int count)
{
	// getSegLengths <splineShape> <spline_index> [cum:<boolean>] [byVertex:<boolean>] [numArcSteps:<integer>]
	check_arg_count_with_keys(getSegLengths, 2, count);
	MAXNode* shape = (MAXNode*)arg_list[0];
	type_check(shape, MAXNode, _T("getSegLengths"));
	deletion_check(shape);
	Value* tmp;
	BOOL cum = bool_key_arg(cum, tmp, FALSE); 
	BOOL byVertex = bool_key_arg(byVertex, tmp, FALSE); 
	int numArcSteps = int_key_arg(numArcSteps, tmp, NUMARCSTEPS); 
	int index = arg_list[1]->to_int();
	BezierShape* s = set_up_spline_access(shape->node, index);
	Spline3D* s3d = s->splines[index-1];
	s3d->ComputeBezPoints();
	int segs = s3d->Segments();
	int knotCount = s3d->KnotCount();
	one_typed_value_local(Array* result);
	vl.result = new Array (segs);
	double totalLength = 0.0;
	double uStep=(double)1.0/numArcSteps;
	double *segLen = new double [segs];

	for(int seg = 0; seg < segs; ++seg) {
		DPoint3 k1p = s3d->GetKnotPoint(seg);
		DPoint3 k1o = s3d->GetOutVec(seg);
		int seg2 = (seg+1) % knotCount;
		DPoint3 k2p = s3d->GetKnotPoint(seg2);
		DPoint3 k2i = s3d->GetInVec(seg2);
		BOOL segIsLine = s3d->GetLineType(seg) == LTYPE_LINE;
		DPoint3 pa = k1p;
		DPoint3 pb;
		double segLength = 0.0;
		for(int arcStep = 1; arcStep < numArcSteps; ++arcStep) {
			double t = uStep*arcStep;
			if (segIsLine) pb = k1p + ((k2p - k1p) * t);
			else {
				double s = (double)1.0-t;
				pb = s*s*s*k1p + t*s*3.0*(k1o*s + k2i*t) + t*t*t*k2p;
			}
			segLength += Length(pb - pa);
			pa = pb;
		}					
		pb = k2p;
		segLength += Length(pb - pa);
		segLen[seg] = segLength;
		totalLength += segLength;
	}

	double cummVal=0.0;
	if (byVertex) vl.result->append(Float::intern((float)cummVal));
	double thisVal;
	for(int seg = 0; seg < segs; ++seg) {
		thisVal=segLen[seg]/totalLength+cummVal;
		vl.result->append(Float::intern((float)thisVal));
		if (cum) cummVal=thisVal;
	}

	cummVal=0.0;
	if (byVertex) vl.result->append(Float::intern((float)cummVal));
	for(int seg = 0; seg < segs; ++seg) {
		thisVal=segLen[seg]+cummVal;
		vl.result->append(Float::intern((float)thisVal));
		if (cum) cummVal=thisVal;
	}

	vl.result->append(Float::intern((float)totalLength));
	delete [] segLen;
	return_value(vl.result);
}

Value*
subdivideSegment_cf(Value** arg_list, int count)
{
	// subdivideSegment <shape> <spline_index> <seg_index> <divisions>   -- return ok
// TODO: check to see if the line and knot types are correct.
	check_arg_count(subdivideSegment, 4, count);
	MAXNode* MAXshape = (MAXNode*)arg_list[0];
	type_check(MAXshape, MAXNode, _T("subdivideSegment"));
	deletion_check(MAXshape);
	int sp_index = arg_list[1]->to_int();
	BezierShape* shape = set_up_spline_access(MAXshape->node, sp_index);
	Spline3D* spline = shape->splines[sp_index-1];
	int seg_index = arg_list[2]->to_int();
	range_check(seg_index, 1, spline->Segments(), GetString(IDS_SPLINE_SEGMENT_INDEX_OUT_OF_RANGE))
	int knots = spline->KnotCount();
	int segs = spline->Segments();
	int segment = seg_index - 1;
	int nextSeg = (segment + 1) % knots;
	int insertSeg = (nextSeg == 0) ? -1 : nextSeg;
	int numDivisions = arg_list[3]->to_int();
	if (numDivisions <= 0) return &ok;
	spline->ComputeBezPoints();

	// Get the initial knot points
	DPoint3 v00 = spline->GetKnotPoint(segment); // prev knot location
	DPoint3 v30 = spline->GetKnotPoint(nextSeg); // next knot location

	// Special: If they're refining a line-type segment, force it to be a bezier curve again
	if(spline->GetLineType(segment) == LTYPE_LINE) {
		spline->SetKnotType(segment, KTYPE_BEZIER_CORNER);
		spline->SetKnotType(nextSeg, KTYPE_BEZIER_CORNER);
		spline->SetLineType(segment, LTYPE_CURVE);
		spline->SetOutVec(segment, DPoint3toPoint3 (v00 + (v30 - v00) / 3.0));
		spline->SetInVec(nextSeg, DPoint3toPoint3 (v30 - (v30 - v00) / 3.0));
	}

	DPoint3 v10 = spline->GetOutVec(segment);  // prev knot out vector
	DPoint3 v20 = spline->GetInVec(nextSeg);   // next knot in vector
	DPoint3 v01;
	for(int divStep = numDivisions; divStep > 0; --divStep) {
		double param=(double)divStep/(double)(divStep+1);

		v01 = v00 + (v10 - v00) * param; // new prev knot out vector
		DPoint3 v21 = v20 + (v30 - v20) * param; // new next knot in vector
		DPoint3 v11 = v10 + (v20 - v10) * param;
		DPoint3 v02 = v01 + (v11 - v01) * param; // new knot in vector
		DPoint3 v12 = v11 + (v21 - v11) * param; // new knot out vector
		DPoint3 v03 = v02 + (v12 - v02) * param; // new knot position

		spline->SetInVec(nextSeg, DPoint3toPoint3 (v21));

		SplineKnot newKnot(KTYPE_BEZIER, LTYPE_CURVE, DPoint3toPoint3 (v03), DPoint3toPoint3 (v02), DPoint3toPoint3 (v12));
		spline->AddKnot(newKnot, insertSeg);
		v20 = v02;
		v10 = v01;
		v30 = v03;
		nextSeg=insertSeg=seg_index;
	}					
	spline->SetOutVec(segment, DPoint3toPoint3 (v01));

	spline->ComputeBezPoints();
	shape->InvalidateGeomCache();

	// Now adjust the spline selection sets
	BitArray& vsel = shape->vertSel[sp_index-1];
	BitArray& ssel = shape->segSel[sp_index-1];
	vsel.SetSize(spline->Verts(), 1);
	int where = (segment + 1) * 3;
	vsel.Shift(RIGHT_BITSHIFT, 3 * numDivisions, where);
	vsel.Clear(where);
	vsel.Clear(where+1);
	vsel.Clear(where+2);
	ssel.SetSize(spline->Segments(), 1);
	ssel.Shift(RIGHT_BITSHIFT, numDivisions, segment + 1);
	for(int i = 1; i > numDivisions; ++i) {ssel.Set(segment+i,ssel[segment]);}
	return  &ok;
}

Value*
interpCurve3D_cf(Value** arg_list, int count)
{
	// interpCurve3D <splineShape> <spline_index> <param_float> [pathParam:<boolean>]
	check_arg_count_with_keys(interpCurve3D, 3, count);
	MAXNode* shape = (MAXNode*)arg_list[0];
	type_check(shape, MAXNode, _T("interpCurve3D"));
	deletion_check(shape);
	Value* tmp;
	int ptype = (bool_key_arg(pathParam, tmp, FALSE)) ? SPLINE_INTERP_SIMPLE : SPLINE_INTERP_NORMALIZED; 
	int index = arg_list[1]->to_int();
	BezierShape* s = set_up_spline_access(shape->node, index);
	Spline3D* s3d = s->splines[index-1];
	Point3 p = s3d->InterpCurve3D(arg_list[2]->to_float(), ptype);
	shape->object_to_current_coordsys(p);
	return new Point3Value(p);
}

Value*
tangentCurve3D_cf(Value** arg_list, int count)
{
	// tangentCurve3D <splineShape> <spline_index> <param_float> [pathParam:<boolean>]
	check_arg_count_with_keys(tangentCurve3D, 3, count);
	MAXNode* shape = (MAXNode*)arg_list[0];
	type_check(shape, MAXNode, _T("tangentCurve3D"));
	deletion_check(shape);
	Value* tmp;
	int ptype = (bool_key_arg(pathParam, tmp, FALSE)) ? SPLINE_INTERP_SIMPLE : SPLINE_INTERP_NORMALIZED; 
	int index = arg_list[1]->to_int();
	BezierShape* s = set_up_spline_access(shape->node, index);
	Spline3D* s3d = s->splines[index-1];
	Point3 p = s3d->TangentCurve3D(arg_list[2]->to_float(), ptype);
	shape->object_to_current_coordsys_rotate(p);
	return new Point3Value(p);
}

Value*
setMaterialID_cf(Value** arg_list, int count)
{
	// setMaterialID <shape> <spline_index> <seg_index> <matID>   -- return ok
	check_arg_count(setMaterialID, 4, count);
	MAXNode* MAXshape = (MAXNode*)arg_list[0];
	type_check(MAXshape, MAXNode, _T("setMaterialID"));
	deletion_check(MAXshape);
	int sp_index = arg_list[1]->to_int();
	BezierShape* shape = set_up_spline_access(MAXshape->node, sp_index);
	Spline3D* spline = shape->splines[sp_index-1];
	int seg_index = arg_list[2]->to_int();
	range_check(seg_index, 1, spline->Segments(), GetString(IDS_SPLINE_SEGMENT_INDEX_OUT_OF_RANGE))
	spline->SetMatID(seg_index-1, arg_list[3]->to_int() - 1);
	return  &ok;
}

Value*
getMaterialID_cf(Value** arg_list, int count)
{
	// getMaterialID <shape> <spline_index> <seg_index>
	check_arg_count(getMaterialID, 3, count);
	MAXNode* MAXshape = (MAXNode*)arg_list[0];
	type_check(MAXshape, MAXNode, _T("getMaterialID"));
	deletion_check(MAXshape);
	int sp_index = arg_list[1]->to_int();
	BezierShape* shape = set_up_spline_access(MAXshape->node, sp_index);
	Spline3D* spline = shape->splines[sp_index-1];
	int seg_index = arg_list[2]->to_int();
	range_check(seg_index, 1, spline->Segments(), GetString(IDS_SPLINE_SEGMENT_INDEX_OUT_OF_RANGE))
	return Integer::intern((int)spline->GetMatID(seg_index-1) + 1);
}

Value*
affectRegionVal_cf(Value** arg_list, int count)
{
	// affectRegionVal <distance_float> <falloff_float> <pinch_float> <bubble_float>
	check_arg_count(affectRegionVal, 4, count);
	float res = AffectRegionFunction(arg_list[0]->to_float(), arg_list[1]->to_float(),
									 arg_list[2]->to_float(), arg_list[3]->to_float());
	return Float::intern(res);
}


//  =================================================
//  Node and Node TM methods
//  =================================================

Value*
getTransformAxis_cf(Value** arg_list, int count)
{
	check_arg_count(getTransformAxis, 2, count);
	INode* node = NULL;
	if (arg_list[0] != &undefined) node = get_valid_node(arg_list[0], getTransformAxis);
	int index = arg_list[1]->to_int()+1;
	return new Matrix3Value(MAXScript_interface->GetTransformAxis(node,index));
}

Value*
invalidateTM_cf(Value** arg_list, int count)
{
	check_arg_count(InvalidateTM, 1, count);
	INode* node = get_valid_node(arg_list[0], InvalidateTM);
	node->InvalidateTM();
	needs_redraw_set();
	return &ok;
}

Value*
invalidateTreeTM_cf(Value** arg_list, int count)
{
	check_arg_count(InvalidateTreeTM, 1, count);
	INode* node = get_valid_node(arg_list[0], InvalidateTreeTM);
	node->InvalidateTreeTM();
	needs_redraw_set();
	return &ok;
}

Value*
invalidateWS_cf(Value** arg_list, int count)
{
	check_arg_count(invalidateWS, 1, count);
	INode* node = get_valid_node(arg_list[0], invalidateWS);
	node->InvalidateWS();
	MAXScript_interface7->InvalidateObCache(node);
	needs_redraw_set();
	return &ok;
}

Value*
snapshotAsMesh_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(snapshot,1, count);
	INode* node = get_valid_node(arg_list[0], snapshotAsMesh);
	Value* tmp;
	BOOL renderMesh = bool_key_arg(renderMesh, tmp, TRUE); 
	Mesh *resMesh = getWorldStateMesh(node, renderMesh);
	Value *resMesh2 = resMesh ? (Value*)(new MeshValue (resMesh,TRUE)) : &undefined;
//	if (resMesh) delete resMesh;
	return resMesh2;
}

//  =================================================
//  Node Selection Lock methods
//  =================================================

Value*
isSelectionFrozen_cf(Value** arg_list, int count)
{
	check_arg_count(isSelectionFrozen, 0, count);
	return ((MAXScript_interface->SelectionFrozen()) ? &true_value : &false_value);
}

Value*
freezeSelection_cf(Value** arg_list, int count)
{
	check_arg_count(freezeSelection, 0, count);
	MAXScript_interface->FreezeSelection();
	return &ok;
}

Value*
thawSelection_cf(Value** arg_list, int count)
{
	check_arg_count(thawSelection, 0, count);
	MAXScript_interface->ThawSelection();
	return &ok;
}


Value*
getInheritanceFlags_cf(Value** arg_list, int count)
{
	check_arg_count(getInheritanceFlags, 1, count);
	INode* node = get_valid_node(arg_list[0], getInheritanceFlags);
	BitArray flags(9);
	DWORD iflags = node->GetTMController()->GetInheritanceFlags();
	for (int i = 0, j = 1; i < 9; i++, j= (j << 1)) 
		flags.Set(i,!(iflags & j));
	return new BitArrayValue(flags);
}

Value*
setInheritanceFlags_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setInheritanceFlags, 2, count);
	INode* node = get_valid_node(arg_list[0], setInheritanceFlags);
	BitArray flags(9);
	Value* tmp;
	BOOL keepPos = bool_key_arg(keepPos, tmp, TRUE); 
	flags.ClearAll();
	if (arg_list[1] == n_all)
		flags.SetAll();
	else if (arg_list[1] != n_none) {
		flags = arg_list[1]->to_bitarray();
		flags.SetSize(9,1);
	}
	DWORD intFlag=0;
	for (int i = 0, j = 1; i < 9; i++, j= (j << 1)) 
		if (flags[i]) intFlag += j;
	node->GetTMController()->SetInheritanceFlags(intFlag,keepPos);
	BOOL suspendPanel = node->Selected();
	if (suspendPanel)
	{
		MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
		MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);
	}
	return &ok;
}

Value*
getTransformLockFlags_cf(Value** arg_list, int count)
{
	check_arg_count(getTransformLockFlags, 1, count);
	INode* node = get_valid_node(arg_list[0], getTransformLockFlags);
	BitArray flags(9);
	flags.Set(0,node->GetTransformLock(INODE_LOCKPOS, INODE_LOCK_X));
	flags.Set(1,node->GetTransformLock(INODE_LOCKPOS, INODE_LOCK_Y));
	flags.Set(2,node->GetTransformLock(INODE_LOCKPOS, INODE_LOCK_Z));
	flags.Set(3,node->GetTransformLock(INODE_LOCKROT, INODE_LOCK_X));
	flags.Set(4,node->GetTransformLock(INODE_LOCKROT, INODE_LOCK_Y));
	flags.Set(5,node->GetTransformLock(INODE_LOCKROT, INODE_LOCK_Z));
	flags.Set(6,node->GetTransformLock(INODE_LOCKSCL, INODE_LOCK_X));
	flags.Set(7,node->GetTransformLock(INODE_LOCKSCL, INODE_LOCK_Y));
	flags.Set(8,node->GetTransformLock(INODE_LOCKSCL, INODE_LOCK_Z));
	return new BitArrayValue(flags);
}

Value*
setTransformLockFlags_cf(Value** arg_list, int count)
{
	check_arg_count(setTransformLockFlags, 2, count);
	INode* node = get_valid_node(arg_list[0], setTransformLockFlags);
	BitArray flags(9);
	flags.ClearAll();
	if (arg_list[1] == n_all)
		flags.SetAll();
	else if (arg_list[1] != n_none) {
		flags = arg_list[1]->to_bitarray();
		flags.SetSize(9,1);
	}
	node->SetTransformLock(INODE_LOCKPOS, INODE_LOCK_X, flags[0]);
	node->SetTransformLock(INODE_LOCKPOS, INODE_LOCK_Y, flags[1]);
	node->SetTransformLock(INODE_LOCKPOS, INODE_LOCK_Z, flags[2]);
	node->SetTransformLock(INODE_LOCKROT, INODE_LOCK_X, flags[3]);
	node->SetTransformLock(INODE_LOCKROT, INODE_LOCK_Y, flags[4]);
	node->SetTransformLock(INODE_LOCKROT, INODE_LOCK_Z, flags[5]);
	node->SetTransformLock(INODE_LOCKSCL, INODE_LOCK_X, flags[6]);
	node->SetTransformLock(INODE_LOCKSCL, INODE_LOCK_Y, flags[7]);
	node->SetTransformLock(INODE_LOCKSCL, INODE_LOCK_Z, flags[8]);
	BOOL suspendPanel = node->Selected();
	if (suspendPanel)
	{
		MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
		MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);
	}
	return &ok;
}

//  =================================================
//  Listener methods
//  =================================================

Value*
get_listener()
{
	return the_listener->edit_stream;
}

Value*
get_macroRecorder()
{
	return the_listener->macrorec_stream;
}

//  =================================================
//  Sound methods
//  =================================================

Value*
setSoundFileName(Value* val)
{
	// Get the current sound object.
	SoundObj *snd = MAXScript_interface->GetSoundObject();

	// See if we can get a wave interface
	IWaveSound *iWav = GetWaveSoundInterface(snd);
	if (iWav) {
		// Set the sound file
		IAssetManager* assetMgr = IAssetManager::GetInstance();
		if(assetMgr)
		{
			iWav->SetSoundFile(assetMgr->GetAsset(val->to_filename(),kSoundAsset));
		}
	}
	return val;
}

Value*
getSoundFileName()
{
	// Get the current sound object.
	SoundObj *snd = MAXScript_interface->GetSoundObject();

	// See if we can get a wave interface
	IWaveSound *iWav = GetWaveSoundInterface(snd);
	if (iWav) 
		// Get the sound file
		return new String (iWav->GetSoundFile().GetFileName());
	else 
		return &undefined;
}

Value*
setSoundStartTime(Value* val)
{
	// Get the current sound object.
	SoundObj *snd = MAXScript_interface->GetSoundObject();

	// See if we can get a wave interface
	IWaveSound *iWav = GetWaveSoundInterface(snd);
	if (iWav)
		// Set the sound file start time
		iWav->SetStartTime(val->to_timevalue());
	return val;
}

Value*
getSoundStartTime()
{
	// Get the current sound object.
	SoundObj *snd = MAXScript_interface->GetSoundObject();

	// See if we can get a wave interface
	IWaveSound *iWav = GetWaveSoundInterface(snd);
	if (iWav) 
		// Get the sound file start time
		return MSTime::intern(iWav->GetStartTime());
	else 
		return &undefined;
}

Value*
getSoundEndTime()
{
	// Get the current sound object.
	SoundObj *snd = MAXScript_interface->GetSoundObject();

	// See if we can get a wave interface
	IWaveSound *iWav = GetWaveSoundInterface(snd);
	if (iWav) 
		// Get the sound file end time
		return MSTime::intern(iWav->GetEndTime());
	else 
		return &undefined;
}

Value*
setSoundMute(Value* val)
{
	// Get the current sound object.
	SoundObj *snd = MAXScript_interface->GetSoundObject();
	snd->SetMute(val->to_bool());
	return val;
}

Value*
getSoundMute()
{
	// Get the current sound object.
	SoundObj *snd = MAXScript_interface->GetSoundObject();
	return ((snd->IsMute()) ? &true_value : &false_value);
}

Value*
getSoundIsPlaying()
{
	// Get the current sound object.
	SoundObj *snd = MAXScript_interface->GetSoundObject();
	return ((snd->Playing()) ? &true_value : &false_value);
}

Value*
SoundScrub_cf(Value** arg_list, int count)
{
	check_arg_count(setSoundMute, 1, count);
	// Get the current sound object.
	SoundObj *snd = MAXScript_interface->GetSoundObject();
	Interval range = arg_list[0]->to_interval();
	snd->Scrub(range.Start(), range.End());
	return &ok;
}

//  =================================================
//  Property controller methods
//  =================================================

Value*
getPropertyController_cf(Value** arg_list, int count)
{
	// getPropertyController <value> <string_or_name>
	check_arg_count(getPropertyController, 2, count);
	Control* c;
	Value* prop = arg_list[1];
	if (is_string(prop)) prop = Name::intern(prop->to_string());
	type_check(prop, Name, _T("getPropertyController"));
	ParamDimension* dim;
	if ((c = MAXWrapper::get_max_prop_controller(arg_list[0]->to_reftarg(), prop, &dim)) != NULL)
		return MAXControl::intern(c, dim);
	else
		return &undefined;
}

Value*
setPropertyController_cf(Value** arg_list, int count)
{
	// setPropertyController <value> <string_or_name> <controller>
	check_arg_count(setPropertyController, 3, count);

	MAXControl* c = (MAXControl*)arg_list[2]; 
	Value* prop = arg_list[1];
	if (is_string(prop)) prop = Name::intern(prop->to_string());
	type_check(prop, Name, _T("setPropertyController"));

	if (!c->is_kind_of(class_tag(MAXControl)))
		throw TypeError (GetString(IDS_ASSIGN_NEEDS_CONTROLLER_GOT), arg_list[2]);
	if (MAXWrapper::set_max_prop_controller(arg_list[0]->to_reftarg(), prop, c)) {
		if (c->flags & MAX_CTRL_NO_DIM) {
			ParamDimension *dim;
			MAXWrapper::get_max_prop_controller(arg_list[0]->to_reftarg(), prop, &dim);
			c->dim = dim;
			c->flags &= ~MAX_CTRL_NO_DIM;
		}
		needs_redraw_set();
		return c;
	}
	else
		return &undefined;
}

Value*
isPropertyAnimatable_cf(Value** arg_list, int count)
{
	// isPropertyAnimatable <value> <string_or_name>
	check_arg_count(isPropertyAnimatable, 2, count);
	Value* prop = arg_list[1];
	if (is_string(prop)) prop = Name::intern(prop->to_string());
	type_check(prop, Name, _T("isPropertyAnimatable"));
	if (arg_list[0]->derives_from_MAXWrapper())
		return bool_result(((MAXWrapper*)(arg_list[0]))->is_max_prop_animatable(prop));
	else
		type_check(arg_list[0], MAXWrapper, _T("isPropertyAnimatable"));
	return &undefined;
}

//  =================================================
//  TransferReferences method
//  =================================================

Value*
replaceInstances_cf(Value** arg_list, int count)
{
	// replaceInstances <old_MAXWrapper> <new_MAXWrapper> transferCAs:<bool>
	check_arg_count_with_keys(replaceInstances, 2, count);

	ReferenceTarget* oldRT = arg_list[0]->to_reftarg();
	ReferenceTarget* newRT = arg_list[1]->to_reftarg();

	Value* dmy;
	BOOL transferCAs = bool_key_arg(transferCAs, dmy, FALSE);

	if (oldRT == NULL || (newRT != NULL && oldRT->SuperClassID() != newRT->SuperClassID()))
		throw RuntimeError (GetString(IDS_CONTROLLER_SUPERCLASSES_NOT_SAME));

	MAXScript_interface7->SuspendEditing(0x00FF);

	// Send out a warning that we're about to paste
	oldRT->NotifyDependents(FOREVER,0,REFMSG_PRENOTIFY_PASTE);

	// Transfer all the references from oldRT to newRT. Also transfer the CAs from oldRT to the new

	int i,j,k;
	Tab<RefMakerHandle> makers;

	// Save a list of reference makers that reference the old target.
	DependentIterator di(oldRT);
	ReferenceMaker* maker = NULL;
	while ((maker = di.Next()) != NULL) {
		makers.Append( 1, &maker, 10 );
	}
	// Now for each maker, replace the reference to oldRT with
	// a reference to this.
	bool first = true;
	for ( i = 0; i < makers.Count(); i++ ) 
	{
		RefMakerHandle makr = makers[i];
		j = makr->FindRef(oldRT);
		if(j >= 0) {
			if (makr->CanTransferReference(j) && !(makr->SuperClassID() == MAXSCRIPT_WRAPPER_CLASS_ID))
			{
				if (first)
				{
					first = false;
					ICustAttribContainer* cc_oldRT = oldRT->GetCustAttribContainer();
					if (newRT != NULL && cc_oldRT && transferCAs)
					{
						if (newRT->GetCustAttribContainer() == NULL) 
							newRT->AllocCustAttribContainer();
						ICustAttribContainer* cc_newRT = newRT->GetCustAttribContainer();
						int ca_count = cc_oldRT->GetNumCustAttribs();
						for (k = 0; k < ca_count; k++)
						{
							CustAttrib *ca = cc_oldRT->GetCustAttrib(k);
							cc_newRT->AppendCustAttrib(ca);
						}
						for (k = ca_count-1; k >= 0; k--)
							cc_oldRT->RemoveCustAttrib(k);
					}
				}
				makr->ReplaceReference(j,newRT);
			}
		}
	}

	// More notifications
	newRT->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	newRT->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	needs_redraw_set();

	MAXScript_interface7->ResumeEditing(0x00FF);

	return &ok;
}

//  =================================================
//  Playback methods
//  =================================================

int indexToSpeed[]={-4,-2,1,2,4};

Value*
setPlaybackSpeed(Value* val)
{
	int speed = val->to_int();
	range_check(speed, 1, 5, GetString(IDS_PLAYBACKSPEED_OUT_OF_RANGE));
	MAXScript_interface->SetPlaybackSpeed(indexToSpeed[speed-1]);
	return val;
}

Value*
getPlaybackSpeed()
{
	int speed = MAXScript_interface->GetPlaybackSpeed();
	for (int i = 0; i < 5; i++) 
		if (indexToSpeed[i] == speed) return Integer::intern(i+1);
	throw RuntimeError (GetString(IDS_BAD_GETPLAYBACKSPEED_RESULT), Integer::intern(speed));
}

//  =================================================
//  IK methods
//  =================================================

Value*
setPinNode_cf(Value** arg_list, int count)
{
	check_arg_count(setPinNode, 2, count);
	int res=0;
	INode* snode = get_valid_node(arg_list[0], setPinNode);
	if (arg_list[1] == &undefined)
		res = snode->SetProperty(PROPID_PINNODE,NULL);
	else
	{
		check_arg_count(setPinNode, 2, count);
		INode* node = get_valid_node((MAXNode*)arg_list[1], setPinNode);
		if (snode->GetTMController()->OKToBindToNode(node))
			res = snode->SetProperty(PROPID_PINNODE,node);
	}
	if (res)
	{
		needs_redraw_set();
		snode->GetTMController()->NodeIKParamsChanged();
		BOOL suspendPanel = snode->Selected();
		if (suspendPanel)
		{
			MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
			MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);
		}
	}
	return (res ? &true_value : &false_value);
}

Value*
getPinNode_cf(Value** arg_list, int count)
{
	check_arg_count(getPinNode, 1, count);
	INode* node = get_valid_node(arg_list[0], getPinNode);
	INode* bnode= (INode*)node->GetProperty(PROPID_PINNODE);
	return ((bnode == NULL) ? &undefined : MAXNode ::intern(bnode));
}

Value*
getPrecedence_cf(Value** arg_list, int count)
{
	check_arg_count(getPrecedence, 1, count);
	INode* node = get_valid_node(arg_list[0], getPrecedence);
	return Integer::intern(
		static_cast<int>(reinterpret_cast<INT_PTR>(node->GetProperty(PROPID_PRECEDENCE))));
}

Value*
setPrecedence_cf(Value** arg_list, int count)
{
	check_arg_count(setPrecedence, 2, count);
	INode* node = get_valid_node(arg_list[0], setPrecedence);
	int val=arg_list[1]->to_int();
	int res = node->SetProperty(PROPID_PRECEDENCE,&val);
	node->GetTMController()->NodeIKParamsChanged();
	BOOL suspendPanel = node->Selected();
	if (suspendPanel)
	{
		MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
		MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);
	}
	return (res ? &true_value : &false_value);
}

Value*
getIsTerminator_cf(Value** arg_list, int count)
{
	check_arg_count(getIsTerminator, 1, count);
	INode* node = get_valid_node(arg_list[0], getIsTerminator);
	return (node->TestAFlag(A_INODE_IK_TERMINATOR) ? &true_value : &false_value);
}

Value*
setIsTerminator_cf(Value** arg_list, int count)
{
	check_arg_count(setIsTerminator, 2, count);
	INode* node = get_valid_node(arg_list[0], setIsTerminator);
	BOOL val=arg_list[1]->to_bool();
	val ? node->SetAFlag(A_INODE_IK_TERMINATOR) : node->ClearAFlag(A_INODE_IK_TERMINATOR);
	node->GetTMController()->NodeIKParamsChanged();
	BOOL suspendPanel = node->Selected();
	if (suspendPanel)
	{
		MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
		MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);
	}
	return &ok;
}

Value*
getBindPos_cf(Value** arg_list, int count)
{
	check_arg_count(getBindPos, 1, count);
	INode* node = get_valid_node(arg_list[0], getBindPos);
	return (node->TestAFlag(A_INODE_IK_POS_PINNED) ? &true_value : &false_value);
}

Value*
setBindPos_cf(Value** arg_list, int count)
{
	check_arg_count(setBindPos, 2, count);
	INode* node = get_valid_node(arg_list[0], setBindPos);
	BOOL val=arg_list[1]->to_bool();
	val ? node->SetAFlag(A_INODE_IK_POS_PINNED) : node->ClearAFlag(A_INODE_IK_POS_PINNED);
	node->GetTMController()->NodeIKParamsChanged();
	return &ok;
}

Value*
getBindOrient_cf(Value** arg_list, int count)
{
	check_arg_count(getBindOrient, 1, count);
	INode* node = get_valid_node(arg_list[0], getBindOrient);
	return (node->TestAFlag(A_INODE_IK_ROT_PINNED) ? &true_value : &false_value);
}

Value*
setBindOrient_cf(Value** arg_list, int count)
{
	check_arg_count(setBindOrient, 2, count);
	INode* node = get_valid_node(arg_list[0], setBindOrient);
	BOOL val=arg_list[1]->to_bool();
	val ? node->SetAFlag(A_INODE_IK_ROT_PINNED) : node->ClearAFlag(A_INODE_IK_ROT_PINNED);
	node->GetTMController()->NodeIKParamsChanged();
	BOOL suspendPanel = node->Selected();
	if (suspendPanel)
	{
		MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
		MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);
	}
	return &ok;
}

//  =================================================
//  Mouse methods
//  =================================================

Value*
getMouseMode()
{
	return Integer::intern((int)MAXScript_interface->GetMouseManager()->GetMouseMode());
}

Value*
getMouseMAXPos()
{
	POINT pt;
	GetCursorPos( &pt );
	ViewExp* view = MAXScript_interface->GetActiveViewport();
	ScreenToClient( view->GetHWnd(), &pt );
	MAXScript_interface->ReleaseViewport(view);
	return new Point2Value(pt);
}

Value*
getMouseScreenPos()
{
	POINT pt;
	GetCursorPos( &pt );
	return new Point2Value(pt);
}

Value*
getMouseButtonStates()
{
	BitArray flags(3);
	int iflags = MAXScript_interface->GetMouseManager()->ButtonFlags();
	int i, j;
	for (i = 0, j = (1<<3); i < 3; i++, j= (j << 1)) 
		flags.Set(i,(iflags & j));
	return new BitArrayValue(flags);
}

Value*
getInMouseAbort()
{
	return (GetInMouseAbort()) ? &true_value : &false_value;
}

Value*
setInMouseAbort(Value* val)
{
	throw RuntimeError (_T("Cannot set mouse inAbort"));
}


//  =================================================
//  MtlBase methods
//  =================================================

Value*
getMTLMEditFlags_cf(Value** arg_list, int count)
{
	check_arg_count(getMTLMEditFlags, 1, count);
	MtlBase* mtl;
	if (arg_list[0]->is_kind_of(class_tag(MAXTexture)))
		mtl = (MtlBase*)arg_list[0]->to_texmap();
	else
	{
		mtl = arg_list[0]->to_mtl();
		if (mtl == NULL) throw ConversionError (arg_list[0], _T("material"));
	}
	BitArray flags(7);
	flags.Set(0,(mtl->TestMtlFlag(MTL_BEING_EDITED)));
	flags.Set(1,(mtl->TestMtlFlag(MTL_MEDIT_BACKGROUND)));
	flags.Set(2,(mtl->TestMtlFlag(MTL_MEDIT_BACKLIGHT)));

#ifndef NO_MTLEDITOR_VIDCOLORCHECK  // orb 01-08-2002
	flags.Set(3,(mtl->TestMtlFlag(MTL_MEDIT_VIDCHECK)));
#endif // NO_MTLEDITOR_VIDCOLORCHECK

	flags.Set(4,(mtl->TestMtlFlag(MTL_SUB_DISPLAY_ENABLED))); // LAM - 7/20/01 added these 3 for gMax - read/only
	flags.Set(5,(mtl->TestMtlFlag(MTL_TEX_DISPLAY_ENABLED)));
	flags.Set(6,(mtl->SupportTexDisplay()));
	return new BitArrayValue(flags);
}

Value*
setMTLMEditFlags_cf(Value** arg_list, int count)
{
	check_arg_count(setMTLMEditFlags, 2, count);
	MtlBase* mtl;
	if (arg_list[0]->is_kind_of(class_tag(MAXTexture)))
		mtl = (MtlBase*)arg_list[0]->to_texmap();
	else
	{
		mtl = arg_list[0]->to_mtl();
		if (mtl == NULL) throw ConversionError (arg_list[0], _T("material"));
	}
	BitArray flags = arg_list[1]->to_bitarray();
	flags.SetSize(7,TRUE);
//	mtl->SetMtlFlag(MTL_BEING_EDITED, flags[0]);	// read-only
	mtl->SetMtlFlag(MTL_MEDIT_BACKGROUND, flags[1]);
	mtl->SetMtlFlag(MTL_MEDIT_BACKLIGHT, flags[2]);

#ifndef NO_MTLEDITOR_VIDCOLORCHECK  // orb 01-08-2002
	mtl->SetMtlFlag(MTL_MEDIT_VIDCHECK, flags[3]);
#endif // NO_MTLEDITOR_VIDCOLORCHECK

	mtl->NotifyDependents(FOREVER,0,REFMSG_SEL_NODES_DELETED,NOTIFY_ALL,FALSE); // button update
	mtl->NotifyDependents(FOREVER,0,REFMSG_CHANGE,NOTIFY_ALL,FALSE); // slot update
	return &ok;
}

Value*
getMTLMeditObjType_cf(Value** arg_list, int count)
{
	check_arg_count(getMTLMeditObjType, 1, count);
	MtlBase* mtl;
	if (arg_list[0]->is_kind_of(class_tag(MAXTexture)))
		mtl = (MtlBase*)arg_list[0]->to_texmap();
	else
	{
		mtl = arg_list[0]->to_mtl();
		if (mtl == NULL) throw ConversionError (arg_list[0], _T("material"));
	}
	return Integer::intern((int)mtl->GetMeditObjType()+1);
}

Value*
setMTLMeditObjType_cf(Value** arg_list, int count)
{
	check_arg_count(setMTLMeditObjType, 2, count);
	MtlBase* mtl;
	if (arg_list[0]->is_kind_of(class_tag(MAXTexture)))
		mtl = (MtlBase*)arg_list[0]->to_texmap();
	else
	{
		mtl = arg_list[0]->to_mtl();
		if (mtl == NULL) throw ConversionError (arg_list[0], _T("material"));
	}
	int objType = arg_list[1]->to_int();
	range_check(objType, 1, 4, GetString(IDS_MEDIT_OBJECT_TYPE_INDEX_OUT_OF_RANGE));
	mtl->SetMeditObjType(objType-1);
	mtl->NotifyDependents(FOREVER,0,REFMSG_SEL_NODES_DELETED,NOTIFY_ALL,FALSE); // button update
	mtl->NotifyDependents(FOREVER,0,REFMSG_CHANGE,NOTIFY_ALL,FALSE); // slot update
	return &ok;
}

Value*
getMTLMeditTiling_cf(Value** arg_list, int count)
{
	check_arg_count(getMTLMeditTiling, 1, count);
#ifdef NO_MTLEDITOR_SAMPLEUVTILING // orb 01-08-2002
	NOT_SUPPORTED_BY_PRODUCT("GetMTLMeditTiling");
#else
	MtlBase* mtl;
	if (arg_list[0]->is_kind_of(class_tag(MAXTexture)))
		mtl = (MtlBase*)arg_list[0]->to_texmap();
	else
	{
		mtl = arg_list[0]->to_mtl();
		if (mtl == NULL) throw ConversionError (arg_list[0], _T("material"));
	}
	return Integer::intern((int)mtl->GetMeditTiling()+1);
#endif // NO_MTLEDITOR_SAMPLEUVTILING
}

Value*
setMTLMeditTiling_cf(Value** arg_list, int count)
{
	check_arg_count(setMTLMeditTiling, 2, count);

#ifdef NO_MTLEDITOR_SAMPLEUVTILING // orb 01-08-2002
	NOT_SUPPORTED_BY_PRODUCT("SetMTLMeditTiling");
#else

	MtlBase* mtl;
	if (arg_list[0]->is_kind_of(class_tag(MAXTexture)))
		mtl = (MtlBase*)arg_list[0]->to_texmap();
	else
	{
		mtl = arg_list[0]->to_mtl();
		if (mtl == NULL) throw ConversionError (arg_list[0], _T("material"));
	}
	int objType = arg_list[1]->to_int();
	range_check(objType, 1, 4, GetString(IDS_MEDIT_TILE_TYPE_INDEX_OUT_OF_RANGE));
	mtl->SetMeditTiling(objType-1);
	mtl->NotifyDependents(FOREVER,0,REFMSG_SEL_NODES_DELETED,NOTIFY_ALL,FALSE); // button update
	mtl->NotifyDependents(FOREVER,0,REFMSG_CHANGE,NOTIFY_ALL,FALSE); // slot update
	return &ok;
#endif // NO_MTLEDITOR_SAMPLEUVTILING
}

Value*
updateMTLInMedit_cf(Value** arg_list, int count)
{
	check_arg_count(updateMTLInMedit, 1, count);
	MtlBase* mtl;
	if (arg_list[0]->is_kind_of(class_tag(MAXTexture)))
		mtl = (MtlBase*)arg_list[0]->to_texmap();
	else
	{
		mtl = arg_list[0]->to_mtl();
		if (mtl == NULL) throw ConversionError (arg_list[0], _T("material"));
	}
	mtl->NotifyDependents(FOREVER,0,REFMSG_SEL_NODES_DELETED,NOTIFY_ALL,TRUE); // button update
	mtl->NotifyDependents(FOREVER,0,REFMSG_CHANGE,NOTIFY_ALL,TRUE); // slot update
	mtl->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED,NOTIFY_ALL,TRUE); // slot update
	return &ok;
}

//  =================================================
//  Array methods
//  =================================================

Value* qsort_user_compare_fn;
Value** qsort_user_compare_fn_args;
int qsort_user_compare_fn_arg_count;

static int
qsort_compare_fn(const void *p1, const void *p2)
{
	// finish building arg list and apply comparison fn
	qsort_user_compare_fn_args[0] = *(Value**)p1;
	qsort_user_compare_fn_args[1] = *(Value**)p2;
	return (qsort_user_compare_fn->apply(qsort_user_compare_fn_args, qsort_user_compare_fn_arg_count))->to_int();
}

Value*
qsort_cf(Value** arg_list, int count)
{
	//@doc@ qsort <array> <function> [start:<integer>] [end:<integer>] [user-defined key args passed to qsort_compare_fn]
	check_arg_count_with_keys(qsort, 2, count);
	type_check(arg_list[0], Array, "qsort");
	Array* theArray = (Array*)arg_list[0];
	if (theArray->size == 0) return &ok;
	qsort_user_compare_fn = arg_list[1];
	if (!is_function(qsort_user_compare_fn))
		throw RuntimeError (GetString(IDS_QSORT_ARG_NOT_FN), qsort_user_compare_fn);
	Value* tmp;
	int startIndex=int_key_arg(start, tmp, 1);
	range_check(startIndex, 1, theArray->size, GetString(IDS_QSORT_STARTINDEX_BAD))
	int endIndex=int_key_arg(end, tmp, theArray->size);
	range_check(endIndex, 1, theArray->size, GetString(IDS_QSORT_ENDINDEX_BAD))
	if (endIndex < startIndex)
		throw RuntimeError (GetString(IDS_QSORT_STARTENDINDEX_BAD));

	startIndex -= 1;
	int sortSize = endIndex - startIndex;

	// copy over any additional arguments that present. These will be passed to the comparison function
	// leave first two elements empty so that the p1 and p2 values in qsort_compare_fn can be put in them.
	qsort_user_compare_fn_args = (Value**)_alloca(sizeof(Value*) * count);
	for (int i = 2; i < count; i++) 
		qsort_user_compare_fn_args[i]=arg_list[i];
	qsort_user_compare_fn_arg_count=count;

	qsort(&theArray->data[startIndex], sortSize, sizeof(Value*), qsort_compare_fn);
	return &ok;
}

Value* bsearch_user_compare_fn;
Value** bsearch_user_compare_fn_args;
int bsearch_user_compare_fn_arg_count;

static int
bsearch_compare_fn(const void *p1, const void *p2)
{
	// finish building arg list and apply comparison fn
	bsearch_user_compare_fn_args[0] = *(Value**)p1;
	bsearch_user_compare_fn_args[1] = *(Value**)p2;
	return (bsearch_user_compare_fn->apply(bsearch_user_compare_fn_args, bsearch_user_compare_fn_arg_count))->to_int();
}

Value*
bsearch_cf(Value** arg_list, int count)
{
	//@doc@ bsearch <key> <array> <function> [start:<integer>] [end:<integer>] [user-defined key args passed to compare_fn]
	check_arg_count_with_keys(bsearch, 3, count);
	Array* theKey = (Array*)arg_list[0];
	type_check(arg_list[1], Array, "bsearch");
	Array* theArray = (Array*)arg_list[1];
	if (theArray->size == 0) return &undefined;
	bsearch_user_compare_fn = arg_list[2];
	if (!is_function(bsearch_user_compare_fn))
		throw RuntimeError (GetString(IDS_BSEARCH_ARG_NOT_FN), bsearch_user_compare_fn);
	Value* tmp;
	int startIndex=int_key_arg(start, tmp, 1);
	range_check(startIndex, 1, theArray->size, GetString(IDS_BSEARCH_STARTINDEX_BAD))
	int endIndex=int_key_arg(end, tmp, theArray->size);
	range_check(endIndex, 1, theArray->size, GetString(IDS_BSEARCH_ENDINDEX_BAD))
	if (endIndex < startIndex)
		throw RuntimeError (GetString(IDS_BSEARCH_STARTENDINDEX_BAD));

	startIndex -= 1;
	int searchSize = endIndex - startIndex;

	// copy over any additional arguments that present. These will be passed to the comparison function
	// leave first two elements empty so that the p1 and p2 values in compare_fn can be put in them.
	bsearch_user_compare_fn_arg_count=(count-1);
	bsearch_user_compare_fn_args = (Value**)_alloca(sizeof(Value*) * bsearch_user_compare_fn_arg_count);
	for (int i = 2; i < bsearch_user_compare_fn_arg_count; i++) 
		bsearch_user_compare_fn_args[i]=arg_list[i+1]; // 3rd function arg is 4th supplied arg, etc

	Value** resultPtr = (Value**)bsearch( &theKey, &theArray->data[startIndex], searchSize, sizeof(Value*), bsearch_compare_fn);
	if( resultPtr==NULL ) return &undefined;
	return *resultPtr;
}

Value*
insertItem_cf(Value** arg_list, int count)
{
	check_arg_count(insertItem, 3, count);
	if (is_tab_param(arg_list[1]))
		return ((MAXPB2ArrayParam*)arg_list[1])->insertItem(arg_list, count);
	type_check(arg_list[1], Array, "insertItem");
	Array* theArray = (Array*)arg_list[1];
	int index;
	Value* arg=arg_list[2];
	if (!is_number(arg) || (index = arg->to_int()) < 1)
		throw RuntimeError (GetString(IDS_ARRAY_INDEX_MUST_BE_VE_NUMBER_GOT), arg);

	int newSize=theArray->size+1;
	int oldSize=newSize;
	if (index > newSize) newSize=index;

	EnterCriticalSection(&Array::array_update);

	if (newSize > theArray->data_size)
	{
		/* grow array */
		theArray->data = (Value**)realloc(theArray->data, newSize * sizeof(Value*));
		theArray->data_size = newSize;
	}

	for (int i = theArray->size; i < newSize; i++)
		theArray->data[i] = &undefined;
	theArray->size = newSize;

	for (int i = newSize-1; i >= index; i--)
		theArray->data[i] = theArray->data[i-1];
	theArray->data[index - 1] = heap_ptr(arg_list[0]);

	LeaveCriticalSection(&Array::array_update);

	return &ok;
}

Value*
amin_cf(Value** arg_list, int count)
{
	Value* smallest;
	int i;
	if (count == 0) return &undefined;
	if (arg_list[0]->is_kind_of(class_tag(Array))) {
		check_arg_count(amin, 1, count);
		Array* theArray = (Array*)arg_list[0];
		if (theArray->size == 0) return &undefined;
		smallest=theArray->data[0];
		for (i = 1; i < theArray->size; i++)
			if (smallest->lt_vf(&theArray->data[i],1) == &false_value) 
				smallest = theArray->data[i];
	}
	else {
		smallest=arg_list[0];
		for (i = 1; i < count; i++)
			if (smallest->lt_vf(&arg_list[i],1) == &false_value) 
				smallest = arg_list[i];
	}
	return smallest;
}

Value*
amax_cf(Value** arg_list, int count)
{
	Value* largest;
	int i;
	if (count == 0) return &undefined;
	if (arg_list[0]->is_kind_of(class_tag(Array))) {
		check_arg_count(amin, 1, count);
		Array* theArray = (Array*)arg_list[0];
		if (theArray->size == 0) return &undefined;
		largest=theArray->data[0];
		for (i = 1; i < theArray->size; i++)
			if (largest->gt_vf(&theArray->data[i],1) == &false_value) 
				largest = theArray->data[i];
	}
	else {
		largest=arg_list[0];
		for (i = 1; i < count; i++)
			if (largest->gt_vf(&arg_list[i],1) == &false_value) 
				largest = arg_list[i];
	}
	return largest;
}

//  =================================================
//  Renderer methods
//  =================================================

int rendTimeType[]={REND_TIMESINGLE,REND_TIMESEGMENT,REND_TIMERANGE,REND_TIMEPICKUP};

Value*
get_RendTimeType()
{
	int res = MAXScript_interface->GetRendTimeType();
	for (int i = 0; i < 4; i++) 
		if (rendTimeType[i] == res) return Integer::intern(i+1);
	throw RuntimeError (GetString(IDS_BAD_RENDTIMETYPE_RESULT), Integer::intern(res));
}


Value*
set_RendTimeType(Value *val)
{
	int i = val->to_int();
	range_check(i, 1, 4, GetString(IDS_RENDTIMETYPE_OUT_OF_RANGE));
	MAXScript_interface->SetRendTimeType (rendTimeType[i-1]);
	return val;
}

Value*
get_RendStart()
{
	return MSTime::intern(MAXScript_interface->GetRendStart ());
}


Value*
set_RendStart(Value *val)
{
	MAXScript_interface->SetRendStart (val->to_timevalue() );
	return val;
}

Value*
get_RendEnd()
{
	return MSTime::intern(MAXScript_interface->GetRendEnd ());
}


Value*
set_RendEnd(Value *val)
{
	MAXScript_interface->SetRendEnd (val->to_timevalue() );
	return val;
}

Value*
get_RendNThFrame()
{
	return Integer::intern(MAXScript_interface->GetRendNThFrame ());
}


Value*
set_RendNThFrame(Value *val)
{
	int i = val->to_int();
	if (i < 1)
		throw RuntimeError (GetString(IDS_RENDNTHFRAME_OUT_OF_RANGE), val);
	MAXScript_interface->SetRendNThFrame (i);
	return val;
}

Value*
get_RendShowVFB()
{
	return MAXScript_interface->GetRendShowVFB() ? &true_value : &false_value;
}

#ifndef WEBVERSION
Value*
set_RendShowVFB(Value* val)
{
	MAXScript_interface->SetRendShowVFB(val->to_bool());
	return val;
}
#endif // WEBVERSION 

Value*
get_RendSaveFile()
{
	return MAXScript_interface->GetRendSaveFile() ? &true_value : &false_value;
}

Value*
set_RendSaveFile(Value* val)
{
	MAXScript_interface->SetRendSaveFile(val->to_bool());
	return val;
}

Value*
get_RendUseDevice()
{
	return MAXScript_interface->GetRendUseDevice() ? &true_value : &false_value;
}

#ifndef WEBVERSION
Value*
set_RendUseDevice(Value* val)
{
	MAXScript_interface->SetRendUseDevice(val->to_bool());
	return val;
}
#endif // WEBVERSION

Value*
get_RendUseNet()
{
	return MAXScript_interface->GetRendUseNet() ? &true_value : &false_value;
}

#ifndef WEBVERSION
Value*
set_RendUseNet(Value* val)
{
	MAXScript_interface->SetRendUseNet(val->to_bool());
	return val;
}
#endif // WEBVERSION

Value*
get_RendFieldRender()
{
	return MAXScript_interface->GetRendFieldRender() ? &true_value : &false_value;
}

#ifndef WEBVERSION
Value*
set_RendFieldRender(Value* val)
{
	MAXScript_interface->SetRendFieldRender(val->to_bool());
	return val;
}
#endif // WEBVERSION

Value*
get_RendColorCheck()
{
	return MAXScript_interface->GetRendColorCheck() ? &true_value : &false_value;
}

#ifndef WEBVERSION
Value*
set_RendColorCheck(Value* val)
{
	MAXScript_interface->SetRendColorCheck(val->to_bool());
	return val;
}
#endif // WEBVERSION

Value*
get_RendSuperBlack()
{
	return MAXScript_interface->GetRendSuperBlack() ? &true_value : &false_value;
}

#ifndef WEBVERSION
Value*
set_RendSuperBlack(Value* val)
{
	MAXScript_interface->SetRendSuperBlack(val->to_bool());
	return val;
}
#endif // WEBVERSION

Value*
get_RendHidden()
{
	return MAXScript_interface->GetRendHidden() ? &true_value : &false_value;
}

Value*
set_RendHidden(Value* val)
{
	MAXScript_interface->SetRendHidden(val->to_bool());
	return val;
}

Value*
get_RendForce2Side()
{
	return MAXScript_interface->GetRendForce2Side() ? &true_value : &false_value;
}

Value*
set_RendForce2Side(Value* val)
{
	MAXScript_interface->SetRendForce2Side(val->to_bool());
	return val;
}

Value*
get_RendAtmosphere()
{
	return MAXScript_interface->GetRendAtmosphere() ? &true_value : &false_value;
}

#ifndef WEBVERSION
Value*
set_RendAtmosphere(Value* val)
{
	MAXScript_interface->SetRendAtmosphere(val->to_bool());
	return val;
}
#endif // WEBVERSION

Value*
get_RendDitherTrue()
{
	return MAXScript_interface->GetRendDitherTrue() ? &true_value : &false_value;
}

Value*
set_RendDitherTrue(Value* val)
{
	MAXScript_interface->SetRendDitherTrue(val->to_bool());
	return val;
}

Value*
get_RendDither256()
{
	return MAXScript_interface->GetRendDither256() ? &true_value : &false_value;
}

Value*
set_RendDither256(Value* val)
{
	MAXScript_interface->SetRendDither256(val->to_bool());
	return val;
}

Value*
get_RendMultiThread()
{
	return MAXScript_interface->GetRendMultiThread() ? &true_value : &false_value;
}

Value*
set_RendMultiThread(Value* val)
{
	MAXScript_interface->SetRendMultiThread(val->to_bool());
	return val;
}

Value*
get_RendNThSerial()
{
	return MAXScript_interface->GetRendNThSerial() ? &true_value : &false_value;
}

Value*
set_RendNThSerial(Value* val)
{
	MAXScript_interface->SetRendNThSerial(val->to_bool());
	return val;
}

Value*
get_RendVidCorrectMethod()
{
	return Integer::intern(MAXScript_interface->GetRendVidCorrectMethod () + 1);
}

Value*
set_RendVidCorrectMethod(Value *val)
{
	int i = val->to_int();
	range_check(i, 1, 3, GetString(IDS_RENDVIDCORRECTMETHOD_OUT_OF_RANGE));
	MAXScript_interface->SetRendVidCorrectMethod (i-1);
	return val;
}

Value*
get_RendFieldOrder()
{
	return Integer::intern(MAXScript_interface->GetRendFieldOrder () + 1);
}

Value*
set_RendFieldOrder(Value *val)
{
	int i = val->to_int();
	range_check(i, 1, 2, GetString(IDS_RENDFIELDORDER_OUT_OF_RANGE));
	MAXScript_interface->SetRendFieldOrder (i-1);
	return val;
}

Value*
get_RendNTSC_PAL()
{
	return Integer::intern(MAXScript_interface->GetRendNTSC_PAL () + 1);
}

Value*
set_RendNTSC_PAL(Value *val)
{
	int i = val->to_int();
	range_check(i, 1, 2, GetString(IDS_RENDNTSC_PAL_OUT_OF_RANGE));
	MAXScript_interface->SetRendNTSC_PAL (i-1);
	return val;
}

Value* get_LockImageAspRatio()
{
	return MAXScript_interface7->GetLockImageAspRatio() ? &true_value : &false_value;
}

#ifndef WEBVERSION
Value* set_LockImageAspRatio(Value* val)
{
	MAXScript_interface7->SetLockImageAspRatio(val->to_bool());
	return val;
}
#endif // WEBVERSION

Value* get_ImageAspRatio()
{
	return Float::intern(MAXScript_interface7->GetImageAspRatio());
}

#ifndef WEBVERSION
Value* set_ImageAspRatio(Value* val)
{
	float v = val->to_float();
	MAXScript_interface7->SetImageAspRatio(v);
	return val;
}
#endif // WEBVERSION

Value* get_LockPixelAspRatio()
{
	return MAXScript_interface7->GetLockPixelAspRatio() ? &true_value : &false_value;
}

#ifndef WEBVERSION
Value* set_LockPixelAspRatio(Value* val)
{
	MAXScript_interface7->SetLockPixelAspRatio(val->to_bool());
	return val;
}
#endif // WEBVERSION

Value* get_PixelAspRatio()
{
	return Float::intern(MAXScript_interface7->GetPixelAspRatio());
}

#ifndef WEBVERSION
Value* set_PixelAspRatio(Value* val)
{
	float v = val->to_float();
	if (v > 0.0)
		MAXScript_interface7->SetPixelAspRatio(v);
	return val;
}
#endif // WEBVERSION

Value*
get_RendSuperBlackThresh()
{
	return Integer::intern(MAXScript_interface->GetRendSuperBlackThresh ());
}

#ifndef WEBVERSION
Value*
set_RendSuperBlackThresh(Value *val)
{
	int i = val->to_int();
	range_check(i, 1, 255, GetString(IDS_RENDSUPERBLACKTHRESH_OUT_OF_RANGE));
	MAXScript_interface->SetRendSuperBlackThresh (i);
	return val;
}
#endif // WEBVERSION

Value*
get_RendFileNumberBase()
{
	return Integer::intern(MAXScript_interface->GetRendFileNumberBase ());
}

Value*
set_RendFileNumberBase(Value *val)
{
	MAXScript_interface->SetRendFileNumberBase (val->to_int());
	return val;
}

Value*
get_RendPickupFrames()
{
	return new String (MAXScript_interface->GetRendPickFramesString());
}

Value*
set_RendPickupFrames(Value *val)
{
	TCHAR* inString = val->to_string();
	TCHAR okchars[] = "- 0123456789,";
	TCHAR badChar[] = " ";
	int n_ok = static_cast<int>(_tcslen(okchars));  
	int n_in = static_cast<int>(_tcslen(inString)); // SR DCAST64: Downcast to 2G limit.
	for (int j=0; j<n_in; j++)
		if (!strchr(okchars,inString[j]))
		{
			badChar[0] = inString[j];
			throw RuntimeError (_T("Invalid RendPickupFrames string character: "), badChar);
		}
		MAXScript_interface->GetRendPickFramesString() = inString;
		return val;
}

Value*
get_RendSimplifyAreaLights()
{
	return MAXScript_interface->GetRendSimplifyAreaLights() ? &true_value : &false_value;
}

Value*
set_RendSimplifyAreaLights(Value *val)
{
	MAXScript_interface->SetRendSimplifyAreaLights(val->to_bool());
	return val;
}

Value*
renderSceneDialog_Open_cf(Value** arg_list, int count)
{
	HoldSuspend suspend;
	check_arg_count(renderSceneDialog.Open, 0, count);
	MAXScript_interface7->OpenRenderDialog();
	return &ok;
}

Value*
renderSceneDialog_Cancel_cf(Value** arg_list, int count)
{
	HoldSuspend suspend;
	check_arg_count(renderSceneDialog.Cancel, 0, count);
	MAXScript_interface7->CancelRenderDialog();
	return &ok;
}

Value*
renderSceneDialog_Close_cf(Value** arg_list, int count)
{
	HoldSuspend suspend;
	check_arg_count(renderSceneDialog.Close, 0, count);
	MAXScript_interface7->CloseRenderDialog();
	return &ok;
}

Value*
renderSceneDialog_Commit_cf(Value** arg_list, int count)
{
	HoldSuspend suspend;
	check_arg_count(renderSceneDialog.Commit, 0, count);
	MAXScript_interface7->CommitRenderDialogParameters();
	return &ok;
}

Value*
renderSceneDialog_Update_cf(Value** arg_list, int count)
{
	HoldSuspend suspend;
	check_arg_count(renderSceneDialog.Update, 0, count);
	MAXScript_interface7->UpdateRenderDialogParameters();
	return &ok;
}

Value*
renderSceneDialog_isOpen_cf(Value** arg_list, int count)
{
	HoldSuspend suspend;
	check_arg_count(renderSceneDialog.isOpen, 0, count);
	return (MAXScript_interface7->RenderDialogOpen()) ? &true_value : &false_value;
}

Value*
get_DraftRenderer()
{
	return MAXRenderer::intern(MAXScript_interface->GetDraftRenderer());
}

Value*
set_DraftRenderer(Value *val)
{
	HoldSuspend suspend;
	Renderer* renderer = val->to_renderer();
	BOOL rendDialogOpen = MAXScript_interface7->RenderDialogOpen() && (MAXScript_interface->GetCurrentRenderSetting() == RS_Draft);
	if (rendDialogOpen)
		MAXScript_interface7->CloseRenderDialog();
	MAXScript_interface->AssignDraftRenderer(renderer);
	if (rendDialogOpen)
		MAXScript_interface7->OpenRenderDialog();
	return val;
}

Value*
get_ProductionRenderer()
{
	return MAXRenderer::intern(MAXScript_interface->GetProductionRenderer());
}

Value*
set_ProductionRenderer(Value *val)
{
	HoldSuspend suspend;
	Renderer* renderer = val->to_renderer();
	BOOL rendDialogOpen = MAXScript_interface7->RenderDialogOpen() && (MAXScript_interface->GetCurrentRenderSetting() == RS_Production);
	if (rendDialogOpen)
		MAXScript_interface7->CloseRenderDialog();
	MAXScript_interface->AssignProductionRenderer(renderer);
	if (rendDialogOpen)
		MAXScript_interface7->OpenRenderDialog();
	return val;
}

Value*
get_CurrentRenderer()
{
	return MAXRenderer::intern(MAXScript_interface->GetCurrentRenderer());
}

Value*
set_CurrentRenderer(Value *val)
{
	HoldSuspend suspend;
	Renderer* renderer = val->to_renderer();
	BOOL rendDialogOpen = MAXScript_interface7->RenderDialogOpen();
	if (rendDialogOpen)
		MAXScript_interface7->CloseRenderDialog();
	MAXScript_interface->AssignCurRenderer(renderer);
	if (rendDialogOpen)
		MAXScript_interface7->OpenRenderDialog();
	return val;
}

Value*
get_MEditRenderer()
{
	return MAXRenderer::intern(MAXScript_interface->GetMEditRenderer());
}

Value*
set_MEditRenderer(Value *val)
{
	HoldSuspend suspend;
	Renderer* renderer = val->to_renderer();
	BOOL rendDialogOpen = MAXScript_interface7->RenderDialogOpen();
	if (rendDialogOpen)
		MAXScript_interface7->CloseRenderDialog();
	MAXScript_interface->AssignMEditRenderer(renderer);
	if (rendDialogOpen)
		MAXScript_interface7->OpenRenderDialog();
	return val;
}

Value*
get_ReshadeRenderer()
{
	return MAXRenderer::intern(MAXScript_interface->GetRenderer(RS_IReshade));
}

Value*
set_ReshadeRenderer(Value *val)
{
	HoldSuspend suspend;
	Renderer* renderer = val->to_renderer();
	// can only use renderers that support a reshading interface
	if ( renderer->GetInterface(I_RENDER_ID) )
	{
		BOOL rendDialogOpen = MAXScript_interface7->RenderDialogOpen() && (MAXScript_interface->GetCurrentRenderSetting() == RS_IReshade);
		if (rendDialogOpen)
			MAXScript_interface7->CloseRenderDialog();
		MAXScript_interface->AssignRenderer(RS_IReshade, renderer);
		if (rendDialogOpen)
			MAXScript_interface7->OpenRenderDialog();
	}
	return val;
}

Value* get_MEditRendererLocked()
{
	return (MAXScript_interface->GetMEditRendererLocked() ? &true_value : &false_value);
}

Value* set_MEditRendererLocked(Value* val)
{
	HoldSuspend suspend;
	BOOL b = val->to_bool();

	BOOL rendDialogOpen = MAXScript_interface7->RenderDialogOpen();
	if (rendDialogOpen)
		MAXScript_interface7->CloseRenderDialog();
	MAXScript_interface->SetMEditRendererLocked(b != 0);
	if (rendDialogOpen)
		MAXScript_interface7->OpenRenderDialog();

	return val;
}

//  =================================================
//  Controller methods
//  =================================================

Value*
getXYZControllers_cf(Value** arg_list, int count)
{
	check_arg_count(getXYZControllers, 1, count);
	Control *c = arg_list[0]->to_controller();
	if(!c) return &undefined; 	
	one_typed_value_local(Array* result);
	vl.result = new Array (3);
	Control *c1 = c->GetXController();
	vl.result->append(c1 ? MAXControl::intern(c1) : &undefined);
	c1 = c->GetYController();
	vl.result->append(c1 ? MAXControl::intern(c1) : &undefined);
	c1 = c->GetZController();
	vl.result->append(c1 ? MAXControl::intern(c1) : &undefined);
	return_value(vl.result);
}

Value*
displayControlDialog_cf(Value** arg_list, int count)
{
	check_arg_count(displayControlDialog, 2, count);
	arg_list[0]->to_controller()->EditTrackParams(MAXScript_time(), 
		((MAXControl*)arg_list[0])->dim,
		arg_list[1]->to_string(),
		MAXScript_interface->GetMAXHWnd(), 
		(IObjParam*)MAXScript_interface, 
		EDITTRACK_BUTTON | EDITTRACK_TRACK);
	return &ok;
}

//  =================================================
//  Mesh/Spline point controller methods
//  =================================================

// EditTriObject References:
#define ET_MASTER_CONTROL_REF  0
#define ET_VERT_BASE_REF 1

Value*
getPointControllers_cf(Value** arg_list, int count)
{
	check_arg_count(getPointControllers, 1, count);
	INode* node = get_valid_node(arg_list[0], getPointControllers);
	Object* obj = Get_Object_Or_XRef_BaseObject(node->GetObjectRef());

	one_typed_value_local(Array* result);

	if (obj->ClassID() == Class_ID(EDITTRIOBJ_CLASS_ID, 0)) {
		// mesh
		TriObject *em = (TriObject*)obj;
		int size = em->NumRefs()-1;
		vl.result = new Array (size);
		for (int i = 0; i < size; i++) {
			Control *c1 = (Control*)em->GetReference(i+ET_VERT_BASE_REF);
			vl.result->append(c1 ? MAXControl::intern(c1) : &undefined);
		}
	}
	else if (obj->ClassID() == splineShapeClassID || obj->ClassID() == Class_ID(SPLINE3D_CLASS_ID,0)) {
		// spline
		Tab<Control*> *cont;
		cont=&((SplineShape*)obj)->cont;
		int size = cont->Count();
		vl.result = new Array (size);
		for (int i = 0; i < size; i++) {
			Control *c1 = (*cont)[i];
			vl.result->append(c1 ? MAXControl::intern(c1) : &undefined);
		}
	}
	else 
		throw RuntimeError (GetString(IDS_CANNOT_GET_POINT_CONTROLLER), node->GetName());

	return_value(vl.result);
}

Value*
getPointController_cf(Value** arg_list, int count)
{
	if (count == 0) throw ArgCountError (_T("getPointController"), 2, count);
	INode* node = get_valid_node(arg_list[0], getPointController);
	Object* obj = Get_Object_Or_XRef_BaseObject(node->GetObjectRef());


	if (obj->ClassID() == Class_ID(EDITTRIOBJ_CLASS_ID, 0)) {
		// mesh
		check_arg_count(getPointController, 2, count);
		TriObject *em = (TriObject*)obj;
		int pointIndex=arg_list[1]->to_int();
		range_check(pointIndex, 1, em->GetMesh().getNumVerts(), GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE))
		Control *c1 = (Control*)em->GetReference(pointIndex-1+ET_VERT_BASE_REF);
		return (c1 ? MAXControl::intern(c1) : &undefined);
	}
	else if (obj->ClassID() == splineShapeClassID || obj->ClassID() == Class_ID(SPLINE3D_CLASS_ID,0)) {
		// spline
		check_arg_count(getPointController, 3, count);
		Tab<Control*> *cont;
		cont=&((SplineShape*)obj)->cont;
		int splineIndex=arg_list[1]->to_int();
		int vertIndex=arg_list[2]->to_int();
		BezierShape* s = set_up_spline_access(node, splineIndex);
		Spline3D* s3d = s->splines[splineIndex-1];
		int pointCount = 3*s3d->KnotCount();
		range_check(vertIndex, 1, pointCount, GetString(IDS_SPLINE_VERTEX_INDEX_OUT_OF_RANGE))
		int pointIndex = s->GetVertIndex(splineIndex-1,vertIndex-1)+1;
		if (pointIndex <= cont->Count()) {
			Control *c1 = (*cont)[pointIndex-1];
			return (c1 ? MAXControl::intern(c1) : &undefined);
		}
		else
			return &undefined;
	}
	else 
		throw RuntimeError (GetString(IDS_CANNOT_GET_POINT_CONTROLLER), node->GetName());

}

//  =================================================
//  Value type methods
//  =================================================

Value*
isStructDef_cf(Value** arg_list, int count)
{
	check_arg_count(isStructDef, 1, count);
	return (is_structdef(arg_list[0]) ? &true_value : &false_value);
}

Value*
isStruct_cf(Value** arg_list, int count)
{
	check_arg_count(isStruct, 1, count);
	return ((is_struct(arg_list[0])) ? &true_value : &false_value);
}

Value*
isController_cf(Value** arg_list, int count)
{
	check_arg_count(isController, 1, count);
	return (is_controller(arg_list[0]) ? &true_value : &false_value);
}

//  =================================================
//  Command Mode methods
//  =================================================

Value*
getCommandMode()
{
	int state = MAXScript_interface->GetCommandMode()->Class();
	def_commandmode_types();
	return GetName(commandmodeTypes, elements(commandmodeTypes), state, Integer::intern(state));
}

Value*
setCommandMode(Value* val)
{
	def_stdcommandmode_types();
	int state = GetID(stdcommandmodeTypes, elements(stdcommandmodeTypes), val);
	MAXScript_interface->SetStdCommandMode(state);
	return val;
}

Value*
getCommandModeID()
{
	int iID = MAXScript_interface->GetCommandMode()->ID();
	return Integer::intern(iID);
}

Value*
getAxisConstraints()
{
	int state = MAXScript_interface->GetAxisConstraints();
	def_axisconstraint_types();
	return GetName(axisconstraintTypes, elements(axisconstraintTypes), state, &undefined);
}

Value*
setAxisConstraints(Value* val)
{
	def_axisconstraint_types();
	int state = GetID(axisconstraintTypes, elements(axisconstraintTypes), val);
	MAXScript_interface->SetAxisConstraints(state);
	return val;
}

Value* 
GetToolBtnState_cf(Value** arg_list, int count)
{
	check_arg_count(GetToolBtnState, 1, count);
	def_toolbtn_types();
	BOOL state = MAXScript_interface->GetToolButtonState(
			GetID(toolbtnTypes, elements(toolbtnTypes), arg_list[0]));
	return ((state) ? &true_value : &false_value);
}

//  =================================================
//  AutoBackup methods
//  =================================================

Value*
getAutoBackupEnabled()
{
	return (MAXScript_interface->AutoBackupEnabled() ? &true_value : &false_value);
}

Value*
setAutoBackupEnabled(Value* val)
{
	MAXScript_interface->EnableAutoBackup(val->to_bool());
	return val;
}

Value*
getAutoBackupTime()
{
	return Float::intern(MAXScript_interface->GetAutoBackupTime());
}

Value*
setAutoBackupTime(Value* val)
{
	float t = val->to_float();
	if (t < 0.01f) {
		t = 0.01f;
		val = Float::intern(t);
	}
	MAXScript_interface->SetAutoBackupTime(t);
	return val;
}

//  =================================================
//  Quat <-> Euler methods
//  =================================================

typedef int EAOrdering[3];
static EAOrdering EAOrderings[] = {
	{0,1,2},
	{0,2,1},
	{1,2,0},
	{1,0,2},
	{2,0,1},
	{2,1,0},
	{0,1,0},
	{1,2,1},
	{2,0,2},
};

Value*
eulerToQuat_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(eulerToQuat, 1, count);
	Value* tmp;
	int order = int_key_arg(order, tmp, 1);
	range_check(order, 1, 12, GetString(IDS_AXIS_ORDER_INDEX_OUT_OF_RANGE));
	if (bool_key_arg(sliding, tmp, FALSE)) order += 16;
	if (!is_eulerangles(arg_list[0])) throw ConversionError (arg_list[0], _T("EulerAngles"));
//	Quat res;
//	EulerToQuat((float*)arg_list[0], res, (order-1));
	EulerAnglesValue* eulerAng = (EulerAnglesValue*)arg_list[0];
	Matrix3 tm(1);
	for (int i=0; i<3; i++) {
		switch (EAOrderings[order-1][i]) {
				case 0: tm.RotateX(eulerAng->angles[i]); break;
				case 1: tm.RotateY(eulerAng->angles[i]); break;
				case 2: tm.RotateZ(eulerAng->angles[i]); break;
		}
	}
	return new QuatValue(Quat(tm));
}

Value*
quatToEuler_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(quatToEuler, 1, count);
	Value* tmp;
	int order = int_key_arg(order, tmp, 1);
	range_check(order, 1, 12, GetString(IDS_AXIS_ORDER_INDEX_OUT_OF_RANGE));
	if (bool_key_arg(sliding, tmp, FALSE)) order += 16;
//	QuatToEuler(arg_list[0]->to_quat(), res, (order-1));
	Quat v = arg_list[0]->to_quat();
	float *res = new float [3];
	Matrix3 tm;
	v.MakeMatrix(tm);
	MatrixToEuler(tm, res, order-1);
	Value* result = new EulerAnglesValue(res[0], res[1], res[2]);
	delete [] res;
	return result;
}


//  =================================================
//  UI Items
//  =================================================


/* -------------------- MultiListBoxControl  ------------------- */
/*
rollout test "test"
(MultiListBox mlb "MultiListBox" items:#("A","B","C") selection:#(1,3)
on mlb selected val do format "selected: % - %\n" val mlb.selection[val]
on mlb doubleclicked val do format "doubleclicked: % - %\n" val mlb.selection[val]
on mlb selectionEnd do format "selectionEnd: %\n" mlb.selection
)
rof=newrolloutfloater "tester" 200 300
addrollout test rof
test.mlb.items
test.mlb.selection=1
test.mlb.selection=#(1,3)
test.mlb.selection=#{}

*/
class MultiListBoxControl;
visible_class (MultiListBoxControl)

class MultiListBoxControl : public RolloutControl
{
public:
	Array*		item_array;
	int			lastSelection;
	BitArray*	selection;

	// Constructor
	MultiListBoxControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
			: RolloutControl(name, caption, keyparms, keyparm_count)
	{
		tag = class_tag(MultiListBoxControl); 
		item_array = NULL;
		selection = new BitArray (0);
	}
	~MultiListBoxControl() { delete selection;}

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
						{ return new MultiListBoxControl (name, caption, keyparms, keyparm_count); }


	classof_methods (MultiListBoxControl, RolloutControl);

	// Garbage collection
	void		collect() {delete this; }

	// Print out the internal name of the control to MXS
	void		sprin1(CharStream* s) { s->printf(_T("MultiListBoxControl:%s"), name->to_string()); }

	// Return the window class name 
	LPCTSTR		get_control_class() { return _T("MULTILISTBOX"); }

	int			num_controls() { return 2; }

	// Top-level call for changing rollout layout. We don't process this.
	void		compute_layout(Rollout *ro, layout_data* pos) { }

	void gc_trace()
	{
		RolloutControl::gc_trace();

		if (item_array && item_array->is_not_marked())
			item_array->gc_trace();
	}

	void compute_layout(Rollout *ro, layout_data* pos, int& current_y)
	{
		setup_layout(ro, pos, current_y);

		TCHAR*	label_text = caption->eval()->to_string();
		int label_height = (strlen(label_text) != 0) ? ro->text_height + SPACING_BEFORE - 2 : 0;
		Value* height_obj;
		int item_count = int_control_param(height, height_obj, 10);
		int lb_height = item_count * ro->text_height + 7;

		process_layout_params(ro, pos, current_y);
		pos->height = label_height + lb_height;
		current_y = pos->top + pos->height;
	}

	// Add the control itself to a rollout window
	void add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
	{
		HWND	label, list_box;
		int		left, top, width, height;
		SIZE	size;
		TCHAR*	label_text = caption->eval()->to_string();

		// add 2 controls for a list box: a static label & the list
		parent_rollout = ro;
		control_ID = next_id();
		WORD label_id = next_id();
		WORD list_box_id = control_ID;

		int label_height = (strlen(label_text) != 0) ? ro->text_height + SPACING_BEFORE - 2 : 0;
		Value* height_obj;
		int item_count = int_control_param(height, height_obj, 10);
		int lb_height = item_count * ro->text_height + 6;

		layout_data pos;
		compute_layout(ro, &pos, current_y);

		// place optional label
		left = pos.left;
		// LAM - defect 298613 - not creating the caption HWND was causing problems (whole screen redrawn
		// when control moved, setting caption text set wrong HWND). Now always create.
//		if (label_height != 0)
//		{
		DLGetTextExtent(ro->rollout_dc, label_text, &size);   
		width = min(size.cx, pos.width); height = ro->text_height;
		top = pos.top; 
		label = CreateWindow(_T("STATIC"),
							 label_text,
							 WS_VISIBLE | WS_CHILD | WS_GROUP,
							 left, top, width, height,    
							 parent, (HMENU)label_id, hInstance, NULL);
//		}

		// place list box
		top = pos.top + label_height;
		width = pos.width; height = lb_height;
		list_box = CreateWindowEx(WS_EX_CLIENTEDGE, 
								_T("LISTBOX"),
								_T(""),
								LBS_EXTENDEDSEL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | WS_BORDER | WS_VSCROLL | WS_VISIBLE | WS_CHILD | WS_TABSTOP,
								left, top, width, height,    
								parent, (HMENU)list_box_id, hInstance, NULL);

		SendMessage(label, WM_SETFONT, (WPARAM)ro->font, 0L);
		SendMessage(list_box, WM_SETFONT, (WPARAM)ro->font, 0L);

		DLSetWindowLongPtr(list_box, this);

		// fill up the list
		if (ro->init_values)
		{
			one_value_local(selection);
			item_array = (Array*)control_param(items);
			if (item_array == (Array*)&unsupplied)
				item_array = new Array (0);
			type_check(item_array, Array, _T("items:"));
			vl.selection = control_param(selection);
			if (vl.selection != &unsupplied) 
				ValueToBitArray(vl.selection, *selection, -1);
			selection->SetSize(item_array->size,1);
			pop_value_locals();
		}

		// add items from array to combobox and compute widest entry & set minwidth
		int max_width = 0;
		SendMessage(list_box, LB_RESETCONTENT, 0, 0);
		for (int i = 0; i < item_array->size; i++)
		{
			TCHAR* item = item_array->data[i]->to_string();
			SendMessage(list_box, LB_ADDSTRING, 0, (LPARAM)item);
			DLGetTextExtent(ro->rollout_dc, item, &size);   
			if (size.cx > max_width) max_width = size.cx;
			SendMessage(list_box, LB_SETSEL, (*selection)[i], i);
		}
	}

	BOOL handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_COMMAND)
		{
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
				int numSel = SendMessage(list_box,LB_GETSELCOUNT,0,0);
				int* selArray = new int[numSel];
				SendMessage(list_box,LB_GETSELITEMS,numSel,(LPARAM)selArray);
				lastSelection = SendMessage(list_box,LB_GETCURSEL,0,0);
				BitArray newSelection (item_array->size);
				for (int i = 0; i < numSel; i++) 
					newSelection.Set(selArray[i]);
				one_value_local(sel);
				BOOL selChanged = FALSE;
				for (int i = 0; i < item_array->size; i++) {
					if (newSelection[i] != (*selection)[i]) {
						selection->Set(i,newSelection[i]);
						vl.sel = Integer::intern(i + 1);
						call_event_handler(ro, n_selected, &vl.sel, 1);
						selChanged = TRUE;
					}
				}
				pop_value_locals();
				delete [] selArray;
				if (selChanged) call_event_handler(ro, n_selectionEnd, NULL, 0);
			}
			else if (HIWORD(wParam) == LBN_DBLCLK)
			{
				one_value_local(sel);
				vl.sel = Integer::intern(lastSelection + 1);
				call_event_handler(ro, n_doubleClicked, &vl.sel, 1); 
				pop_value_locals();
			}
			return TRUE;
		}
		else if (message == WM_CONTEXTMENU)
		{
			HashTable* event_handlers= (HashTable*)ro->handlers->get(n_rightClick);
			Value*   handler = NULL;
			if (event_handlers && ((handler = event_handlers->get(name)) != NULL))
			{
				two_value_locals(arg,ehandler);
				vl.ehandler = handler->eval_no_wrapper();
				if (is_maxscriptfunction(vl.ehandler) && ((MAXScriptFunction*)vl.ehandler)->parameter_count == 1)
				{
					int x = HIWORD(lParam);
					int y = LOWORD(lParam);

					RECT rect;
					HWND  hwnd = GetDlgItem(ro->page, control_ID);
					GetWindowRect(hwnd, &rect);
					x -= rect.top;
					y -= rect.left;

					LRESULT item = SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(y, x)); 
					if (HIWORD(item) == 0)//inside client area
					{
						vl.arg = Integer::intern(item + 1);
						call_event_handler(ro, n_rightClick, &vl.arg, 1);
					}
				}
				else
					call_event_handler(ro, n_rightClick, NULL, 0);
				pop_value_locals();
			}
			return TRUE;
		}
		return FALSE;
	}

	Value* set_property(Value** arg_list, int count)
	{
		Value* val = arg_list[0];
		Value* prop = arg_list[1];

		if (prop == n_text || prop == n_caption)
		{
			TCHAR* text = val->to_string();
			caption = val->get_heap_ptr();
			if (parent_rollout != NULL && parent_rollout->page != NULL)
				set_text(text, GetDlgItem(parent_rollout->page, control_ID + 1), n_left);
		}
		else if (prop == n_items)
		{
			type_check(val, Array, _T("items:"));
			item_array = (Array*)val;
			selection->SetSize(item_array->size,1);
			if (parent_rollout != NULL && parent_rollout->page != NULL)
			{
				HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
				SendMessage(list_box, LB_RESETCONTENT, 0, 0);
				for (int i = 0; i < item_array->size; i++) {
					SendMessage(list_box, LB_ADDSTRING, 0, (LPARAM)item_array->data[i]->to_string());
					SendMessage(list_box, LB_SETSEL, (*selection)[i], i);
				}

			}
		}
		else if (prop == n_selection)
		{
			selection->ClearAll();
			ValueToBitArray(val, *selection, -1);
			selection->SetSize(item_array->size,1);
			if (parent_rollout != NULL && parent_rollout->page != NULL)
			{
				HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
				for (int i = 0; i < item_array->size; i++) 
					SendMessage(list_box, LB_SETSEL, (*selection)[i], i);
			}
		}
		else if(prop == n_height)
		{
			if (parent_rollout != NULL && parent_rollout->page != NULL)
			{
				HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
				int newHeight = val->to_int(); 
				RECT rect;
				GetWindowRect(list_box, &rect);
				MapWindowPoints(NULL, parent_rollout->page, (LPPOINT)&rect, 2);
				SetWindowPos(list_box, NULL, rect.left, rect.top, rect.right-rect.left, newHeight, SWP_NOZORDER);
			}
		}
		else if(prop == n_width)
		{
			if (parent_rollout != NULL && parent_rollout->page != NULL)
			{
				HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
				int newWidth = val->to_int(); 
				RECT rect;
				GetWindowRect(list_box, &rect);
				MapWindowPoints(NULL, parent_rollout->page, (LPPOINT)&rect, 2);
				SetWindowPos(list_box, NULL, rect.left, rect.top, newWidth, rect.bottom-rect.top, SWP_NOZORDER);
			}
		}
		else
			return RolloutControl::set_property(arg_list, count);

		return val;
	}

	Value* get_property(Value** arg_list, int count)
	{
		Value* prop = arg_list[0];

		if (prop == n_items)
			return item_array;
		else if (prop == n_selection)
			return new BitArrayValue(*selection);
		else if (prop == n_doubleClicked || prop == n_selected || prop == n_selectionEnd)
			return get_wrapped_event_handler(prop);
		else if(prop == n_height)
		{
			if (parent_rollout != NULL && parent_rollout->page != NULL)
			{
				HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
				RECT rect;
				GetWindowRect(list_box, &rect);
				return Integer::intern(rect.bottom-rect.top);
			}
			else
				return &undefined;
		}
		else if(prop == n_width)
		{
			if (parent_rollout != NULL && parent_rollout->page != NULL)
			{
				HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
				RECT rect;
				GetWindowRect(list_box, &rect);
				return Integer::intern(rect.right-rect.left);
			}
			else
				return &undefined;
		}
		else
			return RolloutControl::get_property(arg_list, count);
	}

	void set_enable()
	{
		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			// set combox enable
			EnableWindow(GetDlgItem(parent_rollout->page, control_ID), enabled);
			// set caption enable
			HWND ctrl = GetDlgItem(parent_rollout->page, control_ID + 1);
			if (ctrl)
				EnableWindow(ctrl, enabled);
		}
	}
};

visible_class_instance (MultiListBoxControl, "MultiListBoxControl");

//  =================================================
//  Bitmap File Open/Save Filename Dialogs
//  =================================================

Value* 
getBitmapOpenFileName_cf(Value** arg_list, int count)
{
//	getBitmapOpenFileName caption:<title> filename:<seed_filename_string>
	check_arg_count_with_keys(getBitmapOpenFileName, 0, count);
	BitmapInfo bi;
	Value* cap = key_arg(caption);
	TCHAR*  cap_str = (cap != &unsupplied) ? cap->to_string() : NULL ;
	Value* fn = key_arg(filename);
	if (fn != &unsupplied) 
	{
		TSTR fname = fn->to_filename();
		for (TCHAR* e = fname; *e; e++) if (*e == _T('/')) *e = _T('\\'); // xlate '/' to '\'
		bi.SetName(fname);
	}

	if (TheManager->SelectFileInput(&bi, GetActiveWindow(), cap_str))
		return new String((TCHAR*)bi.Name());
	else
		return &undefined;
}

Value* 
getBitmapSaveFileName_cf(Value** arg_list, int count)
{
//	getBitmapSaveFileName caption:<title> filename:<seed_filename_string>
	check_arg_count_with_keys(getBitmapSaveFileName, 0, count);
	BitmapInfo bi;
	Value* cap = key_arg(caption);
	TCHAR*  cap_str = (cap != &unsupplied) ? cap->to_string() : NULL ;
	Value* fn = key_arg(filename);
	if (fn != &unsupplied) 
	{
		TSTR fname = fn->to_filename();
		for (TCHAR* e = fname; *e; e++) if (*e == _T('/')) *e = _T('\\'); // xlate '/' to '\'
		bi.SetName(fname);
	}
	if (TheManager->SelectFileOutput(&bi, GetActiveWindow(), cap_str))
		return new String((TCHAR*)bi.Name());
	else
		return &undefined;
}

Value* 
doesFileExist_cf(Value** arg_list, int count)
{
//	doesFileExist <filename_string>
	check_arg_count(doesFileExist, 1, count);
	return (DoesFileExist(arg_list[0]->to_filename()) ? &true_value : &false_value);
}

//  =================================================
//  MAX File Open/Save Filename Dialogs
//  =================================================

Value* 
getMAXOpenFileName_cf(Value** arg_list, int count)
{
//	getMAXOpenFileName filename:<seed_filename_string> dir:<seed_directory_string>
	check_arg_count_with_keys(getMAXOpenFileName, 0, count);
	TSTR fname;
	TSTR* defFile = NULL;
	TSTR* defDir = NULL;
	Value* dfn = key_arg(filename);
	if (dfn != &unsupplied) 
	{
		TSTR  defFileName = dfn->to_filename();
		AssetId assetId;
		if (IAssetManager::GetInstance()->StringToAssetId(defFileName, assetId))
			defFileName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();
		defFile = new TSTR (defFileName);
	}
	Value* ddir = key_arg(dir);
	if (ddir != &unsupplied) 
	{
		TSTR  defDirName = ddir->to_filename();
		AssetId assetId;
		if (IAssetManager::GetInstance()->StringToAssetId(defDirName, assetId))
			defDirName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();

		defDir = new TSTR (defDirName);
		for (TCHAR* e = defDir->data(); *e; e++) if (*e == _T('/')) *e = _T('\\'); // xlate '/' to '\'
	}

	Value* res;
	if (MAXScript_interface7->DoMaxFileOpenDlg(fname, defDir, defFile))
		res = new String(fname);
	else
		res = &undefined;
	if (defFile) delete defFile;
	if (defDir) delete defDir;
	return res;
}


Value* 
getMAXSaveFileName_cf(Value** arg_list, int count)
{
//	getMAXSaveFileName filename:<seed_filename_string>
	check_arg_count_with_keys(getMAXSaveFileName, 0, count);
	TSTR fileName;
	Value* dfn = key_arg(filename);
	if (dfn != &unsupplied) 
	{
		fileName = dfn->to_filename();
		AssetId assetId;
		if (IAssetManager::GetInstance()->StringToAssetId(fileName, assetId))
			fileName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();
		for (TCHAR* e = fileName; *e; e++) if (*e == _T('/')) *e = _T('\\'); // xlate '/' to '\'
	}
	if (MAXScript_interface7->DoMaxFileSaveAsDlg(fileName, FALSE))
		return new String(fileName);
	else
		return &undefined;
}

//  =================================================
//  Miscellaneous Stuff
//  =================================================

Value* 
getCurNameSelSet_cf(Value** arg_list, int count)
{
//	getCurNameSelSet()
	check_arg_count(getCurNameSelSet, 0, count);
	const int BUFLEN = 17;
	TCHAR buf[BUFLEN];
	GetWindowText( GetCUIFrameMgr()->GetItemHwnd(50037), buf, BUFLEN );
	return new String(buf);
}

Value* 
isDebugBuild_cf(Value** arg_list, int count)
{
//	isDebugBuild()
#ifdef _DEBUG
	return &true_value;
#endif
	return &false_value;

}

Value*
get_superclasses()
{
	one_typed_value_local(Array* result);
	vl.result = new Array (maxwrapper_class.getNumSuperclasses ());
	for (int i = 0; i < maxwrapper_class.getNumSuperclasses (); i++)
		vl.result->append(maxwrapper_class.getSuperclass (i));
	return_value(vl.result);
}

Value*
setFocus_cf(Value** arg_list, int count)
{
	check_arg_count(SetFocus, 1, count);
	if (is_rolloutcontrol(arg_list[0]))
	{
		RolloutControl* roc = (RolloutControl*)arg_list[0];
		return roc->set_focus() ? &true_value : &false_value;
//		if (roc->parent_rollout && roc->parent_rollout->page)
//			SetFocus(roc->parent_rollout->page);
	}
	else if (is_rolloutfloater(arg_list[0]))
	{
		RolloutFloater* rof = (RolloutFloater*)arg_list[0];
		if (rof->window)
		{
			SetFocus(rof->window);
			return &true_value;
		}
	}
	else if (is_rollout(arg_list[0]))
	{
		Rollout* ro = (Rollout*)arg_list[0];
		if (ro->page)
		{
			SetFocus(ro->page);
			return &true_value;
		}
	}
	else
		throw RuntimeError (GetString(IDS_SETFOCUS_ARG_TYPE_ERROR), arg_list[0]);
	return &false_value;
}

//  =================================================
//  Value hasher
//  =================================================

// methods for calculating hash value for various class values

#define HASHPRECISION 6 

// form the hash value for string s
inline void Hash(TCHAR* s, UINT& hashVal)
{
	for (; *s; s++)
		hashVal = *s + 31 * hashVal;
	return;
}

// form the hash value for integer i
inline void Hash(int i, UINT& hashVal)
{
	hashVal = i + 31 * hashVal;
	return;
}

// decompose a float into integer and exponent given precision
inline void decomposeFloat(float v, int& mantissa, int& exponent)
{
	if (v == 0.0f) {
		exponent = mantissa = 0;
		return;
	}
	BOOL neg = FALSE;
	if (v < 0.0f) {
		neg = TRUE;
		v = fabsf(v);
	}
	exponent = int(log10f (v));
	if (v >= 1.0f) exponent += 1;
	mantissa = int(v*(float(10^(HASHPRECISION-exponent)) +0.5f));
	return;
}

// form the hash value for float f
inline void Hash(float f, UINT& hashVal)
{
	int mantissa, exponent;
	decomposeFloat(f, mantissa, exponent);
	Hash(mantissa, hashVal);
	Hash(exponent, hashVal);
	return;
}

// form the hash value for Point2 p
inline void Hash(Point2 p, UINT& hashVal)
{
	Hash(p.x, hashVal);
	Hash(p.y, hashVal);
	return;
}

// form the hash value for Point3 p
inline void Hash(Point3 p, UINT& hashVal)
{
	Hash(p.x, hashVal);
	Hash(p.y, hashVal);
	Hash(p.z, hashVal);
	return;
}

// form the hash value for AColor c
inline void Hash(AColor c, UINT& hashVal)
{
	Hash(c.r, hashVal);
	Hash(c.g, hashVal);
	Hash(c.b, hashVal);
	Hash(c.a, hashVal);
	return;
}

// form the hash value for Quat q
inline void Hash(Quat q, UINT& hashVal)
{
	Hash(q.x, hashVal);
	Hash(q.y, hashVal);
	Hash(q.z, hashVal);
	Hash(q.w, hashVal);
	return;
}

// form the hash value for Angle Axis a
inline void Hash(AngAxis a, UINT& hashVal)
{
	Hash(a.axis, hashVal);
	Hash(a.angle, hashVal);
	return;
}

// form the hash value for Matrix3 m
inline void Hash(Matrix3 m, UINT& hashVal)
{
	Hash(m.GetRow(0), hashVal);
	Hash(m.GetRow(1), hashVal);
	Hash(m.GetRow(2), hashVal);
	Hash(m.GetRow(3), hashVal);
	return;
}

// form the hash value for Interval t
inline void Hash(Interval t, UINT& hashVal)
{
	Hash(t.Start(), hashVal);
	Hash(t.End(), hashVal);
	return;
}

// form the hash value for Ray r
inline void Hash(Ray r, UINT& hashVal)
{
	Hash(r.p, hashVal);
	Hash(r.dir, hashVal);
	return;
}

// form the hash value for BitArray b
inline void Hash(BitArray b, UINT& hashVal)
{
	for (int i = 0; i < b.GetSize(); i++) {
		if (b[i]) Hash(i, hashVal);
	}
	return;
}

BOOL 
getHashValue(Value* val, UINT& HashValue)
{
	if (is_float_number(val)) 
		Hash(val->to_float(),HashValue);
	else if (is_integer_number(val)) 
		Hash(val->to_int(),HashValue);
	else if (val->is_kind_of(class_tag(String))) 
		Hash(val->to_string(),HashValue);
	else if (val->is_kind_of(class_tag(Array))) {
		Array* theArray = (Array*)val;
		for (int i = 0; i < theArray->size; i++) {
			BOOL ok=getHashValue(theArray->data[i],HashValue);
			if (!ok) return FALSE;
		}
	}
	else if (val->is_kind_of(class_tag(BitArrayValue))) 
		Hash(val->to_bitarray(),HashValue);
	else if (val->is_kind_of(class_tag(Point3Value))) 
		Hash(val->to_point3(),HashValue);
	else if (val->is_kind_of(class_tag(RayValue))) 
		Hash(val->to_ray(),HashValue);
	else if (val->is_kind_of(class_tag(QuatValue))) 
		Hash(val->to_quat(),HashValue);
	else if (val->is_kind_of(class_tag(AngAxisValue))) 
		Hash(val->to_angaxis(),HashValue);
	else if (val->is_kind_of(class_tag(EulerAnglesValue))) {
		Hash(((EulerAnglesValue*)val)->angles[0],HashValue);
		Hash(((EulerAnglesValue*)val)->angles[1],HashValue);
		Hash(((EulerAnglesValue*)val)->angles[2],HashValue);
	}
	else if (val->is_kind_of(class_tag(Matrix3Value))) 
		Hash(val->to_matrix3(),HashValue);
	else if (val->is_kind_of(class_tag(Point2Value))) 
		Hash(val->to_point2(),HashValue);
	else if (val->is_kind_of(class_tag(ColorValue))) 
		Hash(val->to_acolor(),HashValue);
	else
		return FALSE;
	return TRUE;
}

// the MAXScript interface

Value* 
getHashValue_cf(Value** arg_list, int count)
{//	getHashValue <value> <oldHashValue> 
	check_arg_count_with_keys(getHashValue, 2, count);
	UINT HashValue = (UINT)arg_list[1]->to_int();
	return (getHashValue(arg_list[0], HashValue) ? Integer::intern((int)HashValue) : &undefined);
}

Value* 
int_cf(Value** arg_list, int count)
{
//	int <number> 
	check_arg_count(int, 1, count);
	return Integer::intern(arg_list[0]->to_int());
}

Value*
getCoreInterfaces_cf(Value** arg_list, int count)
{
	four_typed_value_locals(Array* result, Value* name, FPInterfaceValue* i, Value* g);
	vl.result = new Array (NumCOREInterfaces());
	for (int i = 0; i < NumCOREInterfaces(); i++)
	{
		// look at each interface
		FPInterface* fpi = GetCOREInterfaceAt(i);
		FPInterfaceDesc* fpid = fpi->GetDesc();
		if (!(fpid->flags & (FP_TEST_INTERFACE | FP_MIXIN)) && !fpid->internal_name.isNull())
		{
			vl.name = Name::intern(fpid->internal_name);
			vl.g = globals->get(vl.name);
			if (vl.g != NULL && !(is_thunk(vl.g) && is_fpstaticinterface(vl.g->eval())))
			{
				TSTR newName = vl.name->to_string();
				newName.append(_T("Interface"));
				vl.name = Name::intern(newName);
				vl.g = globals->get(vl.name);
			}
			if (vl.g == NULL)
			{
				vl.i = new FPInterfaceValue (fpi);
				vl.g = new ConstGlobalThunk (vl.name, vl.i);
				globals->put_new(vl.name, vl.g);
			}
			if (vl.g)
				vl.result->append(vl.g->eval());
		}
	}
	return_value(vl.result);
}

Value*
getTextExtent_cf(Value** arg_list, int count)
{
	check_arg_count(getTextExtent, 1, count);
	TCHAR* text = arg_list[0]->to_string();
	SIZE size;
	HWND hwd = MAXScript_interface->GetMAXHWnd();
	HDC hdc = GetDC(hwd);
	HFONT font = MAXScript_interface->GetAppHFont();
	SelectObject(hdc, font);
	DLGetTextExtent(hdc,text,&size);
	ReleaseDC(hwd, hdc);
	return new Point2Value((float)size.cx, (float)size.cy);
}


Value*
isValidNode_cf(Value** arg_list, int count)
{
	check_arg_count(isValidNode, 1, count);
	MAXNode* node = (MAXNode*)arg_list[0];
	return (is_node(node) && !(deletion_check_test(node))) ? &true_value : &false_value;
}


Value*
isValidObj_cf(Value** arg_list, int count)
{
	check_arg_count(isValidObj, 1, count);
	MAXWrapper* obj = (MAXWrapper*)arg_list[0];
	return (obj->is_kind_of(class_tag(MAXWrapper)) && !deletion_check_test(obj)) ? &true_value : &false_value;
}

Value*
isPB2Based_cf(Value** arg_list, int count)
{
	check_arg_count(isPB2Based, 1, count);
	return (arg_list[0]->is_kind_of(class_tag(MAXClass)) && ((MAXClass*)arg_list[0])->cd2) ? &true_value : &false_value;
}

Value*
updateToolbarButtons_cf(Value** arg_list, int count)
{
	check_arg_count(updateToolbarButtons, 0, count);
	GetCUIFrameMgr()->SetMacroButtonStates(FALSE);
	return &ok;
}

static Value* productAppID = NULL;

Value*
get_productAppID()
{
	if (productAppID == NULL)
	{
		three_typed_value_locals(StringStream* source, Parser* parser, Value* code);
		vl.source = new StringStream (_T("maxops.productAppID"));
		vl.parser = new Parser (thread_local(current_stdout));

		// loop through expr compiling & evaling all expressions,
		// keep the code of last expression as expr for this controller
		vl.source->flush_whitespace();
		vl.code = vl.parser->compile_all(vl.source);
		vl.source->close();
		productAppID = vl.code->eval()->make_heap_permanent();
		pop_value_locals();
	}
	return (productAppID) ? productAppID : &undefined;
}


Value*
okToCreate_cf(Value** arg_list, int count)
{
	// okToCreate <maxobjclass>     
	check_arg_count(okToCreate, 1, count);

	if (arg_list[0] == &undefined) return &false_value; //RK: 10/10/01, case when some plugins are missing

	MAXClass* mc = (MAXClass*)arg_list[0];

#ifdef WEBVERSION
	// If we're given a simple string, try to map it to the appropriate MAXClass
	if(is_string(arg_list[0]))
	{
		int i;
		class_table* ct;
		mc = NULL;
		for(ct = MAXClass::classes, i = 0; i < MAXClass::n_classes; i++, ct++)
			if(_tcsicmp(ct->mx_class->name->to_string(), arg_list[0]->to_string()) == 0)
			{
				mc = ct->mx_class;
				break;
			}
	}
#endif // WEBVERSION

	if (!mc->is_kind_of(class_tag(MAXClass)))
		throw RuntimeError ("okToCreate() requires a MAXClass parameter, got: ", mc);
	ClassDirectory& cdir = MAXScript_interface->GetDllDir().ClassDir();
	//RK: 10/10/01, consider the case where the plugin is missing
	ClassEntry* ce = cdir.FindClassEntry(mc->sclass_id, mc->class_id);
	if (!ce) return &false_value;
	ClassDesc* cd = ce->FullCD();
	return (cd->IsPublic() && cd->OkToCreate(GetCOREInterface())) ? &true_value : &false_value;
}

Value*
get_VptUseDualPlanes()
{
	return (MAXScript_interface7->GetDualPlanes() ? &true_value : &false_value);
}

Value*
set_VptUseDualPlanes(Value* val)
{
	MAXScript_interface7->SetDualPlanes(val->to_bool());
	return val;
}

Value*
getManipulateMode()
{
	return (MAXScript_interface7->InManipMode() ? &true_value : &false_value);
}

Value*
setManipulateMode(Value* val)
{
	if (val->to_bool()) 
	{
		if (!MAXScript_interface7->InManipMode())
			MAXScript_interface7->StartManipulateMode();
	}
	else
		MAXScript_interface7->EndManipulateMode();
	return val;
}

Value*
getLastRenderedImage_cf(Value** arg_list, int count)
{
	// getLastRenderedImage copy:<bool>    
	check_arg_count_with_keys(getLastRenderedImage, 0, count);
	Value* tmp;
	BOOL asCopy = bool_key_arg(copy, tmp, TRUE); 

	Bitmap* image = MAXScript_interface7->GetLastRenderedImage();
	if (image == NULL) return &undefined;
	one_typed_value_local(MAXBitMap* mbm);
	vl.mbm = new MAXBitMap ();
	vl.mbm->bi.CopyImageInfo(&image->Storage()->bi);

	if (!asCopy)
	{
		vl.mbm->bm = TheManager->NewBitmap();
		vl.mbm->bm->SetStorage(image->Storage());
	}
	else
	{
		vl.mbm->bi.SetFirstFrame(0);
		vl.mbm->bi.SetLastFrame(0);
		vl.mbm->bi.SetName("");
		vl.mbm->bi.SetDevice("");
		vl.mbm->bi.SetType(GetStorableBitmapInfoTypeForBitmapInfoType(vl.mbm->bi.Type()));
		vl.mbm->bm = CreateBitmapFromBitmapInfo(vl.mbm->bi);

		BMM_Color_64 black64(0,0,0,0);
		vl.mbm->bm->CopyImage(image, COPY_IMAGE_CROP, black64, &vl.mbm->bi);
	}
	return_value(vl.mbm);
}

Value*
get_coordsys_node()
{
	INode *coordNode = MAXScript_interface7->GetRefCoordNode();
	if (coordNode == NULL) 
		return &undefined;
	else
		return MAXNode::intern(coordNode);
}

Value*
set_coordsys_node(Value* val)
{
	MAXScript_interface7->AddRefCoordNode(val->to_node());
	return val;
}

// ============================================================================
// <Point2Value> getMAXWindowSize
Value* getMAXWindowSize_cf( Value** arg_list, int count )
{
	RECT rect;
	check_arg_count( getMAXWindowSize, 0, count );
	GetWindowRect( GetCOREInterface()->GetMAXHWnd(), &rect );
	return new Point2Value( rect.right-rect.left, rect.bottom-rect.top );
}

// <Point2Value> getMAXWindowPos
Value* getMAXWindowPos_cf( Value** arg_list, int count )
{
	RECT rect;
	check_arg_count( getMAXWindowPos, 0, count );
	GetWindowRect( GetCOREInterface()->GetMAXHWnd(), &rect );
	return new Point2Value( rect.left, rect.top );
}

Value*
getSelectionLevel_cf(Value** arg_list, int count)
{
	// getSelectionLevel <maxobject>
	//                                         
	check_arg_count(getSelectionLevel, 1, count);
	IMeshSelect *ims = NULL;
	ISplineSelect *iss = NULL;
	IPatchSelect *ips = NULL;
	Animatable *target = NULL;
	int level;
	if (is_node(arg_list[0]))
		target = Get_Object_Or_XRef_BaseObject(arg_list[0]->to_node()->GetObjectRef());
//	else if (is_modifier(arg_list[0]))
//		target = arg_list[0]->to_modifier();
	else
		target = arg_list[0]->to_reftarg();

	// need to block xref object access
	if (target->GetInterface(IID_XREF_ITEM))
		target = NULL;

	if (target)
	{
		if ((ims = (IMeshSelect*)target->GetInterface(I_MESHSELECT)) != NULL)
			level = ims->GetSelLevel();
		else if ((iss = (ISplineSelect*)target->GetInterface(I_SPLINESELECT)) != NULL)
			level = iss->GetSelLevel();
		else if ((ips = (IPatchSelect*)target->GetInterface(I_PATCHSELECT)) != NULL)
			level = ips->GetSelLevel();
		else
			throw RuntimeError (GetString(IDS_NO_SELECT_INTERFACE_FOR_OBJECT), arg_list[0]);
		def_select_so_types();
		return GetName(selectsoTypes, elements(selectsoTypes),level);
	}
	else
		throw RuntimeError (GetString(IDS_NO_SELECT_INTERFACE_FOR_OBJECT), arg_list[0]);
	return &undefined;
}

Value*
setSelectionLevel_cf(Value** arg_list, int count)
{
	// setSelectionLevel <maxobject> {#object | #vertex | #edge | #face}
	//                                         
	check_arg_count(setSelectionLevel, 2, count);
	IMeshSelect *ims = NULL;
	ISplineSelect *iss = NULL;
	IPatchSelect *ips = NULL;
	Animatable *target = NULL;
	def_select_so_types();
	int level = GetID(selectsoTypes, elements(selectsoTypes),arg_list[1]);
	if (is_node(arg_list[0]))
		target = Get_Object_Or_XRef_BaseObject(arg_list[0]->to_node()->GetObjectRef());
//	else if (is_modifier(arg_list[0]))
//		target = arg_list[0]->to_modifier();
	else
		target = arg_list[0]->to_reftarg();

	// need to block xref object access
	if (target->GetInterface(IID_XREF_ITEM))
		target = NULL;

	if (target)
	{
		if ((ims = (IMeshSelect*)target->GetInterface(I_MESHSELECT)) != NULL)
			ims->SetSelLevel(level);
		else if ((iss = (ISplineSelect*)target->GetInterface(I_SPLINESELECT)) != NULL)
			iss->SetSelLevel(level);
		else if ((ips = (IPatchSelect*)target->GetInterface(I_PATCHSELECT)) != NULL)
			ips->SetSelLevel(level);
		else
			throw RuntimeError (GetString(IDS_NO_SELECT_INTERFACE_FOR_OBJECT), arg_list[0]);
	}
	else
		throw RuntimeError (GetString(IDS_NO_SELECT_INTERFACE_FOR_OBJECT), arg_list[0]);
	return &ok;
}

Value*
stricmp_cf(Value** arg_list, int count)
{
	check_arg_count(stricmp, 2, count);
	return Integer::intern(_stricmp(arg_list[0]->to_string(),arg_list[1]->to_string()));
}

Value*
internetCheckConnection_cf(Value** arg_list, int count)
{
	// Internet.CheckConnection url:<string> force:<bool>
	check_arg_count_with_keys(internet.CheckConnection, 0, count);
	Value* tmp;
	TCHAR *url = NULL;
	Value* urlVal = key_arg(url)->eval();
	if (urlVal != &unsupplied)
		url = urlVal->to_string();
	DWORD flags = bool_key_arg(force,tmp,FALSE) ? FLAG_ICC_FORCE_CONNECTION : 0;
	return (InternetCheckConnection(url,flags,0)) ? &true_value : &false_value;
}

// class Mtl:
Value*
getNumSubMtls_cf(Value** arg_list, int count)
{
	check_arg_count(getNumSubMtls, 1, count);
	Mtl *m = arg_list[0]->to_mtl();
	if (m == NULL) throw ConversionError (arg_list[0], _T("material"));
	return Integer::intern(m->NumSubMtls());
}

Value*
getSubMtl_cf(Value** arg_list, int count)
{
	check_arg_count(getSubMtl, 2, count);
	Mtl *m = arg_list[0]->to_mtl();
	if (m == NULL) throw ConversionError (arg_list[0], _T("material"));
	int which = arg_list[1]->to_int();
	range_check(which, 1, m->NumSubMtls(), GetString(IDS_SUBMTL_INDEX_OUT_OF_RANGE));
	Mtl* sm = m->GetSubMtl(which-1);
	return MAXClass::make_wrapper_for(sm);
}

Value*
setSubMtl_cf(Value** arg_list, int count)
{
	check_arg_count(setSubMtl, 3, count);
	Mtl *m = arg_list[0]->to_mtl();
	if (m == NULL) throw ConversionError (arg_list[0], _T("material"));
	int which = arg_list[1]->to_int();
	range_check(which, 1, m->NumSubMtls(), GetString(IDS_SUBMTL_INDEX_OUT_OF_RANGE));
	Mtl *sm = NULL;
	if (arg_list[2] != &undefined)
	{
		sm = arg_list[2]->to_mtl();
		if (sm->TestForLoop(FOREVER,m) != REF_SUCCEED)
			return &ok;
	}
	m->SetSubMtl(which-1,sm);
	return &ok;
}

Value*
getSubMtlSlotName_cf(Value** arg_list, int count)
{
	check_arg_count(getSubMtlSlotName, 2, count);
	Mtl *m = arg_list[0]->to_mtl();
	if (m == NULL) throw ConversionError (arg_list[0], _T("material"));
	int which = arg_list[1]->to_int();
	range_check(which, 1, m->NumSubMtls(), GetString(IDS_SUBMTL_INDEX_OUT_OF_RANGE));
	TSTR s_name = m->GetSubMtlSlotName(which-1);
	return new String(s_name);
}

// class ISubMap:
Value*
getNumSubTexmaps_cf(Value** arg_list, int count)
{
	check_arg_count(getNumSubTexmaps, 1, count);
	MtlBase *m = arg_list[0]->to_mtlbase();
	return Integer::intern(m->NumSubTexmaps());
}

Value*
getSubTexmap_cf(Value** arg_list, int count)
{
	check_arg_count(getSubTexmap, 2, count);
	MtlBase *m = arg_list[0]->to_mtlbase();
	int which = arg_list[1]->to_int();
	range_check(which, 1, m->NumSubTexmaps(), GetString(IDS_SUBTEXMAP_INDEX_OUT_OF_RANGE));
	Texmap* tm = m->GetSubTexmap(which-1);
	return MAXClass::make_wrapper_for(tm);
}

Value*
setSubTexmap_cf(Value** arg_list, int count)
{
	check_arg_count(setSubTexmap, 3, count);
	MtlBase *m = arg_list[0]->to_mtlbase();
	int which = arg_list[1]->to_int();
	range_check(which, 1, m->NumSubTexmaps(), GetString(IDS_SUBTEXMAP_INDEX_OUT_OF_RANGE));
	Texmap *tm = NULL;
	if (arg_list[2] != &undefined)
	{
		tm = arg_list[2]->to_texmap();
		if (tm->TestForLoop(FOREVER,m) != REF_SUCCEED)
			return &ok;
	}
	m->SetSubTexmap(which-1,tm);
	return &ok;
}

Value*
getSubTexmapSlotName_cf(Value** arg_list, int count)
{
	check_arg_count(getSubTexmapSlotName, 2, count);
	MtlBase *m = arg_list[0]->to_mtlbase();
	int which = arg_list[1]->to_int();
	range_check(which, 1, m->NumSubTexmaps(), GetString(IDS_SUBTEXMAP_INDEX_OUT_OF_RANGE));
	TSTR s_name = m->GetSubTexmapSlotName(which-1);
	return new String(s_name);
}

// ===============================================
//      hold system exposure
// ===============================================

Value*
theHold_Begin_cf(Value** arg_list, int count)
{
//	check_arg_count("Begin", 2, count);
	theHold.Begin();
	return &ok;
}

Value*
theHold_Suspend_cf(Value** arg_list, int count)
{
//	check_arg_count("Suspend", 0, count);
	theHold.Suspend();
	return &ok;
}

Value*
theHold_IsSuspended_cf(Value** arg_list, int count)
{
//	check_arg_count("IsSuspended", 0, count);
	return ((theHold.IsSuspended()) ? &true_value : &false_value);
}

Value*
theHold_Resume_cf(Value** arg_list, int count)
{
//	check_arg_count("Resume", 0, count);
	theHold.Resume();
	return &ok;
}

Value*
theHold_Holding_cf(Value** arg_list, int count)
{
//	check_arg_count("Holding", 0, count);
	return ((theHold.Holding()) ? &true_value : &false_value);
}

Value*
theHold_Restoring_cf(Value** arg_list, int count)
{
//	check_arg_count_with_keys("Restoring", 0, count);
	int isUndo;
	Value* isUndoValue = key_arg(isUndo);
	Thunk* isUndoThunk = NULL;
	if (isUndoValue != &unsupplied && isUndoValue->_is_thunk()) 
		isUndoThunk = isUndoValue->to_thunk();
	int res = theHold.Restoring(isUndo);
	if (isUndoThunk)
		isUndoThunk->assign(Integer::intern(isUndo));
	return ((res) ? &true_value : &false_value);
}

Value*
theHold_Redoing_cf(Value** arg_list, int count)
{
//	check_arg_count("Redoing", 0, count);
	return ((theHold.Redoing()) ? &true_value : &false_value);
}

Value*
theHold_RestoreOrRedoing_cf(Value** arg_list, int count)
{
//	check_arg_count("RestoreOrRedoing", 0, count);
	return ((theHold.RestoreOrRedoing()) ? &true_value : &false_value);
}

Value*
theHold_DisableUndo_cf(Value** arg_list, int count)
{
//	check_arg_count("DisableUndo", 0, count);
	theHold.DisableUndo();
	return &ok;
}

Value*
theHold_EnableUndo_cf(Value** arg_list, int count)
{
//	check_arg_count("EnableUndo", 0, count);
	theHold.EnableUndo();
	return &ok;
}

Value*
theHold_Restore_cf(Value** arg_list, int count)
{
//	check_arg_count("Restore", 0, count);
	theHold.Restore();
	return &ok;
}

Value*
theHold_Release_cf(Value** arg_list, int count)
{
//	check_arg_count("Release", 0, count); 
	theHold.Release();
	return &ok;
}

		// 3 ways to terminate the Begin()...
Value*
theHold_End_cf(Value** arg_list, int count)
{
//	check_arg_count("End", 0, count);
	theHold.End();
	return &ok;
}

Value*
theHold_Accept_cf(Value** arg_list, int count)
{
	if (count == 1 && (is_string(arg_list[0]) || is_name(arg_list[0])))
		theHold.Accept(arg_list[0]->to_string());
	else
		theHold.Accept(_T(""));
//	check_arg_count("Accept", 1, count);
//	arg_list[0]->to_string();
	return &ok;
}

Value*
theHold_Cancel_cf(Value** arg_list, int count)
{
//	check_arg_count("Cancel", 0, count);
	theHold.Cancel();
	return &ok;
}

//		
// Group several Begin-End lists into a single Super-group.
Value*
theHold_SuperBegin_cf(Value** arg_list, int count)
{
//	check_arg_count("SuperBegin", 0, count);
	theHold.SuperBegin();
	return &ok;
}

// do a SuperAccept regardless of error 
Value*
theHold_SuperAccept_cf(Value** arg_list, int count)
{
	if (count == 1 && (is_string(arg_list[0]) || is_name(arg_list[0])))
		theHold.SuperAccept(arg_list[0]->to_string());
	else
		theHold.SuperAccept(_T(""));
//	check_arg_count("SuperAccept", 1, count);
//	arg_list[0]->to_string();
	return &ok;
}

Value*
theHold_SuperCancel_cf(Value** arg_list, int count)
{
//	check_arg_count("SuperCancel", 0, count);
	theHold.SuperCancel();
	return &ok;
}


// Get the number of times Put() has been called in the current session of MAX
Value*
theHold_GetGlobalPutCount_cf(Value** arg_list, int count)
{
//	check_arg_count("GetGlobalPutCount", 0, count);
	return Integer::intern(theHold.GetGlobalPutCount());
}

// Get the superlevel depth
Value*
theHold_GetSuperLevel_cf(Value** arg_list, int count)
{
//	check_arg_count("GetSuperLevel", 0, count);
	return Integer::intern(theHold.GetSuperBeginDepth());
}

// Get whether undo disabled
Value*
theHold_IsUndoDisabled_cf(Value** arg_list, int count)
{
//	check_arg_count("IsUndoDisabled", 0, count);
	return ((theHold.IsUndoDisabled()) ? &true_value : &false_value);
}

// Get the begin depth
Value*
theHold_GetBeginDepth_cf(Value** arg_list, int count)
{
//	check_arg_count("GetBeginDepth", 0, count);
	return Integer::intern(theHold.GetBeginDepth());
}

/* --------------------- Node reset transform methods --------------------------------- */

#define CLUSTOSM_CLASS_ID			0x25215824

Value*
ResetXForm_cf(Value** arg_list, int count)
{
	// ResetXForm <node>
	check_arg_count(ResetXForm, 1, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], ResetXForm);
	TimeValue t = MAXScript_time();

	Matrix3 ntm, ptm, rtm(1), piv(1), tm;

	Object*			obj = node->GetObjectRef();

	SimpleMod *mod = (SimpleMod*)MAXScript_interface->CreateInstance(OSM_CLASS_ID,Class_ID(CLUSTOSM_CLASS_ID,0));
	if (mod == NULL)
		return &false_value;

	// Get Parent and Node TMs
	ntm = node->GetNodeTM(t);
	ptm = node->GetParentTM(t);

	// Compute the relative TM
	ntm = ntm * Inverse(ptm);

	// The reset TM only inherits position
	rtm.SetTrans(ntm.GetTrans());

	// Set the node TM to the reset TM		
	tm = rtm*ptm;
	node->SetNodeTM(t, tm);

	// Compute the pivot TM
	piv.SetTrans(node->GetObjOffsetPos());
	PreRotateMatrix(piv,node->GetObjOffsetRot());
	ApplyScaling(piv,node->GetObjOffsetScale());

	// Reset the offset to 0
	node->SetObjOffsetPos(Point3(0,0,0));
	node->SetObjOffsetRot(IdentQuat());
	node->SetObjOffsetScale(ScaleValue(Point3(1,1,1)));

	// Take the position out of the matrix since we don't reset position
	ntm.NoTrans();

	// Apply the offset to the TM
	ntm = piv * ntm;

	// Apply a derived object to the node's object
	IDerivedObject *dobj = CreateDerivedObject(obj);

	// Apply the transformation to the mod.
	SetXFormPacket pckt(ntm);
	mod->tmControl->SetValue(t,&pckt);

	BOOL was_editing = MAXScript_interface->GetCommandPanelTaskMode() == TASK_MODE_MODIFY &&
		MAXScript_interface->GetSelNodeCount() > 0 ; // && node->Selected();

	// clear any problem command modes
	MAXScript_interface->StopCreating();
	MAXScript_interface->ClearPickMode();
	if (was_editing)
		MAXScript_interface7->SuspendEditing();

	// Add the bend modifier to the derived object.
	dobj->AddModifier(mod);

	// Replace the node's object
	node->SetObjectRef(dobj);

	needs_redraw_set();
	SetSaveRequiredFlag(TRUE);

	MAXScript_interface7->InvalidateObCache(node);

	dobj->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	node->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	if (was_editing)
		MAXScript_interface7->ResumeEditing();

	return &true_value;
}

Value*
SuspendEditing_cf(Value** arg_list, int count)
{
	// SuspendEditing ()
	check_arg_count_with_keys(SuspendEditing, 0, count);
	def_cpanel_types();	
	DWORD which = GetID(cpanelTypes, elements(cpanelTypes), key_arg(which),0x00FF);
	if (which != 0x00FF) which = (1<<which);
	Value* tmp;
	BOOL alwaysSuspend = bool_key_arg(alwaysSuspend, tmp, TRUE); 
	MAXScript_interface7->SuspendEditing(which, alwaysSuspend);
	return &ok;
}

Value*
ResumeEditing_cf(Value** arg_list, int count)
{
	// ResumeEditing ()
	check_arg_count_with_keys(ResumeEditing, 0, count);
	def_cpanel_types();	
	DWORD which = GetID(cpanelTypes, elements(cpanelTypes), key_arg(which),0x00FF);
	if (which != 0x00FF) which = (1<<which);
	Value* tmp;
	BOOL alwaysSuspend = bool_key_arg(alwaysSuspend, tmp, true); 
	MAXScript_interface7->ResumeEditing(which, alwaysSuspend);
	return &ok;
}


Value*
CenterPivot_cf(Value** arg_list, int count)
{
	// CenterPivot <node> 
	check_arg_count(CenterPivot, 1, count);
	INode *node = get_valid_node((MAXNode*)arg_list[0], CenterPivot);
	// Disable animation for this
	if (!(node->IsGroupMember() && !node->IsOpenGroupMember())) {
		AnimateSuspend as; // LAM - 6/27/03 - defect 505170
		node->CenterPivot(MAXScript_time(),FALSE);			
		needs_redraw_set();
		return &true_value;
	}
	return &false_value;
}

Value*
CenterObject_cf(Value** arg_list, int count)
{
	// CenterObject <node> 
	check_arg_count(CenterObject, 1, count);
	INode *node = get_valid_node((MAXNode*)arg_list[0], CenterObject);
	// Disable animation for this
	if (!(node->IsGroupMember() && !node->IsOpenGroupMember())) {
		AnimateSuspend as; // LAM - 6/27/03 - defect 505170
		node->CenterPivot(MAXScript_time(),TRUE);			
		needs_redraw_set();
		return &true_value;
	}
	return &false_value;
}

Value*
WorldAlignPivot_cf(Value** arg_list, int count)
{
	// WorldAlignPivot <node>
	check_arg_count(WorldAlignPivot, 1, count);
	INode *node = get_valid_node((MAXNode*)arg_list[0], WorldAlignPivot);
	// Disable animation for this
	if (!(node->IsGroupMember() && !node->IsOpenGroupMember())) {
		AnimateSuspend as; // LAM - 6/27/03 - defect 505170
		node->WorldAlignPivot(MAXScript_time(),FALSE);			
		needs_redraw_set();
		return &true_value;
	}
	return &false_value;
}

Value*
WorldAlignObject_cf(Value** arg_list, int count)
{
	// WorldAlignObject <node>
	check_arg_count(WorldAlignObject, 1, count);
	INode *node = get_valid_node((MAXNode*)arg_list[0], WorldAlignObject);
	// Disable animation for this
	if (!(node->IsGroupMember() && !node->IsOpenGroupMember())) {
		AnimateSuspend as; // LAM - 6/27/03 - defect 505170
		node->WorldAlignPivot(MAXScript_time(),TRUE);			
		needs_redraw_set();
		return &true_value;
	}
	return &false_value;
}

Value*
AlignPivot_cf(Value** arg_list, int count)
{
	// AlignPivot <node>
	check_arg_count(AlignPivot, 1, count);
	INode *node = get_valid_node((MAXNode*)arg_list[0], AlignPivot);
	// Disable animation for this
	if (!(node->IsGroupMember() && !node->IsOpenGroupMember())) {
		AnimateSuspend as; // LAM - 6/27/03 - defect 505170
		node->AlignPivot(MAXScript_time(),FALSE);			
		needs_redraw_set();
		return &true_value;
	}
	return &false_value;
}

Value*
AlignObject_cf(Value** arg_list, int count)
{
	// AlignObject <node>
	check_arg_count(AlignObject, 1, count);
	INode *node = get_valid_node((MAXNode*)arg_list[0], AlignObject);
	// Disable animation for this
	if (!(node->IsGroupMember() && !node->IsOpenGroupMember())) {
		AnimateSuspend as; // LAM - 6/27/03 - defect 505170
		node->AlignPivot(MAXScript_time(),TRUE);			
		needs_redraw_set();
		return &true_value;
	}
	return &false_value;
}

Value*
AlignToParent_cf(Value** arg_list, int count)
{
	// AlignToParent <node> 
	check_arg_count(AlignToParent, 1, count);
	INode *node = get_valid_node((MAXNode*)arg_list[0], AlignToParent);
	if (!(node->IsGroupMember() && !node->IsOpenGroupMember())) {
		node->AlignToParent(MAXScript_time());			
		needs_redraw_set();
		return &true_value;
	}
	return &false_value;
}

Value*
ResetTransform_cf(Value** arg_list, int count)
{
	// ResetTransform <node>
	check_arg_count(ResetTransform, 1, count);
	INode *node = get_valid_node((MAXNode*)arg_list[0], ResetTransform);
	if (!(node->IsGroupMember() && !node->IsOpenGroupMember())) {
		node->ResetTransform(MAXScript_time(),FALSE);
		needs_redraw_set();
		return &true_value;
	}
	return &false_value;
}

Value*
ResetScale_cf(Value** arg_list, int count)
{
	// ResetScale <node>
	check_arg_count(ResetScale, 1, count);
	INode *node = get_valid_node((MAXNode*)arg_list[0], ResetScale);
	if (!(node->IsGroupMember() && !node->IsOpenGroupMember())) {
		node->ResetTransform(MAXScript_time(),TRUE);
		needs_redraw_set();
		return &true_value;
	}
	return &false_value;
}

Value*
ResetPivot_cf(Value** arg_list, int count)
{
	// ResetPivot <node>
	check_arg_count(ResetPivot, 1, count);
	INode *node = get_valid_node((MAXNode*)arg_list[0], ResetPivot);
	if (!(node->IsGroupMember() && !node->IsOpenGroupMember())) {
		node->ResetPivot(MAXScript_time());
		needs_redraw_set();
		return &true_value;
	}
	return &false_value;
}

static Class_ID  noneClassID(0,0);

Value*
areMtlAndRendererCompatible_cf(Value** arg_list, int count)
{
	// areMtlAndRendererCompatible {<mtlBase*> | <classDesc>} renderer:{<renderer> | <classDesc>}
	check_arg_count_with_keys(areMtlAndRendererCompatible, 1, count);
	ClassDesc* mtlClassDesc = NULL;
	ClassDesc* rendererClassDesc = NULL;
	ClassDirectory& cdir = MAXScript_interface->GetDllDir().ClassDir();
	Value* arg0 = arg_list[0];
	if (arg0->is_kind_of(class_tag(MAXClass)))
	{
		MAXClass* mc = (MAXClass*)arg0;
		ClassEntry* ce = cdir.FindClassEntry(mc->sclass_id, mc->class_id);
		if (ce) mtlClassDesc = ce->FullCD();
	}
	else
	{
		MtlBase *r = arg0->to_mtlbase();
		mtlClassDesc = cdir.FindClass(r->SuperClassID(), r->ClassID());
	}

	Value* arg1 = key_arg(renderer);
	if (arg1 == &unsupplied)
	{
		Renderer* currentRenderer = MAXScript_interface->GetCurrentRenderer();
		if(currentRenderer != NULL)
			rendererClassDesc = cdir.FindClass(currentRenderer->SuperClassID(), currentRenderer->ClassID());
	}
	else if (arg1->is_kind_of(class_tag(MAXClass)))
	{
		MAXClass* mc = (MAXClass*)arg1;
		ClassEntry* ce = cdir.FindClassEntry(mc->sclass_id, mc->class_id);
		if (ce) rendererClassDesc = ce->FullCD();
	}
	else
	{
		Renderer *r = arg1->to_renderer();
		rendererClassDesc = cdir.FindClass(r->SuperClassID(), r->ClassID());
	}

	bool incompatible = ((mtlClassDesc != NULL) && (rendererClassDesc != NULL) && 
						(mtlClassDesc->ClassID() != noneClassID) && 
						(rendererClassDesc->ClassID() != noneClassID) && 
						!AreMtlAndRendererCompatible(*mtlClassDesc, *rendererClassDesc));

	return incompatible ? &false_value : &true_value;
}

Value*
getHideByCategory_geometry()
{
	DWORD f = GetCOREInterface()->GetHideByCategoryFlags();
	return (f & HIDE_OBJECTS) ? &true_value : &false_value;
}
Value*
setHideByCategory_geometry(Value* val)
{
	DWORD f = HIDE_OBJECTS;
	DWORD f1 = GetCOREInterface()->GetHideByCategoryFlags();
	bool isHidden = (f1 & f) == 0; // currently hidden if != 0
	bool hide = val->to_bool() == 0;
	if (isHidden != hide)
		GetCOREInterface()->ExecuteMAXCommand(MAXCOM_HIDE_OBJECT_TOGGLE );
	return val;
}

Value*
getHideByCategory_shapes()
{
	DWORD f = GetCOREInterface()->GetHideByCategoryFlags();
	return (f & HIDE_SHAPES) ? &true_value : &false_value;
}
Value*
setHideByCategory_shapes(Value* val)
{
	DWORD f = HIDE_SHAPES;
	DWORD f1 = GetCOREInterface()->GetHideByCategoryFlags();
	bool isHidden = (f1 & f) == 0; // currently hidden if != 0
	bool hide = val->to_bool() == 0;
	if (isHidden != hide)
		GetCOREInterface()->ExecuteMAXCommand(MAXCOM_HIDE_SHAPE_TOGGLE);
	return val;
}

Value*
getHideByCategory_lights()
{
	DWORD f = GetCOREInterface()->GetHideByCategoryFlags();
	return (f & HIDE_LIGHTS) ? &true_value : &false_value;
}
Value*
setHideByCategory_lights(Value* val)
{
	DWORD f = HIDE_LIGHTS;
	DWORD f1 = GetCOREInterface()->GetHideByCategoryFlags();
	bool isHidden = (f1 & f) == 0; // currently hidden if != 0
	bool hide = val->to_bool() == 0;
	if (isHidden != hide)
		GetCOREInterface()->ExecuteMAXCommand(MAXCOM_HIDE_LIGHT_TOGGLE);
	return val;
}

Value*
getHideByCategory_cameras()
{
	DWORD f = GetCOREInterface()->GetHideByCategoryFlags();
	return (f & HIDE_CAMERAS) ? &true_value : &false_value;
}
Value*
setHideByCategory_cameras(Value* val)
{
	DWORD f = HIDE_CAMERAS;
	DWORD f1 = GetCOREInterface()->GetHideByCategoryFlags();
	bool isHidden = (f1 & f) == 0; // currently hidden if != 0
	bool hide = val->to_bool() == 0;
	if (isHidden != hide)
		GetCOREInterface()->ExecuteMAXCommand(MAXCOM_HIDE_CAMERA_TOGGLE);
	return val;
}

Value*
getHideByCategory_helpers()
{
	DWORD f = GetCOREInterface()->GetHideByCategoryFlags();
	return (f & HIDE_HELPERS) ? &true_value : &false_value;
}
Value*
setHideByCategory_helpers(Value* val)
{
	DWORD f = HIDE_HELPERS;
	DWORD f1 = GetCOREInterface()->GetHideByCategoryFlags();
	bool isHidden = (f1 & f) == 0; // currently hidden if != 0
	bool hide = val->to_bool() == 0;
	if (isHidden != hide)
		GetCOREInterface()->ExecuteMAXCommand(MAXCOM_HIDE_HELPER_TOGGLE);
	return val;
}

Value*
getHideByCategory_spacewarps()
{
	DWORD f = GetCOREInterface()->GetHideByCategoryFlags();
	return (f & HIDE_WSMS) ? &true_value : &false_value;
}
Value*
setHideByCategory_spacewarps(Value* val)
{
	DWORD f = HIDE_WSMS;
	DWORD f1 = GetCOREInterface()->GetHideByCategoryFlags();
	bool isHidden = (f1 & f) == 0; // currently hidden if != 0
	bool hide = val->to_bool() == 0;
	if (isHidden != hide)
		GetCOREInterface()->ExecuteMAXCommand(MAXCOM_HIDE_WSM_TOGGLE);
	return val;
}

Value*
getHideByCategory_particles()
{
	DWORD f = GetCOREInterface()->GetHideByCategoryFlags();
	return (f & HIDE_PARTICLES) ? &true_value : &false_value;
}
Value*
setHideByCategory_particles(Value* val)
{
	DWORD f = HIDE_PARTICLES;
	DWORD f1 = GetCOREInterface()->GetHideByCategoryFlags();
	bool isHidden = (f1 & f) == 0; // currently hidden if != 0
	bool hide = val->to_bool() == 0;
	if (isHidden != hide)
		GetCOREInterface()->ExecuteMAXCommand(MAXCOM_HIDE_SYSTEM_TOGGLE);
	return val;
}

Value*
getHideByCategory_bones()
{
	DWORD f = GetCOREInterface()->GetHideByCategoryFlags();
	return (f & HIDE_BONEOBJECTS) ? &true_value : &false_value;
}
Value*
setHideByCategory_bones(Value* val)
{
	DWORD f = HIDE_BONEOBJECTS;
	DWORD f1 = GetCOREInterface()->GetHideByCategoryFlags();
	((val->to_bool())) ? f1 |= f : f1 &= ~f;
	GetCOREInterface()->SetHideByCategoryFlags(f1);
	GetCOREInterface()->ExecuteMAXCommand(MAXCOM_TOOL_EDGES_ONLY_TOGGLE);
	GetCOREInterface()->ExecuteMAXCommand(MAXCOM_TOOL_EDGES_ONLY_TOGGLE);
	return val;
}

Value* 
HideByCategory_all_cf(Value** arg_list, int count)
{
	//	hideByCategory.all() 
	check_arg_count(hideByCategory.all, 0, count);
	setHideByCategory_geometry(&true_value);
	setHideByCategory_shapes(&true_value);
	setHideByCategory_lights(&true_value);
	setHideByCategory_cameras(&true_value);
	setHideByCategory_bones(&true_value);
	setHideByCategory_helpers(&true_value);
	setHideByCategory_spacewarps(&true_value);
	setHideByCategory_particles(&true_value);
	return &ok;
}

Value* 
HideByCategory_none_cf(Value** arg_list, int count)
{
	//	hideByCategory.none() 
	check_arg_count(hideByCategory.none, 0, count);
	setHideByCategory_geometry(&false_value);
	setHideByCategory_shapes(&false_value);
	setHideByCategory_lights(&false_value);
	setHideByCategory_cameras(&false_value);
	setHideByCategory_bones(&false_value);
	setHideByCategory_helpers(&false_value);
	setHideByCategory_spacewarps(&false_value);
	setHideByCategory_particles(&false_value);
	return &ok;
}

Value* 
GetQuietMode_cf(Value** arg_list, int count)
{
	check_arg_count(GetQuietMode, 0, count);
	return GetCOREInterface()->GetQuietMode() ? &true_value : &false_value;	
}

Value* 
SetQuietMode_cf(Value** arg_list, int count)
{
	check_arg_count(SetQuietMode, 1, count);
	return GetCOREInterface()->SetQuietMode(arg_list[0]->to_bool()) ? &true_value : &false_value;
}

Value* 
GetMAXIniFile_cf(Value** arg_list, int count)
{
	check_arg_count(GetMAXIniFile, 0, count);
	return new String(GetCOREInterface7()->GetMAXIniFile());
}

/* --------------------- Dependency Test --------------------------------- */

Value*
DependencyLoopTest_cf(Value** arg_list, int count)
{
	// DependencyLoopTest <RefTarg_source> <RefTarg_target>
	// returns true if RefTarg1 is dependent on RefTarg2
	check_arg_count(DependencyLoopTest, 2, count);
	ReferenceTarget *source = (is_node(arg_list[0])) ? (ReferenceTarget*)((MAXNode*)arg_list[0])->node : arg_list[0]->to_reftarg();
	ReferenceTarget *target = (is_node(arg_list[1])) ? (ReferenceTarget*)((MAXNode*)arg_list[1])->node : arg_list[1]->to_reftarg();
	if (source == NULL || target == NULL) return &false_value;
	deletion_check((MAXWrapper*)arg_list[0]);
	deletion_check((MAXWrapper*)arg_list[1]);
	return (source->TestForLoop(FOREVER,target)==REF_FAIL) ? &true_value : &false_value;
}

//-----------------------------------------------------------

class GetInstancesProc_AnimEnum : public AnimEnum
{
public:
	MAXClass* mc;
	Array* result;
	GetInstancesProc_AnimEnum(MAXClass* mc, Array* result, int scope) : AnimEnum(scope)
	{
		this->mc = mc;
		this->result = result;
	}
	int proc(Animatable *anim, Animatable *client, int subNum)
	{
		if (anim == NULL)
			return ANIM_ENUM_PROCEED;

		Class_ID cid = anim->ClassID();
		SClass_ID sid = anim->SuperClassID();
		MAXClass* cls = lookup_MAXClass(&cid, sid);
		if (cls == mc)
		{
			Value *item = MAXClass::make_wrapper_for((ReferenceTarget*)anim);
			if (item != &undefined)
			{
				TrackViewPick tvp;
				tvp.anim = (ReferenceTarget*)anim;
				tvp.client = (ReferenceTarget*)client;
				tvp.subNum = subNum;
				result->append(new TrackViewPickValue(tvp));
			}
		}

		if (anim->TestAFlag(A_WORK3))
			return ANIM_ENUM_STOP; // already been down this path....
		anim->SetAFlag(A_WORK3);
		return ANIM_ENUM_PROCEED;
	}
};

class GetInstancesProc_RefEnum : public RefEnumProc
{
public:
	MAXClass* mc;
	Array* result;
	BOOL processChildren;
	GetInstancesProc_RefEnum(MAXClass* mc, Array* result,  BOOL processChildren)
	{
		this->mc = mc;
		this->result = result;
		this->processChildren = processChildren;
	}
	int proc(ReferenceMaker *rm)
	{
		if (rm == NULL)
			return REF_ENUM_CONTINUE;
		Class_ID cid = rm->ClassID();
		SClass_ID sid = rm->SuperClassID();
		MAXClass* cls = lookup_MAXClass(&cid, sid);
		if (cls == mc)
		{
			Value *item = MAXClass::make_wrapper_for((ReferenceTarget*)rm);
			if (item != &undefined)
				result->append(item);
		}
		if (processChildren)
		{
			int numChildren = rm->NumChildren();
			for (int i = 0; i < numChildren; i++)
			{
				ReferenceMaker *child = dynamic_cast<ReferenceMaker*>(rm->ChildAnim(i));
				if (child)
					child->EnumRefHierarchy(*this, true, true, true);
			}
		}
		return REF_ENUM_CONTINUE;
	}
};

class  GetInstancesProc_AnimListEnum : public Animatable::EnumAnimList
{
public:
	MAXClass* mc;
	Array* result;
	GetInstancesProc_AnimListEnum(MAXClass* mc, Array* result)
	{
		this->mc = mc;
		this->result = result;
	}
	bool proc(Animatable *theAnim)
	{
		if (theAnim == NULL)
			return true;
		Class_ID cid = theAnim->ClassID();
		SClass_ID sid = theAnim->SuperClassID();
		MAXClass* cls = lookup_MAXClass(&cid, sid);
		if (cls == mc)
		{
			Value *item = MAXClass::make_wrapper_for((ReferenceTarget*)theAnim);
			if (item != &undefined)
				result->append(item);
		}
		return true;
	}
};

Value*
getClassInstances_cf(Value** arg_list, int count)
{
	// getClassInstances <MAXClass> target:<MAXObject> asTrackViewPick:<bool> processChildren:<bool> processAllAnimatables:<bool>
	check_arg_count_with_keys(getClassInstances, 1, count);
	MAXClass *mc = (MAXClass*)arg_list[0];
	if (!mc->is_kind_of(class_tag(MAXClass)))
		type_check(mc, MAXClass, _T("getClassInstances"));

	one_typed_value_local(Array* result);
	vl.result = new Array (0);

	Value* tmp;
	BOOL processAllAnimatables = bool_key_arg(processAllAnimatables, tmp, FALSE); 
	if (processAllAnimatables)
	{
		GetInstancesProc_AnimListEnum gip(mc, vl.result);
		Animatable::EnumerateAllAnimatables(gip);
		return_value(vl.result);
	}

	// to get a handle to the scene, or use specified target
	ReferenceMaker* target = NULL;

	BOOL asTrackViewPick = bool_key_arg(asTrackViewPick, tmp, FALSE); 

	Value* targetVal = key_arg(target)->eval();
	if (is_node(targetVal))
		target = targetVal->to_node();
	else if (targetVal != &unsupplied)
		target = targetVal->to_reftarg();
	else
		target = MAXScript_interface->GetScenePointer();
	if (target == NULL)
		return &undefined;

	bool isRoot = target == MAXScript_interface->GetScenePointer();
	if (!isRoot)
	{
		INode *node = dynamic_cast<INode*>(target);
		if (node)
			isRoot = node->IsRootNode() != FALSE;
	}
	bool quickClear = !isRoot;
	BOOL processChildren = bool_key_arg(processChildren, tmp, isRoot); 

	if (asTrackViewPick)
	{
		if (target)
		{
			if (!quickClear)
			{
				ClearAnimFlagEnumProc clearProc(A_WORK3);
				clearProc.SetScope(SCOPE_ALL);
				target->EnumAnimTree(&clearProc,NULL,0);		
			}
			else
				ClearAFlagInAllAnimatables(A_WORK3);
			int scope = processChildren? SCOPE_ALL : (SCOPE_SUBANIM|SCOPE_DOCLOSED);
			GetInstancesProc_AnimEnum gip (mc, vl.result, scope);
			target->EnumAnimTree(&gip,NULL,0);
		}
	}
	else
	{
		GetInstancesProc_RefEnum gip (mc, vl.result, processChildren);
		target->EnumRefHierarchy(gip, true, true, true, quickClear);
	}

	return_value(vl.result);
}

Value*
isMSPluginClass_cf(Value** arg_list, int count)
{
	check_arg_count(isMSPluginClass, 1, count);
	return bool_result(arg_list[0]->is_kind_of(class_tag(MSPluginClass)));
}

Value*
isMSCustAttribClass_cf(Value** arg_list, int count)
{
	check_arg_count(isMSCustAttribClass, 1, count);
	return bool_result(arg_list[0]->is_kind_of(class_tag(MSCustAttribDef)));
}

Value*
isMSPlugin_cf(Value** arg_list, int count)
{
	check_arg_count(isMSPlugin, 1, count);
	return ( arg_list[0]->is_kind_of(class_tag(MAXWrapper)) && 
			 (((MAXWrapper*)arg_list[0])->get_max_object()->GetInterface(IID_XREF_ITEM) == NULL) &&
		     ((MAXWrapper*)arg_list[0])->get_max_object()->GetInterface(I_MAXSCRIPTPLUGIN) ) ? &true_value : &false_value;
}

Value*
isMSCustAttrib_cf(Value** arg_list, int count)
{
	check_arg_count(isMSCustAttrib, 1, count);
	return (arg_list[0]->is_kind_of(class_tag(MAXWrapper)) && ((MAXWrapper*)arg_list[0])->get_max_object()->GetInterface(I_SCRIPTEDCUSTATTRIB)) ? &true_value : &false_value;
}

Value*
getClassName_cf(Value** arg_list, int count)
{
	check_arg_count(getClassName, 1, count);
	TSTR name;
	ReferenceTarget* ref = arg_list[0]->to_reftarg();
	if (ref)
		ref->GetClassName(name);
	return new String (name);
}

Value*
isMaxFile_cf(Value** arg_list, int count)
{
	check_arg_count(isMaxFile, 1, count);

	TSTR  fileName = arg_list[0]->to_filename();
	AssetId assetId;
	if (IAssetManager::GetInstance()->StringToAssetId(fileName, assetId))
		fileName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();

	return bool_result(MAXScript_interface->IsMaxFile(fileName));
}

Value*
is64bitApplication_cf(Value** arg_list, int count)
{
	check_arg_count(is64bitApplication, 0, count);
#ifdef WIN64
	return &true_value;
#else
	return &false_value;
#endif
}

//  =================================================
//  Snaps 
//  =================================================

#include "omanapi.h"
#include "osnap.h"
#include "osnaphit.h"

Value*
setSnapHilite(Value* val)
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->sethilite(val->to_point3());
	return val;
}

Value*
getSnapHilite()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	Point3 p = theman->gethilite();
	return ColorValue::intern(p.x, p.y, p.z);
}

Value*
setSnapMarkSize(Value* val)
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->setmarksize(val->to_int());
	return val;
}

Value*
getSnapMarkSize()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	return Integer::intern(theman->getmarksize());
}

Value*
setSnapToFrozen(Value* val)
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->SetSnapToFrozen(val->to_bool()==TRUE);
	return val;
}

Value*
getSnapToFrozen()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	return (theman->SnapToFrozen()) ? &true_value : &false_value;
}

Value*
setSnapAxisConstraint(Value* val)
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	if (val->to_bool())
		theman->SetAFlag(XYZ_CONSTRAINT);
	else
		theman->ClearAFlag(XYZ_CONSTRAINT);
	return val;
}

Value*
getSnapAxisConstraint()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	return (theman->TestAFlag(XYZ_CONSTRAINT)) ? &true_value : &false_value;
}

Value*
setSnapDisplay(Value* val)
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	if (val->to_bool())
		theman->SetAFlag(HIGHLIGHT_POINT | HIGHLIGHT_GEOM);
	else
		theman->ClearAFlag(HIGHLIGHT_POINT | HIGHLIGHT_GEOM);
	return val;
}

Value*
getSnapDisplay()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	return (theman->TestAFlag(HIGHLIGHT_POINT | HIGHLIGHT_GEOM)) ? &true_value : &false_value;
}

Value*
setSnapStrength(Value* val)
{
	throw RuntimeError (_T("Cannot set snap strength. Use snap preview radius instead."));
}

Value*
getSnapStrength()
{
	IOsnapManager* theman = (IOsnapManager*)GetCOREInterface()->GetOsnapManager();
	return Integer::intern(theman->GetSnapStrength());
}

Value*
setDisplaySnapRubberBand(Value* val)
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->SetDisplaySnapRubberBand(val->to_bool()==TRUE);
	return val;
}

Value*
getDisplaySnapRubberBand()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	return (theman->GetDisplaySnapRubberBand()) ? &true_value : &false_value;
}

Value*
setUseAxisCenterAsStartSnapPoint(Value* val)
{
	// Deprecated method
	return &undefined;
}

Value*
getUseAxisCenterAsStartSnapPoint()
{
	// deprecated method
	return &undefined;
}


Value*
setSnapHit(Value* val)
{
	throw RuntimeError (_T("Cannot set snap hit"));
}

Value*
getSnapHit()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	return (theman->TestAFlag(BUFFER_HOLDING)) ? &true_value : &false_value;
}

Value*
getSnapNode()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	if (!theman->getactive()) return &undefined;
	INode* hit = NULL;
	OsnapHit& theHit = theman->GetHit();
	if (&theHit) hit = theHit.GetNode();
	if (hit)
		return MAXNode::intern(hit);
	else
		return &undefined;
}

Value*
setSnapNode(Value* val)
{
	throw RuntimeError (_T("Cannot set snap node"));
}

Value*
getSnapFlags()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	if (!theman->getactive()) return &undefined;
	return Integer::intern(theman->GetSnapFlags());
}

Value*
setSnapFlags(Value* val)
{
	throw RuntimeError (_T("Cannot set snap Flags"));
}

Value*
getHitPoint()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	if (!theman->getactive()) return &undefined;
	OsnapHit& theHit = theman->GetHit();
	INode* hit = NULL;
	if (&theHit) hit = theHit.GetNode();
	if (hit)
		return new Point3Value (theHit.GetHitpoint());
	else
		return &undefined;
}

Value*
setHitPoint(Value* val)
{
	throw RuntimeError (_T("Cannot set snap hitPoint"));
}

Value*
getWorldHitpoint()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	if (!theman->getactive()) return &undefined;
	OsnapHit& theHit = theman->GetHit();
	INode* hit = NULL;
	if (&theHit) hit = theHit.GetNode();
	if (hit)
		return new Point3Value (theHit.GetWorldHitpoint());
	else
		return &undefined;
}

Value*
setWorldHitpoint(Value* val)
{
	throw RuntimeError (_T("Cannot set snap worldHitPoint"));
}

Value*
getScreenHitpoint()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	if (!theman->getactive()) return &undefined;
	OsnapHit& theHit = theman->GetHit();
	INode* hit = NULL;
	if (&theHit) hit = theHit.GetNode();
	if (hit) {
		IPoint3 hitPoint = theHit.GetHitscreen();
		return new Point3Value (hitPoint.x, hitPoint.y, hitPoint.z);
	}
	else
		return &undefined;
}

Value*
setScreenHitpoint(Value* val)
{
	throw RuntimeError (_T("Cannot set snap screenHitPoint"));
}

Value*
getSnapOKForRelativeSnap()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	if (!theman->getactive()) return &undefined;
	return (theman->OKForRelativeSnap()) ? &true_value : &false_value;
}

Value*
setSnapOKForRelativeSnap(Value* val)
{
	throw RuntimeError (_T("Cannot set OKForRelativeSnap"));
}

Value*
getSnapRefPoint()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	if (!theman->getactive() || !theman->OKForRelativeSnap()) return &undefined;
	return new Point3Value (theman->GetRefPoint(FALSE));
}

Value*
setSnapRefPoint(Value* val)
{
	throw RuntimeError (_T("Cannot set snap refPoint"));
}

Value*
getSnapTopRefPoint()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	if (!theman->getactive() || !theman->OKForRelativeSnap()) return &undefined;
	return new Point3Value (theman->GetRefPoint(TRUE));
}

Value*
setSnapTopRefPoint(Value* val)
{
	throw RuntimeError (_T("Cannot set snap topRefPoint"));
}

Value*
getNumOSnaps()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->Enumerate(RESET);//start at the beginning of the table
	int nsnaps=0;
	while(theman->Enumerate(NEXT))
		nsnaps++;
	return Integer::intern(nsnaps);
}

Value*
setNumOSnaps(Value* val)
{
	throw RuntimeError (_T("Cannot set snap NumOSnaps"));
}


Value*
getSnapRadius()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	return Integer::intern(theman->GetSnapRadius());
}

Value*
setSnapRadius(Value* val)
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->SetSnapRadius(val->to_int());
	return val;
}

Value*
setSnapPreviewRadius(Value* val)
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->SetSnapPreviewRadius(val->to_int());
	return val;
}

Value*
getSnapPreviewRadius()
{
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	return Integer::intern(theman->GetSnapPreviewRadius());
}




Value*
snap_getOSnapName_cf(Value** arg_list, int count)
{
	// <string> getOSnapName <int osnap_index>
	check_arg_count(getOSnapName, 1, count);
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->Enumerate(RESET);//start at the beginning of the table
	Osnap* thesnap = NULL;
	int nsnaps=0;
	int which = arg_list[0]->to_int();
	while((thesnap = theman->Enumerate(NEXT)) != NULL) {
		nsnaps++;
		if (nsnaps == which) {
			return new String (thesnap->Category());
		}
	}
	range_check(which, 1, nsnaps, _T("Object snap index out of range: "));
	return &undefined;
}

Value*
snap_getOSnapNumItems_cf(Value** arg_list, int count)
{
	// <string> getOSnapNumItems <int osnap_index>
	check_arg_count(getOSnapNumItems, 1, count);
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->Enumerate(RESET);//start at the beginning of the table
	Osnap* thesnap = NULL;
	int nsnaps=0;
	int which = arg_list[0]->to_int();
	while((thesnap = theman->Enumerate(NEXT)) != NULL) {
		nsnaps++;
		if (nsnaps == which) {
			return Integer::intern(thesnap->numsubs());
		}
	}
	range_check(which, 1, nsnaps, _T("Object snap index out of range: "));
	return &undefined;
}

Value*
snap_getOSnapItemName_cf(Value** arg_list, int count)
{
	// <string> getOSnapItemName <int osnap_index> <int osnap_item_index>
	check_arg_count(getOSnapItemName, 2, count);
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->Enumerate(RESET);//start at the beginning of the table
	Osnap* thesnap = NULL;
	int nsnaps=0;
	int which = arg_list[0]->to_int();
	int item = arg_list[1]->to_int();
	while((thesnap = theman->Enumerate(NEXT)) != NULL) {
		nsnaps++;
		if (nsnaps == which) {
			int nitems = thesnap->numsubs();
			range_check(item, 1, nitems, _T("Object snap item index out of range: "));
			TSTR* snapname = thesnap->snapname(item-1);
			return new String ((snapname) ? snapname->data() : _T(""));
		}
	}
	range_check(which, 1, nsnaps, _T("Object snap index out of range: "));
	return &undefined;
}

Value*
snap_getOSnapItemToolTip_cf(Value** arg_list, int count)
{
	// <string> getOSnapItemToolTip <int osnap_index> <int osnap_item_index>
	check_arg_count(getOSnapItemToolTip, 2, count);
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->Enumerate(RESET);//start at the beginning of the table
	Osnap* thesnap = NULL;
	int nsnaps=0;
	int which = arg_list[0]->to_int();
	int item = arg_list[1]->to_int();
	while((thesnap = theman->Enumerate(NEXT)) != NULL) {
		nsnaps++;
		if (nsnaps == which) {
			int nitems = thesnap->numsubs();
			range_check(item, 1, nitems, _T("Object snap item index out of range: "));
			TSTR* tooltip = thesnap->tooltip(item-1);
			return new String ((tooltip) ? tooltip->data() : _T(""));
		}
	}
	range_check(which, 1, nsnaps, _T("Object snap index out of range: "));
	return &undefined;
}

class MXS_IOsnap
{
public:
	Osnap* osnap;
	MXS_IOsnap(Osnap* osnap){this->osnap=osnap;}
	~MXS_IOsnap() {};
	BOOL GetActive(int index){return osnap->GetActive(index);}
	void SetActive(int index, BOOL state){osnap->SetActive(index, state);}

};

Value*
snap_getOSnapItemActive_cf(Value** arg_list, int count)
{
	// <string> getOSnapItemActive <int osnap_index> <int osnap_item_index>
	check_arg_count(getOSnapItemActive, 2, count);
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->Enumerate(RESET);//start at the beginning of the table
	Osnap* thesnap = NULL;
	int nsnaps=0;
	int which = arg_list[0]->to_int();
	int item = arg_list[1]->to_int();
	while((thesnap = theman->Enumerate(NEXT)) != NULL) {
		nsnaps++;
		if (nsnaps == which) {
			int nitems = thesnap->numsubs();
			range_check(item, 1, nitems, _T("Object snap item index out of range: "));
			MXS_IOsnap osnap (thesnap);
			return (osnap.GetActive(item-1)) ? &true_value : &false_value;
		}
	}
	range_check(which, 1, nsnaps, _T("Object snap index out of range: "));
	return &undefined;
}

Value*
snap_setOSnapItemActive_cf(Value** arg_list, int count)
{
	// <string> setOSnapItemActive <int osnap_index> <int osnap_item_index>
	check_arg_count(setOSnapItemActive, 3, count);
	IOsnapManager7* theman = IOsnapManager7::GetInstance();
	theman->Enumerate(RESET);//start at the beginning of the table
	Osnap* thesnap = NULL;
	int nsnaps=0;
	int which = arg_list[0]->to_int();
	int item = arg_list[1]->to_int();
	while((thesnap = theman->Enumerate(NEXT)) != NULL) {
		nsnaps++;
		if (nsnaps == which) {
			int nitems = thesnap->numsubs();
			range_check(item, 1, nitems, _T("Object snap item index out of range: "));
			MXS_IOsnap osnap (thesnap);
			osnap.SetActive(item-1,arg_list[2]->to_bool());
			return &ok;
		}
	}
	range_check(which, 1, nsnaps, _T("Object snap index out of range: "));
	return &undefined;
}

#include "gencam.h"

#define FOV_W 0	  // width-related FOV
#define FOV_H 1   // height-related FOV
#define FOV_D 2   // diagonal-related FOV

static float GetAspect() {
	return GetCOREInterface()->GetRendImageAspect();
};

static float GetApertureWidth() {
	return GetCOREInterface()->GetRendApertureWidth();
}

Value*
cameraFOV_MMtoFOV_cf(Value** arg_list, int count)
{
	// <float> MMtoFOV <float mm>
	check_arg_count(MMtoFOV, 1, count);
	float mm = arg_list[0]->to_float();
	float fov = float(2.0f*atan(0.5f*GetApertureWidth()/mm));
	return Float::intern(RadToDeg(fov));
}

Value*
cameraFOV_FOVtoMM_cf(Value** arg_list, int count)
{
	// <float> FOVtoMM <float fov>
	check_arg_count(FOVtoMM, 1, count);
	float fov = DegToRad(arg_list[0]->to_float());
	float mm = float((0.5f*GetApertureWidth())/tan(fov/2.0f));
	return Float::intern(mm);
}

Value*
cameraFOV_CurFOVtoFOV_cf(Value** arg_list, int count)
{
	// <float> CurFOVtoWFOV <camera> <float cfov>
	check_arg_count(CurFOVtoWFOV, 2, count);
	INode* node = get_valid_node(arg_list[0], CurFOVtoWFOV);
	Object* obj = Get_Object_Or_XRef_BaseObject(node->GetObjectRef());
	Class_ID cid = obj->ClassID();
	SClass_ID sid = obj->SuperClassID();
	GenCamera *cam;
	if (sid == CAMERA_CLASS_ID && (cid == Class_ID(LOOKAT_CAM_CLASS_ID, 0) || cid == Class_ID(SIMPLE_CAM_CLASS_ID, 0)))
		cam = (GenCamera*)obj;
	else
		throw RuntimeError (GetString(IDS_NEED_CAMERA), arg_list[0]);
	int fovType = cam->GetFOVType();
	float cfov = DegToRad(arg_list[1]->to_float());
	float fov;
	switch (fovType) 
	{
	case FOV_H: 
		fov = float(2.0*atan(GetAspect()*tan(cfov/2.0f)));
		break;
	case FOV_D:
		{
			float w = GetApertureWidth();
			float h = w/GetAspect();
			float d = (float)sqrt(w*w + h*h);	
			fov = float(2.0*atan((w/d)*tan(cfov/2.0f)));
			break;
		}
	default:
		fov = cfov;
	}
	return Float::intern(RadToDeg(fov));
}

Value*
cameraFOV_FOVtoCurFOV_cf(Value** arg_list, int count)
{
	// <float> FOVtoCurFOV <camera> <float fov>
	check_arg_count(WFOVtoCurFOV, 2, count);
	INode* node = get_valid_node(arg_list[0], FOVtoCurFOV);
	Object* obj = Get_Object_Or_XRef_BaseObject(node->GetObjectRef());
	Class_ID cid = obj->ClassID();
	SClass_ID sid = obj->SuperClassID();
	GenCamera *cam;
	if (sid == CAMERA_CLASS_ID && (cid == Class_ID(LOOKAT_CAM_CLASS_ID, 0) || cid == Class_ID(SIMPLE_CAM_CLASS_ID, 0)))
		cam = (GenCamera*)obj;
	else
		throw RuntimeError (GetString(IDS_NEED_CAMERA), arg_list[0]);
	int fovType = cam->GetFOVType();
	float fov = DegToRad(arg_list[1]->to_float());
	float cfov;
	switch (fovType) 
	{
	case FOV_H:
		cfov = float(2.0*atan(tan(fov/2.0f)/GetAspect()));
		break;
	case FOV_D: 
		{
			float w = GetApertureWidth();
			float h = w/GetAspect();
			float d = (float)sqrt(w*w + h*h);	
			cfov =  float(2.0*atan((d/w)*tan(fov/2.0f)));
			break;
		}
	default:
		cfov = fov;
	}

	return Float::intern(RadToDeg(cfov));
}

static BOOL MyComparePixels(float p1, float p2, float var)
{
	float dif = fabsf(p1 - p2);
	return (dif > var);
}


Value* CompareBitmaps_cf(Value **arg_list, int count)
{
	// CompareBitmaps <filename>File1 <filename>File2 <int>tolerance(# different pixels) <int>variation(0-255) useAlpha:<boolean> errorMsg:<&string>
	// CompareBitmaps <bitmap>bitmap1 <bitmap>bitmap2 <int>tolerance(# different pixels) <int>variation(0-255) useAlpha:<boolean> errorMsg:<&string>
	check_arg_count_with_keys(CompareBitmaps, 4, count);

	TSTR myStr = _T("");
	bool res = false;

	Bitmap *bm1 = NULL, *bm2 = NULL;
	BitmapInfo *bi1 = NULL, *bi2 = NULL;
	BMMRES isbm1Loaded, isbm2Loaded;
	BOOL SuccessLoadingBitmap = TRUE;
	bool isbm1MAXBitmap=false, isbm2MAXBitmap=false;

	long int mycount = 0;

	int tolerance = arg_list[2]->to_int();
	float variance = (arg_list[3]->to_float())/255.0;

	BOOL useAlpha = key_arg_or_default(useAlpha,&false_value)->to_bool();

	Value* errorMsgValue = key_arg(errorMsg);
	Thunk* errorMsgThunk = NULL;
	if (errorMsgValue != &unsupplied && errorMsgValue->_is_thunk()) 
		errorMsgThunk = errorMsgValue->to_thunk();

	TSTR bi1Name;
	TSTR bi2Name;

	// two step it here so that if an error is thrown, we don't leak memory
	if (!is_bitmap(arg_list[0]))
	{
		bi1Name = arg_list[0]->to_filename();
		AssetId assetId;
		if (IAssetManager::GetInstance()->StringToAssetId(bi1Name, assetId))
			bi1Name = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();
	}
	if (!is_bitmap(arg_list[1]))
	{
		bi2Name = arg_list[1]->to_filename();
		AssetId assetId;
		if (IAssetManager::GetInstance()->StringToAssetId(bi2Name, assetId))
			bi2Name = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();
	}

	// from here on we won't throw
	if (is_bitmap(arg_list[0]))
	{
		MAXBitMap *mbm = (MAXBitMap*)arg_list[0];
		bi1 = &mbm->bi;
		bi1Name = bi1->Filename();
		bm1 = mbm->bm;
		isbm1Loaded = BMMRES_SUCCESS;
		isbm1MAXBitmap=true;
	}
	else
	{
		bi1 = new BitmapInfo (bi1Name);
		TempBitmapManagerSilentMode tbmsm;
		bm1 = TheManager->Load(bi1,&isbm1Loaded);
	}
	if (is_bitmap(arg_list[1]))
	{
		MAXBitMap *mbm = (MAXBitMap*)arg_list[1];
		bi2 = &mbm->bi;
		bi2Name = bi2->Filename();
		bm2 = mbm->bm;
		isbm2Loaded = BMMRES_SUCCESS;
		isbm2MAXBitmap=true;
	}
	else
	{
		bi2 = new BitmapInfo (bi2Name);
		TempBitmapManagerSilentMode tbmsm;
		bm2 = TheManager->Load(bi2,&isbm2Loaded);
	}

	if (isbm1Loaded == BMMRES_SUCCESS && isbm2Loaded == BMMRES_SUCCESS && bm1 && bm2 && bi1 && bi2)
	{

		int biWidth = bi1->Width();
		int biHeight = bi1->Height();
		int bi2Width = bi2->Width();
		int bi2Height = bi2->Height();
		if ((biWidth == bi2Width) && (biHeight == bi2Height))
		{
			// iterate through pixels

			BMM_Color_fl c1, c2;

			int max_c = useAlpha ? 4 : 3;

			for (int i = 0; i < biWidth; i++)
			{
				for (int j = 0; j < biHeight; j++)
				{
					bm1->GetPixels(i,j,1,&c1);
					bm2->GetPixels(i,j,1,&c2);

					float pi1[4] = {c1.r,c1.g,c1.b,c1.a};
					float pi2[4] = {c2.r,c2.g,c2.b,c2.a};
					for (int c = 0; c < max_c; c++)
					{
						BOOL isDiff = MyComparePixels(pi1[c], pi2[c], variance);
						if (isDiff)
						{
							mycount++;
							break;
						}
					}
				}
			}
			if (mycount > tolerance)
				myStr.printf(GetString(IDS_DIFFERENT_PIXEL_COUNT), mycount);
			else 
			{
				myStr = GetString(IDS_BITMAP_COMPARE_PASSED);
				res = true;
			}

		}
		else 
			myStr = GetString(IDS_BITMAP_COMPARE_DIFF_SIZES);
		if (!isbm1MAXBitmap)
		{
			if (bi1) delete bi1;
			if (bm1) bm1->DeleteThis();
		}
		if (!isbm2MAXBitmap)
		{
			if (bi2) delete bi2;
			if (bm2) bm2->DeleteThis();
		}

	} 
	else 
		myStr = GetString(IDS_BITMAP_COMPARE_NO_BITMAP);


	if (errorMsgThunk)
		errorMsgThunk->assign(new String(myStr));
	return bool_result(res);
}

#define WIDEPRINT_BUF_WIDTH 8192

Value*
formattedPrint_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(formattedPrint, 1, count);
	Value* arg = arg_list[0];
	TSTR format;
	TSTR defaultFloatFormat = _T("%0.8g");
	TSTR Lbracket = _T("[");
	TSTR Rbracket = _T("]");
	TSTR Lparen = _T("(");
	TSTR Rparen = _T(")");
	TSTR space = _T(" ");
	TSTR comma = _T(",");
	TSTR f = _T("f");
	TSTR p = _T("%");
	TSTR formatSpec;
	bool formatGiven = false;
	Value* formatValue = key_arg(format)->eval();
	if (formatValue != &unsupplied)
	{
		formatSpec = formatValue->to_string();
		format = p+formatSpec;
		formatGiven = true;
	}

	set_english_numerics();
	TCHAR buf[WIDEPRINT_BUF_WIDTH];
	buf[0] = '\0';
	if (is_float(arg))
	{
		if (!formatGiven) 
			format = defaultFloatFormat;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, format, EPS(arg->to_float()));
	}
	else if (is_double(arg))
	{
		if (!formatGiven) 
			format = defaultFloatFormat;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, format, EPS(arg->to_double()));
	}
	else if (is_int(arg))
	{
		if (!formatGiven) 
			format = _T("%I32d");
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, format, arg->to_int());
	}
	else if (is_int64(arg))
	{
		if (!formatGiven) 
			format = _T("%I64d");
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, format, arg->to_int64());
	}
	else if (is_intptr(arg))
	{
		if (!formatGiven) 
			format = _T("%Id");
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, format, arg->to_intptr());
	}
	else if (is_string(arg))
	{
		if (!formatGiven) 
			format = _T("%s");
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, format, arg->to_string());
	}
	else if (is_bool(arg))
	{
		if (!formatGiven) 
			format = _T("%d");
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, format, arg->to_bool());
	}
	else if (is_ray(arg))
	{
		Ray r = arg->to_ray();
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatRay = _T("(ray [");
		formatRay += format+comma+format+comma+format+Rbracket+space+Lbracket+format+comma+format+comma+format+Rbracket+Rparen;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatRay, EPS(r.p.x), EPS(r.p.y), EPS(r.p.z), EPS(r.dir.x), EPS(r.dir.y), EPS(r.dir.z));
	}
	else if (is_point2(arg))
	{
		Point2 p = arg->to_point2();
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatP3 = Lbracket+format+comma+format+Rbracket;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatP3, EPS(p.x), EPS(p.y));
	}
	else if (is_point3(arg))
	{
		Point3 p = arg->to_point3();
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatP3 = Lbracket+format+comma+format+comma+format+Rbracket;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatP3, EPS(p.x), EPS(p.y), EPS(p.z));
	}
	else if (is_point4(arg))
	{
		Point4 p = arg->to_point4();
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatP4 = Lbracket+format+comma+format+comma+format+comma+format+Rbracket;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatP4, EPS(p.w), EPS(p.x), EPS(p.y), EPS(p.z));
	}
	else if (is_quat(arg))
	{
		Quat q = arg->to_quat();
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatQ = _T("(quat ");
		formatQ += format+space+format+space+format+space+format+Rparen;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatQ, EPS(q.x), EPS(q.y), EPS(q.z), EPS(q.w));
	}
	else if (is_angaxis(arg))
	{
		AngAxis aa = arg->to_angaxis();
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatAA= _T("(angleAxis ");
		formatAA += format+space+Lbracket+format+comma+format+comma+format+Rbracket+Rparen;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatAA, EPS(RadToDeg(aa.angle)), EPS(aa.axis.x), EPS(aa.axis.y), EPS(aa.axis.z));
	}
	else if (is_eulerangles(arg))
	{
		EulerAnglesValue* eav = (EulerAnglesValue*)arg;
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatEA = _T("(eulerAngles ");
		formatEA += format+space+format+space+format+Rparen;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatEA, EPS(RadToDeg(eav->angles[0])), EPS(RadToDeg(eav->angles[1])), EPS(RadToDeg(eav->angles[2])));
	}
	else if (is_matrix3(arg))
	{
		Matrix3 m = arg->to_matrix3();
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatM3= _T("(matrix3");
		formatM3 += space+Lbracket+format+comma+format+comma+format+Rbracket +
			space+Lbracket+format+comma+format+comma+format+Rbracket +
			space+Lbracket+format+comma+format+comma+format+Rbracket+Rparen;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatM3, 
			EPS(m.GetRow(0).x), EPS(m.GetRow(0).y), EPS(m.GetRow(0).z),
			EPS(m.GetRow(1).x), EPS(m.GetRow(1).y), EPS(m.GetRow(1).z),
			EPS(m.GetRow(2).x), EPS(m.GetRow(2).y), EPS(m.GetRow(2).z),
			EPS(m.GetRow(3).x), EPS(m.GetRow(3).y), EPS(m.GetRow(3).z));

	}
	else if (is_box2(arg))
	{
		Box2 b = arg->to_box2();
		if (!formatGiven) 
			format = _T("%d");
		TSTR formatB2 = _T("(box2 ");
		formatB2 += format+space+format+space+format+space+format+Rparen;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatB2, b.x(), b.y(), b.w(), b.h());
	}
	else if (is_color(arg))
	{
		AColor color = arg->to_acolor();
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatC = _T("(color ");
		formatC += format+space+format+space+format;
		if (color.a != 1.0f)
			formatC += space+format;
		formatC += Rparen;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatC, EPS(color.r * 255.0f), EPS(color.g * 255.0f), EPS(color.b * 255.0f), EPS(color.a * 255.0f));
	}
	else if (is_time(arg))
	{
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatT = format+f;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatT, EPS(arg->to_timevalue() / (float)GetTicksPerFrame()));
	}
	else if (is_interval(arg))
	{
		Interval interval = arg->to_interval();
		if (!formatGiven) 
			format = defaultFloatFormat;
		TSTR formatT = _T("(interval ");
		formatT += format+f+space+format+f+Rparen;
		_snprintf(buf, WIDEPRINT_BUF_WIDTH, formatT, 		
			EPS(interval.Start() / (float)GetTicksPerFrame()), 
			EPS(interval.End() / (float)GetTicksPerFrame()));
	}

	reset_numerics();
	return new String(buf);
}

// ----------------------- usedMaps -----------------------------
class INodeEnum
{
public:
	virtual int proc(INode *node)=0;
};

int INodeEnumTree(INode *node, INodeEnum& inodeEnum) 
{
	if (node==NULL) 
		return 1;
	if (!inodeEnum.proc(node))
		return 0;
	for (int i=0; i<node->NumberOfChildren(); i++) 
		if (!INodeEnumTree(node->GetChildNode(i), inodeEnum)) 
			return 0;
	return 1;
}

// Subclass a class of AssetEnumCallback that appends to the list
class MyNameEnumCallback : public AssetEnumCallback
{
	NameTab *theList;
public:
	MyNameEnumCallback(NameTab* list) { theList = list; }
	void RecordAsset(const MaxSDK::AssetManagement::AssetUser&asset)
	{
		TSTR fullPath = asset.GetFullFilePath();
		if(fullPath.isNull())
			fullPath = asset.GetFileName();
		theList->AddName(fullPath);
	}
};

class NodeAuxFileEnum: public INodeEnum 
{
public:
	MyNameEnumCallback &mne;
	DWORD flags;
	NodeAuxFileEnum(MyNameEnumCallback& _mne, DWORD _flags) : mne(_mne), flags(_flags){}
	int  proc(INode *node) 
	{
		node->EnumAuxFiles(mne, flags);	
		return 1;
	}
};

Value*
used_maps_cf(Value** arg_list, int count)
{
	if (count > 1)
		check_arg_count(usedMaps, 1, count);
	one_typed_value_local(Array* result);

	int i;
	NameTab theList; // a list to hold the files
	MyNameEnumCallback mne(&theList);
	vl.result = new Array (0);
	DWORD flags = FILE_ENUM_VP|FILE_ENUM_RENDER|FILE_ENUM_CHECK_AWORK1;

	if (count > 0)
	{
		if (!arg_list[0]->is_kind_of(class_tag(MAXWrapper)))
			return &ok;
		ReferenceTarget* obj = is_node(arg_list[0]) ? (ReferenceTarget*)arg_list[0]->to_node() : arg_list[0]->to_reftarg();

		if (obj == NULL)
			return &ok;
		// First clear all A_WORK1 flags
		ClearAFlagInHierarchy(obj,A_WORK1); // this flag is used by EnumAuxFiles
		IFileResolutionManager::GetInstance()->PushAllowCachingOfUnresolvedResults(true);
		obj->EnumAuxFiles(mne, flags);
		IFileResolutionManager::GetInstance()->PopAllowCachingOfUnresolvedResults();
	}
	else
	{
		// to get a handle to the scene
		ReferenceTarget* scene = MAXScript_interface->GetScenePointer();

		// First clear all A_WORK1 flags
		ClearAFlagInAllAnimatables(A_WORK1); // this flag is used by EnumAuxFiles

		// now call EnumAuxFiles on all references except the material editor(Ref 1)		
		for (i=1; i < scene->NumRefs(); i++)
		{
			RefTargetHandle ref = scene->GetReference(i);
			IFileResolutionManager::GetInstance()->PushAllowCachingOfUnresolvedResults(true);
			if (ref) ref->EnumAuxFiles(mne, flags);
			IFileResolutionManager::GetInstance()->PopAllowCachingOfUnresolvedResults();
		}

		NodeAuxFileEnum nodeAuxFileEnum(mne, flags);
		INodeEnumTree(MAXScript_interface->GetRootNode(),nodeAuxFileEnum);
	}	
	for (i=0; i < theList.Count(); i++)
		vl.result->append(new String(theList[i]));
	return_value(vl.result);
}

//  =================================================
//  Dynamics space warp force access
//  =================================================

Value*
WSMSupportsForce_cf(Value** arg_list, int count)
{
	check_arg_count(WSMSupportsForce, 1, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], WSMSupportsForce);
	const ObjectState& os = node->EvalWorldState(MAXScript_time());
	Object* ob = os.obj;
	if (ob) {
		int id = (ob ? ob->SuperClassID() : SClass_ID(0));
		if (id == WSM_OBJECT_CLASS_ID) {
			WSMObject *obref = (WSMObject*)node->GetObjectRef();
			ForceField *ff = obref->GetForceField(node);
			if (ff) return &true_value;
		}
	}
	return &false_value;
}


Value*
WSMSupportsCollision_cf(Value** arg_list, int count)
{
	check_arg_count(WSMSupportsCollision, 1, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], WSMSupportsCollision);
	const ObjectState& os = node->EvalWorldState(0);
	Object* ob = os.obj;
	if (ob) {
		int id = (ob ? ob->SuperClassID() : SClass_ID(0));
		if (id == WSM_OBJECT_CLASS_ID) {
			Class_ID classID = ob->ClassID();
			if (classID == DEFLECTOBJECT_CLASSID || classID == PLANARSPAWNDEF_CLASSID ||
				classID == SSPAWNDEFL_CLASSID    || classID == USPAWNDEFL_CLASSID     ||
				classID == SPHEREDEF_CLASSID     || classID == UNIDEF_CLASSID) 
				return &true_value;
		}
	}
	return &false_value;
}

Value*
WSMApplyFC_cf(Value** arg_list, int count)
{
	// WSMApplyFC <WSM_node_array> <pos_array> <vel_array> <force_array> interval:<interval> stepsize:<time> 
	// note: units for velocity is units/tick
	HoldSuspend methodHoldSuspend;
	check_arg_count_with_keys(WSMApplyFC, 4, count);
	TimeValue t = MAXScript_time();
	TimeValue tpf = GetTicksPerFrame();
	Interval defInterval = Interval(t, t+tpf); 
	Value* tmp;
	Interval overInterval = interval_key_arg(interval, tmp, defInterval); 
	if (overInterval.Start() >= overInterval.End())
		overInterval.SetEnd(overInterval.Start()+1);
	TimeValue stepsize = timevalue_key_arg(stepsize, tmp, tpf); 
	Array* wsmArray = NULL;
	Array* posArray = NULL;
	Array* velArray = NULL;
	Array* forceArray = NULL;
	Point3 *thePos = 0;
	Point3 *theVel = 0;
	Point3 *theForce = 0;
	Point3 force, sumForce;
	int i, j, nPts;
	Point3 collPos, collVel, collPosF, collVelF;
	float collTime, collTimeF;
	Class_ID classID;

	// position array
	if (arg_list[1]->is_kind_of(class_tag(Array))) {
		posArray = (Array*)arg_list[1];
	}
	else  
		ConversionError (arg_list[1], _T("Array"));
	nPts=posArray->size;
	// velocity array
	if (arg_list[2]->is_kind_of(class_tag(Array))) {
		velArray = (Array*)arg_list[2];
	}
	else  
		ConversionError (arg_list[2], _T("Array"));
	if (nPts != velArray->size)
		throw RuntimeError (GetString(IDS_WSMAPPLYFC_ARRAYS_NOT_SAME_SIZE));
	// force array
	if (arg_list[3]->is_kind_of(class_tag(Array))) {
		forceArray = (Array*)arg_list[3];
	}
	else  
		ConversionError (arg_list[3], _T("Array"));
	if (nPts != forceArray->size)
		throw RuntimeError (GetString(IDS_WSMAPPLYFC_ARRAYS_NOT_SAME_SIZE));

	// WSM array
	if (arg_list[0]->is_kind_of(class_tag(Array))) {
		wsmArray = (Array*)arg_list[0];
	}
	else  
		ConversionError (arg_list[0], _T("Array"));

	for (i = 0; i < wsmArray->size; i++) 
		type_check(wsmArray->data[i], MAXNode, "WSMApplyFC [WSM array element]");

	ForceField *ff;
	Tab<ForceField*> forceFields;
	ICollisionObject *co;
	Tab<ICollisionObject*> collisionObjects;
	IDeflectMod* deflectMod;
	Tab<IDeflectMod*> deflectModifiers;

	for (i = 0; i < wsmArray->size; i++) {
		INode* node = get_valid_node((MAXNode*)wsmArray->data[i], WSMApplyFC);
		const ObjectState& os = node->EvalWorldState(MAXScript_time());
		Object* ob = os.obj;
		ff = NULL;
		co = NULL;
		deflectMod = NULL;
		if (ob) {
			int id = (ob ? ob->SuperClassID() : SClass_ID(0));
			if (id == WSM_OBJECT_CLASS_ID) {
				WSMObject* obref = (WSMObject*)node->GetObjectRef();
				ff = obref->GetForceField(node);
				if (ff) {
					forceFields.Append(1,&ff);
				}
				else {  // no force field, check to see if deflector type WSM
					classID = ob->ClassID();
					if (classID == DEFLECTOBJECT_CLASSID || classID == PLANARSPAWNDEF_CLASSID ||
						classID == SSPAWNDEFL_CLASSID    || classID == USPAWNDEFL_CLASSID     ||
						classID == SPHEREDEF_CLASSID     || classID == UNIDEF_CLASSID) {
							BOOL was_holding = theHold.Holding();
							if (was_holding) theHold.Suspend();
							deflectMod = (IDeflectMod*)obref->CreateWSMMod(node);
							if (was_holding) theHold.Resume();
					}
					if (deflectMod) {
						co = &deflectMod->deflect;
						co->obj  = (SimpleWSMObject*)deflectMod->GetWSMObject(t);
						co->node = node;
						if (classID == DEFLECTOBJECT_CLASSID) {
							DeflectorField* deflect = (DeflectorField*)co;
							deflect->tmValid.SetEmpty();		
						}
						else if (classID == PLANARSPAWNDEF_CLASSID) {
							PSpawnDeflField* deflect = (PSpawnDeflField*)co;
							deflect->tmValid.SetEmpty();		
							if (t <= deflect->obj->t) deflect->obj->lastrnd = 12345;
							deflect->obj->t=t;
						}
						else if (classID == SSPAWNDEFL_CLASSID) {
							SSpawnDeflField* deflect= (SSpawnDeflField*)co;
							deflect->tmValid.SetEmpty();
							if (t <= deflect->obj->t) deflect->obj->lastrnd = 12345;
							deflect->obj->t=t;
						}
						else if (classID == USPAWNDEFL_CLASSID) {
							USpawnDeflField* deflect = (USpawnDeflField*)co;
							deflect->obj->tmValid.SetEmpty();		
							deflect->obj->mValid.SetEmpty();
							deflect->badmesh = (deflect->obj->custnode == NULL);
							if (t <= deflect->obj->t) deflect->obj->lastrnd = 12345;
							deflect->obj->t = t;
							deflect->obj->dvel = Zero;
						}
						else if (classID == SPHEREDEF_CLASSID) {
							SphereDeflectorField* deflect = (SphereDeflectorField*)co;
							deflect->tmValid.SetEmpty();	
							if (t <= deflect->obj->t) deflect->obj->lastrnd = 12345;
							deflect->obj->t = t;
						}
						else { // if (classID == UNIDEF_CLASSID) {
							UniDeflectorField* deflect = (UniDeflectorField*)co;
							deflect->obj->tmValid.SetEmpty();		
							deflect->obj->mValid.SetEmpty();
							deflect->badmesh = (deflect->obj->custnode == NULL);
							if (t <= deflect->obj->t) deflect->obj->lastrnd = 12345;
							deflect->obj->t = t;
							deflect->obj->dvel = Zero;
						}
						/* can't do path follow since it needs a particle system
						else { //if (classID == PFOLLOW_CLASSID) {
						PFollowField* deflect = (PFollowField*)co;
						deflect->tmValid.SetEmpty();		
						}
						*/						
						collisionObjects.Append(1,&co);
						deflectModifiers.Append(1,&deflectMod);
					}
				}
			}
		}
		if (!deflectMod && !ff) {
			for (j = 0; j < forceFields.Count(); j++) forceFields[j]->DeleteThis();
			for (j = 0; j < deflectModifiers.Count(); j++) {
				deflectModifiers[j]->DeleteAllRefs();
				deflectModifiers[j]->DeleteThis();
			}
			throw RuntimeError (GetString(IDS_FORCE_NOT_FOUND), wsmArray->data[i]);
		}
	}


	thePos   = new Point3[nPts];
	theVel   = new Point3[nPts];
	theForce = new Point3[nPts];

	// any throws need to delete above arrays, the ForceFields, the CollisionObjects, and the DeflectorMods 
	try
	{
		for (i = 0; i < nPts; i++) {
			thePos[i]   = posArray->data[i]->to_point3();
			theVel[i]   = velArray->data[i]->to_point3();
			theForce[i] = forceArray->data[i]->to_point3();
		}
		for (t = overInterval.Start(); t < overInterval.End(); t+=stepsize) {
			for (i = 0; i < nPts; i++) {
				sumForce = theForce[i];
				for (j = 0; j < forceFields.Count(); j++) {
					force = 10.f*forceFields[j]->Force(t, thePos[i], theVel[i], 0);
					sumForce += force;
				}
				theVel[i] += force*(float)stepsize;
				BOOL collided = FALSE;
				collTimeF = (float)stepsize;
				for (j = 0; j < collisionObjects.Count(); j++) {
					collPos = thePos[i];
					collVel = theVel[i];
					if (collisionObjects[j]->CheckCollision(t, collPos, collVel, (float)stepsize, 0, &collTime, FALSE)) {
						collided = TRUE;
						if (collTime < collTimeF) {
							collPosF  = collPos+((float)stepsize-collTime)*collVel;
							collVelF  = collVel;
							collTimeF = collTime;
						}
					}
				}
				if (collided) {
					thePos[i] = collPosF;
					theVel[i] = collVelF;
				}
				else {
					thePos[i] += theVel[i]*(float)stepsize;
				}
			}
		}
		for (i = 0; i < nPts; i++) {
			posArray->data[i]   = new (GC_IN_HEAP) Point3Value(thePos[i]);
			velArray->data[i]   = new (GC_IN_HEAP) Point3Value(theVel[i]);
			forceArray->data[i] = new (GC_IN_HEAP) Point3Value(theForce[i]);
		}
	}
	catch (...)
	{
		if (thePos)   delete []thePos;
		if (theVel)   delete []theVel;
		if (theForce) delete []theForce;
		for (j = 0; j < forceFields.Count(); j++) forceFields[j]->DeleteThis();
		for (j = 0; j < deflectModifiers.Count(); j++) {
			deflectModifiers[j]->DeleteAllRefs();
			deflectModifiers[j]->DeleteThis();
		}
		throw;
	}

	if (thePos)   delete []thePos;
	if (theVel)   delete []theVel;
	if (theForce) delete []theForce;
	for (j = 0; j < forceFields.Count(); j++) forceFields[j]->DeleteThis();
	for (j = 0; j < deflectModifiers.Count(); j++) {
		deflectModifiers[j]->DeleteAllRefs();
		deflectModifiers[j]->DeleteThis();
	}
	return &ok;
}

/*
WSMGetForce $gravity01 posArray velArray forceArray
posArray[1]
velArray[1]
forceArray[1]
point pos:(posArray[1])
particleVelocity $SuperSpray01 1
at time (currenttime+1) particlePos $SuperSpray01 1
$SuperSpray01.pos

(slidertime=4
posArray=#(particlePos $SuperSpray01 1)
velArray=#(at time (currenttime-1) particleVelocity $SuperSpray01 1)
forceArray=#([0,0,0])
WSMApplyFC #($gravity01,$deflector01) posArray velArray forceArray
format "% : %\n" posArray[1] velArray[1]
format "% : % : %\n" (at time (currenttime-1) particlePos $SuperSpray01 1) (particlePos $SuperSpray01 1) (at time (currenttime+1) particlePos $SuperSpray01 1)
format "% : % : %\n" (at time (currenttime-1) particleVelocity $SuperSpray01 1) (particleVelocity $SuperSpray01 1) (at time (currenttime+1) particleVelocity $SuperSpray01 1)
)
*/

//  =================================================
//  Miscellaneous Stuff
//  =================================================

Value*
quatToEuler2_cf(Value** arg_list, int count)
{
	check_arg_count(quatToEuler2, 1, count);
	Quat q = arg_list[0]->to_quat();
	float *res = new float [3];
	QuatToEuler(q, res);
	Value* result = new EulerAnglesValue(res[0], res[1], res[2]);
	delete [] res;
	return result;
}

Value*
close_enough_cf(Value** arg_list, int count)
{
	check_arg_count(close_enough, 3, count);
	float f1 = arg_list[0]->to_float();
	float f2 = arg_list[1]->to_float();
	float nrounds = arg_list[2]->to_float();
	float eps = nrounds*std::numeric_limits<float>::epsilon();
	//	float t1 = 2.0f*fabs(f1-f2)/(fabs(f1)+fabs(f2));
	bool res = f1 == f2 || (2.0f*fabs(f1-f2)/(fabs(f1)+fabs(f2))) < eps;
	return (res) ? &true_value : &false_value;
	//	return Float::intern(t1); 
}

Value*
quatArrayToEulerArray_cf(Value** arg_list, int count)
{
	check_arg_count(quatArrayToEulerArray, 1, count);
	type_check(arg_list[0], Array, "quatArrayToEulerArray");
	Array* theArray = (Array*)arg_list[0];
	one_typed_value_local(Array* result);
	vl.result = new Array (theArray->size);
	float twopi = 2.0f*pi;
	Matrix3 tm;
	Quat qIn, qFirst;
	SimpleDataStore<float> eLast(3);
	SimpleDataStore<float> e(3);
	for (int i = 0; i < theArray->size; i++) {
		qIn = theArray->data[i]->to_quat();
		if (i == 0) {
			qFirst=qIn;
			qIn.MakeMatrix(tm);
			MatrixToEuler(tm, eLast.data,0);
		}
		else {
			qIn.MakeClosest(qFirst);
			qIn.MakeMatrix(tm);
			MatrixToEuler(tm, e.data,0);
			for (int j = 0; j < 3; j++) {
				int revs = (int)(eLast[j]/twopi);
				float a = eLast[j]-(float)revs*twopi;
				float delta = e[j]-a;
				float r = (fabs(delta) > pi) ? (_copysign(twopi,a)+delta) : delta;
				eLast[j]+=r;
			}
		}
		vl.result->append(new EulerAnglesValue(eLast[0], eLast[1], eLast[2]));
	}
	return_value(vl.result);
}

Value*
appendIfUnique_cf(Value** arg_list, int count)
{
	check_arg_count(appendIfUnique, 2, count);
	Value* theArray = arg_list[0];
	Value* theVal = arg_list[1];
	one_value_local(res);
	try
	{
		vl.res = theArray->findItem_vf(&theVal,1);
	}
	catch (NoMethodError)
	{
		throw RuntimeError (_T("Value class does not implement findItem method: "), theArray->tag->name);
	}

	if (is_int(vl.res) && ((Integer*)vl.res)->value == 0)
	{
		try
		{
			theArray->append_vf(&theVal,1);
		}
		catch (NoMethodError)
		{
			throw RuntimeError (_T("Value class does not implement append method: "), theArray->tag->name);
		}
		return &true_value;
	}
	return &false_value;
}

Value*
pasteBitmap_cf(Value** arg_list, int count)
{
	// pasteBitmap <src_bitmap> <dest_bitmap> (<box2> | <point2>) <point2> maskColor:<color> type:{#paste | #composite | #blend | #function} alphaMultiplier:<float> function:<fn>
	// positions are 0-based
	check_arg_count_with_keys(pasteBitmap, 4, count);

	MAXBitMap* src_bm = (MAXBitMap*)arg_list[0];
	type_check(src_bm, MAXBitMap, _T("pasteBitmap"));
	MAXBitMap* targ_bm = (MAXBitMap*)arg_list[1];
	type_check(targ_bm, MAXBitMap, _T("pasteBitmap"));

	Point2 destPos = arg_list[3]->to_point2();
	int destX = destPos.x;
	int destY = destPos.y;

	int sourceWidth = src_bm->bm->Width();
	int sourceHeight = src_bm->bm->Height();
	int targWidth = targ_bm->bm->Width();
	int targHeight = targ_bm->bm->Height();

	int fromX, fromY, numX, numY;
	if (arg_list[2]->is_kind_of(class_tag(Point2Value)))
	{
		Point2 from = arg_list[2]->to_point2();
		fromX = from.x;
		fromY = from.y;
		numX = sourceWidth-fromX;
		numY = sourceHeight-fromY;
	}
	else  
	{
		Box2 rect = arg_list[2]->to_box2();
		fromX = rect.x();
		fromY = rect.y();
		numX = rect.w();
		numY = rect.h();
	}

	if (fromX < 0)
	{
		numX += fromX;
		fromX = 0;
	}

	if (fromY < 0)
	{
		numY += fromY;
		fromY = 0;
	}

	if (destX < 0)
	{
		numX += destX;
		destX = 0;
	}

	if (destY < 0)
	{
		numY += destY;
		destY = 0;
	}

	if (numX <= 0 || numX <= 0 || 
		fromX >= sourceWidth || fromY >= sourceHeight || 
		destX >= targWidth || destY >= targHeight)
		return new Box2Value(0, 0, 0, 0);

	/*	range_check(fromX,0,src_bm->bi.Width()-1,_T("source X startpoint"));
	range_check(fromY,0,src_bm->bi.Height()-1,_T("source Y startpoint"));
	range_check(destX,0,targ_bm->bi.Width()-1,_T("target X startpoint"));
	range_check(destY,0,targ_bm->bi.Height()-1,_T("target Y startpoint"));
	*/

	int numXtarg = targWidth-destX;
	int numYtarg = targHeight-destY;
	numX = __min(numX,numXtarg);
	numY = __min(numY,numYtarg);
	if (numX <= 0 || numY <= 0) 
		return new Box2Value(0, 0, 0, 0);

	int numXsource = sourceWidth-fromX;
	int numYsource = sourceHeight-fromY;
	numX = __min(numX,numXsource);
	numY = __min(numY,numYsource);
	if (numX <= 0 || numY <= 0) 
		return new Box2Value(0, 0, 0, 0);

	five_typed_value_locals(Value* result, ColorValue* pixel1_color, Point2Value* pixel1_pos, ColorValue* pixel2_color, Point2Value* pixel2_pos);
	vl.result = new Box2Value(destX, destY, numX, numY);

	Value *maskColorValue = key_arg(maskColor)->eval();
	Value *type = key_arg_or_default(type,n_paste);
	Value *tmp;
	float alphaMultiplier = float_key_arg(alphaMultiplier, tmp, 1.0f);

	Value* fn = &undefined;
	if (type == n_function)
	{
		fn = key_arg_or_default(function, &undefined);
		if (!is_function(fn))
			throw TypeError (GetString(IDS_PASTEBITMAP_FN_ARG_ERROR), fn);
	}

	//-- Allocate work buffer

	PixelBuf work(numX); // foreground
	PixelBuf work2(type == n_paste ? 1 : numX); // background

	vl.pixel1_color = new ColorValue(0.,0.,0.);
	vl.pixel1_pos = new Point2Value(0.,0.);
	vl.pixel2_color = new ColorValue(0.,0.,0.);
	vl.pixel2_pos = new Point2Value(0.,0.);

	//-- Copy Image

	if (maskColorValue == &unsupplied)
	{
		for (int ycount=0; ycount < numY; ycount++,fromY++,destY++) 
		{
			src_bm->bm->GetLinearPixels(fromX,fromY,numX,work.Ptr());
			if (type == n_blend || type == n_composite || type == n_function)
			{
				targ_bm->bm->GetLinearPixels(destX,destY,numX,work2.Ptr());
				for (int xcount=0; xcount < numX; xcount++)
				{
					BMM_Color_64* p = work.Ptr() + xcount;
					BMM_Color_64* p2 = work2.Ptr() + xcount;
					float a = (((float)p->a) / 65536.0f) * alphaMultiplier;
					float a_inv = (1.0f-a);
					if (type == n_blend)
					{
						p->r = p->r * a + p2->r * a_inv;
						p->g = p->g * a + p2->g * a_inv;
						p->b = p->b * a + p2->b * a_inv;
						p->a = p->a     + p2->a * a_inv;
					}
					else if (type == n_composite)
					{
						int r = (int)p->r + (int)p2->r * a_inv;
						p->r = min(65536,r);
						int g= p->g + p2->g * a_inv;
						p->g = min(65536,g);
						int b = p->b + p2->b * a_inv;
						p->b = min(65536,b);
						int a = p->a + p2->a * a_inv;
						p->a = min(65536,a);
					}
					else if (type == n_function)
					{
						vl.pixel1_color->color = *p;
						vl.pixel1_pos->p.x = fromX+xcount; 
						vl.pixel1_pos->p.y = fromY;
						vl.pixel2_color->color = *p2;
						vl.pixel2_pos->p.x = destX+xcount; 
						vl.pixel2_pos->p.y = destY;
						Value* args[] = {vl.pixel1_color, vl.pixel1_pos, vl.pixel2_color, vl.pixel2_pos};
						AColor res = fn->apply(args, 4)->to_acolor();
						*p = res;
					}
				}
			}
			targ_bm->bm->PutPixels(destX,destY,numX,work.Ptr());  
		}
	}
	else
	{
		AColor ac = maskColorValue->to_acolor();
		BMM_Color_64 maskColor = ((WORD)(ac.r * 65280.0f) & 0xff00, (WORD)(ac.g * 65280.0f) & 0xff00, (WORD)(ac.b * 65280.0f) & 0xff00, (WORD)(ac.a * 65280.0f) & 0xff00);

		for (int ycount=0; ycount < numY; ycount++,fromY++,destY++) 
		{
			src_bm->bm->GetLinearPixels(fromX,fromY,numX,work.Ptr());
			if (type == n_blend || type == n_composite)
				targ_bm->bm->GetLinearPixels(destX,destY,numX,work2.Ptr());
			for (int xcount=0; xcount < numX; xcount++)
			{
				BMM_Color_64* p = work.Ptr() + xcount;
				if ((p->r & 0xff00) != maskColor.r || (p->b & 0xff00) != maskColor.b || (p->g & 0xff00) != maskColor.g)
				{
					if (type == n_blend || type == n_composite || type == n_function)
					{
						BMM_Color_64* p2 = work2.Ptr() + xcount;
						float a = (((float)p->a) / 65536.0f) * alphaMultiplier;
						float a_inv = (1.0f-a);
						if (type == n_blend)
						{
							p->r = p->r * a + p2->r * a_inv;
							p->g = p->g * a + p2->g * a_inv;
							p->b = p->b * a + p2->b * a_inv;
							p->a = p->a     + p2->a * a_inv;
						}
						else if (type == n_composite)
						{
							int r = (int)p->r + (int)p2->r * a_inv;
							p->r = min(65536,r);
							int g= p->g + p2->g * a_inv;
							p->g = min(65536,g);
							int b = p->b + p2->b * a_inv;
							p->b = min(65536,b);
							int a = p->a + p2->a * a_inv;
							p->a = min(65536,a);
						}
						else if (type == n_function)
						{
							vl.pixel1_color->color = *p;
							vl.pixel1_pos->p.x = fromX+xcount; 
							vl.pixel1_pos->p.y = fromY;
							vl.pixel2_color->color = *p2;
							vl.pixel2_pos->p.x = destX+xcount; 
							vl.pixel2_pos->p.y = destY;
							Value* args[] = {vl.pixel1_color, vl.pixel1_pos, vl.pixel2_color, vl.pixel2_pos};
							AColor res = fn->apply(args, 4)->to_acolor();
							*p = res;
						}
					}
					targ_bm->bm->PutPixels(destX+xcount,destY,1,p);
				}
			}
		}
	}

	return_value(vl.result);
}


class SSRestore : public RestoreObj {
public:
	SplineShape *theShape;
	BezierShape oldShape, newShape;
	int oldSteps, newSteps;
	NamedVertSelSetList oldVselSet, newVselSet;
	NamedSegSelSetList oldSselSet, newSselSet;
	NamedPolySelSetList oldPselSet, newPselSet;
	Tab<Control*> oldCont, newCont;	// TH 5/8/99

	BOOL setNew;
	//watje 2-22-99
	Tab<Point3> undoPointList;
	Tab<Point3> redoPointList;

	SSRestore(SplineShape *ss) {
		theShape = ss;			
		oldShape = ss->shape;
		oldSteps = ss->steps;
		oldVselSet = ss->vselSet;
		oldSselSet = ss->sselSet;
		oldPselSet = ss->pselSet;
		oldCont = ss->cont;	// TH 5/8/99
		setNew = FALSE;
		undoPointList = ss->pointList;			
	}
	~SSRestore() {}
	void Restore(int isUndo) {
		if(isUndo && !setNew) {
			newShape = theShape->shape;
			newSteps = theShape->steps;
			newVselSet = theShape->vselSet;
			newSselSet = theShape->sselSet;
			newPselSet = theShape->pselSet;
			newCont = theShape->cont;	// TH 5/8/99
			setNew = TRUE;
			redoPointList = theShape->pointList;
		}
		DWORD selLevel = theShape->shape.selLevel;	// Grab this...
		DWORD dispFlags = theShape->shape.dispFlags;	// Grab this...
		theShape->shape = oldShape;
		theShape->shape.selLevel = selLevel;	// ...and put it back in
		theShape->shape.dispFlags = dispFlags;	// ...and put it back in
		theShape->steps = oldSteps;
		theShape->vselSet = oldVselSet;
		theShape->sselSet = oldSselSet;
		theShape->pselSet = oldPselSet;
		// TH 5/8/99 -- Stuff in the old controllers...
		// RK:5/11/99 -- Replaced it with much shorter ReplaceContArray()
		theShape->ReplaceContArray(oldCont);			
		theShape->pointList = undoPointList;
		theShape->InvalidateGeomCache();

		if(theShape->cont.Count())
			theShape->InvalidateChannels(GEOM_CHANNEL);
		theShape->InvalidateSurfaceUI();
		theShape->SelectionChanged();
		theShape->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
		theShape->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	void Redo() {
		DWORD selLevel = theShape->shape.selLevel;	// Grab this...
		DWORD dispFlags = theShape->shape.dispFlags;	// Grab this...
		theShape->shape = newShape;
		theShape->shape.selLevel = selLevel;	// ...and put it back in
		theShape->shape.dispFlags = dispFlags;	// ...and put it back in
		theShape->steps = newSteps;
		theShape->vselSet = newVselSet;
		theShape->sselSet = newSselSet;
		theShape->pselSet = newPselSet;
		// TH 5/8/99 -- Stuff in the new controllers...
		// RK:5/11/99 -- Replaced it with much shorter ReplaceContArray()
		theShape->ReplaceContArray(newCont);
		theShape->pointList = redoPointList;

		theShape->InvalidateGeomCache();
		if(theShape->cont.Count())
			theShape->InvalidateChannels(GEOM_CHANNEL);
		theShape->InvalidateSurfaceUI();
		theShape->SelectionChanged();
		theShape->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
		theShape->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	int Size() { return 1; }
	void EndHold() { theShape->ClearAFlag(A_HELD); }
	TSTR Description() { return TSTR(_T("SplineShape restore")); }
};

static BOOL NeedsWeld(Spline3D *spline, BitArray sel, float thresh) {
	int knots = spline->KnotCount();
	for(int i = 0; i < knots; ++i) {
		if(sel[i*3+1]) {
			int next = (i + 1) % knots;
			if(sel[next*3+1] && Length(spline->GetKnotPoint(i) - spline->GetKnotPoint(next)) <= thresh)
				return TRUE;
		}
	}
	return FALSE;
}

BOOL WeldSplineAtSelVerts(SplineShape *ss, BezierShape *shape, int poly, float thresh) {
	int i;
	Spline3D *spline = shape->splines[poly];
	BitArray& vsel = shape->vertSel[poly];
	BitArray& ssel = shape->segSel[poly];
	int knots = spline->KnotCount();
	BitArray toPrev(knots);
	BitArray toNext(knots);
	toNext.ClearAll();

	// Create weld attachment flag arrays
	for(i = 0; i < knots; ++i) {
		if(vsel[i*3+1]) {
			int next = (i + 1) % knots;
			if(vsel[next*3+1] && Length(spline->GetKnotPoint(i) - spline->GetKnotPoint(next)) <= thresh)
				toNext.Set(i);
		}
	}

	// Now process 'em!
	for(i = knots - 1; i >= 0; --i) {
		if(toNext[i]) {
			int next = (i + 1) % spline->KnotCount();
			if(i == (spline->KnotCount() - 1))
				spline->SetClosed();
			Point3 midpoint = (spline->GetKnotPoint(i) + spline->GetKnotPoint(next)) / 2.0f;
			Point3 nextDelta = midpoint - spline->GetKnotPoint(next);
			Point3 thisDelta = midpoint - spline->GetKnotPoint(i);
			spline->SetKnotPoint(next, spline->GetKnotPoint(next) + nextDelta);
			spline->SetOutVec(next, spline->GetOutVec(next) + nextDelta);
			spline->SetInVec(next, spline->GetInVec(i) + thisDelta);
			if(spline->IsBezierPt(i) || spline->IsBezierPt(next))
				spline->SetKnotType(next, KTYPE_BEZIER_CORNER);
			else
				if(spline->IsCorner(i) || spline->IsCorner(next))
					spline->SetKnotType(next, KTYPE_CORNER);
			spline->DeleteKnot(i);
		}
	}

	// If the polygon is degenerate, blow it away!
	if(spline->KnotCount() < 2) {
		spline->NewSpline();
		shape->DeleteSpline(poly);
		return TRUE;
	}

	// Update the auto points
	spline->ComputeBezPoints();
	ss->InvalidateGeomCache();

	// Clear out the selection sets for verts and segments
	vsel.SetSize(spline->Verts(),0);
	vsel.ClearAll();
	ssel.SetSize(spline->Segments(),0);
	ssel.ClearAll();

	return FALSE;
}

static void DoPolyEndAttach(SplineShape *ss, BezierShape *shape, int poly1, int vert1, int poly2, int vert2) {
	Spline3D *spline = shape->splines[poly1];
	int knots = spline->KnotCount();
	int verts = spline->Verts();
	int segs = spline->Segments();

	// If connecting endpoints of the same polygon, close it and get rid of vertex 1
	if(poly1 == poly2) {
		spline->SetClosed();
		BitArray& vsel = shape->vertSel[poly1];
		BitArray& ssel = shape->segSel[poly1];
		BOOL bothAuto = (spline->GetKnotType(vert1) == KTYPE_AUTO && spline->GetKnotType(vert2) == KTYPE_AUTO) ? TRUE : FALSE;
		int lastVert = knots - 1;
		Point3 p1 = spline->GetKnotPoint(0);
		Point3 p2 = spline->GetKnotPoint(lastVert);
		Point3 midpoint = (p1 + p2) / 2.0f;
		Point3 p1Delta = midpoint - p1;
		Point3 p2Delta = midpoint - p2;
		// Repoint the knot
		spline->SetKnotPoint(0, midpoint);
		if(!bothAuto) {
			spline->SetKnotType(0, KTYPE_BEZIER_CORNER);
			spline->SetInVec(0, spline->GetInVec(lastVert) + p2Delta);
			spline->SetOutVec(0, spline->GetOutVec(0) + p1Delta);
		}
		ss->DeleteKnot(poly1, lastVert);

		// Make the selection sets the right size
		vsel.SetSize(spline->Verts(), 1);
		ssel.SetSize(spline->Segments(), 1);
		spline->ComputeBezPoints();
		ss->InvalidateGeomCache();
	}
	else { 		// Connecting two different splines -- Splice 'em together!
		Spline3D *spline2 = shape->splines[poly2];
		BOOL bothAuto = (spline->GetKnotType(vert1) == KTYPE_AUTO && spline2->GetKnotType(vert2) == KTYPE_AUTO) ? TRUE : FALSE;
		int knots2 = spline2->KnotCount();
		int verts2 = spline2->Verts();
		int segs2 = spline2->Segments();
		// Enlarge the selection sets for the first spline
		BitArray& vsel = shape->vertSel[poly1];
		BitArray& ssel = shape->segSel[poly1];
		BitArray& vsel2 = shape->vertSel[poly2];
		BitArray& ssel2 = shape->segSel[poly2];

		// Reorder the splines if necessary -- We set them up so that poly 1 is first,
		// ordered so that its connection point is the last knot.  Then we order poly
		// 2 so that its connection point is the first knot.  We then copy the invec
		// of poly 1's last knot to the invec of poly 2's first knot, delete poly 1's
		// last knot and append poly 2 to poly 1.  We then delete poly 2.

		if(vert1 == 0) {
			spline->Reverse();
			vsel.Reverse();
			ssel.Reverse();
		}
		if(vert2 != 0) {
			spline2->Reverse();
			vsel2.Reverse();
			ssel2.Reverse();
		}

		int lastVert = knots - 1;
		Point3 p1 = spline->GetKnotPoint(lastVert);
		Point3 p2 = spline2->GetKnotPoint(0);
		Point3 midpoint = (p1 + p2) / 2.0f;
		Point3 p1Delta = midpoint - p1;
		Point3 p2Delta = midpoint - p2;
		spline2->SetKnotPoint(0, midpoint);
		if(!bothAuto) {
			spline2->SetKnotType(0, KTYPE_BEZIER_CORNER);
			spline2->SetInVec(0, spline->GetInVec(lastVert) + p1Delta);
			spline2->SetOutVec(0, spline2->GetOutVec(0) + p2Delta);
		}
		ss->DeleteKnot(poly1, lastVert);
		BOOL welded = ss->Append(poly1, spline2);

		// Fix up the selection sets
		int base = verts - 3;
		vsel.SetSize(spline->Verts(), 1);
		vsel.Set(base+1);
		for(int i = welded ? 6 : 3; i < verts2; ++i)
			vsel.Set(base+i, vsel2[i]);
		base = segs;
		ssel.SetSize(spline->Segments(), 1);
		for(int i = welded ? 1 : 0; i < segs2; ++i)
			ssel.Set(base+i, ssel2[i]);

		// Compute bezier handles and get rid of the attachee
		spline->ComputeBezPoints();
		ss->DeleteSpline(poly2);
	}

}

/*-------------------------------------------------------------------*/		

Value*
weldSpline_cf(Value** arg_list, int count)
{
	check_arg_count(weldSpline, 2, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], weldSpline);
	float weldThreshold = arg_list[1]->to_float();
	Object* obj = node->GetObjectRef();
	if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		obj = ((IDerivedObject*)obj)->FindBaseObject();
	if (obj->ClassID() != splineShapeClassID && obj->ClassID() != Class_ID(SPLINE3D_CLASS_ID,0))
		throw RuntimeError (GetString(IDS_SHAPE_OPERATION_ON_NONSPLINESHAPE), obj->GetObjectName());
	SplineShape* s = (SplineShape*)obj;
	TimeValue t = MAXScript_time();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	int poly1 = 0;
	BOOL altered = FALSE;
	BOOL shapePut = FALSE;

	if (theHold.Holding())
		s->RecordTopologyTags();

	// Poly-to-poly weld loops back here because a weld between polys creates one
	// polygon which may need its two new endpoints welded
changed:
	int polys = s->shape.splineCount;
	for(int poly = polys - 1; poly >= 0; --poly) {
		Spline3D *spline = s->shape.splines[poly];
		// If any bits are set in the selection set, let's DO IT!!
		if(s->shape.vertSel[poly].NumberSet()) {
			hadSelected = TRUE;
			if(NeedsWeld(spline, s->shape.vertSel[poly], weldThreshold)) {
				altered = holdNeeded = TRUE;
				// TH 6/15/99 -- Radical fix, remove any animation from the object
				if(theHold.Holding() && !shapePut) {
					theHold.Put(new SSRestore(s));
					shapePut = TRUE;
				}

				// Call the weld function
				WeldSplineAtSelVerts(s, &s->shape, poly, weldThreshold);
			}
		}
	}

	// Now check for welds with other polys (endpoints only)
	polys = s->shape.SplineCount();
	for(poly1 = 0; poly1 < polys; ++poly1) {
		Spline3D *spline1 = s->shape.splines[poly1];
		if(!spline1->Closed()) {
			int knots1 = spline1->KnotCount();
			int lastKnot1 = knots1-1;
			int lastSel1 = lastKnot1 * 3 + 1;
			BitArray pSel = s->shape.VertexTempSel(poly1);
			if(pSel[1] || pSel[lastSel1]) {
				Point3 p1 = spline1->GetKnotPoint(0);
				Point3 p2 = spline1->GetKnotPoint(lastKnot1);
				for(int poly2 = 0; poly2 < polys; ++poly2) {
					Spline3D *spline2 = s->shape.splines[poly2];
					if(poly1 != poly2 && !spline2->Closed()) {
						int knots2 = spline2->KnotCount();
						int lastKnot2 = knots2-1;
						int lastSel2 = lastKnot2 * 3 + 1;
						Point3 p3 = spline2->GetKnotPoint(0);
						Point3 p4 = spline2->GetKnotPoint(lastKnot2);
						BitArray pSel2 = s->shape.VertexTempSel(poly2);
						int vert1, vert2;
						if(pSel[1]) {
							if(pSel2[1] && Length(p3 - p1) <= weldThreshold) {
								vert1 = 0;
								vert2 = 0;

attach_it:
								// TH 6/15/99 -- Radical fix, remove any animation from the object
								if(theHold.Holding() && !shapePut) {
									theHold.Put(new SSRestore(s));
									shapePut = TRUE;
								}
								// Do the attach
								DoPolyEndAttach(s, &s->shape, poly1, vert1, poly2, vert2);
								altered = holdNeeded = TRUE;
								goto changed;
							}
							else
								if(pSel2[lastSel2] && Length(p4 - p1) <= weldThreshold) {
									vert1 = 0;
									vert2 = knots2-1;
									goto attach_it;
								}
						}
						if(pSel[lastSel1]) {
							if(pSel2[1] && Length(p3 - p2) <= weldThreshold) {
								vert1 = knots1-1;
								vert2 = 0;
								goto attach_it;
							}
							else
								if(pSel2[lastSel2] && Length(p4 - p2) <= weldThreshold) {
									vert1 = knots1-1;
									vert2 = knots2-1;
									goto attach_it;
								}
						}
					}
				}
			}
		}
	}

	if(holdNeeded) {
		s->ResolveTopoChanges();
	}

	s->SelectionChanged();
	s->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	needs_redraw_set();
	return &ok;
}

Value*
FindPathSegAndParam_cf(Value** arg_list, int count)
{
	check_arg_count(FindPathSegAndParam, 3, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], FindPathSegAndParam);
	Object* obj = node->GetObjectRef();
	if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		obj = ((IDerivedObject*)obj)->FindBaseObject();
	if (obj->ClassID() != splineShapeClassID && obj->ClassID() != Class_ID(SPLINE3D_CLASS_ID,0))
		throw RuntimeError (GetString(IDS_SHAPE_OPERATION_ON_NONSPLINESHAPE), obj->GetObjectName());
	SplineShape* s = (SplineShape*)obj;
	int index = arg_list[1]->to_int();
	if (index < 1 || index > s->shape.splineCount)
		throw RuntimeError (GetString(IDS_SHAPE_SPLINE_INDEX_OUT_OF_RANGE), Integer::intern(index));
	Spline3D *spline = s->shape.splines[index-1];
	float u = arg_list[2]->to_float();
	int seg;
	float param;
	spline->FindSegAndParam(u, SPLINE_INTERP_SIMPLE, seg, param);
	seg += 1;
	if (seg > spline->Segments())
	{
		seg = spline->Segments();
		param = 1.f;
	}
	return new Point2Value(seg,param);
}


Value*
FindLengthSegAndParam_cf(Value** arg_list, int count)
{
	check_arg_count(FindLengthSegAndParam, 3, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], FindLengthSegAndParam);
	Object* obj = node->GetObjectRef();
	if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		obj = ((IDerivedObject*)obj)->FindBaseObject();
	if (obj->ClassID() != splineShapeClassID && obj->ClassID() != Class_ID(SPLINE3D_CLASS_ID,0))
		throw RuntimeError (GetString(IDS_SHAPE_OPERATION_ON_NONSPLINESHAPE), obj->GetObjectName());
	SplineShape* s = (SplineShape*)obj;
	int index = arg_list[1]->to_int();
	if (index < 1 || index > s->shape.splineCount)
		throw RuntimeError (GetString(IDS_SHAPE_SPLINE_INDEX_OUT_OF_RANGE), Integer::intern(index));
	Spline3D *spline = s->shape.splines[index-1];
	float u = arg_list[2]->to_float();
	int seg;
	float param;
	spline->FindSegAndParam(u, SPLINE_INTERP_NORMALIZED, seg, param);
	seg += 1;
	if (seg > spline->Segments())
	{
		seg = spline->Segments();
		param = 1.f;
	}
	return new Point2Value(seg,param);
}

Value*
interpBezier3D_cf(Value** arg_list, int count)
{
	// interpBezier3D <splineShape> <spline_index> <segment_index> <param_float> [pathParam:<boolean>]
	check_arg_count_with_keys(interpBezier3D, 4, count);
	MAXNode* shape = (MAXNode*)arg_list[0];
	INode* node = get_valid_node(shape, interpBezier3D);
	Object* obj = node->GetObjectRef();
	if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		obj = ((IDerivedObject*)obj)->FindBaseObject();
	if (obj->ClassID() != splineShapeClassID && obj->ClassID() != Class_ID(SPLINE3D_CLASS_ID,0))
		throw RuntimeError (GetString(IDS_SHAPE_OPERATION_ON_NONSPLINESHAPE), obj->GetObjectName());
	SplineShape* s = (SplineShape*)obj;
	int index = arg_list[1]->to_int();
	if (index < 1 || index > s->shape.splineCount)
		throw RuntimeError (GetString(IDS_SHAPE_SPLINE_INDEX_OUT_OF_RANGE), Integer::intern(index));
	Spline3D *spline = s->shape.splines[index-1];
	int seg_index = arg_list[2]->to_int();
	if (seg_index < 1 || seg_index > spline->Segments())
		throw RuntimeError (GetString(IDS_SPLINE_SEGMENT_INDEX_OUT_OF_RANGE), Integer::intern(seg_index));
	Value* tmp;
	int ptype = (bool_key_arg(pathParam, tmp, TRUE)) ? SPLINE_INTERP_SIMPLE : SPLINE_INTERP_NORMALIZED; 
	Point3 p = spline->InterpBezier3D(seg_index-1, arg_list[3]->to_float(), ptype);
	shape->object_to_current_coordsys(p);
	return new Point3Value(p);
}

Value*
tangentBezier3D_cf(Value** arg_list, int count)
{
	// tangentBezier3D <splineShape> <spline_index> <segment_index> <param_float> [pathParam:<boolean>]
	check_arg_count_with_keys(tangentBezier3D, 4, count);
	MAXNode* shape = (MAXNode*)arg_list[0];
	INode* node = get_valid_node(shape, tangentBezier3D);
	Object* obj = node->GetObjectRef();
	if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		obj = ((IDerivedObject*)obj)->FindBaseObject();
	if (obj->ClassID() != splineShapeClassID && obj->ClassID() != Class_ID(SPLINE3D_CLASS_ID,0))
		throw RuntimeError (GetString(IDS_SHAPE_OPERATION_ON_NONSPLINESHAPE), obj->GetObjectName());
	SplineShape* s = (SplineShape*)obj;
	int index = arg_list[1]->to_int();
	if (index < 1 || index > s->shape.splineCount)
		throw RuntimeError (GetString(IDS_SHAPE_SPLINE_INDEX_OUT_OF_RANGE), Integer::intern(index));
	Spline3D *spline = s->shape.splines[index-1];
	int seg_index = arg_list[2]->to_int();
	if (seg_index < 1 || seg_index > spline->Segments())
		throw RuntimeError (GetString(IDS_SPLINE_SEGMENT_INDEX_OUT_OF_RANGE), Integer::intern(seg_index));
	Value* tmp;
	int ptype = (bool_key_arg(pathParam, tmp, TRUE)) ? SPLINE_INTERP_SIMPLE : SPLINE_INTERP_NORMALIZED; 
	Point3 p = spline->TangentBezier3D(seg_index-1, arg_list[3]->to_float(), ptype);
	shape->object_to_current_coordsys_rotate(p);
	return new Point3Value(p);
}

Value*
getLocalTime_cf(Value** arg_list, int count)
{
	// getLocalTime()
	check_arg_count(getLocalTime, 0, count);
	SYSTEMTIME time;
	GetLocalTime(&time);
	one_typed_value_local(Array* result);
	vl.result = new Array (8);
	vl.result->append(Integer::intern(time.wYear));
	vl.result->append(Integer::intern(time.wMonth));
	vl.result->append(Integer::intern(time.wDayOfWeek));
	vl.result->append(Integer::intern(time.wDay));
	vl.result->append(Integer::intern(time.wHour));
	vl.result->append(Integer::intern(time.wMinute));
	vl.result->append(Integer::intern(time.wSecond));
	vl.result->append(Integer::intern(time.wMilliseconds));
	return_value(vl.result);
}

Value*
getUniversalTime_cf(Value** arg_list, int count)
{
	// getUniversalTime()
	check_arg_count(getUniversalTime, 0, count);
	SYSTEMTIME time;
	GetSystemTime(&time);
	one_typed_value_local(Array* result);
	vl.result = new Array (8);
	vl.result->append(Integer::intern(time.wYear));
	vl.result->append(Integer::intern(time.wMonth));
	vl.result->append(Integer::intern(time.wDayOfWeek));
	vl.result->append(Integer::intern(time.wDay));
	vl.result->append(Integer::intern(time.wHour));
	vl.result->append(Integer::intern(time.wMinute));
	vl.result->append(Integer::intern(time.wSecond));
	vl.result->append(Integer::intern(time.wMilliseconds));
	return_value(vl.result);
}


Value*
isDeformable_cf(Value** arg_list, int count)
{
	check_arg_count(isDeformable, 1, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], isDeformable);
	const ObjectState& os = node->EvalWorldState(MAXScript_time());
	Object* ob = os.obj;
	if (ob && ob->IsDeformable()) return &true_value;
	return &false_value;
}

Value*
numPoints_cf(Value** arg_list, int count)
{
	check_arg_count(numPoints, 1, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], numPoints);
	const ObjectState& os = node->EvalWorldState(MAXScript_time());
	Object* ob = os.obj;
	if (ob && ob->IsDeformable()) return Integer::intern(ob->NumPoints());
	return Integer::intern(0);
}

Value*
getPointPos_cf(Value** arg_list, int count)
{
	check_arg_count(getPointPos, 2, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], getPointPos);
	const ObjectState& os = node->EvalWorldState(MAXScript_time());
	Object* ob = os.obj;
	if (ob && ob->IsDeformable()) 
	{
		int index = arg_list[1]->to_int();
		range_check(index,1,ob->NumPoints(),_T("point number out of range"));
		Point3 p = ob->GetPoint(index-1);
		((MAXNode*)arg_list[0])->object_to_current_coordsys(p);
		return new Point3Value (p);
	}
	throw RuntimeError (_T("Node's object not deformable: "),arg_list[0]);
	return Integer::intern(0);
}

Value*
nodeGetBoundingBox_cf(Value** arg_list, int count)
{
	check_arg_count(nodeGetBoundingBox, 2, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], nodeGetBoundingBox);
	const ObjectState& os = node->EvalWorldState(MAXScript_time());
	Object* ob = os.obj;
	Matrix3 otm = node->GetObjectTM(MAXScript_time());
	Matrix3 xfrm = arg_list[1]->to_matrix3();
	xfrm = otm * Inverse(xfrm);
	if (ob) 
	{
		Box3 bbox;
		ob->GetDeformBBox(MAXScript_time(), bbox, &xfrm, FALSE);
		Point3 pmin = bbox.Min();
//		((MAXNode*)arg_list[0])->object_to_current_coordsys(pmin);
		Point3 pmax = bbox.Max();
//		((MAXNode*)arg_list[0])->object_to_current_coordsys(pmax);
		one_typed_value_local(Array* result);
		vl.result = new Array (2);
		vl.result->append(new Point3Value (pmin));
		vl.result->append(new Point3Value (pmax));
		return_value(vl.result);
	}
	return &undefined;
}

Value*
nodeLocalBoundingBox_cf(Value** arg_list, int count)
{
	check_arg_count(nodeLocalBoundingBox, 1, count);
	INode* node = get_valid_node((MAXNode*)arg_list[0], nodeLocalBoundingBox);
	const ObjectState& os = node->EvalWorldState(MAXScript_time());
	Object* ob = os.obj;
	if (ob) 
	{
		Box3 bbox;
		ob->GetDeformBBox(MAXScript_time(), bbox, NULL, FALSE);
		Point3 pmin = bbox.Min();
		((MAXNode*)arg_list[0])->object_to_current_coordsys(pmin);
		Point3 pmax = bbox.Max();
		((MAXNode*)arg_list[0])->object_to_current_coordsys(pmax);
		one_typed_value_local(Array* result);
		vl.result = new Array (2);
		vl.result->append(new Point3Value (pmin));
		vl.result->append(new Point3Value (pmax));
		return_value(vl.result);
	}
	return &undefined;
}

Value*
intersectRayScene(INode* node, Value* vRay)
{
	float		at;
	Point3		pt, normal;
	Ray			ray, os_ray;
	GeomObject*	obj;

	// Get the object from the node
	ObjectState os = node->EvalWorldState(MAXScript_time());
	if (os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
		obj = (GeomObject*)os.obj;
	else
		return &undefined;

	// Back transform the ray into object space.		
	Matrix3 obtm  = node->GetObjectTM(MAXScript_time());
	Matrix3 iobtm = Inverse(obtm);
	ray = vRay->to_ray();
	os_ray.p   = iobtm * ray.p;
	os_ray.dir = VectorTransform(iobtm, ray.dir);

	// See if we hit the object
	if (obj->IntersectRay(MAXScript_time(), os_ray, at, normal))
	{
		// Calculate the hit point
		pt = os_ray.p + os_ray.dir * at;

		// transform back into world space & build result ray
		pt = pt * obtm;
		normal = Normalize(VectorTransform(obtm, normal));
		return new RayValue (pt, normal);
	}
	else
		return &undefined;
}

Value* 
intersect_nodes(Value** arg_list, int count)
{
	// node tab collector for selection node mapping
	// arg0 is node candidate, arg1 is result Array, arg2 is the ray value
	Value* res = intersectRayScene(((MAXNode*)arg_list[0])->node, arg_list[2]);
	if (res != &undefined) 
	{
		one_typed_value_local(Array* result);
		vl.result = new Array (2);
		vl.result->append(arg_list[0]);
		vl.result->append(res);
		((Array*)arg_list[1])->append(vl.result);
	}
	return &ok;
}

Value*
intersectRayScene_cf(Value** arg_list, int count)
{
	check_arg_count(intersectRayScene, 1, count);
	arg_list[0]->to_ray();
	one_typed_value_local(Array* result);
	vl.result = new Array (0);
	Value* args[3] = { NULL, vl.result, arg_list[0] };
	node_map m = { NULL, intersect_nodes, args, 2 };
	Value* all_objects = globals->get(Name::intern(_T("objects")))->eval();
	all_objects->map(m);
	return_value(vl.result);
}


Value*
getBezierTangentTypeIn()
{
	def_bezier_tangent_types();
	int in, out;
	GetBezierDefaultTangentType(in, out);
	return GetName(bezierTangentTypes, elements(bezierTangentTypes), in);	
}

Value*
setBezierTangentTypeIn(Value* val)
{
	def_bezier_tangent_types();
	int in, out;
	GetBezierDefaultTangentType(in, out);
	in = GetID(bezierTangentTypes, elements(bezierTangentTypes), val);
	SetBezierDefaultTangentType(in, out);
	return val;
}

Value*
getBezierTangentTypeOut()
{
	def_bezier_tangent_types();
	int in, out;
	GetBezierDefaultTangentType(in, out);
	return GetName(bezierTangentTypes, elements(bezierTangentTypes), out);	
}

Value*
setBezierTangentTypeOut(Value* val)
{
	def_bezier_tangent_types();
	int in, out;
	GetBezierDefaultTangentType(in, out);
	out = GetID(bezierTangentTypes, elements(bezierTangentTypes), val);
	SetBezierDefaultTangentType(in, out);
	return val;
}

Value*
getTCBDefaultParam_t()
{
	float t, c, b, easeIn, easeOut;
	GetTCBDefaultParams(t, c, b, easeIn, easeOut);
	return Float::intern(ToTCBUI(t));
}

Value*
setTCBDefaultParam_t(Value* val)
{
	float t, c, b, easeIn, easeOut;
	GetTCBDefaultParams(t, c, b, easeIn, easeOut);
	SetTCBDefaultParams(FromTCBUI(val->to_float()), c, b, easeIn, easeOut);
	return val;
}

Value*
getTCBDefaultParam_c()
{
	float t, c, b, easeIn, easeOut;
	GetTCBDefaultParams(t, c, b, easeIn, easeOut);
	return Float::intern(ToTCBUI(c));
}

Value*
setTCBDefaultParam_c(Value* val)
{
	float t, c, b, easeIn, easeOut;
	GetTCBDefaultParams(t, c, b, easeIn, easeOut);
	SetTCBDefaultParams(t, FromTCBUI(val->to_float()), b, easeIn, easeOut);
	return val;
}

Value*
getTCBDefaultParam_b()
{
	float t, c, b, easeIn, easeOut;
	GetTCBDefaultParams(t, c, b, easeIn, easeOut);
	return Float::intern(ToTCBUI(b));
}

Value*
setTCBDefaultParam_b(Value* val)
{
	float t, c, b, easeIn, easeOut;
	GetTCBDefaultParams(t, c, b, easeIn, easeOut);
	SetTCBDefaultParams(t, c, FromTCBUI(val->to_float()), easeIn, easeOut);
	return val;
}

Value*
getTCBDefaultParam_easeIn()
{
	float t, c, b, easeIn, easeOut;
	GetTCBDefaultParams(t, c, b, easeIn, easeOut);
	return Float::intern(ToEaseUI(easeIn));
}

Value*
setTCBDefaultParam_easeIn(Value* val)
{
	float t, c, b, easeIn, easeOut;
	GetTCBDefaultParams(t, c, b, easeIn, easeOut);
	SetTCBDefaultParams(t, c, b, FromEaseUI(val->to_float()), easeOut);
	return val;
}

Value*
getTCBDefaultParam_easeOut()
{
	float t, c, b, easeIn, easeOut;
	GetTCBDefaultParams(t, c, b, easeIn, easeOut);
	return Float::intern(ToEaseUI(easeOut));
}

Value*
setTCBDefaultParam_easeOut(Value* val)
{
	float t, c, b, easeIn, easeOut;
	GetTCBDefaultParams(t, c, b, easeIn, easeOut);
	SetTCBDefaultParams(t, c, b, easeIn, FromEaseUI(val->to_float()));
	return val;
}

Value*
MatEditor_GetMode()
{
	def_mtlDlgMode_types();
	int mode = MAXScript_interface13->GetMtlDlgMode();
	return GetName(mtlDlgModeTypes, elements(mtlDlgModeTypes), mode);
}

Value*
MatEditor_SetMode(Value* val)
{
	def_mtlDlgMode_types();
	int mode = GetID(mtlDlgModeTypes, elements(mtlDlgModeTypes), val);
	MAXScript_interface13->SetMtlDlgMode( mode );
	return val;
}


// -----------------------------------------------------------------------------------------
Value*
make_max_reftarg(MAXClass* cls, ReferenceTarget* existing, Value** arg_list, int count)
{
	ReferenceTarget* ref;
	one_value_local(new_ref);
	check_arg_count_with_keys(reftarg create, 0, count);

	// wrap given reftarget or or create the MAX object & initialize it
	SuspendAnimate();
	AnimateOff();
	try
	{
		if (existing == NULL)
		{
			if (cls->md_flags & md_no_create)
				throw RuntimeError (_T("Non-creatable class: "), cls->name->to_string());

			ref = (ReferenceTarget*)MAXScript_interface->CreateInstance(cls->sclass_id, cls->class_id);
			if (ref == NULL)
				throw RuntimeError (_T("Non-creatable class: "), cls->name->to_string());

			cls->initialize_object(ref);   

			if (count > 0)	// process keyword args 
			{
				cls->apply_keyword_parms(ref, arg_list, count);
				cls->superclass->apply_keyword_parms(ref, arg_list, count);
			}
		}
		else
			ref = (ReferenceTarget*)existing;

		// make the wrapper 
		vl.new_ref = MAXRefTarg::intern(ref);
	}
	catch (...)
	{
		ResumeAnimate();
		throw;
	}
	ResumeAnimate();
	return_value (vl.new_ref);
}


MAXSuperClass layer_class
(_T("Base_Layer"), LAYER_CLASS_ID, &maxwrapper_class, make_max_reftarg,
 end
 );

#define SCENE_REF_LAYERREF		10

Value*
LayerManager_getLayerObject_cf(Value** arg_list, int count)
{
	// <ReferenceTarget> ILayerManager.getLayerObject {<int layer_index> | <string layer_name>}
	check_arg_count(getLayerObject, 1, count);
	ReferenceTarget* target = MAXScript_interface->GetScenePointer();
	ILayerManager* theman = (ILayerManager*)target->GetReference(SCENE_REF_LAYERREF);
	if (theman == NULL) 
		return &undefined;
	if (is_integer(arg_list[0]))
	{
		int index = arg_list[0]->to_int();
		range_check(index, 0, theman->GetLayerCount()-1,_T("Layer index out of range: "));
		IFPLayerManager* themanFPi = (IFPLayerManager*)GetCOREInterface(LAYERMANAGER_INTERFACE);
		if (themanFPi == NULL) 
			return &undefined;
		ILayerProperties* layerProps = themanFPi->getLayer(index);
		if (layerProps == NULL) 
			return &undefined;
		TSTR layerName = layerProps->getName();
		ILayer* res = theman->GetLayer(layerName);
		return (res) ? (Value*) MAXRefTarg::intern(res) : &undefined;
	}
	TSTR layerName = arg_list[0]->to_string();
	ILayer* res = theman->GetLayer(layerName);
	return (res) ? (Value*) MAXRefTarg::intern(res) : &undefined;
}

Value*
setNeedsRedraw_cf(Value** arg_list, int count)
{
	// setNeedsRedraw()
	check_arg_count_with_keys(setNeedsRedraw, 0, count);
	Value* tmp;
	BOOL complete = bool_key_arg(complete,tmp,FALSE);
	if (complete)
		needs_complete_redraw_set();
	else
		needs_redraw_set();
	return &ok;
}

Value*
makeUniqueArray_cf(Value** arg_list, int count)
{
	check_arg_count(makeUniqueArray, 1, count);
	Array* theArray = (Array*)arg_list[0];
	type_check(theArray, Array, "makeUniqueArray");
	one_typed_value_local(Array* result);
	vl.result = new Array (theArray->size); // assume few duplicates
	for (int i = 0; i < theArray->size; i++)
	{
		Value *ele = theArray->data[i];
		bool notFound = true;
		for (int j = 0; j < vl.result->size; j++)
		{
			if (ele->tag == vl.result->data[j]->tag && // prevent coercion
				ele->eq_vf(&vl.result->data[j], 1) == &true_value)
			{
				notFound = false;
				break;
			}
		}
		if (notFound)
			vl.result->append(ele);
	}
	return_value(vl.result);
}

Value*
toUpper_cf(Value** arg_list, int count)
{
	// toUpper <string>
	check_arg_count(toUpper, 1, count);
	TSTR s = arg_list[0]->to_string();
	s.toUpper();
	return new String (s);
}

Value*
toLower_cf(Value** arg_list, int count)
{
	// toLower <string>
	check_arg_count(toLower, 1, count);
	TSTR s = arg_list[0]->to_string();
	s.toLower();
	return new String (s);
}


Value*
substituteString_cf(Value** arg_list, int count)
{
	check_arg_count(substituteString, 3, count);
	TSTR workString = arg_list[0]->to_string();
	TSTR fromString = arg_list[1]->to_string();
	TSTR toString = arg_list[2]->to_string();
	int workLen = workString.Length();
	int fromLen = fromString.Length();
	int toLen = toString.Length();

	if (fromLen == 0)
		return new String (workString);

	const TCHAR firstFromChar = fromString[0];

	for (int i = 0; i < workLen; i++)
	{
		if ((workString[i] == firstFromChar) && 
			((fromLen == 1) || (workString.Substr(i,fromLen) == fromString)))
		{
			if (fromLen == toLen)
			{
				for (int j = 0; j < toLen; j++)
					workString[i+j] = toString[j];
			}
			else
			{
				TSTR tmpString;
				if (i != 0)
					tmpString.Append(workString.Substr(0,i));
				tmpString.Append(toString);
				int k = i+fromLen;
				if (k < workLen)
					tmpString.Append(workString.Substr(k,workLen-k));

				workString = tmpString;
				workLen = workString.Length();
			}
			i += toLen-1;
		}
	}
	return new String (workString);
}

Value*
replace_LF_with_CRLF_cf(Value** arg_list, int count)
{
	// replace_LF_with_CRLF <string>
	check_arg_count(replace_LF_with_CRLF, 1, count);
	TSTR workString = arg_list[0]->to_string();
	Replace_LF_with_CRLF(workString);
	return new String(workString);
}

Value*
replace_CRLF_with_LF_cf(Value** arg_list, int count)
{
	// replace_CRLF_with_LF <string>
	check_arg_count(replace_CRLF_with_LF, 1, count);
	TSTR workString = arg_list[0]->to_string();
	Replace_CRLF_with_LF(workString);
	return new String(workString);
}

Value*
clearControllerNewFlag_cf(Value** arg_list, int count)
{
	check_arg_count(clearControllerNewFlag, 1, count);
	MAXControl* cv = (MAXControl*)arg_list[0];
	Control* c = cv->to_controller(); // type check
	cv->flags &= ~MAX_CTRL_NEW;
	return &ok;
}


Value*
scan_for_new_plugins_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(scanForNewPlugins, 0, count);
	// scan class directory and all public subclasses for new
	// scripter-accessible classes to register
	Value* tmp;
	BOOL includeUnknownSuperclasses = bool_key_arg(includeUnknownSuperclasses, tmp, FALSE); 
	ClassDirectory& cdir = MAXScript_interface->GetDllDir().ClassDir();
	for (int i = 0; i < cdir.Count(); i++)
	{
		SubClassList& scl = cdir[i];
		for (int j = 0; j < scl.Count(ACC_ALL); j++)
		{
			// pick up the ClassDesc for each class & build class table entry
			ClassEntry& ce = scl[j];
			ClassDesc* cd = ce.CD();
			SClass_ID sid = cd->SuperClassID();
			Class_ID cid = cd->ClassID();
			if (includeUnknownSuperclasses || lookup_MAXSuperClass(sid))
				MAXClass* mc =  MAXClass::lookup_class(&cid, sid, true);
		} 
	}
	BOOL includeCoreInterfaces = bool_key_arg(includeCoreInterfaces, tmp, TRUE); 
	if (includeCoreInterfaces)
	{
		three_typed_value_locals(Value* name, FPInterfaceValue* i, Value* globalVal);
		for (int i = 0; i < NumCOREInterfaces(); i++)
		{
			// add each interface
			FPInterface* fpi = GetCOREInterfaceAt(i);
			FPInterfaceDesc* fpid = fpi->GetDesc();
			if ((fpid->flags & FP_TEST_INTERFACE) || (fpid->flags & FP_MIXIN))
				continue;   // but not mixin descriptors, only statics
			vl.name = Name::intern(fpid->internal_name);
			vl.globalVal = globals->get(vl.name);
			vl.i = new FPInterfaceValue (fpi);
			vl.name = Name::intern(fpid->internal_name);
			globals->put_new(vl.name, new ConstGlobalThunk (vl.name, vl.i));
		}
		pop_value_locals();
	}

	return &ok;
}

Value*
colorPickerDlg_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(colorPickerDlg, 2, count);
	one_value_local(result);
	vl.result = &undefined;
	Value *tmp;
	BOOL alpha = bool_key_arg(alpha,tmp,FALSE);
	IPoint2 pos;
	IPoint2 *ppos = NULL;
	Thunk* posThunk = NULL;
	Value* posValue = key_arg(pos);
	if (posValue != &unsupplied && posValue->_is_thunk()) 
	{
		posThunk = posValue->to_thunk();
		posValue = posThunk->eval();
	}
	if (posValue != &unsupplied) 
	{
		Point2 posf = posValue->to_point2();
		pos.x = posf.x;
		pos.y = posf.y;
		ppos = &pos;
	}
	TCHAR *name = arg_list[1]->to_string();
	int res;
	if (alpha)
	{
		AColor color = arg_list[0]->to_acolor();
		res = HSVDlg_Do(MAXScript_interface->GetMAXHWnd(), &color, ppos, NULL, name);
		if (res) vl.result = new ColorValue(color);
	}
	else
	{

		DWORD color = arg_list[0]->to_colorref();
		res = HSVDlg_Do(MAXScript_interface->GetMAXHWnd(), &color, ppos, NULL, name);
		if (res) vl.result = new ColorValue(color);
	}
	if (posThunk)
		posThunk->assign(new Point2Value(pos.x,pos.y));
	return_value(vl.result);
}

Value*  
SetGridSpacing_cf(Value** arg_list, int count)
{
	check_arg_count(SetGridSpacing, 1, count); 
	float val = arg_list[0]->to_float();
	if (val < 0.001f) val = 0.001f;
	MAXScript_interface7->SetGridSpacing(val);
	return &ok;
}

Value*  
SetGridMajorLines_cf(Value** arg_list, int count)
{
	check_arg_count(SetGridMajorLines, 1, count); 
	float val = arg_list[0]->to_float();
	if (val < 2.0f) val = 2.0f;
	MAXScript_interface7->SetGridMajorLines(val);
	return &ok;
}

Value*  
getNodeTM_cf(Value** arg_list, int count)
{
	check_arg_count(getNodeTM, 1, count); 
	INode *node;
	if is_rootnode(arg_list[0]) 
		node = arg_list[0]->to_rootnode();
	else
		node = arg_list[0]->to_node();
	Matrix3 tm = node->GetNodeTM(MAXScript_time());
	return new Matrix3Value(tm);
}

// Assumes a 32-bit machine (or that j < 2^32).
inline int find_most_significant_1_index (int j) 
{
	if (j == 0) 
		return 0;
	int new_j = 0, r = 0;
	if ((new_j = j & 0xFFFF0000) != 0) { r |= 16; j = new_j; }
	if ((new_j = j & 0xFF00FF00) != 0) { r |=  8; j = new_j; }
	if ((new_j = j & 0xF0F0F0F0) != 0) { r |=  4; j = new_j; }
	if ((new_j = j & 0xCCCCCCCC) != 0) { r |=  2; j = new_j; }
	if (         j & 0xAAAAAAAA)    r |=  1;
	return r;
}

Value*  
getBitmapInfo_cf(Value** arg_list, int count)
{
	check_arg_count(getBitmapInfo, 1, count); 
	bool deleteBitmap = false;
	MAXBitMap *mbm = NULL;
	Bitmap *bm = NULL;
	BitmapInfo bi;

	if (is_bitmap(arg_list[0]))
	{
		mbm = (MAXBitMap*)arg_list[0];
		bi = mbm->bi;
		bm = mbm->bm;
	}
	else
	{
		Value* file_name = arg_list[0];
		TSTR  fileName = arg_list[0]->to_filename();
		AssetId assetId;
		if (IAssetManager::GetInstance()->StringToAssetId(fileName, assetId))
			fileName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();

		TCHAR  filepath[MAX_PATH];
		TCHAR* fpart;

		// search in the package, IMAGE, current scene and then the MAPS directories
		BOOL found = 
			MSZipPackage::search_current_package(fileName, filepath) ||
			SearchPath(MAXScript_interface->GetDir(APP_IMAGE_DIR), fileName, NULL, MAX_PATH, filepath, &fpart);
		if (!found)
		{
			MaxSDK::Util::Path path(fileName);
			IFileResolutionManager* pFRM = IFileResolutionManager::GetInstance();
			DbgAssert(pFRM);
			if (pFRM && pFRM->GetFullFilePath(path, MaxSDK::AssetManagement::kBitmapAsset))
			{
				found = TRUE;
				_tcscpy(filepath, path.GetCStr());
			}
		}
		if (found)
		{
			bi.SetName(filepath);
			BMMRES status;
			TempBitmapManagerSilentMode tbmsm;
			bm = TheManager->Load(&bi, &status);
		}
		if (bm == NULL)
			throw RuntimeError (_T("unable to find bitmap file: "), fileName);
		deleteBitmap = true;
	}

	int maxRGBLevel = bm->MaxRGBLevel();
	int numRGBBits = (maxRGBLevel == 0) ? 0 : find_most_significant_1_index(maxRGBLevel)+1;
	int maxAlphaLevel = bm->MaxAlphaLevel();
	int numAlphaBits = (maxAlphaLevel == 0) ? 0 : find_most_significant_1_index(maxAlphaLevel)+1;

	one_typed_value_local(Array* result);
	vl.result = new Array (13);
	vl.result->append(new String ((TCHAR*)bi.Name()));
	vl.result->append(new String ((TCHAR*)bi.Device()));
	vl.result->append(Integer::intern(bi.Width()));
	vl.result->append(Integer::intern(bi.Height()));
	vl.result->append(Integer::intern(numRGBBits));
	vl.result->append(Integer::intern(numAlphaBits));
	vl.result->append(Float::intern(bi.Aspect()));
	vl.result->append(Float::intern(bi.Gamma()));
	vl.result->append(bool_result(bm->HasAlpha()));
	vl.result->append(Integer::intern(bi.FirstFrame()));
	vl.result->append(Integer::intern(bi.LastFrame()));
	vl.result->append(Integer::intern(bi.Type()));
	vl.result->append(Integer::intern(bi.Flags()));

	if (deleteBitmap) bm->DeleteThis();

	return_value(vl.result);
}

Value*  
isAnimPlaying_cf(Value** arg_list, int count)
{
	check_arg_count(isAnimPlaying, 0, count); 
	return bool_result(GetCOREInterface()->IsAnimPlaying());
}

Value*  
NodeExposureInterface_create_cf(Value** arg_list, int count)
{
	check_arg_count(NodeExposureInterface.create, 1, count); 
	INode* node = arg_list[0]->to_node();
	FPInterface* nfpi = reinterpret_cast<FPInterface*>(node->GetInterface(NODEEXPOSURE_INTERFACE_TOAPPEND));
	if (nfpi)
		return FPMixinInterfaceValue::intern(nfpi);
	else
		return &undefined;
}
Value*  
NodeExposureInterface_get_cf(Value** arg_list, int count)
{
	check_arg_count(NodeExposureInterface.get, 1, count); 
	INode* node = arg_list[0]->to_node();
	FPInterface* nfpi = reinterpret_cast<FPInterface*>(node->GetInterface(NODEEXPOSURE_INTERFACE));
	if (nfpi)
		return FPMixinInterfaceValue::intern(nfpi);
	else
		return &undefined;
}

Value*  
particleLife_cf(Value** arg_list, int count)
{
	check_arg_count(particleLife, 2, count); 
	INode *node = arg_list[0]->to_node();
	ObjectState os = node->EvalWorldState(MAXScript_time());
	ParticleObject *po = GetParticleInterface(os.obj);
	int index = arg_list[1]->to_int()-1;
	if (po)
		return MSTime::intern(po->ParticleLife(MAXScript_time(),index));
	else
		return &undefined;
}
Value*  
particleCenter_cf(Value** arg_list, int count)
{
	check_arg_count(particleCenter, 2, count); 
	INode *node = arg_list[0]->to_node();
	ObjectState os = node->EvalWorldState(MAXScript_time());
	ParticleObject *po = GetParticleInterface(os.obj);
	int index = arg_list[1]->to_int()-1;
	if (po)
		return Integer::intern(po->ParticleCenter(MAXScript_time(),index));
	else
		return &undefined;
}
Value*  
particleSize2_cf(Value** arg_list, int count)
{
	check_arg_count(particleSize2, 2, count); 
	INode *node = arg_list[0]->to_node();
	ObjectState os = node->EvalWorldState(MAXScript_time());
	ParticleObject *po = GetParticleInterface(os.obj);
	int index = arg_list[1]->to_int()-1;
	if (po)
		return Float::intern(po->ParticleSize(MAXScript_time(),index));
	else
		return &undefined;
}

Value*  
pathIsNetworkPath_cf(Value** arg_list, int count)
{
	check_arg_count(pathIsNetworkPath, 1, count); 

	TSTR  fileName = arg_list[0]->to_filename();
	AssetId assetId;
	if (IAssetManager::GetInstance()->StringToAssetId(fileName, assetId))
		fileName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();

	return bool_result(PathIsNetworkPath(fileName));
}

Value*
get_TimeDisplayMode()
{
	def_TimeDisplayMode_types();
	return GetName(timeDisplayModeTypes, elements(timeDisplayModeTypes), GetTimeDisplayMode());	
}

Value*
set_TimeDisplayMode(Value* val)
{
	def_TimeDisplayMode_types();
	TimeDisp mode = (TimeDisp)GetID(timeDisplayModeTypes, elements(timeDisplayModeTypes), val);
	SetTimeDisplayMode(mode);
	return val;
}

Value* areNodesInstances_cf(Value** arg_list, int count)
{
	// areNodesInstances <node> <node>
	check_arg_count(areNodesInstances, 2, count);
	INode* node1 = arg_list[0]->to_node();
	Object* obj1 = node1->GetObjectRef();
	INode* node2 = arg_list[1]->to_node();
	Object* obj2 = node2->GetObjectRef();
	return bool_result(obj1 == obj2);
}

// HiddenDOSCommand source from Mike BiddleCombe
Value* HiddenDOSCommand_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(HiddenDOSCommand, 1, count);
	TCHAR* params = arg_list[0]->to_string();

	Value* key;

	// Look for a starting path to launch the command from
	MSTR startpath;
	key = key_arg(startpath);
	if (key != &unsupplied) 
	{
		startpath = key->to_filename();
		AssetId assetId;
		if (IAssetManager::GetInstance()->StringToAssetId(startpath, assetId))
			startpath = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();
	}

	// Look for a wait-for-completion override 
	const bool DEFAULT_WAIT_FOR_COMPLETION = false;
	key = key_arg(donotwait);
	BOOL donotwait = (key == &unsupplied) ? DEFAULT_WAIT_FOR_COMPLETION : key->to_bool();

	// Look for a prompt string
	key = key_arg(prompt);
	TCHAR* prompt = (key == &unsupplied) ? 0 : key->to_string();

	// Look for a exit code return variable
	Thunk* exitCodeThunk = NULL;
	Value* exitCodeValue = key_arg(ExitCode);
	if (exitCodeValue != &unsupplied) 
		exitCodeThunk = exitCodeValue->to_thunk();

	// Look for a exit code return variable
	Thunk* errorMsgThunk = NULL;
	Value* errorMsgValue = key_arg(errorMsg);
	if (errorMsgValue != &unsupplied) 
		errorMsgThunk = errorMsgValue->to_thunk();

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory( &pi, sizeof(pi) );
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	GetCOREInterface()->PushPrompt( prompt ? prompt : params ) ;

	// You may want to adjust the arguments passed to cmd here,
	// type 'cmd /?' from the console for additional documentation
	TSTR args("cmd /e:on /d /c ");

	// Make sure we have a quoted command string
	if ( (params[0] != '\"') || (params[_tcslen(params)-1] != '\"') )
	{
		args.append("\"");
		args.append(params);
		args.append("\"");
	}
	else
	{
		args.append(params);
	}

	TCHAR* pStartPath = NULL;
	if (!startpath.isNull())
		pStartPath = startpath.data();

	BOOL ret = CreateProcess(
		NULL,				// lpApplicationName
		args.data(),		// lpCommandLine
		NULL,				// lpProcessAttributes
		NULL,				// lpThreadAttributes
		FALSE,				// bInheritHandles
		CREATE_NO_WINDOW,	// dwCreationFlags
		NULL,				// lpEnvironment
		pStartPath,			// lpCurrentDirectory
		&si,				// lpStartupInfo
		&pi					// lpProcessInformation
		);					

	DWORD ExitCode = 0;

	if (ret && !donotwait)
	{
		ret = (WAIT_OBJECT_0 == WaitForSingleObject( pi.hProcess, INFINITE ));
		GetExitCodeProcess(pi.hProcess,&ExitCode);
	}

	GetCOREInterface()->PopPrompt();

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	if (exitCodeThunk)
		exitCodeThunk->assign(Integer::intern(ExitCode));

	if (ret) 
	{
		if (errorMsgThunk)
			errorMsgThunk->assign(&undefined);
		return &true_value;
	}
	else 
	{
		if (errorMsgThunk)
		{
			DWORD lastError = GetLastError();
			LPVOID lpMsgBuf;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
				lastError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf, 0, NULL);
			TCHAR* errorMsg = (LPTSTR)lpMsgBuf;
			size_t len = _tcslen(errorMsg);
			if (len >=2 )
			{
				if (_tcscmp(&errorMsg[len-2],_T("\r\n")) == 0)
					errorMsg[len-2] = _T('\0');
			}
			errorMsgThunk->assign(new String((LPTSTR)lpMsgBuf));
			LocalFree(lpMsgBuf);
		}
		return &false_value;
	}
}

Value*
GetDraftRenderer_cf(Value** arg_list, int count)
{
	check_arg_count(GetDraftRenderer, 0, count);
	return MAXRenderer::intern(MAXScript_interface->GetDraftRenderer());
}

Value*
ClearDraftRenderer_cf(Value** arg_list, int count)
{
	check_arg_count(ClearDraftRenderer, 0, count);
	HoldSuspend suspend;
	MAXScript_interface->AssignDraftRenderer(NULL);
	return &ok;
}

Value* 
getHandleByAnim_cf(Value** arg_list, int count)
{
	check_arg_count(getHandleByAnim, 1, count);
	Value* v = arg_list[0];
	ReferenceTarget* refTarg;

	if( v->is_kind_of(class_tag(MAXNode)) )
		refTarg = ((MAXNode*)arg_list[0])->node;
	else refTarg = v->to_reftarg();

	if( refTarg==NULL ) return Integer::intern( 0 );

	return IntegerPtr::intern( Animatable::GetHandleByAnim(refTarg) ); // UINT_PTR
}

Value*
getAnimByHandle_cf(Value** arg_list, int count)
{
	check_arg_count(getAnimByHandle, 1, count);
	UINT_PTR handle = (arg_list[0]->to_intptr());// UINT_PTR
	Animatable* anim = Animatable::GetAnimByHandle(handle);
	if (anim && anim->IsRefMaker() && ((ReferenceMaker*)anim)->IsRefTarget())
		return MAXClass::make_wrapper_for((ReferenceTarget*)anim);
	return &undefined; 
}

Value*
setNumberThreads_cf(Value** arg_list, int count)
{
	check_arg_count(setNumberThreads, 3, count);
	int id = arg_list[0]->to_int();
	int maxNumThreads = arg_list[1]->to_int();
	int minNumElements = arg_list[2]->to_int();

	MaxSDK::PerformanceTools::ThreadTools::SetNumberOfThreads((MaxSDK::PerformanceTools::ThreadTools::ThreadType)id,maxNumThreads,minNumElements);
	BOOL res = TRUE;
	return bool_result(res);
}

Value*
getD3DTimer_cf(Value** arg_list, int count)
{
	check_arg_count( getD3DTimer, 1, count);
	unsigned int timerID = arg_list[0]->to_int();

	MaxSDK::PerformanceTools::Timer timer;
	timer = MaxSDK::PerformanceTools::Timer();

	float s =  (float) timer.GetTimerGlobal(timerID);
	s = s/1000; //return time in ms
	return Float::intern(s);
}

#define ASSETMETADATA_DEFSTRUCT_NAME _T("AssetMetadata_StructDef")

// helper classes and function to read an asset's metadata 

class AssetMetaData
{
public:
	AssetId assetId;
	WStr filename;
	WStr assetType;
	AssetMetaData (AssetId &assetId, const wchar_t* filename, const wchar_t* assetType) : assetId(assetId), filename(filename), assetType(assetType) {}
	AssetMetaData () : assetType(_T("OtherAsset")) {}
};

class IUnknownDestructorPolicy
{
public:
	static void Delete(IUnknown* pIUnknown)
	{
		if (pIUnknown)
			pIUnknown->Release(); 
	}
};

bool ReadAssetMetaData(LPSTREAM pIStream, AssetMetaData &assetMetaData, unsigned __int64 &number_bytes_read)
{
	// For each asset, the metadata saved is the assetId, the assetType as a null-terminated wchar_t string, 
	// and the asset filename as a null-terminated wchar_t string
	// Need to store assetType as string to properly support dynamic asset types. Those will be
	// registered via plugins by name, and the plugin will get a AssetType value back.
	ULONG nb;
	HRESULT hres = pIStream->Read(&assetMetaData.assetId,sizeof(assetMetaData.assetId),&nb);
	DbgAssert(hres == S_OK); DbgAssert(nb == sizeof(assetMetaData.assetId));
	if (hres != S_OK || nb != sizeof(assetMetaData.assetId))
		return false;
	number_bytes_read += nb;
	int len;
	wchar_t buf[MAX_PATH+1];
	hres = pIStream->Read(&len,sizeof(len),&nb);
	DbgAssert(hres == S_OK); DbgAssert(nb == sizeof(len));
	number_bytes_read += nb;
	int string_byte_len = (len+1)*sizeof(wchar_t);
	hres = pIStream->Read(&buf,string_byte_len,&nb);
	DbgAssert(hres == S_OK); DbgAssert(nb == string_byte_len);
	number_bytes_read += nb;
	assetMetaData.assetType = buf;
	hres = pIStream->Read(&len,sizeof(len),&nb);
	DbgAssert(hres == S_OK); DbgAssert(nb == sizeof(len));
	if (hres != S_OK || nb != sizeof(len))
		return false;
	number_bytes_read += nb;
	string_byte_len = (len+1)*sizeof(wchar_t);
	hres = pIStream->Read(&buf,string_byte_len,&nb);
	DbgAssert(hres == S_OK); DbgAssert(nb == string_byte_len);
	if (hres != S_OK || nb != string_byte_len)
		return false;
	number_bytes_read += nb;
	assetMetaData.filename = buf;
	return true;
}

Value*
get_max_file_asset_metadata_cf(Value** arg_list, int count)
{
	// getMAXFileAssetMetadata <filename>
	check_arg_count_with_keys(getMAXFileAssetMetadata, 1, count);

	TSTR  fileName = arg_list[0]->to_filename();
	AssetId assetId;
	if (IAssetManager::GetInstance()->StringToAssetId(fileName, assetId))
		fileName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();

	const TCHAR* scene_dir = MAXScript_interface->GetDir(APP_SCENE_DIR);
	TCHAR  buf[MAX_PATH] = {0};
	seven_typed_value_locals(Name* struct_name, StructDef* sd, Array* result, Value* struct_instance, Value* assetId, Value* filename, Value* assetType);

	// Return value will be an array of AssetMetadata_StructDef instances. Get the StructDef value
	vl.result = (Array*)&undefined;
	vl.struct_name = (Name*)Name::intern(ASSETMETADATA_DEFSTRUCT_NAME);
	vl.sd = (StructDef*)globals->get(vl.struct_name)->eval();

	// find the file
	if (MSZipPackage::search_current_package(fileName, buf) ||
		SearchPath(scene_dir, fileName, NULL, MAX_PATH, buf, NULL) ||
		SearchPath(NULL, fileName, NULL, MAX_PATH, buf, NULL))
	{
		// This opens the file as an OLE structured storage file with read access
		IStorage*	pIStorage = NULL;
		HRESULT hres = GetCOREInterface11()->OpenMAXStorageFile(WStr(buf), &pIStorage);
		if (hres != S_OK || pIStorage == NULL)
			return_value(vl.result);
		// define AutoPtr for guaranteed cleanup of pIStorage
		MaxSDK::AutoPtr<IUnknown, IUnknownDestructorPolicy> pIStorageAutoPtr(pIStorage);

		// Get the Asset Metadata stream
		LPSTREAM pIStream = NULL;
		hres = pIStorage->OpenStream(L"FileAssetMetaData2", NULL, STGM_READ|STGM_SHARE_EXCLUSIVE, 0, &pIStream);
		if (hres != S_OK  || pIStream == NULL)
			return_value(vl.result);  // valid for old files
		// define AutoPtr for guaranteed cleanup of pIStream
		MaxSDK::AutoPtr<IUnknown, IUnknownDestructorPolicy> pIStreamAutoPtr(pIStream);

		// get the length of the stream
		STATSTG statstg;
		pIStream->Stat(&statstg,STATFLAG_NONAME);
		unsigned __int64 nBytes = statstg.cbSize.QuadPart;
		unsigned __int64 nBytesRead = 0;

		// read from the stream while there are bytes left.
		AssetMetaData assetMetaData;
		vl.result = new Array (0);
		while (nBytesRead < nBytes)
		{
			// load an asset's metadata
			if (!ReadAssetMetaData(pIStream, assetMetaData, nBytesRead))
				throw RuntimeError(GetString(IDS_ERROR_READING_ASSETMETADATA_STREAM));

			// create AssetMetadata_StructDef struct instance
			vl.assetId = new String(IAssetManager::GetInstance()->AssetIdToString(assetMetaData.assetId));
			vl.filename = new String(TSTR(assetMetaData.filename)); // don't want to specify assetId in the ctor as this isn't a current asset
			vl.assetType = Name::intern(TSTR(assetMetaData.assetType));
			// create the AssetMetadata_StructDef instance
			Value* args[3] = {vl.assetId, vl.filename, vl.assetType};
			vl.struct_instance = vl.sd->apply(args,3);
			// add it to the output array
			vl.result->append(vl.struct_instance);
		}
	}
	else
		throw RuntimeError(GetString(IDS_FILE_NOT_FOUND), arg_list[0]);
	return_value(vl.result);
}

Value*
set_max_file_asset_metadata_cf(Value** arg_list, int count)
{
	// setMAXFileAssetMetadata <filename> <array of AssetMetadata_StructDef instances>
	check_arg_count_with_keys(setMAXFileAssetMetadata, 2, count);
	TSTR  fileName = arg_list[0]->to_filename();
	AssetId assetId;
	if (IAssetManager::GetInstance()->StringToAssetId(fileName, assetId))
		fileName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();

	const TCHAR* scene_dir = MAXScript_interface->GetDir(APP_SCENE_DIR);
	TCHAR  buf[MAX_PATH] = {0};
	five_typed_value_locals(Name* struct_name, StructDef* sd, Value* assetId, Value* filename, Value* assetType);

	// for each element of <array of AssetMetadata_StructDef instances>, make sure it's an AssetMetadata_StructDef instance
	// collect elements as AssetMetaData instances.
	vl.struct_name = (Name*)Name::intern(ASSETMETADATA_DEFSTRUCT_NAME);
	vl.sd = (StructDef*)globals->get(vl.struct_name)->eval();
	Array* metadata_array = (Array*)arg_list[1];
	type_check(metadata_array, Array, _T("setMAXFileAssetMetadata"));
	MaxSDK::Array<AssetMetaData> new_assetMetaData(metadata_array->size);
	for (int i = 0; i < metadata_array->size; i++)
	{
		Struct* element = (Struct*)metadata_array->data[i];
		if (!is_struct(element)) 
			throw RuntimeError(GetString(IDS_EXPECTED_ASSETMETADATA_DEFSTRUCT), element);
		StructDef *sdef = element->definition;
		if (sdef != vl.sd)
			throw RuntimeError(GetString(IDS_EXPECTED_ASSETMETADATA_DEFSTRUCT), element);
		vl.assetId = element->member_data[1];
		vl.filename = element->member_data[2];
		vl.assetType = element->member_data[3];
		bool success = IAssetManager::GetInstance()->StringToAssetId(vl.assetId->to_string(), new_assetMetaData[i].assetId);
		if (!success)
			throw RuntimeError(GetString(IDS_CANNOT_CONVERT_ASSETID), vl.assetId);

		TSTR  assetFileName = vl.filename->to_filename();
		AssetId assetId;
		if (IAssetManager::GetInstance()->StringToAssetId(assetFileName, assetId))
			assetFileName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();
		new_assetMetaData[i].filename = assetFileName;

		new_assetMetaData[i].assetType = vl.assetType->to_string();
	}

	// find the file
	if (MSZipPackage::search_current_package(fileName, buf) ||
		SearchPath(scene_dir, fileName, NULL, MAX_PATH, buf, NULL) ||
		SearchPath(NULL, fileName, NULL, MAX_PATH, buf, NULL))
	{
		// This opens the file as an OLE structured storage file with read/write access
		STGOPTIONS  StgOptions = { 0};
		StgOptions.usVersion    = 1;		/* Win2k+ */
		StgOptions.ulSectorSize = 4096;	/* 512 limits us to 2GB */
		DWORD mode = (STGM_DIRECT|STGM_READWRITE|STGM_SHARE_EXCLUSIVE);
		IStorage*	pIStorage = NULL;
		HRESULT hres = StgOpenStorageEx(WStr(buf), mode, STGFMT_DOCFILE, 0, &StgOptions, NULL,
			IID_IStorage, (void **) &pIStorage);
		if (hres != S_OK || pIStorage == NULL)
			return_value(&false_value);
		// define AutoPtr for guaranteed cleanup of pIStorage
		MaxSDK::AutoPtr<IUnknown, IUnknownDestructorPolicy> pIStorageAutoPtr(pIStorage);

		// This opens the Asset Metadata stream with read/write access
		LPSTREAM pIStream;
		hres = pIStorage->OpenStream(L"FileAssetMetaData2", NULL, mode, 0, &pIStream);
		if (hres != S_OK  || pIStream == NULL)
			return_value(&false_value); // valid for old files
		// define AutoPtr for guaranteed cleanup of pIStream
		MaxSDK::AutoPtr<IUnknown, IUnknownDestructorPolicy> pIStreamAutoPtr(pIStream);

		// get the length of the stream
		STATSTG statstg;
		pIStream->Stat(&statstg,STATFLAG_NONAME);
		unsigned __int64 nBytes = statstg.cbSize.QuadPart;
		unsigned __int64 nBytesRead = 0;

		// read from the stream while there are bytes left.
		MaxSDK::Array<AssetMetaData> assetMetaDataArray;
		AssetMetaData assetMetaData;
		while (nBytesRead < nBytes)
		{
			// load an asset's metadata
			if (!ReadAssetMetaData(pIStream, assetMetaData, nBytesRead))
				throw RuntimeError(GetString(IDS_ERROR_READING_ASSETMETADATA_STREAM));
			// see if user has specified data for this asset id
			for (int i = 0; i < new_assetMetaData.length(); i++)
			{
				if (new_assetMetaData[i].assetId == assetMetaData.assetId)
				{
					// assetType has to match, Case-insensitive compare
					if (_wcsicmp(new_assetMetaData[i].assetType, assetMetaData.assetType) != 0)
					{
						TSTR errorMsg;
						errorMsg.printf(GetString(IDS_INCORRECT_ASSETTYPE), vl.assetId->to_string(), 
							assetMetaData.assetType.data(), new_assetMetaData[i].assetType.data());
						throw RuntimeError(errorMsg);
					}
					assetMetaData.filename = new_assetMetaData[i].filename;
					break;
				}
			}
			assetMetaDataArray.append(assetMetaData);
		}
		// done reading asset metadata, reset stream and rewrite it.
		LARGE_INTEGER zero = {0, 0};
		pIStream->Seek(zero, STREAM_SEEK_SET, NULL);
		ULARGE_INTEGER uzero = {0, 0};
		pIStream->SetSize(uzero);
		for (int i = 0; i < assetMetaDataArray.length(); i++)
		{
			ULONG nb;
			// store assetId, assetType width in characters, assetType as unicode string - null terminated, 
			// filename width in characters, filename as unicode string - null terminated
			AssetMetaData &assetMetaData = assetMetaDataArray[i];
			pIStream->Write(&assetMetaData.assetId,sizeof(assetMetaData.assetId),&nb); 
			int len = assetMetaData.assetType.Length();
			pIStream->Write(&len,sizeof(len),&nb);
			if (len != 0)
				pIStream->Write(assetMetaData.assetType.data(),len*sizeof(wchar_t),&nb);
			wchar_t null_char = 0;
			pIStream->Write(&null_char,sizeof(null_char),&nb);
			len = assetMetaData.filename.Length();
			pIStream->Write(&len,sizeof(len),&nb);
			if (len != 0)
				pIStream->Write(assetMetaData.filename.data(),len*sizeof(wchar_t),&nb);
			pIStream->Write(&null_char,sizeof(null_char),&nb);
		}
	}
	else
		throw RuntimeError(GetString(IDS_FILE_NOT_FOUND), arg_list[0]);
	return_value(&true_value);
}

#include "sys/types.h"
#include "sys/stat.h"
#include "io.h"
#include "Aclapi.h"

Value*
get_file_security_info_cf(Value** arg_list, int count)
{
	// getFileSecurityInfo <file_name> <attribute> testFileAttribute:<bool>
	check_arg_count_with_keys(getFileSecurityInfo, 2, count);

	TSTR fn = arg_list[0]->to_filename();
	AssetId assetId;
	if (IAssetManager::GetInstance()->StringToAssetId(fn, assetId))
		fn = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();

	Value* dummyVal;
	BOOL testFileAttribute = bool_key_arg(testFileAttribute, dummyVal, FALSE);

	struct _stat stat_buf;
	if (_stat (fn, &stat_buf ) == -1)
		throw RuntimeError (GetString(IDS_FILE_NOT_FOUND), arg_list[0]);

	HANDLE			hToken;
	PRIVILEGE_SET	PrivilegeSet;
	DWORD			PrivSetSize = sizeof (PRIVILEGE_SET);
	DWORD			DesiredAccess;
	DWORD			GrantedAccess;

	BOOL			bAccessReadGranted = FALSE;
	BOOL			bAccessWriteGranted = FALSE;
	BOOL			bAccessExecuteGranted = FALSE;
	BOOL			bAccessAllGranted = FALSE;
	BOOL			bAccessCheckReturnCode;

	SECURITY_INFORMATION RequestedInformation = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;
	GENERIC_MAPPING GenericMapping = { FILE_GENERIC_READ, FILE_GENERIC_WRITE, FILE_GENERIC_EXECUTE, FILE_ALL_ACCESS };

	PSECURITY_DESCRIPTOR pSDRes = NULL;
	DWORD errorCode = ::GetNamedSecurityInfo(fn, SE_FILE_OBJECT, RequestedInformation, NULL, NULL, NULL, NULL, &pSDRes);
	if (errorCode != ERROR_SUCCESS)
		return &undefined;

	// Perform security impersonation of the user and open the resulting thread token.
	if (!ImpersonateSelf (SecurityImpersonation))
	{
		// Unable to perform security impersonation.
		LocalFree (pSDRes);
		return &undefined;
	}
	if (!OpenThreadToken (GetCurrentThread (), TOKEN_DUPLICATE | TOKEN_QUERY, FALSE, &hToken))
	{
		// Unable to get current thread's token.
		LocalFree (pSDRes);
		return &undefined;
	}
	RevertToSelf ();

	// Perform access checks using the token.
	DesiredAccess = GENERIC_READ;
	MapGenericMask (&DesiredAccess, &GenericMapping);
	bAccessCheckReturnCode = AccessCheck (pSDRes, hToken, DesiredAccess, &GenericMapping, &PrivilegeSet, &PrivSetSize, &GrantedAccess, &bAccessReadGranted);
	DbgAssert(bAccessCheckReturnCode);
	if (!bAccessCheckReturnCode)
		errorCode = GetLastError();

	DesiredAccess = GENERIC_WRITE;
	MapGenericMask (&DesiredAccess, &GenericMapping);
	bAccessCheckReturnCode = AccessCheck (pSDRes, hToken, DesiredAccess, &GenericMapping, &PrivilegeSet, &PrivSetSize, &GrantedAccess, &bAccessWriteGranted);
	DbgAssert(bAccessCheckReturnCode);
	if (!bAccessCheckReturnCode)
		errorCode = GetLastError();

	DesiredAccess = GENERIC_EXECUTE;
	MapGenericMask (&DesiredAccess, &GenericMapping);
	bAccessCheckReturnCode = AccessCheck (pSDRes, hToken, DesiredAccess, &GenericMapping, &PrivilegeSet, &PrivSetSize, &GrantedAccess, &bAccessExecuteGranted);
	DbgAssert(bAccessCheckReturnCode);
	if (!bAccessCheckReturnCode)
		errorCode = GetLastError();

	DesiredAccess = GENERIC_ALL;
	MapGenericMask (&DesiredAccess, &GenericMapping);
	bAccessCheckReturnCode = AccessCheck (pSDRes, hToken, DesiredAccess, &GenericMapping, &PrivilegeSet, &PrivSetSize, &GrantedAccess, &bAccessAllGranted);
	DbgAssert(bAccessCheckReturnCode);
	if (!bAccessCheckReturnCode)
		errorCode = GetLastError();

	// Clean up.
	LocalFree (pSDRes);
	CloseHandle (hToken);

	// For a directory, the security is all we need to check (theoretically - see below....)
	// For a file, however, we also need to check the attributes to see that the read-only attribute is not set for write mode.
	if (!(stat_buf.st_mode & _S_IFDIR) && testFileAttribute)
	{
		if (bAccessReadGranted) 
			bAccessReadGranted = stat_buf.st_mode & _S_IREAD ? TRUE : FALSE;
		if (bAccessWriteGranted) 
			bAccessWriteGranted = stat_buf.st_mode & _S_IWRITE ? TRUE : FALSE;
		if (bAccessExecuteGranted) 
			bAccessExecuteGranted = stat_buf.st_mode & _S_IEXEC ? TRUE : FALSE;
		bAccessAllGranted = bAccessAllGranted && bAccessReadGranted && bAccessWriteGranted && bAccessExecuteGranted;
	}
	else if (stat_buf.st_mode & _S_IFDIR)
	{
		// not always getting correct results when checking to see directory on network drive is writable.
		// only way to tell that we know of so far is to try creating a temp file in the directory
		if ((arg_list[1] == n_write && bAccessWriteGranted) || (arg_list[1] == n_all && bAccessAllGranted))
		{
			MaxSDK::Util::Path path(fn);
			path.AddTrailingBackslash();

			TSTR fn = path.GetCStr();
			fn.append(_T("__tempFileXXXXXX"));
			_tmktemp(fn);

			FILE* file = fopen(fn, _T("wD"));
			if (file)
				fclose(file);
			else
				bAccessWriteGranted = bAccessAllGranted = FALSE;
		}
	}

	if (arg_list[1] == n_read)
		return bool_result(bAccessReadGranted);
	else if (arg_list[1] == n_write)
		return bool_result(bAccessWriteGranted);
	else if (arg_list[1] == n_execute)
		return bool_result(bAccessExecuteGranted);
	else if (arg_list[1] == n_all)
		return bool_result(bAccessAllGranted);
	else
		throw RuntimeError (GetString(IDS_BAD_FILE_SECURITY_ATTRIB), arg_list[1]);
	return &undefined;
}

Value*
is_directory_writeable_cf(Value** arg_list, int count)
{
	// isDirectoryWriteable <dir>
	check_arg_count(isDirectoryWriteable, 1, count);

	TSTR dirName = arg_list[0]->to_filename();
	AssetId assetId;
	if (IAssetManager::GetInstance()->StringToAssetId(dirName, assetId))
		dirName = IAssetManager::GetInstance()->GetAsset(assetId).GetFileName();

	struct _stat stat_buf;
	int statRes = _stat (dirName, &stat_buf );
	if (statRes == -1) // not found
		return &false_value;
	if (!(stat_buf.st_mode & _S_IFDIR)) // not a dir
		return &false_value;

	MaxSDK::Util::Path path(dirName);
	path.AddTrailingBackslash();
	
	TSTR fn = path.GetCStr();
	fn.append(_T("__tempFileXXXXXX"));
	_tmktemp(fn);

	FILE* file = fopen(fn, _T("wD"));
	if (file)
		fclose(file);
	return bool_result(file);
}

Value*														
GetSimilarNodes_cf(Value** arg_list, int count)
{
	// getSimilarNodes <dir>
	check_arg_count_with_keys( getSimilarNodes, 1, count);
	NodeTab nodes;
	GetINodeTabResult ret = GetINodeTabFromValue(arg_list[0], nodes, NULL);
	if (ErrSelectionEmpty == ret)
	{
		throw RuntimeError (GetString(IDS_GETSIMILARNODES_SELECTION_EMPTY));
	}
	else if (ErrRequireNodes == ret)
	{
		throw RuntimeError (GetString(IDS_GETSIMILARNODES_REQUIRES_A_NODE_OR_NODE_COLLECTION_GOT), arg_list[0]);
	}

	def_nodePropsToMatch_types();
	Value* nodePropsToMatchVal = key_arg(nodePropsToMatch);
	int nodePropsToMatch = GetID(nodePropsToMatchTypes, elements(nodePropsToMatchTypes), nodePropsToMatchVal, Interface10::kNodeProp_All);

	// have the NodeTab, find similar nodes
	INodeTab similarNodes;
	MAXScript_interface11->FindNodes(nodes, similarNodes);
	one_typed_value_local(Array* result);
	vl.result = new Array (similarNodes.Count());
	for (int i = 0; i < similarNodes.Count(); i++)
		vl.result->append(MAXNode::intern(similarNodes[i]));	
	return_value(vl.result);
}

Value*
get_scene_root()
{
	return MAXClass::make_wrapper_for(MAXScript_interface->GetScenePointer());
}


/* --------------------- plug-in init --------------------------------- */
// this is called by the dlx initializer, register the global vars here
void avg_init()
{

	#include "lam_glbls.h"
	
//	def_user_prop (pinNode, MAXNode, getPinNode_cf, setPinNode_cf);
//	def_user_generic(testBitArray_UG, BitArrayValue, test);
//#define def_user_propM(_prop, _getter, _setter)					\
//		MAXNode_class.add_user_prop(#_prop, _getter, _setter);	\
//		MAXObject_class.add_user_prop(#_prop, _getter, _setter);	\
//		MeshValue_class.add_user_prop(#_prop, _getter, _setter);
#define def_user_propM(_prop, _getter, _setter)				\
	def_user_prop(_prop, MAXNode,   _getter, _setter);	\
	def_user_prop(_prop, MAXObject, _getter, _setter);	\
	def_user_prop(_prop, MeshValue, _getter, _setter);
	def_user_propM (hiddenVerts,	meshop_getHiddenVerts_cf,	meshop_setHiddenVerts_cf);
	def_user_propM (hiddenFaces,	meshop_getHiddenFaces_cf,	meshop_setHiddenFaces_cf);
	def_user_propM (openEdges,		meshop_getOpenEdges_cf,		NULL);

	def_user_prop (isPB2Based,	MAXClass,	isPB2Based_cf,	NULL);
	def_user_prop (isPB2Based,	MSPluginClass,	isPB2Based_cf,	NULL);
	def_user_prop (isPB2Based,	MSCustAttribDef,	isPB2Based_cf,	NULL);

	def_user_prop (isMSPluginClass,	MAXClass,	isMSPluginClass_cf,	NULL);
	def_user_prop (isMSPluginClass,	MSPluginClass,	isMSPluginClass_cf,	NULL);
	def_user_prop (isMSPluginClass,	MSCustAttribDef,	isMSPluginClass_cf,	NULL);

	def_user_prop (isMSCustAttribClass,	MAXClass,	isMSCustAttribClass_cf,	NULL);
	def_user_prop (isMSCustAttribClass,	MSPluginClass,	isMSCustAttribClass_cf,	NULL);
	def_user_prop (isMSCustAttribClass,	MSCustAttribDef,	isMSCustAttribClass_cf,	NULL);

	install_rollout_control(n_multiListBox,		MultiListBoxControl::create);	

	// create a structure def for handling asset metadata. Members: assetId, name, type
	push_alloc_frame();
	three_typed_value_locals(Name* struct_name, StructDef* sd, HashTable* members);
	vl.members = new HashTable();
	int member_count = 0;
	vl.members->put_new(n_this,		Integer::heap_intern(member_count++));
	vl.members->put_new(n_assetId,	Integer::heap_intern(member_count++));
	vl.members->put_new(n_filename,	Integer::heap_intern(member_count++));
	vl.members->put_new(n_type,		Integer::heap_intern(member_count++));
	vl.struct_name = (Name*)Name::intern(ASSETMETADATA_DEFSTRUCT_NAME);
	vl.sd = new StructDef (vl.struct_name, member_count, NULL, vl.members, NULL, NULL);
	globals->put(vl.struct_name, new ConstGlobalThunk (vl.struct_name, vl.sd));
	pop_value_locals();
	pop_alloc_frame();
}

/*


mouse test script:

rollout test "test1" 
( 
 timer clock "testClock" interval:40
 label lbl1 ""  
 label lbl2 ""
 on clock tick do 
 (lbl1.text = getMouseButtonStates() as string
  lbl2.text = getMouseMode() as string
 ) 
) 
floater=newrolloutfloater "" 100 100
addrollout test floater

MtlBase test script:

--	1 - MTL_BEING_EDITED
--	2 - MTL_MEDIT_BACKGROUND
--	3 - MTL_MEDIT_BACKLIGHT
--	4 - MTL_MEDIT_VIDCHECK

s=sphere()
tm
sm=s.material=standard diffusemap:(tm=checker())
meditmaterials[1]=sm
meditmaterials[2]=tm
-- material
getMTLMEditFlags sm
setMTLMEditFlags sm  #{}
getMTLMEditFlags sm
getMTLMeditObjType sm
setMTLMeditObjType sm  2
getMTLMeditObjType sm
-- texture
getMTLMEditFlags tm
setMTLMEditFlags tm  #{1..4}
getMTLMEditFlags tm
getMTLMeditObjType tm
setMTLMeditObjType tm  3
getMTLMeditObjType tm

*/

/*
Value*
tester_cf(Value** arg_list, int count)
{
ICUIFrame *cui = GetCUIFrameMgr()->GetICUIFrame("Command Panel");
if (cui) cui->Hide(TRUE);
if (cui) cui->Hide(FALSE);
return &ok;
}
*/

/*
// This used to cause an exception, now it doesn't. No idea what is being done differently
// might have had wrong run-time library specified.

Value*
tester_cf(Value** arg_list, int count)
{
	AdjEdgeList ae(*arg_list[0]->to_mesh());
	return &ok;
}


script for running tester:

mo=mesh lengthsegs:1 widthsegs:1
tester (copy mo.mesh)

result: user breakpoint:

NTDLL! 77f76274()
NTDLL! 77f8318c()
KERNEL32! 77f12e5a()
_CrtIsValidHeapPointer(const void * 0x088049e0) line 1606
_free_dbg_lk(void * 0x088049e0, int 1) line 1011 + 9 bytes
_free_dbg(void * 0x088049e0, int 1) line 970 + 13 bytes
free(void * 0x088049e0) line 926 + 11 bytes
operator delete(void * 0x088049e0) line 7 + 9 bytes
DWORDTab::`vector deleting destructor'(unsigned int 3) + 67 bytes
AdjEdgeList::~AdjEdgeList() line 48 + 78 bytes
tester_cf(Value * * 0x0012fa24, int 1) line 235 + 16 bytes
Primitive::apply(Value * * 0x08804c60, int 1) line 270 + 14 bytes
CodeTree::eval() line 117 + 26 bytes
ListenerDlgProc(HWND__ * 0x003e07c6, unsigned int 1288, unsigned int 0, long 89718596) line 705 + 17 bytes
USER32! 77e80cbb()
USER32! 77e83451()
USER32! 77e71268()
0558ff44()
*/


/*
Value*
tester_cf(Value** arg_list, int count)
{
	int counter=GetCUIFrameMgr()->GetCount();
	TCHAR* theName;
	for (int i = 0; i < counter; i++) 
		for (int j = 0; j < 3; j++) 
			for (int k = 0; k < 3; k++) {
				ICUIFrame *cui = GetCUIFrameMgr()->GetICUIFrame(i,j,k);
				if (cui) {
					theName = cui->GetName();
					OutputDebugString(theName);
				}
	}
	return &ok;
}
// output: Main Menu, Tab Panel 
*/

/*
g=geosphere()
at time 10 with animate on g.radius = 100
a=refs.dependents g.radius.controller
pb=a[1]
tester470888 pb
*/

/*
Value*
tester470888_cf(Value** arg_list, int count)
{
	ReferenceTarget *ref = arg_list[0]->to_reftarg();
	IParamBlock2 *pblock = (IParamBlock2*)ref;

	//PDL: Reset the param and try to set a value.
	//pblock->RemoveController(eFloatParam, 0); // Uncoment this line to work around the problem
	pblock->Reset(0); 
	// This line crashes because the implementation of reset doesn't
	//properly reset the flags. In particular Reset will delete the
	//controller but not change the flag to indicate that the param
	//is now constant.
	pblock->SetValue(0, 0, 25.0f);
	return &ok;
}
*/

/*
#ifdef _DEBUG
def_visible_primitive	( test_function,		"test_function");

PolyObject* GetPolyObjectFromNode(INode *pNode, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *pObj = pNode->EvalWorldState(t).obj;
	if (pObj->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0))) { 
		PolyObject *pTri = (PolyObject *)pObj->ConvertToType(t, Class_ID(POLYOBJ_CLASS_ID,0));
		if (pObj != pTri) deleteIt = TRUE;
		return pTri;
	} else {
		return NULL;
	}
}

TriObject* GetTriObjectFromNode(INode *Node, TimeValue t, int &deleteIt) 
{
	deleteIt = FALSE;
	Object *pObj = Node->EvalWorldState(t).obj;
	if (pObj && pObj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID,0))) { 
		TriObject *pTri = (TriObject *)pObj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID,0));
		if (pObj != pTri) deleteIt = TRUE;
		return pTri;
	} else {
		return NULL;
	}

}

Value* 
test_function_cf(Value** arg_list, int count)
{
	if(MAXScript_interface->GetSelNodeCount() != 0) 
	{
		INode *pNode = MAXScript_interface->GetSelNode(0);
		BOOL deleteIt;
		PolyObject *pPolyObj = GetPolyObjectFromNode(pNode, MAXScript_interface->GetTime(), deleteIt);
		if (pPolyObj)
		{
			MNMesh *pMesh = &pPolyObj->mm;
			MNNormalSpec *pNormals = pMesh->GetSpecifiedNormals();
			if(deleteIt) delete pPolyObj;
		}
		TriObject *pTriObj = GetTriObjectFromNode(pNode, MAXScript_interface->GetTime(), deleteIt);
		if (pTriObj)
		{
			Mesh *pMesh = &pTriObj->mesh;
			MeshNormalSpec *pNormals = pMesh->GetSpecifiedNormals();
			if(deleteIt) delete pTriObj;
		}
	}
	FPValue res;
	ExecuteMAXScriptScript("maxops.pivotMode", TRUE, &res);

	static FunctionID fid;
	static FPInterface* maxops = NULL;

	if (maxops == NULL)
	{
		maxops = GetCOREInterface(MAIN_MAX_INTERFACE);
		FPInterfaceDesc* maxops_desc = maxops->GetDesc();
		for (int i = 0; i < maxops_desc->props.Count(); i++)
		{
			FPPropDef *propDef = maxops_desc->props[i];
			if (_tcsicmp("pivotMode", propDef->internal_name) == 0)
			{
				fid = propDef->getter_ID;
				break;
			}
		}
	}
	FPValue result;
	maxops->Invoke(fid, result);

	return &ok;
}

def_visible_primitive	( test_RefTargContainer,		"test_RefTargContainer");
#include "IRefTargContainer.h"

// NOTE: should cause 3 asserts to fire

Value* 
test_RefTargContainer_cf(Value** arg_list, int count)
{
	ClassDirectory& cdir = MAXScript_interface->GetDllDir().ClassDir();
	ClassDesc* cd = cdir.FindClassEntry(REF_TARGET_CLASS_ID, REFTARG_CONTAINER_CLASS_ID)->FullCD();
	ReferenceTarget *rt = (ReferenceTarget *)cd->Create();
	IRefTargContainer *rtc = (IRefTargContainer*)rt;

	int n = rtc->GetNumItems();
	assert(n == 0);
	ReferenceTarget *r = rtc->GetItem(0);
	assert(r == NULL);

	ClassDesc* bend_cd = cdir.FindClassEntry(OSM_CLASS_ID, Class_ID(BENDOSM_CLASS_ID,0))->FullCD();

	ReferenceTarget *bend1 = (ReferenceTarget *)bend_cd->Create();
	rtc->AppendItem(bend1);
	n = rtc->GetNumItems();
	assert(n == 1);
	r = rtc->GetItem(0);
	assert(r == bend1);

	ReferenceTarget *bend2 = (ReferenceTarget *)bend_cd->Create();
	rtc->AppendItem(bend2);
	n = rtc->GetNumItems();
	assert(n == 2);
	r = rtc->GetItem(0);
	assert(r == bend1);
	r = rtc->GetItem(1);
	assert(r == bend2);

	ReferenceTarget *bend3 = (ReferenceTarget *)bend_cd->Create();
	rtc->SetItem(0,bend3);
	n = rtc->GetNumItems();
	assert(n == 2);
	r = rtc->GetItem(0);
	assert(r == bend3);
	r = rtc->GetItem(1);
	assert(r == bend2);

	ReferenceTarget *bend4 = (ReferenceTarget *)bend_cd->Create();
	rtc->InsertItem(1,bend4);
	n = rtc->GetNumItems();
	assert(n == 3);
	r = rtc->GetItem(0);
	assert(r == bend3);
	r = rtc->GetItem(1);
	assert(r == bend4);
	r = rtc->GetItem(2);
	assert(r == bend2);

	ReferenceTarget *bend5 = (ReferenceTarget *)bend_cd->Create();
	rtc->AppendItem(bend5);
	n = rtc->GetNumItems();
	assert(n == 4);

	rtc->RemoveItem(0);
	n = rtc->GetNumItems();
	assert(n == 3);
	r = rtc->GetItem(0);
	assert(r == bend4);
	r = rtc->GetItem(1);
	assert(r == bend2);
	r = rtc->GetItem(2);
	assert(r == bend5);

	rtc->RemoveItem(1);
	n = rtc->GetNumItems();
	assert(n == 2);
	r = rtc->GetItem(0);
	assert(r == bend4);
	r = rtc->GetItem(1);
	assert(r == bend5);

	rtc->RemoveItem(1);
	n = rtc->GetNumItems();
	assert(n == 1);
	r = rtc->GetItem(0);
	assert(r == bend4);

	mputs(_T(" assert coming...   \n"));
	rtc->RemoveItem(1); // test assert
	r = rtc->GetItem(1);
	assert(r == NULL);

	rtc->RemoveItem(0);
	n = rtc->GetNumItems();
	assert(n == 0);

	bend1 = (ReferenceTarget *)bend_cd->Create();
	rtc->AppendItem(bend1);
	bend2 = (ReferenceTarget *)bend_cd->Create();
	rtc->AppendItem(bend2);
	n = rtc->GetNumItems();
	assert(n == 2);

	rt->MaybeAutoDelete();

	theHold.Accept(_T("phase 1"));
	theHold.Begin();

	rt = (ReferenceTarget *)cd->Create();
	rtc = (IRefTargContainer*)rt;
	bend1 = (ReferenceTarget *)bend_cd->Create();
	bend2 = (ReferenceTarget *)bend_cd->Create();
	rtc->SetItem(0,bend1);

	theHold.Accept(_T("phase 2"));
	theHold.Begin();

	rtc->SetItem(2,bend2);
	n = rtc->GetNumItems();
	assert(n == 3);
	r = rtc->GetItem(0);
	assert(r == bend1);
	r = rtc->GetItem(1);
	assert(r == NULL);
	r = rtc->GetItem(2);
	assert(r == bend2);

	theHold.Accept(_T("phase 3"));

	MAXScript_interface->ExecuteMAXCommand(MAXCOM_EDIT_UNDO);
	n = rtc->GetNumItems();
	assert(n == 1);
	r = rtc->GetItem(0);
	assert(r == bend1);

	MAXScript_interface->ExecuteMAXCommand(MAXCOM_EDIT_REDO);
	n = rtc->GetNumItems();
	assert(n == 3);
	r = rtc->GetItem(0);
	assert(r == bend1);
	r = rtc->GetItem(1);
	assert(r == NULL);
	r = rtc->GetItem(2);
	assert(r == bend2);

	MAXScript_interface->ExecuteMAXCommand(MAXCOM_EDIT_UNDO);
	n = rtc->GetNumItems();
	assert(n == 1);
	r = rtc->GetItem(0);
	assert(r == bend1);

	MAXScript_interface->ExecuteMAXCommand(MAXCOM_EDIT_REDO);
	n = rtc->GetNumItems();
	assert(n == 3);
	r = rtc->GetItem(0);
	assert(r == bend1);
	r = rtc->GetItem(1);
	assert(r == NULL);
	r = rtc->GetItem(2);
	assert(r == bend2);

	theHold.Begin();

	return &ok;
}
#endif // ifdef _DEBUG

def_visible_primitive	( test_function,		"test_function");

Value* 
test_function_cf(Value** arg_list, int count)
{
	TCHAR filename1[MAX_PATH];
	TSTR filename2 = _T("C:\\test\\");
	_tcscpy(filename1, filename2);
	BMMRemoveSlash(filename1);
	BMMRemoveSlash(filename1);
	BMMAppendSlash(filename1);
	BMMAppendSlash(filename1);

	BMMRemoveSlash(filename2);
	BMMRemoveSlash(filename2);
	BMMAppendSlash(filename2);
	BMMAppendSlash(filename2);

	filename2 = _T("C:/test/");
	_tcscpy(filename1, filename2);
	BMMRemoveSlash(filename1);
	BMMRemoveSlash(filename1);
	BMMAppendSlash(filename1);
	BMMAppendSlash(filename1);

	BMMRemoveSlash(filename2);
	BMMRemoveSlash(filename2);
	BMMAppendSlash(filename2);
	BMMAppendSlash(filename2);

	return &ok;
	}

def_visible_primitive	( test_function,		"test_function");

class  TestAnimEnumProc : public Animatable::EnumAnimList
{
public:
	int index;
	Animatable *delAnim, *delAnimNext;
	ULONG numAnims, numRefMakers, numRetTargs;
	TestAnimEnumProc(int _index, Animatable *_delAnim, Animatable *_delAnimNext) : 
		index(_index), numAnims(0), numRefMakers(0), numRetTargs(0), 
		delAnim(_delAnim), delAnimNext(_delAnimNext){}
	bool proc(Animatable *theAnim)
	{
		if (theAnim == NULL) return true;
		if (!theAnim->TestFlagBit(index))
		{
			theAnim->SetFlagBit(index);
			numAnims++;
			if (theAnim->IsRefMaker()) 
			{
				numRefMakers++;
				assert (theAnim->GetInterface(REFERENCE_MAKER_INTERFACE));
				if (((ReferenceMaker*)theAnim)->IsRefTarget()) 
				{
					numRetTargs++;
					assert (theAnim->GetInterface(REFERENCE_TARGET_INTERFACE));
				}
				else
				{
					assert (theAnim->GetInterface(REFERENCE_TARGET_INTERFACE) == NULL);
				}
			}
			else
			{
				assert (theAnim->GetInterface(REFERENCE_MAKER_INTERFACE) == NULL);
			}
		}
		if (delAnim == theAnim)
		{
			delAnimNext->DeleteThis();
		}
		return true;
	}
};

Value* 
test_function_cf(Value** arg_list, int count)
{
	int index1 = Animatable::RequestFlagBit();
	Animatable::ReleaseFlagBit(index1);
	index1 = Animatable::RequestFlagBit();
	int index2 = Animatable::RequestFlagBit();
	Animatable::ClearFlagBitInAllAnimatables(index2);
	Animatable* testAnim1 = new Animatable();
	Animatable* testAnim2 = new Animatable();
	Animatable* testAnim3 = new Animatable();
	TestAnimEnumProc testAnimEnumProc1(index2, testAnim1, testAnim2);
	testAnim2 = NULL;
	Animatable::EnumerateAllAnimatables(testAnimEnumProc1);
	DebugPrint(_T("# anims: %d; # refmakers: %d; # reftargs: %d\n"), 
		testAnimEnumProc1.numAnims,
		testAnimEnumProc1.numRefMakers,
		testAnimEnumProc1.numRetTargs);
	TestAnimEnumProc testAnimEnumProc2(index2, testAnim1, testAnim3);
	testAnim3 = NULL;
	Animatable::EnumerateAllAnimatables(testAnimEnumProc2);
	Animatable::ClearFlagBitInAllAnimatables(index2);
	TestAnimEnumProc testAnimEnumProc3(index2, NULL, NULL);
	Animatable::EnumerateAllAnimatables(testAnimEnumProc3);
	testAnim1->DeleteThis();
	testAnim1 = NULL;
	Animatable::ReleaseFlagBit(index2);
	Animatable::ReleaseFlagBit(index1);
	return &ok;
}


def_visible_primitive	( test_function,		"test_function");

Value*
test_function_cf(Value** arg_list, int count)
{
	CStr cstr = "Hello there world";
	WStr wstr = "Hello there sun";
	DebugPrint(_T("WStr: %s; CStr: %s;\n"), wstr.data(), cstr.data());
	return &undefined; 
}

def_visible_primitive	( test_function,		"test_function");

int MBtoBytes( int i) { return i*1024*1024; }
int KBtoBytes( int i) { return i*1024; }
Value*
test_function_cf(Value** arg_list, int count)
{
	// case 1
	void *ptr1 = malloc(MBtoBytes(5));
	void *ptr1r = realloc(ptr1, 100);
	if (ptr1r) ptr1 = ptr1r;
	void *ptr2 = malloc(MBtoBytes(5));
	free(ptr1);
	free(ptr2);

	// case 2
	ptr1 = malloc(MBtoBytes(5));
	ptr1r = realloc(ptr1, 100);
	if (ptr1r) ptr1 = ptr1r;
	HeapCompact (GetProcessHeap(), 0);
	ptr2 = malloc(MBtoBytes(5));
	free(ptr1);
	free(ptr2);

	// case 3
	ptr1 = malloc(MBtoBytes(5));
	HeapCompact (GetProcessHeap(), 0);
	ptr2 = malloc(MBtoBytes(5));
	free(ptr2);
	ptr2 = malloc(MBtoBytes(6));
	free(ptr1);
	free(ptr2);

	return &ok; 
}

*/
