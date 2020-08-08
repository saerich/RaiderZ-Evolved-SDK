/**********************************************************************
*<
FILE:       AVCUtil.cpp
DESCRIPTION:   Vertex Color Utility
CREATED BY:    Christer Janson
HISTORY:    Created Monday, June 02, 1997

  *>  Copyright (c) 1997 Kinetix, All Rights Reserved.
**********************************************************************/

#include "ApplyVC.h"
#include "modstack.h"
#include "Radiosity.h"
#include "radiosityMesh.h"
#include "mentalray\mentalrayInMax.h"     // MRRENDERER_CLASSID

#include "3dsmaxport.h"

static ApplyVCUtil theApplyVC;

#define CFGFILE      _T("APPLYVC.CFG")
#define CFGVERSION   5

//==============================================================================
// class ApplyVCClassDesc
//==============================================================================
class ApplyVCClassDesc:public ClassDesc {
public:
    int        IsPublic() {return 1;}
    void *        Create(BOOL loading = FALSE) {return &theApplyVC;}
    const TCHAR * ClassName() {return GetString(IDS_AVCU_CNAME);}
    SClass_ID     SuperClassID() {return UTILITY_CLASS_ID;}
    Class_ID      ClassID() {return APPLYVC_UTIL_CLASS_ID;}
    const TCHAR*  Category() {return _T("");}
};

static ApplyVCClassDesc ApplyVCUtilDesc;
ClassDesc* GetApplyVCUtilDesc() {return &ApplyVCUtilDesc;}

#ifndef USE_ORIGINAL_VERTEX_PAINT
//==============================================================================
// class ProgressDialog
//==============================================================================
class ProgressDialog {
public:
    void Begin();
    void End();
    BOOL Progress(int done, int total); //return TRUE if the user canceled

   void SetText(const TCHAR* text);
   static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    HWND hWnd;
    BOOL canceled;
   Interface* ip;
};

INT_PTR CALLBACK ProgressDialog::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ProgressDialog* dialog = NULL;
   if(uMsg == WM_INITDIALOG) {
      dialog = (ProgressDialog*)(lParam);
      dialog->hWnd = hDlg;
      DLSetWindowLongPtr(hDlg, lParam);
      CenterWindow(hDlg, GetCOREInterface()->GetMAXHWnd());
      SendDlgItemMessage(hDlg, IDC_PROGRESSDLG_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
   }
   else
      dialog = DLGetWindowLongPtr<ProgressDialog*>(hDlg);

   // Check if 'canceled' was pressed.
   if( (dialog!=NULL) && (uMsg == WM_COMMAND) && (LOWORD(wParam) == IDCANCEL))
      dialog->canceled = TRUE;

   return FALSE;
}

void ProgressDialog::Begin() {
   ip = GetCOREInterface();
   canceled = FALSE;
   HWND hWnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_PROGRESSDLG), 
      ip->GetMAXHWnd(), DialogProc, (LPARAM)(this));
    DbgAssert(hWnd != NULL);
}

void ProgressDialog::End() {
    if(hWnd!=NULL) EndDialog( hWnd, !canceled );
    hWnd = NULL;
}

BOOL ProgressDialog::Progress(int done, int total)
{
    if(total > 0)
      SendDlgItemMessage(hWnd, IDC_PROGRESSDLG_PROGRESS, PBM_SETPOS, (done*1000)/total, 0);

    if(!ip->CheckMAXMessages())
        return FALSE;

    return canceled;
}

void ProgressDialog::SetText(const TCHAR* text) {
   DbgAssert(hWnd!=NULL);
   SetDlgItemText(hWnd, IDC_PROGRESSDLG_STATUS, text);
    ip->CheckMAXMessages();
}
#endif   // !use_original_vertex_paint

//==============================================================================
// class ProgCallback
//
// Progress callback class for the VC evaluation functions
//==============================================================================

class ProgCallback : public EvalColProgressCallback {
public:
   ProgCallback(ApplyVCUtil* util) { u = util; }

#ifndef USE_ORIGINAL_VERTEX_PAINT
   void Begin() { dialog.Begin(); canceled = FALSE; }
   void End() { dialog.End(); }
#endif   // use_original_vertex_paint
    BOOL progress(float prog) { //return TRUE if the user cancelled
        static int p=-1;
        // Eliminate ugly flashing of progress bar by only
        // calling update if the value has changed.
        if (p!=int(prog*100.f)) {
#ifndef USE_ORIGINAL_VERTEX_PAINT
         canceled = dialog.Progress( int(prog*100.f), 100 );
#else // use_original_vertex_paint
            ((ApplyVCUtil*)u)->ip->ProgressUpdate(int(prog*100.0f));
#endif   // use_original_vertex_paint
            p = int(prog*100.0f);
        }
#ifndef USE_ORIGINAL_VERTEX_PAINT
      return canceled;
#else // use_original_vertex_paint
        return ((ApplyVCUtil*)u)->ip->GetCancel();
#endif   // use_original_vertex_paint
    }
#ifndef USE_ORIGINAL_VERTEX_PAINT
   void SetProgressText(const TCHAR* text) {dialog.SetText(text);}
#endif   // use_original_vertex_paint

private:
    UtilityObj* u;
#ifndef USE_ORIGINAL_VERTEX_PAINT
   ProgressDialog dialog;
   BOOL canceled;
#endif   // !use_original_vertex_paint
};

//==============================================================================
// class DerivedObjTab
//==============================================================================
class DerivedObjTab : public Tab<IDerivedObject*> {
public:
    BOOL InList(IDerivedObject *dob) {
        for (int i=0; i<Count(); i++) {
            if ((*this)[i]==dob) return TRUE;
        }
        return FALSE;
    }
};



static ActionTable* FindActionTableFromName(IActionManager* in_actionMgr, LPCTSTR in_tableName)
{
   ActionTable* foundTable = NULL;
   if (in_actionMgr) {
      int nbTables = in_actionMgr->NumActionTables();
      for (int i = 0; i < nbTables; i++) {
         ActionTable* table = in_actionMgr->GetTable(i);
         if (table && _tcscmp(table->GetName(), in_tableName) == 0) {
            foundTable = table;
            break;
         }
      }
   }
   return foundTable;
}

static ActionItem* FindActionItemFromName(ActionTable* in_actionTable, LPCTSTR in_actionName)
{
   ActionItem* foundItem = NULL;
   if (in_actionTable) {
      for (int i = 0; i < in_actionTable->Count(); i++) {
         ActionItem* actionItem = (*in_actionTable)[i];
         TSTR desc;
         if (actionItem) { 
            actionItem->GetDescriptionText(desc);
            if (_tcscmp(desc, in_actionName) == 0) {
               foundItem = actionItem;
               break;
            }
         }
      }
   }
   return foundItem;
}

static INT_PTR CALLBACK ApplyVCDlgProc(
                                       HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#ifndef USE_ORIGINAL_VERTEX_PAINT
   static BOOL initialized = FALSE;
#endif   // !use_original_vertex_paint

    switch (msg) {
    case WM_INITDIALOG:
        theApplyVC.Init(hWnd);
#ifndef USE_ORIGINAL_VERTEX_PAINT
      initialized = TRUE;
#endif   // !use_original_vertex_paint
        break;
        
    case WM_DESTROY:
        theApplyVC.Destroy(hWnd);
#ifndef USE_ORIGINAL_VERTEX_PAINT
      initialized = FALSE;
#endif   // !use_original_vertex_paint
        break;
        
    case WM_COMMAND:
#ifndef USE_ORIGINAL_VERTEX_PAINT
      //First, handle only the commands that require a UI update...
        switch (LOWORD(wParam)) {
      case IDC_VCUTIL_CHAN_COLOR:      theApplyVC.currentOptions.mapChannel = 0; break;
      case IDC_VCUTIL_CHAN_ILLUM:      theApplyVC.currentOptions.mapChannel = -1;   break;
      case IDC_VCUTIL_CHAN_ALPHA:      theApplyVC.currentOptions.mapChannel = -2;   break;
      case IDC_VCUTIL_CHAN_MAP:
      case IDC_VCUTIL_CHAN_MAP_EDIT:
      case IDC_VCUTIL_CHAN_MAP_SPIN:
         theApplyVC.currentOptions.mapChannel = theApplyVC.iMapChanSpin->GetIVal();
         break;
        case IDC_VCUTIL_LIGHTING:      
         theApplyVC.currentOptions.lightingModel = LightingModel::kLightingOnly;    
            theApplyVC.UpdateEnableStates();
         break;
        case IDC_VCUTIL_SHADED:        
         theApplyVC.currentOptions.lightingModel = LightingModel::kShadedLighting;  
            theApplyVC.UpdateEnableStates();
         break;
        case IDC_VCUTIL_DIFFUSE:    
         // Force no radiosity in this case
         theApplyVC.currentOptions.lightingModel = LightingModel::kShadedOnly;      
         CheckRadioButton(hWnd, IDC_VCUTIL_NO_RADIOSITY, 
            IDC_VCUTIL_RADIOSITY_INDIRECTONLY, IDC_VCUTIL_NO_RADIOSITY);
            theApplyVC.UpdateEnableStates();
         break;
      case IDC_VCUTIL_COLORBYFACE:  theApplyVC.currentOptions.mixVertColors = false;   break;
      case IDC_VCUTIL_COLORBYVERT:  theApplyVC.currentOptions.mixVertColors = true;    break;
        case IDC_MIX:               theApplyVC.currentOptions.mixVertColors = (IsDlgButtonChecked(hWnd, IDC_MIX) == BST_CHECKED);         break;
        case IDC_CASTSHADOWS:       theApplyVC.currentOptions.castShadows = (IsDlgButtonChecked(hWnd, IDC_CASTSHADOWS) == BST_CHECKED);      break;
        case IDC_USEMAPS:           theApplyVC.currentOptions.useMaps = (IsDlgButtonChecked(hWnd, IDC_USEMAPS) == BST_CHECKED);           break;
      case IDC_VCUTIL_NO_RADIOSITY: 
         theApplyVC.currentOptions.useRadiosity = false; 
         theApplyVC.currentOptions.radiosityOnly = false;   
         theApplyVC.currentOptions.reuseIllumination = false;
            theApplyVC.UpdateEnableStates();
         break;
      case IDC_VCUTIL_RADIOSITYONLY_REUSEILLUM: 
         theApplyVC.currentOptions.useRadiosity = true;  
         theApplyVC.currentOptions.radiosityOnly = true; 
         theApplyVC.currentOptions.reuseIllumination = true;
            theApplyVC.UpdateEnableStates();
         break;
      case IDC_VCUTIL_RADIOSITY_AND_DIRECT:  
         theApplyVC.currentOptions.useRadiosity = true;  
         theApplyVC.currentOptions.radiosityOnly = false;   
         theApplyVC.currentOptions.reuseIllumination = false;
            theApplyVC.UpdateEnableStates();
         break;
      case IDC_VCUTIL_RADIOSITY_INDIRECTONLY:   
         theApplyVC.currentOptions.useRadiosity = true;  
         theApplyVC.currentOptions.radiosityOnly = true; 
         theApplyVC.currentOptions.reuseIllumination = false;
            theApplyVC.UpdateEnableStates();
         break;
        case IDC_VCUTIL_RADIOSITYSETUP:
         RadiosityInterface* r = static_cast<RadiosityInterface*>(GetCOREInterface(RADIOSITY_INTERFACE));
         if (r != NULL)
            r->OpenRadiosityPanel();
         break;
        }

      //Next, handle commands which do NOT require a UI update
#endif   // !use_original_vertex_paint

      switch (LOWORD(wParam)) {
        case IDC_CLOSEBUTTON:
            theApplyVC.iu->CloseUtility();
            break;
        case IDC_VCUTIL_APPLY:
            theApplyVC.ApplySelected();
            break;
#ifndef USE_ORIGINAL_VERTEX_PAINT
        //case IDC_VCUTIL_UPDATEALL:
        //    theApplyVC.UpdateAll();
        //    break;
      case IDC_VCUTIL_EDITCOLORS:
         theApplyVC.EditColors();
         break;
      default: // For all other commands -> Update the UI
         theApplyVC.UpdateUI();
         break;
#else // use_original_vertex_paint
        case IDC_VCUTIL_UPDATEALL:
            theApplyVC.UpdateAll();
            break;
        case IDC_VCUTIL_LIGHTING:
            theApplyVC.currentOptions.lightingModel = LightingModel::kLightingOnly;
            theApplyVC.UpdateEnableStates();
            break;
        case IDC_VCUTIL_SHADED:
            theApplyVC.currentOptions.lightingModel = LightingModel::kShadedLighting;
            theApplyVC.UpdateEnableStates();
            break;
        case IDC_VCUTIL_DIFFUSE:                  
            theApplyVC.currentOptions.lightingModel = LightingModel::kShadedOnly;
            theApplyVC.UpdateEnableStates();
            break;
        case IDC_MIX:
        case IDC_CASTSHADOWS:
        case IDC_USEMAPS:
            theApplyVC.currentOptions.mixVertColors = (IsDlgButtonChecked(hWnd, IDC_MIX) == BST_CHECKED);
            theApplyVC.currentOptions.castShadows = (IsDlgButtonChecked(hWnd, IDC_CASTSHADOWS) == BST_CHECKED);
            theApplyVC.currentOptions.useMaps = (IsDlgButtonChecked(hWnd, IDC_USEMAPS) == BST_CHECKED);
            theApplyVC.UpdateEnableStates();
            break;
#endif   // use_original_vertex_paint
      }

        break;
#ifndef USE_ORIGINAL_VERTEX_PAINT
   case CC_SPINNER_CHANGE:
      //ignore this message if called before InitUI() is completed
      if( LOWORD(wParam)==IDC_VCUTIL_CHAN_MAP_SPIN && initialized ) {
         theApplyVC.currentOptions.mapChannel = theApplyVC.iMapChanSpin->GetIVal();
         theApplyVC.UpdateUI();
      }
      break;
#endif   // use_original_vertex_paint
        
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        theApplyVC.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
        break;
        
    default:
        return FALSE;
    }
    return TRUE;
}  

void ApplyVCUtil::NotifyRefreshUI(void* param, NotifyInfo* info) {

    ApplyVCUtil* util = static_cast<ApplyVCUtil*>(param);
    DbgAssert(util != NULL);
    if(util != NULL) {
        util->UpdateEnableStates();
      #ifndef USE_ORIGINAL_VERTEX_PAINT
      util->UpdateGatheringControls();
      #endif   // !use_original_vertex_paint
   }
}

ApplyVCUtil::ApplyVCUtil()
#ifndef USE_ORIGINAL_VERTEX_PAINT
:IAssignVertexColors_R7 (
   //Interface descriptor for IAssignVertexColors_R7
   IASSIGNVERTEXCOLORS_R7_INTERFACE_ID, _T("AssignVertexColors"), 0, NULL, FP_CORE,
   ApplyVCUtil::fnIdApplyNodes, _T("ApplyNodes"), 0, TYPE_INT, 0, 2,
      _T("nodes"), 0, TYPE_INODE_TAB,
      _T("vertexPaint"), 0, TYPE_REFTARG, f_keyArgDefault, NULL,
   end )
#endif   // !use_original_vertex_paint
//ApplyVCUtil constructor
{
    iu = NULL;
    ip = NULL; 
    hPanel = NULL;
#ifndef USE_ORIGINAL_VERTEX_PAINT
   recentMod = NULL;
    currentOptions.mapChannel = 0, mapChannelSpin = 3; //map channel spinner defaults to 3
#endif   // !use_original_vertex_paint
    currentOptions.lightingModel = LightingModel::kShadedLighting;
    currentOptions.mixVertColors = false;
    currentOptions.castShadows = false;
    currentOptions.useMaps = false;
    currentOptions.useRadiosity = false;
    currentOptions.radiosityOnly = false;
   #ifndef USE_ORIGINAL_VERTEX_PAINT
   currentOptions.reuseIllumination = false;
   #endif   // !use_original_vertex_paint

    // Register notification callbacks, since we need to update controls if
   // either the radiosity plug-in or renderer changes
    RegisterNotification(NotifyRefreshUI, this, NOTIFY_RADIOSITY_PLUGIN_CHANGED);
    RegisterNotification(NotifyRefreshUI, this, NOTIFY_POST_RENDERER_CHANGE);
}

ApplyVCUtil::~ApplyVCUtil()
{
    UnRegisterNotification(NotifyRefreshUI, this, NOTIFY_RADIOSITY_PLUGIN_CHANGED);
    UnRegisterNotification(NotifyRefreshUI, this, NOTIFY_POST_RENDERER_CHANGE);
}

void ApplyVCUtil::SaveOptions()
{
    TSTR f;
    
    f = ip->GetDir(APP_PLUGCFG_DIR);
    f += _T("\\");
    f += CFGFILE;
    
    FILE* out = fopen(f, "w");
    if (out) {
        fputc(CFGVERSION, out);
        int dummy;
        dummy = currentOptions.lightingModel;
        fprintf(out, "%d\n", dummy);
        dummy = currentOptions.mixVertColors;
        fprintf(out, "%d\n", dummy);
        dummy = currentOptions.castShadows;
        fprintf(out, "%d\n", dummy);
        dummy = currentOptions.useMaps;
        fprintf(out, "%d\n", dummy);
        dummy = currentOptions.useRadiosity;
        fprintf(out, "%d\n", dummy);
        dummy = currentOptions.radiosityOnly;
        fprintf(out, "%d\n", dummy);
   #ifndef USE_ORIGINAL_VERTEX_PAINT
        dummy = currentOptions.reuseIllumination;
        fprintf(out, "%d\n", dummy);
   #endif   // !use_original_vertex_paint
        fclose(out);
    }
}

void ApplyVCUtil::LoadOptions()
{
    TSTR f;
    
    f = ip->GetDir(APP_PLUGCFG_DIR);
    f += _T("\\");
    f += CFGFILE;
    
    FILE* in = fopen(f, "r");
    if (in) {
        int version;
        version = fgetc(in);  // Version
        if(version == CFGVERSION) {
            int dummy;
            fscanf(in, "%d\n", &dummy);
            currentOptions.lightingModel = static_cast<LightingModel>(dummy);
            fscanf(in, "%d\n", &dummy);
            currentOptions.mixVertColors = (dummy != 0);
            fscanf(in, "%d\n", &dummy);
            currentOptions.castShadows = (dummy != 0);
            fscanf(in, "%d\n", &dummy);
            currentOptions.useMaps = (dummy != 0);
            fscanf(in, "%d\n", &dummy);
            currentOptions.useRadiosity = (dummy != 0);
            fscanf(in, "%d\n", &dummy);
            currentOptions.radiosityOnly = (dummy != 0);
            #ifndef USE_ORIGINAL_VERTEX_PAINT
            fscanf(in, "%d\n", &dummy);
            currentOptions.reuseIllumination = (dummy != 0);
            #endif   // !use_original_vertex_paint
        }
        fclose(in);
    }
}

#ifndef USE_ORIGINAL_VERTEX_PAINT
void ApplyVCUtil::GetOptions( IAssignVertexColors::Options& options ) 
{
   options.mapChannel = currentOptions.mapChannel;
   options.mixVertColors = currentOptions.mixVertColors;
   options.castShadows = currentOptions.castShadows;
   options.useMaps = currentOptions.useMaps;
   options.useRadiosity = currentOptions.useRadiosity;
   options.radiosityOnly = currentOptions.radiosityOnly;
   options.lightingModel = currentOptions.lightingModel;
}

void ApplyVCUtil::SetOptions( IAssignVertexColors::Options& options ) 
{
   currentOptions.mapChannel = options.mapChannel;
   currentOptions.mixVertColors = options.mixVertColors;
   currentOptions.castShadows = options.castShadows;
   currentOptions.useMaps = options.useMaps;
   currentOptions.useRadiosity = options.useRadiosity;
   currentOptions.radiosityOnly = options.radiosityOnly;
   currentOptions.lightingModel = options.lightingModel;
   UpdateUI();
}

void ApplyVCUtil::GetOptions2( Options2& options ) 
{
   options = currentOptions;
}

void ApplyVCUtil::SetOptions2( Options2& options ) 
{
   currentOptions = options;
   UpdateUI();
}

BaseInterface* ApplyVCUtil::GetInterface(Interface_ID id)
{
   if (id == IASSIGNVERTEXCOLORS_INTERFACE_ID)
      return this;
   else if (id == IASSIGNVERTEXCOLORS_R7_INTERFACE_ID)
      return this;
   else
   {
      BaseInterface* intf = IAssignVertexColors_R7::GetInterface(id);
      return intf;
   }
}

#endif   // !use_original_vertex_paint

void ApplyVCUtil::BeginEditParams(Interface *ip,IUtil *iu) 
{
    this->iu = iu;
    this->ip = ip;
#ifdef USE_ORIGINAL_VERTEX_PAINT
    hPanel = 
#endif   // use_original_vertex_paint
    ip->AddRollupPage(
        hInstance,
        MAKEINTRESOURCE(IDD_VCUTIL_PANEL),
        ApplyVCDlgProc,
        GetString(IDS_AVCU_PANELTITLE),
        0);
#ifndef USE_ORIGINAL_VERTEX_PAINT
   recentMod=NULL;
#endif   // use_original_vertex_paint
   LoadOptions();
   bool useRadiosity = false;
   RadiosityEffect* radiosity = GetCompatibleRadiosity();
   if (radiosity) {
      RadiosityMesh* radiosityEngine = GetRadiosityMesh(radiosity);
      if (radiosityEngine) 
         useRadiosity = radiosityEngine->DoesSolutionExist();
   }
   currentOptions.useRadiosity = useRadiosity;
   currentOptions.radiosityOnly = useRadiosity;
#ifndef USE_ORIGINAL_VERTEX_PAINT
   currentOptions.reuseIllumination = useRadiosity;
#endif   // use_original_vertex_paint
   UpdateUI();
   UpdateEnableStates();
}

void ApplyVCUtil::EndEditParams(Interface *ip,IUtil *iu) 
{
    SaveOptions();
    
    this->iu = NULL;
    this->ip = NULL;
    ip->DeleteRollupPage(hPanel);
    hPanel = NULL;
}

#ifndef USE_ORIGINAL_VERTEX_PAINT
void ApplyVCUtil::UpdateRenderOptionControls()
{
   if (hPanel) {
      CheckDlgButton(hPanel, IDC_VCUTIL_NO_RADIOSITY, 
         (theApplyVC.currentOptions.useRadiosity)? BST_UNCHECKED : BST_CHECKED);
      CheckDlgButton(hPanel, IDC_VCUTIL_RADIOSITYONLY_REUSEILLUM, 
         (theApplyVC.currentOptions.reuseIllumination)? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hPanel, IDC_VCUTIL_RADIOSITY_AND_DIRECT, 
         (theApplyVC.currentOptions.useRadiosity && !theApplyVC.currentOptions.radiosityOnly)? 
            BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hPanel, IDC_VCUTIL_RADIOSITY_INDIRECTONLY, 
         (theApplyVC.currentOptions.radiosityOnly && !theApplyVC.currentOptions.reuseIllumination)? 
            BST_CHECKED : BST_UNCHECKED);
   }
}


void ApplyVCUtil::UpdateGatheringControls()
{
   if (hPanel) {
        TSTR msg;
      RadiosityEffect* radiosity = GetCompatibleRadiosity();
      if (!radiosity)
         return;

      IRadiosityRenderParameters* radiosityParams = 
         (IRadiosityRenderParameters*)radiosity->GetInterface(
            IRADIOSITY_RENDER_PARAMETERS_INTERFACE);
      if (!radiosityParams)
         return;

      BOOL gather = radiosityParams->GetRegather();
      if (radiosityParams->GetRegather())
         msg = GetString(IDS_AVCU_ENABLED);
      else
         msg = GetString(IDS_AVCU_DISABLED);
        SetDlgItemText(hPanel, IDC_VCUTIL_GATHERING_STATIC, msg.data());
   }
}
#endif

void ApplyVCUtil::Init(HWND hWnd)
{
#ifndef USE_ORIGINAL_VERTEX_PAINT
   hPanel = hWnd;

   HWND hMapChanEdit = GetDlgItem(hWnd,IDC_VCUTIL_CHAN_MAP_EDIT);
   iMapChanEdit = GetICustEdit( hMapChanEdit );
   iMapChanSpin = GetISpinner( GetDlgItem(hWnd,IDC_VCUTIL_CHAN_MAP_SPIN) );
   iMapChanSpin->LinkToEdit( hMapChanEdit, EDITTYPE_POS_INT );
   iMapChanSpin->SetLimits( 1, 99 );
   iMapChanSpin->SetValue( mapChannelSpin, FALSE );
   currentOptions.useRadiosity = true; 
   currentOptions.radiosityOnly = true;   
   currentOptions.reuseIllumination = true;

    TSTR msg = GetString(IDS_AVCU_REMINDER);
    SetDlgItemText(hPanel, IDC_REMINDER_STATIC, msg.data());
    msg = GetString(IDS_AVCU_REGATHERING);
    SetDlgItemText(hPanel, IDC_REGATHER_STATIC, msg.data());
   UpdateRenderOptionControls();
   UpdateGatheringControls();
#endif   // use_original_vertex_paint
}

void ApplyVCUtil::Destroy(HWND hWnd)
{
#ifndef USE_ORIGINAL_VERTEX_PAINT
   mapChannelSpin = iMapChanSpin->GetIVal(); //save for the next time we open the UI
   ReleaseICustEdit( iMapChanEdit );
   ReleaseISpinner( iMapChanSpin );
#endif   // use_original_vertex_paint
}

void ApplyVCUtil::UpdateUI() {
   if( hPanel==NULL ) return;

#ifndef USE_ORIGINAL_VERTEX_PAINT
   int channelRadioButtonID;
    switch( currentOptions.mapChannel ){
    case -2:   channelRadioButtonID = IDC_VCUTIL_CHAN_ALPHA;   break;
    case -1:   channelRadioButtonID = IDC_VCUTIL_CHAN_ILLUM;   break;
    case 0:    channelRadioButtonID = IDC_VCUTIL_CHAN_COLOR;   break;
   default: channelRadioButtonID = IDC_VCUTIL_CHAN_MAP;     break;
    }
   iMapChanEdit->Enable( currentOptions.mapChannel>0 );
   iMapChanSpin->Enable( currentOptions.mapChannel>0 );
   int spinValue = (currentOptions.mapChannel>0? currentOptions.mapChannel : iMapChanSpin->GetIVal());
   if( currentOptions.mapChannel>0 ) iMapChanSpin->SetValue( spinValue, FALSE );
   TSTR chanName;
   if( GetChannelName( spinValue, chanName ) )
      SetDlgItemText( hPanel, IDC_VCUTIL_CHAN_NAME, chanName.data() );
   else SetDlgItemText( hPanel, IDC_VCUTIL_CHAN_NAME, GetString(IDS_MAPNAME_NONE) );
#endif   // use_original_vertex_paint

    int radioButtonID;
    switch(currentOptions.lightingModel){
    case LightingModel::kLightingOnly:
        radioButtonID = IDC_VCUTIL_LIGHTING;
        break;
    case LightingModel::kShadedLighting:
        radioButtonID = IDC_VCUTIL_SHADED;
        break;
    case LightingModel::kShadedOnly:
        radioButtonID = IDC_VCUTIL_DIFFUSE;       
        break;
    }

    CheckRadioButton(hPanel, IDC_VCUTIL_DIFFUSE, IDC_VCUTIL_LIGHTING, radioButtonID);
#ifndef USE_ORIGINAL_VERTEX_PAINT
    CheckRadioButton(hPanel, IDC_VCUTIL_CHAN_COLOR, IDC_VCUTIL_CHAN_MAP, channelRadioButtonID);
   CheckRadioButton(hPanel, IDC_VCUTIL_COLORBYFACE, IDC_VCUTIL_COLORBYVERT, 
      currentOptions.mixVertColors? IDC_VCUTIL_COLORBYVERT : IDC_VCUTIL_COLORBYFACE );
#endif   // use_original_vertex_paint

    CheckDlgButton(hPanel, IDC_MIX, currentOptions.mixVertColors ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hPanel, IDC_CASTSHADOWS, currentOptions.castShadows ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hPanel, IDC_USEMAPS, currentOptions.useMaps ? BST_CHECKED : BST_UNCHECKED);

#ifndef USE_ORIGINAL_VERTEX_PAINT
   //FIXME: the way the state of the Edit button is controlled, is a hack...
   //but keeping of track of the modifier to determine if/when it's still valid would be a lot of overhead
   ICustButton* editBtn = GetICustButton( GetDlgItem(hPanel,IDC_VCUTIL_EDITCOLORS) );
   editBtn->Enable( recentMod!=NULL );
   ReleaseICustButton(editBtn);
   UpdateRenderOptionControls();
   UpdateGatheringControls();
#else // use_original_vertex_paint
    UpdateEnableStates();
#endif   // use_original_vertex_paint
}

void ApplyVCUtil::UpdateEnableStates() {
    
    if (hPanel != NULL) {
      #ifndef USE_ORIGINAL_VERTEX_PAINT
      EnableWindow(GetDlgItem(hPanel, IDC_CASTSHADOWS), 
         theApplyVC.currentOptions.lightingModel != LightingModel::kShadedOnly &&
         (!theApplyVC.currentOptions.useRadiosity ||
          (!theApplyVC.currentOptions.radiosityOnly && !theApplyVC.currentOptions.reuseIllumination)));
      #else  // !use_original_vertex_paint
      EnableWindow(GetDlgItem(hPanel, IDC_CASTSHADOWS), 
         theApplyVC.currentOptions.lightingModel != LightingModel::kShadedOnly &&
         (!theApplyVC.currentOptions.useRadiosity || !theApplyVC.currentOptions.radiosityOnly));
      #endif   // !use_original_vertex_paint
      EnableWindow(GetDlgItem(hPanel, IDC_USEMAPS),
         theApplyVC.currentOptions.lightingModel != LightingModel::kLightingOnly);

      bool enableRadiosity = (GetCompatibleRadiosity() != NULL);
      EnableWindow(GetDlgItem(hPanel, IDC_VCUTIL_RADIOSITYONLY_REUSEILLUM), enableRadiosity &&
         currentOptions.lightingModel != LightingModel::kShadedOnly);
      EnableWindow(GetDlgItem(hPanel, IDC_VCUTIL_REUSE_STATIC), enableRadiosity &&
         currentOptions.lightingModel != LightingModel::kShadedOnly);
      EnableWindow(GetDlgItem(hPanel, IDC_VCUTIL_RADIOSITY_AND_DIRECT), enableRadiosity &&
         currentOptions.lightingModel != LightingModel::kShadedOnly);
      EnableWindow(GetDlgItem(hPanel, IDC_VCUTIL_RENDER_STATIC), enableRadiosity &&
         currentOptions.lightingModel != LightingModel::kShadedOnly);
      EnableWindow(GetDlgItem(hPanel, IDC_VCUTIL_RADIOSITY_INDIRECTONLY), enableRadiosity &&
         currentOptions.lightingModel != LightingModel::kShadedOnly);
      EnableWindow(GetDlgItem(hPanel, IDC_VCUTIL_INDIRECT_STATIC), enableRadiosity &&
         currentOptions.lightingModel != LightingModel::kShadedOnly);
      #ifndef USE_ORIGINAL_VERTEX_PAINT
      EnableWindow(GetDlgItem(hPanel, IDC_REGATHER_STATIC), enableRadiosity &&
         !theApplyVC.currentOptions.reuseIllumination);
      EnableWindow(GetDlgItem(hPanel, IDC_VCUTIL_GATHERING_STATIC), enableRadiosity &&
         !theApplyVC.currentOptions.reuseIllumination);
      // Disable radiosity set-up button if the renderer is mental ray
      Renderer *renderer = GetCOREInterface()->GetProductionRenderer();
      EnableWindow(GetDlgItem(hPanel, IDC_VCUTIL_RADIOSITYSETUP), 
         renderer && renderer->ClassID() != MRRENDERER_CLASSID);
      #endif   // !use_original_vertex_paint
    }
}


//***************************************************************************
//
// Apply / Update selected
//
//***************************************************************************

void ApplyVCUtil::ApplySelected()
{
#ifndef USE_ORIGINAL_VERTEX_PAINT
   Tab<INode*> nodes;
   for (int i=0; i<ip->GetSelNodeCount(); i++) {
      INode* node = ip->GetSelNode(i);
      nodes.Append( 1, &node );
   }

   ApplyNodes( &nodes );

#else // use_original_vertex_paint
    for (int i=0; i<ip->GetSelNodeCount(); i++)
    {
        if (!ApplyNode(ip->GetSelNode(i))) {
            break;
        }
    }
#endif   // use_original_vertex_paint

   ip->RedrawViews(ip->GetTime());
}


DWORD WINAPI dummy(LPVOID arg) 
{
    return(0);
}

#ifndef USE_ORIGINAL_VERTEX_PAINT
int ApplyVCUtil::ApplyNodes( Tab<INode*>* nodes, ReferenceTarget* dest ) 
{
   DbgAssert( nodes!=NULL );
   if (ip==NULL) ip=GetCOREInterface(); //hack

   // Filter out non-geometry objects
   for (int i=0; i < nodes->Count(); i++) {
      INode* node = (*nodes)[i];
      ObjectState os = node->EvalWorldState(ip->GetTime());
      if (os.obj && os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID) {
         nodes->Delete(i, 1);
         i--;
      }
   }

   Modifier* mod=NULL; //modifier to apply to the nodes, if needed
   IVertexPaint_R7* ivertexPaint; //the interface to the modifier

   if( dest==NULL ) {
      mod = (Modifier*)CreateInstance(OSM_CLASS_ID, PAINTLAYERMOD_CLASS_ID);
      if (mod != NULL) {
         ivertexPaint = (IVertexPaint_R7*)mod->GetInterface(IVERTEXPAINT_R7_INTERFACE_ID);
         if (ivertexPaint)
            ivertexPaint->SetOptions2(currentOptions);
      }
   }
   else ivertexPaint = (IVertexPaint_R7*)dest->GetInterface(IVERTEXPAINT_R7_INTERFACE_ID);

   if( ivertexPaint==NULL ) {
      if( mod!=NULL ) mod->DeleteThis();
      return 0;
   }

   //-- Handle radiosity settings
   RadiosityEffect* radiosity = NULL;
   IRadiosityRenderParameters* radiosityParams = NULL;
   bool savedRadiosityRenderMode;
   if( currentOptions.useRadiosity ) {
      // Check if radiosity is available
      radiosity = GetCompatibleRadiosity();
      if( radiosity!=NULL ) {
         radiosityParams = (IRadiosityRenderParameters*)radiosity->GetInterface(
               IRADIOSITY_RENDER_PARAMETERS_INTERFACE);
         if (radiosityParams==NULL )
            return 0;

         // Save radiosity rendering parameter
         savedRadiosityRenderMode = radiosityParams->GetReuseDirectIllumination();
         if (currentOptions.reuseIllumination)
            // Force "Reuse direct illumination mode"
            radiosityParams->SetReuseDirectIllumination(true);
         else
            // Force "Render direct illumination mode"
            radiosityParams->SetReuseDirectIllumination(false);
      }
      else //radiosity==NULL
         return 0;
   }

   theHold.Begin();
   if( mod!=NULL ) { // need to put the vertex paint modifier on the nodes
      AddModifier( *nodes, mod );
      recentMod = mod;
   }

   //-- Apply the vertex colors to each node
   for( int i=0; i<nodes->Count(); i++ ) {
      INode* node = nodes->operator[]( i );
      if( node!=NULL ) {
         BOOL result;
         result = ApplyNode( node, ivertexPaint );
         if (!result)
            break;
      }
   }

   theHold.Accept( GetString(IDS_HOLDMSG_APPLYVTXCOLOR) );

   if (radiosity && currentOptions.useRadiosity)
      radiosityParams->SetReuseDirectIllumination(savedRadiosityRenderMode);

   UpdateUI();
   return 1;
}
#endif   // use_original_vertex_paint
#ifndef USE_ORIGINAL_VERTEX_PAINT
BOOL ApplyVCUtil::ApplyNode(INode* node, IVertexPaint* ivertexPaint, bool noUpdate)
#else // use_original_vertex_paint
BOOL ApplyVCUtil::ApplyNode(INode* node, bool noUpdate)
#endif   // use_original_vertex_paint
{
    ProgCallback fn(this);
    
    if (!node)
        return TRUE;
    
    ObjectState os = node->EvalWorldState(ip->GetTime());
    if (!os.obj)
        return TRUE;
    
    if (os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID)
        return TRUE;
    if (!os.obj->CanConvertToType(triObjectClassID))
        return TRUE;
    
    FaceColorTab faceColors;
    ColorTab mixedVertexColors;
#ifndef USE_ORIGINAL_VERTEX_PAINT
   int result = TRUE;
#endif   // !use_original_vertex_paint
    
    // Update the vertex colors if needed
    if (!noUpdate) {
#ifndef USE_ORIGINAL_VERTEX_PAINT
      theHold.Suspend();
      fn.Begin();
      fn.SetProgressText( (TSTR(GetString(IDS_PROGRESSMSG_RENDERING)) + TSTR(" ") + TSTR( node->GetName() ) + TSTR(_T("..."))).data() );
#else // use_original_vertex_paint
        ip->ProgressStart(GetString(IDS_AVCU_PROGRESS), TRUE, dummy, NULL);
#endif   // use_original_vertex_paint
        
#ifndef USE_ORIGINAL_VERTEX_PAINT
      if (currentOptions.mixVertColors) {
         result = calcMixedVertexColors(node, ip->GetTime(), currentOptions, mixedVertexColors, &fn);
      }
      else {
         result = calcFaceColors(node, ip->GetTime(), currentOptions, faceColors, &fn);
      }
#else // use_original_vertex_paint
        if (currentOptions.mixVertColors) {
            if (!calcMixedVertexColors(node, ip->GetTime(), currentOptions, mixedVertexColors, &fn)) {
                ip->ProgressEnd();
                return FALSE;
            }
        }
        else {
            if (!calcFaceColors(node, ip->GetTime(), currentOptions, faceColors, &fn)) {
                ip->ProgressEnd();
                return FALSE;
            }
        }
#endif   // use_original_vertex_paint
        
#ifndef USE_ORIGINAL_VERTEX_PAINT
      fn.End();
      theHold.Resume();
#else // use_original_vertex_paint
        ip->ProgressEnd();
#endif   // use_original_vertex_paint
    }
    
    
#ifdef USE_ORIGINAL_VERTEX_PAINT
    // Retrieve the modifier, and create it if it does not exist on this node.
    ApplyVCMod* mod;
    if (!(mod = (ApplyVCMod*)GetModifier(node, APPLYVC_MOD_CLASS_ID))) {
        mod = (ApplyVCMod*)CreateInstance(OSM_CLASS_ID, APPLYVC_MOD_CLASS_ID);
        
        Object* obj = node->GetObjectRef();
        IDerivedObject* dobj = CreateDerivedObject(obj);
        dobj->AddModifier(mod);
        node->SetObjectRef(dobj);
    }

    // Update the modifier with the new vertex colors
    if (!noUpdate) {
        if (currentOptions.mixVertColors) {
            mod->SetMixedColors(mixedVertexColors);
        }
        else {
            mod->SetColors(faceColors);
        }
    }
#else // use_original_vertex_paint
   if( !result ) return FALSE;
    
   //If the object to hold the colors has not been specified, create a new Vertex Paint mod
   if( ivertexPaint==NULL ) {    
      // Retrieve the modifier, and create it if it does not exist on this node.
      ApplyVCMod* mod;
	  mod = (ApplyVCMod*)GetModifier(node, PAINTLAYERMOD_CLASS_ID);
      if (!mod) {
         mod = (ApplyVCMod*)CreateInstance(OSM_CLASS_ID, PAINTLAYERMOD_CLASS_ID);
           
         Object* obj = node->GetObjectRef();
         IDerivedObject* dobj = CreateDerivedObject(obj);
         dobj->AddModifier(mod);
         node->SetObjectRef(dobj);
      }   
      IVertexPaint* ivertexPaint = (IVertexPaint*)mod->GetInterface(IVERTEXPAINT_INTERFACE_ID);
   }

   if( ivertexPaint!=NULL ) {
      if (currentOptions.mixVertColors)
      {
         if (mixedVertexColors.Count() > 0)
            ivertexPaint->SetColors( node, mixedVertexColors );
      }
      else
         if (faceColors.Count() > 0)
            ivertexPaint->SetColors( node, faceColors );
   }

   //-- now obsolete - MAB - 05/15/03
#endif   // use_original_vertex_paint
    

    // [dl | 15feb2002] Note: This first loop was commented out, and I can't
    // figure out why. I'm un-commenting it, since it seems necessary to
    // prevent memory leaks.
    int i;
    for (i=0; i<mixedVertexColors.Count(); i++) {
        delete mixedVertexColors[i];
    }
    for (i=0; i<faceColors.Count(); i++) {
        delete faceColors[i];
    }
    
    return TRUE;
}

//***************************************************************************
//
// Update all
//
//***************************************************************************

#ifdef USE_ORIGINAL_VERTEX_PAINT
void ApplyVCUtil::UpdateAll()
{   
    CheckForAndMakeUnique();
    
    for (int i=0; i<ip->GetRootNode()->NumberOfChildren(); i++)
    {
        if (!UpdateAllEnum(ip->GetRootNode()->GetChildNode(i))) {
            break;
        }
    }
    
    ip->RedrawViews(ip->GetTime());
}

BOOL ApplyVCUtil::UpdateAllEnum(INode* node)
{
    for (int i=0; i<node->NumberOfChildren(); i++) {
        if (!UpdateAllEnum(node->GetChildNode(i))) {
            return FALSE;
        }
    }
    
    if (GetModifier(node, APPLYVC_MOD_CLASS_ID)) {
        ApplyNode(node);
    }
    
    return TRUE;
}
#endif   // use_original_vertex_paint

#ifndef USE_ORIGINAL_VERTEX_PAINT
//***************************************************************************
//
// Edit Colors
//
//***************************************************************************

void ApplyVCUtil::EditColors() 
{
   ip->SetCommandPanelTaskMode( TASK_MODE_MODIFY );
   //FIXME: after opening the mod panel, this should also
   //ensure that the recentMod modifier is opened for editing
}
#else // use_original_vertex_paint
//***************************************************************************
//
// Analyze scene for instanced modifiers and instanced nodes sharing the
// modifier
//
//***************************************************************************

void ApplyVCUtil::CheckForAndMakeUnique()
{
    ModInfoTab modTab;
    
    CheckAllNodesEnum(ip->GetRootNode(), modTab);
    
    for (int i=0; i<modTab.Count(); i++) {
        if (modTab[i]->nodes.Count() > 1) {
            MakeUnique(modTab[i]);
        }
        delete modTab[i];
    }
    modTab.ZeroCount();
    modTab.Shrink();
}

void ApplyVCUtil::CheckAllNodesEnum(INode* node, ModInfoTab& modTab)
{
    int i;
    BOOL found = FALSE;
    Modifier* mod = GetModifier(node, APPLYVC_MOD_CLASS_ID);
    if (mod) {
        for (i=0; i<modTab.Count(); i++) {
            if (modTab[i]->mod == mod) {
                modTab[i]->nodes.Append(1, &node, 5);
                found = TRUE;
            }
        }
        if (!found) {
            ModInfo* mh = new ModInfo;
            mh->mod = mod;
            mh->nodes.Append(1, &node, 5);
            modTab.Append(1, &mh, 5);
        }
    }
    int numChildren = node->NumberOfChildren();
    for (i=0; i<numChildren; i++) {
        CheckAllNodesEnum(node->GetChildNode(i), modTab);
    }
}

BOOL ApplyVCUtil::MakeUnique(ModInfo* mh)
{
    int i;
    
    if (!mh) {
        return FALSE;
    }
    
    // Step through all the nodes sharing this modifier  
    // and delete the modifier.
    // The modifier will be applied later when all the nodes
    // are finally updated.
    
    int nodeCount = mh->nodes.Count();
    for (i=0; i<nodeCount; i++) {
        Object* obj = mh->nodes[i]->GetObjectRef();
        
        if (!obj)
            return FALSE;
        
        while (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
            IDerivedObject* dobj = (IDerivedObject*)obj;
            int m;
            int numMods = dobj->NumModifiers();
            
            // This is really only needed for the first node
            // since they all share the same DerivedObject
            for (m=0; m<numMods; m++) {
                Modifier* mod = dobj->GetModifier(m);
                if (mod) {
                    dobj->DeleteModifier(m);
                }
            }
            
            if (dobj->NumModifiers() == 0 && !dobj->TestAFlag(A_DERIVEDOBJ_DONTDELETE)) {
                obj = dobj->GetObjRef();
                obj->TransferReferences(dobj);
                dobj->SetAFlag(A_LOCK_TARGET);
                dobj->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
                obj->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
                dobj->ClearAFlag(A_LOCK_TARGET);
                dobj->MaybeAutoDelete();
            }
            else {
                obj = dobj->GetObjRef();
            }
        }
        
        ApplyNode(mh->nodes[i], true);
    }
    
    return TRUE;
}
#endif   // use_original_vertex_paint


//***************************************************************************
//
// Utility methods
//
//***************************************************************************

// Traverse the pipeline for this node and return the first modifier with a
// specified ClassID

Modifier* ApplyVCUtil::GetModifier(INode* node, Class_ID modCID)
{
    Object* obj = node->GetObjectRef();
    
    if (!obj)
        return NULL;
    
    ObjectState os = node->EvalWorldState(0);
    if (os.obj && os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID) {
        return NULL;
    }
    
    // For all derived objects (can be > 1)
    while (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
        IDerivedObject* dobj = (IDerivedObject*)obj;
        int m;
        int numMods = dobj->NumModifiers();
        // Step through all modififers and verify the class id
        for (m=0; m<numMods; m++) {
            Modifier* mod = dobj->GetModifier(m);
            if (mod) {
                if (mod->ClassID() == modCID) {
                    // Match! Return it
                    return mod;
                }
            }
        }
        obj = dobj->GetObjRef();
    }
    
    return NULL;
}

#ifndef USE_ORIGINAL_VERTEX_PAINT
void ApplyVCUtil::AddModifier(Tab<INode*>& nodes, Modifier* mod) {
   theHold.Begin();

   for( int i=0; i<nodes.Count(); i++ ) {
      if( nodes[i]==NULL ) continue;
      Object* obj = nodes[i]->GetObjectRef();
      IDerivedObject* dobj = CreateDerivedObject(obj);
      //dobj->SetAFlag(A_LOCK_TARGET); //FIXME: needed?
      dobj->AddModifier(mod);
      //dobj->ClearAFlag(A_LOCK_TARGET); //FIXME: needed?
      nodes[i]->SetObjectRef(dobj);
   }

   theHold.Accept( GetString(IDS_HOLDMSG_ADDMODIFIER) );
}

void ApplyVCUtil::DeleteModifier( INode* node, Modifier* mod ) {
   //for( int i=0; i<nodes.Count(); i++ ) {
   Object* obj =  node->GetObjectRef(); //nodes[i]->GetObjectRef();
   if (!obj) return;

   theHold.Begin();

   // For all derived objects (can be > 1)
   while (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
      IDerivedObject* dobj = (IDerivedObject*)obj;
      int numMods = dobj->NumModifiers();
      // Step through all modififers
      for (int j=(numMods-1); j>=0; j--) {
         Modifier* curMod = dobj->GetModifier(j);
         if( curMod==mod )
            dobj->DeleteModifier(j);
      }
      obj = dobj->GetObjRef();
   }
   //}

   theHold.Accept( GetString(IDS_HOLDMSG_DELETEMODIFIER) ); //FIXME: localize
}

BOOL ApplyVCUtil::GetChannelName( int index, TSTR& name ) {
   Interface* ip = GetCOREInterface();
   int nodeCount = ip->GetSelNodeCount();

   TCHAR propKey[16];
   _stprintf(propKey, _T("MapChannel:%i"), index );

   //get the channel name for the first selected obejct
   INode* node;
   BOOL ok = (
      (nodeCount>0) &&
      ((node=ip->GetSelNode(0))!=NULL) &&
      (node->GetUserPropString( propKey, name )) );

   //check the channel name for each other selected object...
   for( int i=1; ok && i<nodeCount; i++ ) {
      TSTR nextName; 
      INode* node = ip->GetSelNode(i);
      ok = node->GetUserPropString( propKey, nextName );
      //...we'll return a default value unless all the objects have a maching channel name
      if( nextName!=name ) ok=FALSE;
   }
   
   return ok;
}
#endif   // !use_original_vertex_paint
