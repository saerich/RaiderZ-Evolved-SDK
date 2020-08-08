/**********************************************************************
 *<
	FILE: MorphByBone.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:  Peter Watje

	HISTORY:
		This is a morph modifier that is driven by bone orientation.  Basically the user 
		defines an orientation of a bone and then a morph target is assigned to that orientation.
		The morphs deltas are stored in the bones parent space ( or maybe local space).

		The angles between the axis tripod of the morph target and the current bone axis tripod
		is used to determine how close the bone is to the morph.



 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __MORPHBYBONE__H
#define __MORPHBYBONE__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "icurvctl.h"
#include "macrorec.h"

#include "meshadj.h"
#include "XTCObject.h"

#include "modstack.h"
#include "iMorphByBone.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define MORPHBYBONE_CLASS_ID	Class_ID(0x7ad09741, 0x69842453)

#define GROW_SEL	0
#define SHRINK_SEL	1
#define LOOP_SEL	2
#define RING_SEL	3

enum { morphbybone_params };
enum { morphbybone_mapparams,morphbybone_mapparamsprop,morphbybone_mapparamscopy, morphbybone_mapparamsoptions,morphbybone_mapparamsselection };



//TODO: Add enums for various parameters
enum { 
	pb_bonelist,			// this is a list of all the driver bones.  This is the child bone that drives the morph target
	pb_lname,				// this is a temporary text field used to show and edit morph names
	pb_linfluence,			
	pb_lfalloff,
	pb_showmirror,
	pb_mirrorplane,
	pb_mirroroffset,
	pb_mirrortm,
	pb_mirrorthreshold,
	pb_mirrorpreviewverts,
	pb_mirrorpreviewbone,
	pb_ljointtype,

//	pb_ltargnode,
	pb_targnodelist,
	pb_falloffgraphlist,
	pb_reloadselected,

	pb_safemode,
	pb_showdriverbone,pb_showmorphbone,
	pb_showcurrentangle,pb_showanglelimit,
	pb_matrixsize,
	pb_showdeltas,
	pb_drawx,pb_drawy,pb_drawz,
	pb_bonesize,

	pb_usesoftselection,
	pb_edgelimit,
	pb_selectiongraph,
	pb_selectionradius,
	pb_useedgelimit,

	pb_lenabled,
	pb_showedges


};



#define PBLOCK_REF	0

class MorphByBone;

class MorphData
{
public:
	int vertexID;
	Point3 vec;
	Point3 vecParentSpace;
	Point3 originalPt;
	int localOwner;
};

#define FLAG_DEADMORPH		1
#define FLAG_BALLJOINT		2
#define FLAG_PLANARJOINT	4
#define FLAG_MULTISELECTED	8
#define FLAG_DISABLED		16

#define FALLOFF_LINEAR	0
#define FALLOFF_SINUAL	1
#define FALLOFF_FAST	2
#define FALLOFF_SLOW	3
#define FALLOFF_CUSTOM	4

class MorphByBoneData;

class MorphTargets
{
public:
	Tab<MorphData> d;
//	int boneID;
	Matrix3 boneTMInParentSpace;
	Matrix3 parentTM;
	float influenceAngle;
	int falloff;
	TSTR name;
	HTREEITEM treeHWND;
	TVITEM tvi;
	float weight;
	int flags;
	int externalNodeID;
	int externalFalloffID;
	ICurve *curve;
	BOOL compressed;


	MorphTargets()
		{
		compressed = TRUE;
		curve = NULL;
		externalNodeID = -1;
		externalFalloffID = -1;
		weight = 0.0f;
		flags = 0;
		falloff = FALLOFF_LINEAR;
		}

	BOOL IsDead() {return flags & FLAG_DEADMORPH;}
	void SetDead(BOOL dead) 
		{ 
		if (dead) flags |= FLAG_DEADMORPH;
			else flags &= ~FLAG_DEADMORPH;
		}

	BOOL IsDisabled() {return flags & FLAG_DISABLED;}
	void SetDisabled(BOOL disabled) 
		{ 
		if (disabled) flags |= FLAG_DISABLED;
			else flags &= ~FLAG_DISABLED;
		}

	void CreateGraph(MorphByBone *mod, IParamBlock2 *pblock)
		{
		if (externalFalloffID == -1)
			{
			
			int slot = -1;
			//look for an empty slot to add it to or append it to the list
			for (int i = 0; i < pblock->Count(pb_falloffgraphlist); i++)
				{
				ReferenceTarget *ref = NULL;
				pblock->GetValue(pb_falloffgraphlist,0,ref,FOREVER,i);
				if (ref == NULL)
					{
					slot = i;
					i = pblock->Count(pb_falloffgraphlist);
					}
				}
			//create a graph
			ICurveCtl *graph;
			
			graph = (ICurveCtl *) CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID);

			graph->SetXRange(0,1.0);
			graph->SetYRange(0.0f,1.0f);

			graph->RegisterResourceMaker((ReferenceMaker *)mod);
			graph->SetNumCurves(1);
			

			graph->SetTitle(GetString(IDS_PW_GRAPH));
			graph->SetZoomValues( 2.7f, 80.0f);
			
			BitArray ba;
			ba.SetSize(1);
			ba.SetAll();
			graph->SetDisplayMode(ba);
			graph->SetCCFlags(CC_ASPOPUP|CC_DRAWBG|CC_DRAWSCROLLBARS|CC_AUTOSCROLL|
							  CC_DRAWRULER|CC_DRAWGRID|
							  CC_RCMENU_MOVE_XY|CC_RCMENU_MOVE_X|CC_RCMENU_MOVE_Y|
							  CC_RCMENU_SCALE|CC_RCMENU_INSERT_CORNER | CC_RCMENU_INSERT_BEZIER|
							CC_RCMENU_DELETE | CC_SHOW_CURRENTXVAL	
							);	

			ICurve *curve = graph->GetControlCurve(0);

			if (curve)
				{

				CurvePoint pt;
				curve->SetNumPts(2);

				pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
				pt.out.x = 0.33f;
				pt.out.y = 0.0f;
				pt.in.x = -0.33f;
				pt.in.y = 0.0f;

				pt.p.x = 0.0f;			
				pt.p.y = 0.0f;			
				curve->SetPoint(0,0,&pt);
					
				pt.p.x = 1.0f;			
				pt.p.y = 1.0f;			
				pt.in.x = -.33f;
				pt.in.y = 0.0f;
				pt.out.x = .33f;
				pt.out.y = 0.0f;

				curve->SetPoint(0,1,&pt);
				curve->SetLookupTableSize(500);
				}

			//assign it to our id
			if (slot != -1)
				{
				externalFalloffID = slot;
				pblock->SetValue(pb_falloffgraphlist,0,(ReferenceTarget*)graph,slot);
				}
			else
				{
				ReferenceTarget *ref = (ReferenceTarget*)graph;
				pblock->Append(pb_falloffgraphlist,1,&ref);
				externalFalloffID = pblock->Count(pb_falloffgraphlist)-1;
				}

			}
		}



	MorphTargets *Clone()
		{
		MorphTargets *copy = new MorphTargets();
		copy->d = d;
		copy->boneTMInParentSpace = boneTMInParentSpace;
		copy->influenceAngle = influenceAngle;
		copy->falloff = falloff;
		copy->name = name;
		copy->flags = flags;
		treeHWND = NULL;
		return copy;
		}

	void CompressList(bool compress,float size)
	{
		compressed = compress;
		if (compress)
		{
		}
		else
		{
/*
			Tab<MorphData> tempd;
			tempD = d;
			d.SetCount(size);
			for (int i = 0; i < tempD.Count(); i++)
			{
			}

			for (int i = 0; i < tempD.Count(); i++)
			{
				index = tempD[i].vertexID;				
				d[index] = tempD[i];
			}
*/

		}
	}

	void Dump()
	{
		DebugPrint(_T("Morph Name : %s\n"),name);
		DebugPrint(_T("Morph TM in parent Space:\n"));
		Matrix3 tm = boneTMInParentSpace;
		for (int i = 0; i < 3; i++)
			DebugPrint(_T("   [%f,%f,%f]\n"),tm.GetRow(i).x,tm.GetRow(i).y,tm.GetRow(i).z);
		tm = parentTM;
		DebugPrint(_T("Morph TM in parent Space:\n"));
		for (int i = 0; i < 3; i++)
			DebugPrint(_T("   [%f,%f,%f]\n"),tm.GetRow(i).x,tm.GetRow(i).y,tm.GetRow(i).z);

		DebugPrint(_T("Morph Angle : %f\n"),influenceAngle);
		DebugPrint(_T("Morph falloff : %d\n"),falloff);
		DebugPrint(_T("Morph wieght : %f\n"),weight);
		DebugPrint(_T("Morph flags : %d\n"),flags);
		DebugPrint(_T("Morph externalNodeID : %d\n"),externalNodeID);
		DebugPrint(_T("Morph externalFalloffID : %d\n"),externalFalloffID);
		DebugPrint(_T("\n"));
		DebugPrint(_T("Morph Count : %d\n"),d.Count());
		for (int i = 0; i < d.Count(); i++)
		{
			DebugPrint(_T("	ID %d\n"),d[i].vertexID);
			DebugPrint(_T("	localOwner %d\n"),d[i].localOwner);
			Point3 v;
			v = d[i].vec;
			DebugPrint(_T("	vec  %f %f %f\n"),v.x,v.y,v.x);
			v = d[i].vecParentSpace;
			DebugPrint(_T("	vecP %f %f %f\n"),v.x,v.y,v.x);
			v = d[i].originalPt;
			DebugPrint(_T("	Pt   %f %f %f\n"),v.x,v.y,v.x);
		}


//		for int
	}

};

class EdgeConnections
{
public:
	Tab<int> verts;
};

class MorphByBoneLocalData : public LocalModData 
{
public:
	

	int id;
	Tab<float> softSelection;
	Matrix3 selfNodeTM;
	Matrix3 selfObjectTM;
	INode *selfNode;

	Tab<Point3> tempPoints;
	Tab<int> tempMatchList;

	Tab<Point3> originalPoints;

	Tab<Point3> postDeform;
	Tab<Point3> preDeform;
	Matrix3 selfNodeCurrentTM;
	Matrix3 selfObjectCurrentTM;
	
	BOOL rebuildSoftSelection;


	MorphByBoneLocalData()
		{
		rebuildSoftSelection = FALSE;
		id = -1;
		selfNode = NULL;
		}
	~MorphByBoneLocalData()
		{
		}
	LocalModData *Clone()
		{
		MorphByBoneLocalData* d = new MorphByBoneLocalData();
		d->connectionList.ZeroCount();
		d->id = id;
		return d;
		}	
	void ComputeSoftSelection(float radius, ICurve *pCurve, BOOL useEdgeLimit, int edgeLimit)
	{
		//build or edgelimit list
		BitArray useVerts;
		useVerts.SetSize(postDeform.Count());
		useVerts.SetAll();
		if (useEdgeLimit)
		{
			if (connectionList.Count() == postDeform.Count())
			{
				useVerts.ClearAll();
				for (int i = 0; i < postDeform.Count(); i++)
				{
					if (softSelection[i] == 1.0f)
					{
						useVerts.Set(i,TRUE);
					}
				}
				BitArray newVerts;
				newVerts.SetSize(postDeform.Count());
				for (int i = 0; i < edgeLimit; i++)
				{
					newVerts.ClearAll();
					for (int j = 0; j < postDeform.Count(); j++)
					{
						if (useVerts[j])
						{
							int numberOfConnectedVerts = connectionList[j]->verts.Count();
							for (int k = 0; k < numberOfConnectedVerts; k++)
							{
								int index = connectionList[j]->verts[k];
								newVerts.Set(index,TRUE);
							}
	
						}
					}

					useVerts |= newVerts;

				}


			}
		}
		//get our bounds of our selected verts
		Box3 bounds;
		bounds.Init();
		Tab<int> selectedIDs;
		//expand bounds by the radius
		for (int i = 0; i < postDeform.Count(); i++)
		{
			if (softSelection[i] == 1.0f)
			{
				bounds += postDeform[i];
				selectedIDs.Append(1,&i,500);
			}
		}

		bounds.EnlargeBy(radius);
		//clear our non 1 selected verts
		for (int i = 0; i < postDeform.Count(); i++)
		{
			if (softSelection[i] != 1.0f)
				softSelection[i] = 0.0f;
		}

		
		//find all the verts in the bounding box
		for (int i = 0; i < postDeform.Count(); i++)
		{
			Point3 b = postDeform[i];
			Point3 ba;
			if (bounds.Contains(b) && (softSelection[i] != 1.0f) && useVerts[i])
			{		
		//get the distance
				float closestDist = -1.0f;
				int closestIndex = -1;
				for (int j = 0; j <  selectedIDs.Count(); j++)
				{
		//normalize it and get the weight
					int index = selectedIDs[j];
					Point3 a = postDeform[index];
					float dist = LengthSquared(a-b);
					if ((dist < closestDist) || (closestIndex == -1))
					{
						closestDist = dist; 					
						closestIndex = j;
						ba = a;
					}
				}	
				if (closestIndex != -1)
				{
					float dist = Length(b-ba);
					if (dist < radius)
					{
						float per = dist/radius;
						per = pCurve->GetValue(0,per,FOREVER,TRUE);
						softSelection[i] = per;
					}
				}
			}
		}						
	}

	Tab<EdgeConnections*> connectionList;
	void BuildConnectionList(Object *obj);
	void FreeConnectionList();



};

class MorphByBoneData

{
public: 
//	INode *node;
	Matrix3 intialBoneNodeTM;
	Matrix3 intialBoneObjectTM;
	Matrix3 intialParentNodeTM;
	Box3 localBounds;
	Tab<MorphTargets*> morphTargets;

	Matrix3 currentBoneNodeTM;
	Matrix3 currentBoneObjectTM;
	HTREEITEM treeHWND;
	TVITEM tvi;

	Matrix3 parentBoneNodeCurrentTM;
	Matrix3 parentBoneObjectCurrentTM;

	int flags;

	BOOL IsBallJoint() { return flags & FLAG_BALLJOINT; }
	void SetBallJoint()
		{
		flags |= FLAG_BALLJOINT;
		flags &= ~FLAG_PLANARJOINT;
		}
	BOOL IsPlanarJoint() { return flags & FLAG_PLANARJOINT; }
	void SetPlanarJoint()
		{
		flags |= FLAG_PLANARJOINT;
		flags &= ~FLAG_BALLJOINT;
		}

	BOOL IsMultiSelected() { return flags & FLAG_MULTISELECTED; }
	void SetMultiSelected(BOOL selected)
		{
		if (selected)
			flags |= FLAG_MULTISELECTED;
		else flags &= ~FLAG_MULTISELECTED;
		}


	MorphByBoneData()
		{
		flags = FLAG_BALLJOINT;
		}
	~MorphByBoneData() 
		{ 
			for (int i = 0; i < morphTargets.Count(); i++)  
			{
				delete morphTargets[i]; morphTargets[i] = NULL;	
			}
		}

	MorphByBoneData *Clone(BOOL all = TRUE)
		{
		MorphByBoneData *copyData = new MorphByBoneData();
		copyData->currentBoneNodeTM = currentBoneNodeTM;
		copyData->currentBoneObjectTM = currentBoneObjectTM;
		copyData->intialParentNodeTM = intialParentNodeTM;
		copyData->intialBoneNodeTM = intialBoneNodeTM;
		copyData->intialBoneObjectTM = intialBoneObjectTM;
		copyData->localBounds = localBounds;
//		copyData->node = node;
		copyData->parentBoneNodeCurrentTM = parentBoneNodeCurrentTM;
		copyData->parentBoneObjectCurrentTM = parentBoneObjectCurrentTM;
		copyData->treeHWND = NULL;
		copyData->flags = FLAG_BALLJOINT;
//hmm what to do with this		copyData->tvi = ??

		copyData->morphTargets.SetCount(morphTargets.Count());

		for (int i = 0; i < morphTargets.Count(); i++)
			{
			copyData->morphTargets[i] = morphTargets[i]->Clone();			
			}

		return copyData;

		}

	void DeleteThis()
		{
		delete this;
		}
};

class CopyBuffer
{
public:
	MorphByBoneData *copyData;
	CopyBuffer()
		{
		copyData = NULL;
		}
//if which morph == -1 all targets are copied
	void CopyToBuffer(MorphByBoneData *data);
	void PasteToBuffer(MorphByBone *mod, MorphByBoneData *targ, Matrix3 mirrorTM, float threshold, BOOL flipTM);
};

class MorphByBone : public Modifier, public  IMorphByBone {
	public:

		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode	*rotMode;

		static UScaleModBoxCMode	*uscaleMode;
		static NUScaleModBoxCMode	*nuscaleMode;
		static SquashModBoxCMode	*squashMode;
		static SelectModBoxCMode	*selectMode;



		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

		//From Modifier
		ChannelMask ChannelsUsed()  { return GEOM_CHANNEL|TOPO_CHANNEL; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return GEOM_CHANNEL; }
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return defObjectClassID;}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);


		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return FALSE;}		
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		BOOL HasUVW();
		void SetGenUVW(BOOL sw);


		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);


		Interval GetValidity(TimeValue t);

		// Automatic texture support
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
		IOResult SaveLocalData(ISave *isave, LocalModData *pld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);


		//From Animatable
		Class_ID ClassID() {return MORPHBYBONE_CLASS_ID;}		
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

		void DrawMirrorData(GraphicsWindow *gw, MorphByBoneLocalData *ld);

		void DrawBox(GraphicsWindow *gw, Box3 box, float size, BOOL makeThin = FALSE, Matrix3 *tm = NULL);
		void Draw3dEdge(GraphicsWindow *gw, float size, Box3 box, Color c, Matrix3 *tm = NULL);
		void DrawArc(GraphicsWindow *gw, float dist, float angle,float thresholdAngle, Point3 color);
		void DrawCap(GraphicsWindow *gw, Matrix3 tm, int axis, float dist, float angle, Point3 color);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);


		void GetWorldBoundBox(	TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);

	


		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);

		void ActivateSubobjSel(int level, XFormModes& modes);
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);

		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);


		void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );

		void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin);
		void Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);


		void TransformStart(TimeValue t);
		void TransformFinish(TimeValue t);

		INode* GetNodeFromModContext(ModContext *mcd);

		//Constructor/Destructor

		MorphByBone();
		~MorphByBone();
		
		Tab<MorphByBoneData*> boneData;
		int currentBone, currentMorph;

		HWND rollupHWND;
		HWND rollupPropHWND;
//		void AddItemToTree(HWND hwndTV, int nodeID);
		void BuildTreeList();
//		void UpdateTreeListWeights();

		void AddNodes();

		INode *GetNode(int id);

		Tab<MorphByBoneLocalData*> localDataList;

		void SetSelection(int id, BOOL updateWindow = FALSE);

		float AngleBetweenTMs(Matrix3 tmA, Matrix3 tmB);

		void CreateMorph(INode *node, BOOL force = FALSE);
		void SetMorph(Point3 vec);
		void SetMorph(Matrix3 tm, Matrix3 itm);

		void DeleteMorph(int whichBone, int whichMorph);
		void DeleteBone(int whichBone);

		void Deform(Object *obj, int ldID);

		static ICustButton	*iEditButton ;
		static ICustButton	*iNodeButton ;
		static ICustButton	*iPickBoneButton ;
		static ICustButton	*iDeleteMorphButton ;
		static ICustButton	*iCreateMorphButton ;
		static ICustButton	*iResetMorphButton ;
		static ICustButton	*iClearVertsButton ;
		static ICustButton	*iRemoveVertsButton ;

		static ICustButton	*iReloadButton;
		static ICustButton	*iGraphButton;
		static ICustEdit	*iNameField;
		static ISpinnerControl		*iInfluenceAngleSpinner;
		static HWND hwndFalloffDropList;
		static HWND hwndJointTypeDropList;
		static HWND hwndSelectedVertexCheckBox;
		static HWND hwndEnabledCheckBox;

		static ICustButton	*iMultiSelectButton ;
		BOOL multiSelectMode;
		BOOL editMorph;

		void SetMorphName(TCHAR *s, int whichBone, int whichMorph);	
		void SetMorphInfluence(float f, int whichBone, int whichMorph);	
		float GetFalloff(float v, int falloff, ICurve *curve);
		void SetMorphFalloff(int falloff, int whichBone, int whichMorph);	
		void SetMorphEnabled(BOOL enabled,int whichBone, int whichMorph);

		void SetJointType(int type, int whichBone);	
		void SetNode(INode *node,int whichBone, int whichMorph);

		BOOL pauseAccessor;
		void UpdateLocalUI();

		BOOL pasteMode;

		Matrix3 mirrorTM;
		BOOL showMirror;
		Box3 bounds;
		int mirrorPlane;
		float mirrorOffset;

		BOOL previewBones;
		BOOL previewVertices;


		CopyBuffer copyBuffer;

		void CopyCurrent(int id = -1);
		void PasteMirrorCurrent();
		void PasteToMirror();
		
		void MirrorMorph(int sourceIndex, int targetIndex, BOOL mirrorData);

		BOOL suspendUI;

		void ClearSelectedVertices(bool deleteVerts);
		int GetMirrorBone(int id = -1);

		void ReloadMorph(int whichBone, int whichMorph);
		void BringUpGraph(int whichBone, int whichMorph);
		void ResetSelectionGraph();

		BOOL safeMode;
			
		void ResetMorph(int whichBone, int whichMorph);

		ICurveCtl* GetCurveCtl();

		void HoldSelections();
		void HoldBoneSelections();

		void GrowVerts(BOOL grow);
		BOOL GrowVertSel(BitArray &sel, INode *node, int mode);

		void EdgeSel(BOOL loop);
		BOOL EdgeSel(BitArray &sel, INode *node, int mode);

		BaseInterface* GetInterface(Interface_ID id) 
			{ 
			if (id == MORPHBYBONE_INTERFACE) 
				return (IMorphByBone*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
			} 

		int		GetBoneIndex(INode *node);
		
		TCHAR*	GetMorphName(INode *node, int index);
		int		GetMorphIndex(INode *node, TCHAR *name);

		void	AddBone(INode *node);
		void	fnAddBone(INode *node);
		void	fnRemoveBone(INode *node);
		void	fnSelectBone(INode *node,TCHAR* morphName);
		INode*	fnGetSelectedBone();
		TCHAR*	fnGetSelectedMorph();

		void	fnSelectVertices(INode *node,BitArray *sel);
		BOOL	fnIsSelectedVertex(INode *node,int index);
		void	fnResetGraph();
		void	fnShrink();
		void	fnGrow();
		void	fnLoop();
		void	fnRing();

		void	fnCreateMorph(INode *node);
		void	fnRemoveMorph(INode *node, TCHAR *name);

		void	fnEdit(BOOL edit);

		void	fnClearSelectedVertices();
		void	fnDeleteSelectedVertices();

		void	fnResetOrientation(INode *node, TCHAR *name);

		void	fnReloadTarget(INode *node, TCHAR *name);
		void	fnMirrorPaste(INode *node);

		void	fnEditFalloffGraph(INode *node, TCHAR *name);
		void	fnSetExternalNode(INode *node, TCHAR *name,INode *exnode);

		void	fnMoveVerts(Point3 vec);
		void	fnTransFormVerts(Matrix3 a, Matrix3 b);

		int		fnNumberOfBones();

		Matrix3*	fnBoneGetInitialNodeTM(INode *node);
		void	fnBoneSetInitialNodeTM(INode *node, Matrix3 tm);

		Matrix3*	fnBoneGetInitialObjectTM(INode *node);
		void	fnBoneSetInitialObjectTM(INode *node, Matrix3 tm);

		Matrix3*	fnBoneGetInitialParentTM(INode *node);
		void	fnBoneSetInitialParentTM(INode *node, Matrix3 tm);

		int		fnBoneGetNumberOfMorphs(INode *node);

		TCHAR*	fnBoneGetMorphName(INode *node, int morphIndex);
		void	fnBoneSetMorphName(INode *node, int morphIndex,TCHAR* name);

		float	fnBoneGetMorphAngle(INode *node, int morphIndex);
		void	fnBoneSetMorphAngle(INode *node, int morphIndex,float angle);

		Matrix3* fnBoneGetMorphTM(INode *node, int morphIndex);
		void	 fnBoneSetMorphTM(INode *node, int morphIndex,Matrix3 tm);

		Matrix3* fnBoneGetMorphParentTM(INode *node, int morphIndex);
		void	 fnBoneSetMorphParentTM(INode *node, int morphIndex,Matrix3 tm);

		BOOL	 fnBoneGetMorphIsDead(INode *node, int morphIndex);
		void	 fnBoneSetMorphSetDead(INode *node, int morphIndex,BOOL dead);


		int		 fnBoneGetMorphNumPoints(INode *node, int morphIndex);
		void	 fnBoneSetMorphNumPoints(INode *node, int morphIndex, int numberPoints);


		int		 fnBoneGetMorphVertID(INode *node, int morphIndex, int ithIndex);
		void	 fnBoneSetMorphVertID(INode *node, int morphIndex, int ithIndex, int vertIndex);

		Point3*	 fnBoneGetMorphVec(INode *node, int morphIndex, int ithIndex);
		void	 fnBoneSetMorphVec(INode *node, int morphIndex, int ithIndex, Point3 vec);

		Point3*	 fnBoneGetMorphPVec(INode *node, int morphIndex, int ithIndex);
		void	 fnBoneSetMorphPVec(INode *node, int morphIndex, int ithIndex, Point3 vec);

		Point3*	 fnBoneGetMorphOP(INode *node, int morphIndex, int ithIndex);
		void	 fnBoneSetMorphOP(INode *node, int morphIndex, int ithIndex, Point3 vec);

		INode*	 fnBoneGetMorphOwner(INode *node, int morphIndex, int ithIndex);
		void	 fnBoneSetMorphOwner(INode *node, int morphIndex, int ithIndex, INode *onode);

		int		 fnBoneGetMorphFalloff(INode *node, int morphIndex);
		void	 fnBoneSetMorphFalloff(INode *node, int morphIndex, int falloff);

		int		 fnBoneGetJointType(INode *node);
		void	 fnBoneSetJointType(INode *node, int jointType);

		void	 fnUpdate();

		float	fnGetWeight(INode *node, TCHAR*name);

		BOOL	 fnBoneGetMorphEnabled(INode *node, int morphIndex);
		void	 fnBoneSetMorphEnabled(INode *node, int morphIndex, BOOL enabled);



		BOOL    scriptMoveMode;
		Point3  scriptMoveVec;
		Matrix3 scriptTMA,scriptTMB;
};


class MorphByBoneDeformer : public Deformer {
	public:	
		Point3 *pList;
		MorphByBoneLocalData *ld;

		MorphByBoneDeformer(MorphByBoneLocalData *ld,Point3 *pList);
		Point3 Map(int i, Point3 p);
	};


class MorphByBoneParamsMapDlgProc : public ParamMap2UserDlgProc {
	public:
		MorphByBone *mod;		
		MorphByBoneParamsMapDlgProc(MorphByBone *m) {mod = m;}		
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};

class MorphByBoneParamsMapDlgProcProp : public ParamMap2UserDlgProc {
	public:
		MorphByBone *mod;		
		MorphByBoneParamsMapDlgProcProp(MorphByBone *m) {mod = m;}		
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};

class MorphByBoneParamsMapDlgProcCopy : public ParamMap2UserDlgProc {
	public:
		MorphByBone *mod;		
		MorphByBoneParamsMapDlgProcCopy(MorphByBone *m) {mod = m;}		
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};

class MorphByBoneParamsMapDlgProcSoftSelection : public ParamMap2UserDlgProc {
	public:
		MorphByBone *mod;		
		MorphByBoneParamsMapDlgProcSoftSelection(MorphByBone *m) {mod = m;}		
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};


class PickControlNode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:				
		MorphByBone *mod;
		BOOL pickExternalNode;
		PickControlNode() {
					mod=NULL;
					pickExternalNode = TRUE;
					};
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
		HCURSOR GetDefCursor(IObjParam *ip);
		HCURSOR GetHitCursor(IObjParam *ip);
	};

class DumpHitDialog : public HitByNameDlgCallback {
public:
	MorphByBone *eo;
	DumpHitDialog(MorphByBone *e) {eo=e;};
	TCHAR *dialogTitle() {return _T(GetString(IDS_PW_SELECTBONES));};
	TCHAR *buttonText() {return _T(GetString(IDS_PW_SELECT));};
	BOOL singleSelect() {return FALSE;};
	BOOL useProc() {return TRUE;};
	int filter(INode *node);
	void proc(INodeTab &nodeTab);
	};



class MorphByBoneClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE);// { return new MorphByBone(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return MORPHBYBONE_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("MorphByBone"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};


class MeshEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};




class MorphRestore : public RestoreObj {
	public:
		MorphByBone *mod;

		int whichBone;
		int whichMorph;

		Tab<MorphData> uVec,rVec;

		MorphRestore(MorphByBone *c, int bone, int morph) 
			{
			mod = c;
			whichBone = bone;
			whichMorph = morph;

			if ( (bone >= 0) && (bone < mod->boneData.Count()))
				uVec = mod->boneData[bone]->morphTargets[morph]->d;

			}   		
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				if ((whichBone >= 0) && (whichBone < mod->boneData.Count()))
					rVec = mod->boneData[whichBone]->morphTargets[whichMorph]->d;
				}

			if ((whichBone >= 0) && (whichBone < mod->boneData.Count()))
				mod->boneData[whichBone]->morphTargets[whichMorph]->d = uVec;

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		void Redo()
			{
			if ((whichBone >= 0) && (whichBone < mod->boneData.Count()))
				mod->boneData[whichBone]->morphTargets[whichMorph]->d = rVec;

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_MOVE))); }
	};



class SelectionRestore : public RestoreObj {
	public:
		MorphByBone *mod;
		MorphByBoneLocalData *ld;

		Tab<float> uSel,rSel;

		SelectionRestore(MorphByBoneLocalData *c,MorphByBone *m) 
			{
			mod = m;
			ld = c;
			uSel = ld->softSelection;

			}   		
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				rSel = ld->softSelection;
				}
			ld->softSelection = uSel;
			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		void Redo()
			{
			ld->softSelection = rSel;
			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_SELECTION))); }
	};


class BoneSelectionRestore : public RestoreObj {
	public:
		MorphByBone *mod;
		

		int uSelBone,rSelBone;
		int uSelMorph,rSelMorph;

		BoneSelectionRestore(MorphByBone *m) 
			{
			mod = m;
			uSelBone = mod->currentBone;
			uSelMorph = mod->currentMorph;

			}   		
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				rSelBone = mod->currentBone;
				rSelMorph = mod->currentMorph;
				}

			mod->currentBone = uSelBone;
			mod->currentMorph = uSelMorph;

			mod->UpdateLocalUI();
			mod->BuildTreeList();

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		void Redo()
			{
			mod->currentBone = rSelBone;
			mod->currentMorph = rSelMorph;

			mod->UpdateLocalUI();
			mod->BuildTreeList();

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_SELECTION))); }
	};

class MorphUIRestore : public RestoreObj 
{
	public:
		MorphByBone *mod;
		MorphUIRestore(MorphByBone *m) 
			{
			mod = m;
			}   
		void Restore(int isUndo) 
			{
			mod->UpdateLocalUI();
			mod->BuildTreeList();

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		void Redo()
			{

			mod->UpdateLocalUI();
			mod->BuildTreeList();

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_SELECTION))); }


};

class CreateMorphRestore : public RestoreObj 
{
	public:
		MorphByBone *mod;
		MorphTargets *morphTarget;
		
		CreateMorphRestore(MorphByBone *m, MorphTargets *target) 
			{
			mod = m;	
			morphTarget = target;
			}   
		void Restore(int isUndo) 
			{
			morphTarget->SetDead(TRUE);
			mod->UpdateLocalUI();
			mod->BuildTreeList();

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		void Redo()
			{

			morphTarget->SetDead(FALSE);
			mod->UpdateLocalUI();
			mod->BuildTreeList();

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_SELECTION))); }


};

class RemoveMorphRestore : public RestoreObj 
{
	public:
		MorphByBone *mod;
		MorphTargets *morphTarget;
		
		RemoveMorphRestore(MorphByBone *m, MorphTargets *target) 
			{
			mod = m;	
			morphTarget = target;
			}   
		void Restore(int isUndo) 
			{
			morphTarget->SetDead(FALSE);
			mod->UpdateLocalUI();
			mod->BuildTreeList();

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		void Redo()
			{

			morphTarget->SetDead(TRUE);
			mod->UpdateLocalUI();
			mod->BuildTreeList();

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_SELECTION))); }


};




class ResetOrientationMorphRestore : public RestoreObj 
{
	public:
		MorphByBone *mod;
		MorphTargets *morphTarget;
		Matrix3 uTM,rTM;
		
		ResetOrientationMorphRestore(MorphByBone *m, MorphTargets *target) 
			{
			mod = m;	
			morphTarget = target;
			uTM = morphTarget->boneTMInParentSpace;
			}   
		void Restore(int isUndo) 
			{

			if (isUndo)
				rTM = morphTarget->boneTMInParentSpace;

			morphTarget->boneTMInParentSpace = uTM;


			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		void Redo()
			{

			morphTarget->boneTMInParentSpace = rTM;

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_RESETORIENTATION))); }


};





#endif // __MORPHBYBONE__H
