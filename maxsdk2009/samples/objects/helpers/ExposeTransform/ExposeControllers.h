/**********************************************************************
 *<
	FILE: ExposeControllers.h
	DESCRIPTION:	Includes for Plugins

	CREATED BY: Michael Zyracki

	HISTORY:

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/
#ifndef __EXPOSE_CONTROLLER_H
#define __EXPOSE_CONTROLLER_H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"


#define FloatExposeController_CLASS_ID	Class_ID(0x72930021, 0x6951211)
#define Point3ExposeController_CLASS_ID	Class_ID(0x69f672c0, 0x27927afa)
#define EulerExposeController_CLASS_ID	Class_ID(0x19357028, 0x428c5294)

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

class ExposeTransform;
class BaseExposeControl :public Control,public IUnReplaceableControl
{
public:
	BaseExposeControl():exposeTransform(NULL),paramID(-1),evaluating(0),evaluatingTime(0),ivalid(NEVER){}
	int IsKeyable(){return 0;}
	BOOL IsReplaceable();
	BOOL CanApplyEaseMultCurves() {return FALSE;}
	BOOL IsAnimated() {return TRUE;}
	BOOL CanInstanceController() {return FALSE;} //in theory only for tranform but you never know
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message){return REF_SUCCEED;}
	BOOL IsLeaf() {return TRUE;}
	void CommitValue(TimeValue t) { }
	void RestoreValue(TimeValue t) { }
	void DeleteThis() {delete this;}
	BOOL CanCopyAnim(){return FALSE;}
	void* GetInterface(ULONG id) { if (id ==  I_UNREPLACEABLECTL) return (IUnReplaceableControl*)this; else return Control::GetInterface(id); }

	
	//error dialog when there's a dependency loop
	void PopupErrorMessage();

	//we never save this out.. we just set it up after load or creation.
	ExposeTransform *exposeTransform; 
	int paramID;

	//flag to avoid loops such as defect # 593959
	int evaluating;
	TimeValue evaluatingTime;

	//added to fix defect # 592326, where we break a bone chain by having GetNodeTM call a node that's in UpdateTM
	BOOL AreNodesOrParentsInTMUpdate(); 

	//Interval for updates
	Interval ivalid;

	virtual void Update(TimeValue t)=0;


};

class FloatExposeControl : public BaseExposeControl
{
public:
	FloatExposeControl();
	FloatExposeControl(ExposeTransform *e,int id);
	~FloatExposeControl();

	void Copy(Control *from);
	void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);	
	void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);

	//from IUnReplaceableControl
	Control * GetReplacementClone();

	//From Animatable
	Class_ID ClassID() {return FloatExposeController_CLASS_ID;}		
	SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	void GetClassName(TSTR& s) {s = GetString(IDS_FLOATEXPOSECONTROL_CLASS_NAME);}

	RefTargetHandle Clone(RemapDir& remap);

	void Update(TimeValue t);
	float curVal;
};

class FloatExposeControlClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading) { return new FloatExposeControl(); }
	const TCHAR *	ClassName() { return GetString(IDS_FLOATEXPOSECONTROL_CLASS_NAME); }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return FloatExposeController_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
	};


class Point3ExposeControl : public BaseExposeControl
{
public:
	Point3ExposeControl();
	Point3ExposeControl(ExposeTransform *e,int id);
	~Point3ExposeControl();

	void Copy(Control *from);
	void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);	
	void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);

	RefTargetHandle Clone(RemapDir& remap);
	//from IUnReplaceableControl
	Control * GetReplacementClone();


	//From Animatable
	Class_ID ClassID() {return Point3ExposeController_CLASS_ID;}		
	SClass_ID SuperClassID() { return CTRL_POINT3_CLASS_ID; }
	void GetClassName(TSTR& s) {s = GetString(IDS_POINT3EXPOSECONTROL_CLASS_NAME);}

	void Update(TimeValue t);
	Point3 curVal;

};

class Point3ExposeControlClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading) { return new Point3ExposeControl(); }
	const TCHAR *	ClassName() { return GetString(IDS_POINT3EXPOSECONTROL_CLASS_NAME); }
	SClass_ID		SuperClassID() { return CTRL_POINT3_CLASS_ID; }
	Class_ID		ClassID() { return Point3ExposeController_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
	};



class EulerExposeControl : public BaseExposeControl
{
public:
	EulerExposeControl();
	EulerExposeControl(ExposeTransform *e,int id);
	~EulerExposeControl();

	void Copy(Control *from);
	void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);	
	void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);

	RefTargetHandle Clone(RemapDir& remap);
	//from IUnReplaceableControl
	Control * GetReplacementClone();


	//From Animatable
	Class_ID ClassID() {return EulerExposeController_CLASS_ID;}		
	SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
	void GetClassName(TSTR& s) {s = GetString(IDS_EULEREXPOSECONTROL_CLASS_NAME);}


	void Update(TimeValue t);
	Quat curVal;
};

class EulerExposeControlClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading) { return new EulerExposeControl(); }
	const TCHAR *	ClassName() { return GetString(IDS_EULEREXPOSECONTROL_CLASS_NAME); }
	SClass_ID		SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
	Class_ID		ClassID() { return EulerExposeController_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
	};

#endif
