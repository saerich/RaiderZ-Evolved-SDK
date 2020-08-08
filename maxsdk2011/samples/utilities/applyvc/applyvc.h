/**********************************************************************
 *<
	FILE:		    ApplyVC.h
	DESCRIPTION:	Vertex Color Utility
	CREATED BY:		Christer Janson
	HISTORY:		Created Monday, June 02, 1997

 *>	Copyright (c) 1997 Kinetix, All Rights Reserved.
 **********************************************************************/

#ifndef __APPLYVC__H
#define __APPLYVC__H

#include "Max.h"
#include "notify.h"
#include "resource.h"
#include "utilapi.h"
#include "istdplug.h"
#include "EvalCol.h"

//classID defined in istdplug.h

//The former ApplyVC modifier is no longer used by the Apply Vertex Color Utility.
//But, older files will still load if they have an object with an ApplyVC modifier.
#ifdef USE_ORIGINAL_VERTEX_PAINT
#define APPLYVC_UTIL_CLASS_ID	Class_ID(0x6e989195, 0x5dfb41b7)
#endif	// use_original_vertex_paint
#define APPLYVC_MOD_CLASS_ID	Class_ID(0x104170cc, 0x66373204)

class ModInfo {
public:
	Modifier*	mod;
	INodeTab	nodes;
};

typedef Tab<ModInfo*> ModInfoTab;

class ApplyVCUtil : public UtilityObj
#ifndef USE_ORIGINAL_VERTEX_PAINT
					, public IAssignVertexColors_R7
#endif	// use_original_vertex_paint
{
	public:
		IUtil		*iu;
		Interface	*ip;
		HWND		hPanel;
#ifndef USE_ORIGINAL_VERTEX_PAINT
		ICustEdit*			iMapChanEdit;
		ISpinnerControl*	iMapChanSpin;
		int			mapChannelSpin; //the current value in the map channel spinner
		Modifier* recentMod; //most recently used modifier
#endif	// use_original_vertex_paint
        
        // [dl | 18feb2002] This structure encapsulates all the UI options.
        EvalVCOptions currentOptions;

        static void NotifyRefreshUI(void* param, NotifyInfo* info);

		ApplyVCUtil();
		~ApplyVCUtil();

		void	BeginEditParams(Interface *ip,IUtil *iu);
		void	EndEditParams(Interface *ip,IUtil *iu);
		void	DeleteThis() {}

		void	Init(HWND hWnd);
		void	Destroy(HWND hWnd);

#ifndef USE_ORIGINAL_VERTEX_PAINT
		//-- From IAssignVertexColors
		int		ApplyNodes( Tab<INode*>* nodes, ReferenceTarget* dest=NULL );
		void	GetOptions( IAssignVertexColors::Options& options );
		void	SetOptions( IAssignVertexColors::Options& options );

		//-- From IAssignVertexColors_R7
		virtual void	GetOptions2( Options2& options );
		virtual void	SetOptions2( Options2& options );

		virtual BaseInterface*	GetInterface(Interface_ID id);
		//{ if (id == FPINTERFACEDESC_ID) return this; else return UtilityObj::GetInterface(id); }

		//-- function map for IAssignVertexColors
		enum { fnIdApplyNodes };
		BEGIN_FUNCTION_MAP
			FN_2( fnIdApplyNodes, TYPE_INT, ApplyNodes, TYPE_INODE_TAB, TYPE_REFTARG );
		END_FUNCTION_MAP
#endif	// !use_original_vertex_paint

		void	ApplySelected();
#ifndef USE_ORIGINAL_VERTEX_PAINT
		BOOL	ApplyNode(INode* node, IVertexPaint* ivertexPaint = NULL, bool noUpdate = false);
		void	EditColors();
#else	// use_original_vertex_paint
		BOOL	ApplyNode(INode* node, bool noUpdate = false);
		Modifier*	GetModifier(INode* node, Class_ID modCID);
		void	UpdateAll();
		BOOL	UpdateAllEnum(INode* node);

		void	CheckForAndMakeUnique();
		void	CheckAllNodesEnum(INode* node, ModInfoTab& modTab);
		BOOL	MakeUnique(ModInfo* mh);
#endif	// use_original_vertex_paint

		void	UpdateUI();

        // Updates the enabled/disabled states of the checkboxes based on the current options
        void    UpdateEnableStates();

		void	SaveOptions();
		void	LoadOptions();
#ifndef USE_ORIGINAL_VERTEX_PAINT

		//-- helper methods
		Modifier*	GetModifier(INode* node, Class_ID modCID);
		void	AddModifier(Tab<INode*>& nodes, Modifier* mod);
		void	DeleteModifier( INode* nodes, Modifier* mod );
		BOOL	GetChannelName( int index, TSTR& name );
protected:
		void    UpdateRenderOptionControls();
		void    UpdateGatheringControls();
#endif	// !use_original_vertex_paint
};

class ApplyVCMod : public Modifier {	
	public:
		static IObjParam*	ip;
		HWND			hPanel;
		ColorTab		mixedVertexColors;
		//VertexColorTab	vertexColors;
		FaceColorTab	faceColors;
		Interval		iValid;

		ApplyVCMod(BOOL create);
		void		InitControl(ModContext &mc,TriObject *obj,int type,TimeValue t);

		// From Animatable
		void		DeleteThis();
		void		GetClassName(TSTR& s);
		Class_ID	ClassID();
		void		BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void		EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);
		TCHAR*		GetObjectName();
		CreateMouseCallBack*	GetCreateMouseCallBack();
		BOOL		CanCopyAnim();
		BOOL		DependOnTopology(ModContext &mc);

		ChannelMask	ChannelsUsed();
		ChannelMask	ChannelsChanged();
		Class_ID	InputType();
		void		ModifyTriObject(TriObject* tobj, TimeValue t);
		void		ModifyPolyObject(PolyObject* pPolyObj, TimeValue t);
		void		ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval	LocalValidity(TimeValue t);
		Interval  GetValidity(TimeValue t) { return iValid; }

		int			NumRefs();
		RefTargetHandle	GetReference(int i);
		void		SetReference(int i, RefTargetHandle rtarg);

		int			NumSubs();
		Animatable* SubAnim(int i);
		TSTR		SubAnimName(int i);

		RefTargetHandle	Clone(RemapDir& remap);
		RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

		IOResult	Load(ILoad *iload);
		IOResult	Save(ISave *isave);

		void		ResetColTab();
//		void		SetColors(VertexColorTab& colorTab);
		void		SetColors(FaceColorTab& colorTab);
		void		SetMixedColors(ColorTab& colorTab);
};

extern ClassDesc*	GetApplyVCUtilDesc();
extern ClassDesc*	GetApplyVCModDesc();
extern HINSTANCE	hInstance;
extern TCHAR*		GetString(int id);
extern TriObject*	GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);

#endif // __APPLYVC__H
