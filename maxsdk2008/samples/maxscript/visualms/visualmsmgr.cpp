/*	
 *		VisualMSMgr.cpp - public manager interface for VisualMS
 *
 *			Copyright © Autodesk, Inc, 2000.  John Wainwright.
 *
 */

#include "stdafx.h"
#include "VisualMS.h"
#include "VisualMSThread.h"
#include "VisualMSDoc.h"
#include "MainFrm.h"
#include "FormEd.h"

#include "iVisualMS.h"

// ----------- FnPub interface implementation class & descriptors ---------------------

class VisualMSMgr : public IVisualMSMgr
{
private:

public:
	IVisualMSForm* CreateForm();						// create a new form
	IVisualMSForm* CreateFormFromFile(TCHAR* fname);	// create form from a .vms file

	DECLARE_DESCRIPTOR(VisualMSMgr) 
	// dispatch map
	BEGIN_FUNCTION_MAP
		FN_0(createForm,		 TYPE_INTERFACE, CreateForm); 
		FN_1(createFormFromFile, TYPE_INTERFACE, CreateFormFromFile, TYPE_FILENAME); 
	END_FUNCTION_MAP 

	void    Init() { }
};

// VisualMS Manager interface instance and descriptor
VisualMSMgr visualMSMgr(VISUALMS_MGR_INTERFACE, _T("visualMS"), 0, NULL, FP_CORE, 
	IVisualMSMgr::createForm,		  _T("createForm"), 0,	       TYPE_INTERFACE, 0, 0, 
	IVisualMSMgr::createFormFromFile, _T("createFormFromFile"), 0, TYPE_INTERFACE, 0, 1, 
		_T("fileName"), 0, TYPE_FILENAME,
	end 
); 

// ------- interface to individual VisiualMS forms ------------

FPInterfaceDesc visualMSFormDesc(VISUALMS_FORM_INTERFACE, _T("visualMSForm"), 0, NULL, 0, 
	IVisualMSForm::open,		_T("open"), 0,	TYPE_VOID, 0, 0, 
	IVisualMSForm::close,		_T("close"), 0, TYPE_VOID, 0, 0, 
	IVisualMSForm::genScript,	_T("genScript"), 0,	TYPE_BOOL, 0, 1, 
		_T("script"), 0, TYPE_TSTR_BR,
	end 
); 

FPInterfaceDesc* IVisualMSForm::GetDesc() 
{
	return &visualMSFormDesc;
}

class VisualMSForm : public IVisualMSForm 
{
	bool	has_source;
	TCHAR*  source;					// source string if any
	int		from, to;				// bounds of original rollout within source
	CVisualMSThread *pThread;

public:
			VisualMSForm();
	void	reset() { has_source = false; source = NULL; from = 0; to = 0; }
	void	Open(IVisualMSCallback* cb=NULL, TCHAR* source=NULL);	// open the form editor on the form
	void	Close();						// close the form editor
	void	InitForm(TCHAR* formType, TCHAR* formName, TCHAR* caption);
	IVisualMSItem* AddItem(TCHAR* itemType, TCHAR* itemName, TCHAR* text, int src_from=-1, int src_to=-1);
	IVisualMSItem* AddCode(TCHAR* code, int src_from=-1, int src_to=-1);
	IVisualMSItem* FindItem(TCHAR* itemName);
	BOOL	GenScript(TSTR& script, TCHAR* indent=NULL);
	BOOL	HasSourceBounds(int& from, int& to);
	void	SetSourceBounds(int from, int to);
	void	SetWidth(int w);
	void	SetHeight(int h);
};

VisualMSForm::VisualMSForm() : has_source(false), source(NULL)
{
	pThread = CVisualMSThread::Create(false); // create hidden
	CVisualMSDoc* doc = pThread->GetDoc();
	if (doc)
	{
		doc->SetModifiedFlag(FALSE);
		doc->m_creating = true;
	}
	reset();
}

void 
VisualMSForm::Open(IVisualMSCallback* cb, TCHAR* source)	// open the form editor on the form
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	this->source = source;
	CVisualMSDoc* doc = pThread->GetDoc();
	if (doc)
	{
		doc->SetEditCallback(cb);
		doc->SetModifiedFlag(FALSE);
		doc->m_creating = false;
	}
	pThread->ShowApp();
	if (doc) doc->Invalidate();
}

void 
VisualMSForm::Close()				// close the form editor
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: This function should take a paremeter that lets VMS know if 
	// it should close without saving or prompt !!!
	CVisualMSDoc* doc = pThread->GetDoc();
	if (doc)
	{
		doc->SetModifiedFlag(FALSE);
	}
	pThread->Destroy();

	// DELETE THIS ?
}

void 
VisualMSForm::InitForm(TCHAR* formType, TCHAR* formName, TCHAR* caption)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// reset current doc
	CVisualMSDoc* doc = pThread->GetDoc();

	// fill in form details
	CFormObj *formObj = doc->GetFormObj();
	formObj->Class() = formType;
	formObj->Name() = formName; 
	formObj->Caption() = caption; 
	formObj->PropChanged(IDX_NAME);
	doc->Invalidate();
}

void			
VisualMSForm::SetWidth(int w)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CVisualMSDoc* doc = pThread->GetDoc();
	CFormObj *formObj = doc->GetFormObj();
	formObj->m_properties[IDX_WIDTH] = w;
	formObj->PropChanged(IDX_WIDTH);
	doc->Invalidate();
}

void			
VisualMSForm::SetHeight(int h)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CVisualMSDoc* doc = pThread->GetDoc();
	CFormObj *formObj = doc->GetFormObj();
	formObj->m_properties[IDX_HEIGHT] = h;
	formObj->PropChanged(IDX_HEIGHT);
}

IVisualMSItem* 
VisualMSForm::AddItem(TCHAR* itemType, TCHAR* itemName, TCHAR* text, int src_from, int src_to)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CVisualMSDoc* doc = pThread->GetDoc();

	// NOTE: Since VMS is now multi-threaded and CWnd derived objects cannot be created 
	// across threads use the new WM_CREATEOBJ message to create CGuiObj based objects.
	CRect rect(0,0,10,10);
	CGuiObj *obj = (CGuiObj*)::SendMessage(
		pThread->GetFrame()->GetSafeHwnd(),
		WM_CREATEOBJ,
		doc->ItemIDFromName(itemType),
		(LPARAM)&rect);

	if(obj)
	{
		obj->m_selected = FALSE;
		obj->src_from = src_from;
		obj->src_to = src_to;
		doc->AddObj(obj);
		obj->Class() = itemType;
		obj->Name() = itemName;
		obj->Caption() = text;
		obj->PropChanged(IDX_CAPTION);
	}
	return obj;

}

IVisualMSItem* 
VisualMSForm::FindItem(TCHAR* itemName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CVisualMSDoc* doc = pThread->GetDoc();
	for (int i = 0; i < doc->m_objects.GetSize(); i++)
	{
		CGuiObj* obj = (CGuiObj*)doc->m_objects[i];
		if (_stricmp(itemName, obj->Name()) == 0)
			return obj;
	}
	return NULL;
}

IVisualMSItem* 
VisualMSForm::AddCode(TCHAR* code, int src_from, int src_to)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CVisualMSDoc* doc = pThread->GetDoc();
	CFormEd *formEd = &doc->GetFormObj()->m_formEd;
	CCodeObj *obj; 
	if (src_from >= 0)
	{
		// specified as offsets within source
		TCHAR c;
		// ignore if just a line of whitespace
      int sf;
		for (sf = src_from; sf < src_to && isspace(c = code[sf]) && c != _T('\n') && c != _T('\r'); sf++) ;
		if (sf == src_to)
			return NULL;
		// trim any initial linefeed
		if (code[src_from] == _T('\n')) src_from++;
		else if (code[src_from] == _T('\r'))
			if (code[src_from] == _T('\n')) src_from += 2;
			else src_from++;
		// trim any final blank line
		while (src_to > src_from && isspace(c = code[src_to-1]) && c != _T('\n') && c != _T('\r')) --src_to;
		// make code obj
		c = code[src_to];
		code[src_to] = _T('\0');
		obj = new CCodeObj (&code[src_from]);
		code[src_to] = c;
	}
	else
		obj = new CCodeObj (code);
	if(obj)
	{
		obj->m_selected = FALSE;
		obj->src_from = src_from;
		obj->src_to = src_to;
		doc->AddObj(obj);
	}
	return obj;
}

BOOL 
VisualMSForm::GenScript(TSTR& script, TCHAR* indent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CVisualMSDoc* doc = pThread->GetDoc();
	if (doc)
	{
		CString cscript;
		bool res = doc->GenScript(cscript, indent);
		if (res)
		{
			script = cscript;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL			
VisualMSForm::HasSourceBounds(int& from, int& to)
{
	if (has_source)
	{
		from = this->from;
		to = this->to;
	}
	return has_source;
}

void			
VisualMSForm::SetSourceBounds(int from, int to)
{
	has_source = true;
	this->from = from;
	this->to = to;
}

// ------- interface to individual VisiualMS form items ------------

FPInterfaceDesc visualMSItemDesc(VISUALMS_ITEM_INTERFACE, _T("visualMSItem"), 0, NULL, 0, 
	end 
); 

FPInterfaceDesc* IVisualMSItem::GetDesc() 
{
	return &visualMSItemDesc;
}

// ------------ VisualMS manager interface implementation methods ---------

IVisualMSForm* 
VisualMSMgr::CreateForm()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	VisualMSForm *pVmsForm = new VisualMSForm();
	return pVmsForm;
}

IVisualMSForm* 
VisualMSMgr::CreateFormFromFile(TCHAR* fname)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return NULL;
}


// ------- interface descriptor for base class for VMS edit callbacks ------------

FPInterfaceDesc visualMSCallback(VISUALMS_CALLBACK_INTERFACE, _T("visualMSCallback"), 0, NULL, 0, 
	IVisualMSCallback::save,	_T("Save"), 0,	TYPE_VOID, 0, 0, 
	IVisualMSCallback::close,	_T("Close"), 0,  TYPE_VOID, 0, 0, 
	end 
); 

/* FPInterfaceDesc* IVisualMSCallback::GetDesc() 
{
	return &visualMSCallback;
}
*/