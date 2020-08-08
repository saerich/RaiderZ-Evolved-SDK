//*****************************************************************************
// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies 
// this software in either electronic or hard copy form.   
//
//*****************************************************************************
//
//	FILE: EditPoly.cpp
//  DESCRIPTION: Edit Poly Modifier
//	CREATED BY:	Steve Anderson, based on Face Extrude modifier by Berteig, and my own Poly Select modifier.
//
//	HISTORY: created March 2002
//*****************************************************************************

#include "epoly.h"
#include "EditPoly.h"
#include "MeshDLib.h"
#include "setkeymode.h"
#include "EditPolyUI.h"
#include <IPathConfigMgr.h>
#include "iEPoly.h" 
#include "..\editablepoly\EPolyManipulatorGrip.h"
#include <vector>

static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Edge(2);
static GenSubObjType SOT_Border(9);
static GenSubObjType SOT_Face(4);
static GenSubObjType SOT_Element(5);

//--- ClassDescriptor and class vars ---------------------------------

Tab<PolyOperation *> EditPolyMod::mOpList;
IObjParam *EditPolyMod::ip = NULL;
EditPolyMod *EditPolyMod::mpCurrentEditMod = NULL;
bool EditPolyMod::mUpdateCachePosted = FALSE;
DWORD EditPolyMod::mHitLevelOverride = 0;
DWORD EditPolyMod::mDispLevelOverride = 0;
bool EditPolyMod::mHitTestResult = false;
bool EditPolyMod::mDisplayResult = false;
bool EditPolyMod::mIgnoreNewInResult = false;
bool EditPolyMod::mForceIgnoreBackfacing = false;
bool EditPolyMod::mSuspendConstraints = false;
bool EditPolyMod::mSliceMode(false);
int EditPolyMod::mLastCutEnd(-1);
int EditPolyMod::mAttachType(0);
bool EditPolyMod::mAttachCondense(true);
int EditPolyMod::mCreateShapeType(IDC_SHAPE_SMOOTH);
int EditPolyMod::mCloneTo(IDC_CLONE_ELEM);
bool EditPolyMod::mDetachToObject(true);
bool EditPolyMod::mDetachAsClone(false);
TSTR EditPolyMod::mCreateShapeName(_T(""));
TSTR EditPolyMod::mDetachName(_T(""));
Tab<ActionItem*> EditPolyMod::mOverrides;
IParamMap2 *EditPolyMod::mpDialogAnimate = NULL;
IParamMap2 *EditPolyMod::mpDialogSelect = NULL;
IParamMap2 *EditPolyMod::mpDialogSoftsel = NULL;
IParamMap2 *EditPolyMod::mpDialogGeom = NULL;
IParamMap2 *EditPolyMod::mpDialogSubobj = NULL;
IParamMap2 *EditPolyMod::mpDialogSurface = NULL;
IParamMap2 *EditPolyMod::mpDialogSurfaceFloater = NULL;
IParamMap2 *EditPolyMod::mpDialogFaceSmooth = NULL;
IParamMap2 *EditPolyMod::mpDialogFaceSmoothFloater = NULL;
IParamMap2 *EditPolyMod::mpDialogOperation = NULL;
IParamMap2 *EditPolyMod::mpDialogPaintDeform = NULL;
bool EditPolyMod::mRollupAnimate = false;
bool EditPolyMod::mRollupSelect = false;
bool EditPolyMod::mRollupSoftsel = true;
bool EditPolyMod::mRollupGeom = false;
bool EditPolyMod::mRollupSubobj = false;
bool EditPolyMod::mRollupSurface = false;
bool EditPolyMod::mRollupFaceSmooth = false;
bool EditPolyMod::mRollupPaintDeform = true;
EditPolyBackspaceUser EditPolyMod::backspacer;
EditPolyActionCB *EditPolyMod::mpEPolyActions = NULL;

SelectModBoxCMode *        EditPolyMod::selectMode     = NULL;
MoveModBoxCMode*        EditPolyMod::moveMode        = NULL;
RotateModBoxCMode*         EditPolyMod::rotMode     = NULL;
UScaleModBoxCMode*         EditPolyMod::uscaleMode      = NULL;
NUScaleModBoxCMode*        EditPolyMod::nuscaleMode     = NULL;
SquashModBoxCMode*         EditPolyMod::squashMode      = NULL;
EditPolyCreateVertCMode*   EditPolyMod::createVertMode  = NULL;
EditPolyCreateEdgeCMode*   EditPolyMod::createEdgeMode   = NULL;
EditPolyCreateFaceCMode*   EditPolyMod::createFaceMode   = NULL;
EditPolyAttachPickMode*    EditPolyMod::attachPickMode   = NULL;
EditPolyShapePickMode *    EditPolyMod::mpShapePicker = NULL;
EditPolyDivideEdgeCMode*   EditPolyMod::divideEdgeMode   = NULL;
EditPolyDivideFaceCMode *  EditPolyMod::divideFaceMode   = NULL;
EditPolyBridgeBorderCMode *   EditPolyMod::bridgeBorderMode = NULL;
EditPolyBridgeEdgeCMode *  EditPolyMod::bridgeEdgeMode = NULL;
EditPolyBridgePolygonCMode* EditPolyMod::bridgePolyMode = NULL;
EditPolyExtrudeCMode *     EditPolyMod::extrudeMode      = NULL;
EditPolyExtrudeVECMode *   EditPolyMod::extrudeVEMode = NULL;
EditPolyChamferCMode *     EditPolyMod::chamferMode      = NULL;
EditPolyBevelCMode*        EditPolyMod::bevelMode     = NULL;
EditPolyInsetCMode*        EditPolyMod::insetMode = NULL;
EditPolyOutlineCMode*      EditPolyMod::outlineMode = NULL;
EditPolyCutCMode *         EditPolyMod::cutMode    = NULL;
EditPolyQuickSliceCMode *  EditPolyMod::quickSliceMode = NULL;
EditPolyWeldCMode *        EditPolyMod::weldMode      = NULL;
EditPolyHingeFromEdgeCMode *EditPolyMod::hingeFromEdgeMode = NULL;
EditPolyPickHingeCMode *   EditPolyMod::pickHingeMode = NULL;
EditPolyPickBridge1CMode * EditPolyMod::pickBridgeTarget1 = NULL;
EditPolyPickBridge2CMode * EditPolyMod::pickBridgeTarget2 = NULL;
EditPolyEditTriCMode *     EditPolyMod::editTriMode      = NULL;
EditPolyTurnEdgeCMode *    EditPolyMod::turnEdgeMode = NULL;
//EditPolyEditSSMode *    EditPolyMod::editSSMode = NULL;
EditSSMode *			   EditPolyMod::editSSMode = NULL;

//--- EditPolyMod methods -------------------------------

EditPolyMod::EditPolyMod() : selLevel(EPM_SL_OBJECT), mpParams(NULL), 
         mpSliceControl(NULL), mpCurrentOperation(NULL),
         mPreserveMapSettings (true,false), mpPointMaster(NULL),mLastConstrainType(0) {
   GetEditPolyDesc()->MakeAutoParamBlocks(this);

   ReplaceReference (EDIT_SLICEPLANE_REF, NewDefaultMatrix3Controller());
   Matrix3 tm(1);
   mpSliceControl->GetValue (0, &tm, FOREVER, CTRL_RELATIVE);

   if (mOpList.Count () == 0) InitializeOperationList ();
   mCurrentDisplayFlags = 0;

   mLastOperation = ep_op_null;

   LoadPreferences ();

   if (mCreateShapeName.Length() == 0) mCreateShapeName = GetString (IDS_SHAPE);
   if (mDetachName.Length() == 0) mDetachName = GetString(IDS_OBJECT);

   mRingSpinnerValue       = 0;
   mLoopSpinnerValue       = 0;
   mCtrlKey             = false;
   mAltKey                 = false;
   m_Transforming          = false; 
   m_CageColor             = GetUIColor(COLOR_GIZMOS);
   m_SelectedCageColor        = GetUIColor(COLOR_SEL_GIZMOS);
   m_iColorSelectedCageSwatch = NULL; 
   m_iColorCageSwatch         = NULL; 
   mMeshHit = false;
   mSuspendCloneEdges = false;

   theHold.Suspend();
   ReplaceReference (EDIT_MASTER_POINT_REF, NewDefaultMasterPointController());
   theHold.Resume();

   //mz  prototype
   mSelLevelOverride = -1;

   mManipGrip.Init(this);
}

void EditPolyMod::ResetClassParams (BOOL fileReset) {
   EditPolyMod::mRollupAnimate = false;
   EditPolyMod::mRollupSelect = false;
   EditPolyMod::mRollupSoftsel = true;
   EditPolyMod::mRollupGeom = false;
   EditPolyMod::mRollupSubobj = false;
   EditPolyMod::mRollupSurface = false;
   EditPolyMod::mRollupFaceSmooth = false;
   EditPolyMod::mRollupPaintDeform = true;
   EditPolyMod::mUpdateCachePosted = false;
   EditPolyMod::mHitLevelOverride = 0;
   EditPolyMod::mDispLevelOverride = 0;
   EditPolyMod::mHitTestResult = false;
   EditPolyMod::mDisplayResult = false;
   EditPolyMod::mIgnoreNewInResult = false;
   EditPolyMod::mForceIgnoreBackfacing = false;
   EditPolyMod::mSuspendConstraints = false;
   EditPolyMod::mSliceMode = false;
   EditPolyMod::mLastCutEnd = -1;
   EditPolyMod::mAttachType = 0;
   EditPolyMod::mAttachCondense = false;
   EditPolyMod::mCreateShapeType = IDC_SHAPE_SMOOTH;
   EditPolyMod::mCloneTo = IDC_CLONE_ELEM;
   EditPolyMod::mDetachToObject = true;
   EditPolyMod::mDetachAsClone = false;
   EditPolyMod::mCreateShapeName = GetString(IDS_SHAPE);
   EditPolyMod::mDetachName = GetString(IDS_OBJECT);

   LoadPreferences ();
}

// The following strings do not need to be localized
const TCHAR* EditPolyMod::kINI_FILE_NAME = _T("EditPolyMod.ini");
const TCHAR* EditPolyMod::kINI_SECTION_NAME = _T("EPolySettings");
const TCHAR* EditPolyMod::kINI_CREATE_SHAPE_SMOOTH_OPT  = _T("CreateShapeSmooth");
const TCHAR* EditPolyMod::kINI_DETACH_TO_OBJECT_OPT = _T("DetachToObject");
const TCHAR* EditPolyMod::kINI_DETACH_AS_CLONE_OPT = _T("DetachAsClone");

TSTR EditPolyMod::GetINIFileName()
{
	IPathConfigMgr* pPathMgr = IPathConfigMgr::GetPathConfigMgr();
	TSTR iniFileName(pPathMgr->GetDir(APP_PLUGCFG_DIR));
	iniFileName += _T("\\");
	iniFileName += kINI_FILE_NAME;
	return iniFileName;
}

void EditPolyMod::SavePreferences ()
{
	TSTR iniFileName = EditPolyMod::GetINIFileName();
	TCHAR buf[MAX_PATH] = {0};
	// The ini file should always be saved and read with the same locale settings
	MaxLocaleHandler localeGuard;

   int createShapeSmooth = (mCreateShapeType == IDC_SHAPE_SMOOTH) ? 1 : 0;
	_stprintf_s(buf, sizeof(buf), "%d", createShapeSmooth);
	BOOL res = WritePrivateProfileString(kINI_SECTION_NAME, kINI_CREATE_SHAPE_SMOOTH_OPT, buf, iniFileName);
	DbgAssert(res);
	_stprintf_s(buf, sizeof(buf), "%d", mDetachToObject);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kINI_DETACH_TO_OBJECT_OPT, buf, iniFileName);
	DbgAssert(res);
	_stprintf_s(buf, sizeof(buf), "%d", mDetachAsClone);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kINI_DETACH_AS_CLONE_OPT, buf, iniFileName);
	DbgAssert(res);
}

void EditPolyMod::LoadPreferences ()
{
	TSTR iniFileName = EditPolyMod::GetINIFileName();
	TCHAR buf[MAX_PATH] = {0};
	TCHAR defaultVal[MAX_PATH] = {0};
	IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();
	if (pathMgr->DoesFileExist(iniFileName)) 
	{
		// The ini file should always be saved and read with the same locale settings
		MaxLocaleHandler localeGuard;
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", ((mCreateShapeType == IDC_SHAPE_SMOOTH) ? 1 : 0));
		if (GetPrivateProfileString(kINI_SECTION_NAME, kINI_CREATE_SHAPE_SMOOTH_OPT, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int createShapeSmooth = atoi(buf);
			mCreateShapeType = (createShapeSmooth ? IDC_SHAPE_SMOOTH : IDC_SHAPE_LINEAR);
   }
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mDetachToObject);
		if (GetPrivateProfileString(kINI_SECTION_NAME, kINI_DETACH_TO_OBJECT_OPT, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int detachToObject = atoi(buf);
      mDetachToObject = (detachToObject != 0);
   }
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", mDetachAsClone);
		if (GetPrivateProfileString(kINI_SECTION_NAME, kINI_DETACH_AS_CLONE_OPT, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int detachAsClone = atoi(buf);
      mDetachAsClone = (detachAsClone != 0);
   }
	}
}

RefTargetHandle EditPolyMod::Clone(RemapDir& remap) {
   EditPolyMod *mod = new EditPolyMod();
   mod->selLevel = selLevel;
   mod->mPreserveMapSettings = mPreserveMapSettings;
   mod->mPointRange = mPointRange;
   mod->mPointNode = mPointNode;
   mod->paintDeformActive = paintDeformActive;
   mod->paintSelActive = paintSelActive;
   mod->mManipGrip = mManipGrip;
   mod->ReplaceReference (EDIT_PBLOCK_REF, remap.CloneRef(mpParams));
   mod->ReplaceReference (EDIT_SLICEPLANE_REF, remap.CloneRef(mpSliceControl));
   if (mPointControl.Count()) {
      mod->AllocPointControllers (mPointControl.Count());
      for (int i=0; i<mPointControl.Count(); i++) {
         if (mPointControl[i] == NULL) continue;
         mod->ReplaceReference ( EDIT_POINT_BASE_REF + i, remap.CloneRef(mPointControl[i]));
      }
   }

   mod->SetSelectMode(mod->GetSelectMode());
   BaseClone(this, mod, remap);
   return mod;
}

BOOL EditPolyMod::DependOnTopology(ModContext &mc) {
   if (mpCurrentOperation) return true;
   if (!mc.localData) return false;
   EditPolyData *pData = (EditPolyData *) mc.localData;
   return pData->HasCommittedOperations();
}

void EditPolyMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {

   if (!os->obj->IsSubClassOf (polyObjectClassID)) return;
   PolyObject *pObj = (PolyObject *) os->obj;
   MNMesh &mesh = pObj->GetMesh();

   EditPolyData *pData  = (EditPolyData *) mc.localData;
   if (!pData) mc.localData = pData = new EditPolyData (pObj, t);
   if (pData->CacheValidity().InInterval(t) && pData->GetMesh()) {
      mesh = *pData->GetMesh();
   } else {
      if (!mesh.GetFlag (MN_MESH_FILLED_IN)) mesh.FillInMesh();
      mesh.ClearSpecifiedNormals(); // We always lose specified normals in this mod.

      // In order to "Get Stack Selection", we need to preserve it somehow, even though
      // we often set the MN_SEL flag according to temporary needs.
      // Here, we copy it over, in all selection levels, to the MN_EDITPOLY_STACK_SELECTION flag.
      mesh.ClearVFlags (MN_EDITPOLY_STACK_SELECT);
      mesh.ClearEFlags (MN_EDITPOLY_STACK_SELECT);
      mesh.ClearFFlags (MN_EDITPOLY_STACK_SELECT);
      mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_EDITPOLY_STACK_SELECT, MNM_SL_VERTEX, MN_SEL);
      mesh.PropegateComponentFlags (MNM_SL_EDGE, MN_EDITPOLY_STACK_SELECT, MNM_SL_EDGE, MN_SEL);
      mesh.PropegateComponentFlags (MNM_SL_FACE, MN_EDITPOLY_STACK_SELECT, MNM_SL_FACE, MN_SEL);

      pData->ApplyAllOperations (mesh);
      pData->SetCache (pObj, t);

      //May not be necessary, already handled by SetCache() / SynchBitArrays()
      UpdatePaintObject(pData);
   }


   //Hide the sub objects of the object's mesh (Vertices,Edges,Faces...). 
   pData->ApplyHide (mesh);

   //Defect 751562 - 733563 
   //Hide the sub objects of the modifier's mesh (Vertices,Edges,Faces...).
   pData->ApplyHide (*pData->GetMesh());  

   // Set the selection:
   mesh.selLevel = meshSelLevel[selLevel];
   mesh.dispFlags = mCurrentDisplayFlags;
   pData->GetMesh()->selLevel = meshSelLevel[selLevel];
   pData->GetMesh()->dispFlags = mCurrentDisplayFlags;

   Interval ourValidity(FOREVER);
   Interval selectValidity(FOREVER);
   int stackSel;
   mpParams->GetValue (epm_stack_selection, t, stackSel, selectValidity);
   if (stackSel)
   {
      mesh.ClearVFlags (MN_SEL);
      mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_SEL, MNM_SL_VERTEX, MN_EDITPOLY_STACK_SELECT);
      mesh.ClearEFlags (MN_SEL);
      mesh.PropegateComponentFlags (MNM_SL_EDGE, MN_SEL, MNM_SL_EDGE, MN_EDITPOLY_STACK_SELECT);
      mesh.ClearFFlags (MN_SEL);
      mesh.PropegateComponentFlags (MNM_SL_FACE, MN_SEL, MNM_SL_FACE, MN_EDITPOLY_STACK_SELECT);

      pData->GetMesh()->ClearVFlags (MN_SEL);
      pData->GetMesh()->PropegateComponentFlags (MNM_SL_VERTEX, MN_SEL, MNM_SL_VERTEX, MN_EDITPOLY_STACK_SELECT);
      pData->GetMesh()->ClearEFlags (MN_SEL);
      pData->GetMesh()->PropegateComponentFlags (MNM_SL_EDGE, MN_SEL, MNM_SL_EDGE, MN_EDITPOLY_STACK_SELECT);
      pData->GetMesh()->ClearFFlags (MN_SEL);
      pData->GetMesh()->PropegateComponentFlags (MNM_SL_FACE, MN_SEL, MNM_SL_FACE, MN_EDITPOLY_STACK_SELECT);

      selectValidity &= pObj->ChannelValidity (t, SELECT_CHAN_NUM);
   }
   else
   {
      mesh.VertexSelect (pData->mVertSel);
      mesh.EdgeSelect (pData->mEdgeSel);
      mesh.FaceSelect (pData->mFaceSel);
      pData->GetMesh()->VertexSelect (pData->mVertSel);
      pData->GetMesh()->EdgeSelect (pData->mEdgeSel);
      pData->GetMesh()->FaceSelect (pData->mFaceSel);
   }

   float *softsel = SoftSelection (t, pData, selectValidity);
   if (softsel)
   {
      mesh.SupportVSelectionWeights ();
      float *vs = mesh.getVSelectionWeights ();
      int nv = mesh.VNum();
      memcpy (vs, softsel, nv*sizeof(float));

      pData->GetMesh()->SupportVSelectionWeights ();
      vs = pData->GetMesh()->getVSelectionWeights ();
      memcpy (vs, softsel, nv*sizeof(float));
   }
   else
   {
      mesh.setVDataSupport (VDATA_SELECT, false);
      pData->GetMesh()->setVDataSupport (VDATA_SELECT, false);
   }

   PolyOperation *chosenOp = GetPolyOperation();
   int msl = meshSelLevel[selLevel];
   Interval geomValidity = FOREVER;

   if (chosenOp != NULL) {
      // Selection could change any other channel, so match validities appropriately:
      ourValidity &= selectValidity;

      int id = chosenOp->OpID();
      int numBefore=0;
      if (((id==ep_op_weld_vertex)||(id==ep_op_weld_edge)) && mpDialogOperation)
      {
         // Weld dialog has readouts of before, after vertex counts.
         // This seems like a logical place to update them.

         // Get the before figure.
         numBefore = mesh.numv;
         for (int i=0; i<mesh.numv; i++) {
            if (mesh.v[i].GetFlag (MN_DEAD)) numBefore--;
         }
      } else if ( (id == ep_op_weld_vertex) ||(id == ep_op_weld_edge))
	  {
		  if(GetIGripManager()->GetActiveGrip() == &mWeldVerticesGrip)
		  {
			  numBefore = mesh.numv;
			  for (int i=0; i<mesh.numv; i++) 
			  {
				  if (mesh.v[i].GetFlag (MN_DEAD)) numBefore--;
			  }
		  } else if(GetIGripManager()->GetActiveGrip() == &mWeldEdgesGrip)
		  {
			  numBefore = mesh.numv;
			  for(int i=0; i<mesh.numv; i++)
			  {
				  if(mesh.v[i].GetFlag(MN_DEAD)) numBefore--;
			  }
		  }
	  }

      chosenOp->RecordSelection (mesh, this, pData);
      chosenOp->SetUserFlags (mesh);
      chosenOp->GetValues (this, t, ourValidity);
      if (chosenOp->ModContextTM())
      {
         if (mc.tm) *(chosenOp->ModContextTM()) = Inverse(*mc.tm);
         else chosenOp->ModContextTM()->IdentityMatrix();
      }
      chosenOp->GetNode (this, t, ourValidity);

      // If we're doing a transform and have point controllers,
      // update the local transform data.
      if (chosenOp->OpID() == ep_op_transform) UpdateTransformData (t, geomValidity, pData);

      bool ret = chosenOp->Apply (mesh, pData->GetPolyOpData());
      if (ret && chosenOp->CanDelete ()) mesh.CollapseDeadStructs ();

      if (ret && (chosenOp->OpID()==ep_op_cut) && pData->GetFlag (kEPDataLastCut))
      {
         LocalCutData *pCut = (LocalCutData *) pData->GetPolyOpData();
         mLastCutEnd = pCut->GetEndVertex ();
         pData->ClearFlag (kEPDataLastCut);
      }

      if (((id==ep_op_weld_vertex)||(id==ep_op_weld_edge)) && mpDialogOperation) {
         // Weld dialog has readouts of before, after vertex counts.
         // This seems like a logical place to update them.
         HWND hWnd = GetDlgHandle (ep_settings);
         if (hWnd) {
            TSTR buf;
            HWND hBefore = GetDlgItem (hWnd, IDC_WELD_BEFORE);
            if (hBefore) {
               buf.printf ("%d", numBefore);
               LPCTSTR lbuf = static_cast<LPCTSTR>(buf);
               SendMessage (hBefore, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(lbuf));
            }

            HWND hAfter = GetDlgItem (hWnd, IDC_WELD_AFTER);
            if (hAfter) {
               int numAfter = mesh.numv;
               for (int i=0; i<mesh.numv; i++) {
                  if (mesh.v[i].GetFlag (MN_DEAD)) numAfter--;
               }
               buf.printf ("%d", numAfter);
               LPCTSTR lbuf = static_cast<LPCTSTR>(buf);
               SendMessage (hAfter, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(lbuf));
            }
         }		 
      } else if( (id == ep_op_weld_vertex)||(id== ep_op_weld_edge) )
	  {
		  // For Grips
		  //Note we have also added grips as dialog replacements so add the strings to the grips
		  if(GetIGripManager()->GetActiveGrip() == &mWeldVerticesGrip)
		  {
			  int numAfter = mesh.numv;
			  for(int i=0;i<mesh.numv;i++)
			  {
				  if(mesh.v[i].GetFlag(MN_DEAD))  numAfter--;
			  }
			  mWeldVerticesGrip.SetNumVerts(numBefore,numAfter);
		  }
		  else if( GetIGripManager()->GetActiveGrip() == &mWeldEdgesGrip)
		  {
              int numAfter = mesh.numv;
			  for(int i=0; i<mesh.numv;i++)
			  {
				  if(mesh.v[i].GetFlag(MN_DEAD))  numAfter--;
			  }
			  mWeldEdgesGrip.SetNumVerts(numBefore,numAfter);
		  }
	  }
   }

   // TODO: Potential Optimizations:
   // When is the output cache actually needed?  Can we avoid setting it the rest of the time?
   // How about only saving the PARTs of the output cache we actually need?
   // Maybe we make an "OutputCacheNeeded()" method?
   // We could save the memory used by one "basic" copy of the output mesh - significant savings.
   // Also, setting this cache clears the "TempDataOutput" cache, which means we have to rebuild any TempData we need.
   // Perhaps we can avoid this in cases where the output cache hasn't changed.
   pData->SetOutputCache (mesh);

   pObj->UpdateValidity (GEOM_CHAN_NUM, ourValidity & geomValidity);
   pObj->UpdateValidity (TOPO_CHAN_NUM, ourValidity);
   pObj->UpdateValidity (VERT_COLOR_CHAN_NUM, ourValidity);
   pObj->UpdateValidity (TEXMAP_CHAN_NUM, ourValidity);
   pObj->UpdateValidity (SELECT_CHAN_NUM, ourValidity & selectValidity);
}

float *EditPolyMod::SoftSelection (TimeValue t, EditPolyData *pData, Interval & validity)
{
   int useSoftsel;
   mpParams->GetValue (epm_ss_use, t, useSoftsel, validity);
   if (!useSoftsel) return NULL;

   bool softSelLock = mpParams->GetInt( epm_ss_lock ) != 0;

   //If the soft selection is locked, don't change it
   if (softSelLock && pData->GetPaintSelValues()) {
      if (pData->GetMesh() && pData->GetPaintSelCount() != pData->GetMesh()->numv) {
         // Some sort of topological change has occurred?
         // Get the soft selection values from the mesh.
         // NOTE: This shouldn't happen - this should be covered by EditPolyData::SynchBitArrays().
         //DbgAssert(false);
         pData->SetPaintSelCount (pData->GetMesh()->numv);
         if (pData->GetMesh()->getVSelectionWeights())
            memcpy (pData->GetPaintSelValues(), pData->GetMesh()->getVSelectionWeights(), pData->GetMesh()->numv*sizeof(float));
      }

      return pData->GetPaintSelValues();
   }

   float bubble, pinch, falloff;
   mpParams->GetValue (epm_ss_falloff, t, falloff, validity);
   mpParams->GetValue (epm_ss_pinch, t, pinch, validity);
   mpParams->GetValue (epm_ss_bubble, t, bubble, validity);

   int useEDist, edist=0, affectBackfacing;
   Interval edistValid=FOREVER;
   mpParams->GetValue (epm_ss_edist_use, t, useEDist, validity);
   if (useEDist) {
      mpParams->GetValue (epm_ss_edist, t, edist, edistValid);
      validity &= edistValid;
   }

   mpParams->GetValue (epm_ss_affect_back, t, affectBackfacing, validity);

   return pData->GetSoftSelection (t, falloff, pinch, bubble,
      edist, !affectBackfacing, edistValid);
}

Interval EditPolyMod::LocalValidity(TimeValue t) {
   if (TestAFlag(A_MOD_BEING_EDITED)) return NEVER;
   return GetValidity (t);
}

Interval EditPolyMod::GetValidity (TimeValue t) {
   // We can't just use mpParams->GetValidity,
   // because the parameters we're not currently using
   // shouldn't influence the validity interval.
   Interval ret = FOREVER;
   PolyOperation *pOp = GetPolyOperation ();
   if (pOp != NULL) {
      pOp->GetValues (this, t, ret);
      pOp->GetNode (this, t, ret);
   }
   return ret;
}

// --- Slice: not really a command mode, just looks like it.--------- //

bool EditPolyMod::EpInSlice () {
   int currentOp = GetPolyOperationID();
   if ((currentOp != ep_op_slice) && (currentOp != ep_op_slice_face)) return false;
   if (ip->GetCommandMode() == quickSliceMode) return false;
   return true;
}

static bool slicePreviewMode = false;
static bool sPopSliceCommandMode = false;

void EditPolyMod::EnterSliceMode () {
   int currentOp = GetPolyOperationID();
   if ((currentOp != ep_op_slice) && (currentOp != ep_op_slice_face))
   {
      // Need to record that we started slice operation here, so we know to auto-cancel it later.
      slicePreviewMode = true;
      if (meshSelLevel[selLevel] == MNM_SL_FACE) EpModSetOperation (ep_op_slice_face);
      else EpModSetOperation (ep_op_slice);
   } else slicePreviewMode = false;
   mSliceMode = true;

   UpdateSliceUI();

   // If we're already in a SO move or rotate mode, stay in it;
   // Otherwise, enter SO move.
   if ((ip->GetCommandMode() != moveMode) && (ip->GetCommandMode() != rotMode)) {
      ip->PushCommandMode (moveMode);
      sPopSliceCommandMode = true;
   } else sPopSliceCommandMode = false;

   NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
   EpModRefreshScreen ();
}

void EditPolyMod::ExitSliceMode () {
   if (slicePreviewMode) {
      EpModCancel ();
      slicePreviewMode = false;
   }
   mSliceMode = false;

   UpdateSliceUI();

   if (sPopSliceCommandMode && (ip->GetCommandStackSize()>1)) ip->PopCommandMode ();

   NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
   EpModRefreshScreen ();
}

bool EditPolyMod::InSlicePreview () {
   if (!EpInSlice()) return false;
   return slicePreviewMode;
}

Matrix3 EditPolyMod::CompSliceMatrix (TimeValue t,INode *inode,ModContext *mc) {
   if (mc->localData == NULL) mc->localData = (LocalModData *) new EditPolyData();
   EditPolyData *pData = (EditPolyData *) mc->localData;
   if (pData->GetSlicePlaneSize() < 0) {
      float size1 = (mc->box->pmax.x - mc->box->pmin.x)*.52f;
      float size2 = (mc->box->pmax.y - mc->box->pmin.y)*.52f;
      if (size2>size1) size1=size2;
      if (size1 < 1.0f) size1 = 1.0f;
      pData->SetSlicePlaneSize (size1);
   }

   Interval iv;
   Matrix3 tm(1);
   if (mc && mc->tm) tm = Inverse(*(mc->tm));
   if (inode) tm = tm * inode->GetObjTMBeforeWSM(t,&iv);

   Matrix3 planeTM(1);
   mpSliceControl->GetValue (t, &planeTM, FOREVER, CTRL_RELATIVE);

   return planeTM*tm;
}

void EditPolyMod::GetSlicePlaneBoundingBox (TimeValue t, INode *inode, ViewExp *vpt, Box3 & box, ModContext *mc) {
   Matrix3 tm = CompSliceMatrix(t,inode,mc);
   EditPolyData *smd=(EditPolyData *)mc->localData;
   float size = smd->GetSlicePlaneSize ();
   box.Init();
   box += Point3(-size,-size, 0.0f)*tm;
   box += Point3(-size,size,0.0f)*tm;
   box += Point3(size,size,0.0f)*tm;
   box += Point3(size,-size,0.0f)*tm;
}

void EditPolyMod::DisplaySlicePlane (TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
   GraphicsWindow *gw = vpt->getGW();
   Matrix3 tm = CompSliceMatrix(t,inode,mc);
   EditPolyData *smd=(EditPolyData *)mc->localData;
   float size = smd->GetSlicePlaneSize();
   int savedLimits;

   gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
   gw->setTransform(tm);

   // Draw rectangle representing slice plane.
   if (mSliceMode) gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
   else gw->setColor (LINE_COLOR, GetUIColor (COLOR_GIZMOS));

   Point3 rp[5];
   rp[0] = Point3(-size,-size,0.0f);
   rp[1] = Point3(-size,size,0.0f);
   rp[2] = Point3(size,size,0.0f);
   rp[3] = Point3(size,-size,0.0f);
   gw->polyline (4, rp, NULL, NULL, TRUE, NULL);
   
   gw->setRndLimits(savedLimits);
}

int EditPolyMod::HitTestSlice (TimeValue t, INode* inode, int type, int crossing, int flags,
                    IPoint2 *p, ViewExp *vpt, ModContext* mc) {
   GraphicsWindow *gw = vpt->getGW();
   Point3 pt;
   HitRegion hr;
   int savedLimits, res = 0;
   Matrix3 tm = CompSliceMatrix(t,inode,mc);
   EditPolyData *smd=(EditPolyData *)mc->localData;
   float size = smd->GetSlicePlaneSize();

   MakeHitRegion(hr,type, crossing,4,p);
   gw->setHitRegion(&hr);
   gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM); 
   gw->setTransform(tm);

   gw->clearHitCode();
   Point3 rp[5];
   rp[0] = Point3 (-size, -size, 0.0f);
   rp[1] = Point3 (-size,  size, 0.0f);
   rp[2] = Point3 ( size,  size, 0.0f);
   rp[3] = Point3 ( size, -size, 0.0f);
   gw->polyline (4, rp, NULL, NULL, TRUE, NULL);
   if (gw->checkHitCode()) {
      vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL);
      res = 1;
   }

   gw->setRndLimits(savedLimits);
   return res;
}

// What EditPolyMod level should we be hit-testing on?
DWORD EditPolyMod::CurrentHitLevel (int *selByVert) {
   // Under normal circumstances, pretty much the level we're at.
   
   DWORD hitLev = hitLevel[selLevel];

   if(mHitLevelOverride==0&&GetSelectMode()==PS_MULTI_SEL&&mSelLevelOverride>0) //for multiselect mode
   {
	   switch(mSelLevelOverride)
	   {
		case MNM_SL_VERTEX:
			hitLev = hitLevel[EPM_SL_VERTEX];
			break;
		case MNM_SL_EDGE:
			hitLev = hitLevel[EPM_SL_EDGE];
			break;
		case MNM_SL_FACE:
			hitLev = hitLevel[EPM_SL_FACE];
			break;
		//otherwise leave it alone
		}
   }

   // But we might be selecting by vertex...
   int sbv;
   if (selByVert == NULL) selByVert = &sbv;
   mpParams->GetValue (epm_by_vertex, TimeValue(0), *selByVert, FOREVER);
   if (*selByVert) {
      hitLev = SUBHIT_MNVERTS;
      if (selLevel != EPM_SL_VERTEX) hitLev |= SUBHIT_MNUSECURRENTSEL;
      if (selLevel == EPM_SL_BORDER) hitLev |= SUBHIT_OPENONLY;
   }

   // And if we're in a command mode, it may override the current hit level:
   if (mHitLevelOverride != 0x0) hitLev = mHitLevelOverride;
   return hitLev;
}

MNMesh *EditPolyMod::GetMeshToHitTest (EditPolyData *pData) {
   if (mHitTestResult) return pData->GetMeshOutput();
   if (mpCurrentOperation) {
      if (mpCurrentOperation->OpID() == ep_op_transform) return pData->GetMeshOutput();
   }
   return pData->GetMesh();
}

void EditPolyMod::SetConstraintType(int  ) 
{
	if(mpDialogGeom&&mpDialogGeom->GetHWnd())
		TheConstraintUIHandler()->Update(mpDialogGeom->GetHWnd(),this);
}

void EditPolyMod::SetSelectMode(int what)
{
	switch(what)
	{
	case 0: //none
		mSelLevelOverride = -1;
		break;
	case 1: //subobj
		{
		   switch(selLevel)
		   {
		   case EPM_SL_VERTEX:
			   mSelLevelOverride = MNM_SL_VERTEX;
			   break;
		   case EPM_SL_EDGE:
		   case EPM_SL_BORDER:
			   mSelLevelOverride = MNM_SL_EDGE;
			   break;
		   case EPM_SL_FACE:
		   default:
			   mSelLevelOverride = MNM_SL_FACE;
			   break;
		   }
		}
		break;
	case 2: //multi
		mSelLevelOverride = -1;
		break;
	};

	InvalidateNumberHighlighted(TRUE);
	UpdateDisplayFlags();
	SetEnableStates(); 

}
int EditPolyMod::GetSelectMode()
{
	return	getParamBlock()->GetInt (epm_select_mode, TimeValue(0));
}


MeshSubHitRec * EditPolyMod::GetClosestHitRec(SubObjHitList &hitList)
{

	MeshSubHitRec *closest = NULL,*rec = hitList.First();
	if(rec)
	{
		closest = rec;
		float minDist = rec->dist;
		for (; rec; rec = rec->Next())
		{
			if(rec &&rec->dist<minDist)
			{
				closest = rec;
				minDist = rec->dist;
			}
		
		}

	}
	return closest;

}

SubObjHitList * EditPolyMod::GetClosestHitList( SubObjHitList &vertHitList,SubObjHitList &edgeHitList,SubObjHitList &faceHitList,
 								   int vertRes, int edgeRes, int faceRes,
								   MeshSubHitRec *vertClosest,MeshSubHitRec *edgeClosest,MeshSubHitRec *faceClosest,
								   int &res)
{
	SubObjHitList *hitList = NULL;
	//find the closest
	if(vertClosest)
	{
	   DWORD vertDist = vertClosest->dist-5; //make vert a little bit closer resolves any issues with poly's and edges...
	   if(edgeClosest)
	   {
		   DWORD edgeDist = edgeClosest->dist; 
		   if(faceClosest)
		   {
				DWORD faceDist = faceClosest->dist; 
				if(edgeDist<vertDist)
				{
					if(edgeDist<faceDist)
					{
						hitList = &edgeHitList;
						res = edgeRes;
					}
					else
					{
						hitList = &faceHitList;
						res = faceRes;
					}
				}
				else
				{
					if(vertDist<faceDist)
					{
						hitList = &vertHitList;
						res = vertRes;
					}
					else
					{
						hitList = &faceHitList;
						res = faceRes;
					}
				}
		   }
		   else
		   {
			   //vert vs edge
				if(edgeDist<vertDist)
				{
					hitList = &edgeHitList;
					res = edgeRes;
				}
				else
				{
					hitList = &vertHitList;
					res = vertRes;
				}
		   }
	   }
	   else
	   {
		   //no edge
		   if(faceClosest)
		   {
				DWORD faceDist = faceClosest->dist; 
				if(vertDist<faceDist)
				{
					hitList = &vertHitList;
					res = vertRes;
				}
				else
				{
					hitList = &faceHitList;
					res = faceRes;
				}
		   }
		   else
		   {
			   hitList = &vertHitList;
			   res = vertRes;
		   }
	   }
	}
	else
	{
	   //no vert
		if(edgeClosest)
		{
			DWORD edgeDist = edgeClosest->dist; 
			if(faceClosest)
			{
				DWORD faceDist = faceClosest->dist; 
				if(edgeDist<faceDist)
				{
					hitList = &edgeHitList;
					res = edgeRes;
				}
				else
				{
					hitList = &faceHitList;
					res = faceRes;
				}
			}
			else
			{
				hitList = &edgeHitList;
				res = edgeRes;
			}
		}
		else
		{
			hitList = &faceHitList;
			res = faceRes;
		}
	}
	return hitList;
}
int EditPolyMod::HitTestHighlight(TimeValue t, INode* inode,  int flags,
          ViewExp *vpt, ModContext* mc,HitRegion &hr,GraphicsWindow *gw, EditPolyData *pData)
{
   static const float HIGHLIGHT_PICK_ERROR = 0.0001f;
   static const int HIGHLIGHT_PICK_OFFSET = 10;

   MNMesh *pMesh = GetMeshToHitTest (pData);
   if (pMesh == NULL) return 0;
  
   DWORD hitFlag = HIT_ANYSOLID;
   int buttonFlags = GetCOREInterface()->GetMouseManager()->ButtonFlags();
   SubObjHitList *hitList,vertHitList,edgeHitList,faceHitList;

   //first seee if we are override already, and if so if that override level has bee active selection. if so then only test that mode stay in that mode!
   bool testVerts = true, testEdges = true, testTris = true;

   bool keepOld  = (GetKeyState(VK_CONTROL)&0x8000) ? true :false;

   //if we are doing an area select we basically keep the subobject mode of where we are in
   bool areaSelect = (hr.type!=POINT_RGN) ? true :false; 


   //we use these guys to fix 924888
   bool leftButtonDown = (buttonFlags&(MOUSE_LBUTTON))?true:false;
   bool selOnly = (flags & HIT_SELONLY) ? true: false;
   bool unSelOnly = (flags &HIT_UNSELONLY) ? true : false;
   if(unSelOnly)
	   hitFlag &=flags;


   //if we are holding CTRL and already have items so we are in a mode, or just in highlight mode only hittest one type.
   //this overrides the area select
   if( (keepOld==true && pData->mHighlightedItems.empty()==false) ||GetSelectMode()==PS_SUBOBJ_SEL)
   {

	   switch(mSelLevelOverride)
	   {
	   case MNM_SL_VERTEX:
			{
				testEdges = false;
				testTris = false;
			}
			break;

	   case MNM_SL_EDGE:
			{
				testVerts = false;
				testTris = false;

			}

		   break;
	   case MNM_SL_FACE:
			{
				testEdges = false;
				testVerts = false;
			}
		   break;

	   };
   }
   else if(areaSelect) //the mode is subobj mode
   {
		switch(selLevel)
		   {
		   case EPM_SL_VERTEX:
			   mSelLevelOverride = MNM_SL_VERTEX;
			   testEdges = false;
			   testTris = false;
			   break;
		   case EPM_SL_EDGE:
		   case EPM_SL_BORDER:
			   mSelLevelOverride = MNM_SL_EDGE;
			   testVerts = false;
			   testTris = false;
			   break;
		   case EPM_SL_FACE:
		   default:
			   mSelLevelOverride = MNM_SL_FACE;
			   testEdges  = false;
			   testVerts = false;
			   break;
		   }

   }
   if(keepOld==false)
	   pData->mHighlightedItems.clear();
   int res = 0, vertRes = 0, edgeRes = 0, faceRes = 0;

   BOOL hitMade = FALSE;

   MeshSubHitRec *vertClosest=NULL,*edgeClosest=NULL,*faceClosest=NULL;

   BOOL logAll = FALSE;
   if(hr.type!= POINT_RGN)
	   logAll= TRUE;

   if(testVerts)
   {
		int epsilon = hr.epsilon;
		if(areaSelect==false)
			hr.epsilon *=2;
		vertRes = pMesh->SubObjectHitTest(gw, gw->getMaterial(), &hr, hitFlag|SUBHIT_MNVERTS, vertHitList);
		if( vertHitList.First())
		{
			hitMade = TRUE;
			vertClosest = GetClosestHitRec(vertHitList);
		}
		hr.epsilon = epsilon;
   }
	//then try to hit edges...
   if(testEdges)
   {
		int epsilon = hr.epsilon;
		if(areaSelect==false)
			hr.epsilon *=2;
	    edgeRes = pMesh->SubObjectHitTest(gw, gw->getMaterial(), &hr, hitFlag|SUBHIT_MNEDGES, edgeHitList);
		if(edgeHitList.First())
		{
			hitMade = TRUE;
			edgeClosest = GetClosestHitRec(edgeHitList);
		}
		hr.epsilon = epsilon;
   }
	//finally try to hit faces...
   if(testTris)
   {
	  
		faceRes = pMesh->SubObjectHitTest(gw, gw->getMaterial(), &hr, hitFlag|SUBHIT_MNFACES, faceHitList);
		if(faceHitList.First())
		{
			hitMade = TRUE;
			faceClosest = GetClosestHitRec(faceHitList);
		}
   }
   // we need to modify the closest vert in case it shares the edge...
   if(vertClosest && edgeClosest  &&vertClosest->dist >= edgeClosest->dist)
   {
		float vDist = static_cast<float>(vertClosest->dist);  float eDist = static_cast<float>(edgeClosest->dist);
		if( ((fabs(vDist-eDist)/eDist))<HIGHLIGHT_PICK_ERROR) //no edge just vert!
		{
			vertClosest->dist = edgeClosest->dist - HIGHLIGHT_PICK_OFFSET;

		}
   }
   //or if it shares a face
// we need to modify the closest vert in case it shares the edge...
   if(vertClosest && faceClosest  &&vertClosest->dist >= faceClosest->dist)
   {
		float vDist = static_cast<float>(vertClosest->dist);  float fDist = static_cast<float>(faceClosest->dist);
		if( ((fabs(vDist-fDist)/fDist))<HIGHLIGHT_PICK_ERROR) //no edge just vert!
		{
			vertClosest->dist = faceClosest->dist - HIGHLIGHT_PICK_OFFSET;

		}
   }
   // we need to modify the closest edge in case it shares a face...
   if(faceClosest && edgeClosest  &&edgeClosest->dist >= faceClosest->dist)
   {
		float fDist = static_cast<float>(faceClosest->dist);  float eDist = static_cast<float>(edgeClosest->dist);
		if( ((fabs(eDist-fDist)/fDist))<HIGHLIGHT_PICK_ERROR) 
		{
			edgeClosest->dist = faceClosest->dist - HIGHLIGHT_PICK_OFFSET;

		}
   }
   if(hitMade==FALSE)
   {
	   if(keepOld==false&&(GetSelectMode()==PS_SUBOBJ_SEL)==FALSE) //if we are in highlight mode we keep our sellevel
	   {
		   mSelLevelOverride = -1;  //default just back to may just remove this anyway mz todo
	   }
	   EpModHighlightChanged();
   	   EpModRefreshScreen();
	   return 0;
   }

   //find the closest
   hitList = GetClosestHitList(vertHitList,edgeHitList,faceHitList,vertRes,edgeRes,faceRes,
	   vertClosest,edgeClosest,faceClosest,res);

   DbgAssert(hitList);
   if(hitList)
   {
		if(hitList==&faceHitList)
			mSelLevelOverride = MNM_SL_FACE;
		else if(hitList==&edgeHitList)
			mSelLevelOverride = MNM_SL_EDGE;
		else if(hitList == &vertHitList)
			mSelLevelOverride = MNM_SL_VERTEX;
   }
   if(hitList&&selOnly&&leftButtonDown)  //fix for 924888
   {

	   //first fix for 948440... if the override is different from the current selLevel, then we don't hit!
	   switch(mSelLevelOverride)
	   {
			case MNM_SL_VERTEX:
				if(selLevel!=EPM_SL_VERTEX)
				{
					EpModHighlightChanged();
   					EpModRefreshScreen();
					return 0;
				}
				break;
			case MNM_SL_EDGE:
				if(selLevel!=EPM_SL_EDGE&&selLevel!=  EPM_SL_BORDER)	  
				{
					EpModHighlightChanged();
   					EpModRefreshScreen();
					return 0;
				}
				break;

			case MNM_SL_FACE:
				if(selLevel!=EPM_SL_FACE && selLevel !=  EPM_SL_ELEMENT)
				{
					EpModHighlightChanged();
   					EpModRefreshScreen();
					return 0;
				}
				break;
	   }
		MeshSubHitRec *rec = hitList->First();
		if(rec)
		{
			//make sure that the closest is really selected.. if not then say we don't hit anything.. this is used for drag-click moves
			BitArray sel;
			switch(mSelLevelOverride)
			{
			case MNM_SL_VERTEX:
				rec = vertClosest;
				pMesh->getVertexSel(sel);
				break;
			case MNM_SL_EDGE:
				rec = edgeClosest;
				pMesh->getEdgeSel(sel);
				break;

			case MNM_SL_FACE:
				rec= faceClosest;
				pMesh->getFaceSel(sel);
				break;
			}
			if(sel[rec->index]==false)
			{
				EpModHighlightChanged();
				EpModRefreshScreen();
				return 0;
			}
		}
   }
   if(hitList==NULL)
   {
		 EpModHighlightChanged();
	   	 EpModRefreshScreen();
		 mSelLevelOverride = -1;  //de
		 return 0;
   }


   if(GetSelectMode()==PS_SUBOBJ_SEL&&(selLevel==EPM_SL_BORDER|| selLevel==EPM_SL_ELEMENT)) //if in border or element mode than pick a  border or element!
   {

		BitArray toSelection;
		if(selLevel==EPM_SL_BORDER&&edgeClosest)
		{
			BitArray edgeArray(pMesh->ENum());

			MeshSubHitRec *rec = edgeClosest;
			{
				edgeArray.Set(rec->index);
			}
			pData->mSelConv.EdgeToBorder (*pMesh, edgeArray, toSelection);
			if(logAll==FALSE)
				vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
			
		}

		else if(selLevel==EPM_SL_ELEMENT&&faceClosest)
		{
			BitArray faceArray(pMesh->FNum());

			MeshSubHitRec *rec = faceClosest;
			{
				faceArray.Set(rec->index);
			}
			pData->mSelConv.FaceToElement (*pMesh, faceArray, toSelection);
			if(logAll==FALSE)
				vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
		}
		if(toSelection.AnyBitSet())
		{
			for(int i =0;i<toSelection.GetSize();++i)
			{
				if(toSelection[i])
				{
					pData->mHighlightedItems.insert(i);
				}
			}
			if(logAll)
			{
				//and still log the hits but no more higlights
				MeshSubHitRec *rec = hitList->First();
				for(; rec; rec = rec->Next())
				{
					vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
				}
			}
		}
		else
		{
			EpModHighlightChanged();
			EpModRefreshScreen();
			return 0;
		}
   }
   else
   {
	   switch(mSelLevelOverride)
	   {
	   case MNM_SL_VERTEX:
			if(vertClosest)
			{
				pData->mHighlightedItems.insert(vertClosest->index);
				if(logAll==FALSE)
					vpt->LogHit(inode,mc,vertClosest->dist,vertClosest->index,NULL);
			}
		   break;

	   case MNM_SL_EDGE:
		   if(edgeClosest)
		   {
			   pData->mHighlightedItems.insert(edgeClosest->index);
			   if(logAll==FALSE)
					vpt->LogHit(inode,mc,edgeClosest->dist,edgeClosest->index,NULL);
		   }
		   break;

	   case MNM_SL_FACE:
		   if(faceClosest)
		   {
			   pData->mHighlightedItems.insert(faceClosest->index);
			   if(logAll==FALSE)
					vpt->LogHit(inode,mc,faceClosest->dist,faceClosest->index,NULL);
		   }
		   break;

	   }
	    

	   if(logAll)
	   {
			MeshSubHitRec *rec = hitList->First();
			for(; rec; rec = rec->Next())
			{
			//need to get max when mHitTestResult is true and we ignore newInResult. bascially mHitTestResult should never be true here!
			//if ((max>0) && (rec->index >= max)) {
			//		res--;
			//		continue;
			//	}
			//	
				vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
			//	pData->GetNewSelection()->Set (rec->index, true);
			}
	   }
   }
	EpModHighlightChanged();
	EpModRefreshScreen();
	return res;

}


int EditPolyMod::HitTest (TimeValue t, INode* inode, int type, int crossing, int flags,
                    IPoint2 *p, ViewExp *vpt, ModContext* mc) {

   mMeshHit = false;
   mSuspendCloneEdges = false;
   if (mSliceMode && CheckNodeSelection (ip, inode))
      return HitTestSlice (t, inode, type, crossing, flags, p, vpt, mc);

   if (!selLevel && !mHitLevelOverride) return 0;

   Interval valid;
   int savedLimits;
   GraphicsWindow *gw = vpt->getGW();
   HitRegion hr;
   
   int ignoreBackfacing;
   mpParams->GetValue (epm_ignore_backfacing, t, ignoreBackfacing, FOREVER);
   if (mForceIgnoreBackfacing) ignoreBackfacing = true;

   // Setup GW
   MakeHitRegion(hr,type, crossing,4,p);
   gw->setHitRegion(&hr);
   Matrix3 mat = inode->GetObjectTM(t);
   gw->setTransform(mat);  
   gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
   if (ignoreBackfacing) gw->setRndLimits (gw->getRndLimits() | GW_BACKCULL);
   else gw->setRndLimits (gw->getRndLimits() & ~GW_BACKCULL);
   gw->clearHitCode();

   if (!mc->localData) return 0;
   EditPolyData *pData = (EditPolyData *) mc->localData;
   MNMesh *pMesh = GetMeshToHitTest (pData);
   if (pMesh == NULL) return 0;

   pData->SetConverterFlag (MNM_SELCONV_REQUIRE_ALL, (crossing||(type==HITTYPE_POINT))?false:true);

   int selByVert;
   SubObjHitList hitList;

   	BOOL multiSelect = GetSelectMode()==PS_MULTI_SEL;
	BOOL highlightSelect = GetSelectMode()==PS_SUBOBJ_SEL;
  

   if(mHitLevelOverride==0&&(multiSelect||highlightSelect)) //special case prototype for override mode...
   {
	 return   HitTestHighlight(t,inode,flags,vpt,mc,hr,gw,pData);
   }

   DWORD hitLev = CurrentHitLevel (&selByVert);
   int res = pMesh->SubObjectHitTest(gw, gw->getMaterial(), &hr,
      flags|hitLev, hitList);

   MeshSubHitRec *rec = hitList.First();

   if (rec) {
      int max=0;
      if (mHitTestResult && mIgnoreNewInResult && pData->GetMesh()) {
         if (hitLev & SUBHIT_MNVERTS) max = pData->GetMesh()->numv;
         else if (hitLev & SUBHIT_MNEDGES) max = pData->GetMesh()->nume;
         else if (hitLev & SUBHIT_MNFACES) max = pData->GetMesh()->numf;
      }

      Tab<int> diagStart;
      Tab<int> diagToFace;
      if (hitLev & SUBHIT_MNDIAGONALS) {
         diagStart.SetCount (pMesh->numf+1);
         diagStart[0] = 0;
         for (int i=0; i<pMesh->numf; i++) {
            int numDiags = (pMesh->f[i].GetFlag (MN_DEAD)||pData->HiddenFace(i)) ? 0 : pMesh->f[i].deg-3;
            diagStart[i+1] = diagStart[i] + numDiags;
         }

         diagToFace.SetCount (diagStart[pMesh->numf]);
         for (int i=0; i<pMesh->numf; i++) {
            for (int j=diagStart[i]; j<diagStart[i+1]; j++) diagToFace[j] = i;
         }
      }

      for (; rec; rec = rec->Next()) {
         if ((max>0) && (rec->index >= max)) {
            res--;
            continue;
         }
         if (hitLev & SUBHIT_MNDIAGONALS)
         {
            int fid = diagToFace[rec->index];
            int did = rec->index - diagStart[fid];
            vpt->LogHit (inode, mc, rec->dist, rec->index, new MNDiagonalHitData (fid, did));
         }
         else vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
      }
   }

   if ((GetKeyState(VK_SHIFT) & 0x8000) && (type == HITTYPE_POINT) )
   {
	   SubObjHitList tempHitList;
	   if (pMesh->SubObjectHitTest(gw, gw->getMaterial(), &hr, hitLev|SUBHIT_ABORTONHIT, tempHitList))
	   {
		   mMeshHit = true;
	   }

   }

   gw->setRndLimits(savedLimits);   
   return res; 
}

void EditPolyMod::UpdateDisplayFlags () {
   mCurrentDisplayFlags = editPolyLevelToDispFlags[selLevel] | mDispLevelOverride;

   // If we're showing the gizmo, always suspend showing the end result vertices.
   if (ShowGizmo()) mCurrentDisplayFlags -= (mCurrentDisplayFlags & (MNDISP_SELVERTS|MNDISP_VERTTICKS));
}

MNMesh *EditPolyMod::GetMeshToDisplay (EditPolyData *pData) {
   if (mDisplayResult) return pData->GetMeshOutput();
   if (mpCurrentOperation) {
      if (mpCurrentOperation->OpID() == ep_op_transform) return pData->GetMeshOutput();
      if (mpCurrentOperation->OpID() == ep_op_create) return pData->GetMeshOutput();
   }
   return pData->GetMesh();
}

int EditPolyMod::Display (TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
   if (!selLevel) return 0;
   if (!mc->localData) return 0;
   if (EpInSlice() && CheckNodeSelection(ip, inode))
      DisplaySlicePlane (t, inode, vpt, flags, mc);

   // Set up GW
   GraphicsWindow *gw = vpt->getGW();
   Matrix3 tm = inode->GetObjectTM(t);
   int savedLimits = gw->getRndLimits();
   gw->setTransform(tm);

   EditPolyData *pData = (EditPolyData *) mc->localData;
   MNMesh *pMesh = GetMeshToDisplay(pData);
   //draw the multi-select selected polygons that aren't in your group as a darker selection color..

 
  if(selLevel>0)
  {
	   if(GetSelectMode()==PS_MULTI_SEL&&GetMeshToDisplay(pData))
	   {
		   IColorManager *pClrMan = GetColorManager();;
		   Point3 oldSubColor = pClrMan->GetOldUIColor(COLOR_SUBSELECTION); //hack to fix bud with render3dface in mnmesh.. automatically takes it from this!
		   Point3 newSubColor = oldSubColor *0.5f;
		   gw->setColor (FILL_COLOR,newSubColor);
		   gw->setColor (LINE_COLOR,newSubColor);
		   pClrMan->SetOldUIColor(COLOR_SUBSELECTION,&newSubColor);

		   MNMesh *pSelectMesh = GetMeshToDisplay(pData);

		   if(selLevel!=EPM_SL_VERTEX)
		   {
 			   gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS) & (GW_ILLUM|GW_WIREFRAME) );
			   if(pSelectMesh->numv==pData->mVertSel.GetSize())
			   {
				   for(int i=0;i<pSelectMesh->numv;++i)
				   {
					   if(pData->mVertSel[i])
					   {
							DisplayHighlightVert(t,gw,pSelectMesh,i);
					   }
				   }
			   }

		   }		
		   if(selLevel!=EPM_SL_EDGE&&selLevel!=  EPM_SL_BORDER)
		   {
 			   gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS) & (GW_ILLUM|GW_WIREFRAME) );
			   if(pSelectMesh->nume==pData->mEdgeSel.GetSize())
			   {
				   for(int i=0;i<pSelectMesh->nume;++i)
				   {
					   if(pData->mEdgeSel[i])
					   {
							DisplayHighlightEdge(t,gw,pSelectMesh,i);
					   }
				   }
			   }

		   }
		   if(selLevel!=EPM_SL_FACE && selLevel !=  EPM_SL_ELEMENT)
		   {
				if(pSelectMesh->numf==pData->mFaceSel.GetSize())
				{
					DWORD illum =0;
					if(savedLimits &GW_ILLUM)
						illum = GW_ILLUM;
					else if(savedLimits & GW_FLAT)
						illum = GW_FLAT;
					DWORD zbuffer =0;
					if( (savedLimits&GW_WIREFRAME)==FALSE)
						zbuffer = GW_Z_BUFFER;
					else
						illum = GW_ILLUM;
					if(savedLimits & GW_SHADE_SEL_FACES)
					   gw->setRndLimits(GW_SHADE_SEL_FACES|zbuffer|illum|GW_TRANSPARENCY|GW_TEXTURE| GW_TRANSPARENT_PASS);
					else
					  gw->setRndLimits((GW_WIREFRAME) );
					if(savedLimits & GW_SHADE_SEL_FACES)
					{
						Material mat;
						//Set Material properties
						mat.dblSided = 1;
						//diffuse, ambient and specular
						mat.Ka = newSubColor ;
						mat.Kd = newSubColor ;
						mat.Ks = newSubColor ;
						// Set opacity
						mat.opacity = 0.33f;
						//Set shininess
						mat.shininess = 0.0f;
						mat.shinStrength = 0.0f;
						gw->setMaterial(mat);
						gw->setTransparency( GW_TRANSPARENT_PASS|GW_TRANSPARENCY);
					}
					Point3 xyz[4];
					Point3 nor[4];
					Point3 uvw[4];
					uvw[0] = Point3(0.0f,0.0f,0.0f);
					uvw[1] = Point3(1.0f,0.0f,0.0f);
					uvw[2] = Point3(0.0f,1.0f,0.0f);

					gw->startTriangles();
					Tab<int> tri;
					int		edgePtr[4];
					for(int i=0;i<pSelectMesh->numf;++i)
				    {
						if(pData->mFaceSel[i])
						{
							tri.ZeroCount();
							MNFace *face = &pSelectMesh->f[i];
							face->GetTriangles(tri);
							int		*vv		= face->vtx;
							int		deg		= face->deg;
							DWORD	smGroup = face->smGroup;
							for (int tt=0; tt<tri.Count(); tt+=3)
							{
								int *triv = tri.Addr(tt);
								for (int z=0; z<3; z++) xyz[z] = pSelectMesh->v[vv[triv[z]]].p;
								nor[0] = nor[1] = nor[2] = pSelectMesh->GetFaceNormal(i, TRUE);
								if(savedLimits & GW_SHADE_SEL_FACES)
									gw->triangleN(xyz,nor,uvw);
								else
								{
									for (int k=0; k<3; k++)
									{
										if ((tri[tt+k] + 1)%deg != tri[tt+(k+1)%3])
											edgePtr[k] = GW_EDGE_SKIP;
										else
										{
											edgePtr[k] = GW_EDGE_VIS;
										}
									}
									gw->polyline(3, xyz, NULL, 1, edgePtr);
								}
							
							}

						}
					}
					gw->endTriangles();
					if(savedLimits & GW_SHADE_SEL_FACES)
						gw->setTransparency(0);
			   }
		   }
		   pClrMan->SetOldUIColor(COLOR_SUBSELECTION,&oldSubColor);
	   }
     

	 if(pData->mHighlightedItems.empty()==false&&GetMeshToHitTest(pData))
	   { 
		   MNMesh *pHighlightMesh = GetMeshToHitTest(pData);

		   gw->setColor (FILL_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
		   gw->setColor (LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));

		   IColorManager *pClrMan = GetColorManager();;
		   Point3 oldSubColor = pClrMan->GetOldUIColor(COLOR_SUBSELECTION); //hack to fix bud with render3dface in mnmesh.. automatically takes it from this!
		   pClrMan->SetOldUIColor(COLOR_SUBSELECTION,&pClrMan->GetOldUIColor(COLOR_SEL_GIZMOS));
		   //pClrMan->SetOldUIColor(COLOR_SUBSELECTION,&fillColor);

		   switch(mSelLevelOverride)
		   {
			case MNM_SL_VERTEX:
				{
	 			    gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS) & (GW_ILLUM|GW_WIREFRAME) );
					std::set<int>::iterator it = pData->mHighlightedItems.begin(); 
					while(it != pData->mHighlightedItems.end()) 
					{
						DisplayHighlightVert(t,gw,pHighlightMesh,*it);
						++it;
					}
				}
				break;
			case MNM_SL_EDGE:
				{
	 			    gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS) & (GW_ILLUM|GW_WIREFRAME) );
					std::set<int>::iterator it = pData->mHighlightedItems.begin(); 
					while(it != pData->mHighlightedItems.end()) 
					{
						DisplayHighlightEdge(t,gw,pHighlightMesh,*it);
						++it;
					}
				}
				break;
			case MNM_SL_FACE:
				{
				    DWORD illum =0;
					if(savedLimits &GW_ILLUM)
						illum = GW_ILLUM;
					else if(savedLimits & GW_FLAT)
						illum = GW_FLAT;
					DWORD zbuffer =0;
					if( (savedLimits&GW_WIREFRAME)==FALSE)
						zbuffer = GW_Z_BUFFER;
					else
						illum = GW_ILLUM;
					if(savedLimits & GW_SHADE_SEL_FACES)
					   gw->setRndLimits(GW_SHADE_SEL_FACES|zbuffer|illum|GW_TRANSPARENCY|GW_TEXTURE| GW_TRANSPARENT_PASS);
					else
					  gw->setRndLimits((GW_WIREFRAME) );
					if(savedLimits & GW_SHADE_SEL_FACES)
					{
						Material mat;
						//Set Material properties
						mat.dblSided = 1;
						//diffuse, ambient and specular
						mat.Ka = GetUIColor(COLOR_SEL_GIZMOS) ;
						mat.Kd = GetUIColor(COLOR_SEL_GIZMOS) ;
						mat.Ks = GetUIColor(COLOR_SEL_GIZMOS) ;
						// Set opacity
						mat.opacity = 0.5f;
						//Set shininess
						mat.shininess = 0.0f;
						mat.shinStrength = 0.0f;
						gw->setMaterial(mat);
						gw->setTransparency( GW_TRANSPARENT_PASS|GW_TRANSPARENCY);
					}
					
					Point3 xyz[4];
					Point3 nor[4];
					Point3 uvw[4];
					uvw[0] = Point3(0.0f,0.0f,0.0f);
					uvw[1] = Point3(1.0f,0.0f,0.0f);
					uvw[2] = Point3(0.0f,1.0f,0.0f);
					
					gw->startTriangles();
					std::set<int>::iterator it = pData->mHighlightedItems.begin(); 
					Tab<int> tri;				
					int		edgePtr[4];
					while(it != pData->mHighlightedItems.end()) 
					{
						tri.ZeroCount();
						MNFace *face = &pHighlightMesh->f[*it];
						face->GetTriangles(tri);
						int		*vv		= face->vtx;
						int		deg		= face->deg;
						DWORD	smGroup = face->smGroup;
						for (int tt=0; tt<tri.Count(); tt+=3)
						{
							int *triv = tri.Addr(tt);
							for (int i=0; i<3; i++) xyz[i] = pHighlightMesh->v[vv[triv[i]]].p;
							nor[0] = nor[1] = nor[2] = pHighlightMesh->GetFaceNormal(*it, TRUE);
							if(savedLimits & GW_SHADE_SEL_FACES)
								gw->triangleN(xyz,nor,uvw);
							else
							{
								for (int i=0; i<3; i++)
								{
									if ((tri[tt+i] + 1)%deg != tri[tt+(i+1)%3])
										edgePtr[i] = GW_EDGE_SKIP;
									else
									{
										edgePtr[i] = GW_EDGE_VIS;
									}
								}
								gw->polyline(3, xyz, NULL, 1, edgePtr);
							}
						}
						++it;
					}
					gw->endTriangles();
					if(savedLimits & GW_SHADE_SEL_FACES)
						gw->setTransparency(0);
				}

				break;
		   }
		   pClrMan->SetOldUIColor(COLOR_SUBSELECTION,&oldSubColor);
	   }
	 }



   gw->setRndLimits(savedLimits);
   if (ShowGizmo())
   {

      if (pMesh) {

         // We need to draw a "gizmo" version of the mesh:
         Point3 colSel=GetSubSelColor();
         Point3 colTicks=GetUIColor (COLOR_VERT_TICKS);
         gw->setColor (LINE_COLOR, m_CageColor);

         Point3 rp[3];
         int i;
         int es[3];
         for (i=0; i<pMesh->nume; i++) {
            if (pMesh->e[i].GetFlag (MN_DEAD)) continue;
            if (!pMesh->e[i].GetFlag (MN_EDGE_INVIS)) {
               es[0] = GW_EDGE_VIS;
            } else {
               if (selLevel < EPM_SL_EDGE) continue;
               if (selLevel > EPM_SL_FACE) continue;
               es[0] = GW_EDGE_INVIS;
            }
            bool displayEdgeSel = false;
			DWORD tempSelLevel = meshSelLevel[selLevel];
			if(mSelLevelOverride)
				tempSelLevel = mSelLevelOverride;
			switch (tempSelLevel) {
            case MNM_SL_EDGE:
               displayEdgeSel = IsEdgeSelected (pData, i);
               break;
            case MNM_SL_FACE:
               displayEdgeSel = IsFaceSelected (pData, pMesh->e[i].f1);
               if (pMesh->e[i].f2>-1) displayEdgeSel |= IsFaceSelected (pData, pMesh->e[i].f2);
               break;
            }
            if (displayEdgeSel) gw->setColor (LINE_COLOR, m_SelectedCageColor);
            else gw->setColor (LINE_COLOR, m_CageColor);
            rp[0] = pMesh->v[pMesh->e[i].v1].p;
            rp[1] = pMesh->v[pMesh->e[i].v2].p;
            gw->polyline (2, rp, NULL, NULL, FALSE, es);
         }
         DWORD dispFlags = editPolyLevelToDispFlags[selLevel] | mDispLevelOverride;
         if (dispFlags & MNDISP_VERTTICKS) {
            float *ourvw = (dispFlags & MNDISP_SELVERTS) ? SoftSelection(t, pData, FOREVER) : NULL;
            gw->setColor (LINE_COLOR, colTicks);
            for (i=0; i<pMesh->numv; i++) {
               if (pMesh->v[i].GetFlag (MN_DEAD)) continue;
               if (pData->HiddenVertex (i)) continue;

               if (dispFlags & MNDISP_SELVERTS)
               {
                  if (IsVertexSelected (pData, i)) gw->setColor (LINE_COLOR, colSel);
                  else if (ourvw && ourvw[i]) gw->setColor (LINE_COLOR, SoftSelectionColor(ourvw[i]));
                  else gw->setColor (LINE_COLOR, colTicks);
               }

               if(getUseVertexDots()) gw->marker (&(pMesh->v[i].p), VERTEX_DOT_MARKER(getVertexDotType()));
               else gw->marker (&(pMesh->v[i].p), PLUS_SIGN_MRKR);
            }
         }
      }
   }

   if (ip->GetCommandMode() == createFaceMode) createFaceMode->Display(gw);

   gw->setRndLimits(savedLimits);
   return 0;   
}

void EditPolyMod::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
   if (!ip) return;

   if (EpInSlice() && CheckNodeSelection (ip, inode))
      GetSlicePlaneBoundingBox (t, inode, vpt, box, mc);

   if (ShowGizmo ()|| GetSelectMode()==PS_SUBOBJ_SEL || GetSelectMode()==PS_MULTI_SEL) {
      Matrix3 tm = inode->GetObjectTM(t);
      EditPolyData *pData = (EditPolyData *) mc->localData;
      MNMesh *pMesh = GetMeshToDisplay(pData);
      if (!pMesh) return;
      box = pMesh->getBoundingBox (&tm);
   }
}

void EditPolyMod::GetSubObjectCenters (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
   Matrix3 tm = node->GetObjectTM(t);

   if (mSliceMode) {
      Matrix3 tm = CompSliceMatrix (t, node, mc);
      cb->Center (tm.GetTrans(), 0);
      return;
   }

   if (!mc->localData) return;
   if (selLevel == EPM_SL_OBJECT) return; // shouldn't happen.

   EditPolyData *pData = (EditPolyData *) mc->localData;
   MNMesh *pMesh = GetMeshToDisplay (pData);
   if (!pMesh) return;
   float *vsw = SoftSelection (t, pData, FOREVER);

   if (selLevel == EPM_SL_VERTEX) {
      BitArray vTempSel = pMesh->VertexTempSel();
      Point3 cent(0,0,0);
      int ct=0;
      for (int i=0; i<pMesh->numv; i++) {
         if (vTempSel[i] || (vsw && vsw[i])) {
            cent += pMesh->v[i].p;
            ct++;
         }
      }
      if (ct) {
         cent /= float(ct);         
         cb->Center(cent*tm,0);
      }
      return;
   }

   MNTempData *pTemp = (pMesh == pData->GetMeshOutput())
      ? pData->TempDataOutput() : pData->TempData();

   if (pTemp == NULL) return;
   Tab<Point3> *centers = pTemp->ClusterCenters(meshSelLevel[selLevel], MN_SEL);
   DbgAssert (centers);
   if (!centers) return;
   for (int i=0; i<centers->Count(); i++) cb->Center((*centers)[i]*tm,i);
}

void EditPolyMod::GetSubObjectTMs (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
   if (mSliceMode) {
      cb->TM (CompSliceMatrix (t, node, mc), 0);
      return;
   }

   if (!mc->localData) return;
   EditPolyData *pData = (EditPolyData *) mc->localData;

   MNMesh *pMesh = GetMeshToDisplay (pData);
   MNTempData *pTemp = (pMesh == pData->GetMeshOutput()) ? pData->TempDataOutput() : pData->TempData ();
   if (!pMesh) return;
   if (!pTemp) return;
   float *vsw = SoftSelection (t, pData, FOREVER);

   Matrix3 otm = node->GetObjectTM(t);
   Matrix3 tm;

   switch (selLevel) {
   case EPM_SL_OBJECT:
      break;

   case EPM_SL_VERTEX:
      if (ip->GetCommandMode()->ID()==CID_SUBOBJMOVE) {
         Matrix3 otm = node->GetObjectTM(t);
         for (int i=0; i<pMesh->numv; i++) {
            if (!pMesh->v[i].FlagMatch (MN_SEL|MN_DEAD, MN_SEL) && (!vsw || !vsw[i])) continue;
            pMesh->GetVertexSpace (i, tm);
            Point3 p = pMesh->v[i].p;
            tm = tm * otm;
            tm.SetTrans(pMesh->v[i].p*otm);
            cb->TM(tm,i);
         }
      } else {
         int i;
         for (i=0; i<pMesh->numv; i++) if (pMesh->v[i].FlagMatch (MN_SEL|MN_DEAD, MN_SEL) || (vsw && vsw[i])) break;
         if (i >= pMesh->numv) return;
         Point3 norm(0,0,0), *nptr=pTemp->VertexNormals ()->Addr(0);
         Point3 cent(0,0,0);
         int ct=0;

         // Compute average face normal
         for (; i<pMesh->numv; i++) {
            if (!pMesh->v[i].FlagMatch (MN_SEL|MN_DEAD, MN_SEL) && (!vsw || !vsw[i])) continue;
            cent += pMesh->v[i].p;
            norm += nptr[i];
            ct++;
         }
         cent /= float(ct);
         norm = Normalize (norm/float(ct));

         cent = cent * otm;
         norm = Normalize(VectorTransform(otm,norm));
         Matrix3 mat;
         MatrixFromNormal(norm,mat);
         mat.SetTrans(cent);
         cb->TM(mat,0);
      }
      break;

   case EPM_SL_EDGE:
   case EPM_SL_BORDER:
      int i, ct;
      ct = pTemp->ClusterNormals (MNM_SL_EDGE, MN_SEL)->Count();
      for (i=0; i<ct; i++) {
         tm = pTemp->ClusterTM (i) * otm;
         cb->TM(tm,i);
      }
      break;

   default:
      ct = pTemp->ClusterNormals (MNM_SL_FACE, MN_SEL)->Count();
      for (i=0; i<ct; i++) {
         tm = pTemp->ClusterTM (i) * otm;
         cb->TM(tm,i);
      }
      break;
   }
}

// Indicates whether the Show Cage checkbox is relevant:
bool EditPolyMod::ShowGizmoConditions () {
   if (!ip) return false;
   if (mpCurrentEditMod != this) return false;
   if (selLevel == EPM_SL_OBJECT) return false;
   return true;
}

// Indicates whether we're currently showing the cage gizmo.
bool EditPolyMod::ShowGizmo () {
   if (!ip) return false;
   if (mpCurrentEditMod != this) return false;
   if (selLevel == EPM_SL_OBJECT) return false;
   if (mpParams == NULL) return false;
   return mpParams->GetInt (epm_show_cage) != 0;
}

void EditPolyMod::CreatePointControllers () {
   if (mPointControl.Count()) return;

   // Find the number of vertices, and assign the ranges, for all nodes:
   ModContextList list;
   INodeTab nodes;
   ip->GetModContexts (list, nodes);
   if (list.Count() == 0) return;
   mPointNode.removeAll();
   TSTR empty;
   mPointRange.SetCount (list.Count()+1);
   int numv=0;
   for (int i=0; i<list.Count(); i++) {
      mPointNode.append (empty);
      mPointNode[i] = nodes[i]->GetName();
      mPointRange[i] = numv;
      if (list[i]->localData == NULL) continue;

      EditPolyData *pData = (EditPolyData *) list[i]->localData;
      if (pData->GetMesh()) numv += pData->GetMesh()->numv;
      else numv += pData->mVertSel.GetSize();
   }
   nodes.DisposeTemporary ();
   mPointRange[list.Count()] = numv;

   AllocPointControllers (numv);
}

void EditPolyMod::AllocPointControllers (int count) {
   int oldCount = mPointControl.Count();
   mPointControl.SetCount (count);
   if (mpPointMaster) mpPointMaster->SetNumSubControllers (count);
   for (int i=oldCount; i<count; i++) {
      mPointControl[i] = NULL;
      if (mpPointMaster) mpPointMaster->SetSubController(i, NULL);
   }
}

void EditPolyMod::DeletePointControllers () {
   if (!mPointControl.Count()) return;

   for (int i=0; i<mPointControl.Count(); i++) {
      if (mPointControl[i] == NULL) continue;
      ReplaceReference (i+EDIT_POINT_BASE_REF, NULL, true);
   }
   AllocPointControllers (0);
}

void EditPolyMod::PlugControllers (TimeValue t, Point3 *pOffsets, int numOffsets, INode *pNode) {
   if (numOffsets<0) return;
   SetKeyModeInterface *ski = GetSetKeyModeInterface(GetCOREInterface());
   if( !ski || !ski->TestSKFlag(SETKEY_SETTING_KEYS) ) {
      if(!AreWeKeying(t)) { return; }
   }

   BitArray set;
   set.SetSize (numOffsets);
   for (int i=0; i<numOffsets; i++) {
      if (pOffsets[i] != Point3(0,0,0)) set.Set(i);
   }

   PlugControllers (t, set, pNode);
}

void EditPolyMod::PlugControllers (TimeValue t, BitArray &set, INode *pNode) {
   SetKeyModeInterface *ski = GetSetKeyModeInterface(GetCOREInterface());
   if( !ski || !ski->TestSKFlag(SETKEY_SETTING_KEYS) ) {
      if(!AreWeKeying(t)) { return; }
   }

   // Find the range for this particular node:
   if (mPointControl.Count() == 0) CreatePointControllers ();
   int j;
   for (j=0; j<mPointNode.length(); j++) {
      if (mPointNode[j] == TSTR(pNode->GetName())) break;
   }
   if (j==mPointNode.length()) {
      // Node not listed in our set of nodes.
      // This can happen if the Edit Poly is instanced to a new node,
      // which can be done through Maxscript at least.  (A = $.modifiers[#Edit_Poly] / addModifier $Box01 A)
      EditPolyData *pData = GetEditPolyDataForNode (pNode);
      if (pData == NULL) return;

      TSTR empty;
      mPointNode.append (empty);
      mPointNode[j] = pNode->GetName();
      mPointRange.SetCount (j+2);
      if (pData->GetMesh()) {
         mPointRange[j+1] = mPointRange[j] + pData->GetMesh()->numv;
      } else {
         mPointRange[j+1] = mPointRange[j] + pData->mVertSel.GetSize();
      }
      AllocPointControllers (mPointRange[j+1]);
   }

   BOOL res = FALSE;
   for (int i=0; i<set.NumberSet(); i++) {
      if (!set[i]) continue;
      if (i + mPointRange[j]>=mPointRange[j+1]) break;
      if (PlugController (t, i, mPointRange[j], pNode)) res = TRUE;
   }
   if (res) NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

BOOL EditPolyMod::PlugController (TimeValue t, int vertex, int nodeStart, INode *pNode) {
   int i = vertex + nodeStart;
   if (!AreWeKeying(t)) return FALSE;
   if (!mPointControl.Count()) CreatePointControllers ();
   if ( i >= mPointControl.Count()) return FALSE;
   if (mPointControl[i]) return false; // already got a controller here.

   EditPolyData *pData = GetEditPolyDataForNode (pNode);
   if (!pData) return false;
   if (!pData->GetPolyOpData() || (pData->GetPolyOpData()->OpID() != ep_op_transform)) return false;
   LocalTransformData *pTransData = (LocalTransformData *) pData->GetPolyOpData();

   ReplaceReference (i + EDIT_POINT_BASE_REF, NewDefaultPoint3Controller());
   SuspendAnimate();
   AnimateOff();
   SuspendSetKeyMode(); // AF (5/13/03)
   theHold.Suspend ();
#if defined(NEW_SOA)
   GetPolyObjDescriptor()->Execute(I_EXEC_EVAL_SOA_TIME, reinterpret_cast<ULONG_PTR>(&t));
#endif

   // These guys are all offsets, but I need to initialize to the "current" offset, if any:
   if (pTransData->NumOffsets() > vertex) mPointControl[i]->SetValue (t, pTransData->OffsetPointer(vertex));
   else {
      Point3 zero(0,0,0);
      mPointControl[i]->SetValue (t, &zero);
   }
   ResumeSetKeyMode();
   theHold.Resume ();

   ResumeAnimate ();
   mpPointMaster->SetSubController (i, mPointControl[i]);
   return TRUE;
}

RefTargetHandle EditPolyMod::GetReference(int i) {
   switch (i) {
   case EDIT_PBLOCK_REF: return mpParams;
   case EDIT_SLICEPLANE_REF: return mpSliceControl;
   case EDIT_MASTER_POINT_REF: return mpPointMaster;
   }
   i -= EDIT_POINT_BASE_REF;
   if ((i<0) || (i>=mPointControl.Count())) return NULL;
   return mPointControl[i];
}

void EditPolyMod::SetReference(int i, RefTargetHandle rtarg) {
   switch (i) {
   case EDIT_PBLOCK_REF:
      {
      mpParams = (IParamBlock2*)rtarg;
         if (mpParams )
         {
            m_CageColor       = mpParams->GetPoint3(epm_cage_color,TimeValue(0) );
            m_SelectedCageColor  = mpParams->GetPoint3(epm_selected_cage_color,TimeValue(0) );
         }
      }
      return;
   case EDIT_SLICEPLANE_REF:
      mpSliceControl = (Control *)rtarg;
      return;
   case EDIT_MASTER_POINT_REF:
      mpPointMaster = (MasterPointControl*)rtarg;
      if (mpPointMaster) mpPointMaster->SetNumSubControllers (mPointControl.Count());
      return;
   }

   i -= EDIT_POINT_BASE_REF;
   if (i<0) return;  // shouldn't happen.
   if (!mPointControl.Count()) CreatePointControllers ();
   if (i>=mPointControl.Count()) return;
   mPointControl[i] = (Control *) rtarg;
}

RefResult EditPolyMod::NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
      PartID& partID, RefMessage message) {
   if (message == REFMSG_CHANGE) {
      if (hTarget == mpParams) {
         // if this was caused by a NotifyDependents from pblock, LastNotifyParamID()
         // will contain ID to update, else it will be -1 => inval whole rollout
         int pid = mpParams->LastNotifyParamID();
         if (ip && (pid>-1))
		 {
			InvalidateDialogElement (pid);
			ResetGripUI(); //also need to reset the grip ui to handle UNDOS
		 }


         if (((pid==epm_ss_use) || (pid==epm_ss_lock)) && InPaintMode()) 
            EndPaintMode (); //end paint mode before invalidating UI

         switch (pid) {
         case epm_ss_use:
    		SetUpManipGripIfNeeded(); //and pass through, no break
		 case epm_ss_edist_use:
         case epm_ss_edist:
            InvalidateDistanceCache ();
            break;
         case epm_ss_affect_back:
         case epm_ss_falloff:
         case epm_ss_pinch:
         case epm_ss_bubble:
         case epm_ss_lock:
            InvalidateSoftSelectionCache ();
            break;
         case epm_current_operation:
            if (mpDialogOperation) EpModCloseOperationDialog ();
            UpdateOpChoice ();
            UpdateEnables (ep_animate);
            UpdateSliceUI ();
            break;
         case epm_extrude_spline_node:
            if (mpCurrentOperation) mpCurrentOperation->ClearNode (pid);
            UpdateOperationDialogParams ();
            break;
         case epm_stack_selection:
            InvalidateDistanceCache ();
            UpdateEnables (ep_select);
            SetNumSelLabel();
			InvalidateNumberHighlighted ();
            break;
         case epm_show_cage:
            UpdateDisplayFlags ();
            break;
         case epm_animation_mode:
            UpdateEnables (ep_geom);
            UpdateEnables (ep_subobj);
            break;
         case epm_cage_color:
            UpdateCageColor();
            break;
         case epm_selected_cage_color:
            UpdateSelectedCageColor();
            break;
         }
      }
   }
   return REF_SUCCEED;
}

TSTR EditPolyMod::SubAnimName(int i)
{
   switch (i)
   {
   case EDIT_PBLOCK_REF: return GetString (IDS_PARAMETERS);
   case EDIT_SLICEPLANE_REF: return GetString (IDS_EDITPOLY_SLICE_PLANE);
   case EDIT_MASTER_POINT_REF: return GetString (IDS_EDITPOLY_MASTER_POINT_CONTROLLER);
   }

   i -= EDIT_POINT_BASE_REF;
   if ((i<0) || (i>=mPointControl.Count())) return _T("");

   TSTR buf;
   if (mPointRange.Count()>2) {
      // Find out which node, and which vertex, this refers to:
      int j;
      for (j=1; j<mPointRange.Count(); j++) {
         if (i<mPointRange[j]) break;
      }
      if (j==mPointRange.Count()) return _T("");   // shouldn't happen.
      buf.printf ("%s: %s %d", mPointNode[j-1], GetString(IDS_VERTEX), i+1-mPointRange[j-1]);
   } else {
      // Edit Poly only applied to one node, so leave out the node name.
      buf.printf ("%s %d", GetString (IDS_VERTEX), i+1);
   }
   return buf;
}

BOOL EditPolyMod::AssignController (Animatable *control, int subAnim) {
   ReplaceReference (subAnim, (Control*)control);
   if (subAnim==EDIT_MASTER_POINT_REF) {
      int n = mPointControl.Count();
      mpPointMaster->SetNumSubControllers(n);
      for (int i=0; i<n; i++) if (mPointControl[i]) mpPointMaster->SetSubController(i,mPointControl[i]);
   }
   NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
   NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
   return TRUE;
}

BOOL EditPolyMod::SelectSubAnim (int subNum) {
   if (subNum<EDIT_POINT_BASE_REF) return FALSE;   // can only select points, not other controllers.
   subNum -= EDIT_POINT_BASE_REF;
   if (subNum >= mPointControl.Count()) return FALSE;

   int j;
   for (j=1; j<mPointRange.Count(); j++) {
      if (subNum < mPointRange[j]) break;
   }
   if (j==mPointRange.Count()) return false;
   subNum -= mPointRange[j-1];

   // Find the node corresponding to the node name:
   TSTR name = mPointNode[j-1];
   ModContextList list;
   INodeTab nodes;
   ip->GetModContexts (list, nodes);
   int i;
   for (i=0; i<nodes.Count(); i++) {
      if (name == TSTR(nodes[i]->GetName())) break;
   }
   if (i==nodes.Count()) return false;
   INode *pNode = nodes[i]->GetActualINode();
   nodes.DisposeTemporary ();

   EditPolyData *pData = GetEditPolyDataForNode (nodes[i]);
   BitArray nvs;
   nvs.SetSize (pData->mVertSel.GetSize());
   nvs.ClearAll ();
   nvs.Set (subNum);

   BOOL add = GetKeyState(VK_CONTROL)<0;
   BOOL sub = GetKeyState(VK_MENU)<0;

   if (add || sub) {
      EpModSelect (MNM_SL_VERTEX, nvs, false, sub!=0, pNode);
   } else {
      EpModSetSelection (MNM_SL_VERTEX, nvs, pNode);
   }

   EpModRefreshScreen ();
   return TRUE;
}

// Subobject API
int EditPolyMod::NumSubObjTypes() {
   return 5;
}

ISubObjType *EditPolyMod::GetSubObjType(int i) {
   static bool initialized = false;
   if(!initialized) {
      initialized = true;
      SOT_Vertex.SetName(GetString(IDS_EDITPOLY_VERTEX));
      SOT_Edge.SetName(GetString(IDS_EDITPOLY_EDGE));
      SOT_Border.SetName(GetString(IDS_EDITPOLY_BORDER));
      SOT_Face.SetName(GetString(IDS_EDITPOLY_FACE));
      SOT_Element.SetName(GetString(IDS_EDITPOLY_ELEMENT));
   }

   switch(i) {
   case -1:
      if(selLevel > 0) 
         return GetSubObjType(selLevel-1);
      break;
   case 0:
      return &SOT_Vertex;
   case 1:
      return &SOT_Edge;
   case 2:
      return &SOT_Border;
   case 3:
      return &SOT_Face;
   case 4:
      return &SOT_Element;
   }
   return NULL;
}

// Class EPolyBackspaceUser: Used to process backspace key input.
void EditPolyBackspaceUser::Notify() {
   if (!mpEditPoly) return;

   // If we're in create-face mode, send the backspace event there:
   if ((mpEditPoly->createFaceMode)  &&
      (mpEditPoly->ip->GetCommandMode () == mpEditPoly->createFaceMode)) {
      mpEditPoly->createFaceMode->Backspace ();
      return;
   }

   // Otherwise, use it to generate a "remove" event:
   if ((mpEditPoly->selLevel > EPM_SL_OBJECT) && (mpEditPoly->selLevel < EPM_SL_FACE)) {
      theHold.Begin ();
      switch (mpEditPoly->GetMNSelLevel()) {
      case MNM_SL_VERTEX: mpEditPoly->EpModButtonOp (ep_op_remove_vertex); break;
      case MNM_SL_EDGE: mpEditPoly->EpModButtonOp (ep_op_remove_edge); break;
      }
      theHold.Accept (GetString (IDS_REMOVE));
   }
}

void EditPolyMod::SetCommandModeFromOld(int oldCommandMode)
{
	switch (oldCommandMode)
	{
	case ep_mode_create_vertex:
	case ep_mode_create_edge:
	case ep_mode_create_face:
		switch(GetEPolySelLevel())
		{
		case EPM_SL_FACE:
			EpModToggleCommandMode(ep_mode_create_face);
			break;
		case EPM_SL_EDGE:
			EpModToggleCommandMode(ep_mode_create_edge);
			break;
		case EPM_SL_VERTEX:
			EpModToggleCommandMode(ep_mode_create_vertex);
			break;
		}
		break;
	case ep_mode_divide_edge:
	case ep_mode_divide_face:
		switch(GetEPolySelLevel())
		{
		case EPM_SL_FACE:
			EpModToggleCommandMode(ep_mode_divide_face);
			break;
		case EPM_SL_EDGE:
			EpModToggleCommandMode(ep_mode_divide_edge);
			break;
		}
		break;

	case ep_mode_cut:
		EpModToggleCommandMode(ep_mode_cut);
		break;

	case ep_mode_quickslice:
		EpModToggleCommandMode (ep_mode_quickslice);
		break;

	case ep_mode_sliceplane:
		EpModToggleCommandMode (ep_mode_sliceplane);
		break;

	case ep_mode_bridge_border:
	case ep_mode_bridge_polygon:
	case ep_mode_bridge_edge:
		switch(GetEPolySelLevel())
		{
		case EPM_SL_BORDER:
			EpModToggleCommandMode(ep_mode_bridge_border);
			break;
		case EPM_SL_FACE:
			EpModToggleCommandMode(ep_mode_bridge_polygon);
			break;
		case EPM_SL_EDGE:
			EpModToggleCommandMode(ep_mode_bridge_edge);
			break;
		}
		break;
	case ep_mode_extrude_vertex:
	case ep_mode_extrude_edge:
	case ep_mode_extrude_face:
		switch(GetEPolySelLevel())
		{
		case EPM_SL_VERTEX:
			EpModToggleCommandMode(ep_mode_extrude_vertex);
			break;
		case EPM_SL_EDGE:
			EpModToggleCommandMode(ep_mode_extrude_edge);
			break;
		case EPM_SL_FACE:
			EpModToggleCommandMode(ep_mode_extrude_face);
			break;
		}
		break;
	case ep_mode_chamfer_vertex:
	case ep_mode_chamfer_edge:
		switch(GetEPolySelLevel())
		{
		case EPM_SL_VERTEX:
			EpModToggleCommandMode(ep_mode_chamfer_vertex);
			break;
		case EPM_SL_EDGE:
			EpModToggleCommandMode(ep_mode_chamfer_edge);
			break;
		}
		break;
	};
}


void EditPolyMod::ActivateSubobjSel(int level, XFormModes& modes) {
   if (ip) {
      // Register or unregister delete, backspace key notification
      if (selLevel==EPM_SL_OBJECT && level!=EPM_SL_OBJECT) {
         ip->RegisterDeleteUser(this);
         backspacer.SetEPoly (this);
         backspaceRouter.Register (&backspacer);
      }
      if (selLevel!=EPM_SL_OBJECT && level==EPM_SL_OBJECT) {
         ip->UnRegisterDeleteUser(this);
         backspacer.SetEPoly (NULL);
         backspaceRouter.UnRegister (&backspacer);
      }
   }

   int oldMode = EpModGetCommandMode();
   if (selLevel != level) {
      ExitAllCommandModes (level == EPM_SL_OBJECT, level == EPM_SL_OBJECT);
   }

   SetSubobjectLevel(level);

   // Fill in modes with our sub-object modes
   if (level!=EPM_SL_OBJECT)
      modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);


   // Update UI
   UpdateCageCheckboxEnable ();
   UpdateSelLevelDisplay ();
   UpdateDisplayFlags ();
   SetNumSelLabel ();
   InvalidateNumberHighlighted ();
   if (ip) {
      ip->PipeSelLevelChanged();
      SetupNamedSelDropDown ();
      UpdateNamedSelDropDown ();
   }

    SetCommandModeFromOld(oldMode);
}

void EditPolyMod::SetSubobjectLevel(int level) {
   int oldSelLevel = selLevel;
   selLevel = level;

   EpModAboutToChangeSelection ();

   if (mpCurrentEditMod==this) UpdateUIToSelectionLevel (oldSelLevel);
   EpModLocalDataChanged (PART_SUBSEL_TYPE|PART_DISPLAY);

    if (ip)
    {
        ip->RedrawViews (ip->GetTime());
        ModContextList list;
        INodeTab nodes;
        ip->GetModContexts (list, nodes);
        for (int i=0; i<list.Count(); i++)
        {
            EditPolyData *pData = (EditPolyData *)list[i]->localData;
            if (!pData || !pData->GetMesh())
            {
                continue;
            }

            MNMesh & mesh = *(pData->GetMesh());
            mesh.selLevel = meshSelLevel[selLevel];
        }
		//reset these guys, which then makes sure we are all set up for these modes.
		SetSelectMode(GetSelectMode());
		//this will hide/show any grip items if we are in manip mode based upon the sub object mode
		SetUpManipGripIfNeeded();

    }
}

void EditPolyMod::SelectSubComponent (HitRecord *pFirstHit, BOOL selected, BOOL all, BOOL invert)
{
   if (mpParams->GetInt (epm_stack_selection)) return;
   if (!ip) return;
   if (mSliceMode) return;
   if(selLevel==EPM_SL_OBJECT)return;

   if (mSelLevelOverride>0) //special multi or highlight selection
   {

 	  //first we select, based upon what we are selecting based upon selLevel

	   EditPolyData *pData = NULL;
	   HitRecord *pHitRec  = NULL;
	   // Prepare a bitarray representing the particular hits we got:
	   HitRecord* hr = NULL;
	   for (hr=pFirstHit; hr!=NULL; hr = hr->Next())
	   {
			pData = (EditPolyData*)hr->modContext->localData;
			if (!pData->GetNewSelection ())
			{

			   pData->SetupNewSelection (mSelLevelOverride);
			   pData->SynchBitArrays();
			}
			pData->GetNewSelection()->Set (hr->hitInfo, true);
			std::set<int>::iterator it =  pData->mHighlightedItems.begin(); 
			while(it != pData->mHighlightedItems.end()) 
		    {
				pData->GetNewSelection()->Set (*it, true);	
				++it;
		    }
		    pData->mHighlightedItems.clear();
			if (!all) break;
		  
	   }

	   ModContextList list;
	   INodeTab nodes;   
	   ip->GetModContexts(list,nodes);
	   int numContexts = list.Count();
	   nodes.DisposeTemporary ();

	   TCHAR *levelName = LookupMNMeshSelLevel (mSelLevelOverride);
	   // Apply the "new selections" to all the local mod datas:
	   // (Must be after assembling all hits so we don't over-invert normals.)
	   bool changeOccurred = false;
	   for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next())
	   {
			pData = (EditPolyData *) pHitRec->modContext->localData;
			if (!pData->GetNewSelection()) continue;

			if ((GetKeyState(VK_SHIFT) & 0x8000) && mMeshHit)//Select loop or ring if Shift key is pressed
			{
				MNMesh* mm = pData->GetMesh();
				BitArray* newSel = pData->GetNewSelection();
				BitArray currentSel;
				bool foundLoopRing = false;
				IMNMeshUtilities13* l_mesh13 = static_cast<IMNMeshUtilities13*>(mm->GetInterface( IMNMESHUTILITIES13_INTERFACE_ID ));
				if (l_mesh13)
				{
					if (selLevel == EPM_SL_VERTEX)
					{
						mm->getVertexSel(currentSel);
						foundLoopRing = l_mesh13->FindLoopVertex(*newSel,currentSel);
					}
					else if (selLevel == EPM_SL_EDGE)
					{
						mSuspendCloneEdges = true;
						mm->getEdgeSel(currentSel);
						foundLoopRing = l_mesh13->FindLoopOrRingEdge(*newSel,currentSel);
					}
					else if (selLevel == EPM_SL_FACE)
					{
						mm->getFaceSel(currentSel);
						foundLoopRing = l_mesh13->FindLoopFace(*newSel,currentSel);
					}
					if (!foundLoopRing) (*newSel).ClearAll(); //If no loop or ring is found, do not add anything to selection
				}
			}

			if (pData->ApplyNewSelection(this, true, invert?true:false, selected?true:false)) //never invert and always keep selected for the element mode subselection
			 changeOccurred = true;

			if (!all) break;
	   }
	   EpModHighlightChanged();
	   EpModLocalDataChanged (PART_SELECT);

	   //now enter that mode based upon what's selected... if in multi select mode
	   if(GetSelectMode()==PS_MULTI_SEL)
	   {
		   switch(mSelLevelOverride)
		   {
			case MNM_SL_VERTEX:
				SetEPolySelLevel (EPM_SL_VERTEX);
				break;
			case MNM_SL_EDGE:
				SetEPolySelLevel (EPM_SL_EDGE);
				break;
			case MNM_SL_FACE:
				SetEPolySelLevel (EPM_SL_FACE);
				break;
		   };
	   }

	   return;
   }

   EpModAboutToChangeSelection ();

   EditPolyData *pData = NULL;
   HitRecord* pHitRec = NULL;

   int selByVert = mpParams->GetInt (epm_by_vertex);

   // Prepare a bitarray representing the particular hits we got:
   HitRecord* hr = NULL;
   if (selByVert) {
      for (hr=pFirstHit; hr!=NULL; hr = hr->Next()) {
         pData = (EditPolyData*)hr->modContext->localData;
         if (!pData->GetNewSelection ()) {
            pData->SetupNewSelection (MNM_SL_VERTEX);
            pData->SynchBitArrays();
         }
         pData->GetNewSelection()->Set (hr->hitInfo, true);
         if (!all) break;
      }
   } else {
      if (mpParams->GetInt (epm_select_by_angle) && (selLevel == EPM_SL_FACE)) {
         float selectAngle = mpParams->GetFloat (epm_select_angle);
         for (hr=pFirstHit; hr!=NULL; hr = hr->Next()) {
            pData = (EditPolyData*)hr->modContext->localData;
            if (!pData->GetNewSelection ()) {
               pData->SetupNewSelection (meshSelLevel[selLevel]);
               pData->SynchBitArrays();
            }
            if (!pData->GetMesh()) {
               // Mesh mysteriously missing.  Fall back on selecting hit faces.
               pData->GetNewSelection()->Set (hr->hitInfo, true);
            } else {
               MNMeshUtilities mmu(pData->GetMesh());
               mmu.SelectPolygonsByAngle (hr->hitInfo, selectAngle, *(pData->GetNewSelection()));
            }
            if (!all) break;
         }
      } else {
         for (hr=pFirstHit; hr!=NULL; hr = hr->Next()) {
            pData = (EditPolyData*)hr->modContext->localData;
            if (!pData->GetNewSelection ()) {
               pData->SetupNewSelection (meshSelLevel[selLevel]);
               pData->SynchBitArrays();
            }
            pData->GetNewSelection()->Set (hr->hitInfo, true);
            if (!all) break;
         }
      }
   }

   ModContextList list;
   INodeTab nodes;   
   ip->GetModContexts(list,nodes);
   int numContexts = list.Count();
   nodes.DisposeTemporary ();

   TCHAR *levelName = LookupMNMeshSelLevel (meshSelLevel[selLevel]);

   // Apply the "new selections" to all the local mod datas:
   // (Must be after assembling all hits so we don't over-invert normals.)
   bool changeOccurred = false;
   for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next()) {
      pData = (EditPolyData *) pHitRec->modContext->localData;
      if (!pData->GetNewSelection()) continue;

      // Translate the hits into the correct selection level:
      if (selByVert && (selLevel>EPM_SL_VERTEX)) {
         pData->TranslateNewSelection (EPM_SL_VERTEX, selLevel);
      } else {
         if (selLevel == EPM_SL_BORDER) pData->TranslateNewSelection (EPM_SL_EDGE, EPM_SL_BORDER);
         if (selLevel == EPM_SL_ELEMENT) pData->TranslateNewSelection (EPM_SL_FACE, EPM_SL_ELEMENT);
      }

	  if ((GetKeyState(VK_SHIFT) & 0x8000) && mMeshHit) //Select loop or ring if Shift key is pressed
	  {
			MNMesh* mm = pData->GetMesh();
			BitArray* newSel = pData->GetNewSelection();
			BitArray currentSel;
			bool foundLoopRing = false;
			IMNMeshUtilities13* l_mesh13 = static_cast<IMNMeshUtilities13*>(mm->GetInterface( IMNMESHUTILITIES13_INTERFACE_ID ));
			if (l_mesh13)
			{
				if (selLevel == EPM_SL_VERTEX)
				{
					mm->getVertexSel(currentSel);
					foundLoopRing = l_mesh13->FindLoopVertex(*newSel,currentSel);
				}
				if (selLevel == EPM_SL_EDGE)
				{
					mSuspendCloneEdges = true;
					mm->getEdgeSel(currentSel);
					foundLoopRing = l_mesh13->FindLoopOrRingEdge(*newSel,currentSel);
				}
				if (selLevel == EPM_SL_FACE)
				{
					mm->getFaceSel(currentSel);
					foundLoopRing = l_mesh13->FindLoopFace(*newSel,currentSel);
				}
				if (!foundLoopRing) (*newSel).ClearAll(); //If no loop or ring is found, do not add anything to selection
			}
	  }

      // Spit out the quickest script possible for this action.
      if (numContexts>1) {
         if (invert) {
            // Node and Invert values are non-default
            macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].Select"),
               2, 2, mr_name, levelName,
               mr_bitarray, pData->GetNewSelection(),
               _T("invert"), mr_bool, true,
               _T("node"), mr_reftarg, pHitRec->nodeRef);
         } else {
            if (selected) {
               // Node value is non-default
               macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].Select"),
                  2, 1, mr_name, levelName,
                  mr_bitarray, pData->GetNewSelection(),
                  _T("node"), mr_reftarg, pHitRec->nodeRef);
            } else {
               // Node and Selected values are non-default
               macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].Select"),
                  2, 2, mr_name, levelName,
                  mr_bitarray, pData->GetNewSelection(),
                  _T("select"), mr_bool, false,
                  _T("node"), mr_reftarg, pHitRec->nodeRef);
            }
         }
      } else {
         if (invert) {
            // Invert value is non-default
            macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].Select"),
               2, 1, mr_name, levelName,
               mr_bitarray, pData->GetNewSelection(),
               _T("invert"), mr_bool, true);
         } else {
            if (selected) {
               // all values are default
               macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].Select"),
                  2, 0,  mr_name, levelName,
                  mr_bitarray, pData->GetNewSelection());
            } else {
               // Selected value is non-default
               macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].Select"),
                  2, 1,  mr_name, levelName,
                  mr_bitarray, pData->GetNewSelection(),
                  _T("select"), mr_bool, false);
            }
         }
      }

      if (pData->ApplyNewSelection(this, true, invert?true:false, selected?true:false))
         changeOccurred = true;

      if (!all) break;
   }

   if (changeOccurred)
   {
      EpModLocalDataChanged (PART_SELECT);
      macroRecorder->EmitScript ();
   }
}

void EditPolyMod::EpModAboutToChangeSelection () {
   if (!mpCurrentOperation) return;
   if (mpParams->GetInt (epm_animation_mode)) return;
   if ((mpCurrentOperation->OpID() == ep_op_smooth) || (mpCurrentOperation->OpID() == ep_op_set_material))
   {
      TSTR name = mpCurrentOperation->Name();
      bool localHold = (theHold.Holding()==0);
      if (localHold) theHold.Begin ();
      EpModCommit (0, false, true);
      if (localHold) theHold.Accept (name);
   }
}

void EditPolyMod::ClearSelection(int selLevel) {
   if (selLevel == EPM_SL_OBJECT) return; // shouldn't happen
   if (mSliceMode) return;
   if (!ip) return;
   if (mpParams->GetInt (epm_stack_selection)) return;
   if (GetKeyState(VK_SHIFT) & 0x8000) return; //Do not clear selection when attempting to select loop or ring

   EpModAboutToChangeSelection ();

   ModContextList list;
   INodeTab nodes;   
   ip->GetModContexts(list,nodes);

   int msl = meshSelLevel[selLevel];
   TCHAR *levelName = LookupMNMeshSelLevel (msl);

   bool changeOccurred = false;
   for (int i=0; i<list.Count(); i++) {
      EditPolyData *pData = (EditPolyData*)list[i]->localData;
      if (!pData) continue;

      pData->SetupNewSelection (meshSelLevel[selLevel]);
      BitArray *pSel = pData->GetNewSelection();
      pSel->ClearAll ();

      if (list.Count() == 1)
      {
         macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetSelection"),
            2, 0, mr_name, levelName, mr_bitarray, pSel);
         }
      else
      {
         macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetSelection"),
            2, 1, mr_name, levelName, mr_bitarray, pSel,
            _T("node"), mr_reftarg, nodes[i]);
      }

      if (pData->ApplyNewSelection(this)) changeOccurred = true;
   }
   nodes.DisposeTemporary();
   if (changeOccurred) EpModLocalDataChanged (PART_SELECT);
}

void EditPolyMod::SelectAll(int selLevel) {
   if (!ip) return;
   if (mSliceMode) return;
   if (mpParams->GetInt (epm_stack_selection)) return;

   EpModAboutToChangeSelection ();

   ModContextList list;
   INodeTab nodes;   
   ip->GetModContexts(list,nodes);

   int msl = meshSelLevel[selLevel];
   TCHAR *levelName = LookupMNMeshSelLevel (msl);

   bool changeOccurred = false;
   for (int i=0; i<list.Count(); i++) {
      EditPolyData *pData = (EditPolyData*)list[i]->localData;
      if (!pData) continue;

      pData->SetupNewSelection (msl);
      BitArray *pSel = pData->GetNewSelection();
      pSel->SetAll ();

      if (list.Count() == 1)
      {
         macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetSelection"),
            2, 0, mr_name, levelName, mr_bitarray, pSel);
         }
      else
      {
         macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetSelection"),
            2, 1, mr_name, levelName, mr_bitarray, pSel,
            _T("node"), mr_reftarg, nodes[i]);
      }

      if (pData->ApplyNewSelection(this)) changeOccurred = true;
   }
   nodes.DisposeTemporary();
   if (changeOccurred) EpModLocalDataChanged (PART_SELECT);
}

void EditPolyMod::InvertSelection(int selLevel) {
   if (!ip) return;
   if (mSliceMode) return;
   if (mpParams->GetInt (epm_stack_selection)) return;

   EpModAboutToChangeSelection ();

   ModContextList list;
   INodeTab nodes;   
   ip->GetModContexts(list,nodes);

   int msl = meshSelLevel[selLevel];
   TCHAR *levelName = LookupMNMeshSelLevel (msl);

   bool changeOccurred = false;
   for (int i=0; i<list.Count(); i++) {
      EditPolyData *pData = (EditPolyData*)list[i]->localData;
      if (!pData) continue;

      pData->SetupNewSelection (msl);
      BitArray *pSel = pData->GetNewSelection();
      BitArray *pCurrent = pData->GetCurrentSelection (msl, false);
      for (int j=0; j<pCurrent->GetSize(); j++)
      {
         pSel->Set(j, !(*pCurrent)[j]);
      }

      if (list.Count() == 1)
      {
         macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetSelection"),
            2, 0, mr_name, levelName, mr_bitarray, pSel);
         }
      else
      {
         macroRecorder->FunctionCall(_T("$.modifiers[#Edit_Poly].SetSelection"),
            2, 1, mr_name, levelName, mr_bitarray, pSel,
            _T("node"), mr_reftarg, nodes[i]);
      }

      if (pData->ApplyNewSelection(this, false)) changeOccurred = true;
   }
   nodes.DisposeTemporary();
   if (changeOccurred) EpModLocalDataChanged (PART_SELECT);
}

BitArray *EditPolyMod::EpModGetSelection (int msl, INode *pNode) {

   if (!ip) return NULL;

   ModContextList list;
   INodeTab nodes;
   ip->GetModContexts(list,nodes);
   nodes.DisposeTemporary ();

   int i;
   if (pNode) {
      for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
   } else i=0;
   if (i==list.Count()) return NULL;

   EditPolyData *pData = (EditPolyData*)list[i]->localData;
   if (!pData) return NULL;
   if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
   bool useStackSel = (mpParams->GetInt (epm_stack_selection) != 0);

   return pData->GetCurrentSelection (msl, useStackSel);
}

bool EditPolyMod::EpModSetSelection (int msl, BitArray & selection, INode *pNode) {
   if (!ip) return false;

   EpModAboutToChangeSelection ();

   ModContextList list;
   INodeTab nodes;
   ip->GetModContexts(list,nodes);

   int i;
   if (pNode) {
      for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
   } else i=0;
   if (i==list.Count()) return false;

   EditPolyData *pData = (EditPolyData*)list[i]->localData;
   if (!pData) return false;

   if (theHold.Holding())
      theHold.Put(new EditPolySelectRestore(this, pData, msl));

   if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
   switch (msl)
   {
   case MNM_SL_VERTEX:
      pData->SetVertSel (selection, this, TimeValue(0));
      break;
   case MNM_SL_EDGE:
      pData->SetEdgeSel (selection, this, TimeValue(0));
      break;
   case MNM_SL_FACE:
      pData->SetFaceSel (selection, this, TimeValue(0));
      break;
   }

   EpModLocalDataChanged (PART_SELECT);
   return true;
}

bool EditPolyMod::EpModSelect (int msl, BitArray & selection, bool invert, bool select, INode *pNode) {
   if (!ip) return false;

   EpModAboutToChangeSelection ();

   ModContextList list;
   INodeTab nodes;
   ip->GetModContexts(list,nodes);

   int i;
   if (pNode) {
      for (i=0; i<list.Count(); i++) if (nodes[i] == pNode) break;
   } else i=0;
   if (i==list.Count()) return false;

   EditPolyData *pData = (EditPolyData*)list[i]->localData;
   if (!pData) return false;

   if (theHold.Holding())
      theHold.Put(new EditPolySelectRestore(this, pData, msl));

   if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
   pData->SetupNewSelection (msl);
   BitArray *pSel = pData->GetNewSelection();
   int commonLength = selection.GetSize();
   if (pSel->GetSize()<commonLength) commonLength = pSel->GetSize();
   for (i=0; i<commonLength; i++) pSel->Set(i, selection[i]);
   pData->ApplyNewSelection (this, true, invert, select);

   EpModLocalDataChanged (PART_SELECT);
   return true;
}

// From IMeshSelect
DWORD EditPolyMod::GetSelLevel() {
   switch (selLevel) {
   case EPM_SL_OBJECT: return IMESHSEL_OBJECT;
   case EPM_SL_VERTEX: return IMESHSEL_VERTEX;
   case EPM_SL_EDGE:
   case EPM_SL_BORDER:
      return IMESHSEL_EDGE;
   }
   return IMESHSEL_FACE;
}

int EditPolyMod::GetCalcMeshSelLevel(int i)
{
	if(mSelLevelOverride>0)
		return mSelLevelOverride;
	else
		return meshSelLevel[i];
}

void EditPolyMod::SetSelLevel(DWORD level) {
   // This line protects against changing from border to edge, for instance, when told to switch to edge:
   if (GetSelLevel() == level) return;

   switch (level) {
   case IMESHSEL_OBJECT:
      selLevel = EPM_SL_OBJECT;
      break;
   case IMESHSEL_VERTEX:
      selLevel = EPM_SL_VERTEX;
      break;
   case IMESHSEL_EDGE:
      selLevel = EPM_SL_EDGE;
      break;
   case IMESHSEL_FACE:
      selLevel = EPM_SL_FACE;
      break;
   }
	//reset these guys, which then makes sure we are all set up for these modes.
	SetSelectMode(GetSelectMode());
	//this will hide/show any grip items if we are in manip mode based upon the sub object mode
	SetUpManipGripIfNeeded();
   if (ip) ip->SetSubObjectLevel(selLevel);
}

BaseInterface *EditPolyMod::GetInterface (Interface_ID id) {
   if (id == EPOLY_MOD_INTERFACE) return (EPolyMod *)this;
   else if (id == EPOLY_MOD13_INTERFACE) return (EPolyMod13 *)this;
   return FPMixinInterface::GetInterface(id);
}

void EditPolyMod::LocalDataChanged() {
   EpModLocalDataChanged (PART_SELECT);
}

void EditPolyMod::UpdateCache(TimeValue t) {
	// the following NotifyDependents call causes ModAppEvaluator::PrepareForReEval to be called.
	// in ModAppEvaluator::PrepareForReEval, a REFMSG_OBJECT_CACHE_DUMPED notification
	// is sent to the dependents of the ModApp. This notification ends up going to EditPolyMod::NotifyInputChanged,
	// which does a PostMessage to send a WM_UPDATE_CACHE message to the EditPolySelectProc::DlgProc,
	// which calls EditPolyMod::UpdateCache, putting us right back where we are at now. Since it’s a PostMessage
	// the message isn’t processed until after we have updated the cache. Thus, we end up in an infinite loop
	// By setting mUpdateCachePosted to true, we don’t do the PostMessage call, breaking the loop
	// laurent r and larry m , bug fix 911981, 04/07
	mUpdateCachePosted = true;
   NotifyDependents(Interval(t,t), PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|
      PART_DISPLAY|PART_TOPO, REFMSG_MOD_EVAL);
   mUpdateCachePosted = FALSE;
}

bool EditPolyMod::IsVertexSelected (EditPolyData *pData, int vertexID) {
   int stackSel;
   mpParams->GetValue (epm_stack_selection, 0, stackSel, FOREVER);
   if (!stackSel) {
      return pData->mVertSel[vertexID] ? true : false;
   } else {
      return pData->GetMesh() ? pData->GetMesh()->v[vertexID].GetFlag (MN_SEL) : false;
   }
}

bool EditPolyMod::IsEdgeSelected (EditPolyData *pData, int edgeID) {
   int stackSel;
   mpParams->GetValue (epm_stack_selection, 0, stackSel, FOREVER);
   if (!stackSel) {
      return pData->mEdgeSel[edgeID] ? true : false;
   } else {
      return pData->GetMesh() ? pData->GetMesh()->e[edgeID].GetFlag (MN_SEL) : false;
   }
}

bool EditPolyMod::IsFaceSelected (EditPolyData *pData, int faceID) {
   int stackSel;
   mpParams->GetValue (epm_stack_selection, 0, stackSel, FOREVER);
   if (!stackSel) {
      return pData->mFaceSel[faceID] ? true : false;
   } else {
      return pData->GetMesh() ? pData->GetMesh()->f[faceID].GetFlag (MN_SEL) : false;
   }
}

void EditPolyMod::GetVertexTempSelection (EditPolyData *pData, BitArray & vertexTempSel) {
   int stackSel;
   mpParams->GetValue (epm_stack_selection, 0, stackSel, FOREVER);
   if (!stackSel) {
      if (pData->GetMesh()) vertexTempSel.SetSize (pData->GetMesh()->numv);
      else vertexTempSel.SetSize (pData->mVertSel.GetSize());
      int i = 0, max = 0;
      MNMesh* pMesh = NULL;
      switch (meshSelLevel[selLevel]) {
      case MNM_SL_OBJECT:
         vertexTempSel.SetAll();
         break;
      case MNM_SL_VERTEX:
         vertexTempSel = pData->mVertSel;
         break;
      case MNM_SL_EDGE:
         vertexTempSel.ClearAll();
         pMesh = pData->GetMesh();
         if (!pMesh) break;
         max = pMesh->nume;
         if (pData->mEdgeSel.GetSize()<max) max = pData->mEdgeSel.GetSize();
         for (i=0; i<max; i++) {
            if (pMesh->e[i].GetFlag (MN_DEAD)) continue;
            if (!pData->mEdgeSel[i]) continue;
            vertexTempSel.Set (pMesh->e[i][0]);
            vertexTempSel.Set (pMesh->e[i][1]);
         }
         break;
      case MNM_SL_FACE:
         vertexTempSel.ClearAll();
         pMesh = pData->GetMesh();
         if (!pMesh) break;
         max = pMesh->numf;
         if (pData->mFaceSel.GetSize()<max) max = pData->mFaceSel.GetSize();
         for (i=0; i<max; i++) {
            if (pMesh->f[i].GetFlag (MN_DEAD)) continue;
            if (!pData->mFaceSel[i]) continue;
            for (int j=0; j<pMesh->f[i].deg; j++) {
               vertexTempSel.Set (pMesh->f[i].vtx[j]);
            }
         }
         break;
      }
   } else {
      if (pData->GetMesh()) vertexTempSel = pData->GetMesh()->VertexTempSel ();
      else {
         vertexTempSel.SetSize (pData->mVertSel.GetSize());
         vertexTempSel.ClearAll ();
      }
   }
}

int EditPolyMod::EpModConvertSelection (int epSelLevelFrom, int epSelLevelTo, bool requireAll) {
   if (!ip) return 0;

   if (mpParams->GetInt (epm_stack_selection)) return 0;

   if (epSelLevelFrom == EPM_SL_CURRENT) epSelLevelFrom = selLevel;
   if (epSelLevelTo == EPM_SL_CURRENT) epSelLevelTo = selLevel;
   if (epSelLevelFrom == epSelLevelTo) return 0;

   // We don't select the whole object here.
   if (epSelLevelTo == EPM_SL_OBJECT) return 0;

   EpModAboutToChangeSelection ();

   ModContextList list;
   INodeTab nodes;   
   ip->GetModContexts(list,nodes);

   int numComponentsSelected = 0;
   for (int i=0; i<list.Count(); i++) {
      EditPolyData *pData = (EditPolyData*)list[i]->localData;
      if (!pData) continue;
      MNMesh *pMesh = pData->GetMesh ();
      if (!pMesh) continue;
      DbgAssert (pMesh->GetFlag (MN_MESH_FILLED_IN));
      if (!pMesh->GetFlag (MN_MESH_FILLED_IN)) continue;

      // If 'from' is the whole object, we select all.
      BitArray sel;
      if (epSelLevelFrom == EPM_SL_OBJECT) {
         switch (epSelLevelTo) {
         case EPM_SL_VERTEX: 
            sel.SetSize (pMesh->numv);
            sel.SetAll();
            pData->SetVertSel (sel, this, ip->GetTime());
            break;
         case EPM_SL_EDGE:
            sel.SetSize (pMesh->nume);
            sel.SetAll();
            pData->SetEdgeSel (sel, this, ip->GetTime());
            break;
         case EPM_SL_BORDER:
            {
            sel.SetSize (pMesh->nume);
               for (int i=0; i<pMesh->nume; i++) sel.Set (i, pMesh->e[i].f2<0);
            pData->SetEdgeSel (sel, this, ip->GetTime());
            }
            break;
         default:
            sel.SetSize (pMesh->numf);
            sel.SetAll(); 
            pData->SetFaceSel (sel, this, ip->GetTime());
            break;
         }
         numComponentsSelected += sel.NumberSet();
      } else {
         int mslFrom = meshSelLevel[epSelLevelFrom];
         int mslTo = meshSelLevel[epSelLevelTo];

         if (mslTo != mslFrom) {
            switch (mslFrom)
            {
            case MNM_SL_VERTEX:
               for (i=0; i<pData->mVertSel.GetSize(); i++)
                  pMesh->v[i].SetFlag (MN_EDITPOLY_OP_SELECT, pData->mVertSel[i]!=0);
               break;
            case MNM_SL_EDGE:
               for (i=0; i<pData->mEdgeSel.GetSize(); i++)
                  pMesh->e[i].SetFlag (MN_EDITPOLY_OP_SELECT, pData->mEdgeSel[i]!=0);
               break;
            case MNM_SL_FACE:
               for (i=0; i<pData->mFaceSel.GetSize(); i++)
                  pMesh->f[i].SetFlag (MN_EDITPOLY_OP_SELECT, pData->mFaceSel[i]!=0);
               break;
            }

            switch (mslTo) {
            case MNM_SL_VERTEX:
               pMesh->ClearVFlags (MN_EDITPOLY_OP_SELECT);
               break;
            case MNM_SL_EDGE:
               pMesh->ClearEFlags (MN_EDITPOLY_OP_SELECT);
               break;
            case MNM_SL_FACE:
               pMesh->ClearFFlags (MN_EDITPOLY_OP_SELECT);
               break;
            }

            // Then propegate flags using the convenient MNMesh method:
            pMesh->PropegateComponentFlags (mslTo, MN_EDITPOLY_OP_SELECT, mslFrom, MN_EDITPOLY_OP_SELECT, requireAll);

            switch (epSelLevelTo) {
            case EPM_SL_VERTEX:
               pMesh->getVerticesByFlag (sel, MN_EDITPOLY_OP_SELECT);
               pData->SetVertSel (sel, this, ip->GetTime());
               break;

            case EPM_SL_EDGE:
               pMesh->getEdgesByFlag (sel, MN_EDITPOLY_OP_SELECT);
               pData->SetEdgeSel (sel, this, ip->GetTime());
               break;

            case EPM_SL_FACE:
               pMesh->getFacesByFlag (sel, MN_EDITPOLY_OP_SELECT);
               pData->SetFaceSel (sel, this, ip->GetTime());
               break;

            case EPM_SL_BORDER:
               sel.SetSize(pMesh->nume);
               sel.ClearAll();

               if (requireAll) {
                  // If we require all, then we just need to deselect any borders that have any unselected edges.
                  for (i=0; i<sel.GetSize(); i++) {
                     if (pMesh->e[i].GetFlag (MN_EDITPOLY_OP_SELECT)) continue;
                     if (sel[i]) continue;
                     if (pMesh->e[i].f2 > -1) continue;
                     pMesh->BorderFromEdge (i, sel);
                  }
                  // sel now contains all the edges in borders with any unselected edges.
                  // Deselect "sel".
                  for (i=0; i<pMesh->nume; i++) {
                     sel.Set (i, !sel[i] && (pMesh->e[i].f2<0) && pMesh->e[i].GetFlag (MN_EDITPOLY_OP_SELECT));
                  }
                  pData->SetEdgeSel (sel, this, ip->GetTime());
               } else {
                  for (i=0; i<sel.GetSize(); i++) {
                     if (!pMesh->e[i].GetFlag (MN_EDITPOLY_OP_SELECT)) continue;
                     if (sel[i]) continue;
                     if (pMesh->e[i].f2 > -1) continue;
                     pMesh->BorderFromEdge (i, sel);
                  }
                  pData->SetEdgeSel (sel, this, ip->GetTime());
               }
               break;

            case EPM_SL_ELEMENT:
               sel.SetSize(pMesh->numf);
               sel.ClearAll();

               if (requireAll) {
                  // If we require all, then we just need to deselect any elements that have any unselected faces.
                  for (int i=0; i<pMesh->numf; i++) {
                     if (pMesh->f[i].GetFlag (MN_EDITPOLY_OP_SELECT)) continue;
                     if (sel[i]) continue;
                     pMesh->ElementFromFace (i, sel);
                  }
                  // sel now contains all the faces in elements with any unselected faces.
                  // Deselect "sel".
                  for (int i=0; i<pMesh->numf; i++) {
                     sel.Set (i, !sel[i] && pMesh->f[i].GetFlag (MN_EDITPOLY_OP_SELECT));
                  }
                  pData->SetFaceSel (sel, this, ip->GetTime());
               } else {
                  for (int i=0; i<pMesh->numf; i++) {
                     if (!pMesh->f[i].GetFlag (MN_EDITPOLY_OP_SELECT)) continue;
                     if (sel[i]) continue;
                     pMesh->ElementFromFace (i, sel);
                  }
                  pData->SetFaceSel (sel, this, ip->GetTime());
               }
               break;
            }
         } else {
            // We do nothing for vertex, edge, or face levels.

            switch (epSelLevelTo) {
            case EPM_SL_BORDER:
               sel.SetSize(pMesh->nume);
               sel.ClearAll();

               if (requireAll) {
                  // If we require all, then we just need to deselect any borders that have any unselected edges.
                  BitArray & curSel = pData->mEdgeSel;
                  for (i=0; i<sel.GetSize(); i++) {
                     if (curSel[i]) continue;
                     if (sel[i]) continue;
                     if (pMesh->e[i].f2 > -1) continue;
                     pMesh->BorderFromEdge (i, sel);
                  }
                  // sel now contains all the edges in borders with any unselected edges.
                  // Deselect "sel".
                  for (i=0; i<sel.GetSize(); i++)
                     sel.Set (i, !sel[i] && curSel[i] && (pMesh->e[i].f2<0));
                  pData->SetEdgeSel (sel, this, ip->GetTime());
               } else {
                  BitArray & curSel = pData->mEdgeSel;
                  for (i=0; i<sel.GetSize(); i++) {
                     if (!curSel[i]) continue;
                     if (sel[i]) continue;
                     if (pMesh->e[i].f2 > -1) continue;
                     pMesh->BorderFromEdge (i, sel);
                  }
                  pData->SetEdgeSel (sel, this, ip->GetTime());
               }
               break;

            case EPM_SL_ELEMENT:
               sel.SetSize(pMesh->numf);
               sel.ClearAll();

               if (requireAll) {
                  // If we require all, then we just need to deselect any elements that have any unselected faces.
                  BitArray & curSel = pData->mFaceSel;
                  for (int i=0; i<pMesh->numf; i++) {
                     if (curSel[i]) continue;
                     if (sel[i]) continue;
                     pMesh->ElementFromFace (i, sel);
                  }
                  // sel now contains all the faces in elements with any unselected faces.
                  // Deselect "sel".
                  sel = (~sel) & curSel;
                  pData->SetFaceSel (sel, this, ip->GetTime());
               } else {
                  BitArray & curSel = pData->mFaceSel;
                  for (int i=0; i<pMesh->numf; i++) {
                     if (!curSel[i]) continue;
                     if (sel[i]) continue;
                     pMesh->ElementFromFace (i, sel);
                  }
                  pData->SetFaceSel (sel, this, ip->GetTime());
               }
               break;
            }
         }
         numComponentsSelected += sel.NumberSet();
      }
   }

   if (requireAll) {
      macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].ConvertSelection"), 2, 1,
         mr_name, LookupEditPolySelLevel (epSelLevelFrom),
         mr_name, LookupEditPolySelLevel (epSelLevelTo),
         _T("requireAll"), mr_bool, true);
   } else {
      macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].ConvertSelection"), 2, 0,
         mr_name, LookupEditPolySelLevel (epSelLevelFrom),
         mr_name, LookupEditPolySelLevel (epSelLevelTo));
   }
   macroRecorder->EmitScript ();

   EpModLocalDataChanged (PART_SELECT);
   return numComponentsSelected;
}

//////////////////////////////////////////////////////////////////////////
//    this method uses a simple algorithm to convert a selection between 
//    3 different types ( only ) faces, vertices and edges, into its border sub-objects. 
//    to convert a set of faces into border vertices, we set a flag ( MN_USER ), to all 
//    selected faces' vertices, all other non-selected faces vertices have another flag set ( MN_NOT_SELECTED ), 
//    we only select vertices having both flags set. they represent the vertices on the border of 
//    the selected faces. 
// different algorithms apply for the conversion between the other sub-object types. lr, fev 05
//////////////////////////////////////////////////////////////////////////


int EditPolyMod::EpModConvertSelectionToBorder (int in_epSelLevelFrom, int in_epSelLevelTo) 
{
   if (!ip) 
      return 0;

   if (mpParams->GetInt (epm_stack_selection)) 
      return 0;

   if (in_epSelLevelFrom == EPM_SL_CURRENT) 
      in_epSelLevelFrom = selLevel;

   if (in_epSelLevelTo == EPM_SL_CURRENT) 
      in_epSelLevelTo = selLevel;

   if (in_epSelLevelFrom == in_epSelLevelTo) 
      return 0;

   // We don't select the whole object here.
   if (in_epSelLevelTo == EPM_SL_OBJECT) 
      return 0;

   EpModAboutToChangeSelection ();

   ModContextList list;
   INodeTab nodes;   
   ip->GetModContexts(list,nodes);

   int l_numComponentsSelected = 0;

   DWORD MN_NOT_SELECTED   = MN_USER << 1; 
   DWORD l_Edge2Faceflag   = MN_SEL | MN_WHATEVER ; 
   DWORD l_BorderFlag      = MN_USER | MN_NOT_SELECTED ; 
   DWORD l_flag; 

   BitArray l_newSel;


   for ( int i = 0; i < list.Count(); i++) 
   {
      EditPolyData *pData = (EditPolyData*)list[i]->localData;

      if (!pData) 
         continue;

      MNMesh *pMesh = pData->GetMesh ();

      if (!pMesh) 
         continue;

      DbgAssert (pMesh->GetFlag (MN_MESH_FILLED_IN));

      if (!pMesh->GetFlag (MN_MESH_FILLED_IN)) 
         continue;

      switch (in_epSelLevelTo )
      {
         case EPM_SL_VERTEX:
            pMesh->ClearVFlags (MN_SEL);
            break;
         case EPM_SL_EDGE:
            pMesh->ClearEFlags (MN_SEL);
            break;
         case EPM_SL_FACE:
            pMesh->ClearFFlags (MN_SEL);
            break;
         case EPM_SL_BORDER:
         case EPM_SL_ELEMENT:
            return 0;
            break;

      }


      switch (in_epSelLevelFrom) {
         case EPM_SL_FACE: 
         case EPM_SL_ELEMENT:
            {
               pMesh->ClearVFlags (l_BorderFlag);
               pMesh->ClearEFlags (l_BorderFlag);

               for (int i=0; i <pMesh->numf; i++)
               {
                  if (pMesh->f[i].GetFlag (MN_DEAD)) 
                     continue;

                  if (pMesh->f[i].GetFlag (MN_SEL))
                     l_flag = MN_USER;
                  else 
                     l_flag = MN_NOT_SELECTED;

                  for (int j = 0; j < pMesh->f[i].deg; j++) 
                  {
                     if ( in_epSelLevelTo == EPM_SL_VERTEX )
                     {
                        pMesh->v[pMesh->f[i].vtx[j]].SetFlag (l_flag);
                     }
                     else if (in_epSelLevelTo == EPM_SL_EDGE )
                     {
                        // set all edges of this face to being set
                        pMesh->e[pMesh->f[i].edg[j]].SetFlag (l_flag);
                     }
                  }

               }

               if ( in_epSelLevelTo == EPM_SL_VERTEX )
               {
                  l_newSel.SetSize (pMesh->numv);
                  for (int i=0; i<pMesh->numv; i++) 
                     l_newSel.Set (i, pMesh->v[i].GetFlag (MN_USER) && (pMesh->v[i].GetFlag (MN_NOT_SELECTED) ||
                     pMesh->vedg[i].Count() > pMesh->vfac[i].Count()));

                  pData->SetVertSel(l_newSel, this, ip->GetTime());

               }
               else if ( in_epSelLevelTo == EPM_SL_EDGE )
               {
                  l_newSel.SetSize (pMesh->nume);
                  for (int i=0; i<pMesh->nume; i++) 
                     l_newSel.Set (i, pMesh->e[i].GetFlag (MN_USER) && (pMesh->e[i].GetFlag (MN_NOT_SELECTED)||
                     pMesh->e[i].f2 < 0));
                  pData->SetEdgeSel(l_newSel, this, ip->GetTime());
               }

            }
            break;

         case EPM_SL_EDGE: //in_epSelLevelFrom
         case EPM_SL_BORDER:
         case EPM_SL_VERTEX: //in_epSelLevelFrom
            {
               pMesh->ClearVFlags (l_BorderFlag  );
               pMesh->ClearEFlags (l_BorderFlag);

               if ( in_epSelLevelFrom == EPM_SL_EDGE)
               {
                  //go through the vertices 
                  for ( int j =0; j < pMesh->nume; j++)
                  {
                     if (pMesh->e[j].GetFlag (MN_DEAD)) continue;

                     if ( pMesh->e[j].GetFlag (MN_SEL))
                     {
                        pMesh->v[pMesh->e[j].v1].SetFlag(MN_WHATEVER);
                        pMesh->v[pMesh->e[j].v2].SetFlag(MN_WHATEVER);
                     }
                  }
               }

               for (int i = 0; i < pMesh->numv; i++)
               {
                  if (pMesh->v[i].GetFlag (MN_DEAD)) 
                     continue;


                  for ( int j = 0; j < pMesh->vedg[i].Count(); j++) 
                  {
                     // determine which vertices are at the 'border
                     // is this vertex on the 'border' ? 
                     if (  (pMesh->v[i].GetFlag (l_Edge2Faceflag)) &&
                        (!pMesh->v[pMesh->e[pMesh->vedg[i][j]].v1].GetFlag(l_Edge2Faceflag)||
                        !pMesh->v[pMesh->e[pMesh->vedg[i][j]].v2].GetFlag(l_Edge2Faceflag) || 
                        pMesh->e[pMesh->vedg[i][j]].f2 < 0 ))
                     {
                        pMesh->v[i].SetFlag (l_BorderFlag);
                     }
                  }
                  
               }

               // process now the 'border vertices'
               l_newSel.SetSize (pMesh->nume);
               for (int i = 0; i < pMesh->numv; i++) 
               {
                  if ( pMesh->v[i].GetFlag(l_BorderFlag)) 
                  {
                     for ( int j = 0; j < pMesh->vedg[i].Count(); j++) 
                     {
                        if (( pMesh->e[pMesh->vedg[i][j]].v1 == i && 
                           pMesh->v[pMesh->e[pMesh->vedg[i][j]].v2].GetFlag(l_BorderFlag)) ||   
                           (pMesh->e[pMesh->vedg[i][j]].v2 == i  &&
                           pMesh->v[pMesh->e[pMesh->vedg[i][j]].v1].GetFlag(l_BorderFlag) ))
                              // this edge is a border edge 
                              pMesh->e[pMesh->vedg[i][j]].SetFlag(l_BorderFlag);

                        
                     }

                  }
                  

                  if ( in_epSelLevelTo == EPM_SL_EDGE )
                  {

                     // process now the border edges. 
                     for (int i = 0; i < pMesh->nume; i++) 
                     {
                        l_newSel.Set (i, pMesh->e[i].GetFlag (l_BorderFlag));
                     }
                     
                     pData->SetEdgeSel(l_newSel, this, ip->GetTime());
                  }
                  else if (  in_epSelLevelTo == EPM_SL_FACE )
                  {
                     // use the 'border' edges to figure out the 'border' faces 
                     l_newSel.SetSize (pMesh->numf);
                     for (int i = 0; i < pMesh->nume; i++) 
                     {
                        if (  pMesh->e[i].GetFlag (l_BorderFlag))
                        {
                           // this is a edge border 
                           for ( int k = 0 ; k < 2 ; k++ )
                           {
                              // process both faces 
                              int l_faceIndex = 0;

                              if ( k == 0 )
                                 l_faceIndex = pMesh->e[i].f1;
                              else 
                                 l_faceIndex = pMesh->e[i].f2;

                              if ( l_faceIndex >= 0 )
                              {
                                 int l_numOfSelectedVertices = 0 ;
                                 for ( int j =0; j < pMesh->f[l_faceIndex].deg ; j++)
                                 {
                                    // loop through all vertices 
                                    if ( (in_epSelLevelFrom == EPM_SL_VERTEX && pMesh->v[pMesh->f[l_faceIndex].vtx[j]].GetFlag(MN_SEL)) ||
                                        (in_epSelLevelFrom == EPM_SL_EDGE   && pMesh->v[pMesh->f[l_faceIndex].vtx[j]].GetFlag(MN_WHATEVER)) )

                                    {
                                       l_numOfSelectedVertices++;
                                    }
                                 }

                                 // this is an interior face , who's edge is on the border 
                                 if ( l_numOfSelectedVertices == pMesh->f[l_faceIndex].deg)
                                    l_newSel.Set (l_faceIndex, true);
                              }

                           }

                        }

                     }

                     pData->SetFaceSel(l_newSel, this, ip->GetTime());


                  }
               }        
            }
            break ; 

         default: 
            break;



      }
      l_numComponentsSelected += l_newSel.NumberSet();

      // cleanup temp flags 
      switch (in_epSelLevelTo )
      {
         case EPM_SL_VERTEX:
            pMesh->ClearVFlags (l_BorderFlag | l_Edge2Faceflag );
            break;
         case EPM_SL_EDGE:
            pMesh->ClearEFlags (l_BorderFlag | l_Edge2Faceflag);
            break;
         case EPM_SL_FACE:
            pMesh->ClearFFlags (l_BorderFlag | l_Edge2Faceflag);
            break;
      }  
   }


   macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].ConvertSelectionToBorder"), 2, 0,
      mr_name, LookupEditPolySelLevel (in_epSelLevelFrom),
      mr_name, LookupEditPolySelLevel (in_epSelLevelTo));
   macroRecorder->EmitScript ();


   EpModLocalDataChanged (PART_SELECT);
   return l_numComponentsSelected;


}

void EditPolyMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {
   if (!mc->localData) return;
   if (partID == PART_SELECT) return;
   if( InPaintMode() ) EndPaintMode();
   ((EditPolyData*)mc->localData)->FreeCache();
   if (!ip || (mpCurrentEditMod != this) || mUpdateCachePosted) return;

   if (!mpDialogSelect) return;
   HWND hWnd = mpDialogSelect->GetHWnd();
   if (!hWnd) return;

   TimeValue t = ip->GetTime();
   PostMessage(hWnd,WM_UPDATE_CACHE,(WPARAM)t,0);
   mUpdateCachePosted = true;
}


// IO
const int kChunkModifier = 0x80;
const int kChunkSelLevel = 0x100;
const int kChunkPreserveMapSettings = 0x110;
const int kChunkMeshPaintHandler = 0x120;
const int kChunkPointRange = 0x130;
const int kChunkPointRangeNum = 0x138;
const int kChunkPointNodeName = 0x140;
const int kChunkPointNodeNameNum = 0x148;
const int kManipBitArray = 0x149;

IOResult EditPolyMod::Save(ISave *isave) {
   IOResult res;
   ULONG nb;

   isave->BeginChunk (kChunkModifier);
   Modifier::Save(isave);
   isave->EndChunk ();

   isave->BeginChunk(kChunkSelLevel);
   res = isave->Write(&selLevel, sizeof(selLevel), &nb);
   isave->EndChunk();

   isave->BeginChunk (kChunkPreserveMapSettings);
   mPreserveMapSettings.Save (isave);
   isave->EndChunk();

   isave->BeginChunk(kChunkMeshPaintHandler);
   MeshPaintHandler::Save(isave);
   isave->EndChunk();

   int num = mPointRange.Count();
   if (num) {
      isave->BeginChunk (kChunkPointRangeNum);
      isave->Write (&num, sizeof(int), &nb);
      isave->EndChunk ();

      isave->BeginChunk (kChunkPointRange);
      isave->Write (mPointRange.Addr(0), sizeof(int)*num, &nb);
      isave->EndChunk ();
   }

   num = static_cast<int>(mPointNode.length());
   if (num) {
      isave->BeginChunk (kChunkPointNodeNameNum);
      isave->Write (&num, sizeof(int), &nb);
      isave->EndChunk ();

      for (int i=0; i<num; i++) {
         isave->BeginChunk (kChunkPointNodeName);
         //isave->Write (&i, sizeof(int), &nb);
         isave->WriteWString (mPointNode[i]);
         isave->EndChunk ();
      }
   }

   return IO_OK;
}

IOResult EditPolyMod::Load(ILoad *iload) {
   IOResult res(IO_OK);
   ULONG nb(0);
   int i(0), num(0);
   TCHAR* readString = NULL;
   TSTR empty;

   while (IO_OK==(res=iload->OpenChunk())) {
      switch(iload->CurChunkID())  {
      case kChunkModifier:
         res = Modifier::Load(iload);
         break;
      case kChunkSelLevel:
         res = iload->Read(&selLevel, sizeof(selLevel), &nb);
         break;
      case kChunkPreserveMapSettings:
         res = mPreserveMapSettings.Load (iload);
         break;
      case kChunkMeshPaintHandler:
         res = MeshPaintHandler::Load (iload);
         break;
	  case kManipBitArray:
		  {
			 BitArray temp;
			 temp.SetSize(mManipGrip.GetNumGripItems());
			 res = temp.Load(iload);
		  }
		  break;
      case kChunkPointRangeNum:
         res = iload->Read (&num, sizeof(int), &nb);
         if (res != IO_OK) break;
         mPointRange.SetCount (num);
         break;

      case kChunkPointRange:
         res = iload->Read (mPointRange.Addr(0), sizeof(int)*mPointRange.Count(), &nb);
         if (res != IO_OK) break;
         AllocPointControllers (mPointRange[mPointRange.Count()-1]);
         break;

      case kChunkPointNodeNameNum:
         res = iload->Read (&num, sizeof(int), &nb);
         if (res != IO_OK) break;
         mPointNode.removeAll();
         for (i=0; i<num; i++) mPointNode.append (empty);
         i=0;
         break;

      case kChunkPointNodeName:
         res =iload->ReadWStringChunk (&readString);
         if (res != IO_OK) break;
         mPointNode[i] = TSTR(readString);
         i++;
      }
      iload->CloseChunk();
      if (res!=IO_OK) return res;
   }
   return IO_OK;
}

const USHORT kChunkVertexSelection = 0x200;
const USHORT kChunkEdgeSelection = 0x210;
const USHORT kChunkFaceSelection = 0x220;
const USHORT kChunkVertexHide = 0x224;
const USHORT kChunkFaceHide = 0x228;
const USHORT kChunkOperationID = 0x230;
const USHORT kChunkOperation = 0x234;
const USHORT kChunkOperationData = 0x238;
const USHORT kChunkMeshPaintHost = 0x240;
const USHORT kChunkCurrentOperationID = 0x260;
const USHORT kChunkCurrentOperationData = 0x268;

IOResult EditPolyMod::SaveLocalData(ISave *isave, LocalModData *ld) {
   ULONG nb;
   EditPolyData *pData = (EditPolyData*)ld;

   isave->BeginChunk(kChunkVertexSelection);
   pData->mVertSel.Save(isave);
   isave->EndChunk();

   isave->BeginChunk(kChunkEdgeSelection);
   pData->mEdgeSel.Save(isave);
   isave->EndChunk();

   isave->BeginChunk(kChunkFaceSelection);
   pData->mFaceSel.Save(isave);
   isave->EndChunk();

   isave->BeginChunk (kChunkVertexHide);
   pData->mVertHide.Save (isave);
   isave->EndChunk ();

   isave->BeginChunk (kChunkFaceHide);
   pData->mFaceHide.Save (isave);
   isave->EndChunk ();

   isave->BeginChunk (kChunkMeshPaintHost);
   ((MeshPaintHost *)pData)->Save (isave);   // calls MeshPaintHost implementation.
   isave->EndChunk ();

   // Save the operations - in order!  (Important for reconstructing later.)
   for (PolyOperationRecord *pop = pData->mpOpList; pop != NULL; pop=pop->Next())
   {
      int id = pop->Operation()->OpID ();
      isave->BeginChunk (kChunkOperationID);
      isave->Write (&id, sizeof(int), &nb);
      isave->EndChunk ();

      isave->BeginChunk (kChunkOperation);
      pop->Operation()->Save (isave);
      isave->EndChunk ();

      if (pop->LocalData())
      {
         isave->BeginChunk (kChunkOperationData);
         pop->LocalData()->Save (isave);
         isave->EndChunk ();
      }
   }

   LocalPolyOpData *currentOpData = pData->GetPolyOpData();
   if (currentOpData) {
      isave->BeginChunk (kChunkCurrentOperationID);
      int id = currentOpData->OpID();
      isave->Write (&id, sizeof(int), &nb);
      isave->EndChunk ();

      isave->BeginChunk (kChunkCurrentOperationData);
      currentOpData->Save (isave);
      isave->EndChunk ();
   }

   return IO_OK;
}

IOResult EditPolyMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
   EditPolyData *pData = new EditPolyData;
   *pld = pData;
   IOResult res(IO_OK);
   int id(0);
   ULONG nb(0);
   PolyOperation *currentOp=NULL;
   PolyOperationRecord *currentRecord=NULL, *lastRecord=NULL;
   LocalPolyOpData *currentData=NULL;

   while (IO_OK==(res=iload->OpenChunk())) {
      switch(iload->CurChunkID())  {
      case kChunkVertexSelection:
         res = pData->mVertSel.Load(iload);
         break;
      case kChunkEdgeSelection:
         res = pData->mEdgeSel.Load(iload);
         break;
      case kChunkFaceSelection:
         res = pData->mFaceSel.Load(iload);
         break;
      case kChunkVertexHide:
         res = pData->mVertHide.Load (iload);
         break;

      case kChunkFaceHide:
         res = pData->mFaceHide.Load (iload);
         break;

      case kChunkMeshPaintHost:
         res = ((MeshPaintHost *)pData)->Load (iload);   // calls MeshPaintHost implementation.
         break;
      case kChunkOperationID:
         res = iload->Read (&id, sizeof(int), &nb);
         currentOp = GetPolyOperationByID(id)->Clone ();
         currentRecord = new PolyOperationRecord (currentOp);
         break;
      case kChunkOperation:
         res = currentOp->Load (iload);
         if (res != IO_OK) break;
         if (lastRecord == NULL) pData->mpOpList = currentRecord;
         else lastRecord->SetNext (currentRecord);
         lastRecord = currentRecord;
         break;
      case kChunkOperationData:
         // id should still be current...
         currentData = pData->CreateLocalPolyOpData(id);
         if (!currentData) break;
         res = currentData->Load (iload);
         if (res == IO_OK) currentRecord->SetLocalData (currentData);
         else currentData->DeleteThis ();
         currentData = NULL;
         break;

      case kChunkCurrentOperationID:
         res = iload->Read (&id, sizeof(int), &nb);
         break;

      case kChunkCurrentOperationData:
         // id should still be current...
         pData->SetPolyOpData (id);
         if (!pData->GetPolyOpData()) break;
         res = pData->GetPolyOpData()->Load (iload);
         if (res != IO_OK) pData->ClearPolyOpData();
         break;
      }
      iload->CloseChunk();
      if (res!=IO_OK) return res;
   }
   return IO_OK;
}

void EditPolyMod::InvalidateDistanceCache () {
   if (ip==NULL) return;

   ModContextList list;
   INodeTab nodes;
   ip->GetModContexts(list,nodes);

   for (int i=0; i<list.Count(); i++) {
      EditPolyData *pData = (EditPolyData*)list[i]->localData;
      if (!pData) continue;
      pData->InvalidateDistances ();
   }
}

void EditPolyMod::InvalidateSoftSelectionCache () {
   if (ip==NULL) return;

   ModContextList list;
   INodeTab nodes;
   ip->GetModContexts(list,nodes);

   for (int i=0; i<list.Count(); i++) {
      EditPolyData *pData = (EditPolyData*)list[i]->localData;
      if (!pData) continue;
      pData->InvalidateSoftSelection ();
   }
   int softsel = mpParams->GetInt (epm_ss_use, TimeValue(0));
   if (softsel) NotifyDependents (FOREVER, PART_SELECT, REFMSG_CHANGE);
}

class RescalerEnumProc : public ModContextEnumProc {
   float mScale;
public:
   void SetScale (float scale) { mScale=scale; }
   BOOL proc(ModContext *mc);
};

BOOL RescalerEnumProc::proc (ModContext *mc) {
   EditPolyData *pData = (EditPolyData*)mc->localData;
   if (!pData) return true;
   pData->RescaleWorldUnits (mScale);
   pData->FreeCache ();
   return true;
}

static RescalerEnumProc theRescalerEnumProc;

void EditPolyMod::RescaleWorldUnits (float f) {
   theRescalerEnumProc.SetScale (f);
   EnumModContexts (&theRescalerEnumProc);
   EpModLocalDataChanged (PART_ALL);   // Could affect everything - whether welds occur, etc.
}

void EditPolyMod::ResetOperationParams () {
   // This method clears all animations and temporary parameters.

   if (mSliceMode) {
      slicePreviewMode = false;
      ExitSliceMode ();
   }

   // Clear the animations on all the operation parameters.
   for (int i=0; i<mpParams->NumParams (); i++)
   {
      if (mpParams->GetController (i) == NULL) continue;
      mpParams->RemoveController (i,0);
   }

   // Clear the Extrude Along Spline node:
   if (mpParams->GetINode (epm_extrude_spline_node)) {
      INode *nullNode = NULL;
      mpParams->SetValue (epm_extrude_spline_node, ip->GetTime(), nullNode);
   }

   // And these parameters should be reset to their default values:
   mpParams->Reset (epm_bridge_target_1);
   mpParams->Reset (epm_bridge_target_2);
   mpParams->Reset (epm_bridge_twist_1);
   mpParams->Reset (epm_bridge_twist_2);
   mpParams->Reset (epm_bridge_selected);

   if (mpSliceControl->IsAnimated()) {
      Matrix3 currentSlicePlane = EpGetSlicePlaneTM (ip->GetTime());
      ReplaceReference (EDIT_SLICEPLANE_REF, NewDefaultMatrix3Controller(), true);
      Point3 N = currentSlicePlane.GetRow (2);
      Point3 p = currentSlicePlane.GetTrans();

      SuspendAnimate();
      AnimateOff();
      SuspendSetKeyMode();

      EpSetSlicePlane (N, p, ip->GetTime());

      ResumeSetKeyMode();
      ResumeAnimate ();
   }

   if (mpParams->GetInt (epm_smooth_group_set))
      mpParams->SetValue (epm_smooth_group_set, TimeValue(0), 0);
   if (mpParams->GetInt (epm_smooth_group_clear))
      mpParams->SetValue (epm_smooth_group_clear, TimeValue(0), 0);

   // Clear all vertex animations:
   DeletePointControllers ();
}

class RemoveLocalPolyOpDataRestore : public RestoreObj {
private:
   EditPolyData *mpData;
   LocalPolyOpData *mpPolyOpData;
public:
   RemoveLocalPolyOpDataRestore (EditPolyData *pData) : mpData(pData) { mpPolyOpData = pData->GetPolyOpData(); }
   void Restore (int isUndo) { mpData->SetPolyOpData (mpPolyOpData); }
   void Redo () { mpData->RemovePolyOpData (); }
   int Size () { return 8; }
   TSTR Description() { return TSTR(_T("RemoveLocalPolyOpData")); }
};

void EditPolyMod::EpModCancel ()
{
   if (ip == NULL) return;

   PolyOperation *pOp = GetPolyOperation ();
   if (pOp == NULL) return;
   if (pOp->OpID() == ep_op_null) return;

   theHold.Begin ();

   // Cancel out of the paint deform mode, if applicable.
   if (pOp->OpID() == ep_op_paint_deform) {
      if( InPaintMode() ) EndPaintMode();
      if (IsPaintDataActive (PAINTMODE_DEFORM)) DeactivatePaintData( PAINTMODE_DEFORM, true);
      // (false for the last argument would generate a "Revert" we don't need.)
   }

   pOp = NULL; // must be done using this now.

   bool mre = macroRecorder->Enabled()!=0;
   if (mre) macroRecorder->Disable();
   mpParams->SetValue (epm_current_operation, TimeValue(0), ep_op_null);
   if (mre) macroRecorder->Enable ();

   ResetOperationParams ();

   // Clear all local operation data:
   ModContextList list;
   INodeTab nodes;
   ip->GetModContexts (list, nodes);
   for (int i=0; i<list.Count(); i++) {
      if (!list[i]->localData) continue;
      EditPolyData *pData = (EditPolyData *) list[i]->localData;
      if (!pData->GetPolyOpData()) continue;

      theHold.Put (new RemoveLocalPolyOpDataRestore (pData));
      pData->RemovePolyOpData ();
   }
   nodes.DisposeTemporary ();

   theHold.Accept (GetString (IDS_EDITPOLY_CANCEL_OPERATION));

   EpModLocalDataChanged (PART_TOPO|PART_GEOM|PART_SELECT|PART_VERTCOLOR|PART_TEXMAP);

   if (macroRecorder->Enabled())
   {
      macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].Cancel"), 0, 0);
      macroRecorder->EmitScript();
   }
}

void EditPolyMod::EpModCommitUnlessAnimating (TimeValue t)
{
   int animationMode = mpParams->GetInt (epm_animation_mode, t);
   PolyOperation *pOp = GetPolyOperation ();
   if (pOp && !pOp->CanAnimate()) animationMode = false;
   if (animationMode) return;
   EpModCommit (t, true);
}

void EditPolyMod::EpModCommit (TimeValue t)
{
   EpModCommit (t, true);
}

void EditPolyMod::UpdateTransformData (TimeValue t, Interval & valid, EditPolyData *pData) {
   if (!pData->GetPolyOpData()) return;
   if (pData->GetPolyOpData()->OpID() != ep_op_transform) return;
   if (mPointControl.Count() == 0) return;
   LocalTransformData *pTransform = (LocalTransformData*)pData->GetPolyOpData();

   int j;
   for (j=0; j<mPointNode.length(); j++) {
      if (mPointNode[j] == pTransform->GetNodeName()) break;
   }
   if (j==mPointNode.length()) return;

   int limit = mPointRange[j+1] - mPointRange[j];
   if (limit>pTransform->NumOffsets()) limit = pTransform->NumOffsets();
   for (int i=0; i<limit; i++) {
      int k = i+mPointRange[j];
      if (k>=mPointControl.Count()) break;
      if (mPointControl[k] == NULL) continue;
      mPointControl[k]->GetValue (t, pTransform->OffsetPointer(i), valid);
   }
}

void EditPolyMod::EpModCommit (TimeValue t, bool macroRecord, bool clearAfter, bool undoable)
{
   if (ip==NULL) return;

   int editType = mpParams->GetInt (epm_current_operation, t);
   if (editType == ep_op_null) return;

   PolyOperation *pOp = GetPolyOperation ();

   if (pOp == NULL)
   {
      mpParams->SetValue (epm_current_operation, t, ep_op_null);
      return;
   }

   UpdateCache (t);

   if (pOp->OpID() != ep_op_paint_deform) {
      if( InPaintMode() ) EndPaintMode();
      if (IsPaintDataActive(PAINTMODE_DEFORM)) DeactivatePaintData (PAINTMODE_DEFORM, true);
   }

   ModContextList list;
   INodeTab nodes;
   ip->GetModContexts (list, nodes);

   for (int i=0; i<list.Count(); i++)
   {
      EditPolyData *pData = (EditPolyData *)list[i]->localData;
      if (!pData || !pData->GetMesh()) continue;

      MNMesh & mesh = *(pData->GetMesh());

      // We need an instance of our op for each local mod data:
      PolyOperation *localOp = pOp->Clone ();
      localOp->GetValues (this, t, FOREVER); // Don't affect validity, since we're "freezing" these parameters.
      localOp->RecordSelection (mesh, this, pData);
      localOp->SetUserFlags (mesh);
      if (localOp->ModContextTM())
      {
         if (list[i]->tm) *(localOp->ModContextTM()) = Inverse(*(list[i]->tm));
         else localOp->ModContextTM()->IdentityMatrix();
      }
      localOp->GetNode (this, t, FOREVER);

      // If we're doing a transform and have point controllers,
      // update the local transform data.
      if (localOp->OpID() == ep_op_transform) UpdateTransformData (t, FOREVER, pData);

      bool ret = localOp->Apply (mesh, pData->GetPolyOpData());
      if (ret && pOp->CanDelete())
      {
         pData->CollapseDeadSelections (this, mesh);
         mesh.CollapseDeadStructs ();
      }

      if (ret && pOp->ChangesSelection())
      {
         BitArray sel;
         mesh.getVerticesByFlag (sel, MN_SEL);
         pData->SetVertSel (sel, this, t);
         mesh.getEdgesByFlag (sel, MN_SEL);
         pData->SetEdgeSel (sel, this, t);
         mesh.getFacesByFlag (sel, MN_SEL);
         pData->SetFaceSel (sel, this, t);
         /* 07 fev 2006 OC: Removed to fix bug #733566: Edit Poly ops unhide hidden faces.
		 mesh.getVerticesByFlag (sel, MN_HIDDEN);
         pData->SetVertHide (sel, this);
         mesh.getFacesByFlag (sel, MN_HIDDEN);
         pData->SetFaceHide (sel, this);*/

         if (pData->GetPaintSelCount()) {
            if (pData->GetPaintSelCount() != mesh.numv) pData->SetPaintSelCount (mesh.numv);
            float *softsel = pData->GetPaintSelValues();
            for (int j=0; j<mesh.numv; j++) if (mesh.v[j].GetFlag(MN_SEL)) softsel[j] = 1.0f;
         }
      }
      else
      {
         // Just make sure we have the right number of everything.
         pData->SynchBitArrays ();
      }

      if (undoable)
      {
         if (theHold.Holding ()) {
            theHold.Put (new AddOperationRestoreObj (this, pData));
         } else {
            DebugPrint ("Edit Poly WARNING: Operation being committed without restore object.\n");
            DbgAssert(0);
         }
      }
      pData->PushOperation (localOp);
   }
   nodes.DisposeTemporary();

   if (clearAfter)
   {
      // Clean up the paint deform data.
      if (pOp->OpID() == ep_op_paint_deform) {
         if( InPaintMode() ) EndPaintMode();
         if (IsPaintDataActive(PAINTMODE_DEFORM)) DeactivatePaintData (PAINTMODE_DEFORM, true);
      }

      pOp->Reset ();
	  pOp->HideGrip();
      pOp = NULL; // must be done using this now.

      bool mre = macroRecorder->Enabled()!=0;
      if (mre) macroRecorder->Disable();
      mpParams->SetValue (epm_current_operation, t, ep_op_null);
      if (mre) macroRecorder->Enable ();

      ResetOperationParams ();
   }

   EpModLocalDataChanged (PART_TOPO|PART_GEOM|PART_SELECT|PART_VERTCOLOR|PART_TEXMAP);

   if (macroRecord && macroRecorder->Enabled())
   {
      macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].Commit"), 0, 0);
      macroRecorder->EmitScript();
   }
}

void EditPolyMod::EpModCommitAndRepeat (TimeValue t)
{
   if (ip==NULL) return;

   int editType = mpParams->GetInt (epm_current_operation);
   if (editType == ep_op_null) return;

   PolyOperation *pOp = GetPolyOperation ();

   EpModCommit (t, false, false);

   if (macroRecorder->Enabled())
   {
      macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].CommitAndRepeat"), 0, 0);
      macroRecorder->EmitScript();
   }
}

int EditPolyMod::GetPolyOperationID ()
{
   return mpParams->GetInt (epm_current_operation);
}

PolyOperation *EditPolyMod::GetPolyOperation ()
{
   int operation = mpParams->GetInt (epm_current_operation);
   if ((mpCurrentOperation != NULL) && (mpCurrentOperation->OpID() == operation))
      return mpCurrentOperation;

   if (mpCurrentOperation != NULL)
   {
      mpCurrentOperation->DeleteThis ();
      mpCurrentOperation = NULL;
   }

   if (operation == ep_op_null) return NULL;

   PolyOperation *staticPolyOp = GetPolyOperationByID (operation);
   if (staticPolyOp == NULL) return NULL;
   mpCurrentOperation = staticPolyOp->Clone();
   return mpCurrentOperation;
}

PolyOperation *EditPolyMod::GetPolyOperationByID (int operationId)
{
   for (int i=0; i<mOpList.Count (); i++)
      if (operationId == mOpList[i]->OpID ()) return mOpList[i];
   return NULL;
}

void EditPolyMod::EpSetLastOperation (int op) {
   ICustButton *pButton = NULL;
   HWND hGeom = GetDlgHandle (ep_geom);
   if (hGeom) pButton = GetICustButton (GetDlgItem (hGeom, IDC_REPEAT_LAST));

   switch (op) {
   case ep_op_null:
   case ep_op_unhide_vertex:
   case ep_op_unhide_face:
   case ep_op_ns_copy:
   case ep_op_ns_paste:
   case ep_op_reset_plane:
   case ep_op_remove_iso_verts:
   case ep_op_remove_iso_map_verts:
   case ep_op_cut:
   case ep_op_toggle_shaded_faces:
   case ep_op_get_stack_selection:
      // Don't modify button or mLastOperation.
      break;
   case ep_op_hide_vertex:
   case ep_op_hide_face:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_HIDE));
      break;
   case ep_op_hide_unsel_vertex:
   case ep_op_hide_unsel_face:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_HIDE_UNSELECTED));
      break;
   case ep_op_cap:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_CAP));
      break;
   case ep_op_delete_vertex:
   case ep_op_delete_face:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_DELETE));
      break;
   case ep_op_remove_vertex:
   case ep_op_remove_edge:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_REMOVE));
      break;
   case ep_op_detach_vertex:
   case ep_op_detach_face:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_DETACH));
      break;
   case ep_op_split:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_SPLIT));
      break;
   case ep_op_break:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_BREAK));
      break;
   case ep_op_collapse_vertex:
   case ep_op_collapse_edge:
   case ep_op_collapse_face:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_COLLAPSE));
      break;
   case ep_op_weld_vertex:
   case ep_op_weld_edge:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_WELD_SEL));
      break;
   case ep_op_slice:
   case ep_op_slice_face:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_SLICE));
      break;
   case ep_op_create_shape:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_CREATE_SHAPE_FROM_EDGES));
      break;
   case ep_op_make_planar:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_MAKE_PLANAR));
      break;
   case ep_op_meshsmooth:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_MESHSMOOTH));
      break;
   case ep_op_tessellate:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_TESSELLATE));
      break;
   case ep_op_flip_face:
   case ep_op_flip_element:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_FLIP_NORMALS));
      break;
   case ep_op_retriangulate:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_RETRIANGULATE));
      break;
   case ep_op_autosmooth:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_AUTOSMOOTH));
      break;
   case ep_op_extrude_vertex:
   case ep_op_extrude_edge:
   case ep_op_extrude_face:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_EXTRUDE));
      break;
   case ep_op_bevel:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_BEVEL));
      break;
   case ep_op_chamfer_vertex:
   case ep_op_chamfer_edge:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_CHAMFER));
      break;
   case ep_op_inset:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_INSET));
      break;
   case ep_op_outline:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_OUTLINE));
      break;
   case ep_op_hinge_from_edge:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_HINGE_FROM_EDGE));
      break;
   case ep_op_connect_edge:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_CONNECT_EDGES));
      break;
   case ep_op_connect_vertex:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_CONNECT_VERTICES));
      break;
   case ep_op_extrude_along_spline:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_EXTRUDE_ALONG_SPLINE));
      break;
   case ep_op_sel_grow:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_SELECTION_GROW));
      break;
   case ep_op_sel_shrink:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_SELECTION_SHRINK));
      break;
   case ep_op_select_loop:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_SELECT_EDGE_LOOP));
      break;
   case ep_op_select_ring:
      mLastOperation = op;
      if (pButton) pButton->SetTooltip (true, GetString (IDS_SELECT_EDGE_RING));
      break;
   }
   if (pButton) ReleaseICustButton (pButton);
}

// For Maxscript access:
// Methods to get information about our mesh.

int EditPolyMod::EpMeshGetNumVertices (INode *node)
{
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   return mesh->numv;
}

Point3 EditPolyMod::EpMeshGetVertex (int vertIndex, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return Point3(0,0,0);
   if ((vertIndex<0) || (vertIndex>=mesh->numv)) return Point3(0,0,0);
   Point3 currentPoint = mesh->v[vertIndex].p;
   //going to need a valid node
   INode * curNode = NULL;
   //if nothing was passed in, get the first selected node from the context list
   if ( node == NULL )
   {
      curNode = getSelectedNodeFromContextList();
      if ( curNode == NULL )
      {
         return Point3( 0, 0, 0 );        //not sure if we will ever get in here
      }
   }
   else
   {
      //use the node passed in
      curNode = node;
   }
   convertPointToCurrentCoordSys( currentPoint, curNode );
   return currentPoint;
}

int EditPolyMod::EpMeshGetVertexFaceCount (int vertIndex, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((vertIndex<0) || (vertIndex>=mesh->numv) || !mesh->vfac) return -1;
   return mesh->vfac[vertIndex].Count();
}

int EditPolyMod::EpMeshGetVertexFace (int vertIndex, int whichFace, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((vertIndex<0) || (vertIndex>=mesh->numv) || !mesh->vfac) return -1;
   if ((whichFace<0) || (whichFace>=mesh->vfac[vertIndex].Count())) return -1;
   return mesh->vfac[vertIndex][whichFace];
}

int EditPolyMod::EpMeshGetVertexEdgeCount (int vertIndex, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((vertIndex<0) || (vertIndex>=mesh->numv) || !mesh->vedg) return 0;
   return mesh->vedg[vertIndex].Count();
}

int EditPolyMod::EpMeshGetVertexEdge (int vertIndex, int whichEdge, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((vertIndex<0) || (vertIndex>=mesh->numv) || !mesh->vedg) return -1;
   if ((whichEdge<0) || (whichEdge>=mesh->vedg[vertIndex].Count())) return -1;
   return mesh->vedg[vertIndex][whichEdge];
}

int EditPolyMod::EpMeshGetNumEdges (INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   return mesh->nume;
}

int EditPolyMod::EpMeshGetEdgeVertex (int edgeIndex, int end, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((edgeIndex<0) || (edgeIndex>=mesh->nume)) return -1;
   if ((end<0) || (end>1)) return -1;
   return mesh->e[edgeIndex][end];
}

int EditPolyMod::EpMeshGetEdgeFace (int edgeIndex, int side, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((edgeIndex<0) || (edgeIndex>=mesh->nume)) return -1;
   if ((side<0) || (side>1)) return -1;
   return side ? mesh->e[edgeIndex].f2 : mesh->e[edgeIndex].f1;
}

int EditPolyMod::EpMeshGetNumFaces(INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   return mesh->numf;
}

int EditPolyMod::EpMeshGetFaceDegree (int faceIndex, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((faceIndex<0) || (faceIndex>=mesh->numf)) return 0;
   return mesh->f[faceIndex].deg;
}

int EditPolyMod::EpMeshGetFaceVertex (int faceIndex, int corner, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((faceIndex<0) || (faceIndex>=mesh->numf)) return -1;
   if ((corner<0) || (corner>=mesh->f[faceIndex].deg)) return -1;
   return mesh->f[faceIndex].vtx[corner];
}

int EditPolyMod::EpMeshGetFaceEdge (int faceIndex, int side, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((faceIndex<0) || (faceIndex>=mesh->numf)) return -1;
   if ((side<0) || (side>=mesh->f[faceIndex].deg)) return -1;
   return mesh->f[faceIndex].edg[side];
}

int EditPolyMod::EpMeshGetFaceDiagonal (int faceIndex, int diagonal, int end, INode *node) {
   MNMesh *mesh = EpModGetMesh(node);
   if (!mesh) return 0;
   if ((faceIndex<0) || (faceIndex>=mesh->numf)) return -1;
   if ((diagonal<0) || (diagonal>=mesh->f[faceIndex].deg-2)) return -1;
   if ((end<0) || (end>1)) return -1;
   return mesh->f[faceIndex].diag[diagonal*2+end];
}

int EditPolyMod::EpMeshGetFaceMaterial (int faceIndex, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((faceIndex<0) || (faceIndex>=mesh->numf)) return 0;
   return mesh->f[faceIndex].material;
}

DWORD EditPolyMod::EpMeshGetFaceSmoothingGroup (int faceIndex, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if ((faceIndex<0) || (faceIndex>=mesh->numf)) return 0;
   return mesh->f[faceIndex].smGroup;
}

int EditPolyMod::EpMeshGetNumMapChannels (INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   return mesh->numm;
}

bool EditPolyMod::EpMeshGetMapChannelActive (int mapChannel, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return false;
   if ((mapChannel <= -NUM_HIDDENMAPS) || (mapChannel >= mesh->numm)) return false;
   return mesh->M(mapChannel)->GetFlag (MN_DEAD) ? false : true;
}

int EditPolyMod::EpMeshGetNumMapVertices (int mapChannel, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if (!EpMeshGetMapChannelActive(mapChannel)) return 0;
   return mesh->M(mapChannel)->numv;
}

UVVert EditPolyMod::EpMeshGetMapVertex (int mapChannel, int vertIndex, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return UVVert(0,0,0);
   if (!EpMeshGetMapChannelActive(mapChannel)) return UVVert(0,0,0);
   if ((vertIndex < 0) || (vertIndex >= mesh->M(mapChannel)->numv)) return UVVert(0,0,0);
   return mesh->M(mapChannel)->v[vertIndex];
}

int EditPolyMod::EpMeshGetMapFaceVertex (int mapChannel, int faceIndex, int corner, INode *node) {
   MNMesh *mesh = EpModGetMesh (node);
   if (!mesh) return 0;
   if (!EpMeshGetMapChannelActive(mapChannel)) return -1;
   if ((faceIndex<0) || (faceIndex>=mesh->numf)) return -1;
   if ((corner<0) || (corner>=mesh->f[faceIndex].deg)) return -1;
   return mesh->M(mapChannel)->f[faceIndex].tv[corner];
}

//-----------------------------------------------------------------
//Retrieve the normal of the specified face
Point3 EditPolyMod::EPMeshGetFaceNormal( const int in_faceIndex, INode *in_node )
{
   //Get mesh and perform validation checks
   MNMesh *mesh = EpModGetMesh (in_node);
   if ( mesh == NULL )
   {
      return Point3(0,0,0);
   }
   if ( ( in_faceIndex < 0 ) || ( in_faceIndex >= mesh->numf ) )
   {
      return Point3(0,0,0);
   }
   return mesh->GetFaceNormal( in_faceIndex, true ); // LOCAL COORDINATES!!!
}

//Retrieve the center coordintes of the specified face
Point3 EditPolyMod::EPMeshGetFaceCenter( const int in_faceIndex, INode *in_node )
{
   //Get mesh and perform validation checks
   MNMesh *mesh = EpModGetMesh (in_node);
   if ( mesh == NULL )
   {
      return Point3(0,0,0);
   }
   if ( ( in_faceIndex < 0 ) || ( in_faceIndex >= mesh->numf ) )
   {
      return Point3(0,0,0);
   }
   //Computer center of the face and try to convert it to the current reference coordinate system
   Point3 faceCenter;
   mesh->ComputeCenter( in_faceIndex, faceCenter );   //LOCAL COORDINATES
   //going to need a valid node
   INode * curNode = NULL;
   //if nothing was passed in, get the first selected node from the context list
   if ( in_node == NULL )
   {
      curNode = getSelectedNodeFromContextList();
      if ( curNode == NULL )
      {
         return Point3( 0, 0, 0 );        //not sure if we will ever get in here
      }
   }
   else
   {
      //use the node passed in
      curNode = in_node;
   }
   convertPointToCurrentCoordSys( faceCenter, curNode );
   return faceCenter;
}

//Get the area of the face specified
float EditPolyMod::EPMeshGetFaceArea( const int in_faceIndex, INode *in_node )
{
   //Get the mesh and validate information
   MNMesh *mesh = EpModGetMesh (in_node);
   if ( mesh == NULL )
   {
      return 0.0f;
   }
   if ( ( in_faceIndex < 0 ) || ( in_faceIndex >= mesh->numf ) )
   {
      return 0.0f;
   }
   //Calculate area based on the face normal - not 100% accurate, but still very close
   //Code taken from polyop.cpp
   return (float)( Length( mesh->GetFaceNormal( in_faceIndex ) ) / 2.0f );
}

//Retrieve the list of edges that are touching less than two faces
BitArray * EditPolyMod::EPMeshGetOpenEdges( INode *in_node )
{
   //Get the mesh and validate information
   static BitArray staticOpenEdges;
   MNMesh *mesh = EpModGetMesh (in_node);
   if ( mesh == NULL )
   {
      return NULL;
   }
   if ( !mesh->GetFlag(MN_MESH_FILLED_IN) )
   {
      return NULL;
   }

   int nEdges = mesh->ENum();
   staticOpenEdges.SetSize( nEdges );
   staticOpenEdges.ClearAll();
   //loop through all of the edges, and check if either of the faces is empty
   for ( int i = 0; i < nEdges; i++ )
   {
      MNEdge* e = mesh->E(i);
      if ( !( e->GetFlag(MN_DEAD) ) && ( ( e->f1 == -1 ) || ( e->f2 == -1 ) ) )
      {
         //set the bitarray to indicate that the edge is next to an open face
         staticOpenEdges.Set(i);
      }
   }
   return &staticOpenEdges;
}

//Retrieve all vertices that have the specified flag - return the results in the vset BitArray
bool EditPolyMod::EPMeshGetVertsByFlag( BitArray & out_vset, const DWORD in_flags, const DWORD in_fmask, INode * in_node )
{
   //Get the mesh
   MNMesh *mesh = EpModGetMesh (in_node);
   if ( mesh == NULL )
   {
      return false;
   }
   //Pass call onto the mesh - the call will update the referenced bitarray
   return mesh->getVerticesByFlag( out_vset, in_flags, in_fmask );
}

//Retrieve all edges that have the specified flag - return the results in the eset BitArray
bool EditPolyMod::EPMeshGetEdgesByFlag( BitArray & out_eset, const DWORD in_flags, const DWORD in_fmask, INode * in_node )
{
   //Get the mesh
   MNMesh *mesh = EpModGetMesh (in_node);
   if ( mesh == NULL )
   {
      return false;
   }
   //Pass call onto the mesh - the call will update the referenced bitarray
   return mesh->getEdgesByFlag( out_eset, in_flags, in_fmask );
}

//Retrieve all faces that have the specified flag - return the results in the fset BitArray
bool EditPolyMod::EPMeshGetFacesByFlag( BitArray & out_fset, const DWORD in_flags, const DWORD in_fmask, INode * in_node )
{
   //Get the mesh
   MNMesh *mesh = EpModGetMesh (in_node);
   if ( mesh == NULL )
   {
      return false;
   }
   //Pass call onto the mesh - the call will update the referenced bitarray
   return mesh->getFacesByFlag( out_fset, in_flags, in_fmask );
}

//Set the flags of the vertices specified in the vset BitArray to the flags passed in
void EditPolyMod::EPMeshSetVertexFlags( BitArray & in_vset, const DWORD in_flags, DWORD in_fmask, const bool in_undoable, INode * in_node )
{
   //Need the EditPolyData to allow the user to undo the set
   EditPolyData * pData = GetEditPolyDataForNode(in_node);
   if ( pData == NULL )
   {
      return;
   }
   //Get the mesh
   MNMesh * mesh = pData->GetMesh();
   if ( mesh == NULL )
   {
      return;
   }
   //selected subobjects about to change
   if ( (in_flags & MN_SEL) && ip )
   {
      ip->ClearCurNamedSelSet();
   }
   //If the operation is undoable, add the undo call to the stack of undo methods
   if ( in_undoable && theHold.Holding() )
   {
      theHold.Put ( new EditPolyFlagRestore( this, pData, MNM_SL_VERTEX ) );
   }

   //Get the new mask
   in_fmask = in_fmask | in_flags;
   //Get the max between the number of vertices and the bitarray size
   //since the it's possible that the bitarray is not big enough
   int max = (mesh->numv > in_vset.GetSize()) ? in_vset.GetSize() : mesh->numv;
   //loop through the bitarray
   for ( int i = 0; i < max; i++ )
   {
      //ignore dead vertices
      if ( mesh->v[i].GetFlag(MN_DEAD) )
      {
         continue;
      }
      //ignore the vertex if the bit array contains 0
      if ( in_vset[i] == 0 )
      {
         continue;
      }
      //otherwise, clear the flags using the mask, and assign the new value
      mesh->v[i].ClearFlag(in_fmask);
      mesh->v[i].SetFlag(in_flags);
   }
   //if the new flags indicate hidden or selected, propagate the changes, and ensure that the changes are reflected
   if ( in_flags & (MN_HIDDEN|MN_SEL) )
   {
      mesh->PropegateComponentFlags (MNM_SL_VERTEX, MN_SEL, MNM_SL_VERTEX, MN_HIDDEN, FALSE, FALSE);
      DWORD parts = PART_DISPLAY|PART_SELECT;
      if ( in_flags & MN_HIDDEN )
      {
         parts |= PART_GEOM;  // for bounding box change
      }
      EpModLocalDataChanged (parts);
   }
}

//Set the flags of the edges specified in the eset BitArray to the flags passed in
void EditPolyMod::EPMeshSetEdgeFlags( BitArray & in_eset, const DWORD in_flags, DWORD in_fmask, const bool in_undoable, INode * in_node )
{
   //Get the EditPolyData - required for undo
   EditPolyData * pData = GetEditPolyDataForNode(in_node);
   if ( pData == NULL )
   {
      return;
   }
   //Get the mesh
   MNMesh * mesh = pData->GetMesh();
   if ( mesh == NULL )
   {
      return;
   }
   //If the the selected flag is passed, cleared the current selection set
   if ( (in_flags & MN_SEL) && ip )
   {
      ip->ClearCurNamedSelSet();
   }
   //If undoable, add the undo class to the stack to allow undo later on
   if ( in_undoable && theHold.Holding() )
   {
      theHold.Put ( new EditPolyFlagRestore( this, pData, MNM_SL_EDGE ) );
   }

   //calculate the mask
   in_fmask = in_fmask | in_flags;
   //Get the max of the number of edges and the size of the bitarray
   //since the bitarray could be smaller than the number of edges
   int max = ( mesh->nume > in_eset.GetSize() ) ? in_eset.GetSize() : mesh->nume;
   //Loop through the edges
   for ( int i = 0; i < max; i++ )
   {
      //ignore dead edges
      if ( mesh->e[i].GetFlag(MN_DEAD) )
      {
         continue;
      }
      //ignore edges of the bitarray that are 0
      if ( in_eset[i] == 0 )
      {
         continue;
      }
      //otherwise, clear the flag using the mask, and set the new one
      mesh->e[i].ClearFlag(in_fmask);
      mesh->e[i].SetFlag(in_flags);
   }
   //if the selection has changed, update the local data to reflect the change
   if (in_flags & MN_SEL)
   {
      EpModLocalDataChanged (PART_SELECT|PART_DISPLAY);
   }
}

//Set the flags of the faces specified in the fset BitArray to the flags passed in
void EditPolyMod::EPMeshSetFaceFlags( BitArray & in_fset, const DWORD in_flags, DWORD in_fmask, const bool in_undoable, INode * in_node )
{
   //Get the EditPolyData for undo
   EditPolyData * pData = GetEditPolyDataForNode( in_node );
   if ( pData == NULL )
   {
      return;
   }
   //Get the mesh
   MNMesh * mesh = pData->GetMesh();
   if ( mesh == NULL )
   {
      return;
   }

   //If the selection is about to changed, clear the existing selection
   if ( (in_flags & MN_SEL) && ip )
   {
      ip->ClearCurNamedSelSet();
   }
   //if undoable, add the undo class to the stack to allow undo to work
   if ( in_undoable && theHold.Holding() )
   {
      theHold.Put ( new EditPolyFlagRestore( this, pData, MNM_SL_FACE ) );
   }

   //get the mask
   in_fmask = in_fmask | in_flags;
   //get the max of the number of faces and the bitarray size
   //since the bitarray could be smaller than the number of edges
   int max = ( mesh->numf > in_fset.GetSize() ) ? in_fset.GetSize() : mesh->numf;
   //loop through the faces
   for ( int i = 0; i < max; i++ )
   {
      //ignore dead faces
      if ( mesh->f[i].GetFlag(MN_DEAD) )
      {
         continue;
      }
      //ignore faces in bitarray that are set to 0
      if ( in_fset[i] == 0 )
      {
         continue;
      }
      //otherwise, clear the flag using the mask, and set the new flag
      mesh->f[i].ClearFlag(in_fmask);
      mesh->f[i].SetFlag(in_flags);
   }
   //if hidden or selected passed in, propagate the flags, and update the local data to reflect the changes
   if ( in_flags & (MN_HIDDEN|MN_SEL) )
   {
      mesh->PropegateComponentFlags (MNM_SL_FACE, MN_SEL, MNM_SL_FACE, MN_HIDDEN, FALSE, FALSE);
      DWORD parts = PART_DISPLAY|PART_SELECT;
      if ( in_flags & MN_HIDDEN )
      {
         parts |= PART_GEOM;  // for bounding box change
      }
      EpModLocalDataChanged(parts);
   }
}

//Get the flags of the specified vertex
int EditPolyMod::EPMeshGetVertexFlags( const int in_vertexIndex, INode * in_node )
{
   //Get the mesh and validate
   MNMesh *mesh = EpModGetMesh (in_node);
   if ( mesh == NULL )
   {
      return 0;
   }
   if ( ( in_vertexIndex < 0 ) || ( in_vertexIndex >= mesh->numv ) )
   {
      return 0;
   }
   //retrieve the vertex flags
   return mesh->V( in_vertexIndex )->ExportFlags();
}

//Get the flags of the specified edge
int EditPolyMod::EPMeshGetEdgeFlags( const int in_edgeIndex, INode * in_node )
{
   //Get the mesh and validate
   MNMesh *mesh = EpModGetMesh (in_node);
   if ( mesh == NULL )
   {
      return 0;
   }
   if ( ( in_edgeIndex < 0 ) || ( in_edgeIndex >= mesh->nume ) )
   {
      return 0;
   }
   //retrive the edge flags
   return mesh->E( in_edgeIndex )->ExportFlags();
}

//Get the flags of the specified face
int EditPolyMod::EPMeshGetFaceFlags( const int in_faceIndex, INode * in_node )
{
   //Get the mesh and validate
   MNMesh *mesh = EpModGetMesh (in_node);
   if ( mesh == NULL )
   {
      return 0;
   }
   if ( ( in_faceIndex < 0 ) || ( in_faceIndex >= mesh->numf ) )
   {
      return 0;
   }
   //retrieve the face flags
   return mesh->F( in_faceIndex )->ExportFlags();
}

//Get all of the vertices that are used by any of the edges specified in eset - return results in vset BitArray
void EditPolyMod::EPMeshGetVertsUsingEdge( BitArray & out_vset, const BitArray & in_eset, INode * in_node )
{
   //Get the mesh
   MNMesh * mesh = EpModGetMesh( in_node );
   if ( mesh == NULL )
   {
      return;
   }
   if ( !( mesh->GetFlag( MN_MESH_FILLED_IN ) ) )
   {
      return;
   }

   //Ensure that the vset bitarray is the correct size
   int nEdges = mesh->ENum();
   int max = ( nEdges > in_eset.GetSize() ) ? in_eset.GetSize() : nEdges;
   int nVerts = mesh->VNum();
   out_vset.SetSize( nVerts );
   out_vset.ClearAll();

   //loop through the edges
   for ( int i = 0; i < max; i++ )
   {
      //check if the user specified this edge to check for
      if ( in_eset[i] != 0 )
      {
         //ignore dead edges
         MNEdge * e = mesh->E(i);
         if ( e->GetFlag( MN_DEAD ) )
         {
            continue;
         }
         //otherwise set the vertices of the edge
         out_vset.Set( e->v1 );
         out_vset.Set( e->v2 );
      }
   }
}

//Get all of the edges that are used by any of the verices specified in vset - return results in eset BitArray
void EditPolyMod::EPMeshGetEdgesUsingVert( BitArray & out_eset, BitArray & in_vset, INode * in_node )
{
   //Get the mesh
   MNMesh * mesh = EpModGetMesh( in_node );
   if ( mesh == NULL )
   {
      return;
   }
   if ( !( mesh->GetFlag( MN_MESH_FILLED_IN ) ) )
   {
      return;
   }

   //Ensure that the edge and vertex bitarrays are the correct size
   int nEdges = mesh->ENum();
   out_eset.SetSize( nEdges );
   out_eset.ClearAll();
   int nVerts = mesh->VNum();
   in_vset.SetSize( nVerts, true );

   //loop throug the edges
   for ( int i = 0; i < nEdges; i++ )
   {
      //ignore dead edges
      MNEdge * e = mesh->E(i);
      if ( e->GetFlag( MN_DEAD ) )
      {
         continue;
      }
      //if edge contains a vertex specified in the bitarray passed in, set the edge bit
      if ( ( in_vset[e->v1] != 0 ) || ( in_vset[e->v2] != 0 ) )
      {
         out_eset.Set(i);
      }
   }
}

//Get all of the faces that are used by any of the edges specified in eset - return results in fset BitArray
void EditPolyMod::EPMeshGetFacesUsingEdge( BitArray & out_fset, BitArray & in_eset, INode * in_node )
{
   //Get the mesh
   MNMesh * mesh = EpModGetMesh( in_node );
   if ( mesh == NULL )
   {
      return;
   }
   if ( !( mesh->GetFlag( MN_MESH_FILLED_IN ) ) )
   {
      return;
   }

   //Ensure that the face and edge bitarrays are the right size
   int nFaces = mesh->FNum();
   out_fset.SetSize( nFaces );
   out_fset.ClearAll();
   int nEdges = mesh->ENum();
   in_eset.SetSize( nEdges, true );

   //loop through the faces
   for ( int i = 0; i < nFaces; i++ )
   {
      //ignore dead ffaces
      MNFace * f = mesh->F(i);
      if ( f->GetFlag( MN_DEAD ) )
      {
         continue;
      }
      //loop through the edges of the face
      for ( int j = 0; j < f->deg; j++ )
      {
         //if the face contains any of the edges specified in eset, add the face to the list
         if ( in_eset[ f->edg[j] ] != 0 )
         {
            out_fset.Set(i);
            break;
         }
      }
   }
}

//Get all of the elements that are used by any of the faces specified in fset - return results in eset BitArray
void EditPolyMod::EPMeshGetElementsUsingFace( BitArray & out_eset, BitArray & in_fset, BitArray & in_fenceSet, INode * in_node )
{
   //Get the mesh
   MNMesh * mesh = EpModGetMesh( in_node );
   if ( mesh == NULL )
   {
      return;
   }
   if ( !( mesh->GetFlag( MN_MESH_FILLED_IN ) ) )
   {
      return;
   }

   //ensure the edge, face, and fence bitarrays are the right size
   int nFaces = mesh->FNum();
   out_eset.SetSize( nFaces );
   out_eset.ClearAll();
   in_fset.SetSize( nFaces, true );
   in_fenceSet.SetSize( nFaces, true );

   //another bitarray for calculating the elements from the face
   BitArray fromThisFace(nFaces);

   //loop through the faces
   for ( int i = 0; i < nFaces; i++ )
   {
      //check if this face was specified by the user
      if ( in_fset[i] != 0 )
      {
         //Ignore daed faces
         if ( mesh->F(i)->GetFlag( MN_DEAD ) )
         {
            continue;
         }
         //copy the fence set to the temporary bitarray
         fromThisFace = in_fenceSet;
         //calculate the elements from this specific face given the fence set
         mesh->ElementFromFace( i, fromThisFace );
         //add the elements found to the element set
         out_eset |= fromThisFace;
      }
   }
}

//Get all of the faces that are used by any of the vertices specified in vset - return results in fset BitArray
void EditPolyMod::EPMeshGetFacesUsingVert( BitArray & out_fset, BitArray & in_vset, INode * in_node )
{
   //Get the mesh
   MNMesh * mesh = EpModGetMesh( in_node );
   if ( mesh == NULL )
   {
      return;
   }

   //ensure that the face and vertex bitarrays are the right size
   int nFaces = mesh->FNum();
   out_fset.SetSize( nFaces );
   out_fset.ClearAll();
   int nVerts = mesh->VNum();
   in_vset.SetSize( nVerts, true );

   //loop through the faces
   for ( int i = 0; i < nFaces; i++ )
   {
      //ignore dead faces
      MNFace * f = mesh->F(i);
      if ( f->GetFlag( MN_DEAD ) )
      {
         continue;
      }
      //loop through the vertices of the face
      for ( int j = 0; j < f->deg; j++ )
      {
         //if the face contains any of the vertices specified by the user, add the face to the set
         if ( in_vset[ f->vtx[j] ] != 0 )
         {
            out_fset.Set(i);
            break;
         }
      }
   }
}

//Get all of the vertices that are used by any of the faces specified in fset - return results in vset BitArray
void EditPolyMod::EPMeshGetVertsUsingFace( BitArray & out_vset, BitArray & in_fset, INode * in_node )
{
   //Get the mesh
   MNMesh * mesh = EpModGetMesh( in_node );
   if ( mesh == NULL )
   {
      return;
   }

   //ensure the vertex and face bitarrays are the right size
   int nVerts = mesh->VNum();
   int nFaces = mesh->FNum();
   out_vset.SetSize( nVerts );
   out_vset.ClearAll();
   in_fset.SetSize( nFaces, true );

   //loop through the faces
   for ( int i = 0; i < nFaces; i++ )
   {
      //check if the face was specified by the user
      if ( in_fset[i] != 0 )
      {
         //ignore dead faces
         MNFace * f = mesh->F(i);
         if ( f->GetFlag( MN_DEAD ) )
         {
            continue;
         }
         //add all of the vertices of the face to the vertex set
         for ( int j = 0; j < f->deg; j++ )
         {
            out_vset.Set( f->vtx[j] );
         }
      }
   }
}
void EditPolyMod::EPMeshSetVert( const BitArray & in_vset, const Point3 & in_point, INode * in_node )
{
   EditPolyData * pData = GetEditPolyDataForNode( in_node );
   if ( pData == NULL )
   {
      return;
   }
   //Get the mesh - have to use the mesh output instead of the normal mesh unless we want to commit every time
   MNMesh * mesh = NULL;
   mesh = pData->GetMeshOutput();
   if ( mesh == NULL )
   {
      mesh = pData->GetMesh();
      if ( mesh == NULL )
      {
         return;
      }
   }

   //going to need a valid node
   INode * curNode = NULL;
   //if nothing was passed in, get the first selected node from the context list
   if ( in_node == NULL )
   {
      curNode = getSelectedNodeFromContextList();
      if ( curNode == NULL )
      {
         return;        //not sure if we will ever get in here
      }
   }
   else
   {
      //use the node passed in
      curNode = in_node;
   }

   int nVerts = mesh->numv;
   Tab<Point3> delta;
   delta.SetCount( nVerts );
   Point3 localCoords = in_point;
   //Convert the point passed in from whatever the current system is into the object's local coordinate system
   //Not entirely sure whether we want this.  The polyOp version always seems to use world coordinates even though it has
   //a conversion function.  Does the "thread_local(current_coordsys)" that it uses always returns a default coordsys?
   convertPointFromCurrentCoordSys( localCoords, curNode );

   //loop through the list of specified vertices and find the offset required to move them to the point specified
   int max = ( nVerts > in_vset.GetSize() ) ? in_vset.GetSize() : nVerts;
   for ( int i = 0; i < max; i++ )
   {
      if ( in_vset[i] != 0 )
      {
         delta[i] = localCoords - mesh->V(i)->p;
      }
      else
      {
         delta[i] = Point3( 0, 0, 0 );
      }
   }
   //Null out the rest of the points
   for ( int i = max; i < nVerts; i++ )
   {
      delta[i] = Point3( 0, 0, 0 );
   }
   //Going to do a transform since most of the code is already set up to handle this type of operation
   EpModSetOperation (ep_op_transform);
   
   TimeValue t = GetCOREInterface()->GetTime();

   //Now use drag - since this is exactly what we want to do
   EpModDragInit( curNode, t );
   EpModDragMove( delta, curNode, t);
   EpModDragAccept( curNode, t );
}

//find the first selected node from the modifier context list
INode * EditPolyMod::getSelectedNodeFromContextList()
{
   if ( ip == NULL )
   {
      return NULL;
   }

   ModContextList mcList;
   INodeTab nodes;
   INode * curNode = NULL;
   
   ip->GetModContexts(mcList,nodes);
   if ( nodes.Count() == 0 )
   {
      nodes.DisposeTemporary();
      return NULL;
   }
   //If no node is passed in, then assuming we're using the selected node
   for ( int i = 0; i < nodes.Count(); i++ )
   {
      if ( nodes[i]->Selected() )
      {
         curNode = nodes[i];
         break;
      }
   }
   
   //get rid of the temporary data we just used
   nodes.DisposeTemporary();

   return curNode;
}

//get the transformation matrices required to convert from one coordinate system to another
//Node must not be NULL
void EditPolyMod::getTransformationMatrices( Matrix3 & refMatrix, Matrix3 & worldTransformationMatrix, INode * node )
{
   //Can't do this without the interface or the node
   if ( !ip || !node )
   {
      return;
   }

   //Find out the reference coordinate system
   int coordsys = ip->GetRefCoordSys();
   //Get the active viewport
   ViewExp* view = ip->GetActiveViewport();

   if ( coordsys == COORDS_LOCAL )
   {
      //don't have to do anything if it should be the local coordinate system
      ip->ReleaseViewport( view );
      refMatrix.IdentityMatrix();
      worldTransformationMatrix.IdentityMatrix();
      return;
   }
   else if ( ( coordsys == COORDS_PARENT ) || ( coordsys == COORDS_GIMBAL ) )
   {
      //Get the parent matrix if possible, otherwise take the node's
      INode*   parent = node->GetParentNode();
      if (parent != NULL)
      {
         refMatrix = parent->GetObjectTM( ip->GetTime() );
      }
      else
      {
         refMatrix = node->GetObjectTM( ip->GetTime() );
      }
   }
   //other coordinate systems (this code was taken from maxnode.cpp)
   else if (coordsys == COORDS_SCREEN)
   {
      view->GetAffineTM( refMatrix );
   }
   else if (coordsys == COORDS_OBJECT)
   {
      view->GetConstructionTM( refMatrix );
   }

   ip->ReleaseViewport( view );

   //get world matrix
   worldTransformationMatrix = node->GetObjectTM( ip->GetTime() );
}

//convert a point from its local coordinates to the reference coordinates used by the system
//node must not be NULL
void EditPolyMod::convertPointToCurrentCoordSys( Point3 &refPoint, INode * node )
{
   Matrix3 m(1);
   Matrix3 wtm(1);

   getTransformationMatrices( m, wtm, node );

   // first map from object to world
   refPoint = refPoint * wtm;
   // then to current
   refPoint = refPoint * Inverse(m);
}

//convert a point from the reference coordinates used by the system to the local coordinate system of the node
//node must not be NULL
void EditPolyMod::convertPointFromCurrentCoordSys( Point3 &refPoint, INode * node )
{
   Matrix3 m(1);
   Matrix3 wtm(1);

   getTransformationMatrices( m, wtm, node );

   // 1st map from current into world coords
   refPoint = refPoint * m;
   m = wtm;
   // then to current
   refPoint = refPoint * Inverse(m);

}

//--- Named selection sets -----------------------------------------

class EditPolyAddSetRestore : public RestoreObj {
public:
   BitArray set;     
   DWORD id;
   TSTR name;
   GenericNamedSelSetList *setList;

   EditPolyAddSetRestore (GenericNamedSelSetList *sl, DWORD i, TSTR &n) : setList(sl), id(i), name(n) { }
   void Restore(int isUndo) {
      set  = *setList->GetSet (id);
      setList->RemoveSet(id);
   }
   void Redo() { setList->InsertSet (set, id, name); }
         
   TSTR Description() {return TSTR(_T("Add NS Set"));}
};

class EditPolyAddSetNameRestore : public RestoreObj {
public:     
   TSTR name;
   DWORD id;
   int index;  // location in list.

   EditPolyMod *et;
   Tab<TSTR*> *sets;
   Tab<DWORD> *ids;

   EditPolyAddSetNameRestore (EditPolyMod *e, DWORD idd, Tab<TSTR*> *s,Tab<DWORD> *i) : et(e), sets(s), ids(i), id(idd) { }
   void Restore(int isUndo);
   void Redo();            
   TSTR Description() {return TSTR(_T("Add Set Name"));}
};

void EditPolyAddSetNameRestore::Restore(int isUndo) {
   int sct = sets->Count();
   for (index=0; index<sct; index++) if ((*ids)[index] == id) break;
   if (index >= sct) return;

   name = *(*sets)[index];
   delete (*sets)[index];
   sets->Delete (index, 1);
   ids->Delete (index, 1);
   if (et->ip) et->ip->NamedSelSetListChanged();
}

void EditPolyAddSetNameRestore::Redo() {
   TSTR *nm = new TSTR(name);
   if (index < sets->Count()) {
      sets->Insert (index, 1, &nm);
      ids->Insert (index, 1, &id);
   } else {
      sets->Append (1, &nm);
      ids->Append (1, &id);
   }
   if (et->ip) et->ip->NamedSelSetListChanged();
}

class EditPolyDeleteSetRestore : public RestoreObj {
public:
   BitArray set;
   DWORD id;
   TSTR name;
   GenericNamedSelSetList *setList;

   EditPolyDeleteSetRestore(GenericNamedSelSetList *sl,DWORD i, TSTR & n) {
      setList = sl;
      id = i;
      BitArray *ptr = setList->GetSet(id);
      if (ptr) set = *ptr;
      name = n;
   }
   void Restore(int isUndo) { setList->InsertSet(set, id, name); }
   void Redo() { setList->RemoveSet(id); }
   TSTR Description() {return TSTR(_T("Delete Set"));}
};

class EditPolyDeleteSetNameRestore : public RestoreObj {
public:     
   TSTR name;
   DWORD id;
   EditPolyMod *et;
   Tab<TSTR*> *sets;
   Tab<DWORD> *ids;

   EditPolyDeleteSetNameRestore(Tab<TSTR*> *s,EditPolyMod *e,Tab<DWORD> *i, DWORD id) {
      sets = s; et = e;
      this->id = id;
      ids = i;
      int index = -1;
      for (int j=0; j<sets->Count(); j++) {
         if ((*ids)[j]==id) {
            index = j;
            break;
            }
         }
      if (index>=0) {
         name = *(*sets)[index];
         }
      }        
   void Restore(int isUndo) {       
      TSTR *nm = new TSTR(name);       
      //sets->Insert(index,1,&nm);
      sets->Append(1,&nm);
      ids->Append(1,&id);
      if (et->ip) et->ip->NamedSelSetListChanged();
      }
   void Redo() {
      int index = -1;
      for (int j=0; j<sets->Count(); j++) {
         if ((*ids)[j]==id) {
            index = j;
            break;
            }
         }
      if (index>=0) {
         sets->Delete(index,1);
         ids->Delete(index,1);
         }
      //sets->Delete(index,1);
      if (et->ip) et->ip->NamedSelSetListChanged();
      }
         
   TSTR Description() {return TSTR(_T("Delete Set Name"));}
};

class EditPolySetNameRestore : public RestoreObj {
public:
   TSTR undo, redo;
   DWORD id;
   Tab<TSTR*> *sets;
   Tab<DWORD> *ids;
   EditPolyMod *et;
   EditPolySetNameRestore(Tab<TSTR*> *s,EditPolyMod *e,Tab<DWORD> *i,DWORD id) {
      this->id = id;
      ids = i;
      sets = s; et = e;
      int index = -1;
      for (int j=0; j<sets->Count(); j++) {
         if ((*ids)[j]==id) {
            index = j;
            break;
            }
         }
      if (index>=0) {
         undo = *(*sets)[index];
         }
      }

   void Restore(int isUndo) {
      int index = -1;
      for (int j=0; j<sets->Count(); j++) {
         if ((*ids)[j]==id) {
            index = j;
            break;
            }
         }
      if (index>=0) {
         redo = *(*sets)[index];
         *(*sets)[index] = undo;
         }        
      if (et->ip) et->ip->NamedSelSetListChanged();
      }
   void Redo() {
      int index = -1;
      for (int j=0; j<sets->Count(); j++) {
         if ((*ids)[j]==id) {
            index = j;
            break;
         }
      }
      if (index>=0) {
         *(*sets)[index] = redo;
      }
      if (et->ip) et->ip->NamedSelSetListChanged();
   }
         
   TSTR Description() {return TSTR(_T("Set Name"));}
};

int EditPolyMod::FindSet(TSTR &setName, int nsl) {
   for (int i=0; i<namedSel[nsl].Count(); i++) {
      if (setName == *namedSel[nsl][i]) return i;
   }
   return -1;
}

DWORD EditPolyMod::AddSet(TSTR &setName,int nsl) {
   DWORD id = 0;
   TSTR *name = new TSTR(setName);
   int nsCount = namedSel[nsl].Count();

   // Find an empty id to assign to this set.
   BOOL found = FALSE;
   while (!found) {
      found = TRUE;
      for (int i=0; i<ids[nsl].Count(); i++) {
         if (ids[nsl][i]!=id) continue;
         id++;
         found = FALSE;
         break;
      }
   }

   // Find location in alphabetized list:
   int pos;
   for (pos=0; pos<nsCount; pos++) if (setName < *(namedSel[nsl][pos])) break;
   if (pos == nsCount) {
      namedSel[nsl].Append(1,&name);
      ids[nsl].Append(1,&id);
   } else {
      namedSel[nsl].Insert (pos, 1, &name);
      ids[nsl].Insert (pos, 1, &id);
   }

   return id;
}

void EditPolyMod::RemoveSet(TSTR &setName,int nsl) {
   int i = FindSet(setName,nsl);
   if (i<0) return;
   delete namedSel[nsl][i];
   namedSel[nsl].Delete(i,1);
   ids[nsl].Delete(i,1);
}

void EditPolyMod::UpdateSetNames () {
   if (!ip) return;

   ModContextList mcList;
   INodeTab nodes;
   ip->GetModContexts(mcList,nodes);

   for (int i=0; i<mcList.Count(); i++) {
      EditPolyData *meshData = (EditPolyData*)mcList[i]->localData;
      if ( !meshData ) continue;
      for (int nsl=0; nsl<3; nsl++) {
         // Make sure the namedSel array is in alpha order.
         // (Crude bubble sort since we expect that it will be.)
         int j, k, kmax = namedSel[nsl].Count();
         for (k=1; k<kmax; k++) {
            if (*(namedSel[nsl][k-1]) < *(namedSel[nsl][k])) continue;
            for (j=0; j<k-1; j++) {
               if (*(namedSel[nsl][j]) > *(namedSel[nsl][k])) break;
            }
            // j now represents the point at which k should be inserted.
            TSTR *hold = namedSel[nsl][k];
            DWORD dhold = ids[nsl][k];
            int j2;
            for (j2=k; j2>j; j2--) {
               namedSel[nsl][j2] = namedSel[nsl][j2-1];
               ids[nsl][j2] = ids[nsl][j2-1];
            }
            namedSel[nsl][j] = hold;
            ids[nsl][j] = dhold;
         }

         GenericNamedSelSetList & gnsl = meshData->GetNamedSel(nsl);
         // Check for old, unnamed or misnamed sets with ids.
         for (k=0; k<gnsl.Count(); k++) {
            for (j=0; j<ids[nsl].Count(); j++) if (ids[nsl][j] == gnsl.ids[k]) break;
            if (j == ids[nsl].Count()) continue;
            if (gnsl.names[k] && !(*(gnsl.names[k]) == *(namedSel[nsl][j]))) {
               delete gnsl.names[k];
               gnsl.names[k] = NULL;
            }
            if (gnsl.names[k]) continue;
            gnsl.names[k] = new TSTR(*(namedSel[nsl][j]));
         }
         gnsl.Alphabetize ();

         // Now check lists against each other, adding any missing elements.
         for (j=0; j<gnsl.Count(); j++) {
            if (*(gnsl.names[j]) == *(namedSel[nsl][j])) continue;
            if (j>= namedSel[nsl].Count()) {
               TSTR *nname = new TSTR(*gnsl.names[j]);
               DWORD nid = gnsl.ids[j];
               namedSel[nsl].Append (1, &nname);
               ids[nsl].Append (1, &nid);
               continue;
            }
            if (*(gnsl.names[j]) > *(namedSel[nsl][j])) {
               BitArray baTemp;
               gnsl.InsertSet (j, baTemp, ids[nsl][j], *(namedSel[nsl][j]));
               continue;
            }
            // Otherwise:
            TSTR *nname = new TSTR(*gnsl.names[j]);
            DWORD nid = gnsl.ids[j];
            namedSel[nsl].Insert (j, 1, &nname);
            ids[nsl].Insert (j, 1, &nid);
         }
         for (; j<namedSel[nsl].Count(); j++) {
            BitArray baTemp;
            gnsl.AppendSet (baTemp, ids[nsl][j], *(namedSel[nsl][j]));
         }
      }
   }

   nodes.DisposeTemporary();
}

void EditPolyMod::ClearSetNames() {
   for (int i=0; i<3; i++) {
      for (int j=0; j<namedSel[i].Count(); j++) {
         delete namedSel[i][j];
         namedSel[i][j] = NULL;
      }
   }
}

void EditPolyMod::ActivateSubSelSet(TSTR &setName) {
   ModContextList mcList;
   INodeTab nodes;
   int nsl = namedSetLevel[selLevel];
   int index = FindSet (setName, nsl); 
   if (index<0 || !ip) return;

   theHold.Begin ();
   ip->GetModContexts(mcList,nodes);

   for (int i = 0; i < mcList.Count(); i++) {
      EditPolyData *meshData = (EditPolyData*)mcList[i]->localData;
      if (!meshData) continue;
      if (theHold.Holding() && !meshData->GetFlag(kEPDataHeld))
         theHold.Put(new EditPolySelectRestore(this,meshData));

      BitArray *set = NULL;

      switch (nsl) {
      case NS_VERTEX:
         set = meshData->vselSet.GetSet(ids[nsl][index]);
         if (set) {
            if (set->GetSize()!=meshData->mVertSel.GetSize()) {
               set->SetSize(meshData->mVertSel.GetSize(),TRUE);
            }
            meshData->SetVertSel (*set, this, ip->GetTime());
         }
         break;

      case NS_FACE:
         set = meshData->fselSet.GetSet(ids[nsl][index]);
         if (set) {
            if (set->GetSize()!=meshData->mFaceSel.GetSize()) {
               set->SetSize(meshData->mFaceSel.GetSize(),TRUE);
            }
            meshData->SetFaceSel (*set, this, ip->GetTime());
         }
         break;

      case NS_EDGE:
         set = meshData->eselSet.GetSet(ids[nsl][index]);
         if (set) {
            if (set->GetSize()!=meshData->mEdgeSel.GetSize()) {
               set->SetSize(meshData->mEdgeSel.GetSize(),TRUE);
            }
            meshData->SetEdgeSel (*set, this, ip->GetTime());
         }
         break;
      }
   }
   
   nodes.DisposeTemporary();
   LocalDataChanged ();
   theHold.Accept (GetString (IDS_SELECT));
   ip->RedrawViews(ip->GetTime());
}

void EditPolyMod::NewSetFromCurSel(TSTR &setName) {
   ModContextList mcList;
   INodeTab nodes;
   DWORD id = -1;
   int nsl = namedSetLevel[selLevel];
   int index = FindSet(setName, nsl);
   if (index<0) {
      id = AddSet(setName, nsl);
      if (theHold.Holding()) theHold.Put (new EditPolyAddSetNameRestore (this, id, &namedSel[nsl], &ids[nsl]));
   } else id = ids[nsl][index];

   ip->GetModContexts(mcList,nodes);

   for (int i = 0; i < mcList.Count(); i++) {
      EditPolyData *meshData = (EditPolyData*)mcList[i]->localData;
      if (!meshData) continue;
      
      BitArray *set = NULL;

      switch (nsl) {
      case NS_VERTEX:   
         if (index>=0 && (set = meshData->vselSet.GetSet(id)) != NULL) {
            *set = meshData->mVertSel;
         } else {
            meshData->vselSet.InsertSet (meshData->mVertSel,id, setName);
            if (theHold.Holding()) theHold.Put (new EditPolyAddSetRestore (&(meshData->vselSet), id, setName));
         }
         break;

      case NS_FACE:
         if (index>=0 && (set = meshData->fselSet.GetSet(id)) != NULL) {
            *set = meshData->mFaceSel;
         } else {
            meshData->fselSet.InsertSet(meshData->mFaceSel,id, setName);
            if (theHold.Holding()) theHold.Put (new EditPolyAddSetRestore (&(meshData->fselSet), id, setName));
         }
         break;

      case NS_EDGE:
         if (index>=0 && (set = meshData->eselSet.GetSet(id)) != NULL) {
            *set = meshData->mEdgeSel;
         } else {
            meshData->eselSet.InsertSet(meshData->mEdgeSel, id, setName);
            if (theHold.Holding()) theHold.Put (new EditPolyAddSetRestore (&(meshData->eselSet), id, setName));
         }
         break;
      }
   }  
   nodes.DisposeTemporary();
}

void EditPolyMod::RemoveSubSelSet(TSTR &setName) {
   int nsl = namedSetLevel[selLevel];
   int index = FindSet (setName, nsl);
   if (index<0 || !ip) return;      

   ModContextList mcList;
   INodeTab nodes;
   ip->GetModContexts(mcList,nodes);

   DWORD id = ids[nsl][index];

   for (int i = 0; i < mcList.Count(); i++) {
      EditPolyData *meshData = (EditPolyData*)mcList[i]->localData;
      if (!meshData) continue;      

      switch (nsl) {
      case NS_VERTEX:   
         if (theHold.Holding()) theHold.Put(new EditPolyDeleteSetRestore(&meshData->vselSet,id, setName));
         meshData->vselSet.RemoveSet(id);
         break;

      case NS_FACE:
         if (theHold.Holding()) theHold.Put(new EditPolyDeleteSetRestore(&meshData->fselSet,id, setName));
         meshData->fselSet.RemoveSet(id);
         break;

      case NS_EDGE:
         if (theHold.Holding()) theHold.Put(new EditPolyDeleteSetRestore(&meshData->eselSet,id, setName));
         meshData->eselSet.RemoveSet(id);
         break;
      }
   }
   
   if (theHold.Holding()) theHold.Put(new EditPolyDeleteSetNameRestore(&(namedSel[nsl]),this,&(ids[nsl]),id));
   RemoveSet (setName, nsl);
   ip->ClearCurNamedSelSet();
   nodes.DisposeTemporary();
}

void EditPolyMod::SetupNamedSelDropDown() {
   if (selLevel == EPM_SL_OBJECT) return;
   ip->ClearSubObjectNamedSelSets();
   int nsl = namedSetLevel[selLevel];
   for (int i=0; i<namedSel[nsl].Count(); i++)
      ip->AppendSubObjectNamedSelSet(*namedSel[nsl][i]);
   UpdateNamedSelDropDown ();
}

int EditPolyMod::NumNamedSelSets() {
   int nsl = namedSetLevel[selLevel];
   return namedSel[nsl].Count();
}

TSTR EditPolyMod::GetNamedSelSetName(int i) {
   int nsl = namedSetLevel[selLevel];
   return *namedSel[nsl][i];
}

void EditPolyMod::SetNamedSelSetName(int i,TSTR &newName) {
   int nsl = namedSetLevel[selLevel];
   if (theHold.Holding()) theHold.Put(new EditPolySetNameRestore(&namedSel[nsl],this,&ids[nsl],ids[nsl][i]));
   *namedSel[nsl][i] = newName;
}

void EditPolyMod::NewSetByOperator(TSTR &newName,Tab<int> &sets,int op) {
   int nsl = namedSetLevel[selLevel];
   DWORD id = AddSet(newName, nsl);
   if (theHold.Holding())
      theHold.Put(new EditPolyAddSetNameRestore(this, id, &namedSel[nsl], &ids[nsl]));

   ModContextList mcList;
   INodeTab nodes;
   ip->GetModContexts(mcList,nodes);
   for (int i = 0; i < mcList.Count(); i++) {
      EditPolyData *meshData = (EditPolyData*)mcList[i]->localData;
      if (!meshData) continue;
   
      BitArray bits;
      GenericNamedSelSetList *setList = NULL;

      switch (nsl) {
      case NS_VERTEX: setList = &meshData->vselSet; break;
      case NS_FACE: setList = &meshData->fselSet; break;       
      case NS_EDGE:   setList = &meshData->eselSet; break;        
      }     

      bits = (*setList)[sets[0]];

      for (int i=1; i<sets.Count(); i++) {
         switch (op) {
         case NEWSET_MERGE:
            bits |= (*setList)[sets[i]];
            break;

         case NEWSET_INTERSECTION:
            bits &= (*setList)[sets[i]];
            break;

         case NEWSET_SUBTRACT:
            bits &= ~((*setList)[sets[i]]);
            break;
         }
      }
      setList->InsertSet (bits, id, newName);
      if (theHold.Holding()) theHold.Put(new EditPolyAddSetRestore(setList, id, newName));
   }
}

void EditPolyMod::NSCopy() {
   int index = SelectNamedSet();
   if (index<0) return;
   if (!ip) return;

   int nsl = namedSetLevel[selLevel];
   MeshNamedSelClip *clip = new MeshNamedSelClip(*namedSel[nsl][index]);

   ModContextList mcList;
   INodeTab nodes;
   ip->GetModContexts(mcList,nodes);

   for (int i = 0; i < mcList.Count(); i++) {
      EditPolyData *meshData = (EditPolyData*)mcList[i]->localData;
      if (!meshData) continue;

      GenericNamedSelSetList *setList = NULL;
      switch (nsl) {
      case NS_VERTEX: setList = &meshData->vselSet; break;           
      case NS_FACE: setList = &meshData->fselSet; break;       
      case NS_EDGE: setList = &meshData->eselSet; break;       
      }     

      BitArray *bits = new BitArray(*setList->sets[index]);
      clip->sets.Append(1,&bits);
   }
   SetMeshNamedSelClip(clip, namedClipLevel[selLevel]);
   
   // Enable the paste button
   HWND hWnd = GetDlgHandle (ep_geom);
   if (!hWnd) return;
   ICustButton* but = NULL;
   but = GetICustButton(GetDlgItem(hWnd,IDC_PASTE_NS));
   but->Enable();
   ReleaseICustButton(but);
}

void EditPolyMod::NSPaste() {
   int nsl = namedSetLevel[selLevel];
   MeshNamedSelClip *clip = GetMeshNamedSelClip(namedClipLevel[selLevel]);
   if (!clip) return;   
   TSTR name = clip->name;
   if (!GetUniqueSetName(name)) return;

   EpModAboutToChangeSelection ();

   ModContextList mcList;
   INodeTab nodes;
   theHold.Begin();

   DWORD id = AddSet (name, nsl);   
   theHold.Put(new EditPolyAddSetNameRestore(this, id, &namedSel[nsl], &ids[nsl])); 

   ip->GetModContexts(mcList,nodes);
   for (int i = 0; i < mcList.Count(); i++) {
      EditPolyData *meshData = (EditPolyData*)mcList[i]->localData;
      if (!meshData) continue;

      GenericNamedSelSetList *setList = NULL;
      switch (nsl) {
      case NS_VERTEX: setList = &meshData->vselSet; break;
      case NS_EDGE: setList = &meshData->eselSet; break;
      case NS_FACE: setList = &meshData->fselSet; break;
      }
            
      if (i>=clip->sets.Count()) {
         BitArray bits;
         setList->InsertSet(bits,id,name);
      } else setList->InsertSet(*clip->sets[i],id,name);
      if (theHold.Holding()) theHold.Put(new EditPolyAddSetRestore(setList, id, name));
   }  
   
   ActivateSubSelSet(name);
   ip->SetCurNamedSelSet(name);
   theHold.Accept(GetString (IDS_PASTE_NS));
   SetupNamedSelDropDown();
}

static INT_PTR CALLBACK EditPolyPickSetDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg) {
   case WM_INITDIALOG:  {
      Tab<TSTR*> *setList = (Tab<TSTR*>*)lParam;
      for (int i=0; i<setList->Count(); i++) {
         int pos  = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_ADDSTRING,0,
            (LPARAM)(TCHAR*)*(*setList)[i]);
         SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_SETITEMDATA,pos,i);
      }
      break;
   }

   case WM_COMMAND:
      switch (LOWORD(wParam)) {
      case IDC_NS_LIST:
         if (HIWORD(wParam)!=LBN_DBLCLK) break;
         // fall through

      case IDOK:
         int sel;
         sel = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETCURSEL,0,0);
         if (sel!=LB_ERR) {
            int res =SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETITEMDATA,sel,0);
            EndDialog(hWnd,res);
            break;
         }
         // fall through

      case IDCANCEL:
         EndDialog(hWnd,-1);
         break;
      }
      break;

   default:
      return FALSE;
   }
   return TRUE;
}

int EditPolyMod::SelectNamedSet() {
   Tab<TSTR*> &setList = namedSel[namedSetLevel[selLevel]];
   if (!ip) return false;
   return DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_NAMEDSET_SEL),
      ip->GetMAXHWnd(), EditPolyPickSetDlgProc, (LPARAM)&setList);
}

void EditPolyMod::UpdateNamedSelDropDown () {
   if (!ip) return;
   if (selLevel == EPM_SL_OBJECT) {
      ip->ClearCurNamedSelSet ();
      return;
   }
   // See if this selection matches a named set
   ModContextList mcList;
   INodeTab nodes;
   ip->GetModContexts (mcList, nodes);
   BitArray nselmatch;
   nselmatch.SetSize (namedSel[namedSetLevel[selLevel]].Count());
   nselmatch.SetAll ();
   int nd, i, foundone=FALSE;
   for (nd=0; nd<mcList.Count(); nd++) {
      EditPolyData *d = (EditPolyData *) mcList[nd]->localData;
      if (!d) continue;
      foundone = TRUE;
      switch (selLevel) {
      case EPM_SL_VERTEX:
         for (i=0; i<nselmatch.GetSize(); i++) {
            if (!nselmatch[i]) continue;
            if (!(*(d->vselSet.sets[i]) == d->mVertSel)) nselmatch.Clear(i);
         }
         break;
      case EPM_SL_EDGE:
      case EPM_SL_BORDER:
         for (i=0; i<nselmatch.GetSize(); i++) {
            if (!nselmatch[i]) continue;
            if (!(*(d->eselSet.sets[i]) == d->mEdgeSel)) nselmatch.Clear(i);
         }
         break;
      default:
         for (i=0; i<nselmatch.GetSize(); i++) {
            if (!nselmatch[i]) continue;
            if (!(*(d->fselSet.sets[i]) == d->mFaceSel)) nselmatch.Clear(i);
         }
         break;
      }
      if (nselmatch.NumberSet () == 0) break;
   }
   if (foundone && nselmatch.AnyBitSet ()) {
      for (i=0; i<nselmatch.GetSize(); i++) if (nselmatch[i]) break;
      ip->SetCurNamedSelSet (*(namedSel[namedSetLevel[selLevel]][i]));
   } else ip->ClearCurNamedSelSet ();
}

static INT_PTR CALLBACK PickSetNameDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
   static TSTR *name;

   switch (msg) {
   case WM_INITDIALOG: {
      name = (TSTR*)lParam;
      ICustEdit *edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
      edit->SetText(*name);
      ReleaseICustEdit(edit);
      break;
      }

   case WM_COMMAND:
      switch (LOWORD(wParam)) {
      case IDOK: {
         ICustEdit *edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
         TCHAR buf[256];
         edit->GetText(buf,256);
         *name = TSTR(buf);
         ReleaseICustEdit(edit);
         EndDialog(hWnd,1);
         break;
         }

      case IDCANCEL:
         EndDialog(hWnd,0);
         break;
      }
      break;

   default:
      return FALSE;
   };
   return TRUE;
}

BOOL EditPolyMod::GetUniqueSetName(TSTR &name) {
   while (1) {          
      Tab<TSTR*> &setList = namedSel[namedSetLevel[selLevel]];

      BOOL unique = TRUE;
      for (int i=0; i<setList.Count(); i++) {
         if (name==*setList[i]) {
            unique = FALSE;
            break;
         }
      }
      if (unique) break;

      if (!ip) return FALSE;
      if (!DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_NAMEDSET_PASTE),
         ip->GetMAXHWnd(), PickSetNameDlgProc, (LPARAM)&name)) return FALSE;
   }
   return TRUE;
}


void EditPolyMod::UpdateCageColor() 
{
   if ( m_iColorCageSwatch)
   {
      DWORD l_rgb = m_iColorCageSwatch->GetColor();
      m_CageColor.x = (float) GetRValue(l_rgb)/255.0;
      m_CageColor.y = (float) GetGValue(l_rgb)/255.0;
      m_CageColor.z = (float) GetBValue(l_rgb)/255.0;
   }
}


void EditPolyMod::UpdateSelectedCageColor() 
{
   if ( m_iColorSelectedCageSwatch)
   {
      DWORD l_rgb = m_iColorSelectedCageSwatch->GetColor();
      m_SelectedCageColor.x = (float) GetRValue(l_rgb)/255.0;
      m_SelectedCageColor.y = (float) GetGValue(l_rgb)/255.0;
      m_SelectedCageColor.z = (float) GetBValue(l_rgb)/255.0;
   }

}


void EditPolyMod::UpdateLoopEdgeSelection( const int in_spinVal )
{
   bool     l_add       = getCtrlKey();
   bool     l_remove    = getAltKey() ;
   TimeValue   t           = ip->GetTime();

   ModContextList l_mcList;
   INodeTab    nodes;   

   
   ip->GetModContexts(l_mcList,nodes);

   for (int i = 0; i < l_mcList.Count(); i++) 
   {
      EditPolyData *l_meshData = (EditPolyData*)l_mcList[i]->localData;
      if (!l_meshData) continue;    

      MNMesh *pMesh = l_meshData->GetMesh();

      BitArray l_newSel;
      l_newSel.SetSize(pMesh->nume);
      l_newSel.ClearAll();


      IMNMeshUtilities8* l_meshToBridge = static_cast<IMNMeshUtilities8*>(pMesh->GetInterface( IMNMESHUTILITIES8_INTERFACE_ID ));

      if (l_meshToBridge )
      {

         if ( in_spinVal - mLoopSpinnerValue < 0 )
            if ( l_remove)
               l_meshToBridge->SelectEdgeLoopShift(1,l_newSel);
            else 
               l_meshToBridge->SelectEdgeLoopShift(-1,l_newSel);
         else if ( in_spinVal - mLoopSpinnerValue > 0 )
            if ( l_remove)
               l_meshToBridge->SelectEdgeLoopShift(-1,l_newSel);
            else 
               l_meshToBridge->SelectEdgeLoopShift(1,l_newSel);


         // update local value 
         if ( l_add )
         {
            // add the current selection to the shifted selection
            l_newSel |= l_meshData->mEdgeSel ;
         }
         else if ( l_remove )
         {
            l_newSel &= l_meshData->mEdgeSel;
         }

         if ( !l_newSel.IsEmpty()  )
         {
            // we don't want this action to be recorded in the undo stack 
            // but we want to take full advantage of all its sub-actions ( like clearing selected edge data .. )
            // laurent R. 05/05 
            theHold.Suspend();
            l_meshData->mpMesh->ClearEFlags (MN_SEL);
            // update the mesh selection with a new bitarray
            l_meshData->SetEdgeSel (l_newSel, this, t);
            theHold.Resume();
         }
         macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].LoopSelect"), 3, 0,
            mr_int,     in_spinVal,
            mr_varname, !l_add&&!l_remove ? _T("true") : _T("false"),
            mr_varname, l_add ? _T("true") : _T("false"));
         macroRecorder->EmitScript();

         // update the member data 
         mLoopSpinnerValue = in_spinVal;
   

      }

   }

   // notify the rest of the world : sub-object selection has changed 
   EpModLocalDataChanged (PART_SELECT);



}


void EditPolyMod::UpdateRingEdgeSelection( const int in_spinVal )
{
   bool  l_add          = getCtrlKey();
   bool  l_remove       = getAltKey() ;


   TimeValue t = ip->GetTime();


   ModContextList l_mcList;
   INodeTab    nodes;   
   ip->GetModContexts(l_mcList,nodes);


   for (int i = 0; i < l_mcList.Count(); i++) 
   {
      EditPolyData *l_meshData = (EditPolyData*)l_mcList[i]->localData;
      if (!l_meshData) continue;    

      MNMesh *pMesh = l_meshData->GetMesh();

      BitArray l_newSel;
      l_newSel.SetSize(pMesh->nume);
      l_newSel.ClearAll();


      IMNMeshUtilities8* l_meshToBridge = static_cast<IMNMeshUtilities8*>(pMesh->GetInterface( IMNMESHUTILITIES8_INTERFACE_ID ));

      if (l_meshToBridge )
      {
         if ( in_spinVal - mRingSpinnerValue < 0 )
            if ( l_remove )
               l_meshToBridge->SelectEdgeRingShift(1,l_newSel);
            else 
            l_meshToBridge->SelectEdgeRingShift(-1,l_newSel);
         else if ( in_spinVal - mRingSpinnerValue > 0 )
            if ( l_remove)
               l_meshToBridge->SelectEdgeRingShift(-1,l_newSel);
            else 
            l_meshToBridge->SelectEdgeRingShift(1,l_newSel);

         // update local value 
         if ( l_add )
         {
            // add the current selection to the shifted selection
            l_newSel |= l_meshData->mEdgeSel ;
         }
         else if ( l_remove )
         {
            l_newSel &= l_meshData->mEdgeSel;
         }

         if ( !l_newSel.IsEmpty()  )
         {
            // we don't want this action to be recorded in the undo stack 
            // but we want to take full advantage of all its sub-actions ( like clearing selected edge data .. )
            // laurent R. 05/05 
            theHold.Suspend();
            l_meshData->mpMesh->ClearEFlags (MN_SEL);
            // finally update the mesh selection with a new bitarray
            l_meshData->SetEdgeSel (l_newSel, this, t);
            theHold.Resume();
         }


         macroRecorder->FunctionCall (_T("$.modifiers[#Edit_Poly].RingSelect"), 3, 0,
            mr_int,     in_spinVal,
            mr_varname, !l_add&&!l_remove ? _T("true") : _T("false"),
            mr_varname, l_add ? _T("true") : _T("false"));
         macroRecorder->EmitScript();


         // update the member data 
         mRingSpinnerValue = in_spinVal;
      }
   }

   // notify the rest of the world : sub-object selection has changed 
   EpModLocalDataChanged (PART_SELECT);

}


//new functions for selecting and drawing the new highlighted selections. EP_HIGHLIGHT flag

void EditPolyMod::DisplayHighlightFace (TimeValue t, GraphicsWindow *gw, MNMesh *mesh, int ff) {
	
	if (!mesh||ff<0||ff>=mesh->numf) return;
	mesh->render3DFace(gw, ff);
}

void EditPolyMod::DisplayHighlightEdge (TimeValue t, GraphicsWindow *gw, MNMesh *mesh, int ee)
{
	if (!mesh||ee<0 ||ee>=mesh->nume) return;
	Point3 xyz[3];
	xyz[0] = mesh->v[mesh->e[ee].v1].p;
	xyz[1] = mesh->v[mesh->e[ee].v2].p;
	gw->polyline(2, xyz, NULL, NULL, 0, NULL);
}

void EditPolyMod::DisplayHighlightVert (TimeValue t, GraphicsWindow *gw,MNMesh *mesh, int vv)
{
	if (!mesh||vv<0||vv>=mesh->numv) return;
	Point3 & p = mesh->v[vv].p;
	if (getUseVertexDots()) gw->marker (&p, VERTEX_DOT_MARKER(getVertexDotType()));
	else gw->marker (&p, PLUS_SIGN_MRKR);
}

//3 new functions added for Max 13 by the EPolyMod13 class
void EditPolyMod::EPMeshStartSetVertices(INode* pNode)
{
	UpdateCache(GetCOREInterface()->GetTime());
	EpModSetOperation (ep_op_transform);
	EditPolyData *pData = GetEditPolyDataForNode(pNode);
	// Make sure we're set to the right operation:
	pData->SetPolyOpData (ep_op_transform);

	LocalTransformData *pTransform = (LocalTransformData *) pData->GetPolyOpData();
	int numv = pData->GetMesh()->numv;
	pTransform->SetTempOffsetCount(numv);
	pTransform->SetNodeName(pNode->NodeName());
}
void EditPolyMod::EPMeshSetVert( const int vert, const Point3 & in_point, INode* pNode)
{
	EditPolyData *pData = GetEditPolyDataForNode(pNode);
	if (!pData->GetPolyOpData() || (pData->GetPolyOpData()->OpID() != ep_op_transform)) return;
	LocalTransformData *pTransform = (LocalTransformData *) pData->GetPolyOpData();
	MNMesh* mesh = pData->GetMesh();
	Point3 offset = in_point - mesh->V(vert)->p;
	pTransform->SetTempOffset(vert,offset);
}
void EditPolyMod::EPMeshEndSetVertices(INode* pNode)
{
	TimeValue t = GetCOREInterface()->GetTime();
	EpModDragAccept(pNode, t);
}

void EditPolyMod::ResetGripUI()
{
	IBaseGrip *grip  = GetIGripManager()->GetActiveGrip();
	//try to convert this grip to a EditablePolyGrip

	if(grip)
	{
		EditPolyModPolyOperationGrip * pEditablePolyGrip = dynamic_cast<EditPolyModPolyOperationGrip*>(grip);
		if(pEditablePolyGrip)
			pEditablePolyGrip->SetUpUI();
		else if(grip == &mManipGrip)
			mManipGrip.SetUpUI();

	}
}

void EditPolyMod::ModeChanged(CommandMode *old, CommandMode *) //from callback
{
	//only reset and unregister if we are exiting one of the modes that started this, otherwise we may exit prematurely when the mode gets started instead
	if(old == pickHingeMode || old == pickBridgeTarget1 || old == pickBridgeTarget2)
	{
		ResetGripUI();
		GetCOREInterface()->UnRegisterCommandModeChangedCallback(this);
	}
}

void EditPolyMod::RegisterCMChangedForGrip()
{
	//reset the grip UI, new command mode coming in
	ResetGripUI();
	GetCOREInterface()->RegisterCommandModeChangedCallback(this);

}

void EditPolyMod::SetManipulateGrip(bool on, EPolyMod13::ManipulateGrips val)
{
	SetUpManipGripIfNeeded();
}

bool EditPolyMod::GetManipulateGrip(EPolyMod13::ManipulateGrips val)
{
	return GetEPolyManipulatorGrip()->GetManipulateGrip((EPolyManipulatorGrip::ManipulateGrips)val);
}


bool EditPolyMod::ManipGripIsValid()
{
	//soft's valid for all, but must be on.
	int useSoftsel;
	mpParams->GetValue (epm_ss_use, TimeValue(0), useSoftsel, FOREVER);
	if(useSoftsel)
	{
		if( GetManipulateGrip( EPolyMod13::eSoftSelFalloff) ||  
			GetManipulateGrip( EPolyMod13::eSoftSelBubble)||
			GetManipulateGrip( EPolyMod13::eSoftSelPinch))
			return true;
	}
	if( GetEPolySelLevel() == EPM_SL_EDGE) // only valid for edge modes.
	{
		if( GetManipulateGrip(EPolyMod13::eSetFlow) ||  
		GetManipulateGrip( EPolyMod13::eLoopShift) ||
		GetManipulateGrip( EPolyMod13::eRingShift))
		return true;
	}
	return false;
}

//called when either a selection changed or bitmask changes
//does 2 things, will turn on manip if something got activiated and we should
//turn it on (no other valid grip), and secondly, will change the visibility
//of any active grip.
void EditPolyMod::SetUpManipGripIfNeeded()
{
	//if there's an active grip and we are in manip mode and with our grip active
	if(GetCOREInterface7()->InManipMode()==TRUE )
	{
		if(GetIGripManager()->GetActiveGrip() == &mManipGrip)
		{
			if(ManipGripIsValid()==false)
				GetIGripManager()->SetGripsInactive();
			else
			{
				SetUpManipGripVisibility();			
			}
		}
		else if(GetIGripManager()->GetActiveGrip() == NULL)
		{
			//in manip mode but no grip may be active, so we may need to turn it on!
			if(ManipGripIsValid())
			{
				GetIGripManager()->SetGripActive(&mManipGrip);
				SetUpManipGripVisibility();
			}
		}
	}
}

//called when subobject or bit mask changes and after the manip grip is turned on
void EditPolyMod::SetUpManipGripVisibility()
{
	//if on
	if(GetCOREInterface7()->InManipMode()==TRUE && 
		GetIGripManager()->GetActiveGrip() == &mManipGrip)
	{
		BitArray copy;
		copy.SetSize((int)EPolyMod13::eRingShift+1);
		copy.Set((int)EPolyMod13::eSoftSelFalloff, GetManipulateGrip(EPolyMod13::eSoftSelFalloff)) ;
		copy.Set((int)EPolyMod13::eSoftSelPinch,GetManipulateGrip(EPolyMod13::eSoftSelPinch)) ;
		copy.Set((int)EPolyMod13::eSoftSelBubble,GetManipulateGrip(EPolyMod13::eSoftSelBubble)) ;
		copy.Set((int)EPolyMod13::eSetFlow,GetManipulateGrip(EPolyMod13::eSetFlow)) ;
		copy.Set((int)EPolyMod13::eLoopShift,GetManipulateGrip(EPolyMod13::eLoopShift)) ;
		copy.Set((int)EPolyMod13::eRingShift, GetManipulateGrip(EPolyMod13::eRingShift)) ;
		if( GetEPolySelLevel() != EPM_SL_EDGE) // if not in edge, turn off the edge ones!
		{
			copy.Clear((int) EPolyMod13::eSetFlow);
			copy.Clear((int) EPolyMod13::eLoopShift);
			copy.Clear((int) EPolyMod13::eRingShift);
		}
		int useSoftsel;
		mpParams->GetValue (epm_ss_use, TimeValue(0), useSoftsel, FOREVER);
		if(useSoftsel ==0)
		{
			copy.Clear((int) EPolyMod13::eSoftSelBubble);
			copy.Clear((int) EPolyMod13::eSoftSelFalloff);
			copy.Clear((int) EPolyMod13::eSoftSelPinch);
		}

		//double check, if copy is zero then turn off the grip!
		if(copy.IsEmpty())
			GetIGripManager()->SetGripsInactive();
		else
			mManipGrip.SetUpVisibility(copy);
	}
}
void EditPolyMod::GripChanged(IBaseGrip *grip)
{
	if(grip==NULL && GetCOREInterface7()->InManipMode()==TRUE)
	{
		if(ManipGripIsValid()) 
		{
			SetUpManipGripIfNeeded();
		}
	}
}
