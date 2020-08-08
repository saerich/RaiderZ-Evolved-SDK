/**********************************************************************
 *
	FILE: Link_cnstrnt.h

	DESCRIPTION: A transform constraint controller to control animated links 
				
	CREATED BY: Rolf Berteig 1/25/97
	Added features and Transformed for paramblock2 compatibility 
			: Ambarish Goswami 7/2000
			MacrRecorder related code: Larry Minton 5/2001

	            

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once

class LinkConstTransform; 
class LinkTimeControl;

// from R3.1 to 10/27
#define LINK_CONSTRAINT_V1 310
#define LINK_CONSTRAINT_V2 390
// after 10/27, before 18/9/2008
#define LINK_CONSTRAINT_V3 400
// from 18/9/2008
#define LINK_CONSTRAINT_V4 450


#define LINKCTRL_NAME		GetString(IDS_RB_LINKCTRL)

enum {	link_target_list,		link_key_mode,				link_start_time, 
		link_key_nodes_mode,	link_key_hierarchy_mode, link_key_selection};


class LinkConstTimeChangeCallback : public TimeChangeCallback
{
	public:
		LinkConstTransform* link_controller;
		void TimeChanged(TimeValue t);
};


class LinkConstTransform : public ILinkCtrl  ,public ILockedTrackImp
{
	public:

		Control *tmControl;		// ref 0

		static HWND hWnd;
		static IObjParam *ip;
		static LinkConstTransform *editCont;
		static BOOL valid;
		static ISpinnerControl *iTime;
		static ICustButton *iPickOb, *iDelOb;
		int spDownVal, spUpVal;
		int spDownSel, spUpSel;
		int linkConstraintVersion;
		bool bSortActive;
		bool bRedrawListbox; //if true we redraw the list box in GetValue. We do this as an optimization so that we don't continually
							 //redraw the list box when we receive notifications from the undo system that values have changed.


		Tab<INode*> nodes;		// ref 1-n -- Rolf's old variable
		Tab<TimeValue> times;  // Rolf's old variable

		int state;	// LAM - for controlling macroRecorder output line emittor

		LinkConstTransform(BOOL loading=FALSE);


//		LinkConstValidatorClass validator;							
		void RedrawListbox(TimeValue t, int sel = -1);
		int last_selection;
		int frameNoSpinnerUp;
		int previous_value;
		LinkConstTimeChangeCallback linkConstTimeChangeCallback;	
		IParamBlock2* pblock;
		LinkTimeControl* ltControl;

		LinkConstTransform(const LinkConstTransform &ctrl);

		~LinkConstTransform();

		// Animatable methods
		void DeleteThis() {delete this;}		
		Class_ID ClassID() {return Class_ID(LINKCTRL_CLASSID);}
		SClass_ID SuperClassID() {return CTRL_MATRIX3_CLASS_ID;}
		void GetClassName(TSTR& s) {s = LINKCTRL_NAME;}
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev); 
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next); 
		int NumSubs()  {return 2;}
		Animatable* SubAnim(int i) {return i ? ltControl : tmControl;}
		TSTR SubAnimName(int i) {return i ? GetString(IDS_AG_LINKTIMES) : GetString(IDS_AG_LINKPARAMS);}				
		int IsKeyable() {return 1;}	// do I need this? Inherited from Link constraint

		void* GetInterface(ULONG id)
		{
			switch (id) {
				case I_LOCKED:
						return (ILockedTrackImp*) this;
				}
			return Control::GetInterface(id);
		}


		// Animatable's Schematic View methods
		SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
		TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);
		bool SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
		bool SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
		bool SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild);
		bool SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild);

		virtual void  SelectKeys (TrackHitTab &sel, DWORD flags)
		{
			ltControl->SelectKeys( sel, flags );
		}

		void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags)
		{
			ltControl->CopyKeysFromTime(src,dst,flags);
			tmControl->CopyKeysFromTime(src,dst,flags);
		}

		void SelectKeyByIndex(int i, BOOL sel)
		{
			pblock->SetValue(link_key_selection, 0, sel, i);
		};
		BOOL IsKeySelected(int i)
		{
			return pblock->GetTimeValue(link_key_selection, GetCOREInterface()->GetTime(), i);;
		};

		BOOL IsKeyAtTime(TimeValue t,DWORD flags)
		{
			return tmControl->IsKeyAtTime(t,flags) || ltControl->IsKeyAtTime(t,flags);
		}
		BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
		{
			return tmControl->GetNextKeyTime(t,flags,nt);
		}
		int GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags)
		{
			ltControl->GetKeyTimes(times,range,flags);
			return tmControl->GetKeyTimes(times,range,flags);
		}
		int GetKeySelState(BitArray &sel,Interval range,DWORD flags)
		{
			ltControl->GetKeySelState(sel,range,flags);
			return tmControl->GetKeySelState(sel,range,flags);
		}

		Control *GetPositionController() {return tmControl->GetPositionController();}
		Control *GetRotationController() {return tmControl->GetRotationController();}
		Control *GetScaleController() {return tmControl->GetScaleController();}
		BOOL SetPositionController(Control *c) {return tmControl->SetPositionController(c);}
		BOOL SetRotationController(Control *c) {return tmControl->SetRotationController(c);}
		BOOL SetScaleController(Control *c) {return tmControl->SetScaleController(c);}
//		void MapKeys(TimeMap *map,DWORD flags);
		int SubNumToRefNum(int subNum)
		{
			switch ( subNum )
			{
			case 0:
				return LINKCTRL_PBLOCK_REF;
			case 1:
				return LINKCTRL_LTCTL_REF;
			}
			return -1;
		}
		BOOL CanAssignController( int subAnim )
		{
			return ( subAnim == 1 ) ? FALSE : TRUE;
		}

		int RemapRefOnLoad(int iref);

		// Reference methods
		int NumRefs() {return LINKCTRL_CORE_REFs + nodes.Count();} 
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		RefTargetHandle Clone(RemapDir &remap);
		BOOL AssignController(Animatable *control,int subAnim);
		void RescaleWorldUnits(float f) {} // do I need this? Inherited from Link constraint

		// Control methods
		void Copy(Control *from);
		BOOL IsLeaf() {return FALSE;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);
		void CommitValue(TimeValue t) { }
		void RestoreValue(TimeValue t) { }
		BOOL InheritsParentTransform() {return FALSE;}
		BOOL CanInstanceController() {return FALSE;}
		BOOL IsReplaceable() {return !GetLocked();}  
//		void DeleteKeysFrom(INode *toDeleteNode, TimeValue toDeleteTime);

		// From ILinkCtrl
		int GetNumTargets();
		TimeValue GetLinkTime(int i) {
			return pblock->GetTimeValue(link_start_time, 0, i);
		}
		void SetLinkTime(int i, TimeValue linkTime) {SetTime(linkTime,i);}
		void LinkTimeChanged() {
			SortNodes(-1);
			RedrawListbox(GetCOREInterface()->GetTime());
		}
		void ActivateSort(bool bActive) { bSortActive = bActive;};
		
		static void NotifyPreDeleteNode(void* param, NotifyInfo*);

		
		// From where?
		int SetProperty(ULONG id, void *data);
		void *GetProperty(ULONG id);
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock


		// Local methods
		Matrix3 GetParentTM(TimeValue t,Interval *valid=NULL);
		Matrix3 CompTM(TimeValue t, int i);
		void AddNewLink(INode *node,TimeValue t);
		BOOL DeleteTarget(int i);
		void SetTime(TimeValue linkTime, int i);
		void SortNodes(int sel);

//		void SetupDialog(HWND hWnd);
//		void DestroyDialog();
//		void SetupList(int sel=-1);
//		void ListSelChanged();
//		void Invalidate();
		int Getlink_key_mode();
		int Getlink_key_nodes_mode();
		int Getlink_key_hierarchy_mode();
		BOOL Setlink_key_mode(int keymode);
		BOOL Setlink_key_nodes_mode(int keynodes_mode);
		BOOL Setlink_key_hierarchy_mode(int keyhier_mode);


		void ChangeKeys(int sell);
		int GetFrameNumber(int targetNumber);
		BOOL SetFrameNumber(int targetNumber, int frameNumber);
		BOOL AddTarget(INode *target, int frameNo = GetCOREInterface()->GetTime()/GetTicksPerFrame());
		int InternalAddWorldMethod(int frameNo = GetCOREInterface()->GetTime()/GetTicksPerFrame());
		INode* GetNode(int targetNumber);


//		Function Publishing method (Mixin Interface)
//		Added by Ambarish Goswami (7/20/2000)
//		Adapted from Adam Felt (5-16-00)

		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == LINK_CONSTRAINT_INTERFACE) 
				return (LinkConstTransform*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		}

	};

