/**********************************************************************
 *<
   FILE: BrushPresets.cpp

   DESCRIPTION:   Appwizard generated plugin

   CREATED BY: 

   HISTORY: 

 *>   Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#include "BrushPresets.h"

#include "3dsmaxport.h"


BOOL Clip( RECT& dest, const RECT& src ) {
   if( dest.bottom>src.bottom )  dest.bottom = src.bottom;
   if( dest.right>src.right )    dest.right = src.right;
   if( dest.top<src.top )        dest.top = src.top;
   if( dest.left<src.left )      dest.left = src.left;
   return ((dest.top<=dest.bottom) && (dest.left<=dest.right)? TRUE : FALSE);
}


//-----------------------------------------------------------------------------
// class EditControl

EditControl::EditControl()
{
   hWnd = hParent = NULL;
   defaultWndProc = NULL;
   notifyProc = NULL;
   notifyParam = NULL;
   state = null;

   //hiLite = false;
}

EditControl::~EditControl()
{
   if (IsEditing())
      EndEdit(false);

   if (hWnd!=NULL)
   {
      DestroyWindow(hWnd);
      hWnd = NULL;
   }
}

void EditControl::Init(HWND hParent)
{
   Free();
   
   this->hParent = hParent;
   state = init;
}

void EditControl::Free() {
   hParent = NULL;
   state = null;
   if( hWnd!=NULL )DestroyWindow( hWnd );
   hWnd = NULL;
}

void EditControl::SetNotifyProc( NotifyProc notifyProc, void* param ) {
   this->notifyProc = notifyProc;
   this->notifyParam = param;
}

void EditControl::BeginEdit(RECT & rect)
{
   if (hParent==NULL)
      return;

   savedText = editText;
   int rectWidth = rect.right-rect.left, rectHeight = rect.bottom-rect.top;

   if (hWnd == NULL)
   {
      hWnd = CreateWindow(_T("EDIT"), "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT,
                     rect.left, rect.top, rectWidth, rectHeight,
                     hParent, NULL, hInstance, this);

      defaultWndProc = DLSetWindowLongPtr( hWnd, EditControlProc);
      DLSetWindowLongPtr( hWnd, this);
      SendMessage( hWnd, WM_SETFONT, (WPARAM)SendMessage(hParent, WM_GETFONT, 0, 0), MAKELPARAM(TRUE, 0) );
   }
   else
   {
      SetParent( hWnd, hParent );
      DLSetWindowLongPtr( hWnd, EditControlProc);
      SetWindowPos( hWnd, NULL, rect.left, rect.top, rectWidth, rectHeight, SWP_NOACTIVATE );
   }

   SetWindowText( hWnd, editText );
   SendMessage( hWnd, EM_SETSEL, 0, -1 );
   ShowWindow( hWnd, SW_SHOW );
   SetFocus( hWnd );

   state = editing;
}

bool EditControl::EndEdit(bool accept)
{
   if( !IsEditing() || hParent==NULL || hWnd==NULL )
      return true;

   // Give the notifyProc a change to reject the EndEdit() if it finds a problem with the text
   NotifyInfo notifyInfo;
   notifyInfo.message = notifyEndEdit;
   notifyInfo.editText = editText, notifyInfo.editChar = lastChar;
   if( accept && notifyProc!=NULL ) {
      if( !notifyProc( notifyInfo, notifyParam ) )
         return false; //rejected
   }

   if ( !accept )
   {
      editText = savedText;
      InvalidateRect( hWnd, NULL, TRUE);
      UpdateWindow( hWnd );
   }
   else {
      savedText = editText;
   }

   //FIXME !! This triggers a WM_KILLFOCUS, which causes another EndEdit() call
   ShowWindow( hWnd, SW_HIDE);
   DLSetWindowLongPtr(hWnd, defaultWndProc);
   state = init;

   return true;
}

bool EditControl::IsEditing() {
   return (state == editing);
}

void EditControl::SetText( TSTR text ) {
   editText=text;
   if( editText.isNull() ) editText = _T("");
}

bool EditControl::OnChar( TCHAR editChar )
{
   lastChar = editChar;

   NotifyInfo notifyInfo;
   notifyInfo.message = notifyEditing;
   notifyInfo.editText = editText, notifyInfo.editChar = editChar;
   if( (notifyProc!=NULL) && !notifyProc( notifyInfo, notifyParam ) )
      return false;

   return true;
}

bool EditControl::OnNotify(WPARAM wParam, LPARAM lParam)
{
   // I don't think this method is ever called ... let's check ...
   DbgAssert(FALSE);

   WORD notifyCode = HIWORD(wParam);

   if (notifyCode == EN_KILLFOCUS)
   {
      if (hParent==NULL || hWnd==NULL || GetFocus()!=hWnd) // || IsRetKey()
         return false;

      if (IsEditing()) // && !m_readyToShow
         EndEdit(true);
   }
   return false;
}

LRESULT CALLBACK EditControl::EditControlProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   EditControl * parent = DLGetWindowLongPtr<EditControl *>( hWnd);

   switch (msg)
   {
   case WM_SETFOCUS:
      DisableAccelerators();
      break;

   case WM_KILLFOCUS:
      //parent->retKey = true;
      if (!parent->EndEdit(true))
      {
         SetFocus( hWnd );
         SendMessage( hWnd, EM_SETSEL, 0, -1 );
      }
      //parent->retKey = false;
      EnableAccelerators();
      break;

   case WM_GETDLGCODE:
      return DLGC_WANTARROWS | DLGC_HASSETSEL | DLGC_WANTCHARS | DLGC_WANTALLKEYS;
      break;

   case WM_CHAR:
      if( !parent->OnChar( wParam ) )
         return FALSE;
      break;

   case WM_KEYUP:
      parent->editText.Resize(255);
      GetWindowText( hWnd, parent->editText, 255 );

      if( wParam == VK_ESCAPE )
      {
         parent->EndEdit(false);
         return FALSE;
      }
      else if( wParam == VK_RETURN )
         parent->EndEdit(true);

      break;

   case WM_NOTIFY:
      if (hWnd == parent->hWnd)
         parent->OnNotify(wParam, lParam);
      break;
   }

   return CallWindowProc(parent->defaultWndProc, hWnd, msg, wParam, lParam);
}


//-----------------------------------------------------------------------------
// class ListViewCell

ListViewCell::ListViewCell() {
   Zero();
}

void ListViewCell::Zero() {
   //type = empty;
   flags = 0;
   text = NULL;
   image = -1;
}

void ListViewCell::Free() {
   if( text!=NULL )
      delete text;
   Zero();
}

BOOL ListViewCell::IsTextType() {
   return (text!=NULL? TRUE:FALSE);
   //return ((type==textStaticCell) || (type==textEditCell) ? TRUE:FALSE);
}

BOOL ListViewCell::IsImageType() {
   return (image>=0? TRUE:FALSE);
   //return ((type==imageCustCell) || (type==imageIconCell) || (type==imageListCell) ? TRUE:FALSE);
}

BOOL ListViewCell::IsEditable() {
   return (flags & isEditable? TRUE:FALSE);
}

void ListViewCell::SetEditable( BOOL editable ) {
   if( editable ) flags |= isEditable;
   else        flags &= (~isEditable);
}

void ListViewCell::SetText( TCHAR* text ) {
   if( this->text!=NULL )
      delete this->text;

   int length = static_cast<int>(_tcslen(text));
   this->text = new TCHAR[length+1];
   _tcscpy( this->text, text );
}

void ListViewCell::SetImage( int image ) {
   this->image = image;
}


//-----------------------------------------------------------------------------
// class ListViewRow
// An item for a list view, consisting of a row of cell entries, one entry per column

void ListViewRow::Init( int numCells ) {
   Free();

   cells.SetCount( numCells );
   for( int i=0; i<numCells; i++ )
      cells[i].ListViewCell::ListViewCell(); //call the constructor explicitely
}

void ListViewRow::Free() {
   int numCells = cells.Count();
   for( int i=0; i<numCells; i++ )
      cells[i].Free();
   cells.SetCount(0);
}


//-----------------------------------------------------------------------------
// class ListViewColumn
// A column header for a list view, consisting of a name and width


ListViewColumn::ListViewColumn() {
   this->name = NULL;
   this->width = 0;
}

void ListViewColumn::SetName( TCHAR* name ) {
   int length = static_cast<int>(_tcslen(name));

   if( this->name!=NULL ) delete this->name;
   this->name = new TCHAR[length+1];
   _tcscpy( this->name, name );
}

void ListViewColumn::SetWidth( int width ) {
   this->width = width;
}

void ListViewColumn::Free() {
   if( name!=NULL ) delete name;
   name = NULL;
   width = 0;
}

//-----------------------------------------------------------------------------
// class ListView

int ListView_GetNumColumns( HWND hListView ) {
   HWND hHeader = ListView_GetHeader( hListView );
   return Header_GetItemCount( hHeader );
   //LVTILEINFO tileInfo;
   //ListView_GetTileInfo( hListView, &tileInfo );
   //return tileInfo.cColumns;
}

// Given a screen point, outputs the grid column in Y, grid row in X
POINT ListView_ScreenToGrid( HWND hListView, POINT p ) {
   POINT retval, origin;
   LONG &row = retval.x, &col = retval.y;

   LV_HITTESTINFO hittestinfo;
   hittestinfo.pt = p;
   row = ListView_HitTest( hListView, &hittestinfo);

    ListView_GetItemPosition( hListView, (row==-1? 0 : row), &origin );
   int colPos, numColumns = ListView_GetNumColumns( hListView );
    for( col=0, colPos=0; col<numColumns; col++) {
      colPos  += ListView_GetColumnWidth( hListView, col );
      if ( colPos >= (p.x-origin.x) )
         break;
    }

   if( col==numColumns ) col=-1;
   return retval;
}

// Given a grid row and column, outputs the screen rect
RECT ListView_GridToRect( HWND hListView, int row, int col ) {
   //ListView_EnsureVisible( hListView, row, FALSE );

    RECT rect;
    // 
    // SR NOTE64: This generates an internal compiler error with VS2005 Beta1.
    // ListView_GetItemRect( hListView, row, &rect, LVIR_BOUNDS );
    // 
    // The following is just the expansion of the macro, which circumvents the problem.
    // 
    // #define ListView_GetItemRect(hwnd, i, prc, code) \
    //   (BOOL)SNDMSG((hwnd), LVM_GETITEMRECT, (WPARAM)(int)(i), \
    //      ((prc) ? (((RECT *)(prc))->left = (code),(LPARAM)(RECT *)(prc)) : (LPARAM)(RECT *)NULL))
    // 

    rect.left = LVIR_BOUNDS;
    ::SendMessage(hListView, LVM_GETITEMRECT, row, reinterpret_cast<LPARAM>(&rect));

    for (int i = 0; i < col; i++)
      rect.left += ListView_GetColumnWidth( hListView, i );
    rect.right = rect.left + ListView_GetColumnWidth( hListView, col ) - 1;
   return rect;
}

// If one item selected, returns index.
// If no items selected, returns -1.
// If multiple items selected, returns -2.
int ListView_GetSelectedRow( HWND hListView ) {
   int count = ListView_GetSelectedCount( hListView );
   if( count> 1 ) return -2;
   if( count<=0 ) return -1;

   int mark = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
   return (mark<0?  -1 : mark);
}

BOOL ListView_GetSelectedState( HWND hListView, int i ) {
   return (ListView_GetItemState( hListView, i, LVIS_SELECTED ) == LVIS_SELECTED?  TRUE : FALSE);
}


ListView::ListView() {
   hWnd = NULL;
   hImageList = NULL;
   selRow = selCol = -1;
   notifyProc = NULL;
   notifyParam = NULL;
   defaultWndProc = NULL;
}

void ListView::Init( HWND hWnd ) {
   Free();
   this->hWnd = hWnd;

   defaultWndProc = DLGetWindowProc( hWnd);
   DLSetWindowLongPtr( hWnd, WndProc);
   DLSetWindowLongPtr( hWnd, this);
   editControl.Init( hWnd );
   editControl.SetNotifyProc( EditNotifyProc, this );
}

void ListView::Free() {
   Reset();

   editControl.Free();
   int numColumns = GetNumColumns();
   int numRows = GetNumRows();

   for( int i=0; i<numColumns; i++ )
      columns[i].Free();
   for( int i=0; i<numRows; i++ )
      rows[i].Free();

   columns.SetCount(0);
   rows.SetCount(0);

   DLSetWindowLongPtr( hWnd, defaultWndProc);
}

void ListView::Reset() {
   int numColumns = GetNumColumns();
   int numRows = GetNumRows();

   ListView_DeleteAllItems( hWnd );
   for( int i=(numColumns-1); i>=0; i-- )
      ListView_DeleteColumn( hWnd, i );

   selRow = selCol = -1;
}

void ListView::SetNotifyProc( NotifyProc notifyProc, void* param ) {
   this->notifyProc = notifyProc;
   this->notifyParam = param;
}

void ListView::SetNumColumns( int count ) {
   DbgAssert( GetNumRows()==0 ); //Columns must be initialized before any items are added

   int prevCount = columns.Count();
   for( int i=count; i<prevCount; i++ )
      columns[i].Free();
   columns.SetCount( count );
   for( int i=prevCount; i<count; i++ )
      columns[i].ListViewColumn::ListViewColumn(); //call the constructor explicitely
}

int ListView::GetNumColumns() {
   return columns.Count();
   //LVTILEINFO tileInfo;
   //ListView_GetTileInfo( hWnd, &ileInfo );
   //return tileInfo.cColumns;
}

void ListView::SetSelColumn( int selCol ) {
   //ListView_SetSelectedColumn( hWnd, index ); //FIXME: not currently supported
   this->selCol = selCol;
}

int ListView::GetNumRows() {
   return rows.Count();
}

void ListView::SetNumRows( int count ) {
   int prevCount = rows.Count();
   for( int i=count; i<prevCount; i++ )
      rows[i].Free();
   rows.SetCount( count );

   int numColumns = GetNumColumns();
   for( int i=prevCount; i<count; i++ ) {
      rows[i].ListViewRow::ListViewRow(); //call the constructor explicitely
      rows[i].Init( numColumns );
   }
}

void ListView::SetSelRow( int selRow ) {
   int count = GetNumRows();
   for( int i=0; i<count; i++ ) {
      if( i==selRow ) {
         ListView_SetItemState( hWnd, i, LVIS_SELECTED, LVIS_SELECTED );
      }
      else {
         ListView_SetItemState( hWnd, i, 0, LVIS_SELECTED );
      }
   }
   this->selRow = selRow;
}

ListViewCell* ListView::GetCell( int row, int col ) {
   if( row<0 || row>=GetNumRows() || col<0 || col>=GetNumColumns() )
      return NULL;
   return rows[row].GetCell(col);
}

void ListView::SetImageList( HIMAGELIST hImageList ) {
   this->hImageList = hImageList;
}

HIMAGELIST ListView::GetImageList() {
   return hImageList;
}

void ListView::BeginEdit() {
   int row = selRow, col = selCol;
   int numRows = GetNumRows(), numCols = GetNumColumns();
   if( (row<0) || (row>=numRows) || (col<0) || (col>=numCols) )
      return;

   ListViewCell* cell = GetRow( row )->GetCell( col );

   RECT rect = ListView_GridToRect( hWnd, row, col );
   editControl.SetText( cell->GetText() );
   editControl.BeginEdit( rect );
}

void ListView::EndEdit() {
   editControl.EndEdit();
}

BOOL ListView::IsEditing() {
   return (editControl.IsEditing()? TRUE:FALSE);
}

void ListView::UpdateRow( int row ) {
   int numColumns = GetNumColumns();
   for( int j=0; j<numColumns; j++ )
   {
      UpdateCell( row, j );
   }
}

void ListView::UpdateCell( int row, int col ) {
   ListViewCell* cell = GetCell( row, col );

   LV_ITEM item;
   item.mask = 0; //LVIF_STATE; // | LVIF_PARAM;
   item.iItem = row;
   item.iSubItem = col;
   item.iImage = -1;
   item.pszText = NULL;
   item.cchTextMax = 0;
   //item.state = 0;
   //item.stateMask = 0;
   //item.lParam = (LPARAM)(void *)NULL;

   if( cell->IsImageType() ) {
      item.mask |= LVIF_IMAGE;
      item.iImage = cell->GetImage();
   }

   if( cell->IsTextType() )   {
      item.mask |= LVIF_TEXT;
      item.pszText = cell->GetText();
      item.cchTextMax = static_cast<int>(_tcslen( item.pszText ) + 1);
   }

   //if( j==0 )   ListView_InsertItem( hWnd, &item );
   //else
      ListView_SetItem( hWnd, &item );
}

void ListView::Update() {
   Reset();

   int numColumns = GetNumColumns();
   int numRows = GetNumRows();

   COLORREF fgColor, bgColor;
   fgColor = GetCustSysColor(COLOR_WINDOWTEXT), bgColor = GetCustSysColor(COLOR_WINDOW);
   //if( row==GetSelItem ) fgColor = GetCustSysColor(COLOR_WINDOWTEXT), bgColor = GetCustSysColor(COLOR_WINDOW);
   //else               fgColor = GetCustSysColor(COLOR_HIGHLIGHTTEXT), bgColor = GetCustSysColor(COLOR_HIGHLIGHT);

   // Set colors & imagelist
   ListView_SetTextColor( hWnd, fgColor );
   ListView_SetTextBkColor( hWnd, bgColor );
   ListView_SetBkColor( hWnd, bgColor );
   ListView_SetImageList( hWnd, hImageList, LVSIL_SMALL );     //LVSIL_NORMAL, LVSIL_SMALL, LVSIL_STATE

   // Add Columns
   LV_COLUMN column;
   for( int i=0; i<numColumns; i++ ) {
      column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT; // | LVCFMT_IMAGE | LVCFMT_COL_HAS_IMAGES;
      column.fmt = LVCFMT_LEFT;
      column.pszText = GetColumn(i)->GetName();
      column.cx = 1;
      column.iSubItem = i;
      ListView_InsertColumn( hWnd, i, &column);
   }

   // Add Rows
   LV_ITEM stubItem;
   stubItem.mask = 0;
   stubItem.iSubItem = 0;

   for( int i=0; i<numRows; i++ ) {
      stubItem.iItem = i;
      ListView_InsertItem( hWnd, &stubItem );
      for( int j=0; j<numColumns; j++ )
      {
         UpdateCell( i, j );
      }
   }

   // Resize the columns
   for( int i=0; i<numColumns; i++ ) {
      int width = GetColumn(i)->GetWidth();
      if( width<=0 ) width = LVSCW_AUTOSIZE_USEHEADER;
         //width = LVSCW_AUTOSIZE;
      ListView_SetColumnWidth( hWnd, i, width );
   }

   ListView_SetExtendedListViewStyle( hWnd, LVS_EX_SUBITEMIMAGES ); //LVS_EX_TRACKSELECT | LVS_EX_FULLROWSELECT
}

//-----------------------------------------------------------------------------
// ListView Window Proc

LRESULT CALLBACK ListView::ProcessMessage( UINT msg, WPARAM wParam, LPARAM lParam ) {
   return WndProc( hWnd, msg, wParam, lParam );
}

void ListView::OnCellChange( int row, int col ) {
   if( col==selCol && row==selRow ) return;

   if( editControl.IsEditing() ) EndEdit();
   selCol = col;
   selRow = row;

   if( notifyProc!=NULL ) {
      NotifyInfo notifyInfo;
      notifyInfo.message = notifySelCell;
      notifyInfo.row = row, notifyInfo.col = col;
      notifyProc( notifyInfo, notifyParam );
   }
}

void ListView::OnCellClick( int row, int col ) {
   ListViewCell* cell = GetCell(row,col);
   int prevRow = selRow;
   int prevCol = selCol;

   BOOL isEditable = ((cell!=NULL) && (cell->IsEditable())? TRUE:FALSE);
   BOOL isSecondClick = (row==prevRow && col==prevCol? TRUE:FALSE);
   BOOL isEditing = editControl.IsEditing();
   DbgAssert( !isSecondClick || !isEditing );

   if( !isSecondClick &&  isEditing )              EndEdit();
   if(  isSecondClick && !isEditing && isEditable )   BeginEdit();
   //FIXME: What if the EndEdit() is rejected?

   //if( prevRow>=0 )   ListView_RedrawItems( hWnd, prevRow, prevRow );
   //if( row>=0 )    ListView_RedrawItems( hWnd, row, row );

   if( !isSecondClick ) OnCellChange( row, col );
}

void ListView::OnColumnClick( int col ) {
   if( col>=0 ) {
      selCol = col;
      if( notifyProc!=NULL ) {
         NotifyInfo notifyInfo;
         notifyInfo.message = notifySelCol;
         notifyInfo.col = col;
         notifyProc( notifyInfo, notifyParam );
      }
   }
}

bool ListView::EditNotifyProc( EditControl::NotifyInfo& info, void* param ) {
   ListView* parent = (ListView*)param;
   if( parent->notifyProc==NULL ) return true;
   int row = parent->selRow, col = parent->selCol;

   NotifyInfo notifyInfo;
   notifyInfo.editText = info.editText;
   notifyInfo.editChar = info.editChar;
   notifyInfo.row = row, notifyInfo.col = row;
   if( info.message==EditControl::notifyEditing )
      notifyInfo.message = notifyEditing;
   if( info.message==EditControl::notifyEndEdit ) {
      notifyInfo.message = notifyEndEdit;

      ListViewCell* cell = parent->GetCell( row, col );
      cell->SetText( info.editText.data() );
      parent->UpdateCell( row, col );
   }

   return parent->notifyProc( notifyInfo, parent->notifyParam );
}

LRESULT CALLBACK ListView::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   ListView *parent = DLGetWindowLongPtr<ListView*>( hWnd);
   BOOL notifyMessage = ((msg<=LVN_FIRST) && (msg>=LVN_LAST)? TRUE:FALSE);
   BOOL defaultHandling = FALSE;

   switch( msg ) {
   case WM_LBUTTONDOWN:
   case WM_RBUTTONDOWN:
   case WM_LBUTTONDBLCLK: {
         POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
         POINT cell = ListView_ScreenToGrid( hWnd, p );
         if( parent->defaultWndProc!=NULL )
            CallWindowProc( parent->defaultWndProc, hWnd, msg, wParam, lParam );
         parent->OnCellClick( cell.x, cell.y );
         //defaultHandling = TRUE;
         break;
      }

   case WM_HSCROLL:
   case WM_VSCROLL:
      if( parent->IsEditing() )
         parent->EndEdit();
      else defaultHandling = TRUE;
      break;

   case WM_KEYUP: {
         int x = ListView_GetSelectedRow( hWnd );
         if( x!=parent->selRow )
            parent->OnCellChange( x, parent->selCol );
         break;
      }

   //case WM_DRAWITEM: {
   //    parent->DrawItem( (LPDRAWITEMSTRUCT) lParam );
   //    break;
   // }

   case LVN_COLUMNCLICK: {
         LPNMLISTVIEW lpn = (LPNMLISTVIEW)lParam;
         int col = lpn->iSubItem;
         parent->OnColumnClick( col );
         break;
      }
   default:
      defaultHandling = TRUE;
      break;
   }

   if( defaultHandling && !notifyMessage ) {
      if( parent->defaultWndProc!=NULL ) return CallWindowProc( parent->defaultWndProc, hWnd, msg, wParam, lParam );
      else return FALSE;
   }

   return FALSE;
}

//-----------------------------------------------------------------------------
// Brush Preset Manager Dialog
int layerViewWidthDelta = 16;
int listViewHeightDelta = 82;

INT_PTR CALLBACK BrushPresetMgr::DialogWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   BrushPresetMgr *parent = DLGetWindowLongPtr<BrushPresetMgr*>( hWnd);

   switch( msg ) {
   case WM_INITDIALOG: {
         parent = (BrushPresetMgr*)lParam;
         DLSetWindowLongPtr(hWnd, parent);

         parent->OnInitDialog( hWnd );
         //FIXME: GetWindowPos() ... get the delta values
         break;
      }
   case WM_DESTROY: {
         parent->OnDestroyDialog( hWnd );
         break;
      }

   case WM_SIZE: {
         int width = LOWORD(lParam) - layerViewWidthDelta;
         int height = HIWORD(lParam) - listViewHeightDelta;
         HWND hListView = GetDlgItem( hWnd, IDC_PRESET_LIST );
         BOOL res = SetWindowPos( hListView, 0, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
         parent->OnDialogResized();
         break;
      }
   case WM_MOVING:
      parent->OnDialogMoved();
      break;

   case WM_COMMAND: {
         int id = LOWORD(wParam);
         switch( id ) {
         case IDC_PRESET_LOAD:         parent->OnLoadPresetsBtn();         break;
         case IDC_PRESET_SAVE:         parent->OnSavePresetsBtn();         break;
         case IDC_PRESET_ADD:       parent->OnAddPresetBtn();        break;
         case IDC_PRESET_DUPLICATE:    parent->OnDuplicatePresetBtn();     break;
         case IDC_PRESET_DELETE:       parent->OnRemovePresetBtn();     break;
         case IDC_PRESET_CONTEXT:
            switch( HIWORD(wParam) ) {
            case CBN_SELCHANGE: {
                  HWND hComboBox = GetDlgItem( hWnd, IDC_PRESET_CONTEXT );
                  int index = SendMessage( hComboBox, CB_GETCURSEL, 0, 0 );
                  IBrushPresetContext* context =
                     (IBrushPresetContext*)SendMessage( hComboBox, CB_GETITEMDATA, index, 0 );
                  parent->SetFocusContextID( context->ContextID() );
                  parent->UpdateDialog( BrushPresetMgr::updateDialog_ListView );
                  break;
               }
            }
            break;
         }
         break;
      }

   case CC_SPINNER_CHANGE:
      //spin = (ISpinnerControl*)lParam; 
      parent->OnSpinnerChange();
      break;

   //Derived messages sent by the list view to its parent
   case WM_DRAWITEM:
   case WM_HSCROLL:
   case WM_VSCROLL:
      return parent->listView.ProcessMessage( msg, wParam, lParam );

   case WM_NOTIFY: {
         LPNMHDR lp = (LPNMHDR)lParam;
         UINT code = lp->code;
         // The 'first' value is higher than the 'last'.  Oh the irony
         if( (code<=LVN_FIRST) && (code>=LVN_LAST) ) {
            HWND hListView = GetDlgItem( hWnd, IDC_PRESET_LIST );
            if( hListView == lp->hwndFrom )
               return parent->listView.ProcessMessage( code, wParam, lParam );
         }
         break;
      }

   case WM_CLOSE:
      parent->HideDialog();
      break;

   default: return FALSE;
   }

   return TRUE;
}