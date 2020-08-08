//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
/*===========================================================================*\
	FILE: sunlight.cpp

	DESCRIPTION: Sunlight system plugin.

	HISTORY: Created Oct.15 by John Hutchinson
			Derived from the ringarray

	Copyright (c) 1996, All Rights Reserved.
 \*==========================================================================*/
/*===========================================================================*\
 | Include Files:
\*===========================================================================*/

#pragma managed
#include "natLight.h"
#include "suntypes.h"
#include "sunclass.h"
#include "sunlight.h"
#include "WeatherFileDialog.h"
#include "IDaylightControlledLightSystem.h"

#pragma managed(push, off)
extern TCHAR* GetString(int);
extern HINSTANCE hInstance;

#pragma managed(pop)



const SClass_ID kValidSuperClass = LIGHT_CLASS_ID;
const Class_ID kStandardSunClass = Class_ID(DIR_LIGHT_CLASS_ID, 0);

#define OVERSHOOT TRUE
//#define SUN_RGB Point3(0.88235294f, 0.88235294f, 0.88235294f)  // 225
#define SUN_RGB Point3(1.00f, 0.95f, 0.90f)  // 225

static bool clampColor(Point3& color)
{
	if (color.x < 0.0f)
		color.x = 0.0f;
	else if (color.x > 1.0f)
		color.x = 1.0f;

	if (color.y < 0.0f)
		color.y = 0.0f;
	else if (color.y > 1.0f)
		color.y = 1.0f;

	if (color.z < 0.0f)
		color.z = 0.0f;
	else if (color.z > 1.0f)
		color.z = 1.0f;
	return true;
}

static void GetSceneBBox(INode* node, TimeValue t, Box3& box, bool hideFrozen)
{
	if (node == NULL)
		return;

	ObjectState os = node->EvalWorldState(t);
	if (os.obj != NULL) {
		switch (os.obj->SuperClassID()) {
		default:
			if (!(node->IsNodeHidden()
					|| (hideFrozen && node->IsFrozen())
					|| !node->Renderable()
					|| os.obj == NULL
					|| !os.obj->IsRenderable())) {
				Box3 objBox;
				Matrix3 m(node->GetObjTMAfterWSM(t));
				os.obj->GetDeformBBox(t, objBox, &m, false);
				box += objBox;
			}
			break;
		case LIGHT_CLASS_ID:
			break;
		}
	}

	for (int i = node->NumberOfChildren(); --i >= 0; ) {
		GetSceneBBox(node->GetChildNode(i), t, box, hideFrozen);
	}
}

void SetUpSun(SClass_ID sid, const Class_ID& systemClassID, Interface* ip,
	GenLight* obj, const Point3& sunColor)
{
	MarketDefaults* def = GetMarketDefaults();

	obj->Enable(1);
	obj->SetShadow(def->GetInt(sid, systemClassID,
		_T("sunCastShadows"), 1) != 0);
	ShadowType* s = dynamic_cast<ShadowType*>(def->CreateInstance(
		sid, systemClassID, _T("sunShadowGenerator"),
		SHADOW_TYPE_CLASS_ID, Class_ID(0, 0), MarketDefaults::CheckNULL));
	BOOL useGlobal = obj->GetUseGlobal();
	if (s != NULL) {
		obj->SetUseGlobal(0);
		obj->SetShadowGenerator(s);
	}
	obj->SetUseGlobal(def->GetInt(sid, systemClassID,
		_T("sunUseGlobalShadowSettings"), useGlobal) != 0);
	obj->SetOvershoot(def->GetInt(sid, systemClassID,
		_T("sunOvershoot"), OVERSHOOT) != 0);
	SuspendAnimate();
	obj->SetRGBColor(ip->GetTime(),
		def->GetRGBA(sid, systemClassID,
			_T("sunColor"), sunColor, clampColor));

	// Pretty ugly. The core code doesn't have a normal way to
	// determine whether frozen nodes are hidden and INode::IsNodeHidden
	// doesn't return true when the node is frozen and frozen
	// nodes are hidden. However, SetupRendParams does set the
	// RENDER_HIDE_FROZEN flag in RendParams::extraFlags if frozen
	// nodes are hidden. So we use this to help determine when nodes
	// are hidden.
	RendParams params;
	Box3 box;
	TimeValue t = ip->GetTime();

	GetCOREInterface()->SetupRendParams(params, NULL);
	GetSceneBBox(ip->GetRootNode(), t, box, (params.extraFlags & RENDER_HIDE_FROZEN) != 0);

	if (!box.IsEmpty()) {
		float len = ceilf(FLength(box.pmax - box.pmin) * 0.65f);

		if (len > obj->GetFallsize(t)) {
			obj->SetHotspot(t, len);
			obj->SetFallsize(t, len + 2);
		}
	}

	ResumeAnimate();
}

/*===========================================================================*\
 | NatLightAssembly Class Descriptor
\*===========================================================================*/
namespace {

bool checkStandin(Class_ID& id)
{
	return id.PartA() != STANDIN_CLASS_ID || id.PartB() != 0;
}

bool IsValidSun( ClassDesc& cd) 
{
	bool bValidSun = ((cd.ClassID() == kStandardSunClass) != FALSE);
	if (!bValidSun)
	{
		INaturalLightClass* pNC = GetNaturalLightClassInterface(&cd);
		if (pNC != NULL && (pNC->GetDesc()->flags & FP_STATIC_METHODS)) 
		{
			bValidSun = (pNC->IsSun() != FALSE);
		}
	}
	return bValidSun;
}

bool IsValidSky( ClassDesc& cd) 
{
	bool bValidSky = false;
	INaturalLightClass* pNC = GetNaturalLightClassInterface(&cd);
	if (pNC != NULL && (pNC->GetDesc()->flags & FP_STATIC_METHODS)) 
	{
		bValidSky = (pNC->IsSky() != FALSE);
	}
	return bValidSky;
}

} // namespace

class NatLightAssemblyClassDesc:public ClassDesc2 {
	public:
	NatLightAssemblyClassDesc() { ResetClassParams(true); }
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new NatLightAssembly(); }
	const TCHAR *	ClassName() { return GetString(IDS_NAT_LIGHT_NAME); }
	SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
	Class_ID		ClassID() 
		{ return NATLIGHT_HELPER_CLASS_ID; }

	const TCHAR* InternalName() { return _T("DaylightAssemblyHead"); }
	HINSTANCE HInstance() { return hInstance; }

	// The slave controllers don't appear in any of the drop down lists, 
	// so they just return a null string.
	const TCHAR* 	Category() { return _T("");  }

	void ResetClassParams(BOOL fileReset) {
		// IES Sun class ID - Defaults to directional light
		mSunID = GetMarketDefaults()->GetClassID(HELPER_CLASS_ID,
			NATLIGHT_HELPER_CLASS_ID, _T("sun"), kStandardSunClass,
			checkStandin);
		// IES Sky class ID - Defaults to Skylight
		mSkyID = GetMarketDefaults()->GetClassID(HELPER_CLASS_ID,
			NATLIGHT_HELPER_CLASS_ID, _T("sky"),
			Class_ID(0x7bf61478, 0x522e4705), checkStandin);
		mSunValid = true;
		mSkyValid = true;
	}

	Object* CreateSun(Interface* ip, const Class_ID* clsID = NULL) 
	{
		theHold.Suspend();
		Object* obj = NULL;
		if (NULL == clsID) 
		{
			obj = static_cast<Object*>(mSunValid ? ip->CreateInstance(kValidSuperClass, mSunID) : NULL);
		}
		else 
		{
			ClassDirectory& dir = ip->GetDllDir().ClassDir();
			ClassDesc* cd =  dir.FindClass(kValidSuperClass, *clsID);
			if (cd != NULL && IsValidSun(*cd))
			{
				obj = static_cast<Object*>(ip->CreateInstance(kValidSuperClass, *clsID));
			}
		}
		theHold.Resume();
		return obj;
	}

	Object* CreateSky(Interface* ip, const Class_ID* clsID = NULL) 
	{
		theHold.Suspend();
		Object* obj = NULL;
		if (NULL == clsID)
		{
			obj = static_cast<Object*>(mSkyValid ? ip->CreateInstance(kValidSuperClass, mSkyID) : NULL);
		}
		else 
		{
			ClassDirectory& dir = ip->GetDllDir().ClassDir();
			ClassDesc* cd =  dir.FindClass(kValidSuperClass, *clsID);
			if (cd != NULL && IsValidSky(*cd))
			{
				obj = static_cast<Object*>(ip->CreateInstance(kValidSuperClass, *clsID));
			}
		}
		theHold.Resume();
		return obj;
	}

	private:

	bool IsPublicClass(const Class_ID& id) {
		if (id == Class_ID(0, 0))
			return true;

		ClassEntry* pE = GetCOREInterface()->GetDllDir().ClassDir()
			.FindClassEntry(kValidSuperClass, id);
		return pE != NULL && pE->IsPublic() != 0;
	}

	Class_ID					mSunID;
	Class_ID					mSkyID;
	bool						mSunValid;
	bool						mSkyValid;
};


// A single instance of the class descriptor.
static NatLightAssemblyClassDesc natLightDesc;
// This returns a pointer to the instance.
ClassDesc* GetNatLightAssemblyDesc() { return &natLightDesc; }

/*===========================================================================*\
 | TraverseAssembly Class
\*===========================================================================*/

class TraverseAssembly {
public:
	enum ExcludeMembers {
		kNone = 0x00,
		kHead = 0x01,
		kOpen = 0x02,
		kClosed = 0x04,
		kOpenSubAssemblies = 0x08,
		kClosedSubAssemblies = 0x10,
		kOpenSubMembers = 0x20,
		kClosedSubMembers = 0x40
	};

	bool traverse(
		INode*	node,
		int		exclude = kNone
	);

protected:
	virtual bool proc(
		INode*		head,
		INode*		node,
		IAssembly*	asmb
	) = 0;

private:
	bool traverseChild(
		INode*		head,
		INode*		child,
		int			exclude,
		bool		subAssembly
	);
};

bool TraverseAssembly::traverse(
	INode*	node,
	int		exclude
)
{
	IAssembly* asmb;

	if (node == NULL || ((asmb = GetAssemblyInterface(node)) != NULL
			&& !asmb->IsAssemblyHead()))
		return false;

	if (!(exclude & kHead) && !proc(node, node, asmb))
		return false;

	int count = node->NumberOfChildren();
	for (int i = 0; i < count; ++i) {
		if (!traverseChild(node, node->GetChildNode(i), exclude, false))
			return false;
	}

	return true;
}

bool TraverseAssembly::traverseChild(
	INode*		head,
	INode*		child,
	int			exclude,
	bool		subAssembly
)
{
	IAssembly* asmb;
	if (child == NULL || ((asmb = GetAssemblyInterface(child)) != NULL
			&& !asmb->IsAssemblyMember()))
		return true;

	if (asmb != NULL && asmb->IsAssemblyHead()) {
		static int excludeSubMask[2] = { kClosedSubAssemblies, kOpenSubAssemblies };
		if (exclude & excludeSubMask[asmb->IsAssemblyHeadOpen()])
			return true;
		if (!proc(head, child, asmb))
			return false;
		subAssembly = true;
		head = child;
	}
	else {
		static int selectType[2][2] = {
			{ kClosed, kOpen },
			{ kClosedSubMembers, kOpenSubMembers },
		};

		if (!(exclude & selectType[subAssembly][asmb != NULL && asmb->IsAssemblyMemberOpen()])
				&& !proc(head, child, asmb))
			return false;
	}

	if (asmb != NULL) {
		int count = child->NumberOfChildren();
		for (int i = 0; i < count; ++i) {
			if (!traverseChild(head, child->GetChildNode(i), exclude, subAssembly))
				return false;
		}
	}

	return true;
}

/*===========================================================================*\
 | AppendAssembly Class:
\*===========================================================================*/

class AppendAssembly : public TraverseAssembly {
public:
	AppendAssembly(INodeTab& nodes) : mNodes(nodes) {}

	bool proc(
		INode*		head,
		INode*		node,
		IAssembly*	asmb
	);

private:
	INodeTab&	mNodes;
};

bool AppendAssembly::proc(
	INode*,
	INode*		node,
	IAssembly*
)
{
	mNodes.Append(1, &node);
	return true;
}

/*===========================================================================*\
 | Natural Light Assembly CreateMouseCallBack
\*===========================================================================*/

class NatLightAssembly::CreateCallback : public CreateMouseCallBack {
public:

	virtual int proc(
		ViewExp*	vpt,
		int			msg,
		int			point,
		int			flags,
		IPoint2		m,
		Matrix3&	mat
	);
};

int NatLightAssembly::CreateCallback::proc(
	ViewExp*	vpt,
	int			msg,
	int			point,
	int			flags,
	IPoint2		m,
	Matrix3&	mat
)
{
	// Process the mouse message sent...
	switch ( msg ) {
		case MOUSE_POINT:
			return CREATE_STOP;
	}

	return CREATE_CONTINUE;
}

/*===========================================================================*\
 | Natural Light Assembly Dialog Class:
\*===========================================================================*/

class NatLightAssembly::ParamDlgProc {
public:
	ParamDlgProc() : mpEditing(NULL),
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		mpMaster(NULL),
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		mpIp(NULL), mpEditSun(NULL), mpEditSky(NULL) {}

	void Begin(
		NatLightAssembly*	editing,
		IObjParam*			ip,
		ULONG				flags,
		Animatable*			prev
	);
	void End(
		NatLightAssembly*	editing,
		IObjParam*			ip,
		ULONG				flags,
		Animatable*			next
	);

	void Begin(
		NatLightAssembly*	natLight,
		Object*				obj
	);
	void End(
		NatLightAssembly*	natLight,
		Object*				obj
	);

	virtual void DeleteThis();
	virtual void Update(NatLightAssembly* natLight);

protected:
	bool isModifyPanel() const {
		return (mFlags & (BEGIN_EDIT_CREATE | BEGIN_EDIT_MOTION | BEGIN_EDIT_HIERARCHY
			| BEGIN_EDIT_IK | BEGIN_EDIT_LINKINFO)) == 0;
	}

#ifndef NO_DAYLIGHT_SELECTOR
   static INT_PTR CALLBACK DialogProc(
		HWND hwnd,
		UINT msg,
		WPARAM w,
		LPARAM l
	);
   virtual INT_PTR DlgProc(
		HWND		hWnd,
		UINT		msg,
		WPARAM		wParam,
		LPARAM		lParam
	);
#endif	// NO_DAYLIGHT_SELECTOR

private:
#ifndef NO_DAYLIGHT_SELECTOR
   INT_PTR InitDialog(HWND focus);
	void ReplaceNode(
		ParamID	param,
		HWND	combo
	);
#endif	// NO_DAYLIGHT_SELECTOR

	NatLightAssembly*		mpEditing;
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	SunMaster*				mpMaster;
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	IObjParam*				mpIp;
	Object*					mpEditSun;
	Object*					mpEditSky;
#ifndef NO_DAYLIGHT_SELECTOR
	HWND					mDlgWnd;
#endif	// NO_DAYLIGHT_SELECTOR
	ULONG					mFlags;
};


/*===========================================================================*\
 | Natural Light Assembly Param Accessor Class:
\*===========================================================================*/

class NatLightAssembly::Accessor : public PBAccessor {
protected:
	virtual void Get(
		PB2Value&		v,
		ReferenceMaker*	owner,
		ParamID			id,
		int				tabIndex,
		TimeValue		t,
		Interval&		valid
	);

	virtual void Set(
		PB2Value&		v,
		ReferenceMaker*	owner,
		ParamID			id,
		int				tabIndex,
		TimeValue		t
	);
};

/*===========================================================================*\
 | Natural Light Assembly Find Assembly Head Class
\*===========================================================================*/

class NatLightAssembly::FindAssemblyHead : protected DependentEnumProc
{
public:
	FindAssemblyHead(
		NatLightAssembly*	natLight
	) : mpNatLight(natLight) {}

	void FindHeadNodes() { mpNatLight->DoEnumDependents(this); }

protected:
	virtual int proc(ReferenceMaker *rmaker);
	virtual int proc(INode* head) = 0;

private:
	NatLightAssembly*	mpNatLight;
};

/*===========================================================================*\
 | Natural Light Assembly Are there any Sun and Sky Class
\*===========================================================================*/

class NatLightAssembly::AnySunAndSkyNodes : protected FindAssemblyHead,
											protected TraverseAssembly
{
public:
	AnySunAndSkyNodes(
		NatLightAssembly*	natLight,
		Object*				sun,
		Object*				sky
	) : FindAssemblyHead(natLight),
		mpSun(sun),
		mpSky(sky),
		mAnySuns(false),
		mAnySkys(false) {}

	void CheckNodes() { FindHeadNodes(); }
	bool AnySuns() const { return mAnySuns; }
	bool AnySkys() const { return mAnySkys; }

protected:
	virtual bool proc(
		INode*		head,
		INode*		node,
		IAssembly*	asmb
	);
	virtual int proc(INode* head);

private:
	Object*				mpSun;
	Object*				mpSky;
	bool				mAnySuns;
	bool				mAnySkys;
};

/*===========================================================================*\
 | Natural Light Assembly Replace Sun and Sky Class
\*===========================================================================*/

class NatLightAssembly::ReplaceSunAndSkyNodes : protected FindAssemblyHead,
												protected TraverseAssembly
{
public:
	ReplaceSunAndSkyNodes(
		NatLightAssembly*	natLight,
		Object*				old,
		bool				storeDirect
	) : FindAssemblyHead(natLight),
		mpOld(old),
		_storeDirect(storeDirect) {}

	void ReplaceNodes(
		TimeValue			t,
		IAssemblyMgr*		mgr,
		Interface*			ip,
		Object*				newObj,
		const TCHAR*		name
	);

protected:
	virtual bool proc(
		INode*		head,
		INode*		node,
		IAssembly*	asmb
	);
	virtual int proc(INode* head);

private:
	struct ObjEntry {
		ObjEntry(INode* head, INode* node) : mpHead(head), mpNode(node) {}
		INode*		mpHead;
		INode*		mpNode;
	};

	Object*				mpOld;
	Tab<ObjEntry>		mNodes;
	bool				_storeDirect;
};

/*===========================================================================*\
 | Natural Light Assembly Find Manual Setting Class
\*===========================================================================*/

class NatLightAssembly::FindManual : protected FindAssemblyHead
{
public:
	FindManual(
		NatLightAssembly*	natLight,
		TimeValue			t
	) : FindAssemblyHead(natLight),
		mT(t),
		mManualCount(0),
		mDateTimeCount(0),mWeatherCount(0) {}

	int FindManualSetting();

protected:
	virtual int proc(INode* head);

private:
	TimeValue	mT;
	ULONG		mManualCount;
	ULONG		mDateTimeCount;
	ULONG		mWeatherCount;
};

/*===========================================================================*\
 | Natural Light Assembly Set Manual Setting Class
\*===========================================================================*/

class NatLightAssembly::SetManual : protected FindAssemblyHead
{
public:
	SetManual(
		NatLightAssembly*	natLight,
		TimeValue			t,
		int				manual
	) : FindAssemblyHead(natLight),
		mT(t),
		mManual(manual) {}

	void SetManualSetting();

protected:
	virtual int proc(INode* head);

private:
	TimeValue	mT;
	int		mManual;
};

/*===========================================================================*\
 | Natural Light Assembly Open and Close Assembly Class
\*===========================================================================*/

class NatLightAssembly::OpenAssembly : protected FindAssemblyHead
{
public:
	OpenAssembly(
		NatLightAssembly*	natLight
	) : FindAssemblyHead(natLight) {}

	void Open();

protected:
	virtual int proc(INode* head);

private:
	INodeTab		mTab;
};

class NatLightAssembly::CloseAssembly : protected FindAssemblyHead
{
public:
	CloseAssembly(
		NatLightAssembly*	natLight
	) : FindAssemblyHead(natLight) {}

	void Close();

protected:
	virtual int proc(INode* head);

private:
	INodeTab		mSel, mNotSel;
};

/*===========================================================================*\
 | Natural Light Assembly Sun Mesh Class
\*===========================================================================*/

class NatLightAssembly::SunMesh : public Mesh
{
public:
	SunMesh() : mBuilt(false) {}

	void BuildMesh() {
		if (!mBuilt) {
			CreateMesh(8.0f, 11, 3.0f);
			mBuilt = true;
		}
	}

private:
	void CreateMesh(
		float	radius,
		int		nbRays,
		float	heightFactor
	);

	bool	mBuilt;
};

NatLightAssembly::Accessor NatLightAssembly::sAccessor;
NatLightAssembly::ParamDlgProc NatLightAssembly::sEditParams;
NatLightAssembly::CreateCallback	NatLightAssembly::sCreateCallback;
NatLightAssembly::SunMesh NatLightAssembly::sSunMesh;

ParamBlockDesc2 NatLightAssembly::sMainParams(
	MAIN_PBLOCK, _T("NaturalLightParameters"), IDS_NATLIGHT_PARAMS,
		&natLightDesc, P_AUTO_CONSTRUCT,
	// Auto-construct params
	PBLOCK_REF,

	// Sun
	SUN_PARAM, _T("sun"), TYPE_REFTARG, 0, IDS_LIGHT_NAME,
		p_accessor, &sAccessor,
		end,

	// Sky
	SKY_PARAM, _T("sky"), TYPE_REFTARG, 0, IDS_SKY_NAME,
		p_accessor, &sAccessor,
		end,

	// Manual position
	MANUAL_PARAM, _T("manual"), TYPE_INT, P_TRANSIENT, IDS_MANUAL,
		p_default, TRUE,
		p_accessor, &sAccessor,
		end,

	WEATHER_FILE_PARAM, _T("weatherFileName"),TYPE_FILENAME, P_READ_ONLY, IDS_WEATHER_FILE_NAME,
		p_accessor, &sAccessor,
		end,

	end
);

/*===========================================================================*\
 | Natural Light Assembly Class:
\*===========================================================================*/

NatLightAssembly::NatLightAssembly()
	: mpParams(NULL),
	  mpSunObj(NULL),
	  mpSkyObj(NULL),
	  mpMult(NULL),
	  mpIntense(NULL),
	  mpSkyCond(NULL),
	  mExtDispFlags(0),
	  mWeatherFileMissing(false),
	  mErrorLoadingWeatherFile(false)
{
	natLightDesc.MakeAutoParamBlocks(this);
	assert(mpParams != NULL);
	//added so we can give MR Sky it's visualiztion values.
	RegisterNotification(NatLightAssembly::RenderFrame,this, NOTIFY_RENDER_PREEVAL);
}

NatLightAssembly::~NatLightAssembly()
{
	UnRegisterNotification(NatLightAssembly::RenderFrame,this, NOTIFY_RENDER_PREEVAL);
	DeleteAllRefs();
}

BaseInterface* NatLightAssembly::GetInterface(Interface_ID id)
{
	if(id == DAYLIGHT_SYSTEM_INTERFACE)
		return this;
	else if(id == IID_DAYLIGHT_SYSTEM2)
		return static_cast<IDaylightSystem2*>(this);
	else if(id == IID_DAYLIGHT_SYSTEM3)
		return static_cast<IDaylightSystem3*>(this);
	else
		return Animatable::GetInterface(id);
}

INode* NatLightAssembly::CreateAssembly(
	NatLightAssembly*&	natObj,
	IObjCreate*			createInterface,
	const Class_ID* sunClassID, 
	const Class_ID* skyClassID 
)
{
	theHold.Suspend();
	natObj = new NatLightAssembly();
	theHold.Resume();
	INode* natNode = createInterface->CreateObjectNode(natObj);
	assert(natNode != NULL);

	IAssembly* asmb = GetAssemblyInterface(natNode);
	if (asmb != NULL) {
		asmb->SetAssemblyHead(true);
	}

//	TSTR natName = GetString(IDS_DAY_CLASS);
//	createInterface->MakeNameUnique(natName);
//	natNode->SetName(natName);

	DbgAssert(natObj->ClassID() == NATLIGHT_HELPER_CLASS_ID);

	Object* sunObj = natLightDesc.CreateSun(createInterface, sunClassID);

	if (sunObj != NULL && sunObj->SuperClassID() == LIGHT_CLASS_ID) {
		SetUpSun(HELPER_CLASS_ID, NATLIGHT_HELPER_CLASS_ID, createInterface,
			static_cast<GenLight*>(sunObj), SUN_RGB);
	}

	natObj->mpParams->SetValue(SUN_PARAM, 0, sunObj);
	assert(natObj->mpSunObj == sunObj);

	Object* skyObj = natLightDesc.CreateSky(createInterface, skyClassID);

	natObj->mpParams->SetValue(SKY_PARAM, 0, skyObj);
	assert(natObj->mpSkyObj == skyObj);

	return natNode;
}

void NatLightAssembly::AppendAssemblyNodes(
	INode*			theAssembly,
	INodeTab&		nodes,
	SysNodeContext
)
{
	AppendAssembly(nodes).traverse(theAssembly, TraverseAssembly::kHead);
}

static inline bool IsValidSunObject(ReferenceTarget* obj)
{
	INaturalLightClass* nl;
	return obj != NULL && obj->SuperClassID() == kValidSuperClass
		&& (obj->ClassID() == kStandardSunClass
			|| ((nl = GetNaturalLightClass(obj)) != NULL
				&& nl->IsSun()));
}

static inline bool IsValidSkyObject(ReferenceTarget* obj)
{
	INaturalLightClass* nl;
	return obj != NULL && obj->SuperClassID() == kValidSuperClass
		&& (nl = GetNaturalLightClass(obj)) != NULL
		&& nl->IsSky();
}

static inline bool IsValidSunOrSkyObject(
	int					ref,
	ReferenceTarget*	obj
)
{
	// Make sure the objects are valid. Suns can be standard
	// or export the ISunLightInterface, or be None.
	// Skys can be None or export the ISkyLightInterface
	if (obj != NULL) {
		if (ref == NatLightAssembly::SUN_REF) {
			if (!IsValidSunObject(obj))
				return false;
		}
		else if (ref == NatLightAssembly::SKY_REF) {
			if (!IsValidSkyObject(obj))
				return false;
		}
		else
			return false;
	}

	return true;
}

static void SetController(Object* obj, const TCHAR* name, Control* slave)
{
	if (obj != NULL && obj->SuperClassID() == LIGHT_CLASS_ID) {
      init_thread_locals();
      push_alloc_frame();
		two_value_locals(prop, val);			// Keep two local variables
      save_current_frames();
      trace_back_active = FALSE;

		try {
			// Get the name and value to set
			vl.prop = Name::intern(const_cast<TCHAR*>(name));
			vl.val = slave == NULL ? NULL : MAXControl::intern(slave, defaultDim);

			// Set the value.
			MAXWrapper::set_max_prop_controller(obj, vl.prop,
				static_cast<MAXControl*>(vl.val));
      } catch ( ... ) {
         clear_error_source_data();
         restore_current_frames();
         MAXScript_signals = 0;
         if (progress_bar_up)
            MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		}
		pop_value_locals();
      pop_alloc_frame();
	}
}
//called when we change any of our params (sun,sky or daylight control type
void NatLightAssembly::SetDaylightControlledIfNeeded(ReferenceTarget *obj, bool setControlled)
{
	if(obj&&obj->GetInterface(DAYLIGHT_CONTROLLED_LIGHT_SYSTEM))
	{
		IDaylightControlledLightSystem *dls = dynamic_cast<IDaylightControlledLightSystem*>(obj->GetInterface(DAYLIGHT_CONTROLLED_LIGHT_SYSTEM));
		if(dls)
		{
			if(setControlled==true)
				dls->ControlledByDaylightSystem( static_cast<IDaylightSystem2*>(this),setControlled);
			else
				dls->ControlledByDaylightSystem( NULL,setControlled);
		}
	}
}

void NatLightAssembly::SetSunAndSky(
	TimeValue			t,
	int					ref,
	ParamID				param,
	const TCHAR*		name,
	ReferenceTarget*	obj
)
{
	Object* old = static_cast<Object*>(GetReference(ref));
	if (obj != old) {
		IAssemblyMgr* mgr = GetAssemblyMgr();
		if (IsValidSunOrSkyObject(ref, obj)) {  //valid check
			//set the IDaylightControlledLightSystem based upon our weather file state.. 
			if(GetDaylightControlType()== IDaylightSystem2::eWeatherFile)
				SetDaylightControlledIfNeeded(obj,true);
			else
				SetDaylightControlledIfNeeded(obj,false);
			SetDaylightControlledIfNeeded(GetReference(ref),false);

			// If restoring or redoing, then everything will happen automatically
			if (!theHold.RestoreOrRedoing()) {
				// Param block 2 is calling us with the hold suspended
				// which is not good.
				ULONG resume = 0;
				while (theHold.IsSuspended()) {
					theHold.Resume();
					++resume;
				}

				Interface* ip = GetCOREInterface();
				bool wereInvalid = mpSunObj == NULL && mpSkyObj == NULL;

				ip->DisableSceneRedraw();
//				sEditParams.End(this, old);
				ReplaceReference(ref, obj);

				bool storeDirect = ref != SUN_REF && obj != NULL
					&& GetMarketDefaults()->GetInt(LIGHT_CLASS_ID, obj->ClassID(),
						_T("storeDirectIllumination"), 0) != 0;
				ReplaceSunAndSkyNodes replace(this, old, storeDirect);
				replace.ReplaceNodes(t, mgr, ip, static_cast<Object*>(obj), name);

				if (obj != NULL) {
					if (ref == SUN_REF) {
						ISunLight* sun = GetSunLightInterface(mpSunObj);
						if (sun == NULL || sun->IsIntensityInMAXUnits()) {
							if (mpMult != NULL)
								SetController(mpSunObj, _T("multiplier"), mpMult);
						} else {
							if (mpIntense != NULL)
								SetController(mpSunObj, _T("multiplier"), mpIntense);
						}
					} else {
						if (mpSkyCond != NULL)
							SetController(mpSkyObj, _T("sky_cover"), mpSkyCond);
					}
				}

//				sEditParams.Begin(this, static_cast<Object*>(obj));
				sEditParams.Update(this);
				bool areInvalid = mpSunObj == NULL && mpSkyObj == NULL;
				if (wereInvalid != areInvalid) {
					if (areInvalid) {
						OpenAssembly open(this);
						open.Open();
					} else {
						CloseAssembly close(this);
						close.Close();
					}
					NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
				}

				ip->EnableSceneRedraw();
				ip->RedrawViews(ip->GetTime());

				while (resume > 0) {
					theHold.Suspend();
					--resume;
				}
			}
		}
		else {
			mpParams->SetValue(param, 0, old);
		}
	}
}

LightObject* NatLightAssembly::GetSun() const
{ 
	return static_cast<LightObject*>(mpSunObj); 
}


LightObject* NatLightAssembly::GetSky() const
{ 
	return static_cast<LightObject*>(mpSkyObj); 
}

void NatLightAssembly::SetMultController(Control* c)
{
	ReplaceReference(MULT_REF, c);
	if (mpSunObj != NULL) {
		ISunLight* sun = GetSunLightInterface(mpSunObj);
		if (sun == NULL || sun->IsIntensityInMAXUnits())
			SetController(mpSunObj, _T("multiplier"), c);
	}
}

void NatLightAssembly::SetIntenseController(Control* c)
{
	ReplaceReference(INTENSE_REF, c);
	if (mpSunObj != NULL) {
		ISunLight* sun = GetSunLightInterface(mpSunObj);
		if (sun != NULL && !sun->IsIntensityInMAXUnits())
			SetController(mpSunObj, _T("multiplier"), c);
	}
}

void NatLightAssembly::SetSkyCondController(Control* c)
{
	ReplaceReference(SKY_COND_REF, c);
	if (mpSkyObj != NULL)
		SetController(mpSkyObj, _T("sky_cover"), c);
}

SunMaster* NatLightAssembly::FindMaster() const
{
	static Control* NatLightAssembly::*controllers[] = {
		&NatLightAssembly::mpMult,
		&NatLightAssembly::mpIntense,
		&NatLightAssembly::mpSkyCond
	};
	int i;
	ReferenceTarget* master;
	
	for (i = 0; i < sizeof(controllers) / sizeof(controllers[0]); ++i) {
		Control* c = this->*controllers[i];
		if (c != NULL
				&& c->IsSubClassOf(Class_ID(DAYLIGHT_SLAVE_CONTROL_CID1,
					DAYLIGHT_SLAVE_CONTROL_CID2))
				&& (master = c->GetReference(0)) != NULL
				&& master->IsSubClassOf(DAYLIGHT_CLASS_ID))
			return static_cast<SunMaster*>(master);
	}
	return NULL;
}

void NatLightAssembly::GetDeformBBox(
	TimeValue	t,
	Box3&		box,
	Matrix3*	tm,
	BOOL		useSel
)
{
	sSunMesh.BuildMesh();
	box = sSunMesh.getBoundingBox(tm);
}

void NatLightAssembly::InitNodeName(TSTR& s)
{
#ifdef LOCALIZED_APL		// Object name (use English) naming conflicts in APL (only?) -- LOCALIZATION_TODO
	s = _T("Daylight");
#else
	s = GetString(IDS_DAY_CLASS_NODE_NAME);
#endif	// LOCALIZED_APL
}

// From Object

ObjectState NatLightAssembly::Eval(TimeValue t)
{
	return ObjectState(this);
}

// From BaseObject

static void RemoveScaling(Matrix3 &tm) {
	AffineParts ap;
	decomp_affine(tm, &ap);
	tm.IdentityMatrix();
	tm.SetRotate(ap.q);
	tm.SetTrans(ap.t);
}

static void GetMat(
	TimeValue	t,
	INode*		inode,
	ViewExp*	vpt,
	Matrix3&	tm
) 
{
	tm = inode->GetObjectTM(t);
//	tm.NoScale();
	RemoveScaling(tm);
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
	tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
}

void NatLightAssembly::SetExtendedDisplay(int flags)
{
	mExtDispFlags = flags;
}

int NatLightAssembly::Display(
	TimeValue	t,
	INode*		inode,
	ViewExp*	vpt,
	int			flags
)
{
	Matrix3 m;
	GraphicsWindow *gw = vpt->getGW();
	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL|(gw->getRndMode() & GW_Z_BUFFER));
	if (inode->Selected())
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen() && !inode->Dependent())	{
		Color color(inode->GetWireColor());
		gw->setColor( LINE_COLOR, color );
	}
	sSunMesh.BuildMesh();
	sSunMesh.render( gw, gw->getMaterial(),
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL);	
	gw->setRndLimits(rlim);
	return 0 ;
}

int NatLightAssembly::HitTest(
	TimeValue t,
	INode*		inode,
	int			type,
	int			crossing,
	int			flags,
	IPoint2*	p,
	ViewExp*	vpt
)
{
	HitRegion hitRegion;
	DWORD savedLimits;
	int res;
	Matrix3 m;
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = gw->getMaterial();

	MakeHitRegion(hitRegion,type,crossing,4,p);	
	gw->setRndLimits( ((savedLimits = gw->getRndLimits()) | GW_PICK) & ~(GW_ILLUM|GW_BACKCULL));
	GetMat(t,inode,vpt,m);
	gw->setTransform(m);
	// if we get a hit on the mesh, we're done
	sSunMesh.BuildMesh();
	res = sSunMesh.select( gw, mtl, &hitRegion, flags & HIT_ABORTONHIT);
	gw->setRndLimits(savedLimits);
	return res;
}

void NatLightAssembly::Snap(
	TimeValue	t,
	INode*		inode,
	SnapInfo*	snap,
	IPoint2*	p,
	ViewExp*	vpt
)
{
	// Make sure the vertex priority is active and at least as important as the best snap so far
	if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) {
		Matrix3 tm = inode->GetObjectTM(t);	
		GraphicsWindow *gw = vpt->getGW();	
   	
		gw->setTransform(tm);

		Point2 fp = Point2((float)p->x, (float)p->y);
		IPoint3 screen3;
		Point2 screen2;
		Point3 vert(0.0f,0.0f,0.0f);

		gw->wTransPoint(&vert,&screen3);

		screen2.x = (float)screen3.x;
		screen2.y = (float)screen3.y;

		// Are we within the snap radius?
		int len = (int)Length(screen2 - fp);
		if(len <= snap->strength) {
			// Is this priority better than the best so far?
			if(snap->vertPriority < snap->priority) {
				snap->priority = snap->vertPriority;
				snap->bestWorld = vert * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
			}
			// Closer than the best of this priority?
			else if(len < snap->bestDist) {
				snap->priority = snap->vertPriority;
				snap->bestWorld = vert * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
			}
		}
	}
}

void NatLightAssembly::GetWorldBoundBox(
	TimeValue	t,
	INode*		inode,
	ViewExp*	vpt,
	Box3&		box
)
{
	int nv;
	Matrix3 tm;
	GetMat(t, inode,vpt,tm);
	Point3 loc = tm.GetTrans();
	sSunMesh.BuildMesh();
	nv = sSunMesh.getNumVerts();
	box.Init();
	if(!(mExtDispFlags & EXT_DISP_ZOOM_EXT)) 
		box.IncludePoints(sSunMesh.verts,nv,&tm);
	else
		box += loc;
}

void NatLightAssembly::GetLocalBoundBox(
	TimeValue	t,
	INode*		inode,
	ViewExp*	vpt,
	Box3&		box
)
{
	Point3 loc = inode->GetObjectTM(t).GetTrans();
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(loc) / 360.0f;
	sSunMesh.BuildMesh();
	box = sSunMesh.getBoundingBox();
	box.Scale(scaleFactor);
}

CreateMouseCallBack* NatLightAssembly::GetCreateMouseCallBack()
{
	return &sCreateCallback;
}

TCHAR* NatLightAssembly::GetObjectName()
{
	return GetString(IDS_NAT_LIGHT_NAME);
}


BOOL NatLightAssembly::HasUVW()
{
	return FALSE;
}

// From ReferenceTarget

RefTargetHandle NatLightAssembly::Clone(RemapDir &remap)
{
    NatLightAssembly* newl = new NatLightAssembly();
	if (mpSunObj != NULL)
      newl->ReplaceReference(SUN_REF,remap.CloneRef(mpSunObj));
	if (mpSkyObj != NULL)
      newl->ReplaceReference(SKY_REF,remap.CloneRef(mpSkyObj));
   BaseClone(this, newl, remap);
   //set up the sun/sky if they are controlled by the daylight system
   newl->SunMasterManualChanged(newl->GetDaylightControlType());
	return(newl);
}

// From ReferenceMaker

RefResult NatLightAssembly::NotifyRefChanged(
	Interval		changeInt,
	RefTargetHandle	hTarget,
	PartID&			partID,
	RefMessage		message
)
{
	switch (message) {
		case REFMSG_CHANGE: {
			if (hTarget == mpSunObj || hTarget == mpSkyObj) {
				sEditParams.Update(this);
			}
		} break;
	}
	return REF_SUCCEED;
}

int NatLightAssembly::NumRefs()
{
	return NUM_REFS;
}

RefTargetHandle NatLightAssembly::GetReference(int i)
{
	switch (i) {
	case PBLOCK_REF:
		return mpParams;
	case SUN_REF:
		return mpSunObj;
	case SKY_REF:
		return mpSkyObj;
	case MULT_REF:
		return mpMult;
	case INTENSE_REF:
		return mpIntense;
	case SKY_COND_REF:
		return mpSkyCond;
	}
	return NULL;
}

void NatLightAssembly::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) {
	case PBLOCK_REF:
		mpParams = static_cast<IParamBlock2*>(rtarg);
		break;
	case SUN_REF:
		if (mpSunObj != rtarg) {
			sEditParams.End(this, mpSunObj);
			mpSunObj = static_cast<LightObject*>(rtarg);
			sEditParams.Begin(this, mpSunObj);
			sEditParams.Update(this);
		}
		break;
	case SKY_REF:
		if (mpSkyObj != rtarg) {
			sEditParams.End(this, mpSkyObj);
			mpSkyObj = static_cast<LightObject*>(rtarg);
			sEditParams.Begin(this, mpSkyObj);
			sEditParams.Update(this);
		}
		break;
	case MULT_REF:
		mpMult = static_cast<Control*>(rtarg);
		break;
	case INTENSE_REF:
		mpIntense = static_cast<Control*>(rtarg);
		break;
	case SKY_COND_REF:
		mpSkyCond = static_cast<Control*>(rtarg);
		break;
	}
}


class ErrorPLCB : public PostLoadCallback
{
	public:
		NatLightAssembly *mNLA;
		ErrorPLCB(NatLightAssembly *o) {mNLA=o;}
		void proc(ILoad *iload);
};
void ErrorPLCB::proc(ILoad *iload)
{
	Interval iv;
	TSTR fname = mNLA->GetWeatherFileName();
	if (fname.isNull()==FALSE&&fname.Length()>0)
	{
		MaxSDK::Util::Path path(fname);
		IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();
		if (!pathMgr->SearchForExternalFiles(path)) 
		{
			mNLA->WeatherFileMissing();
		}
	}

	mNLA->SunMasterManualChanged(mNLA->GetDaylightControlType());//still need to set everything eup

	delete this;
}


IOResult NatLightAssembly::Load(ILoad *iload)
{
	iload->RegisterPostLoadCallback(new ErrorPLCB(this));
	return mWeatherData.Load(iload);

}

IOResult NatLightAssembly::Save(ISave *isave)
{
	return mWeatherData.Save(isave);
	return IO_OK;
}



// From Animatable

void NatLightAssembly::GetClassName(TSTR& s)
{
	s = GetString(IDS_NAT_LIGHT_NAME);
}

Class_ID NatLightAssembly::ClassID()
{
	return NATLIGHT_HELPER_CLASS_ID;
}

void NatLightAssembly::DeleteThis()
{
	delete this;
}

void NatLightAssembly::BeginEditParams(
	IObjParam*	ip,
	ULONG		flags,
	Animatable*	prev
)
{
	sEditParams.Begin(this, ip, flags, prev);
}

void NatLightAssembly::EndEditParams(
	IObjParam*	ip,
	ULONG		flags,
	Animatable*	next
)
{
	sEditParams.End(this, ip, flags, next);
}


int NatLightAssembly::NumSubs()
{
	return NUM_SUBS;
}

Animatable* NatLightAssembly::SubAnim(int i)
{
	return NULL;
}

TSTR NatLightAssembly::SubAnimName(int i)
{
	return TSTR();
}

BOOL NatLightAssembly::IsAnimated()
{
	return FALSE;
}

int NatLightAssembly::NumParamBlocks()
{
	return NUM_PBLOCK;
}

IParamBlock2* NatLightAssembly::GetParamBlock(int i)
{
	switch (i) {
	case 0:
		return mpParams;
	}

	return NULL;
}

IParamBlock2* NatLightAssembly::GetParamBlockByID(short id)
{
	switch (id) {
	case MAIN_PBLOCK:
		return mpParams;
	}

	return NULL;
}

int NatLightAssembly::SetProperty(
	ULONG	id,
	void*	data
)
{
	return 0;
}

void* NatLightAssembly::GetProperty(ULONG id)
{
	return NULL;
}


/*===========================================================================*\
 | Natural Light Assembly Dialog Class:
\*===========================================================================*/

void NatLightAssembly::ParamDlgProc::Begin(
	NatLightAssembly*	editing,
	IObjParam*			ip,
	ULONG				flags,
	Animatable*			prev
)
{
	// If asked to edit an object we are already editing
	// then do nothing.
	if (mpEditing == editing)
		return;

	if (mpEditing != NULL) {
		End(mpEditing, mpEditing->mpSunObj);
		End(mpEditing, mpEditing->mpSkyObj);
	}

	mpEditing = editing;
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
#ifndef NO_CREATE_TASK
	mpMaster = SunMaster::IsCreateModeActive() ? NULL : mpEditing->FindMaster();
#else
	mpMaster = mpEditing->FindMaster();
#endif
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	mpIp = ip;
	mFlags = flags;

#ifndef NO_DAYLIGHT_SELECTOR
	if (mDlgWnd == NULL) {
		ip->AddRollupPage(
			hInstance,
			MAKEINTRESOURCE(IDD_NATLIGHTPARAM),
			DialogProc,
			GetString(IDS_NATLIGHT_PARAM_NAME),
			reinterpret_cast<LPARAM>(this),
			0,
			(ROLLUP_CAT_SYSTEM + ROLLUP_CAT_STANDARD) / 2);
		assert(mDlgWnd != NULL);
#endif	// NO_DAYLIGHT_SELECTOR

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		if (mpMaster != NULL)
			mpMaster->BeginEditParams(ip, flags, prev);
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)

#ifndef NO_DAYLIGHT_SELECTOR
	} else {
      DLSetWindowLongPtr(mDlgWnd, this);
		Update(mpEditing);
	}
#endif	// NO_DAYLIGHT_SELECTOR

	Begin(mpEditing, editing->mpSunObj);
	Begin(mpEditing, editing->mpSkyObj);
}

void NatLightAssembly::ParamDlgProc::End(
	NatLightAssembly*	editing,
	IObjParam*			ip,
	ULONG				flags,
	Animatable*			next
)
{
	if (mpEditing != NULL) {
		End(mpEditing, mpEditing->mpSkyObj);
		End(mpEditing, mpEditing->mpSunObj);
	}

#ifndef NO_DAYLIGHT_SELECTOR
	if ((flags & END_EDIT_REMOVEUI) && mDlgWnd != NULL) {
#endif	// NO_DAYLIGHT_SELECTOR

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		if (mpMaster != NULL)
			mpMaster->EndEditParams(ip, flags, next);
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)

#ifndef NO_DAYLIGHT_SELECTOR
		ip->UnRegisterDlgWnd(mDlgWnd);
		ip->DeleteRollupPage(mDlgWnd);
		assert(mDlgWnd == NULL);
	} else
      DLSetWindowLongPtr(mDlgWnd, NULL);
#endif	// NO_DAYLIGHT_SELECTOR

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	mpMaster = NULL;
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	mpEditing = NULL;
	mpIp = NULL;
}

void NatLightAssembly::ParamDlgProc::Begin(
	NatLightAssembly*	natLight,
	Object*				obj
)
{
	if (mpEditing == natLight && obj != NULL
			&& (obj == mpEditing->mpSunObj || obj == mpEditing->mpSkyObj)) {
		End(natLight, obj);
		if (obj == mpEditing->mpSunObj)
			mpEditSun = obj;
		else
			mpEditSky = obj;
		obj->BeginEditParams(mpIp, mFlags, NULL);
		if (obj == mpEditSun && isModifyPanel() && obj->GetCustAttribContainer())
			obj->GetCustAttribContainer()->BeginEditParams(mpIp, mFlags, NULL);
	}
}

void NatLightAssembly::ParamDlgProc::End(
	NatLightAssembly*	natLight,
	Object*				obj
)
{
	if (mpEditing == natLight && obj != NULL
			&& (obj == mpEditSun || obj == mpEditSky)) {
		obj->EndEditParams(mpIp, END_EDIT_REMOVEUI, NULL);
		if (obj == mpEditSun && isModifyPanel() && obj->GetCustAttribContainer())
			obj->GetCustAttribContainer()->EndEditParams(mpIp, END_EDIT_REMOVEUI, NULL);
		if (obj == mpEditSun)
			mpEditSun = NULL;
		else
			mpEditSky = NULL;
	}
}

#ifndef NO_DAYLIGHT_SELECTOR
INT_PTR CALLBACK NatLightAssembly::ParamDlgProc::DialogProc(
	HWND		hwnd,
	UINT		msg,
	WPARAM		w,
	LPARAM		l
)
{
	if (msg == WM_INITDIALOG) {
      DLSetWindowLongPtr(hwnd, l);
		ParamDlgProc* p = reinterpret_cast<ParamDlgProc*>(l);
		p->mDlgWnd = hwnd;
		BOOL rVal = p->InitDialog(reinterpret_cast<HWND>(w));
		p->Update(p->mpEditing);
		return rVal;
	}

   ParamDlgProc* p = DLGetWindowLongPtr<ParamDlgProc*>(hwnd);

	if (p != NULL) {
      INT_PTR rVal = p->DlgProc(hwnd, msg, w, l);
		if (msg == WM_NCDESTROY)
			p->mDlgWnd = NULL;
		return rVal;
	}

	return false;
}

INT_PTR NatLightAssembly::ParamDlgProc::DlgProc(
	HWND		hwnd,
	UINT		msg,
	WPARAM		w,
	LPARAM		l
)
{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(w)) {
				case IDC_SUN_ACTIVE: {
					if (mpEditing->mpSunObj != NULL) {
						BOOL on = IsDlgButtonChecked(mDlgWnd, IDC_SUN_ACTIVE);
						if (on != mpEditing->mpSunObj->GetUseLight()) {
							theHold.Begin();
							mpEditing->mpSunObj->SetUseLight(on);
							theHold.Accept(GetString(IDS_UNDO_PARAM));
							Interface* ip = GetCOREInterface();
							ip->RedrawViews(ip->GetTime());
						}
					}
				} break;

				case IDC_SKY_ACTIVE: {
					if (mpEditing->mpSkyObj != NULL) {
						BOOL on = IsDlgButtonChecked(mDlgWnd, IDC_SKY_ACTIVE);
						if (on != mpEditing->mpSkyObj->GetUseLight()) {
							theHold.Begin();
							mpEditing->mpSkyObj->SetUseLight(on);
							theHold.Accept(GetString(IDS_UNDO_PARAM));
							Interface* ip = GetCOREInterface();
							ip->RedrawViews(ip->GetTime());
						}
					}
				} break;

				case IDC_MANUAL_POSITION: {
					Interface* ip = GetCOREInterface();
					TimeValue t = ip->GetTime();
					theHold.Begin();
					mpEditing->mpParams->SetValue(MANUAL_PARAM, t, MANUAL_MODE);
					theHold.Accept(GetString(IDS_UNDO_PARAM));
					ip->RedrawViews(t);
				} break;

				case IDC_CONTROLLER_POS: {
					Interface* ip = GetCOREInterface();
					TimeValue t = ip->GetTime();
					theHold.Begin();
					mpEditing->mpParams->SetValue(MANUAL_PARAM, t, DATETIME_MODE);
					ip->RedrawViews(t);
					theHold.Accept(GetString(IDS_UNDO_PARAM));
				} break;

				case IDC_WEATHER_DATA_FILE:{
					Interface* ip = GetCOREInterface();
					TimeValue t = ip->GetTime();
					theHold.Begin();
					mpEditing->mpParams->SetValue(MANUAL_PARAM, t, WEATHER_MODE);
					ip->RedrawViews(t);
					theHold.Accept(GetString(IDS_UNDO_PARAM));
				} break;

				case IDC_SETUP_CONTROLLER:
					if (HIWORD(w) == BN_CLICKED)
					{
						Interface* ip = GetCOREInterface();
						TimeValue t = ip->GetTime();
						if(mpEditing&&mpEditing->mpParams->GetInt(MANUAL_PARAM, t)==WEATHER_MODE)
							mpEditing->LaunchWeatherDialog(hwnd);
						else
							mpIp->ExecuteMAXCommand(MAXCOM_MOTION_MODE);
					}
					break;

				case IDC_SUN_COMBO:
					if (HIWORD(w) == CBN_SELENDOK) {
						theHold.Begin();
						ReplaceNode(SUN_PARAM, GetDlgItem(mDlgWnd, IDC_SUN_COMBO));
						theHold.Accept(GetString(IDS_UNDO_PARAM));
					}
					break;

				case IDC_SKY_COMBO:
					if (HIWORD(w) == CBN_SELENDOK) {
						theHold.Begin();
						ReplaceNode(SKY_PARAM, GetDlgItem(mDlgWnd, IDC_SKY_COMBO));
						theHold.Accept(GetString(IDS_UNDO_PARAM));
					}
					break;
			}
			break;
	}

	return false;
}
#endif	// NO_DAYLIGHT_SELECTOR

void NatLightAssembly::ParamDlgProc::DeleteThis()
{
}

#ifndef NO_DAYLIGHT_SELECTOR
static int findEntry(
	HWND				combo,
	Object*				obj
)
{
	int ix;
	if (obj == NULL) {
		for (ix = SendMessage(combo, CB_GETCOUNT, 0, 0); --ix >= 0; ) {
			ClassDesc* pCd = reinterpret_cast<ClassDesc*>(
				SendMessage(combo, CB_GETITEMDATA, ix, 0));
			if (pCd == NULL) {
				break;
			}
		}
	}
	else {
		SClass_ID super = obj->SuperClassID();
		Class_ID id = obj->ClassID();
		for (ix = SendMessage(combo, CB_GETCOUNT, 0, 0); --ix >= 0; ) {
			ClassDesc* pCd = reinterpret_cast<ClassDesc*>(
				SendMessage(combo, CB_GETITEMDATA, ix, 0));
			if (pCd != NULL && pCd->SuperClassID() == super
					&& pCd->ClassID() == id) {
				break;
			}
		}
	}

	return ix;
}
#endif	// NO_DAYLIGHT_SELECTOR

void NatLightAssembly::ParamDlgProc::Update(NatLightAssembly* natLight)
{
#ifndef NO_DAYLIGHT_SELECTOR
	if (mpEditing == natLight && mDlgWnd != NULL) {
//		if (mpEditing->mpSunObj != NULL || mpEditing->mpSkyObj != NULL) {
//			AnySunAndSkyNodes check(mpEditing, mpEditing->mpSunObj, mpEditing->mpSkyObj);
//			check.CheckNodes();
//			if (!check.AnySuns() && mpEditing->mpSunObj != NULL)
//				mpEditing->DeleteReference(SUN_REF);
//			if (!check.AnySkys() && mpEditing->mpSkyObj != NULL)
//				mpEditing->DeleteReference(SKY_REF);
//		}

		bool lightSun = mpEditing->mpSunObj != NULL
			&& mpEditing->mpSunObj->SuperClassID() == kValidSuperClass;

		EnableWindow(GetDlgItem(mDlgWnd, IDC_SUN_ACTIVE), lightSun);
		CheckDlgButton(mDlgWnd, IDC_SUN_ACTIVE,
			lightSun && static_cast<LightObject*>(
				mpEditing->mpSunObj)->GetUseLight() != 0);

		bool lightSky = mpEditing->mpSkyObj != NULL
			&& mpEditing->mpSkyObj->SuperClassID() == kValidSuperClass;

		EnableWindow(GetDlgItem(mDlgWnd, IDC_SKY_ACTIVE), lightSky);
		CheckDlgButton(mDlgWnd, IDC_SKY_ACTIVE,
			lightSky && static_cast<LightObject*>(
				mpEditing->mpSkyObj)->GetUseLight() != 0);

		HWND sunCombo = GetDlgItem(mDlgWnd, IDC_SUN_COMBO);
		SendMessage(sunCombo, CB_SETCURSEL,
			findEntry(sunCombo, mpEditing->mpSunObj), 0);

		HWND skyCombo = GetDlgItem(mDlgWnd, IDC_SKY_COMBO);
		SendMessage(skyCombo, CB_SETCURSEL,
			findEntry(skyCombo, mpEditing->mpSkyObj), 0);

		TimeValue t = GetCOREInterface()->GetTime();
		CheckDlgButton(mDlgWnd, IDC_MANUAL_POSITION,
			mpEditing->mpParams->GetInt(MANUAL_PARAM, t) == MANUAL_MODE);
		CheckDlgButton(mDlgWnd, IDC_CONTROLLER_POS,
			mpEditing->mpParams->GetInt(MANUAL_PARAM, t) == DATETIME_MODE);
		CheckDlgButton(mDlgWnd, IDC_WEATHER_DATA_FILE,
			mpEditing->mpParams->GetInt(MANUAL_PARAM, t) == WEATHER_MODE);
	}
#endif	// NO_DAYLIGHT_SELECTOR
}
namespace {
#ifndef NO_DAYLIGHT_SELECTOR
inline void addLightToCombo(
	HWND			combo,
	const TCHAR*	string,
	ClassDesc*		pCd
)
{
	// Add this one to the sun combo.
	int ix = SendMessage(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(string));
	if (ix >= 0) {
		SendMessage(combo, CB_SETITEMDATA, ix, reinterpret_cast<LPARAM>(pCd));
	}
}

const MCHAR* GetSunName( ClassDesc& cd) 
{
	if (cd.ClassID() == kStandardSunClass) 
{
		// Direct lights are called Standard sunlight
		return GetString(IDS_STANDARD_SUNLIGHT);
	}
	return cd.ClassName();
}

const MCHAR* GetSkyName( ClassDesc& cd) 
{
	return cd.ClassName();
}

void addLightsToComboBoxes(
	Interface*	pIp,
	HWND		sunCombo,
	HWND		skyCombo
)
{
	SendMessage(sunCombo, CB_RESETCONTENT, 0, 0);
	SendMessage(skyCombo, CB_RESETCONTENT, 0, 0);

	// Look for lights that have the SUN_INTERFACE_ID or SKY_INTERFACE_ID
	// mixin interfaces. These will go into the sun and sky combo boxes.
	ClassDirectory& dir = pIp->GetDllDir().ClassDir();
	SubClassList& sc = *dir.GetClassList(kValidSuperClass);

	if (&sc != NULL) 
	{
		for (int isub = sc.Count(ACC_ALL); --isub >= 0; ) 
		{
			ClassDesc* pCd = sc[isub].FullCD();
			if (pCd != NULL) 
			{
				if (IsValidSun(*pCd))
				{
					addLightToCombo(sunCombo, GetSunName(*pCd), pCd);
					}
				else if (IsValidSky(*pCd))
				{
					addLightToCombo(skyCombo, GetSkyName(*pCd), pCd);
				}
			}
		}
	}

	// Finally add "No sunlight" and "No skylight" at the top
	SendMessage(sunCombo, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(LPCTSTR(GetString(IDS_NO_SUNLIGHT))));
	SendMessage(skyCombo, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(LPCTSTR(GetString(IDS_NO_SKYLIGHT))));
}
} // namespace

INT_PTR NatLightAssembly::ParamDlgProc::InitDialog(HWND)
{
	addLightsToComboBoxes(mpIp, GetDlgItem(mDlgWnd, IDC_SUN_COMBO),
		GetDlgItem(mDlgWnd, IDC_SKY_COMBO));
	return true;
}

void NatLightAssembly::ParamDlgProc::ReplaceNode(
	ParamID	param,
	HWND	combo
)
{
	if (mpEditing == NULL)
		return;

	// Get selection from combo. If nothing is selected return.
	int sel = SendMessage(combo, CB_GETCURSEL, 0, 0);
	if (sel < 0)
		return;

	// Get class description from combo. NULL means we don't
	// know how to build the object.
	RefTargetHandle obj = mpEditing->mpParams->GetReferenceTarget(param);
	ClassDesc* pCd = reinterpret_cast<ClassDesc*>(SendMessage(
		combo, CB_GETITEMDATA, sel, 0));

	if (pCd != NULL) {
		// If the object is the same. Don't do anything.
		if (obj == NULL || obj->SuperClassID() != pCd->SuperClassID()
				|| obj->ClassID() != pCd->ClassID()) {
			theHold.Suspend();
			Interface* ip = GetCOREInterface();
			obj = static_cast<RefTargetHandle>(
				ip->CreateInstance(pCd->SuperClassID(), pCd->ClassID()));
			theHold.Resume();
			assert(obj != NULL);
			if (obj->SuperClassID() == LIGHT_CLASS_ID) {
				static_cast<GenLight*>(obj)->Enable(1);
				if (param == SUN_PARAM) {
					SetUpSun(HELPER_CLASS_ID, NATLIGHT_HELPER_CLASS_ID,
						ip, static_cast<GenLight*>(obj), SUN_RGB);
				}
			}

		}
	} else
		obj = NULL;

	mpEditing->mpParams->SetValue(param, GetCOREInterface()->GetTime(), obj);
}
#endif	// NO_DAYLIGHT_SELECTOR

/*===========================================================================*\
 | Natural Light Assembly Param Accessor Class:
\*===========================================================================*/

void NatLightAssembly::Accessor::Get(
	PB2Value&		v,
	ReferenceMaker*	owner,
	ParamID			id,
	int				tabIndex,
	TimeValue		t,
	Interval&		valid
)
{
	NatLightAssembly* p = static_cast<NatLightAssembly*>(owner);

	switch(id) {
		case MANUAL_PARAM: {
			FindManual findManual(p, t);
			v.i = findManual.FindManualSetting();
		} break;
	}
}

void NatLightAssembly::SunMasterManualChanged(IDaylightSystem2::DaylightControlType type)
{
	SunMaster* themaster = FindMaster();	
	bool setControlled = false;
	if(type == IDaylightSystem2::eWeatherFile)
	{
		setControlled = true;
		if(themaster)
		{
			themaster->controlledByWeatherFile = true;
			themaster->natLight = this;
			//we call this instead of just SetUpBasedOnWeatherFile since we just said the file was missing. It's possible
			//that the file has moved or chagned so we re-find it when re-enter weather mode.
			TSTR currentName = GetWeatherFileName();
			SetWeatherFileName(currentName); //reset it so it's found this will call SetUpBasedOnWeatherFile.
		}
	}
	else if(themaster)
		themaster->controlledByWeatherFile = false;
	SetDaylightControlledIfNeeded(GetReference(SKY_REF),setControlled);
	SetDaylightControlledIfNeeded(GetReference(SUN_REF),setControlled);
}

void NatLightAssembly::Accessor::Set(
	PB2Value&		v,
	ReferenceMaker*	owner,
	ParamID			id,
	int				tabIndex,
	TimeValue		t
)
{
	NatLightAssembly* p = static_cast<NatLightAssembly*>(owner);

	switch(id) {
		case SUN_PARAM: {
			TSTR sunName = GetString(IDS_LIGHT_NAME);
			p->SetSunAndSky(t, SUN_REF, SUN_PARAM, sunName, v.r);
		} break;

		case SKY_PARAM: {
			TSTR skyName = GetString(IDS_SKY_NAME);
			p->SetSunAndSky(t, SKY_REF, SKY_PARAM, skyName, v.r);
		} break;

		case MANUAL_PARAM: {
			SunMaster* themaster = p->FindMaster();	
			if(v.i==WEATHER_MODE){
				if(themaster)
				{
					p->mWeatherFileMissing= true; //assume the file isn't loaded yet!
				}
			}
			else{
				if(themaster)
				{
					themaster->controlledByWeatherFile = false;
					themaster->natLight = NULL;
				}
			}
			SetManual setManual(p, t, v.i);
			setManual.SetManualSetting();
			
			
			} break;

		case WEATHER_FILE_PARAM:
			{
			/*	To Do.
				TCHAR resolveFile[MAX_PATH * 2] = {0};
				if(BMMGetUniversalName(resolveFile, v.s)) {
					 if(v.s != NULL)   {
							v.Free(TYPE_STRING);
					 }
					 v.s = _tcscpy((TCHAR*)malloc(_tcslen(resolveFile) + sizeof(TCHAR)), resolveFile);
				}
				*/
			}break;
	}
}


/*===========================================================================*\
 | Natural Light Assembly Find Assembly Head Class
\*===========================================================================*/

int NatLightAssembly::FindAssemblyHead::proc(ReferenceMaker *rmaker)
{
	if (rmaker->SuperClassID() == BASENODE_CLASS_ID) {
		INode* node = static_cast<INode*>(rmaker);
		Object* obj = node->GetObjectRef();
		if (obj != NULL) {
			obj = obj->FindBaseObject();
			if (obj == mpNatLight) {
				return proc(node);
			}
		}
		return DEP_ENUM_SKIP;
	}

	return DEP_ENUM_CONTINUE;
}

/*===========================================================================*\
 | Natural Light Assembly Are there any Sun and Sky Class
\*===========================================================================*/

bool NatLightAssembly::AnySunAndSkyNodes::proc(
	INode*		head,
	INode*		node,
	IAssembly*
)
{
	Object* obj = node->GetObjectRef();
	if (obj != NULL) {
		obj = obj->FindBaseObject();
		if (obj == mpSun && mpSun != NULL) {
			mAnySuns = true;
		}
		if (obj == mpSky && mpSky != NULL) {
			mAnySkys = true;
		}
	}

	return true;
}

int NatLightAssembly::AnySunAndSkyNodes::proc(INode* node)
{
	traverse(node, kHead | kOpenSubAssemblies
		| kClosedSubAssemblies);
	if ((mAnySkys || mpSky == NULL) && (mAnySuns || mpSun == NULL))
		return DEP_ENUM_HALT;
	return DEP_ENUM_CONTINUE;
}


/*===========================================================================*\
 | Natural Light Assembly Replace Sun and Sky Class
\*===========================================================================*/

static void DeleteNode(
	Interface*	ip,
	INode*		newNode,
	INode*		node
)
{
	if (node != NULL) {
		int i, count = node->NumberOfChildren();

		for (i = 0; i < count; ++i) {
			INode* child = node->GetChildNode(i);
			child->Detach(0);
			newNode->AttachChild(child);
		}

		IAssembly* asmb = GetAssemblyInterface(node);
		if (asmb != NULL) {
			asmb->SetAssemblyMember(false);
			node->Detach(0);
			asmb->SetAssemblyMember(true);
		}
		ip->DeleteNode(node);
	}
}

void NatLightAssembly::ReplaceSunAndSkyNodes::ReplaceNodes(
	TimeValue			t,
	IAssemblyMgr*		mgr,
	Interface*			ip,
	Object*				newObj,
	const TCHAR*		baseName
)
{
	FindHeadNodes();

	int i;
	INode* lastHead = NULL;
	INode* node = NULL;

	for (i = mNodes.Count(); --i >= 0; ) {
		// Now we need to replace the object in this node.
		ObjEntry& entry = mNodes[i];

		if (lastHead == entry.mpHead || newObj == NULL) {
			DeleteNode(ip, node, entry.mpNode);
		}
		else if (entry.mpNode != NULL && mpOld == newObj) {
			node = entry.mpNode;
		}
		else {
			node = ip->CreateObjectNode(newObj);
			assert(node != NULL);
			SuspendAnimate();
			node->SetNodeTM(t, entry.mpHead->GetObjTMBeforeWSM(t));
			ResumeAnimate();
			entry.mpHead->AttachChild(node);

			if (mgr != NULL) {
				INodeTab nodes;
				nodes.Append(1, &node);
				mgr->Attach(&nodes, entry.mpHead);
			}

			INodeGIProperties* giNode;
			INodeGIProperties2* giHead2 = static_cast<INodeGIProperties2*>(
				entry.mpHead->GetInterface(NODEGIPROPERTIES2_INTERFACE));
			INodeGIProperties2* giNode2 = static_cast<INodeGIProperties2*>(
				node->GetInterface(NODEGIPROPERTIES2_INTERFACE));
			if (giHead2 != NULL && giNode2 != NULL) {
                giNode2->CopyGIPropertiesFrom(*giHead2);
                giNode = giNode2;
            } else {
				INodeGIProperties* giHead = static_cast<INodeGIProperties*>(
					entry.mpHead->GetInterface(NODEGIPROPERTIES_INTERFACE));
				giNode = static_cast<INodeGIProperties*>(
					node->GetInterface(NODEGIPROPERTIES_INTERFACE));
				if (giHead != NULL && giNode != NULL) {
					giNode->CopyGIPropertiesFrom(*giHead);
				}
			}

			if (giNode != NULL) {
				bool storeDirect = _storeDirect;
				if (entry.mpNode != NULL) {
					INodeGIProperties* gi = static_cast<INodeGIProperties*>(
						entry.mpNode->GetInterface(NODEGIPROPERTIES_INTERFACE));
					storeDirect = gi->GIGetStoreIllumToMesh() != 0;
				}
				giNode->GISetStoreIllumToMesh(storeDirect);
			}

			if (newObj->SuperClassID() == LIGHT_CLASS_ID)
				ip->AddLightToScene(node);

			TSTR name;
			if (entry.mpNode != NULL) {
				name = entry.mpNode->GetName();
				DeleteNode(ip, node, entry.mpNode);
			}
			else {
				name = baseName;
				ip->MakeNameUnique(name);
			}

			node->SetName(name);
		}

		lastHead = entry.mpHead;
	}
}

int NatLightAssembly::ReplaceSunAndSkyNodes::proc(
	INode*		node
)
{
	int count = mNodes.Count();
	// This node has a natural light assembly attached.
	// Find the objects and replace the nodes.
	if (mpOld != NULL) {
		// Find the existing object in the assembly
		traverse(node, kHead | kOpenSubAssemblies
			| kClosedSubAssemblies);
	}

	if (count == mNodes.Count()) {
		// We don't have an existing object, so
		// we will just add an entry to create a new one.
		ObjEntry entry(node, NULL);
		mNodes.Append(1, &entry, 3);
	}

	return DEP_ENUM_SKIP;
}

bool NatLightAssembly::ReplaceSunAndSkyNodes::proc(
	INode*		head,
	INode*		node,
	IAssembly*
)
{
	Object* obj = node->GetObjectRef();
	if (obj != NULL) {
		obj = obj->FindBaseObject();
		if (obj == mpOld) {
			ObjEntry entry(head, node);
			mNodes.Append(1, &entry, 3);
		}
	}

	return true;
}

/*===========================================================================*\
 | Natural Light Assembly Find Manual Setting Class
\*===========================================================================*/

int NatLightAssembly::FindManual::FindManualSetting()
{
	FindHeadNodes();
	if(mDateTimeCount==0&&mWeatherCount==0)
		return MANUAL_MODE;
	else
		if(mDateTimeCount>=mWeatherCount)
			return DATETIME_MODE;
		else
			return WEATHER_MODE;
}

int NatLightAssembly::FindManual::proc(INode* head)
{
	Control* tm = head->GetTMController();
	ReferenceTarget* master;
	if (tm != NULL
			&& tm->IsSubClassOf(Class_ID(DAYLIGHT_SLAVE_CONTROL_CID1,
				DAYLIGHT_SLAVE_CONTROL_CID2))
			&& (master = tm->GetReference(0)) != NULL
			&& master->IsSubClassOf(DAYLIGHT_CLASS_ID)
			) 
	{
		if(static_cast<SunMaster*>(master)->GetManual(mT)==DATETIME_MODE)
			++mDateTimeCount;
		else if (static_cast<SunMaster*>(master)->GetManual(mT)==WEATHER_MODE)
			++mWeatherCount;
		else
			++mManualCount;

	}
	else
		++mManualCount;
	return DEP_ENUM_SKIP;
}

/*===========================================================================*\
 | Natural Light Assembly Set Manual Setting Class
\*===========================================================================*/

void NatLightAssembly::SetManual::SetManualSetting()
{
	FindHeadNodes();
}

int NatLightAssembly::SetManual::proc(INode* head)
{
	Control* tm = head->GetTMController();
	ReferenceTarget* master;
	if (tm != NULL
			&& tm->IsSubClassOf(Class_ID(DAYLIGHT_SLAVE_CONTROL_CID1,
				DAYLIGHT_SLAVE_CONTROL_CID2))
			&& (master = tm->GetReference(0)) != NULL
			&& master->IsSubClassOf(DAYLIGHT_CLASS_ID)) {
		static_cast<SunMaster*>(master)->SetManual(mT, mManual);
	}
	return DEP_ENUM_SKIP;
}

/*===========================================================================*\
 | Natural Light Assembly Open Assembly Class
\*===========================================================================*/

void NatLightAssembly::OpenAssembly::Open()
{
	IAssemblyMgr* mgr = GetAssemblyMgr();
	if (mgr != NULL) {
		mTab.ZeroCount();
		FindHeadNodes();
		mgr->Open(&mTab, false);
	}
}

int NatLightAssembly::OpenAssembly::proc(INode* head)
{
	mTab.Append(1, &head, 3);
	return DEP_ENUM_SKIP;
}

/*===========================================================================*\
 | Natural Light Assembly Open and Close Assembly Class
\*===========================================================================*/

void NatLightAssembly::CloseAssembly::Close()
{
	IAssemblyMgr* mgr = GetAssemblyMgr();
	if (mgr != NULL) {
		mSel.ZeroCount();
		mNotSel.ZeroCount();
		FindHeadNodes();
		if (mSel.Count() > 0)
			mgr->Close(&mSel, true);
		if (mNotSel.Count() > 0)
			mgr->Close(&mNotSel, false);
	}
}

int NatLightAssembly::CloseAssembly::proc(INode* head)
{
	if (head->Selected())
		mSel.Append(1, &head, 3);
	else
		mNotSel.Append(1, &head, 3);
	return DEP_ENUM_SKIP;
}

/*===========================================================================*\
 | Natural Light Assembly Sun Mesh Class
\*===========================================================================*/

void NatLightAssembly::SunMesh::CreateMesh(
	float	radius,
	int		nbRays,
	float	heightFactor
)
{
	const float kPI = 3.14159265358979323846f;
	float seg = float(2 * kPI / nbRays);
	float halfSeg = seg / 2.0f;
	float tip = radius * (1.0f + heightFactor * 2.0f * (1.0f - cosf(seg)));
	int i;

	setNumVerts(2 * nbRays);
	for (i = 0; i < nbRays; ++i) {
		// First the point on the circle
		float angle = i * seg;
		setVert(i + i, Point3(radius * cosf(angle), radius * sinf(angle), 0.0f));
		// Now the tip of the ray
		angle += halfSeg;
		setVert(i + i + 1, Point3(tip * cosf(angle), tip * sinf(angle), 0.0f));
	}

	setNumFaces(2 * nbRays);
	for (i = 0; i < nbRays; ++i) {
		Face* f = faces + i;

		f->v[0] = i + i;
		f->v[1] = i + i + 1;
		f->v[2] = i + i + 2;
		f->smGroup = 0;
		f->flags = EDGE_ALL;

		f = faces + i + nbRays;
		f->v[0] = i + i + 2;
		f->v[1] = i + i + 1;
		f->v[2] = i + i;
		f->smGroup = 0;
		f->flags = EDGE_ALL;
	}

	faces[nbRays - 1].v[2] = 0;
	faces[nbRays + nbRays - 1].v[0] = 0;
}


void NatLightAssembly::SetPosition(const Point3& position)
{
	SunMaster* themaster = FindMaster();
	if(themaster)
		themaster->SetPosition(position);
}

Point3 NatLightAssembly::GetPosition() const
{
	Point3 position(0.0f,0.0f,0.0f);
	SunMaster* themaster = FindMaster();
	if(themaster)
		position = themaster->GetPosition();
	return position;
}

void  NatLightAssembly::SetOrbitalScale(float orbScale)
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	if(themaster)
		themaster->SetRad(ip->GetTime(),orbScale);
}

float NatLightAssembly::GetOrbitalScale() const
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	float orbScale = 0.0f;
	if(themaster)
		orbScale = themaster->GetRad(ip->GetTime());
	return orbScale;
}

void NatLightAssembly::SetNorthDirection(float angle)
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	if(themaster)
		themaster->SetNorth(ip->GetTime(),angle);
}

float NatLightAssembly::GetNorthDirection() const
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	float angle = 0.0f;
	if(themaster)
		angle = themaster->GetNorth(ip->GetTime());
	return angle;
}

void NatLightAssembly::SetCompassDiameter(float compassDiameter)
{
	SunMaster* themaster = FindMaster();
	if(themaster)
		themaster->SetCompassDiameter(compassDiameter);
}

float NatLightAssembly::GetCompassDiameter() const
{
	SunMaster* themaster = FindMaster();
	float compassDiameter = 0.0f;
	if(themaster)
		compassDiameter = themaster->GetCompassDiameter();
	return compassDiameter;
}

void NatLightAssembly::SetTimeOfDay(const Point3& time)
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	if(themaster)
	{
		themaster->SetHour(ip->GetTime(),time.x);
		themaster->SetMin(ip->GetTime(),time.y);
		themaster->SetSec(ip->GetTime(),time.z);
	}
}

Point3 NatLightAssembly::GetTimeOfDay() const
{
	Interface* ip = GetCOREInterface();
	Point3 time(0.0f,0.0f,0.0f);
	SunMaster* themaster = FindMaster();
	if(themaster)
	{
		time.x = themaster->GetHour(ip->GetTime());
		time.y = themaster->GetMin(ip->GetTime());
		time.z = themaster->GetSec(ip->GetTime());
	}
	return time;
}

void NatLightAssembly::SetDate(const Point3& date)
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	if(themaster)
	{
		themaster->SetMon(ip->GetTime(),date.x);
		themaster->SetDay(ip->GetTime(),date.y);
		themaster->SetYr(ip->GetTime(),date.z);
	}
}

Point3 NatLightAssembly::GetDate() const
{
	Interface* ip = GetCOREInterface();
	Point3 date(0.0f,0.0f,0.0f);
	SunMaster* themaster = FindMaster();
	if(themaster)
	{
		date.x = themaster->GetMon(ip->GetTime());
		date.y = themaster->GetDay(ip->GetTime());
		date.z = themaster->GetYr(ip->GetTime());
	}
	return date;
}

void NatLightAssembly::SetLatLong(float latitude, float longitude)
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	if(themaster)
	{
		if(GetDaylightControlType()!= IDaylightSystem2::eWeatherFile)
		{
			themaster->SetLat(ip->GetTime(),latitude);
			themaster->SetLong(ip->GetTime(),longitude);
			themaster->SetZone(ip->GetTime(),getTimeZone(longitude));
			themaster->calculateCity(ip->GetTime());
			// Calculate and set Azimuth and Altitude of the sun
			// based on date, time, latitude, longitude, time zone,
			// and daylight savings that are set above.
			themaster->calculate(ip->GetTime(),FOREVER);
		}
	}
}

float NatLightAssembly::GetLatitude() const
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	float latitude = 0.0f;
	if(themaster)
		latitude = themaster->GetLat(ip->GetTime());
	return latitude;
}


float NatLightAssembly::GetLongitude() const
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	float longitude = 0.0f;
		if(themaster)
			longitude = themaster->GetLong(ip->GetTime());
	return longitude;
}

void NatLightAssembly::SetDaylightSavingTime(BOOL isDaylightSavingTime)
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	if(themaster)
		themaster->SetDst(ip->GetTime(),isDaylightSavingTime);
}

BOOL NatLightAssembly::GetDaylightSavingTime() const
{
	Interface* ip = GetCOREInterface();
	SunMaster* themaster = FindMaster();
	BOOL isDaylightSavingTime = false;
	if(themaster)
		isDaylightSavingTime = themaster->GetDst(ip->GetTime());
	return isDaylightSavingTime;
}

int NatLightAssembly::GetTimeZone(float longitude) const
{
	return getTimeZone(longitude);
}

LightObject* NatLightAssembly::SetSun(const Class_ID& sunClassID)
{
	NatLightAssemblyClassDesc* natLightCD = dynamic_cast<NatLightAssemblyClassDesc*>(GetNatLightAssemblyDesc());
	DbgAssert(natLightCD != NULL);
	LightObject* lightObj = NULL;
	Object* sunObj = natLightCD->CreateSun(GetCOREInterface(), &sunClassID);
	if (sunObj != NULL)
	{
		TSTR sunName = GetString(IDS_LIGHT_NAME);
		SetSunAndSky(0, SUN_REF, SUN_PARAM, sunName, sunObj);
		lightObj = dynamic_cast<LightObject*>(sunObj);
	}
	return lightObj;
}

LightObject* NatLightAssembly::SetSky(const Class_ID& skyClassID)
{
	NatLightAssemblyClassDesc* natLightCD = dynamic_cast<NatLightAssemblyClassDesc*>(GetNatLightAssemblyDesc());
	DbgAssert(natLightCD != NULL);
	LightObject* lightObj = NULL;
	Object* skyObj = natLightCD->CreateSky(GetCOREInterface(), &skyClassID);
	if (skyObj != NULL)
	{
		TSTR skyName = GetString(IDS_SKY_NAME);
		SetSunAndSky(0, SKY_REF, SKY_PARAM, skyName, skyObj);
		lightObj = dynamic_cast<LightObject*>(skyObj);
	}
	return lightObj;
}

Object* NatLightAssembly::fpSetSun(ClassDesc* sunClass)
{
	if (sunClass != NULL)
	{
		return SetSun(sunClass->ClassID());
	}
	return NULL;
}

Object* NatLightAssembly::fpSetSky(ClassDesc* skyClass)
{
	if (skyClass != NULL)
	{
		return SetSky(skyClass->ClassID());
	}
	return NULL;
}

FPInterfaceDesc NatLightAssembly::mFPInterfaceDesc(DAYLIGHT_SYSTEM_INTERFACE, 
	_T("IDaylightSystem"), // interface name used by maxscript - don't localize it!
	0, 
	&natLightDesc, 
	FP_MIXIN, 

	// - Methods -
	NatLightAssembly::eGetSun, _T("GetSun"), 0, TYPE_OBJECT, 0, 0, 
	NatLightAssembly::eSetSun, _T("SetSun"), 0, TYPE_OBJECT, 0, 1, 
		_T("sunClass"), 0, TYPE_CLASS, f_inOut, FPP_IN_PARAM, 
	NatLightAssembly::eGetSky, _T("GetSky"), 0, TYPE_OBJECT, 0, 0, 
	NatLightAssembly::eSetSky, _T("SetSky"), 0, TYPE_OBJECT, 0, 1, 
		_T("skyClass"), 0, TYPE_CLASS, f_inOut, FPP_IN_PARAM, 

	NatLightAssembly::eGetDaylightControlType,_T("GetDaylightControlType"),0,TYPE_INT,0,0,

	NatLightAssembly::eSetDaylightControlType,_T("SetDaylightControlType"),0,TYPE_VOID,0,1,
				_T("type"),0,TYPE_INT,
	NatLightAssembly::eGetLocation,_T("GetLocation"),0 , TYPE_TSTR_BV,0,0,
	NatLightAssembly::eGetAltitudeAzimuth,_T("GetAltitudeAzimuth"), 0, TYPE_VOID, 0,3, 
				_T("time"), 0, TYPE_TIMEVALUE,
				_T("altitude"),0, TYPE_FLOAT_BR,
				_T("azimuth"), 0, TYPE_FLOAT_BR,
	NatLightAssembly::eGetWeatherFileName,_T("GetWeatherFileName"), 0, TYPE_FILENAME,0,0,
	NatLightAssembly::eOpenWeatherFileDlg,_T("OpenWeatherFileDlg"), 0, TYPE_VOID, 0,0,
	NatLightAssembly::eMatchTimeLine,_T("MatchTimeLine"), 0, TYPE_VOID, 0,0,
	NatLightAssembly::eLoadWeatherFile,_T("LoadWeatherFile"),0, TYPE_VOID,  0, 1,
				_T("file"), 0, TYPE_FILENAME,
	NatLightAssembly::eSetWeatherFileFilterAnimated,_T("SetWeatherFileFilterAnimated"),0, TYPE_VOID, 0,1,
				_T("animated"), 0, TYPE_BOOL,
	NatLightAssembly::eGetWeatherFileFilterAnimated,_T("GetWeatherFileFilterAnimated"),0, TYPE_BOOL, 0,0,
	NatLightAssembly::eSetWeatherFileSpecificTimeAndDate,_T("SetWeatherFileSpecificTimeAndDate"), 0, TYPE_VOID,0,2,
				_T("timeOfDay"),0, TYPE_POINT3_BR,
				_T("date"), 0, TYPE_POINT3_BR,
	NatLightAssembly::eGetWeatherFileSpecificTimeAndDate,_T("GetWeatherFileSpecificTimeAndDate"),0, TYPE_VOID,0,2,
				_T("timeOfDay"),0, TYPE_POINT3_BR,
				_T("date"), 0, TYPE_POINT3_BR,
	NatLightAssembly::eSetWeatherFileAnimStartTimeAndDate,_T("SetWeatherFileAnimStartTimeAndDate"),0,TYPE_VOID,0,2,
				_T("timeOfDay"),0, TYPE_POINT3_BR,
				_T("date"), 0, TYPE_POINT3_BR,
	NatLightAssembly::eGetWeatherFileAnimStartTimeAndDate,_T("GetWeatherFileAnimStartTimeAndDate"),0,TYPE_VOID,0,2,
				_T("timeOfDay"),0, TYPE_POINT3_BR,
				_T("date"), 0 ,TYPE_POINT3_BR,
	NatLightAssembly::eSetWeatherFileAnimEndTimeAndDate,_T("SetWeatherFileAnimEndTimeAndDate"),0,TYPE_VOID,0,2,
				_T("timeOfDay"),0, TYPE_POINT3_BR,
				_T("date"), 0 ,TYPE_POINT3_BR,
	NatLightAssembly::eGetWeatherFileAnimEndTimeAndDate,_T("GetWeatherFileAnimEndTimeAndDate"),0,TYPE_VOID,0,2,
				_T("timeOfDay"),0, TYPE_POINT3_BR,
				_T("date"), 0 ,TYPE_POINT3_BR,
	NatLightAssembly::eSetWeatherFileSkipHours,_T("SetWeatherFileSkipHours"), 0, TYPE_VOID, 0, 3,
				_T("skipHours"), 0, TYPE_BOOL,
				_T("startHour"), 0, TYPE_INT,
				_T("endHour"), 0,   TYPE_INT,	
	NatLightAssembly::eGetWeatherFileSkipHours,_T("GetWeatherFileSkipHours"), 0, TYPE_VOID, 0, 3, 
				_T("skipHours"), 0, TYPE_BOOL_BR,
				_T("startHour"), 0, TYPE_INT_BR,
				_T("endHour"), 0,   TYPE_INT_BR,
	NatLightAssembly::eSetWeatherFileSkipWeekend,_T("SetWeatherFileSkipWeekend"), 0, TYPE_VOID, 0, 1,
				_T("skipWeekend"), 0, TYPE_BOOL,
	NatLightAssembly::eGetWeatherFileSkipWeekend,_T("GetWeatherFileSkipWeekend"), 0, TYPE_BOOL, 0, 0, 
	NatLightAssembly::eSetWeatherFileSkipFramePer,_T("SetWeatherFileSkipFramePer"), 0, TYPE_VOID, 0, 1,
				_T("framePer"), 0, TYPE_INT,
	NatLightAssembly::eGetWeatherFileSkipFramePer,_T("GetWeatherFileSkipFramePer"), 0, TYPE_INT, 0, 0, 

	// - Properties -
	end 
); 

int NatLightAssembly::fpGetDaylightControlType()
{
	IDaylightSystem2::DaylightControlType type = GetDaylightControlType();
	return static_cast<int>(type);
}

void NatLightAssembly::fpSetDaylightControlType(int type)
{
	if(type<0||type>2)//for now it's 2
		return;

	IDaylightSystem2::DaylightControlType val = static_cast<IDaylightSystem2::DaylightControlType>(type);
	SetDaylightControlType(val);
}

void NatLightAssembly::LoadWeatherFile(TCHAR *filename)
{
	if(filename==NULL||_tcslen(filename)<1)
		return;
	TSTR val(filename);
	SetWeatherFileName(val);
}

void NatLightAssembly::SetWeatherFileAnimated(BOOL val)
{
	mWeatherData.mAnimated = val;
}

BOOL NatLightAssembly::GetWeatherFileAnimated()
{
	return mWeatherData.mAnimated;
}

void NatLightAssembly::SetWeatherFileSpecificTimeAndDate(Point3 &time,Point3 &date)
{
	mWeatherData.mSpecificTimeHMS.x = (int)time.x;
	mWeatherData.mSpecificTimeHMS.y = (int)time.y;
	mWeatherData.mSpecificTimeHMS.z = (int)time.z;
	mWeatherData.mSpecificTimeMDY.x = (int)date.x;
	mWeatherData.mSpecificTimeMDY.y = (int)date.y;
	mWeatherData.mSpecificTimeMDY.z = (int)date.z;
}

void NatLightAssembly::GetWeatherFileSpecificTimeAndDate(Point3 &time,Point3 &date)
{
	time.x = (float)mWeatherData.mSpecificTimeHMS.x; 
	time.y = (float)mWeatherData.mSpecificTimeHMS.y; 
	time.z = (float)mWeatherData.mSpecificTimeHMS.z; 
	date.x = (float)mWeatherData.mSpecificTimeMDY.x; 
	date.y = (float)mWeatherData.mSpecificTimeMDY.y; 
	date.z = (float)mWeatherData.mSpecificTimeMDY.z; 
}

void  NatLightAssembly::SetWeatherFileAnimStartTimeAndDate(Point3 &time,Point3 &date)
{
	mWeatherData.mAnimStartTimeHMS.x = (int)time.x;
	mWeatherData.mAnimStartTimeHMS.y = (int)time.y;
	mWeatherData.mAnimStartTimeHMS.z = (int)time.z;
	mWeatherData.mAnimStartTimeMDY.x = (int)date.x;
	mWeatherData.mAnimStartTimeMDY.y = (int)date.y;
	mWeatherData.mAnimStartTimeMDY.z = (int)date.z;
}

void NatLightAssembly::GetWeatherFileAnimStartTimeAndDate(Point3 &time,Point3 &date)
{
	time.x = (float)mWeatherData.mAnimStartTimeHMS.x; 
	time.y = (float)mWeatherData.mAnimStartTimeHMS.y; 
	time.z = (float)mWeatherData.mAnimStartTimeHMS.z; 
	date.x = (float)mWeatherData.mAnimStartTimeMDY.x; 
	date.y = (float)mWeatherData.mAnimStartTimeMDY.y; 
	date.z = (float)mWeatherData.mAnimStartTimeMDY.z; 
}

void NatLightAssembly::SetWeatherFileAnimEndTimeAndDate(Point3 &time,Point3 &date)
{
	mWeatherData.mAnimEndTimeHMS.x = (int)time.x;
	mWeatherData.mAnimEndTimeHMS.y = (int)time.y;
	mWeatherData.mAnimEndTimeHMS.z = (int)time.z;
	mWeatherData.mAnimEndTimeMDY.x = (int)date.x;
	mWeatherData.mAnimEndTimeMDY.y = (int)date.y;
	mWeatherData.mAnimEndTimeMDY.z = (int)date.z;
}

void NatLightAssembly::GetWeatherFileAnimEndTimeAndDate(Point3 &time,Point3 &date)
{
	time.x = (float)mWeatherData.mAnimEndTimeHMS.x; 
	time.y = (float)mWeatherData.mAnimEndTimeHMS.y; 
	time.z = (float)mWeatherData.mAnimEndTimeHMS.z; 
	date.x = (float)mWeatherData.mAnimEndTimeMDY.x; 
	date.y = (float)mWeatherData.mAnimEndTimeMDY.y; 
	date.z = (float)mWeatherData.mAnimEndTimeMDY.z; 
}

void NatLightAssembly::SetWeatherFileSkipHours(BOOL skipHours,int hourStart, int hourEnd)
{
	if(hourStart<1||hourStart>23)
		hourStart = 1;
	if(hourEnd<1||hourEnd>23)
		hourEnd = 23;
	mWeatherData.mFilter.mSkipHours = skipHours;
	mWeatherData.mFilter.mSkipHoursStart = hourStart;
	mWeatherData.mFilter.mSkipHoursEnd = hourEnd;
}

void NatLightAssembly::GetWeatherFileSkipHours(BOOL &skipHours, int &hourStart, int  &hourEnd)
{
	skipHours = mWeatherData.mFilter.mSkipHours;
	hourStart = mWeatherData.mFilter.mSkipHoursStart;
	hourEnd =  mWeatherData.mFilter.mSkipHoursEnd;
}

void NatLightAssembly::SetWeatherFileSkipWeekend(BOOL weekend)
{
	mWeatherData.mFilter.mSkipWeekends = weekend;
}

BOOL NatLightAssembly::GetWeatherFileSkipWeekend()
{
	return mWeatherData.mFilter.mSkipWeekends;
}
void NatLightAssembly::SetWeatherFileFramePer(int frameper)
{
	if(frameper<0||frameper> 4)
		return;
	mWeatherData.mFilter.mFramePer  = static_cast<WeatherCacheFilter::FramePer>(frameper);
}
int NatLightAssembly::GetWeatherFileFramePer()
{
	return static_cast<int>(mWeatherData.mFilter.mFramePer);
}

TSTR NatLightAssembly::GetLocation()
{
	SunMaster* themaster = FindMaster();
	TSTR str("");
	if(themaster)
	{
		
		if(	themaster->GetCity())
			str = TSTR(themaster->GetCity());
	}
	return str;
}


IDaylightSystem2::DaylightControlType NatLightAssembly::GetDaylightControlType()const 
{
	if(mpParams)
	{
		TimeValue t = GetCOREInterface()->GetTime(); //not animated doesn't matter.
		int type = mpParams->GetInt(MANUAL_PARAM, t);
		switch(type)
		{
		case MANUAL_MODE:
			return IDaylightSystem2::eManual;
		case DATETIME_MODE:
			return IDaylightSystem2::eDateAndTime;
		case WEATHER_MODE:
			return IDaylightSystem2::eWeatherFile;
		}
	}
	return IDaylightSystem2::eManual;  //should never happen but never know.
}

void NatLightAssembly::SetDaylightControlType(IDaylightSystem2::DaylightControlType val)
{
	if(mpParams)
	{
		TimeValue t = GetCOREInterface()->GetTime(); //not animated doesn't matter.
		int type = mpParams->GetInt(MANUAL_PARAM, t);
		switch(val)
		{
		case IDaylightSystem2::eManual:
			mpParams->SetValue(MANUAL_PARAM, t, MANUAL_MODE);
			break;
		case IDaylightSystem2::eDateAndTime:
			mpParams->SetValue(MANUAL_PARAM, t, DATETIME_MODE);
		case IDaylightSystem2::eWeatherFile:
				mpParams->SetValue(MANUAL_PARAM, t, WEATHER_MODE);
		}
	}
}

TSTR NatLightAssembly::GetWeatherFileName()const
{
	TCHAR *fname;
	Interval iv = FOREVER;
	mpParams->GetValue(WEATHER_FILE_PARAM,0,fname,iv);
	TSTR filename("");
	if(fname!=NULL)
		filename = TSTR(fname);
	return filename;
}

TCHAR * NatLightAssembly::fpGetWeatherFileName()
{
	TCHAR *fname;
	Interval iv = FOREVER;
	mpParams->GetValue(WEATHER_FILE_PARAM,0,fname,iv);
	return fname;
}

void NatLightAssembly::SetWeatherFileName(TSTR &val)
{
	if(val.isNull()==FALSE&&val.Length()>0)
	{
		MaxSDK::Util::Path path(val);
		IPathConfigMgr::GetPathConfigMgr()->NormalizePathAccordingToSettings(path);
		val = path.GetString();
		//now see if that file exists if not then set a flag so we can possibly give an error message at some point
		IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();
		if (!pathMgr->SearchForExternalFiles(path)) 
			mWeatherFileMissing = true;
		else
			mWeatherFileMissing = false;
	}
	else
	{
		mWeatherFileMissing = true;
	}
	mpParams->SetValue(WEATHER_FILE_PARAM,0,const_cast<TCHAR *>(val.data()));

	SetUpBasedOnWeatherFile();

}

void NatLightAssembly::OpenWeatherFileDlg()
{
	HWND hDlg = GetCOREInterface()->GetMAXHWnd();
	LaunchWeatherDialog(hDlg);
}

void NatLightAssembly::MatchTimeLine()
{
	int totalFrames = mWeatherFileCache.mNumEntries;
	if(totalFrames>1)
	{
		TimeValue start  = GetCOREInterface10()->GetAutoKeyDefaultKeyTime();
		Interval range(start,start+ totalFrames*GetTicksPerFrame()
			-GetTicksPerFrame());
		GetCOREInterface()->SetAnimRange(range);
	}

}
//launch the weather file dialog.
void NatLightAssembly::LaunchWeatherDialog(HWND hDlg)
{
	if(GetDaylightControlType() ==IDaylightSystem2::eWeatherFile)
	{
		WeatherFileDialog dlg;
		WeatherUIData data(mWeatherData);
		data.mFileName = GetResolvedWeatherFileName();
		if(data.mFileName.Length()>0)
			data.mResetTimes = false;
		else
			data.mResetTimes = true;
		Interval oldRange = GetCOREInterface()->GetAnimRange();
		if(dlg.LaunchDialog(hDlg,data))
		{
			mWeatherData = data;
			SetWeatherFileName(data.mFileName);
		}
		else //set the range back we may have changed it!
		{
			Interval currentRange = GetCOREInterface()->GetAnimRange();
			if(currentRange.Start()!=oldRange.Start()||currentRange.End()!= oldRange.End())
				GetCOREInterface()->SetAnimRange(oldRange);
		}

	}
}

void NatLightAssembly::GetAltitudeAzimuth(TimeValue t,float &altitude, float &azimuth) 
{
	Interval inter = FOREVER;
	SunMaster* themaster = FindMaster();
	if(themaster)
	{
		Point2 val = themaster->GetAzAlt(t,inter);
		azimuth = (val.x);
		altitude = (val.y);  
	}
}

void NatLightAssembly::GetAltAz(TimeValue t,float &altitude, float &azimuth) 
{
	Interval inter = FOREVER;
	SunMaster* themaster = FindMaster();
	if(themaster)
	{
		Point2 val = themaster->GetAzAlt(t,inter);
		azimuth = (val.x);
		altitude = (val.y);  
	}
}


//the following defines are character Source and Uncertaintiy bits as defined in the pdf found below.
// Note if the pdf moves, do a search on weather data epw and you should find the spec.
//http://www.eere.energy.gov/buildings/energyplus/weatherdata_format_def.html
#define DRY_BULB_TEMPERATURE_SOURCE						0
#define DRY_BULB_TEMPERATURE_UNCERTAINTY				1
#define DEW_POINT_TEMPERATURE_SOURCE					2	
#define DEW_POINT_TEMPERATURE_UNCERTAINTY				3
#define GLOBAL_HORIZONTAL_RADIATION_SOURCE				10
#define GLOBAL_HORIZONTAL_RADIATION_UNCERTAINTY			11
#define DIRECT_NORMAL_RADIATION_SOURCE					12	
#define DIRECT_NORMAL_RADIATION_UNCERTAINTY				13
#define DIFFUSE_HORIZONTAL_RADIATION_SOURCE				14
#define DIFFUSE_HORIZONTAL_RADIATION_UNCERTAINTY		15
#define GLOBAL_HORIZONTAL_ILLUMINANCE_SOURCE			16
#define GLOBAL_HORIZONTAL_ILLUMINANCE_UNCERTAINTY		17
#define DIRECT_NORMAL_ILLUMINANCE_SOURCE				18	
#define DIRECT_NORMAL_ILLUMINANCE_UNCERTAINTY			19
#define DIFFUSE_HORIZONTAL_ILLUMINANCE_SOURCE			20
#define DIFFUSE_HORIZONTAL_ILLUMINANCE_UNCERTAINTY		21
#define ZENITH_ILLUMINANCE_SOURCE						22
#define ZENITH_ILLUMINANCE_UNCERTAINTY					23	

bool NatLightAssembly::GetTemps(float &dryBulbTemperature, float &dewPointTemperature)
{
	if(GetDaylightControlType()==IDaylightSystem2::eWeatherFile&&mWeatherFileMissing==false &&mErrorLoadingWeatherFile==false&&mWeatherFileCache.mDataVector.size()>0)
	{
		//returns the nth based upon the time value.
		TimeValue t= GetCOREInterface()->GetTime();
		DaylightWeatherData &data = mWeatherFileCache.GetNthEntry(t);
		//check source and uncertainty, 
		if(data.mDataUncertainty.Length()>=( DEW_POINT_TEMPERATURE_UNCERTAINTY +1))
		{
			if(data.mDataUncertainty[ DRY_BULB_TEMPERATURE_SOURCE]=='?'||data.mDataUncertainty[DEW_POINT_TEMPERATURE_SOURCE]=='?'||
				data.mDataUncertainty[DRY_BULB_TEMPERATURE_UNCERTAINTY] == '0' || data.mDataUncertainty[DEW_POINT_TEMPERATURE_UNCERTAINTY] =='0')
				return false;
		}
		dryBulbTemperature = data.mDryBulbTemperature;
		dewPointTemperature = data.mDewPointTemperature;
		return true;
	}

	return false;
}



bool NatLightAssembly::GetExtraTerrestialRadiation(float &extraterrestrialHorizontalRadiation, float &extraterrestrialDirectNormalRadiation)
{
	if(GetDaylightControlType()==IDaylightSystem2::eWeatherFile&&mWeatherFileMissing==false &&mErrorLoadingWeatherFile==false&&mWeatherFileCache.mDataVector.size()>0)
	{
		//returns the nth based upon the time value.
		TimeValue t= GetCOREInterface()->GetTime();
		DaylightWeatherData &data = mWeatherFileCache.GetNthEntry(t);
		return GetExtraTerrestialRadiationLocal(data,extraterrestrialHorizontalRadiation,extraterrestrialDirectNormalRadiation);
	}
	return false;
}


bool NatLightAssembly::GetRadiation(float &globalHorizontalRadiation, float &directNormalRadiation,
											 float &diffuseHorizontalRadiation)
{
	if(GetDaylightControlType()==IDaylightSystem2::eWeatherFile&&mWeatherFileMissing==false &&mErrorLoadingWeatherFile==false&&mWeatherFileCache.mDataVector.size()>0)
	{
		//returns the nth based upon the time value.
		TimeValue t= GetCOREInterface()->GetTime();
		DaylightWeatherData &data = mWeatherFileCache.GetNthEntry(t);
		return GetRadiationLocal(data,globalHorizontalRadiation,directNormalRadiation,
								diffuseHorizontalRadiation);
	}
	return false;
}

bool NatLightAssembly::GetIlluminance(float &globalHorizontalIlluminance, float &directNormalIlluminance,
											 float &diffuseHorizontalIlluminance, float &zenithLuminance)
{
	if(GetDaylightControlType()==IDaylightSystem2::eWeatherFile&&mWeatherFileMissing==false &&mErrorLoadingWeatherFile==false&&mWeatherFileCache.mDataVector.size()>0)
	{
		//returns the nth based upon the time value.
		TimeValue t= GetCOREInterface()->GetTime();
		DaylightWeatherData &data = mWeatherFileCache.GetNthEntry(t);
		return GetIlluminanceLocal(data,globalHorizontalIlluminance, directNormalIlluminance,
								diffuseHorizontalIlluminance,zenithLuminance);
	}
	return false;
}




bool NatLightAssembly::GetTempsLocal(DaylightWeatherData &data ,float &dryBulbTemperature, float &dewPointTemperature)
{
	
	dryBulbTemperature = data.mDryBulbTemperature;
	dewPointTemperature = data.mDewPointTemperature;
	//don't check source and uncertainty for temperatures.. but check the values.. it seems if they can be '99' as an invalid value.
	if(dewPointTemperature>98.0f)
		return false;
	return true;
}

bool NatLightAssembly::GetExtraTerrestialRadiationLocal(DaylightWeatherData &data ,float &extraterrestrialHorizontalRadiation, float &extraterrestrialDirectNormalRadiation)
{
	//check source and uncertainty, but none for extra terrestrial
	extraterrestrialHorizontalRadiation = data.mExtraterrestrialHorizontalRadiation;
	extraterrestrialDirectNormalRadiation =		data.mExtraterrestrialDirectNormalRadiation;
	return true;
}


bool NatLightAssembly::GetRadiationLocal(DaylightWeatherData &data ,float &globalHorizontalRadiation, float &directNormalRadiation,
											 float &diffuseHorizontalRadiation)
{
	//check source and uncertainty, we only check for the directnormal and diffuse since they are key, the others
	//can be uncertain and will be set to -1 if so.
	globalHorizontalRadiation = data.mGlobalHorizontalRadiation;
	directNormalRadiation =  data.mDirectNormalRadiation;
	diffuseHorizontalRadiation = data.mDiffuseHorizontalRadiation;

	if((directNormalRadiation==diffuseHorizontalRadiation&&diffuseHorizontalRadiation ==globalHorizontalRadiation)
		&& (data.mDataUncertainty.Length()))
	{
			return false;
	}

	return true;
}

bool NatLightAssembly::GetIlluminanceLocal(DaylightWeatherData &data,float &globalHorizontalIlluminance, float &directNormalIlluminance,
											 float &diffuseHorizontalIlluminance, float &zenithLuminance)
{
	//check source and uncertainty, we only check for the directnormal and diffuse since they are key, the others
	//can be uncertain and will be set to -1 if so.

	//always set these guys.
	directNormalIlluminance = data.mDirectNormalIlluminance;
	diffuseHorizontalIlluminance = data.mDiffuseHorizontalIlluminance;
	globalHorizontalIlluminance = data.mGlobalHorizontalIlluminance;
	if(directNormalIlluminance== diffuseHorizontalIlluminance&&diffuseHorizontalIlluminance==globalHorizontalIlluminance&&data.mDataUncertainty.Length()>=( DIFFUSE_HORIZONTAL_ILLUMINANCE_UNCERTAINTY  +1))
	{
		return false;
	}

	if(data.mDataUncertainty.Length()>ZENITH_ILLUMINANCE_UNCERTAINTY&&data.mDataUncertainty[ZENITH_ILLUMINANCE_SOURCE]=='?'||data.mDataUncertainty[ZENITH_ILLUMINANCE_UNCERTAINTY]=='0')
		zenithLuminance =  -1.0f;
	else
		zenithLuminance =  data.mZenithLuminance;

	return true;
}


TSTR NatLightAssembly::GetResolvedWeatherFileName()
{

	TSTR filename = GetWeatherFileName();

	MaxSDK::Util::Path path(filename);
	IPathConfigMgr::GetPathConfigMgr()->NormalizePathAccordingToSettings(path);
	//now see if that file exists if not then set a flag so we can possibly give an error message at some point
	IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();
	//just a double check here,
	if (!pathMgr->SearchForExternalFiles(path)) 
			filename = TSTR("");
	else
	{
		filename = path.GetString();
		FILE *file = fopen(filename.data(),"r");
		if(file==NULL)
		{
			filename = TSTR("");
		}
		else
		{
			fclose(file);
		}
	}
	return filename;
}


BOOL NatLightAssembly::GetWeatherFileDST(int month, int day)
{
	if(mWeatherFileCache.mWeatherHeaderInfo.mDaylightSavingsStart.x>0&&
		mWeatherFileCache.mWeatherHeaderInfo.mDaylightSavingsStart.y>0)
	{
		if(month >=mWeatherFileCache.mWeatherHeaderInfo.mDaylightSavingsStart.x &&
			day >= mWeatherFileCache.mWeatherHeaderInfo.mDaylightSavingsStart.y &&
			month <= mWeatherFileCache.mWeatherHeaderInfo.mDaylightSavingsEnd.x &&
			day <= mWeatherFileCache.mWeatherHeaderInfo.mDaylightSavingsEnd.y)
			return TRUE;

	}
	return FALSE;
}

//Render each Frame
void NatLightAssembly::RenderFrame(void *param, NotifyInfo *nInfo)
{
	NatLightAssembly *nLA= static_cast<NatLightAssembly*>(param);
	if(nLA->GetDaylightControlType()== IDaylightSystem2::eWeatherFile&& nLA->mWeatherFileMissing==false &&
		nLA->mErrorLoadingWeatherFile==false)
	{
		TimeValue *t = static_cast<TimeValue*>(nInfo->callParam);
		Interval valid(*t,*t);
		nLA->SetEnergyValuesBasedOnWeatherFile(*t,valid);
	}
}

void NatLightAssembly::SetSunLocationBasedOnWeatherFile(SunMaster *themaster, TimeValue t, Interval &valid)
{
	if(mWeatherFileMissing==false &&mErrorLoadingWeatherFile==false)
	{
		static bool inSet = false;
		if(inSet==false)
		{
			inSet = true;
			//returns the nth based upon the time value.
			DaylightWeatherData &data = mWeatherFileCache.GetNthEntry(t);
			valid = Interval(t,t);		
			SuspendAnimate();
			AnimateOff();
			BOOL dst = GetWeatherFileDST(data.mMonth,data.mDay);
			//okay need to disable the references. this is okay since this function should only
			//be called from within SunMaster::GetValue()
			DisableRefMsgs();
			themaster->SetWeatherMDYHMS(t,data.mMonth,data.mDay,data.mYear,data.mHour,data.mMinute,0,dst);
			EnableRefMsgs();
			ResumeAnimate();
		}
		inSet = false;
	}
}

void NatLightAssembly::SetEnergyValuesBasedOnWeatherFile(TimeValue t, Interval &valid)
{
	if(mWeatherFileMissing==false &&mErrorLoadingWeatherFile==false)
	{
		static bool inSet = false;

		if(inSet==false)
		{
			inSet = true;
			//returns the nth based upon the time value.
			DaylightWeatherData &data = mWeatherFileCache.GetNthEntry(t);
			valid = Interval(t,t);		
			//SunMaster* themaster = FindMaster();
			SuspendAnimate();
			AnimateOff();
			//BOOL dst = GetWeatherFileDST(data.mMonth,data.mDay);
			//themaster->SetWeatherMDYHMS(t,data.mMonth,data.mDay,data.mYear,data.mHour,data.mMinute,0,dst);
			
			//if we have an mr sky, or maybe something else we need to set it here.
			Object* obj = static_cast<Object*>(GetReference(SKY_REF));
			IDaylightControlledLightSystem *dls = dynamic_cast<IDaylightControlledLightSystem*>(obj->GetInterface(DAYLIGHT_CONTROLLED_LIGHT_SYSTEM));
			
			if(dls)
			{
				float temp;
				IDaylightControlledLightSystem::DaylightSimulationParams params;
				if(GetTempsLocal(data,params.mDryBulbTemperature,params.mDewPointTemperature)==false)
				{
					params.mDryBulbTemperature = IDCLS_ILLEGAL_VALUE;
					params.mDewPointTemperature = IDCLS_ILLEGAL_VALUE;
				}
				if(GetIlluminanceLocal(data,temp,params.mDirectNormalIlluminance,params.mDiffuseHorizontalIlluminance,
					params.mZenithLuminance)==false)
				{
					params.mDirectNormalIlluminance = IDCLS_ILLEGAL_VALUE;
					params.mDiffuseHorizontalIlluminance = IDCLS_ILLEGAL_VALUE;
					params.mZenithLuminance = IDCLS_ILLEGAL_VALUE;
				}
				if(GetRadiationLocal(data,temp,params.mDirectNormalIrradiance,params.mDiffuseHorizontalIrradiance)==false)
				{
					//okay if we get radiation as false and so is illuminace then we are hosed.  There are files
					//that don't use the 
					params.mDirectNormalIrradiance = IDCLS_ILLEGAL_VALUE;
					params.mDiffuseHorizontalIrradiance= IDCLS_ILLEGAL_VALUE;
				}
				else
				{
					//convert from radiation to irradiance? TODO figure out what this means.
					//pretty sure it's the same thing.

				}

				dls->SetSimulationParams(params);
			}
			ResumeAnimate();

			inSet = false;
		}
	}

}

// called by SetWeatherFileName and when the mode becomes weathermode, and on postload callback
void NatLightAssembly::SetUpBasedOnWeatherFile()
{

	if(GetDaylightControlType()== IDaylightSystem2::eWeatherFile)
	{
		//double make sure the master is set up, may not be if from post load
		SunMaster* themaster = FindMaster();
		if(themaster)
		{
			themaster->controlledByWeatherFile = true;
			themaster->natLight = this;
		}

		mWeatherFileCache.Clear();
		if(this->mWeatherFileMissing==false)
		{
			if(mWeatherFileMissing==false)
			{
				TSTR filename =GetResolvedWeatherFileName();
				if(filename.Length()>0)
				{
					int numEntries = mWeatherFileCache.ReadWeatherData(filename.data());
					
					numEntries = mWeatherFileCache.FilterEntries(mWeatherData);
					if(numEntries)
					{
						mErrorLoadingWeatherFile = false;
						Interface *ip = GetCOREInterface();
						
						//these params are consistent
						TSTR city =  mWeatherFileCache.mWeatherHeaderInfo.mCity + TSTR(", ") +
						mWeatherFileCache.mWeatherHeaderInfo.mRegion + TSTR(", ") + mWeatherFileCache.mWeatherHeaderInfo.mCountry;
						SuspendAnimate();
						AnimateOff();
						themaster->SetLat(ip->GetTime(),mWeatherFileCache.mWeatherHeaderInfo.mLat);
						themaster->SetLong(ip->GetTime(),mWeatherFileCache.mWeatherHeaderInfo.mLon);
						themaster->SetZone(ip->GetTime(),mWeatherFileCache.mWeatherHeaderInfo.mTimeZone);
						themaster->SetCity(city);
						//nope we set itthemaster->calculateCity(ip->GetTime());
						Interval valid;
						SetSunLocationBasedOnWeatherFile(themaster,ip->GetTime(),valid);
						SetEnergyValuesBasedOnWeatherFile(ip->GetTime(),valid);
						themaster->calculate(ip->GetTime(),FOREVER);
						themaster->ValuesUpdated();
						ResumeAnimate();
					}
					else
						mErrorLoadingWeatherFile = true;
				}
				else
					mWeatherFileMissing = true;
			}
		}
	}
}


//Asset Tracking handling...
class WeatherFileAccessor : public IAssetAccessor	{
public:

	WeatherFileAccessor(NatLightAssembly* assembly) : mNatLightAssembly(assembly) {}

	virtual int GetAssetType() const	{ return IAssetAccessor::kExternalLink; }

	// path accessor functions
	virtual MaxSDK::Util::Path GetPath() const;
	virtual void SetPath(const MaxSDK::Util::Path& aNewPath);

protected:
	NatLightAssembly * mNatLightAssembly;
};

MaxSDK::Util::Path WeatherFileAccessor::GetPath() const	{
	return MaxSDK::Util::Path(mNatLightAssembly->GetWeatherFileName());
}

void WeatherFileAccessor::SetPath(const MaxSDK::Util::Path& aNewPath)	{

	mNatLightAssembly->SetWeatherFileName(aNewPath.GetString());
}

void NatLightAssembly::EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags)	
{
	if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/11/03

	if(flags & FILE_ENUM_ACCESSOR_INTERFACE)	{
		WeatherFileAccessor accessor(this);
		if(!accessor.GetPath().IsEmpty())	{
			IEnumAuxAssetsCallback* callback = static_cast<IEnumAuxAssetsCallback*>(&nameEnum);
			callback->DeclareAsset(accessor);
		}
	}
	else	{
		TSTR fname  = GetWeatherFileName();
		if(fname.isNull()==FALSE&&fname.Length()>0) 
		{
			MaxSDK::Util::Path cachePath(fname);
			IPathConfigMgr::GetPathConfigMgr()->RecordInputAsset(
				cachePath,
				nameEnum, 
				flags);
		}

		ReferenceTarget::EnumAuxFiles( nameEnum, flags ); // LAM - 4/21/03

	}
}