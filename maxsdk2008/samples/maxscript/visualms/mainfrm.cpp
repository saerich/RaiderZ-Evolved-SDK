// ============================================================================
// MainFrm.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "VisualMS.h"
#include "VisualMSThread.h"
#include "VisualMSDoc.h"
#include "MainFrm.h"
#include "PropView.h"
#include "EventView.h"

// from the max sdk
#include "Max.h"
#include "iColorMan.h"

#include "3dsmaxport.h"

#pragma warning(push)
#pragma warning(disable : 4800)   // forcing value to bool 'true' or 'false'


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ============================================================================
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
   //{{AFX_MSG_MAP(CMainFrame)
   ON_WM_CREATE()
   ON_COMMAND(ID_TOGGLE_GRID, OnViewGrid)
   ON_WM_GETMINMAXINFO()
   ON_WM_CLOSE()
   ON_WM_ACTIVATE()
   ON_COMMAND(ID_FILE_EXIT, OnFileExit)
   ON_WM_DESTROY()
   //}}AFX_MSG_MAP
   ON_COMMAND_RANGE(ID_CTRL_SELECT, ID_CTRL_END, OnChangeCreateMode)
END_MESSAGE_MAP()


// ============================================================================
static UINT g_indicators[] =
{
   ID_SEPARATOR,
   ID_SEPARATOR,
   ID_SEPARATOR,
};

// ============================================================================
CMainFrame* CMainFrame::GetFrame()
{
   CVisualMSThread *pThread = CVisualMSThread::GetRunningThread();
   if(pThread) return pThread->GetFrame();
   return NULL;
}

// ============================================================================
HWND CMainFrame::GetFrameHWnd(HWND hWnd)
{
   while(hWnd != NULL)
   {
      if(::GetProp(hWnd, frameWndAtomID))
         return hWnd;
      hWnd = ::GetParent(hWnd);
   }

   return NULL;
}


// ============================================================================
CMainFrame::CMainFrame() {}
CMainFrame::~CMainFrame() {}

// ============================================================================
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if(CFrameWnd::OnCreate(lpCreateStruct) == -1)
      return -1;
   if(CreateStatusBar() == -1)
      return -1;
   if(CreatePropBar() == -1)
      return -1;
   if(CreateMainToolBar() == -1)
      return -1;
   if(CreateCtrlToolBar() == -1)
      return -1;

   EnableDocking(CBRS_ALIGN_ANY);
   DockControlBar(&m_mainToolBar, AFX_IDW_DOCKBAR_TOP);
   DockControlBar(&m_ctrlToolBar, AFX_IDW_DOCKBAR_BOTTOM);
   DockControlBar(&m_propBar);//, AFX_IDW_DOCKBAR_RIGHT);

   // Set the background color for toolbar dock panels
   CControlBar* pTopDockBar = GetControlBar(AFX_IDW_DOCKBAR_TOP);
   HWND hBar = pTopDockBar->GetSafeHwnd();
   ::SetClassLongPtr(
      hBar,
      GCLP_HBRBACKGROUND,
      (LONG_PTR)GetCustSysColorBrush(COLOR_BTNFACE));

   // Set the main window icon to match MAX's icon.
   SetIcon(DLGetClassLongPtr<HICON>(GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM), FALSE);

   // Add an atom identifier to the frame windows HWND for use with GetFrameHWnd
   ::SetProp(GetSafeHwnd(), frameWndAtomID, (HANDLE)TRUE);

   LoadWindowPosition(_T("MainFrame"), GetSafeHwnd());

   return 0;
}

// ============================================================================
void CMainFrame::OnDestroy() 
{
   SaveWindowPosition(_T("MainFrame"), GetSafeHwnd());

   CFrameWnd::OnDestroy();

   ::RemoveProp(GetSafeHwnd(), frameWndAtomID);
}

// ============================================================================
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
   return CFrameWnd::OnCreateClient(lpcs, pContext);
}

// ============================================================================
int CMainFrame::CreateMainToolBar()
{
   if(!m_mainToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE |
      CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
      !m_mainToolBar.LoadToolBar(MAKEINTRESOURCE(IDR_MAINFRAME)))
   {
      TRACE0("Failed to create main toolbar\n");
      return -1;
   }

   m_mainToolBar.SetButtonStyle(m_mainToolBar.CommandToIndex(ID_TOGGLE_GRID), TBBS_CHECKBOX);
   m_mainToolBar.EnableDocking(CBRS_ALIGN_ANY);

   return 0;
}

// ============================================================================
int CMainFrame::CreateCtrlToolBar()
{
   if(!m_ctrlToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | 
      CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
      !m_ctrlToolBar.LoadToolBar(MAKEINTRESOURCE(IDR_CONTROLS)))
   {
      TRACE0("Failed to create controls toolbar\n");
      return -1;      // fail to create
   }

   for(int i = 0; i < m_ctrlToolBar.GetCount(); i++)
      m_ctrlToolBar.SetButtonStyle(i, TBBS_CHECKGROUP);

   m_ctrlToolBar.EnableDocking(CBRS_ALIGN_ANY);

   return 0;
}

// ============================================================================
int CMainFrame::CreatePropBar()
{
   if(!m_propBar.Create(this, IDD_PROPBAR, CBRS_RIGHT | CBRS_SIZE_DYNAMIC, IDD_PROPBAR))
   {
      TRACE0("Failed to create properties bar\n");
      return -1;
   }
   m_propBar.SetBarStyle(m_propBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
   m_propBar.EnableDocking(CBRS_ALIGN_RIGHT | CBRS_ALIGN_LEFT);

   CDocument *pDoc = CVisualMSDoc::GetDoc();
   ASSERT_VALID(pDoc);

   m_propBar.AddView(GetString(IDS_PROPERTIES), RUNTIME_CLASS(CPropView), pDoc);
   m_propBar.AddView(GetString(IDS_EVENTS), RUNTIME_CLASS(CEventView), pDoc);
   m_propBar.SelectTab(0);

   return 0;
}

// ============================================================================
int CMainFrame::CreateStatusBar()
{
   if(!m_statusBar.Create(this) ||
      !m_statusBar.SetIndicators(g_indicators,
        sizeof(g_indicators)/sizeof(UINT)))
   {
      TRACE0("Failed to create status bar\n");
      return -1;
   }

   m_statusBar.GetStatusBarCtrl().SetBkColor(GetCustSysColor(COLOR_BTNFACE));

   UINT nID, nStyle;
   int cxWidth;
   m_statusBar.GetPaneInfo(0, nID, nStyle, cxWidth);
   m_statusBar.SetPaneInfo(0, nID, nStyle, 100);
   m_statusBar.GetPaneInfo(1, nID, nStyle, cxWidth);
   m_statusBar.SetPaneInfo(1, nID, nStyle, 100);
   m_statusBar.GetPaneInfo(2, nID, nStyle, cxWidth);
   m_statusBar.SetPaneInfo(2, nID, nStyle, 100);

   // TODO: is this a resource leak ?
   HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_POSITION), IMAGE_ICON, 16, 16, 0);
   m_statusBar.GetStatusBarCtrl().SetIcon(1, hIcon);
   hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SIZE), IMAGE_ICON, 16, 16, 0);
   m_statusBar.GetStatusBarCtrl().SetIcon(2, hIcon);

   return 0;
}

// ============================================================================
void CMainFrame::PressCtrlButton(UINT nID)
{
   m_ctrlToolBar.GetToolBarCtrl().CheckButton(nID, TRUE);
}

// ============================================================================
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
   if(!CFrameWnd::PreCreateWindow(cs))
      return FALSE;

   // TODO: DONT HARD CODE THIS
   cs.cx = 620;
   cs.cy = 520;

   return TRUE;
}

// ============================================================================
#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
   CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
   CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// ============================================================================
void CMainFrame::OnViewGrid()
{
   CVisualMSDoc *pDoc = (CVisualMSDoc*)GetActiveDocument();
   ASSERT_VALID(pDoc);

   bool isPressed = m_mainToolBar.GetToolBarCtrl().IsButtonChecked(ID_TOGGLE_GRID);
   pDoc->SetUseGrid(isPressed);
}

// ============================================================================
void CMainFrame::OnChangeCreateMode(UINT nID)
{
   CVisualMSDoc *pDoc = (CVisualMSDoc*)GetActiveDocument();
   ASSERT_VALID(pDoc);
   pDoc->SetCreateMode(nID);
}

// ============================================================================
void CMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
   lpMMI->ptMinTrackSize.x = 320;
   lpMMI->ptMinTrackSize.y = 240;
   
   CFrameWnd::OnGetMinMaxInfo(lpMMI);
}

// ============================================================================
LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
   switch(message)
   {
   case WM_SETSTATUS:
      m_statusBar.SetPaneText((int)wParam, (TCHAR*)lParam);
      break;

   case WM_CREATEOBJ:
      {
         CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
         CFormEd *formEd = &pDoc->GetFormObj()->m_formEd;
         CRect *pRect = (CRect*)lParam;

         return (LRESULT)CGuiObj::Create(wParam, formEd, *pRect);
      }
      break;

   case WM_PROPCHANGE:
      {
         CGuiObj *pObj = (CGuiObj*)wParam;
         pObj->OnPropChange((int)lParam);
      }
      break;

   case WM_UPDATEVIEWS:
      {
         CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
         pDoc->UpdateAllViews(NULL, lParam, (CObject*)wParam);
      }
      break;

   default:
      return CFrameWnd::WindowProc(message, wParam, lParam);
   }

   return 0;
}

// ============================================================================
void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
   CFrameWnd::OnActivate(nState, pWndOther, bMinimized);

/* old code - this was fixed when multi-threaded VMS was implemented
   if(nState == WA_INACTIVE)
   {
      GetApp()->StopMsgPump();
      EnableAccelerators();
   }
   else
   {
      DisableAccelerators();
      GetApp()->StartMsgPump();
   }
*/
}

// ============================================================================
void CMainFrame::RecalcLayout(BOOL bNotify) 
{
   CFrameWnd::RecalcLayout(bNotify);
   CFrameWnd::RecalcLayout(bNotify);
}

// ============================================================================
void CMainFrame::OnClose()
{
   OnFileExit();
}

// ============================================================================
void CMainFrame::OnFileExit() 
{
   CVisualMSDoc *pDoc = CVisualMSDoc::GetDoc();
   if(pDoc)
   {
      if(!pDoc->SaveModified())
         return;

      pDoc->OnCloseDocument();
   }

   HWND hMainWnd = GetSafeHwnd();
   if(::IsWindow(hMainWnd))
   {
      ShowWindow(SW_HIDE);
      Interface *ip = GetCOREInterface();
      DLSetWindowLongPtr(hMainWnd, NULL, GWLP_HWNDPARENT);
      ip->UnRegisterDlgWnd(hMainWnd);
   }

   ::PostQuitMessage(0);
}

#pragma warning(pop)


