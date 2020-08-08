/**********************************************************************
 *<
	FILE: ExposeTransform.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY: Michael Zyracki

	HISTORY:

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef __ExposeTransform__H
#define __ExposeTransform__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#define ExposeTransform_CLASS_ID	Class_ID(0x848361cb, 0x913cc083)
extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define PBLOCK_EXPOSE	0
#define PBLOCK_INFO		1
#define PBLOCK_DISPLAY	2
#define PBLOCK_OUTPUT	3

class ExposeTransform : public HelperObject,public TimeChangeCallback {
	public:

		static IObjParam *ip;
		static ExposeTransform *editOb;

		//suspend recalc
		BOOL suspendRecalc;
		//point helper params used for drawing stuff
		// Snap suspension flag (TRUE during creation only)
		BOOL suspendSnap;
		// For use by display system
 		int extDispFlags;

		// validity.. just used now when recievning a change message and the UI is up
		BOOL valid;
		// Parameter block
		IParamBlock2	*pblock_expose;	//ref 0
		IParamBlock2	*pblock_info;//ref 1
		IParamBlock2	*pblock_display;	//ref 2
		IParamBlock2	*pblock_output;	//ref 3

		//custom edits
		ICustEdit *localEulerXEdit;
		ICustEdit *localEulerYEdit;
		ICustEdit *localEulerZEdit;
		ICustEdit *worldEulerXEdit;
		ICustEdit *worldEulerYEdit;
		ICustEdit *worldEulerZEdit;
		ICustEdit *localPositionXEdit;
		ICustEdit *localPositionYEdit;
		ICustEdit *localPositionZEdit;
		ICustEdit *worldPositionXEdit;
		ICustEdit *worldPositionYEdit;
		ICustEdit *worldPositionZEdit;
		ICustEdit *boundingBoxWidthEdit;
		ICustEdit *boundingBoxLengthEdit;
		ICustEdit *boundingBoxHeightEdit;
		ICustEdit *distanceEdit;
		ICustEdit *angleEdit;

		//custom edits
		ICustButton *localEulerXButton;
		ICustButton *localEulerYButton;
		ICustButton *localEulerZButton;
		ICustButton *worldEulerXButton;
		ICustButton *worldEulerYButton;
		ICustButton *worldEulerZButton;
		ICustButton *localPositionXButton;
		ICustButton *localPositionYButton;
		ICustButton *localPositionZButton;
		ICustButton *worldPositionXButton;
		ICustButton *worldPositionYButton;
		ICustButton *worldPositionZButton;
		ICustButton *boundingBoxWidthButton;
		ICustButton *boundingBoxLengthButton;
		ICustButton *boundingBoxHeightButton;
		ICustButton *distanceButton;
		ICustButton *angleButton;

		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//from TimeChangeCallback
		void TimeChanged(TimeValue t);

		// From BaseObject
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		void SetExtendedDisplay(int flags);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() {return GetString(IDS_EXPOSE_TRANSFORM_HELPER_NAME);}

		// From Object
		ObjectState Eval(TimeValue time);
		void InitNodeName(TSTR& s) { s = GetString(IDS_DB_EXPOSE_TRANSFORM); }
		ObjectHandle ApplyTransform(Matrix3& matrix) {return this;}
		int CanConvertToType(Class_ID obtype) {return FALSE;}
		Object* ConvertToType(TimeValue t, Class_ID obtype) {assert(0);return NULL;}		
		void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		int DoOwnSelectHilite()	{ return 1; }
		Interval ObjectValidity(TimeValue t);
		int UsesWireColor() {return TRUE;}

		//From Animatable
		Class_ID ClassID() {return ExposeTransform_CLASS_ID;}		
		SClass_ID SuperClassID() { return HELPER_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);

		int NumSubs() { return 4; }
		TSTR SubAnimName(int i) { if(i==PBLOCK_EXPOSE)return GetString(IDS_EXPOSE_PARAMS); else if(i==PBLOCK_INFO)return GetString(IDS_INFO_PARAMS);
		else if (i==PBLOCK_OUTPUT) return GetString(IDS_DISPLAY_EXPOSED);else return  GetString(IDS_DISPLAY_PARAMS);	}				
		Animatable* SubAnim(int i) {if(i==PBLOCK_EXPOSE)return pblock_expose; else
			if(i==PBLOCK_INFO) return pblock_info;else if(i==PBLOCK_OUTPUT) return pblock_output;else return pblock_display;}
		
		int NumRefs() { return 4; }
		RefTargetHandle GetReference(int i) {if(i==PBLOCK_EXPOSE) return pblock_expose;else if
			(i==PBLOCK_INFO) return pblock_info; else if (i==PBLOCK_OUTPUT) return pblock_output;else return pblock_display;}
		void SetReference(int i, RefTargetHandle rtarg) {if(i==PBLOCK_EXPOSE)pblock_expose = 
			(IParamBlock2*) rtarg; else if (i==PBLOCK_INFO)pblock_info = (IParamBlock2*)rtarg;
			else if(i==PBLOCK_OUTPUT) pblock_output = (IParamBlock2*) rtarg;
			else pblock_display=(IParamBlock2*)rtarg;}

		int	NumParamBlocks() { return 4; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) {if(i==PBLOCK_EXPOSE) return pblock_expose;
		else if(i==PBLOCK_INFO) return pblock_info;	else if(i==PBLOCK_OUTPUT) return pblock_output;else return pblock_display;} // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) {if(pblock_expose->ID()==id) return
			pblock_expose; else if (pblock_info->ID()==id) return pblock_info; else if (pblock_output->ID()==id) return
			pblock_output; else return pblock_display;}
		void DeleteThis() { delete this; }		
		
		//Constructor/Destructor
		ExposeTransform();
		~ExposeTransform();		

		// Local methods
		void CreateController(int where);
		//called by our local controllers;
		Point3 GetExposedPoint3Value(TimeValue t,int paramID,Control *control);
		float GetExposedFloatValue(TimeValue t, int paramID,Control *control);
		Quat GetExposedEulerValue(TimeValue t, int paramID,Control *control);

		void RecalcData(); //main function to recalc everything
		//individual recalc funcs
		Point3 CalculateLocalEuler(TimeValue t);
		Point3 CalculateWorldEuler(TimeValue t);
		Point3 CalculateLocalPosition(TimeValue t);
		Point3 CalculateWorldPosition(TimeValue t);
		Point3 CalculateWorldBoundingBoxSize(TimeValue t);
		float CalculateDistance(TimeValue t);
		float CalculateAngle(TimeValue t);
		INode *GetExposeNode();
		void SetExposeNode(INode *node);
		INode *GetReferenceNode();
		void SetReferenceNodeNULL();

		void InvalidateUI();
		int DrawAndHit(TimeValue t, INode *inode, ViewExp *vpt);
		void InitOutputUI(TimeValue t);
		void InitInfoUI();
		void InitRefTarg();
		void InitEulers();
		void InitTimeOffset();
		void InitOutput(TimeValue t);
		void WMInfoCommand(int id, int notify, HWND hWnd,HWND hCtrl);
		void WMOutputCommand(int id, int notify, HWND hWnd,HWND hCtrl);
		void DeleteCustomEdits();	
		void DeleteCustomButtons();	
		void SetZeroExposeValues();
		void SetExposeValues(TimeValue t);
		INode *GetMyNode();
		void SetUpClipBoard(TSTR &localName);
};



class ExposeTransformClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new ExposeTransform(); }
	const TCHAR *	ClassName() { return GetString(IDS_EXPOSETRANSFORM_CLASS_NAME); }
	SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
	Class_ID		ClassID() { return ExposeTransform_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("ExposeTransform"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	

};

#endif
