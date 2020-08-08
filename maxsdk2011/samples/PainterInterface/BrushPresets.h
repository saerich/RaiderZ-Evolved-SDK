/**********************************************************************
 *<
   FILE: BrushPresets.h

   DESCRIPTION:   Includes for Plugins

   CREATED BY:

   HISTORY:

 *>   Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __BRUSHPRESETS__H
#define __BRUSHPRESETS__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "utilapi.h"
#include "maxicon.h"
#include "helpsys.h"
#include "notify.h"
#include <Noncopyable.h>
#include "IPainterInterface.h"

#include "IBrushPresets.h"

extern HINSTANCE hInstance;
TCHAR *GetString( int id );
TCHAR *GetString( int id, TCHAR* buf, int bufSize );

class BrushPresetMgr;


//-----------------------------------------------------------------------------
//-- WinImageList

class WinImageList {
   public:
      WinImageList() {width=height=0; imagelist=NULL;}
      ~WinImageList() {Free();}

      void           Init( int width, int height );
      void           Free();

      int               Count();
      int               Width() {return width;}
      int               Height() {return height;}
      HIMAGELIST        GetImageList() {return imagelist;}

      int               AddImages( int n );
      RGBTRIPLE*        GetImage( int i );
      RGBTRIPLE*        GetMask( int i );
      void           UpdateImage( int i );

   protected:
      int               width, height;
      HIMAGELIST        imagelist;
      Tab<LPBITMAPINFO> images, masks;
};

//-----------------------------------------------------------------------------
// class FPValueInterfaceType
// Allows a class to be used as an Interface type FPValue 

class FPValueInterfaceType : public FPInterface {
   public:
      FPValueInterfaceType( Interface_ID id );
      virtual ~FPValueInterfaceType() {}

      BaseInterface*    GetInterface(Interface_ID id);
      Interface_ID      GetID() {return id;}
      FPInterfaceDesc*  GetDesc() {return NULL;}
      LifetimeType      LifetimeControl() { return lifetime; }
      void           SetLifetime( LifetimeType lifetime );

      BaseInterface*    AcquireInterface();  
      void           ReleaseInterface();

   protected:
      Interface_ID      id;
      LifetimeType      lifetime;
      int               refCount;
};

//-----------------------------------------------------------------------------
// class CurveObject

#define CURVEIOBJECT_INTERFACE_ID Interface_ID(0x5a746cd, 0x65004926)

class CurvePts : public FPValueInterfaceType {
   public:
      CurvePts();
      ~CurvePts() {}

      float          GetValue( float x );
      int               GetNumPts() {return curvePts.Count();}
      void           SetNumPts( int count ) {curvePts.SetCount(count);}
      CurvePoint&       GetPoint( int i ) {return curvePts[i];}
      void           SetPoint( int i, CurvePoint& point ) {curvePts[i]=point;}

   protected:
      Tab<CurvePoint>      curvePts;
};

//-----------------------------------------------------------------------------
// class EditControl

class EditControl
{
   public:
      EditControl();
      virtual ~EditControl();
      void           Init(HWND hParent);
      void           Free();

      enum { notifyEditing, notifyEndEdit }; //notify messages
      typedef struct {
         int            message;
         TSTR        editText;
         TCHAR       editChar;
      } NotifyInfo;
      typedef bool (*NotifyProc)( NotifyInfo& info, void* param );

      void           SetNotifyProc( NotifyProc notifyProc, void* param );

      void           BeginEdit(RECT& rect);
      bool           EndEdit(bool accept = true);
      bool           IsEditing();

      HWND           GetHWnd() { return hWnd; }
      TSTR           GetText() { return editText; }
      void           SetText( TSTR text );

   protected:
      enum {null=0, init=1, editing=2} state;
      HWND           hWnd, hParent;
      TSTR           editText, savedText;
      TCHAR          lastChar;
      NotifyProc        notifyProc;
      void*          notifyParam;

      WNDPROC           defaultWndProc;
      bool           OnChar( TCHAR editChar ); //Called from WM_CHAR
      bool           OnNotify( WPARAM wParam, LPARAM lParam ); //Called from WM_NOTIFY
      static LRESULT CALLBACK EditControlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

//-----------------------------------------------------------------------------
// class ListViewCell


class ListViewCell {
   public:
      ListViewCell();
      ~ListViewCell() {Free();}
      void           Zero();
      void           Free();

      BOOL           IsImageType();
      BOOL           IsTextType();

      BOOL           IsEditable();
      void           SetEditable( BOOL editable );

      void           SetImage( int image );
      int               GetImage() {return image;}

      void           SetText( TCHAR* text );
      TCHAR*            GetText() {return text;}

   protected:
      enum {isEditable=1};
      int               flags;
      TCHAR          *text;
      int               image;
};

//-----------------------------------------------------------------------------
// class ListViewRow
// An row for a list view, consisting of a row of cell entries, one entry per column

class ListViewRow {
   public:
      ListViewRow() {}
      ~ListViewRow() {Free();}
      void           Init( int numCells );
      void           Free();
      ListViewCell*     GetCell( int index ) { return &(cells[index]); }
   protected:
      Tab<ListViewCell> cells;
};

//-----------------------------------------------------------------------------
// class ListViewColumn
// A column header for a list view, consisting of a name and width

class ListViewColumn {
   public:
      ListViewColumn();
      ~ListViewColumn() {Free();}
      void           Free();
      TCHAR*            GetName() {return name;}
      void           SetName( TCHAR* name );
      int               GetWidth() {return width;}
      void           SetWidth( int width ) ;
   protected:
      TCHAR*            name;
      int               width;
};

//-----------------------------------------------------------------------------
// class ListView

class ListView {
   public:
      ListView();
      ~ListView() {Free();}
      void           Init( HWND hListView );
      void           Free();

      enum { notifySelCell, notifySelCol, notifyEditing, notifyEndEdit }; //notify messages
      typedef struct {
         int            message;
         int            row, col;
         TSTR        editText;
         TCHAR       editChar;
      } NotifyInfo;
      typedef bool (*NotifyProc)( NotifyInfo& info, void* param );

      void           SetNotifyProc( NotifyProc notifyProc, void* param );

      int               GetNumColumns();
      void           SetNumColumns( int count ); //Should not be called unless the row count is zero
      int               GetSelColumn() {return selCol;}
      void           SetSelColumn( int selCol );
      ListViewColumn*      GetColumn( int index ) { return &(columns[index]); }

      int               GetNumRows();
      void           SetNumRows( int count ); //Should only be called after setting the column count
      int               GetSelRow() {return selRow;}
      void           SetSelRow( int selRow );
      ListViewRow*      GetRow( int index ) { return &(rows[index]); }

      ListViewCell*     GetCell( int row, int col );

      void           SetImageList( HIMAGELIST hImageList );
      HIMAGELIST        GetImageList();

      void           Reset();
      void           Update();
      void           UpdateRow( int row );
      void           UpdateCell( int row, int col );

      void           BeginEdit(); //edit the selected item
      void           EndEdit();
      BOOL           IsEditing();
      //void            DrawItem( LPDRAWITEMSTRUCT lpdis );
      //void            DrawCell( HDC hDC, RECT& rect, int row, int col );

      LRESULT CALLBACK  ProcessMessage( UINT msg, WPARAM wParam, LPARAM lParam );

   protected:
      HWND           hWnd;
      HIMAGELIST        hImageList;
      EditControl       editControl;
      Tab<ListViewColumn>  columns;
      Tab<ListViewRow>  rows;
      int               selRow, selCol;
      NotifyProc        notifyProc;
      void*          notifyParam;

      void           OnCellChange( int row, int col );
      void           OnCellClick( int row, int col );
      void           OnColumnClick( int col );

      WNDPROC           defaultWndProc;
      static bool       EditNotifyProc( EditControl::NotifyInfo& info, void* param );
      static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

//-----------------------------------------------------------------------------
// class CreatePresetDialog

class CreatePresetDialog {
public:
   CreatePresetDialog() {hWnd=NULL;}
   int      DoDialog(); //returns 0 if Cancel, nonzero if OK
   void  EndDialog( int result=TRUE );

   TCHAR*   GetPresetName() {return presetName;}
   void  SetPresetName( TCHAR* name );

   static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
    HWND    hWnd;
   TCHAR    presetName[256];
};


//-----------------------------------------------------------------------------
// class StandinBrushPresetParams

class StandinBrushPresetParams : public IBrushPresetParams {
   public:
      StandinBrushPresetParams( Class_ID contextID, BrushPresetMgr* mgr );
      ~StandinBrushPresetParams();
      Class_ID       ContextID()                            { return contextID; }
      void           ApplyParams()                          {}
      void           FetchParams()                          {}
      int               GetNumParams()                         { return paramList.Count(); }
      int               GetParamID( int paramIndex )              { return paramList[paramIndex]->paramID; }
      int               GetParamIndex( int paramID );
      FPValue           GetParamValue( int paramID )              { return paramList[GetParamIndex(paramID)]->val; }
      void           SetParamValue( int paramID, FPValue val )    { paramList[GetParamIndex(paramID)]->val=val; }
      FPValue           GetDisplayParamValue( int paramID )          { return GetParamValue(paramID); }
      void           SetParent( IBrushPreset* parent )            { this->parent=parent; }

      void           AddParam( int id, FPValue val );
      IBrushPresetParams*  ResolveParams(); //Convert the standin to the real value if possible

   protected:
      BrushPresetMgr*      mgr;
      IBrushPreset*     parent;
      Class_ID       contextID;

      struct ParamItem {
         int            paramID;
         FPValue        val;
      };
      Tab<ParamItem*>         paramList;
};

//-----------------------------------------------------------------------------
// class StdBrushPresetParams

#define STDBRUSHPRESETCONTEXT_ID Class_ID(0,0x5fb1707c)


class StdBrushPresetParams : public IBrushPresetParams  {
   public:
      StdBrushPresetParams();
      ~StdBrushPresetParams();
      Class_ID       ContextID() { return STDBRUSHPRESETCONTEXT_ID; }

      void           ApplyParams();
      void           FetchParams();

      int               GetNumParams();
      int               GetParamID( int paramIndex );
      int               GetParamIndex( int paramID );

      BOOL           GetParamToggle( int paramID );
      void           SetParamToggle( int paramID, BOOL onOff );
      FPValue           GetParamValue( int paramID );
      void           SetParamValue( int paramID, FPValue val );
      FPValue           GetDisplayParamValue( int paramID );

      void           SetParent( IBrushPreset* parent );

      TCHAR*            GetName();
      void           SetName( TCHAR* name );

      HIMAGELIST        GetIcon();
      void           UpdateIcon( float iconMin, float iconMax );

   protected:
      int               parentID;
      BitArray       toggles;

      TCHAR          name[256];
      float          minSize, maxSize, minStr, maxStr;
      BOOL           pressureSenseOn, mirrorOn;
      int               pressureAffects, mirrorAxis;
      float          mirrorOffset, mirrorGizmoSize;

      CurvePts       falloffCurve;
      WinImageList      icon;
};


//-----------------------------------------------------------------------------
// class StandinBrushPresetParams

class StandinBrushPresetContext : public IBrushPresetContext {
   public:
      StandinBrushPresetContext( Class_ID contextID, TCHAR* name,
         Class_ID pluginClassID, SClass_ID pluginSClassID );
	 virtual ~StandinBrushPresetContext();
      Class_ID             ContextID() {return contextID;}
      TCHAR*            GetContextName() {return name.data();}

      IBrushPresetParams*  CreateParams() {return NULL;}
      void           DeleteParams( IBrushPresetParams* params ) {}

      // Not supported by the standin context; query standin params instead
      int               GetNumParams()             {return 0;}
      int               GetParamID( int paramIndex )  {return 0;}
      int               GetParamIndex( int paramID )  {return -1;}
      TCHAR*            GetParamName( int paramID )      {return NULL;}
      ParamType2        GetParamType( int paramID )      {return TYPE_VOID;}

      Class_ID       PluginClassID() {return pluginClassID;}
      SClass_ID         PluginSuperClassID() {return pluginSClassID;}

   protected:
      Class_ID       contextID;
      Class_ID       pluginClassID;
      SClass_ID         pluginSClassID;
      TSTR           name;
};

//-----------------------------------------------------------------------------
// class StdBrushPresetContext

class StdBrushPresetContext : public IBrushPresetContext, public MaxSDK::Util::Noncopyable {
   public:
      void           Init();

      Class_ID       ContextID() { return STDBRUSHPRESETCONTEXT_ID; }
      TCHAR*            GetContextName() { return contextName; }

      IBrushPresetParams* CreateParams() { return new StdBrushPresetParams; }
      void           DeleteParams( IBrushPresetParams* params );

      int               GetNumParams();
      int               GetParamID( int paramindex );
      int               GetParamIndex( int paramID );
      TCHAR*            GetParamName( int paramID );
      ParamType2        GetParamType( int paramID );

      int               GetNumDisplayParams();
      int               GetDisplayParamID( int displayIndex );
      int               GetDisplayParamIndex( int paramID );

      BOOL           IsDisplayParam( int paramID );
      BOOL           IsTransientParam( int paramID );

      Class_ID       PluginClassID() {return PAINTERINTERFACE_CLASS_ID;}
      SClass_ID         PluginSuperClassID() {return PAINTERINTERFACE_SUPERCLASS_ID;}

   protected:
      struct ParamDesc {
         ParamType2  type;
         int         nameResID;
         TCHAR    nameStr[256];
         int         displayIndex;
      };
      TCHAR          contextName[256];
      int               paramCount, displayCount;
      static ParamDesc  paramDescs[];
      static ParamDesc* GetParamDescs() {return paramDescs;}

   // Singleton
   protected:
      StdBrushPresetContext();
			~StdBrushPresetContext();
   private:
      static StdBrushPresetContext* mInstance;
   public:
      static StdBrushPresetContext* GetInstance();
      static void DestroyInstance();
};

StdBrushPresetContext* GetStdBrushPresetContext();


//-----------------------------------------------------------------------------
// class BrushPreset

class StdBrushPreset : public IBrushPreset {
   public:
      StdBrushPreset( int presetID, BrushPresetMgr* mgr );
      ~StdBrushPreset();
      int               PresetID() {return presetID;}

      void           Apply(); // Calls ApplyParams() on every param set
      void           Fetch(); // Calls FetchParams() on every param set

      IBrushPresetParams* GetParams( Class_ID contextID ); // returns params or null, attempting to resolve
      int               GetNumContexts() { return paramsList.Count(); }
      Class_ID       GetContextID( int contextIndex );
      int               GetContextIndex( Class_ID contextID );

      void           Copy( StdBrushPreset* src );
      StandinBrushPresetParams* CreateStandinParams( Class_ID contextID );

      IBrushPresetParams* GetParams( int index ); // returns params or standin, no resolve
      int               AddParams( IBrushPresetParams* params );
      int               AddParams( StandinBrushPresetParams* standin );
      void           RemoveParams( int index );
      BOOL           IsStandinParams( int index );
      BOOL           ResolveParams( int index );

   protected:
      BrushPresetMgr*   mgr;
      int               presetID;

      struct ParamsItem {
         ParamsItem( Class_ID contextID );
         Class_ID    contextID;
         BOOL        isStandin, isFinal;
         IBrushPresetParams* params;
      };
      Tab<ParamsItem>      paramsList;
};

//-----------------------------------------------------------------------------
// class BrushPresetMgr

class BrushPresetMgr : public IBrushPresetMgr, public MaxSDK::Util::Noncopyable {
   public:
      void           Init( IPainterInterface_V5* painterInterface );
      void           Reset();

      // IBrushPresetMgr methods

      void           RegisterContext( IBrushPresetContext* context );
      void           UnRegisterContext( IBrushPresetContext* context );

      IBrushPresetContext* GetContext( Class_ID contextID );
      int               GetNumContexts() {return contextList.Count();}
      Class_ID       GetContextID( int contextIndex );
      int               GetContextIndex( Class_ID contextID );
      int               BeginContext( Class_ID contextID );
      int               EndContext( Class_ID contextID );
      BOOL           IsContextActive( Class_ID contextID );

      IBrushPreset*     GetPreset( int presetID );
      IBrushPreset*     CreatePreset();
      void           DeletePreset( IBrushPreset* preset );
      int               GetNumPresets() {return presets.Count();}
      int               GetPresetID( int presetIndex );
      int               GetPresetIndex( int presetID );

      int               GetActivePreset();
      void           SetActivePreset( int presetID );
      void           ApplyPreset( int presetID );

      BOOL           GetIgnoreUpdates() {return (ignoreUpdates>0? TRUE:FALSE);}
      void           SetIgnoreUpdates( BOOL b ) {(b? ignoreUpdates++ : ignoreUpdates--);}
      void           OnContextUpdated( Class_ID contextID );
      void           OnPresetIconUpdated( int presetID );

      int               ReadConfig(TCHAR *cfg=NULL);
      int               WriteConfig(TCHAR *cfg=NULL);

      // BrushPresetMgr methods

      //int             GetFocusPresetID();
      //void            SetFocusPresetID( int presetID );
      Class_ID       GetFocusContextID();
      void           SetFocusContextID( Class_ID contextID );

      IBrushPresetContext* GetContext( int index ); // returns context or standin, no resolve
      int               AddContext( IBrushPresetContext* context );
      int               AddContext( StandinBrushPresetContext* standin );
      void           RemoveContext( int index );
      BOOL           IsStandinContext( int index );
      BOOL           ResolveStandinContext( int index );

      void           ShowToolbar();
      void           HideToolbar();
      BOOL           IsToolbarVisible();

      void           ShowDialog();
      void           HideDialog();
      BOOL           IsDialogVisible();
      void           SetDialogSize( int width, int height );
      void           SetDialogPos( int x, int y, HWND relativeTo=NULL );

   protected:

      // Class data
      struct ContextItem {
         ContextItem( Class_ID contextID );
         Class_ID    contextID;
         BOOL        isStandin, isFinal;
         IBrushPresetContext* context;
      };
      Tab<ContextItem>   contextList;
      Tab<StdBrushPreset*> presets;

      BOOL           initialized;
      int               activePresetID;
      BitArray       activeContexts;
      //int             focusPresetID, focusPresetIndex;
      Tab<Class_ID>     focusContexts;
      Tab<int>       presetIndices;
      int               ignoreUpdates;

      // UI related data
      HWND           hToolWindow, hToolbar, hDialog;
      int               dialogPosX, dialogPosY, dialogWidth, dialogHeight;
      ListView       listView;
      WinImageList      listViewIcons;
      float          iconMin, iconMax;
      ISpinnerControl      *iconMinSpinner, *iconMaxSpinner;
      MaxBmpFileIcon    *icon1, *icon2, *icon3;
      TSTR           promptFileName, startFileName, defaultFileName;
      WNDPROC           defaultToolbarWndProc;

      // Internal methods
      int               ReadPresets(TCHAR* cfg);
      int               WritePresets(TCHAR* cfg);
      IBrushPreset*     CreatePreset( IBrushPreset* src );
      int               CreatePresetID();
      TSTR           CreatePresetName( IBrushPreset* src );
      void           UpdatePresetIndices();
      StdBrushPreset*      GetStdPreset( int presetID );
      StdBrushPreset*      CreateStdPreset( int presetID );
      StandinBrushPresetContext* CreateStandinContext( Class_ID contextID, TCHAR* contextName,
                        Class_ID pluginClassID, SClass_ID pluginSClassID );

      int               ReadToolbar(TCHAR* cfg);
      int               WriteToolbar(TCHAR* cfg);

      enum {updateToolbar_Icons=1, updateToolbar_Toggles=2, updateToolbar_Size=4,
           updateToolbar_All=7};
      void           UpdateToolbar( int updateChannels = updateToolbar_All );
      void           UpdateToolbarSize();
      void           UpdateToolbarIcon( int presetID );
      void           UpdateToolbarToggles();

      void           AddPresetButton( int presetID );
      void           RemovePresetButton( int presetID );
      HWND           GetPresetButton( int presetID );
      int               GetPresetToolID( int presetID );
      int               GetToolPresetID( int toolID );

      enum {promptFileRead, promptFileWrite};
      BOOL           PromptFileName( TCHAR* buf, int promptType );
      BOOL           GetReadFileName( TCHAR* buf );   //file to load from
      BOOL           GetWriteFileName( TCHAR* buf );  //file to save into

      enum {updateDialog_ListView=1, updateDialog_ListViewSel=2,
           updateDialog_ComboBox=4, updateDialog_Spinners=8,
           updateDialog_All=15};
      void           OpenDialog();
      void           UpdateDialog( int updateChannels = updateDialog_All );
      void           UpdateDialogItem( int presetID );

      // Event handlers
      void           OnPresetMgrBtn();
      void           OnLoadPresetsBtn();
      void           OnSavePresetsBtn();
      void           OnAddPresetBtn();
      void           OnDuplicatePresetBtn();
      void           OnRemovePresetBtn();
      void           OnApplyPresetBtn( int toolID );
      void           OnSpinnerChange();
      void           OnInitDialog(HWND hWnd);
      void           OnDestroyDialog(HWND hWnd);
      void           OnDialogResized();
      void           OnDialogMoved();

      static void       OnSystemStartup(void *param, NotifyInfo *info);
      static void       OnSystemShutdown(void *param, NotifyInfo *info);
      static void       OnColorChange(void *param, NotifyInfo *info);
      static void       OnUnitsChange(void *param, NotifyInfo *info);
      static void       OnToolbarsPreLoad(void *param, NotifyInfo *info);
      static void       OnToolbarsPostLoad(void *param, NotifyInfo *info);

      // Function publishing
      BaseInterface*    GetInterface(Interface_ID id);
      BOOL           fnIsActive();
      void           fnOpenPresetMgr();
      void           fnAddPreset();
      void           fnLoadPresetFile( TCHAR* file );
      void           fnSavePresetFile( TCHAR* file );

      enum {
         fnIdIsActive,
         fnIdOpenPresetMgr, fnIdAddPreset,
         fnIdLoadPresetFile, fnIdSavePresetFile,
      };
      BEGIN_FUNCTION_MAP
         FN_0(  fnIdIsActive, TYPE_BOOL, fnIsActive );
         VFN_0( fnIdOpenPresetMgr, fnOpenPresetMgr );
         VFN_0( fnIdAddPreset, fnAddPreset );
         VFN_1( fnIdLoadPresetFile, fnLoadPresetFile, TYPE_FILENAME );
         VFN_1( fnIdSavePresetFile, fnSavePresetFile, TYPE_FILENAME );
      END_FUNCTION_MAP

      // Statics
      static bool       ListViewNotifyProc( ListView::NotifyInfo& info, void* param );
      static LRESULT CALLBACK    ToolbarWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
      static INT_PTR CALLBACK    DialogWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

    // Singleton
   protected:
      BrushPresetMgr();
      ~BrushPresetMgr();
   public:
      static BrushPresetMgr* GetInstance();
      static void DestroyInstance();
   private:
      static BrushPresetMgr* mInstance;
};

BrushPresetMgr* GetBrushPresetMgr();

void OpenBrushPresetMgrDlg();

#endif // __BRUSHPRESETS__H
