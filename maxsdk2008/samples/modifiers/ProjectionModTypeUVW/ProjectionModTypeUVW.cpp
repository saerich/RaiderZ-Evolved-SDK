/**********************************************************************
 *<
	FILE:			ProjectionModTypeUVW.cpp
	DESCRIPTION:	Projection Modifier Type UVW
	CREATED BY:		Michael Russo
	HISTORY:		Created 03-23-2004

 *>	Copyright (c) 2004 Discreet, All Rights Reserved.
 **********************************************************************/

#include "ProjectionModTypeUVW.h"
#include "modstack.h"
#include "containers\array.h"

//=============================================================================
//
//	Class ProjectionModTypeUVW
//
//=============================================================================


//--- ClassDescriptor and class vars ---------------------------------

Interface* ProjectionModTypeUVW::mpIP = NULL;

class ProjectionModTypeUVWClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new ProjectionModTypeUVW(!loading);}
	const TCHAR *	ClassName() { return GetString(IDS_PROJECTIONMODTYPEUVW_CLASS_NAME); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID		ClassID() { return PROJECTIONMODTYPEUVW_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_PROJECTIONMODTYPEUVW_CATEGORY);}

	const TCHAR*	InternalName() { return _T("ProjectionModTypeUVW"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }		// returns owning module handle
};

static ProjectionModTypeUVWClassDesc theProjectionModTypeUVWClassDesc;
extern ClassDesc2* GetProjectionModTypeUVWDesc() {return &theProjectionModTypeUVWClassDesc;}


class ProjectionModTypeUVWPBAccessor : public PBAccessor
{ 
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		ProjectionModTypeUVW* p = (ProjectionModTypeUVW*)owner;
		if( !p )
			return;
		switch (id) {
			case ProjectionModTypeUVW::pb_name:
				if( p->mpPMod )
					p->mpPMod->UpdateProjectionTypeList();
				break;
			case ProjectionModTypeUVW::pb_sourceMapChannel:
				if( (v.i == VERTEX_CHANNEL_NUM) || ((v.i >= -2) && (v.i <= 100)) )
					p->SetSourceMapChannel(v.i);
				break;
			case ProjectionModTypeUVW::pb_targetMapChannel:
				if( (v.i == VERTEX_CHANNEL_NUM) || ((v.i >= -2) && (v.i <= 100)) )
					p->SetTargetMapChannel(v.i);
				break;
		}
		p->MainPanelUpdateUI();
	}

	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		ProjectionModTypeUVW* p = (ProjectionModTypeUVW*)owner;
		if( !p )
			return;
		switch (id) {
			case ProjectionModTypeUVW::pb_sourceMapChannel:
				v.i = p->GetSourceMapChannel();
				break;
			case ProjectionModTypeUVW::pb_targetMapChannel:
				v.i = p->GetTargetMapChannel();
				break;
		}
	}
};

static ProjectionModTypeUVWPBAccessor theProjectionModTypeUVW_accessor;

static ParamBlockDesc2 theProjectionModTypeUVWBlockDesc (
	ProjectionModTypeUVW::pb_params, _T("ProjectionModTypeUVW"),  0, &theProjectionModTypeUVWClassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, 0,

	//rollout
	IDD_PROJECTUVW, IDS_PROJECTIONMODTYPEUVW_CLASS_NAME, 0,0, NULL,

	// Projection Name
	ProjectionModTypeUVW::pb_name,	_T("name"), TYPE_STRING, 0, IDS_PARAM_NAME,
		p_ui,  TYPE_EDITBOX,  IDC_EDIT_NAME,
		p_accessor,		&theProjectionModTypeUVW_accessor,
		end,

	// Holder
	ProjectionModTypeUVW::pb_holderName,	_T("holderName"), TYPE_STRING, 0, IDS_PARAM_HOLDER_NAME,
		p_ui,  TYPE_EDITBOX,  IDC_EDIT_NAME_HOLDER,
		p_accessor,		&theProjectionModTypeUVW_accessor,
		end,
	ProjectionModTypeUVW::pb_holderAlwaysUpdate, _T("alwaysUpdate"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PARAM_HOLDER_ALWAYS_UPDATE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_ALWAYS_UPDATE,
		p_accessor,		&theProjectionModTypeUVW_accessor,
		end,
	ProjectionModTypeUVW::pb_holderCreateNew, _T("createNewHolder"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PARAM_HOLDER_CREATE_NEW,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_NEW_HOLDER,
		p_accessor,		&theProjectionModTypeUVW_accessor,
		end,

	// Source
	ProjectionModTypeUVW::pb_sourceMapChannel,	_T("sourceMapChannel"), TYPE_INT, P_TRANSIENT, IDS_PARAM_MAPCHANNEL,
		p_default,	1,
		p_range,	VERTEX_CHANNEL_NUM, 99,
		p_accessor,		&theProjectionModTypeUVW_accessor,
		end,

	// Target
	ProjectionModTypeUVW::pb_targetSameAsSource, _T("sameAsSource"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PARAM_TARGET_SAME_AS_SOURCE,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_TARGET_SAME_AS_SOURSE,
		p_accessor,		&theProjectionModTypeUVW_accessor,
		end,

	ProjectionModTypeUVW::pb_targetMapChannel,	_T("targetMapChannel"), TYPE_INT, P_TRANSIENT, IDS_PARAM_TARGET_MAPCHANNEL,
		p_default,	1,
		p_range,	VERTEX_CHANNEL_NUM, 99,
		p_accessor,		&theProjectionModTypeUVW_accessor,
		end,

	// Misc
	ProjectionModTypeUVW::pb_projectMaterialIDs, _T("projectMaterialIDs"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PARAM_MATERIAL_IDS,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_MATERIAL_IDS,
		p_accessor,		&theProjectionModTypeUVW_accessor,
		end,
	ProjectionModTypeUVW::pb_sameTopology, _T("sameTopology"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PARAM_SAME_TOPOLOGY,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SAME_TOPOLOGY,
		p_accessor,		&theProjectionModTypeUVW_accessor,
		end,
	ProjectionModTypeUVW::pb_ignoreBackfacing, _T("ignoreBackfacing"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PARAM_IGNORE_BACKFACING,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_IGNOREBACKFACES,
		end,

	ProjectionModTypeUVW::pb_testSeams, _T("testSeams"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PARAM_TESTSEAMS,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_TESTSEAMS,
		end,

	ProjectionModTypeUVW::pb_checkEdgeRatios, _T("checkEdgeRatios"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PARAM_CHECKEDGERATIOS,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_EDGERATIOS,
		end,

	ProjectionModTypeUVW::pb_weldUVs, _T("weldUVs"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PARAM_WELDUVS,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_WELD,
		end,

	ProjectionModTypeUVW::pb_weldUVsThreshold,	_T("weldUVsThreshold"), TYPE_FLOAT, 0, IDS_PARAM_WELDUVSTHRESHOLD,
		p_default,	5.0f,
		p_range,	0.0f, 100.0f,
		p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_WELD_THRESHOLD, IDC_WELD_THRESHOLD_SPIN, 0.1f,
		end,

	ProjectionModTypeUVW::pb_edgeRatiosThreshold,	_T("edgeRatioThreshold"), TYPE_FLOAT, 0, IDS_PARAM_EDGERATIOTHRESHOLD,
		p_default,	0.1f,
		p_range,	0.0f, 100.0f,
		p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_E_THRESHOLD, IDC_E_THRESHOLD_SPIN, 0.1f,
		end,


	end
);

class MainPanelDlgProc : public ParamMap2UserDlgProc {
  public:
	ProjectionModTypeUVW* mpParent;
	BOOL initialized; //set to true after an init dialog message
	MainPanelDlgProc( ProjectionModTypeUVW* parent ) { this->mpParent=parent; initialized=FALSE; }
	void DeleteThis() { delete this; }

	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
		case WM_INITDIALOG: //called after BeginEditParams whenever rollout is displayed
			mpParent->MainPanelInitDialog(hWnd);
			initialized = TRUE;
			break;
		case WM_DESTROY: //called from EndEditParams
			mpParent->MainPanelDestroy(hWnd);
			initialized = FALSE;
			break;
		case CC_SPINNER_CHANGE:
			if ((LOWORD(wParam) != IDC_E_THRESHOLD_SPIN) && (LOWORD(wParam) != IDC_WELD_THRESHOLD_SPIN))
			{			
				mpParent->MainPanelUpdateMapChannels();
				mpParent->MainPanelUpdateUI();
			}
			break;
		case WM_COMMAND:
			switch( LOWORD(wParam) ) {
			case IDC_RADIO_SOURCE_MAPCHANNEL: 
			case IDC_RADIO_SOURCE_VERTEXALPHA: 
			case IDC_RADIO_SOURCE_VERTEXILLUM: 
			case IDC_RADIO_SOURCE_VERTEXCOLOR: 
			case IDC_RADIO_SOURCE_VERTEX: 
			case IDC_RADIO_TARGET_MAPCHANNEL: 
			case IDC_RADIO_TARGET_VERTEXALPHA: 
			case IDC_RADIO_TARGET_VERTEXILLUM: 
			case IDC_RADIO_TARGET_VERTEXCOLOR: 
			case IDC_RADIO_TARGET_VERTEX: 
				mpParent->MainPanelUpdateMapChannels();
				mpParent->MainPanelUpdateUI();
				break;
			}
			break;
		default:
			return FALSE;
	  }
	  return TRUE;
	}
};

//--- ProjectionModTypeUVW methods -------------------------------

ProjectionModTypeUVW::ProjectionModTypeUVW(BOOL create)
{
	mpIP = GetCOREInterface();

	mpPMod = NULL;
	mpPBlock = NULL;
	miIndex = 1;

	mhPanel = NULL;
	mbSuspendPanelUpdate = false;
	mbInModifyObject = false;

	miSourceMapChannel = 1;
	miTargetMapChannel = 1;

	mbEnabled = false;
	mbEditing = false;

	mName.printf( GetString(IDS_PROJECTIONMODTYPEUVW_INITIALNAME), 1 );

	mpPBlock = NULL; 
	theProjectionModTypeUVWClassDesc.MakeAutoParamBlocks( this );
	assert( mpPBlock );


}

ProjectionModTypeUVW::~ProjectionModTypeUVW() 
{

}

RefTargetHandle ProjectionModTypeUVW::Clone( RemapDir &remap )
{
	ProjectionModTypeUVW* newmod = new ProjectionModTypeUVW(FALSE);	
	newmod->ReplaceReference(PBLOCK_REF,remap.CloneRef(mpPBlock));

	// russom - August 15, 2005 - 643944
	newmod->miIndex = miIndex;
	newmod->miSourceMapChannel = miSourceMapChannel;
	newmod->miTargetMapChannel = miTargetMapChannel;
	newmod->mName = mName;

	BaseClone(this, newmod, remap);
	return(newmod);
}

#define SOURCE_MAPCHANNEL_CHUNK	0x100
#define TARGET_MAPCHANNEL_CHUNK	0x110

IOResult ProjectionModTypeUVW::Load(ILoad *pILoad)
{
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=pILoad->OpenChunk())) {
		switch(pILoad->CurChunkID())  {
			case SOURCE_MAPCHANNEL_CHUNK:
				res=pILoad->Read( &miSourceMapChannel, sizeof(int), &nb );
				break;
			case TARGET_MAPCHANNEL_CHUNK:
				res=pILoad->Read( &miTargetMapChannel, sizeof(int), &nb );
				break;
		}
		pILoad->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}

	return IO_OK;
}

IOResult ProjectionModTypeUVW::Save(ISave *pISave)
{
	ULONG nb;
	pISave->BeginChunk(SOURCE_MAPCHANNEL_CHUNK);
	pISave->Write( &miSourceMapChannel, sizeof(int), &nb );
	pISave->EndChunk();

	pISave->BeginChunk(TARGET_MAPCHANNEL_CHUNK);
	pISave->Write( &miTargetMapChannel, sizeof(int), &nb );
	pISave->EndChunk();

	return IO_OK;
}

void ProjectionModTypeUVW::BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev)
{
	TCHAR *szName = NULL;
	mpPBlock->GetValue(pb_name, 0, szName, FOREVER);
	if( !szName )
		SetInitialName(miIndex);

	theProjectionModTypeUVWClassDesc.BeginEditParams(ip, this, flags, prev);

	ParamMap2UserDlgProc* dlgProc;
	dlgProc = new MainPanelDlgProc(this);
	theProjectionModTypeUVWBlockDesc.SetUserDlgProc( pb_params, dlgProc );
}

void ProjectionModTypeUVW::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next)
{
	theProjectionModTypeUVWClassDesc.EndEditParams(ip, this, flags, next);
}

void ProjectionModTypeUVW::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *inode, IProjectionModData *pPModData)
{
	if( mbInModifyObject ) 
		return;

	mbInModifyObject = true;



	BOOL bAlwaysUpdate = FALSE;
	if( mpPBlock )
		mpPBlock->GetValue(pb_holderAlwaysUpdate, 0, bAlwaysUpdate, FOREVER);

	if( bAlwaysUpdate && mpPMod ) {
		BitArray sel;
		for( int s=0; s<mpPMod->NumGeomSels(); s++ ) {
			for( int n=0; n<mpPMod->NumGeomSelNodes(s); n++ ) {
				bool bUseSOSel = pPModData->GetGeomSel(s, sel);
				INode *pNodeTarget = mpPMod->GetGeomSelNode(s, n );
				ProjectToTarget( NULL, os, pNodeTarget, bUseSOSel?&sel:NULL, pPModData );
			}
		}
	}
	mbInModifyObject = false;
}


void ProjectionModTypeUVW::SetInitialName(int iIndex)
{
	miIndex = iIndex;
	TSTR strName;
	strName.printf( GetString(IDS_PROJECTIONMODTYPEUVW_INITIALNAME), iIndex );
	SetName( strName.data() );

	strName.printf( GetString(IDS_PROJECTIONHOLDER_INITIALNAME), iIndex );
	if( mpPBlock )
		mpPBlock->SetValue(pb_holderName, 0, strName.data());
}

TCHAR *ProjectionModTypeUVW::GetName()
{
	if( mpPBlock ) {
		TCHAR *szName = NULL;
		mpPBlock->GetValue(pb_name, 0, szName, FOREVER);
		if( szName )
			return szName;
	}

	return mName.data();
}

void ProjectionModTypeUVW::SetName(TCHAR *name) 
{ 
	if( name ) {
		mName = name; 
		if( mpPBlock )
			mpPBlock->SetValue(pb_name, 0, mName);
	}
}

bool ProjectionModTypeUVW::CanProject(Tab<INode*> &tabSourceNodes, int iSelIndex, int iNodeIndex)
{
	if( !mpPMod || (mpPMod->NumObjects() <= 0) )
		return false;

	return true;
}

void ProjectionModTypeUVW::Project(Tab<INode*> &tabSourceNodes, int iSelIndex, int iNodeIndex)
{
	if( !CanProject(tabSourceNodes, iSelIndex, iNodeIndex) )
		return;

	Interface *ip = GetCOREInterface();

	bool bUseSOSel = false;
	BitArray sel;
	for( int i=0; i<tabSourceNodes.Count(); i++ ) {

		if( tabSourceNodes[i] == NULL )
			continue;

		if( iSelIndex == -1 ) {
			// Project to everything
			for( int s=0; s<mpPMod->NumGeomSels(); s++ ) {
				for( int n=0; n<mpPMod->NumGeomSelNodes(s); n++ ) {
					bUseSOSel = GetSOSelData(tabSourceNodes[i], s, sel);
					INode *pNodeTarget = mpPMod->GetGeomSelNode(s, n );
					ProjectToTarget( tabSourceNodes[i], NULL, pNodeTarget, bUseSOSel?&sel:NULL, NULL );
				}
			}
		}
		else if( iNodeIndex == -1 ) {
			// Project to the nodes of a geometry selection.
			for( int n=0; n<mpPMod->NumGeomSelNodes(iSelIndex); n++ ) {
				bUseSOSel = GetSOSelData(tabSourceNodes[i], iSelIndex, sel);
				INode *pNodeTarget = mpPMod->GetGeomSelNode(iSelIndex, n );
				ProjectToTarget( tabSourceNodes[i], NULL, pNodeTarget, bUseSOSel?&sel:NULL, NULL  );
			}
		}
		else {
			// Project to single node
			bUseSOSel = GetSOSelData(tabSourceNodes[i], iSelIndex, sel);
			INode *pNodeTarget = mpPMod->GetGeomSelNode(iSelIndex, iNodeIndex );
			ProjectToTarget( tabSourceNodes[i], NULL, pNodeTarget, bUseSOSel?&sel:NULL, NULL  );
		}
	}
}
void ProjectionModTypeUVW::MainPanelInitDialog( HWND hWnd ) 
{
	mhPanel = hWnd;

	mSpinnerSourceMapChannel = SetupIntSpinner( mhPanel, IDC_MAPCHANNELSPIN, IDC_MAPCHANNEL, 1, 100, (miSourceMapChannel > 0)?miSourceMapChannel:1 );
	mSpinnerTargetMapChannel = SetupIntSpinner( mhPanel, IDC_MAPCHANNELSPIN_TARGET, IDC_MAPCHANNEL_TARGET, 1, 100, (miTargetMapChannel > 0)?miTargetMapChannel:1 );

	MainPanelUpdateUI();
}

void ProjectionModTypeUVW::MainPanelDestroy( HWND hWnd ) 
{
	mhPanel = NULL;
	ReleaseISpinner(mSpinnerSourceMapChannel);
	ReleaseISpinner(mSpinnerTargetMapChannel);
}

void ProjectionModTypeUVW::MainPanelUpdateUI()
{
	if( (mhPanel == NULL) || mbSuspendPanelUpdate ) 
		return;

	mbSuspendPanelUpdate = true;

	// Holder Group
	BOOL bCreateNew = FALSE;
	mpPBlock->GetValue(pb_holderCreateNew, 0, bCreateNew, FOREVER);
	BOOL bSameTopo = FALSE;
	mpPBlock->GetValue(pb_sameTopology, 0, bSameTopo, FOREVER);

	EnableWindow(GetDlgItem(mhPanel,IDC_ALWAYS_UPDATE), !bCreateNew && bSameTopo );

	// Source Group
	CheckDlgButton( mhPanel, IDC_RADIO_SOURCE_MAPCHANNEL, (miSourceMapChannel > 0) );
	CheckDlgButton( mhPanel, IDC_RADIO_SOURCE_VERTEXCOLOR, (miSourceMapChannel == 0) );	
	CheckDlgButton( mhPanel, IDC_RADIO_SOURCE_VERTEXILLUM, (miSourceMapChannel == -1) );
	CheckDlgButton( mhPanel, IDC_RADIO_SOURCE_VERTEXALPHA, (miSourceMapChannel == -2) );
	CheckDlgButton( mhPanel, IDC_RADIO_SOURCE_VERTEX, (miSourceMapChannel == VERTEX_CHANNEL_NUM) );

	mSpinnerSourceMapChannel->Enable( miSourceMapChannel>0 );

	// Target Group
	BOOL bValue = FALSE;
	mpPBlock->GetValue(pb_targetSameAsSource, 0, bValue, FOREVER);
	EnableWindow(GetDlgItem(mhPanel,IDC_RADIO_TARGET_MAPCHANNEL), !bValue);
	EnableWindow(GetDlgItem(mhPanel,IDC_RADIO_TARGET_VERTEXCOLOR), !bValue);
	EnableWindow(GetDlgItem(mhPanel,IDC_RADIO_TARGET_VERTEXILLUM), !bValue);
	EnableWindow(GetDlgItem(mhPanel,IDC_RADIO_TARGET_VERTEXALPHA), !bValue);
	EnableWindow(GetDlgItem(mhPanel,IDC_RADIO_TARGET_VERTEX), !bValue);

	mSpinnerTargetMapChannel->Enable( !bValue );

	if( !bValue ) {
		CheckDlgButton( mhPanel, IDC_RADIO_TARGET_MAPCHANNEL, (miTargetMapChannel > 0) );
		CheckDlgButton( mhPanel, IDC_RADIO_TARGET_VERTEXCOLOR, (miTargetMapChannel == 0) );	
		CheckDlgButton( mhPanel, IDC_RADIO_TARGET_VERTEXILLUM, (miTargetMapChannel == -1) );
		CheckDlgButton( mhPanel, IDC_RADIO_TARGET_VERTEXALPHA, (miTargetMapChannel == -2) );
		CheckDlgButton( mhPanel, IDC_RADIO_TARGET_VERTEX, (miTargetMapChannel == VERTEX_CHANNEL_NUM) );
	}

	mbSuspendPanelUpdate = false;
}

void ProjectionModTypeUVW::MainPanelUpdateMapChannels()
{
	if( mhPanel == NULL ) 
		return;

	// Source Group
	if( IsDlgButtonChecked(mhPanel, IDC_RADIO_SOURCE_MAPCHANNEL) == BST_CHECKED )
		miSourceMapChannel = mSpinnerSourceMapChannel->GetIVal();
	else if( IsDlgButtonChecked(mhPanel, IDC_RADIO_SOURCE_VERTEXCOLOR) == BST_CHECKED )
		miSourceMapChannel = 0;
	else if( IsDlgButtonChecked(mhPanel, IDC_RADIO_SOURCE_VERTEXILLUM) == BST_CHECKED )
		miSourceMapChannel = -1;
	else if( IsDlgButtonChecked(mhPanel, IDC_RADIO_SOURCE_VERTEXALPHA) == BST_CHECKED )
		miSourceMapChannel = -2;
	else if( IsDlgButtonChecked(mhPanel, IDC_RADIO_SOURCE_VERTEX) == BST_CHECKED )
		miSourceMapChannel = VERTEX_CHANNEL_NUM;

	// Target Group
	if( IsDlgButtonChecked(mhPanel, IDC_TARGET_SAME_AS_SOURSE) == BST_CHECKED )
		miTargetMapChannel = miSourceMapChannel;
	else if( IsDlgButtonChecked(mhPanel, IDC_RADIO_TARGET_MAPCHANNEL) == BST_CHECKED )
		miTargetMapChannel = mSpinnerTargetMapChannel->GetIVal();
	else if( IsDlgButtonChecked(mhPanel, IDC_RADIO_TARGET_VERTEXCOLOR) == BST_CHECKED )
		miTargetMapChannel = 0;
	else if( IsDlgButtonChecked(mhPanel, IDC_RADIO_TARGET_VERTEXILLUM) == BST_CHECKED )
		miTargetMapChannel = -1;
	else if( IsDlgButtonChecked(mhPanel, IDC_RADIO_TARGET_VERTEXALPHA) == BST_CHECKED )
		miTargetMapChannel = -2;
	else if( IsDlgButtonChecked(mhPanel, IDC_RADIO_TARGET_VERTEX) == BST_CHECKED )
		miTargetMapChannel = VERTEX_CHANNEL_NUM;
}

bool ProjectionModTypeUVW::GetAlwaysUpdate()
{
	BOOL bCreateNew = FALSE;
	mpPBlock->GetValue(pb_holderCreateNew, 0, bCreateNew, FOREVER);
	BOOL bSameTopo = FALSE;
	mpPBlock->GetValue(pb_sameTopology, 0, bSameTopo, FOREVER);
	BOOL bAlwaysUpdate = FALSE;
	mpPBlock->GetValue(pb_holderAlwaysUpdate, 0, bAlwaysUpdate, FOREVER);

	return !bCreateNew && bSameTopo && bAlwaysUpdate;
}

bool ProjectionModTypeUVW::GetSOSelData( INode *pSourceNode, int iSelIndex, BitArray &sel )
{
	bool bUseSOSel = false;
	IProjectionModData *pModData = mpPMod->GetProjectionModData(pSourceNode);
	if( pModData )
		bUseSOSel = pModData->GetGeomSel(iSelIndex, sel);
	return bUseSOSel;
}

ProjectionHolderUVW *ProjectionModTypeUVW::GetProjectionHolderUVW( Object* obj )
{
    // For all derived objects (can be > 1)
    while (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
        IDerivedObject* dobj = (IDerivedObject*)obj;
        int numMods = dobj->NumModifiers();
        // Step through all modififers and verify the class id
        for (int i=0; i<numMods; i++) {
            Modifier* mod = dobj->GetModifier(i);
            if (mod && (mod->ClassID()==PROJECTIONHOLDERUVW_CLASS_ID)) {
				ProjectionHolderUVW *pModTest = (ProjectionHolderUVW *)mod;
				if( pModTest->GetProjectionTypeLink() == this )
					return pModTest;
            }
        }
        obj = dobj->GetObjRef();
    }
    
    return NULL;
}

bool ProjectionModTypeUVW::InitProjectionState( ProjectionState &projState, INode *pNodeSrc, ObjectState * osSrc, INode *pNodeTarget, ProjectionHolderUVW *pMod, BitArray *pSOSel, IProjectionModData *pPModData )
{
	TimeValue t = GetCOREInterface()->GetTime();

	// common
	projState.pData = &pMod->GetHolderData();
	projState.pData->Reset();
	FillInHoldDataOptions( projState.pData );
	projState.pSOSel = pSOSel;

	if( projState.iProjType == ProjectionState::pt_sameTopology ) {
		if( (pNodeSrc == NULL) && (pPModData == NULL) )
			return false;

		if( pPModData == NULL ) {
			pPModData = mpPMod->GetProjectionModData(pNodeSrc);
			if( !pPModData )
				return false;
		}

		projState.objectSource = &pPModData->GetBaseObject();

		if( projState.objectSource->IsEmpty() || (projState.objectSource->NumFaces() == 0) )
			return false;

		if( (miSourceMapChannel != VERTEX_CHANNEL_NUM) && !projState.objectSource->GetChannelSupport(miSourceMapChannel) ) 
			return false;

		if( osSrc ) {
			projState.pData->mSrcInterval &= osSrc->obj->ChannelValidity(t, TEXMAP_CHAN_NUM);
			projState.pData->mSrcInterval &= osSrc->obj->ChannelValidity(t, TOPO_CHAN_NUM);
			projState.pData->mSrcInterval &= osSrc->obj->ChannelValidity(t, GEOM_CHAN_NUM);
		}
	}
	else { 
		// Get Target ObjectWrapper
		ObjectState os = pNodeTarget->EvalWorldState(t);
		Matrix3 tarTM = pNodeTarget->GetObjTMAfterWSM( t );
		projState.objectTarget = new ObjectWrapper();
		projState.objectTarget->Init(t, os, false, ObjectWrapper::allEnable, ObjectWrapper::triObject);

		if( projState.objectTarget->IsEmpty() )
			return false;

		// Get Source ObjectWrapper
		IProjectionModData *pPModData = mpPMod->GetProjectionModData(pNodeSrc);
		if( !pPModData )
			return false;

		projState.objectSource = &pPModData->GetBaseObject();
		if( projState.objectSource->IsEmpty() || (projState.objectSource->NumFaces() == 0) )
			return false;

		pNodeSrc->EvalWorldState( t );
		Matrix3 srcTM = pNodeSrc->GetObjTMAfterWSM( t );

		// Init Intersector
		ObjectWrapper &objectCage = pPModData->GetCage();	// cage can be empty

		if( (miSourceMapChannel != VERTEX_CHANNEL_NUM) && !projState.objectSource->GetChannelSupport(miSourceMapChannel) ) 
			return false;

		IProjectionIntersectorMgr* pPInterMgr = GetIProjectionIntersectorMgr();
		if( !pPInterMgr )
			return false;

		projState.pPInter = pPInterMgr->CreateProjectionIntersector();
		if( !projState.pPInter )
			return false;

		projState.pPInter->InitRoot( *(projState.objectSource), objectCage, srcTM );
		if( !projState.pPInter->RootValid() ) {
			projState.pPInter->DeleteThis();
			projState.pPInter = NULL;
			return false;
		}
		projState.mat = (tarTM*Inverse(srcTM));
	}

	return true;
}

void ProjectionModTypeUVW::ProjectToTarget(INode *pNodeSrc, ObjectState *os, INode *pNodeTarget, BitArray *pSOSel, IProjectionModData *pPModData)
{
	if( pNodeTarget ) {
		Object* obj = pNodeTarget->GetObjectRef();

		if( obj ) {
			bool bCreatedMod = false;

			BOOL bCreateNew = FALSE;
			mpPBlock->GetValue(pb_holderCreateNew, 0, bCreateNew, FOREVER);

			ProjectionHolderUVW* mod;
			if( GetAlwaysUpdate() || !bCreateNew )
				mod = GetProjectionHolderUVW(obj);
			else
				mod = NULL;

			// If we aren't reusing a holder modifier, create a new one
			if( !mod ) {
				bCreatedMod = true;
				mod = (ProjectionHolderUVW*)CreateInstance(OSM_CLASS_ID, PROJECTIONHOLDERUVW_CLASS_ID);
			}

			if( mod ) {
				Interface *ip = GetCOREInterface();

				// Set Modifier/Holder name
				TCHAR *szName = NULL;
				mpPBlock->GetValue(pb_holderName, 0, szName, FOREVER);
				if( szName )
					mod->SetName( szName );



				// Set ProjectionType Link to us
				if( mod->GetProjectionTypeLink() != this )
					mod->SetProjectionTypeLink(this);

				// Use Proper projection technique
				BOOL bSameTopology = FALSE;
				mpPBlock->GetValue(pb_sameTopology, 0, bSameTopology, FOREVER);

				ProjectionState projState;

				mpPBlock->GetValue(pb_ignoreBackfacing, 0, projState.mIgnoreBackFaces, FOREVER);
				mpPBlock->GetValue(pb_testSeams, 0, projState.mTestSeams, FOREVER);
				mpPBlock->GetValue(pb_checkEdgeRatios, 0, projState.mCheckEdgeRatios, FOREVER);
				mpPBlock->GetValue(pb_weldUVs, 0, projState.mWeld, FOREVER);
				mpPBlock->GetValue(pb_weldUVsThreshold, 0, projState.mWeldThreshold, FOREVER);
				mpPBlock->GetValue(pb_edgeRatiosThreshold, 0, projState.mEdgeRatioThresold, FOREVER);


				if( bSameTopology ) {
					projState.iProjType = ProjectionState::pt_sameTopology;
					if( InitProjectionState( projState, pNodeSrc, os, pNodeTarget, mod, pSOSel, pPModData ) )
						FillInHoldDataSameTopology( projState );
				}
				else if( miSourceMapChannel > 0 ) {
					// If the source channel is a "real" map channel, use the seam detecting algorithm
					projState.iProjType = ProjectionState::pt_projectionCluster;
					if( InitProjectionState( projState, pNodeSrc, os, pNodeTarget, mod, pSOSel, pPModData ) )
						FillInHoldDataClusterProjection( projState );

				}
				else {
					projState.iProjType = ProjectionState::pt_projection;
					if( InitProjectionState( projState, pNodeSrc, os, pNodeTarget, mod, pSOSel, pPModData ) )
						FillInHoldDataProjection( projState );
				}

				projState.Cleanup();

				if( bCreatedMod ) {
					IDerivedObject* dobj = CreateDerivedObject(obj);
					dobj->AddModifier(mod);
					pNodeTarget->SetObjectRef(dobj);
				}

				mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				if( bCreatedMod )
					pNodeTarget->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}
		}
	}

}

bool ProjectionModTypeUVW::GetProjectMaterialsIDs()
{
	if( !mpPBlock )
		return false;
	return mpPBlock->GetInt(pb_projectMaterialIDs, GetCOREInterface()->GetTime()) != 0;
}

void ProjectionModTypeUVW::FillInHoldDataOptions( ProjectionHolderUVWData *pData )
{
	pData->miSource = ProjectionHolderUVWData::source_uvw;
	pData->miMapChannel = miTargetMapChannel;
	pData->mDoMaterialIDs = GetProjectMaterialsIDs();
}

GenFace ProjectionModTypeUVW::GetChannelFace( ObjectWrapper *object, int iFace, int iMapChannel )
{
	if( iMapChannel == VERTEX_CHANNEL_NUM )
		return object->GetFace( iFace );
	else
		return object->GetMapFace( iMapChannel, iFace );
}

Point3 *ProjectionModTypeUVW::GetChannelVert( ObjectWrapper *object, int iVert, int iMapChannel )
{
	if( iMapChannel == VERTEX_CHANNEL_NUM )
		return object->GetVert( iVert );
	else
		return object->GetMapVert( iMapChannel, iVert );
}

int ProjectionModTypeUVW::FindMostHitFace(Tab<int> &tabFaceHits)
{
	// Find the most hit face
	int iMostHitIndex = 0;
	int iHitCount = 0;
	for(int i=0; i<tabFaceHits.Count(); i++) {
		int iHits = 0;
		for(int j=0; j<tabFaceHits.Count(); j++) {
			if( tabFaceHits[i] == tabFaceHits[j] )
				iHits++;
		}
		if( iHits > iHitCount ) {
			iMostHitIndex = i;
			iHitCount = iHits;
		}
	}

	// Get the "main" or most common value
	return tabFaceHits[iMostHitIndex];
}


// Expand face list by traversing edges.  If edge is UV seam, do not cross edge.
int ProjectionModTypeUVW::ExpandClusterFaces( ProjectionState &projState, Tab<int> &clusterFaceList, int iNewFaceIndex, BitArray &faceVisited )
{
	int iNumAdded = 0;
	int iCurClusterFaceListCount = clusterFaceList.Count();

	for( int f=iNewFaceIndex; f<iCurClusterFaceListCount; f++ ) {

		int iFaceIndex = clusterFaceList[f];
		faceVisited.Set(iFaceIndex);

		GenFace face = projState.objectSource->GetFace(iFaceIndex);

		int iNumEdges = projState.objectSource->NumFaceEdges(iFaceIndex);

		// Traverse through the edges for this face.
		for( int i=0; i<iNumEdges; i++ ) {

			// Find the other face index on this edge
			int iOtherFace = -1;
			int iEdgeIndex = projState.objectSource->GetFaceEdgeIndex(iFaceIndex, i);
			if( iEdgeIndex == -1 )
				continue;
			
			// If this edge is a cluster border, don't look across it
			if( projState.bitClusterEdges[iEdgeIndex] )
				continue;

			GenEdge edge = projState.objectSource->GetEdge(iEdgeIndex);
			if( ((int)edge.f[0] != -1) && ((int)edge.f[0] != iFaceIndex) )
				iOtherFace = (int)edge.f[0];
			else if( ((int)edge.f[1] != -1) && ((int)edge.f[1] != iFaceIndex) )
				iOtherFace = (int)edge.f[1];

			if( iOtherFace == -1 )
				continue;

			if( faceVisited[iOtherFace] )
				continue;

			// Add the new face to the list
			clusterFaceList.Append(1, &iOtherFace);
			faceVisited.Set(iOtherFace);
			iNumAdded++;
		}
	}

	return iNumAdded;
}

// Traverse all the edges of the source object and tag edges that represent UV seams
bool ProjectionModTypeUVW::CreateClusterEdges( ProjectionState &projState )
{
	int iEdgeCount = projState.objectSource->NumEdges();

	for( int i=0; i<iEdgeCount; i++ ) {

		GenEdge edge = projState.objectSource->GetEdge(i);
		if( ((int)edge.f[0] == -1) || ((int)edge.f[1] == -1) )
			continue;

		int iFaceIndex1 = edge.f[0];
		int iFaceIndex2 = edge.f[1];

		GenFace face1 = projState.objectSource->GetFace(iFaceIndex1);
		GenFace face2 = projState.objectSource->GetFace(iFaceIndex2);

		GenFace faceMap1 = projState.objectSource->GetMapFace(miSourceMapChannel, iFaceIndex1);
		GenFace faceMap2 = projState.objectSource->GetMapFace(miSourceMapChannel, iFaceIndex2);

		// Find the corresponding face vert index to edge vert
		int iFace1V1 = 0, iFace1V2 = 0, iFace2V1 = 0, iFace2V2 = 0;
		for( int j=0; j<face1.numVerts; j++ ) {
			if( edge.v[0] == face1.verts[j] )
				iFace1V1 = j;
			if( edge.v[1] == face1.verts[j] )
				iFace1V2 = j;
		}

		for( int j=0; j<face2.numVerts; j++ ) {
			if( edge.v[0] == face2.verts[j] )
				iFace2V1 = j;
			if( edge.v[1] == face2.verts[j] )
				iFace2V2 = j;
		}

		if( (faceMap1.verts[iFace1V1] != faceMap2.verts[iFace2V1])
			|| (faceMap1.verts[iFace1V2] != faceMap2.verts[iFace2V2]) ) 
		{
			projState.bitClusterEdges.Set(i);
		}
	}

	return true;
}

void ProjectionModTypeUVW::CreateClusterData( ProjectionState &projState )
{
	int iFaceCount = projState.objectSource->NumFaces();

	BitArray faceVisited;
	faceVisited.SetSize(iFaceCount);
	faceVisited.ClearAll();

	projState.tabCluster.SetCount(iFaceCount);

	projState.bitClusterEdges.SetSize( projState.objectSource->NumEdges() );
	projState.bitClusterEdges.ClearAll();

	// Tag cluster edges
	CreateClusterEdges(projState);

	int iClusterNum = 1;
	Point2 ptClusterCenter;
	Tab<int> clusterFaceList;
	clusterFaceList.Resize(iFaceCount);
	for( int i=0; i<iFaceCount; i++ ) {
		if( !faceVisited[i] ) {

			// Expand the cluster faces until no more are added
			int iNewFaceIndex = 0;
			clusterFaceList.SetCount(1, FALSE);
			clusterFaceList[0] = i;
			int iNumAdded = 1;
			while( (iNumAdded = ExpandClusterFaces( projState, clusterFaceList, iNewFaceIndex, faceVisited )) != 0 ) {
				iNewFaceIndex = clusterFaceList.Count() - iNumAdded;
			}

			// We now have a cluster list
			Box3 bBox;
			bBox.Init();
			for( int f=0; f<clusterFaceList.Count(); f++ ) {
				int iFaceIndex = clusterFaceList[f];

				// Set cluster number
				projState.tabCluster[iFaceIndex] = iClusterNum;

				// Record bounding box of cluster
				GenFace faceMap = projState.objectSource->GetMapFace(miSourceMapChannel, iFaceIndex);
				for( int j=0; j<faceMap.numVerts; j++ ) {
					Point3 ptVert = *GetChannelVert( projState.objectSource, faceMap.verts[j], miSourceMapChannel );
					if( ptVert[0] < bBox.pmin[0] )
						bBox.pmin[0] = ptVert[0];
					if( ptVert[1] < bBox.pmin[1] )
						bBox.pmin[1] = ptVert[1];
					if( ptVert[0] > bBox.pmax[0] )
						bBox.pmax[0] = ptVert[0];
					if( ptVert[1] > bBox.pmax[1] )
						bBox.pmax[1] = ptVert[1];
				}
			}

			// Store the cluster center UV value
			ptClusterCenter[0] = (bBox.pmax[0]+bBox.pmin[0])*0.5f;
			ptClusterCenter[1] = (bBox.pmax[1]+bBox.pmin[1])*0.5f;
			projState.tabClusterCenter.Append(1, &ptClusterCenter);

			iClusterNum++;
		}
	}

	int iWow = projState.bitClusterEdges.NumberSet();
	iClusterNum = iClusterNum;

}

bool GetDistanceToEdge( Point3 &pt, Point3 &seg1, Point3 &seg2, float &fDist )
{
    float fLength = Length( seg2 - seg1 );
 
	float u = ( ((pt.x - seg1.x) * (seg2.x - seg1.x)) +
				((pt.y - seg1.y) * (seg2.y - seg1.y)) +
				((pt.z - seg1.z) * (seg2.z - seg1.z)) ) /
				(fLength * fLength);
 
    if( (u < 0.0f) || (u > 1.0f) )
        return false;
 
	Point3 ptInter;
    ptInter.x = seg1.x + u * (seg2.x - seg1.x);
    ptInter.y = seg1.y + u * (seg2.y - seg1.y);
    ptInter.z = seg1.z + u * (seg2.z - seg1.z);
 
    fDist = Length(pt - ptInter);
 
    return true;
}

// returns percentage along edge relative to seg1.  So, 1.0f means it is on seg1. 0.0f means it is on seg2
float ProjectPointToEdge( Point3 &pt, Point3 &seg1, Point3 &seg2 )
{
	Point3 p1, p2, p3;

	p1 = seg2-seg1;
	float fLengthP1 = Length(p1);
	p2 = pt-seg1;
	p3 = (float)(DotProd(p2,p1)/(fLengthP1*fLengthP1)) * p1;

	float w1 = 1.0f;
	if( fLengthP1 > 0.0000001f )
		w1 = 1.0f - (Length(p3) / fLengthP1);

	if( w1 < 0.0f )
		w1 = 0.0f;
	if( w1 > 1.0f )
		w1 = 1.0f;

	return w1;
}

void ProjectionModTypeUVW::ProjectToEdgeToFindUV( ProjectionState &projState, Point3 &p3Tar, int iEdgeIndex, int iFaceIndex, Point3 &UV )
{
	if( (iEdgeIndex != -1) && (iFaceIndex != -1) ) {
		// Prototype code to project the hi-res point onto the matching low-res edge.
		// Once we have the point projected onto the low-res edge, weigh the UV coords
		// at the two verts on the edge based on the edge projection location.
		Point3 v1, v2;
		GenEdge edgeSource = projState.objectSource->GetEdge(iEdgeIndex);
		v1 = *projState.objectSource->GetVert( edgeSource.v[0] );
		v2 = *projState.objectSource->GetVert( edgeSource.v[1] );
		float w1 = 1.0f, w2 = 0.0f;
		w1 = ProjectPointToEdge( p3Tar, v1, v2 );
		w2 = 1.0f - w1;
		GenFace faceSource = projState.objectSource->GetFace(iFaceIndex);
		int iFaceV0 = 0, iFaceV1 = 0;
		for( int j=0; j<faceSource.numVerts; j++ ) {
			if( edgeSource.v[0] == faceSource.verts[j] )
				iFaceV0 = j;
			if( edgeSource.v[1] == faceSource.verts[j] )
				iFaceV1 = j;
		}
		GenFace faceMapSource = GetChannelFace( projState.objectSource, iFaceIndex, miSourceMapChannel );
		v1 = *GetChannelVert( projState.objectSource, faceMapSource.verts[iFaceV0], miSourceMapChannel );
		v2 = *GetChannelVert( projState.objectSource, faceMapSource.verts[iFaceV1], miSourceMapChannel );
		UV = (v1*w1)+(v2*w2);
	}
}

void ProjectionModTypeUVW::FindClosestFaceAndEdgeByCluster( ProjectionState &projState, Point3 &p3Tar, int iMainCluster, int &iClosestEdgeIndex, int &iClosestFaceIndex )
{
	bool bInit = true;
	float fDist;
	float fClosestDist;
	int iClosestEdge = -1;
	int iClosestFace = -1;
	for(int i=0; i<projState.objectSource->NumEdges(); i++ ) {

		// Criteria #1 edge needs to be on a cluster
		if( !projState.bitClusterEdges[i] )
			continue;

		GenEdge edgeSource = projState.objectSource->GetEdge(i);

		// Criteria #2 one of the faces needs to be in the correct cluster
		int iFaceIndex = -1;
		if( ((int)edgeSource.f[0] != -1) && (projState.tabCluster[edgeSource.f[0]] == iMainCluster) )
			iFaceIndex = (int)edgeSource.f[0];
		else if( ((int)edgeSource.f[1] != -1) && (projState.tabCluster[edgeSource.f[1]] == iMainCluster) )
			iFaceIndex = (int)edgeSource.f[1];

		if( iFaceIndex != -1 ) {
			// Edge okay to test
			Point3 v0 = *projState.objectSource->GetVert( edgeSource.v[0] );
			Point3 v1 = *projState.objectSource->GetVert( edgeSource.v[1] );
			if( !GetDistanceToEdge( p3Tar, v0, v1, fDist ) )
				continue;
			if( bInit || (fDist < fClosestDist) ) {
				bInit = false;
				fClosestDist = fDist;
				iClosestEdge = i;
				iClosestFace = iFaceIndex;
			}
		}
	}

	iClosestEdgeIndex = iClosestEdge;
	iClosestFaceIndex = iClosestFace;
}

int ProjectionModTypeUVW::GetUVQuadrant( ProjectionState &projState, int iCluster, Point3 &ptUVW )
{
	int iQuad = -1;

	if( (iCluster < 1) || ((iCluster-1) >= projState.tabClusterCenter.Count()) )
		return iQuad;

	Point2 ptRelUV = projState.tabClusterCenter[iCluster-1];
	ptRelUV[0] = ptUVW[0] - ptRelUV[0];
	ptRelUV[1] = ptUVW[1] - ptRelUV[1];

	//	0	1
	//	2	3
	iQuad  = (ptRelUV[1] >= 0.0f) ? 0 : 2;
	iQuad += (ptRelUV[0] >= 0.0f) ? 1 : 0;

	return iQuad;
}


void ProjectionModTypeUVW::FillInHoldDataSameTopology( ProjectionState &projState )
{
	int iNumFaces = projState.objectSource->NumFaces();
    int iNumVerts;
	if( miSourceMapChannel == VERTEX_CHANNEL_NUM )
		iNumVerts = projState.objectSource->NumVerts();
	else
		iNumVerts = projState.objectSource->NumMapVerts(miSourceMapChannel);

	// Init tab counts in holder data
	projState.pData->mFaces.SetCount(iNumFaces);
	projState.pData->mP3Data.SetCount(iNumVerts);
	if( projState.pData->mDoMaterialIDs )
		projState.pData->mMatID.SetCount(iNumFaces);

	// Store the chosen point3 data
	for( int i=0; i<iNumVerts; i++ ) 
		projState.pData->mP3Data[i] = *GetChannelVert(projState.objectSource, i, miSourceMapChannel );
	
	// Recreate GenFace structures
	for( int i=0; i<iNumFaces; i++ ) {
		if( projState.pData->mDoMaterialIDs )
			projState.pData->mMatID[i] = projState.objectSource->GetMtlID(i);
		GenFace face = GetChannelFace(projState.objectSource, i, miSourceMapChannel );
		projState.pData->mFaces[i].numVerts = face.numVerts;
		projState.pData->mFaces[i].verts = new DWORD[face.numVerts];
		for( int j=0; j<face.numVerts; j++ )
			projState.pData->mFaces[i].verts[j] = face.verts[j];
	}

}

void ProjectionModTypeUVW::FillInHoldDataProjection( ProjectionState &projState )
{
	int iNumVerts = projState.objectTarget->NumVerts();
	int iNumFaces = projState.objectTarget->NumFaces();

	// Init tab counts in holder data
	projState.pData->mFaces.SetCount(iNumFaces);
	projState.pData->mP3Data.SetCount(iNumVerts);
	if( projState.pData->mDoMaterialIDs )
		projState.pData->mMatID.SetCount(iNumFaces);

	Point3 p3Tar, uv;
	Point3 p3Zero(0.0f,0.0f,0.0f);
	float fDist;
	DWORD dwFace;
	Tab<float> tabBary;

	// cached vertex hit results
	Tab<int> tabFaceHits;
	tabFaceHits.SetCount( iNumVerts );

	// Use ProjectionIntersector to find closest source face per target vertex
	for( int i=0; i<iNumVerts; i++ ) {
		uv = p3Zero;
		tabFaceHits[i] = -1;
		// Get target vert
		p3Tar = *projState.objectTarget->GetVert( i );
		// Put into object space of source
		p3Tar = p3Tar*projState.mat;

		Ray pointAndNormal;
		pointAndNormal.p = p3Tar;
		int flags = IProjectionIntersector::FINDCLOSEST_CAGED;

		if( projState.pPInter->RootClosestFace( pointAndNormal, flags, fDist, dwFace, tabBary ) ) {
			// Check sub-object selection
			if( (projState.pSOSel == NULL) || (*projState.pSOSel)[dwFace] ) {
				tabFaceHits[i] = dwFace;
				GenFace face = GetChannelFace( projState.objectSource, dwFace, miSourceMapChannel );
				// Calc value based on bary hit results
				for( int j=0; j<tabBary.Count(); j++ ) {
					uv += ((*GetChannelVert(projState.objectSource, face.verts[j], miSourceMapChannel)) * tabBary[j]);
				}
			}
			else if( (miTargetMapChannel == VERTEX_CHANNEL_NUM) || projState.objectTarget->GetChannelSupport(miTargetMapChannel) ) {
				uv = *GetChannelVert(projState.objectTarget, i, miTargetMapChannel);
			}
		}
		projState.pData->mP3Data[i] = uv;
	}
	
	// Recreate GenFace structures
	Tab<int> tabPerFaceHit;
	for( int i=0; i<iNumFaces; i++ ) {
		GenFace face = projState.objectTarget->GetFace( i );
		projState.pData->mFaces[i].numVerts = face.numVerts;
		projState.pData->mFaces[i].verts = new DWORD[face.numVerts];
		if( projState.pData->mDoMaterialIDs )
			tabPerFaceHit.SetCount( face.numVerts );
		for( int j=0; j<face.numVerts; j++ ) {
			projState.pData->mFaces[i].verts[j] = face.verts[j];
			if( projState.pData->mDoMaterialIDs )
				tabPerFaceHit[j] = tabFaceHits[face.verts[j]];
		}
		// To get the material ID, determine the most hit source face for the target verts on this
		// target face
		if( projState.pData->mDoMaterialIDs ) {
			int iSourceFace = FindMostHitFace( tabPerFaceHit );
			if( iSourceFace != -1 )
				projState.pData->mMatID[i] = projState.objectSource->GetMtlID(iSourceFace);
			else
				projState.pData->mMatID[i] = projState.objectTarget->GetMtlID(i);
		}
	}

}

void ProjectionModTypeUVW::FillInHoldDataClusterProjection( ProjectionState &projState   )
{

	int iNumFaces = projState.objectTarget->NumFaces();

	int vertexBufferCount = 0;
	for( int i=0; i<iNumFaces; i++ ) 
	{
		GenFace faceTarget = projState.objectTarget->GetFace(i);
		vertexBufferCount += faceTarget.numVerts;
	}
	int iNumVerts = projState.objectTarget->NumVerts();

	// Get cluster data
	// This will calculate the data to group faces into UVW clusters/charts
	// It will also mark cluster edges.
	CreateClusterData( projState );

	// Init tab counts in holder data
	projState.pData->mFaces.SetCount(iNumFaces);
	projState.pData->mP3Data.SetCount(vertexBufferCount);

	if( projState.pData->mDoMaterialIDs )
		projState.pData->mMatID.SetCount(iNumFaces);

	Point3 p3Tar, uv;
	Point3 p3Zero(0.0f,0.0f,0.0f);
	float fDist;
	DWORD dwFace;
	Tab<float> tabBary;

	// cached vertex hit results
	Tab<int> tabFaceHits;
	tabFaceHits.SetCount( iNumVerts );

	Tab<Point3> tempHitUVs;
	tempHitUVs.SetCount(iNumVerts);
	// Use ProjectionIntersector to find closest source face per target vertex
	for( int i=0; i<iNumVerts; i++ ) 
	{
		tabFaceHits[i] = -1;
		uv = p3Zero;
		// Get target vert
		p3Tar = *projState.objectTarget->GetVert( i );
		// Put into object space of source
		p3Tar = p3Tar*projState.mat;

		Ray pointAndNormal;
		pointAndNormal.p = p3Tar;
		pointAndNormal.dir = *projState.objectTarget->GetVertexNormal(i);
		pointAndNormal.dir = Normalize( pointAndNormal.dir );

		int flags = 0;
		if (projState.mIgnoreBackFaces)
			flags |= IProjectionIntersector::FINDCLOSEST_IGNOREBACKFACING;

		BOOL bInterRes = projState.pPInter->RootClosestFace( pointAndNormal, flags, fDist, dwFace, tabBary );

		if( !bInterRes )
		{
			flags = IProjectionIntersector::FINDCLOSEST_CAGED;
			if (projState.mIgnoreBackFaces)
				flags |= IProjectionIntersector::FINDCLOSEST_IGNOREBACKFACING;
			bInterRes = projState.pPInter->RootClosestFace( pointAndNormal, 0, fDist, dwFace, tabBary );
		}
		
		if( bInterRes ) 
		{
			// Check sub-object selection
			if( (projState.pSOSel == NULL) || (*projState.pSOSel)[dwFace] ) 
			{
				tabFaceHits[i] = dwFace;
				GenFace face = GetChannelFace(projState.objectSource, dwFace, miSourceMapChannel);
				GenFace geoFace = GetChannelFace(projState.objectSource, dwFace, VERTEX_CHANNEL_NUM);
				// Calc value based on bary hit results
				for( int j=0; (j<tabBary.Count()) && (j<face.numVerts); j++ ) 
				{
					uv += ((*GetChannelVert(projState.objectSource, face.verts[j], miSourceMapChannel)) * tabBary[j]);
				}
			}
		}
		tempHitUVs[i] = uv;
	}
	int index = 0;
	for( int i=0; i<iNumFaces; i++ ) 
	{
		GenFace faceTarget = projState.objectTarget->GetFace(i);
		for (int j = 0; j < faceTarget.numVerts; j++)
		{
			int vIndex = faceTarget.verts[j];
			projState.pData->mP3Data[index++] = tempHitUVs[vIndex];
		}
	}

	Point3 norm;
	Point3 ptCenter;
	Point3 ptCenterUVW;
	Tab<int> tabPerFaceHit;
	int vertexBufferIndex = 0;
	for( int i=0; i<iNumFaces; i++ ) 
	{
		ptCenter = p3Zero;
		ptCenterUVW = p3Zero;
		norm = p3Zero;
		GenFace faceTarget = projState.objectTarget->GetFace(i);
		tabPerFaceHit.SetCount( faceTarget.numVerts );
		// Create a source face hit array for the verts on this target face
		// In addition, we will also find the closest source face to the center
		// point of this target face.  This will help use determine the proper
		// cluster to use.
		for( int j=0; j<faceTarget.numVerts; j++ ) 
		{
			tabPerFaceHit[j] = tabFaceHits[faceTarget.verts[j]];
			ptCenter += *projState.objectTarget->GetVert(faceTarget.verts[j]);
			norm += *projState.objectTarget->GetVertexNormal(faceTarget.verts[j]);
		}
		norm /= faceTarget.numVerts;
		ptCenter /= faceTarget.numVerts;
		ptCenter = ptCenter * projState.mat;
		int iCenterFace = -1;

		Ray pointAndNormal;
		pointAndNormal.p = ptCenter;
		pointAndNormal.dir = norm;
		pointAndNormal.dir = Normalize( pointAndNormal.dir );
		int flags = IProjectionIntersector::FINDCLOSEST_CAGED | IProjectionIntersector::FINDCLOSEST_IGNOREBACKFACING;

		// Find closest source face to the center point of this target face
		BOOL bInterRes = projState.pPInter->RootClosestFace( pointAndNormal, flags, fDist, dwFace, tabBary );
		if ( !bInterRes )
			bInterRes = projState.pPInter->RootClosestFace( pointAndNormal, IProjectionIntersector::FINDCLOSEST_IGNOREBACKFACING, fDist, dwFace, tabBary );
		if ( !bInterRes && !projState.mIgnoreBackFaces)
			bInterRes = projState.pPInter->RootClosestFace( pointAndNormal, 0, fDist, dwFace, tabBary );
		
		if( bInterRes ) {
			if( (projState.pSOSel == NULL) || (*projState.pSOSel)[dwFace] ) {
				iCenterFace = dwFace;
				GenFace face = GetChannelFace(projState.objectSource, dwFace, miSourceMapChannel);
				// Calc value based on bary hit results
				for( int j=0; (j<tabBary.Count()) && (j<face.numVerts); j++ )
				{					
					ptCenterUVW += ((*GetChannelVert(projState.objectSource, face.verts[j], miSourceMapChannel)) * tabBary[j]);
				}
			}
		}
	


		FixMapFaceData( projState, i, tabPerFaceHit, iCenterFace, ptCenter, ptCenterUVW,vertexBufferIndex );
	}

	//weld up vertices now
	if (projState.mWeld)
	{	
		//loop through the geom vertices and find all the tv vertices connected to them
		//this is used to check the threshold to find close vertices
		Tab<NeighborList> connectionData;
		connectionData.SetCount(projState.objectTarget->NumVerts());

		for (int i = 0; i < projState.objectTarget->NumVerts(); i++)
			connectionData[i].mStart = NULL;
			
		Tab<NeighborList*> lookupData;

		lookupData.SetCount(vertexBufferIndex);
		for (int i = 0; i < vertexBufferIndex; i++)
			lookupData[i] = NULL;
		
		projState.objectTarget->NumFaces();

		for( int i=0; i<iNumFaces; i++ ) 
		{
			GenFace geoFace = GetChannelFace(projState.objectTarget, i, VERTEX_CHANNEL_NUM);
			
			for( int j=0; j< geoFace.numVerts; j++ ) 
			{
				int geoVert =  geoFace.verts[j];
				int tvVert = projState.pData->mFaces[i].verts[j];
				connectionData[geoVert].Add(tvVert);
				lookupData[tvVert] = connectionData.Addr(geoVert);
			}
		}

		BitArray processedVerts;
		Tab<int> newLookUpID;
		newLookUpID.SetCount(vertexBufferIndex);
		processedVerts.SetSize(vertexBufferIndex);
		processedVerts.ClearAll();	

		Tab<float> closestVertDist;
		closestVertDist.SetCount(vertexBufferIndex);
		for (int i  = 0; i < closestVertDist.Count(); i++)
			closestVertDist[i] = -1.0f;


		//compute our threshold, this is the shortest UV edge attached the vertex
		//times our ratio
		for( int i=0; i< projState.objectTarget->NumFaces(); i++ ) 
		{
			int numEdges = projState.pData->mFaces[i].numVerts;
			for (int j = 0; j < numEdges; j++)
			{
				int a = projState.pData->mFaces[i].verts[j];
				int b = projState.pData->mFaces[i].verts[(j+1)%numEdges];
				Point3 pa = projState.pData->mP3Data[a];
				Point3 pb = projState.pData->mP3Data[b];
				pa.z = 0.0f;
				pb.z = 0.0f;
				float d = Length(pa-pb);
				if ((closestVertDist[a] == -1.0f) || (d < closestVertDist[a]))
					closestVertDist[a] = d;
				if ((closestVertDist[b] == -1.0f) || (d < closestVertDist[b]))
					closestVertDist[b] = d;
					
			}
		}
		
		//find the vertices that are within in the threshold and weld them
		for (int i = 0; i < vertexBufferIndex; i++)
		{
			if (!processedVerts[i])
			{	
				newLookUpID[i] = i;
				processedVerts.Set(i,TRUE);
				Point3 uv = projState.pData->mP3Data[i];
				if (lookupData[i])
				{
					Neighbor *current = lookupData[i]->mStart;


					float threshold = 1.0f;
					//find the closest threshold
					while (current != NULL)	
					{
						int testID = current->mID;
						if (closestVertDist[testID] < threshold)
							threshold = closestVertDist[testID];
						current = current->mNext;
					}
					threshold *= projState.mWeldThreshold;


					current = lookupData[i]->mStart;
					Point3 centroid = uv;
					int ct = 1;
					while (current != NULL)	
					{
						int testID = current->mID;
						if (!processedVerts[testID])
						{		
							Point3 uvZZero = uv;
							uvZZero.z = 0.0f;

							Point3 testUV = projState.pData->mP3Data[testID];
							testUV.z = 0.0f;

							float d = Length(testUV-uvZZero);
							if ( d < threshold)
							{
								processedVerts.Set(testID,TRUE);
								newLookUpID[testID] = i;
								ct++;
								centroid += testUV;
							}
							
						}
						current = current->mNext;
					}
					if (ct != 0)
					{
						centroid = centroid/(float)ct;
						centroid.z = 0.0f;
						projState.pData->mP3Data[i] = centroid;
						
					}
					
				}
			}
		}

		//now correct the face indices
		for( int i=0; i<iNumFaces; i++ ) 
		{
			GenFace geoFace = GetChannelFace(projState.objectTarget, i, VERTEX_CHANNEL_NUM);

			for( int j=0; j< geoFace.numVerts; j++ ) 
			{
				int tvVert = projState.pData->mFaces[i].verts[j];
				projState.pData->mFaces[i].verts[j] = newLookUpID[tvVert];
			}
		}

		//now clean up dead vertices
		//find the dead verts
		BitArray deadVerts;
		deadVerts.SetSize(vertexBufferIndex);
		deadVerts.SetAll();

		//this is the lookup list for the new indices
		Tab<int> lookup;
		lookup.SetCount(vertexBufferIndex);
		for (int i = 0; i < vertexBufferIndex; i++)
			lookup[i] = -1;

		//set our dead verts
		for( int i=0; i<iNumFaces; i++ ) 
		{
			GenFace geoFace = GetChannelFace(projState.objectTarget, i, VERTEX_CHANNEL_NUM);

			for( int j=0; j< geoFace.numVerts; j++ ) 
			{
				int tvVert = projState.pData->mFaces[i].verts[j];
				deadVerts.Set(tvVert,FALSE);
			}
		}

		//condense our UVW verts
		int currentIndex = 0;
		for (int i =0; i < vertexBufferIndex; i++)
		{
			if (!deadVerts[i])
			{
				lookup[i] = currentIndex;
				projState.pData->mP3Data[currentIndex] = projState.pData->mP3Data[i];
				currentIndex++;
			}			
		}

		//fix up the faces
		for( int i=0; i<iNumFaces; i++ ) 
		{
			GenFace geoFace = GetChannelFace(projState.objectTarget, i, VERTEX_CHANNEL_NUM);

			for( int j=0; j< geoFace.numVerts; j++ ) 
			{
				int tvVert = projState.pData->mFaces[i].verts[j];
				projState.pData->mFaces[i].verts[j] = lookup[tvVert];
				
			}
		}

		int numberUsed = vertexBufferIndex - deadVerts.NumberSet();
		projState.pData->mP3Data.SetCount(numberUsed,TRUE);

	}

	mProcessedFaces.SetSize(0);
}



void ProjectionModTypeUVW::FixMapFaceData( ProjectionState &projState, int iTargetFace, Tab<int> &tabFaceHits, int iCenterFaceHit, Point3 &ptCenter, Point3 &ptCenterUVW , int &vertexBufferOffset)
{
	GenFace faceTarget = projState.objectTarget->GetFace(iTargetFace);

	// Set number of verts and allocate vert array
	projState.pData->mFaces[iTargetFace].numVerts = faceTarget.numVerts;
	projState.pData->mFaces[iTargetFace].verts = new DWORD[faceTarget.numVerts];

	// Regardless of the most hit face, we will use iCenterFaceHit to determine
	// the "most hit face".  This will help us make a better cluster decision.

	int iMostHitFace;
	int iMostHitClusterIndex = 0;
	if( iCenterFaceHit != -1 ) 
	{
		iMostHitFace = iCenterFaceHit;
	}
	else {
		// no iCenterFaceHit, find the most hit cluster
		int iMostHitCount = 0;
		for(int i=0; i<tabFaceHits.Count(); i++) {
			int iHits = 0;
			for(int j=0; j<tabFaceHits.Count(); j++) {
				if( (tabFaceHits[i] != -1) 
					&& (tabFaceHits[j] != -1) 
					&& (projState.tabCluster[tabFaceHits[i]] == projState.tabCluster[tabFaceHits[j]]) )
				{
					iHits++;
				}
			}
			if( iHits > iMostHitCount ) {
				iMostHitClusterIndex = i;
				iMostHitCount = iHits;
			}
		}

		// Record most hit face
		iMostHitFace = tabFaceHits[iMostHitClusterIndex];
	}

	// Handle material ID first
	if( projState.pData->mDoMaterialIDs )
	{
		if (iMostHitFace != -1)
			projState.pData->mMatID[iTargetFace] = projState.objectSource->GetMtlID(iMostHitFace);
		else 
			projState.pData->mMatID[iTargetFace] = 0;
	}

	// If we do not need to Recalc any of hits, just transfer the face data from the projections.
	// note we now break every face so we have basically the initial number if uvs is roughly 3 to 4 * number faces
	// it easier to weld them all back when we are done intstead of trying to break faces as we go
	for( int j=0; j<faceTarget.numVerts; j++ )
		projState.pData->mFaces[iTargetFace].verts[j] = vertexBufferOffset++;

	if (tabFaceHits.Count() == 0)
		return;

	BOOL fixupVertices = FALSE;  //if this is set we need to approx our UVs since we crossed a seam or something bad happened
	int masterFace = -1;		//this is the face used to do our approx.  we use bary coords to get an approx

	//first check to see if the face hit list has any misses in it
	//then we must approximate
	BOOL misses = FALSE;
	for(int i=0; i<tabFaceHits.Count(); i++) 
	{
		if (tabFaceHits[i] == -1)
			misses = TRUE;
	}

	BOOL edgeRatioError = FALSE;

	//we had a miss so just use one of our hit faces to approx the new UVW
	if (misses)
	{
		for(int i=0; i<tabFaceHits.Count(); i++) 
		{
			if (tabFaceHits[i] != -1)
				masterFace = tabFaceHits[i];				
		}
		fixupVertices = TRUE;
	}
	//next we need to check to see if the target face is contained completely within one source face
	else
	{		
		//if so we are done
		BOOL hitAllSameFace = TRUE;
		int testHitFace = iCenterFaceHit;
		for( int i=0; i<tabFaceHits.Count(); i++) 
		{
			if (tabFaceHits[i] != testHitFace)
				hitAllSameFace = FALSE;
		}

		//if we are not all on the same face need to do some digging
		if (!hitAllSameFace)
		{

			//next see if we span across 2 more or UVW clusters
			BOOL hitAllSameUVCluster = TRUE;
			int testHitFace = iCenterFaceHit;
			if (testHitFace == -1)
				hitAllSameUVCluster = FALSE;
			else
			{
				int testUVCluster = projState.tabCluster[testHitFace];
				for( int i=0; i<tabFaceHits.Count(); i++) 
				{				
					if (projState.tabCluster[tabFaceHits[i]] != testUVCluster)
						hitAllSameUVCluster = FALSE;
				}
			}
			//if span across 2 uvw clusteer we can just approximate
			if (!hitAllSameUVCluster)
			{
				fixupVertices = TRUE;
				//compute the master face
			}
			else  //we still need to do some more through checks now
			{
				//do a quick edge ratio check to find potential error faces				
				if (projState.mCheckEdgeRatios)
				{
					float change;

					if (CheckEdgeRatio(projState, iTargetFace, projState.mEdgeRatioThresold,change))
					{
						fixupVertices = TRUE;
						edgeRatioError = TRUE;
					}
					if (CheckAngleRatio(projState, iTargetFace,0.5f,change))
					{
						fixupVertices = TRUE;
						edgeRatioError = TRUE;
					}
				}

				//final check to see if any of our target edges cross a source seam in target face space
				//this is pretty slow so this is the last check
				if (!fixupVertices && projState.mTestSeams && TestFaceCrossesSeams(projState,iTargetFace,tabFaceHits))
					fixupVertices = TRUE;

			}
			//need to find our master face
			//the best we can do here is to find the closest face
			if (fixupVertices)
			{
				masterFace = FindMasterFace(projState, iTargetFace, tabFaceHits,iCenterFaceHit);
				if (masterFace != -1)
					iCenterFaceHit = -1;  //we set the iCenterHitFace to -1 so it wont be used since that is the default and we found something better
			}

		}
	}

	//we need to fix up the face since it crosses a seam or is a potential error face
	if (fixupVertices )
	{
		//get our initial hit faces

		if (iCenterFaceHit != -1)
			masterFace = iCenterFaceHit;

		Tab<Point3> results;

		ReprojectFace(projState, iTargetFace,  masterFace, tabFaceHits, results);

		
		for( int j=0; j<faceTarget.numVerts; j++ ) 
		{
			int vIndex = projState.pData->mFaces[iTargetFace].verts[j];
			projState.pData->mP3Data[vIndex] = results[j];
		}
	}
}

BOOL ProjectionModTypeUVW::TestFaceCrossesSeams( ProjectionState &projState, int targetFace, Tab<int> &sourceFace)
{
	int sourceFaceCount = sourceFace.Count();
//do simple check see if any of the faces share a common edge		
	
	if (mProcessedFaces.GetSize() != projState.objectSource->NumFaces())
		mProcessedFaces.SetSize(projState.objectSource->NumFaces());
	mProcessedFaces.ClearAll();



	//get our initial hit faces
	Tab<int> checkFaces;  //this is just a list of all our hit faces no repeats
	for (int i = 0; i < sourceFaceCount; i++)
	{
		int faceIndex = sourceFace[i];
		if (!mProcessedFaces[faceIndex])
		{
			checkFaces.Append(1,&faceIndex,10);
			mProcessedFaces.Set(faceIndex,TRUE);
		}
	}

	//do simple check first
	//get all the target faces
	//loop through them
	for (int i = 0; i < checkFaces.Count(); i++)
	{
		int faceIndex = checkFaces[i];
		//loop through there edges
		GenFace face = projState.objectSource->GetFace(faceIndex);
		int numEdges = face.numVerts;
		for (int j = 0; j < numEdges; j++)
		{
			//if seam
			int edgeIndex = projState.objectSource->GetFaceEdgeIndex(faceIndex, j);
			if( projState.bitClusterEdges[edgeIndex] )
			{
				//see if opposing edge is a processed face
				GenEdge edge = projState.objectSource->GetEdge(edgeIndex);
				int opposingFaceA = (int)edge.f[0];
				int opposingFaceB = (int)edge.f[1];
				if ( (opposingFaceA != -1) && (opposingFaceB != -1))
				{
					if (mProcessedFaces[opposingFaceA] && mProcessedFaces[opposingFaceB])
					{

						return TRUE;
					}
				}
			}
		}
	}	
	
	//we failed the simple check now need to try something more sophisticated
	//we need to use  centroids since our hit point can lie outside the hit face
	//which will mess up the edge walking
	Tab<Point3> hitFaceCentroids;
	hitFaceCentroids.SetCount(checkFaces.Count());
	for (int i = 0; i < checkFaces.Count(); i++)
	{
		Point3 center(0.0f,0.0f,0.0f);
		GenFace geoFace = GetChannelFace(projState.objectSource, checkFaces[i], VERTEX_CHANNEL_NUM);
		int numVerts = geoFace.numVerts;
		for (int j = 0; j < numVerts; j++)
		{
			center += (*projState.objectSource->GetVert(geoFace.verts[j]));
		}

		if (numVerts)
			hitFaceCentroids[i] = center/(float)numVerts;
		else hitFaceCentroids[i] = center;

		//transform that point form 

	}

//build a tm for our check face
	Point3 masterXYZs[3];

	Matrix3 tm(1),itm(1);
	if (hitFaceCentroids.Count() > 2)
	{
		masterXYZs[0] = hitFaceCentroids[0];
		masterXYZs[1] = hitFaceCentroids[1];

		Point3 vecA = Normalize(masterXYZs[1] - masterXYZs[0]);
		Point3 vecB(1.0f,0.0f,0.0f);

		for (int i = 2; i < hitFaceCentroids.Count(); i++)
		{		
			masterXYZs[2] = hitFaceCentroids[i];

			//make sure last one does not cause a degenerate face
			vecB = Normalize(masterXYZs[2] - masterXYZs[0]);
			float dot = DotProd(vecA,vecB);
			if ((dot != 1.0f) && (dot != -1.0f))
				i = hitFaceCentroids.Count(); 
		}
		//compute our geo plane equation

		Point3 norm = Normalize(CrossProd(vecA,vecB));
		
		MatrixFromNormal(norm,tm);
		tm.SetRow(3,masterXYZs[0]);
		itm = Inverse(tm);
	}
	else
	{
		GenFace geoFace = GetChannelFace(projState.objectTarget, targetFace, VERTEX_CHANNEL_NUM);
		masterXYZs[0] = (*projState.objectTarget->GetVert(geoFace.verts[0])) * projState.mat;
		masterXYZs[1] = (*projState.objectTarget->GetVert(geoFace.verts[1])) * projState.mat;

		Point3 vecA = Normalize(masterXYZs[1] - masterXYZs[0]);
		Point3 vecB = Normalize(masterXYZs[2] - masterXYZs[0]);

		for (int i = 2; i < geoFace.numVerts; i++)
		{		
			masterXYZs[2] = (*projState.objectTarget->GetVert(geoFace.verts[i])) * projState.mat;

			//make sure last one does not cause a degenerate face
			vecB = Normalize(masterXYZs[2] - masterXYZs[0]);
			float dot = DotProd(vecA,vecB);
			if ((dot != 1.0f) && (dot != -1.0f))
				i = geoFace.numVerts; 
		}
		//compute our geo plane equation

		Point3 norm = Normalize(CrossProd(vecA,vecB));
		tm;
		MatrixFromNormal(norm,tm);
		tm.SetRow(3,masterXYZs[0]);
		itm = Inverse(tm);
	}


	//transform out points into local face space
	Tab<Point3> targetVerts;	
	int numTargetVerts = hitFaceCentroids.Count(); 
	targetVerts.SetCount( hitFaceCentroids.Count()+1);
	for (int i = 0; i <  hitFaceCentroids.Count();  i++)
	{
		targetVerts[i] = hitFaceCentroids[i] * itm;
		targetVerts[i].z = 0.0f;

	}
	targetVerts[hitFaceCentroids.Count()] = targetVerts[0];


	//build our test edges
	//loop through our hit face
	Tab<int> testFaces;
	Tab<int> newFaces;

	Tab<Point3> sourceVerts;

	Matrix3 fromSourceToTargetTM = Inverse(projState.mat) * itm;

	mProcessedFaces.ClearAll();
	
	BOOL iret = FALSE;
	for (int i = 0; i < checkFaces.Count(); i++)
	{
		//get our test faces
		testFaces.Append(1,&checkFaces[i],100);	
		
		while (testFaces.Count())
		{
			newFaces.SetCount(0,FALSE);
			for (int i = 0; i < testFaces.Count(); i++)
			{
				//test all our new faces
				int testFace = testFaces[i];


				//transform the Source verts into our target space
				if (!mProcessedFaces[testFace])
				{

					mProcessedFaces.Set(testFace,TRUE);
					//loop through there edges
					GenFace face = projState.objectSource->GetFace(testFace);
					int numEdges = face.numVerts;

					sourceVerts.SetCount(numEdges);
					for (int j = 0; j < numEdges; j++)
					{				
						sourceVerts[j] = (*projState.objectSource->GetVert(face.verts[j])) * itm;//fromSourceToTargetTM;
						sourceVerts[j].z = 0.0f;
					}

					for (int j = 0; j < numEdges; j++)
					{
					//if seam
						Point3 edgePoints[2];
						edgePoints[0] = sourceVerts[j];
						if (j == (numEdges-1))
							edgePoints[1] = sourceVerts[0];
						else
							edgePoints[1] = sourceVerts[j+1];
						//see if any of there edges intersect the target edges


						for (int k = 0; k < (numTargetVerts); k++)
						{	
							int edgeIndex = projState.objectTarget->GetFaceEdgeIndex(targetFace, j);
							GenEdge edge = projState.objectTarget->GetEdge(edgeIndex);
							if (IntersectEdge(targetVerts.Addr(k), edgePoints))
							{
								//if edge is a seam we are done
								int edgeIndex = projState.objectSource->GetFaceEdgeIndex(testFace, j);
								if (projState.bitClusterEdges[edgeIndex])
								{
									iret = TRUE;
								}

								GenEdge edge = projState.objectSource->GetEdge(edgeIndex);
								int opposingFaceA = (int)edge.f[0];

								if ((opposingFaceA != -1) && (opposingFaceA != testFace))
									newFaces.Append(1,&opposingFaceA,1000);

								opposingFaceA = (int)edge.f[1];

								if ((opposingFaceA != -1) && (opposingFaceA != testFace))
									newFaces.Append(1,&opposingFaceA,1000);
							}
						}												
					}
				}
			}
			testFaces = newFaces;
		}
	}
	return iret;

}


#define SAME_SIGNS(A, B) (((A>=0.0f) && (B>0.0f)) || ((A<0.0f) && (B<0.0f)))

#define  maxmin(x1, x2, min) (x1 >= x2 ? (min = x2, x1) : (min = x1, x2))

BOOL ProjectionModTypeUVW::IntersectEdge(Point3 *targetVerts, Point3 *sourceEdge)
{

	Point3 p1 = targetVerts[0];
	Point3 p2 = targetVerts[1]; 
	Point3 q1 = sourceEdge[0];
	Point3 q2 = sourceEdge[1];


	float a, b, c, d, det;  /* parameter calculation variables */
	float max1, max2, min1, min2; /* bounding box check variables */

	/*  First make the bounding box test. */
	max1 = maxmin(p1.x, p2.x, min1);
	max2 = maxmin(q1.x, q2.x, min2);
	if((max1 < min2) || (min1 > max2)) return(FALSE); /* no intersection */
	max1 = maxmin(p1.y, p2.y, min1);
	max2 = maxmin(q1.y, q2.y, min2);
	if((max1 < min2) || (min1 > max2)) return(FALSE); /* no intersection */

	/* See if the endpoints of the second segment lie on the opposite
	sides of the first.  If not, return 0. */
	a = (q1.x - p1.x) * (p2.y - p1.y) -
		(q1.y - p1.y) * (p2.x - p1.x);
	b = (q2.x - p1.x) * (p2.y - p1.y) -
		(q2.y - p1.y) * (p2.x - p1.x);
	if(a!=0.0f && b!=0.0f && SAME_SIGNS(a, b)) return(FALSE);

	/* See if the endpoints of the first segment lie on the opposite
	sides of the second.  If not, return 0.  */
	c = (p1.x - q1.x) * (q2.y - q1.y) -
		(p1.y - q1.y) * (q2.x - q1.x);
	d = (p2.x - q1.x) * (q2.y - q1.y) -
		(p2.y - q1.y) * (q2.x - q1.x);
	if(c!=0.0f && d!=0.0f && SAME_SIGNS(c, d) ) return(FALSE);

	/* At this point each segment meets the line of the other. */
	det = a - b;
	if(det == 0.0f) return(FALSE); /* The segments are colinear.  Determining */
	return(TRUE);
}

float ProjectionModTypeUVW::LineToPoint(Point3 p1, Point3 l1, Point3 l2, float &u)
{
	Point3 VectorA,VectorB,VectorC;
	double Angle;
	double dist = 0.0f;
	VectorA = l2-l1;
	VectorB = p1-l1;
	float dot = DotProd(Normalize(VectorA),Normalize(VectorB));
	if (dot == 1.0f) dot = 0.99f;
	Angle =  acos(dot);
	if (Angle > (3.14/2.0))
	{
		dist = Length(p1-l1);
		u = 0.0f;
	}
	else
	{
		VectorA = l1-l2;
		VectorB = p1-l2;
		dot = DotProd(Normalize(VectorA),Normalize(VectorB));
		if (dot == 1.0f) dot = 0.99f;
		Angle = acos(dot);
		if (Angle > (3.14/2.0))
		{
			dist = Length(p1-l2);
			u = 1.0f;
		}
		else
		{
			double hyp;
			hyp = Length(VectorB);
			dist =  sin(Angle) * hyp;
			double du =  (cos(Angle) * hyp);
			double a = Length(VectorA);
			if ( a== 0.0f)
				return 0.0f;
			else u = (float)((a-du) / a);
		}
	}
	return (float) dist;
}

int ProjectionModTypeUVW::FindMasterFace(ProjectionState &projState, int iTargetFace, Tab<int> &tabFaceHits, int centerFaceHit)
{

	int masterFace = -1;
	//loop through hit faces looking for a seam
	//this is a simplistic check to keep things fast, we can expand this but it would
	//really slow things down
	BOOL seamFound = FALSE;
	Point3 seam[2];
	BitArray hitFaces;
	hitFaces.SetSize(projState.objectSource->NumFaces());
	hitFaces.ClearAll();
	for (int i = 0; i < tabFaceHits.Count(); i++) 
	{
		//get the seam end points
		int faceIndex = tabFaceHits[i];
		hitFaces.Set(faceIndex);
	}

	for (int i = 0; i < tabFaceHits.Count(); i++) 
	{
		//get the seam end points
		int faceIndex = tabFaceHits[i];
		GenFace face = projState.objectSource->GetFace(faceIndex);
		int numEdges = face.numVerts;
		for (int j = 0; j < numEdges; j++)
		{
			int edgeIndex = projState.objectSource->GetFaceEdgeIndex(faceIndex, j);
			if( projState.bitClusterEdges[edgeIndex] )
			{
				//see if opposing edge is a processed face
				GenEdge edge = projState.objectSource->GetEdge(edgeIndex);
				if ((edge.f[0] != -1) && (edge.f[1] != -1))
				{
					if ((hitFaces[edge.f[0]]) && (hitFaces[edge.f[1]]))
					{					
						int a = edge.v[0];
						int b = edge.v[1];
						seam[0] = *projState.objectSource->GetVert(a);
						seam[1] = *projState.objectSource->GetVert(b);
						seamFound = TRUE;
					}
				}
			}						
		}
	}

	if (seamFound)
	{
		GenFace face = projState.objectTarget->GetFace(iTargetFace);
		int numVerts = face.numVerts;
		int dist = -1.0f;
		//loop through our targets point
		for (int j = 0; j < numVerts; j++)
		{
			if (tabFaceHits[j] != -1)
			{			
				//find the one farthest from the line
				int index = face.verts[j];
				Point3 p = (*projState.objectTarget->GetVert(index)) * projState.mat;
				float u;
				float d = LineToPoint(p,seam[0],seam[1],u);
				//that is our master face since that face will most likely have the most coverage
				if (d > dist)
				{
					dist = d;
					masterFace = tabFaceHits[j];
//					iCenterFaceHit = -1;
				}
			}
		}
	}

	//if we still dont have a master face use centroids
	//and just find the face that is closest by centroids
	if (masterFace == -1)
	{
		Tab<Point3> centroids;
		Tab<int> faces;
		centroids.SetCount(tabFaceHits.Count());
		faces.SetCount(tabFaceHits.Count());

		//get our hit face centroid
		for (int i = 0; i < tabFaceHits.Count(); i++) 
		{
			GenFace face = projState.objectSource->GetFace(tabFaceHits[i]);
			int numEdges = face.numVerts;
			Point3 center(0.0f,0.0f,0.0f);
			for (int j = 0; j < numEdges; j++)
			{
				center += (*projState.objectSource->GetVert(face.verts[j]));
			}
			if (numEdges)
				center = center/numEdges;
			centroids[i] = center;
			faces[i] = tabFaceHits[i];
		}

		//get our center face centroid
		if (centerFaceHit != -1)
		{
			GenFace face = projState.objectSource->GetFace(centerFaceHit);
			int numEdges = face.numVerts;
			Point3 center(0.0f,0.0f,0.0f);
			for (int j = 0; j < numEdges; j++)
			{
				center += (*projState.objectSource->GetVert(face.verts[j]));
			}
			if (numEdges)
				center = center/numEdges;
			centroids.Append(1,&center);
			faces.Append(1,&centerFaceHit);
		}


		//get our target centroid
		Point3 targetCentroid(0.0f,0.0f,0.0f);
		GenFace face = projState.objectTarget->GetFace(iTargetFace);
		int numEdges = face.numVerts;
		Point3 center(0.0f,0.0f,0.0f);
		for (int j = 0; j < numEdges; j++)
		{
			center += (*projState.objectTarget->GetVert(face.verts[j]));
		}
		if (numEdges)
			center = center/numEdges;
		targetCentroid = center * projState.mat;
		if (centroids.Count())
		{
			float closestDist = Length(targetCentroid - centroids[0]);
			int closestID = faces[0];
			//find closest
			for (int i = 1; i < centroids.Count(); i++)
			{
				float d =  Length(targetCentroid - centroids[i]);
				if (d < closestDist)
				{
					closestDist = d;
					closestID = faces[i];
				}
			}			
			masterFace = closestID;

		}
	}
	return masterFace;
}

BOOL ProjectionModTypeUVW::CheckAngleRatio(ProjectionState &projState, int iTargetFace, float ratioThresold, float &largestChange)
{
	BOOL edgeRatioError = FALSE;
	GenFace geoFace = GetChannelFace(projState.objectTarget, iTargetFace, VERTEX_CHANNEL_NUM);
	int numEdges = geoFace.numVerts;
	Tab<float> uvwAngle;
	Tab<float> xyzAngle;
	uvwAngle.SetCount(numEdges);
	xyzAngle.SetCount(numEdges);

	//compute our UVW and XZY edge angles
	for (int j = 0; j < numEdges; j++)
	{
		int vIndex = projState.pData->mFaces[iTargetFace].verts[j];	
		Point3 uvw1 = projState.pData->mP3Data[vIndex];
		uvw1.z = 0.0f;
		
		vIndex = projState.pData->mFaces[iTargetFace].verts[(j+1)%numEdges];
		Point3 uvw2 = projState.pData->mP3Data[vIndex];
		uvw2.z = 0.0f;

		vIndex = projState.pData->mFaces[iTargetFace].verts[(j+2)%numEdges];
		Point3 uvw3 = projState.pData->mP3Data[vIndex];
		uvw3.z = 0.0f;

		Point3 veca = Normalize(uvw1 - uvw2);
		Point3 vecb = Normalize(uvw3 - uvw2);
		float dot = DotProd(veca,vecb);
		uvwAngle[j] = dot;



		vIndex = geoFace.verts[j];
		Point3 xyz1 = (*GetChannelVert(projState.objectTarget, vIndex, VERTEX_CHANNEL_NUM));

		vIndex = geoFace.verts[(j+1)%numEdges];
		Point3 xyz2 = (*GetChannelVert(projState.objectTarget, vIndex, VERTEX_CHANNEL_NUM));

		vIndex = geoFace.verts[(j+2)%numEdges];
		Point3 xyz3 = (*GetChannelVert(projState.objectTarget, vIndex, VERTEX_CHANNEL_NUM));

		veca = Normalize(xyz1 - xyz2);
		vecb = Normalize(xyz3 - xyz2);
		dot = DotProd(veca,vecb);

		xyzAngle[j] = dot;
	}



	//check to see if that ratio is out of whack
	largestChange = 0.0f;
	for (int j = 0; j < numEdges; j++)
	{			
		float angleChange =  fabs(xyzAngle[j]-uvwAngle[j]);
		if (angleChange > largestChange)
			largestChange = angleChange;
		if (angleChange > ratioThresold)
		{
			edgeRatioError = TRUE;
		}
	}

	return edgeRatioError;
}

BOOL ProjectionModTypeUVW::CheckEdgeRatio(ProjectionState &projState, int iTargetFace, float ratioThresold, float &largestChange)
{
	BOOL edgeRatioError = FALSE;
	float xyzTotalEdgeLength = 0.0f;
	float uvwTotalEdgeLength = 0.0f;
	GenFace geoFace = GetChannelFace(projState.objectTarget, iTargetFace, VERTEX_CHANNEL_NUM);
	int numEdges = geoFace.numVerts;
	Tab<float> uvwEdgeDists;
	Tab<float> xyzEdgeDists;
	uvwEdgeDists.SetCount(numEdges);
	xyzEdgeDists.SetCount(numEdges);

	//compute our UVW and XZY edge distances
	for (int j = 0; j < numEdges; j++)
	{
		int vIndex = projState.pData->mFaces[iTargetFace].verts[j];
		Point3 uvw1 = projState.pData->mP3Data[vIndex];
		uvw1.z = 0.0f;
		vIndex = projState.pData->mFaces[iTargetFace].verts[(j+1)%numEdges];
		Point3 uvw2 = projState.pData->mP3Data[vIndex];
		uvw2.z = 0.0f;

		vIndex = geoFace.verts[j];
		Point3 xyz1 = (*GetChannelVert(projState.objectTarget, vIndex, VERTEX_CHANNEL_NUM));
		vIndex = geoFace.verts[(j+1)%numEdges];
		Point3 xyz2 = (*GetChannelVert(projState.objectTarget, vIndex, VERTEX_CHANNEL_NUM));

		float uvwDist = Length(uvw2 - uvw1);
		float xyzDist = Length(xyz2 - xyz1);

		xyzTotalEdgeLength += xyzDist;
		uvwTotalEdgeLength += uvwDist;
		xyzEdgeDists[j] = xyzDist;
		uvwEdgeDists[j] = uvwDist;
	}

	float ratio = -1.0f;

	//get our ratio of UVW vs XYZ edges
	for (int j = 0; j < numEdges; j++)
	{
		if (uvwEdgeDists[j] != 0.0f)
		{
			float testRatio = xyzEdgeDists[j]/uvwEdgeDists[j];							
			if ((testRatio < ratio) || (ratio == -1.0f))
				ratio = testRatio;
		}
	}

	//check to see if that ratio is out of whack
	if (ratio != -1.0f)
	{					
		for (int j = 0; j < numEdges; j++)
		{
			uvwEdgeDists[j] *= ratio;
			float edgeLengthChange =  xyzEdgeDists[j]/uvwEdgeDists[j];
			if (edgeLengthChange > ratioThresold)
			{				
				edgeRatioError = TRUE;
			}
		}
	}
	return edgeRatioError;
}

void ProjectionModTypeUVW::ReprojectFace(ProjectionState &projState, int iTargetFace, int masterFace, Tab<int> &tabFaceHits, Tab<Point3> &results)
{
	results.SetCount(0,FALSE);
	// We need create and calculate new map verts
	//get our master UVs position
	Point3 masterUVs[3];
	Point3 masterXYZs[3];

	GenFace faceTarget = projState.objectTarget->GetFace(iTargetFace);

	//build our face space
	GenFace face = GetChannelFace(projState.objectSource, masterFace, miSourceMapChannel);
	GenFace geoFace = GetChannelFace(projState.objectSource, masterFace, VERTEX_CHANNEL_NUM);

	masterUVs[0] = (*GetChannelVert(projState.objectSource, face.verts[0], miSourceMapChannel));
	masterUVs[0].z = 0.0f;
	masterXYZs[0] = (*projState.objectSource->GetVert(geoFace.verts[0]));
	masterUVs[1] = (*GetChannelVert(projState.objectSource, face.verts[1], miSourceMapChannel));
	masterUVs[1].z = 0.0f;
	masterXYZs[1] = (*projState.objectSource->GetVert(geoFace.verts[1]));

	//compute our geo plane equation
	Point3 vecGeoA = Normalize(masterXYZs[1] - masterXYZs[0]);
	Point3 vecGeoB = Normalize(masterXYZs[2] - masterXYZs[0]);

	for (int i = 2; i < face.numVerts; i++)
	{		
		masterUVs[2] = (*GetChannelVert(projState.objectSource, face.verts[i], miSourceMapChannel));
		masterUVs[2].z = 0.0f;
		masterXYZs[2] = (*projState.objectSource->GetVert(geoFace.verts[i]));

		//make sure last one does not cause a degenerate face
		vecGeoB = Normalize(masterXYZs[2] - masterXYZs[0]);
		float dot = DotProd(vecGeoA,vecGeoB);
		if ((dot != 1.0f) && (dot != -1.0f))
			i = face.numVerts; 
	}

	Point3 norm = Normalize(CrossProd(vecGeoA,vecGeoB));
	Matrix3 tm;
	MatrixFromNormal(norm,tm);
	tm.SetRow(3,masterXYZs[0]);
	Matrix3 itm = Inverse(tm);

	//transfer the target into or face space and use bary coords to compute approx UVW
	for( int j=0; j<faceTarget.numVerts; j++ ) 
	{
		//project our point onto the master face				
		Point3 p = (*projState.objectTarget->GetVert(faceTarget.verts[j]));
		p = p * projState.mat;
		//better math here would make this faster
		p = p * itm;
		p.z = 0.0f;
		p = p * tm ;
		Point3 bry;
		bry = BaryCoords(masterXYZs[0], masterXYZs[1], masterXYZs[2], p);

		//compute the new uvw using the new bary coords
		Point3 uv(0.0f,0.0f,0.0f);
		uv.x = masterUVs[0].x * bry.x + masterUVs[1].x * bry.y + masterUVs[2].x * bry.z; 
		uv.y = masterUVs[0].y * bry.x + masterUVs[1].y * bry.y + masterUVs[2].y * bry.z; 
		uv.z = 0.0f;//masterXYZs[0].z * bry.x + masterXYZs[1].z * bry.y + masterXYZs[2].z * bry.z; 
		results.Append(1,&uv,10);


	}
}
