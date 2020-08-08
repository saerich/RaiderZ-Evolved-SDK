/**********************************************************************
 *<
	FILE: ViewportLoader.cpp

	DESCRIPTION:	Viewport Manager for loading up Effects

	CREATED BY:		Neil Hazzard

	HISTORY:		Created:  02/15/02
					

 *>	Copyright (c) 2002, All Rights Reserved.
************************************************************************/

#ifndef	__VIEWPORTLOADER_H__
#define __VIEWPORTLOADER_H__

#include "CustAttrib.h"
#include "IViewportManager.h"


#define VIEWPORTLOADER_CLASS_ID Class_ID(0x5a06293c, 0x30420c1e)
#define PBLOCK_REF 0


enum { viewport_manager_params };  // pblock ID

enum 
{ 
	pb_enabled,
	pb_effect,
	pb_dxStdMat,
	pb_saveFX,	//not used
	pb_saveFX2,
	
};

class ViewportLoader : public CustAttrib, public IViewportShaderManager3
{
public:
	
	int effectIndex;
	IParamBlock2 *pblock;
	IAutoMParamDlg* masterDlg;
	ParamDlg * clientDlg;
	ReferenceTarget * effect;
	ReferenceTarget * oldEffect;
	bool undo;
	HWND mEdit;
	IMtlParams *mparam;

	enum {	get_num_effects2, get_active_effect2,	is_effect_active2, is_manager_active2,
		get_effect_name2,set_effect2,activate_effect2, is_dxStdMat_enabled2, set_dxStdMat2,saveFXfile2, get_EffectName2};

	BEGIN_FUNCTION_MAP
		FN_0(get_num_effects2,		TYPE_INT,  GetNumEffects);
		FN_0(get_active_effect2,		TYPE_REFTARG, GetActiveEffect);
		FN_1(get_effect_name2,		TYPE_STRING, GetEffectName, TYPE_INT);
		FN_1(set_effect2,			TYPE_REFTARG,SetViewportEffect, TYPE_INT);
		VFN_2(activate_effect2,		ActivateEffect,TYPE_MTL,TYPE_BOOL);

		FN_0(is_dxStdMat_enabled2,	TYPE_bool, IsDxStdMtlEnabled);
		VFN_1(set_dxStdMat2,SetDxStdMtlEnabled, TYPE_bool);
		FN_1(saveFXfile2, TYPE_bool, SaveFXFile, TYPE_FILENAME);
		FN_0(get_EffectName2,	TYPE_STRING, GetActiveEffectName);

	END_FUNCTION_MAP

	

	ViewportLoader();
	~ViewportLoader();
	Tab < ClassDesc *> effectsList;

	virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,  PartID& partID,  RefMessage message);
	
	virtual ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);


	int	NumParamBlocks() { return 1; }			
	IParamBlock2* GetParamBlock(int i) { if(i == 0) return pblock; else return NULL;} 
	IParamBlock2* GetParamBlockByID(short id) { if(id == viewport_manager_params ) return pblock; else return NULL;} 

	int NumRefs() { return 2;}
	void SetReference(int i, RefTargetHandle rtarg);
	RefTargetHandle GetReference(int i);
	
	virtual	int NumSubs();
	virtual	Animatable* SubAnim(int i);
	virtual TSTR SubAnimName(int i);

	// Override CustAttrib::SvTraverseAnimGraph
	SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);


	SClass_ID		SuperClassID() {return CUST_ATTRIB_CLASS_ID;}
	Class_ID 		ClassID() {return VIEWPORTLOADER_CLASS_ID;}	
	


	ReferenceTarget *Clone(RemapDir &remap = DefaultRemapDir());
	virtual bool CheckCopyAttribTo(ICustAttribContainer *to) { return true; }
	
	virtual TCHAR* GetName(){ return GetString(IDS_VIEWPORT_MANAGER);}
	void GetClassName(TSTR& s){s = TSTR("DirectX Manager");}
		
	void DeleteThis() { delete this;}

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);



	int FindManagerPosition();
	ClassDesc * FindandLoadDeferedEffect(ClassDesc * defered);
	void LoadEffectsList();
	ClassDesc* GetEffectCD(int i);
	int FindEffectIndex(ReferenceTarget * e);
	
	int NumShaders();
	void LoadEffect(ClassDesc * pd);
	void SwapEffect(ReferenceTarget *e);

	int GetNumEffects();
	ReferenceTarget* GetActiveEffect();

	TCHAR * GetEffectName(int i);
	ReferenceTarget * SetViewportEffect(int i);
	void ActivateEffect (MtlBase * mtl, BOOL state);

	bool IsDxStdMtlEnabled();
	bool SaveFXFile(TCHAR * fileName);
	void SetDxStdMtlEnabled(bool state);
	TCHAR * GetActiveEffectName();

	BaseInterface* GetInterface(Interface_ID id) ;

public:
	virtual bool IsCurrentEffectEnabled();
	virtual bool SetCurrentEffectEnabled(bool enabled);
	virtual bool IsDxStdMtlSupported();
	virtual bool IsSaveFxSupported();
	virtual bool IsCurrentEffectSupported();
	virtual MtlBase* FindOwnerMaterial();

private:
	friend class EffectPBAccessor;
	friend class EffectsDlgProc;

	void HandleEffectIndexChanged(TimeValue t, int effectIndex);
	void HandleEffectStateChanged(TimeValue t, bool enabled);
	void HandleDxStdMtlChanged(TimeValue t, bool enabled);
	void HandleSaveFxFile(TimeValue t, const TCHAR* fileName);

	void EnableDlgItem(INT dlgItem, BOOL enabled);
};


class EffectsDlgProc : public ParamMap2UserDlgProc 
{
	public:
		ViewportLoader *vl;

		EffectsDlgProc() {}
		EffectsDlgProc(ViewportLoader *loader) { vl = loader; }

		INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() { }


};

// the following is for the Undo system.....



class AddEffectRestore : public RestoreObj {
	public:
		ViewportLoader *vpLoader;
		SingleRefMaker  undoEffect;
		SingleRefMaker  redoEffect;

		AddEffectRestore(ViewportLoader *c,ReferenceTarget *oldEffect, ReferenceTarget * newEffect) 
		{
			vpLoader = c;
			undoEffect.SetRef(oldEffect);
			redoEffect.SetRef(newEffect);
			
		}   		
		void Restore(int isUndo) 
		{
			vpLoader->SwapEffect(undoEffect.GetRef());
			
		}
		void Redo()
		{
			vpLoader->SwapEffect(redoEffect.GetRef());

		}		
		void EndHold() 
		{ 
			vpLoader->ClearAFlag(A_HELD);
		}
	};


#endif