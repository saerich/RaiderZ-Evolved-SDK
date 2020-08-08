#include "avg_maxver.h"

#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "Strings.h"
#include "Arrays.h"
#include "colorval.h"
#include "ITabDialog.h"

#include "resource.h"

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "MXSAgni.h"
#include "agnidefs.h"

// ============================================================================

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#ifndef NO_RENDEREFFECTS 

Value*
envEffectsDialog_Open_cf(Value** arg_list, int count)
{
	check_arg_count(envEffectsDialog.Open, 0, count);
	HoldSuspend suspend;
	MAXScript_interface7->OpenEnvEffectsDialog();
	return &ok;
}

Value*
envEffectsDialog_Close_cf(Value** arg_list, int count)
{
	check_arg_count(envEffectsDialog.Close, 0, count);
	HoldSuspend suspend;
	MAXScript_interface7->CloseEnvEffectsDialog();
	return &ok;
}

Value*
envEffectsDialog_isOpen_cf(Value** arg_list, int count)
{
	check_arg_count(envEffectsDialog.isOpen, 0, count);
	return (MAXScript_interface7->EnvEffectsDialogOpen()) ? &true_value : &false_value;
}

#endif // NO_RENDEREFFECTS

Value*
MatEditor_Open_cf(Value** arg_list, int count)
{
	// Supports either Medit or SME, according to current mtlDlgMode
	check_arg_count(MatEditor.Open, 0, count);
	HoldSuspend suspend;
	MAXScript_interface7->OpenMtlDlg();
	return &ok;
}

Value*
MatEditor_Close_cf(Value** arg_list, int count)
{
	// Supports either Medit or SME, according to current mtlDlgMode
	check_arg_count(MatEditor.Close, 0, count);
	HoldSuspend suspend;
	MAXScript_interface7->CloseMtlDlg();
	return &ok;
}

Value*
MatEditor_isOpen_cf(Value** arg_list, int count)
{
	// Supports either Medit or SME, according to current mtlDlgMode
	check_arg_count(MatEditor.isOpen, 0, count);
	return (MAXScript_interface7->IsMtlDlgShowing()) ? &true_value : &false_value;
}

// See also MatEditor_GetMode() and MatEditor_SetMode() in avg_DLX.cpp

static Class_ID Value_to_Dialog_ClassID(Value* arg)
{
	if (is_array(arg))
	{
		Array* theArray = (Array*)arg;
		if (theArray->size != 2) 
			throw RuntimeError (GetString(IDS_EXPECTED_TWO_ELEMENT_INT_ARRAY_OR_TABBED_DIALOG_NAMEVAL),arg);
		return Class_ID(theArray->data[0]->to_int(), theArray->data[1]->to_int());
	}
	else if (arg == n_render)
		return TAB_DIALOG_RENDER_ID;

	else if (arg == n_renderVP)
		return TAB_DIALOG_VIDEO_POST_ID;

	else if (arg == n_envEffects)
		return TAB_DIALOG_ENVIRONMENT_ID;

	else if (arg == n_objProp)
		return TAB_DIALOG_PROPERTIES_ID;

	else if (arg == n_preferences)
		return TAB_DIALOG_PREFERENCES_ID;

	else if (arg == n_configPath)
		return TAB_DIALOG_PATH_ID;

	else
		throw RuntimeError (GetString(IDS_EXPECTED_TWO_ELEMENT_INT_ARRAY_OR_TABBED_DIALOG_NAMEVAL),arg);
}

static Class_ID Value_to_Page_ClassID(Value* arg)
{
	if (is_array(arg))
	{
		Array* theArray = (Array*)arg;
		if (theArray->size != 2) 
			throw RuntimeError (GetString(IDS_EXPECTED_TWO_ELEMENT_INT_ARRAY_OR_INT),arg);
		return Class_ID(theArray->data[0]->to_int(), theArray->data[1]->to_int());
	}
	else
		throw RuntimeError (GetString(IDS_EXPECTED_TWO_ELEMENT_INT_ARRAY_OR_INT),arg);
}

static ITabPage* Get_Dialog_Page(ITabbedDialog *dialog, Value* arg)
{
	ITabPage *page = NULL;
	if (is_integer_number(arg))
	{
		int index = arg->to_int();
		range_check(index, 1, dialog->GetNbPages(), GetString(IDS_DIALOG_PAGE_INDEX_OUT_OF_RANGE));
		page = dialog->GetPage(index - 1);
	}
	else
		page = dialog->GetPage(Value_to_Page_ClassID(arg));
	return page;
}

// ============================================================================
// <bool> tabbedDialogs.isOpen <tabbedDialogSpec>
Value*
tabbedDialogs_isOpen_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.isOpen, 1, count);
	ITabDialogManager* manager = GetTabDialogManager();
	return (manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]))) ? &true_value : &false_value;
}

// ============================================================================
// <#(<int>,<int>)> tabbedDialogs.getDialogID <tabbedDialogSpec>
Value*
tabbedDialogs_getDialogID_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.getDialogID, 1, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
	{
		one_typed_value_local(Array* result);
		vl.result = new Array (2);
		Class_ID cid = dialog->GetDialogID();
		vl.result->append(Integer::intern(cid.PartA()));
		vl.result->append(Integer::intern(cid.PartB()));
		return_value(vl.result);
	}
	return &undefined;
}

// ============================================================================
// tabbedDialogs.invalidate <tabbedDialogSpec>
Value*
tabbedDialogs_invalidate_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.invalidate, 1, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
		dialog->Invalidate();
	return &ok;
}

// ============================================================================
// <bool> tabbedDialogs.OkToCommit <tabbedDialogSpec>
Value*
tabbedDialogs_OkToCommit_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.OkToCommit, 1, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
		return (dialog->OkToCommit()) ? &true_value : &false_value;
	return &undefined;
}

// ============================================================================
// tabbedDialogs.CommitPages <tabbedDialogSpec>
Value*
tabbedDialogs_CommitPages_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.CommitPages, 1, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
		dialog->CommitPages();
	return &ok;
}

// ============================================================================
// tabbedDialogs.CloseDialog <tabbedDialogSpec>
Value*
tabbedDialogs_CloseDialog_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.CloseDialog, 1, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
		dialog->CloseDialog();
	return &ok;
}

// ============================================================================
// tabbedDialogs.CancelDialog <tabbedDialogSpec>
Value*
tabbedDialogs_CancelDialog_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.CancelDialog, 1, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
		dialog->CancelDialog();
	return &ok;
}

// ============================================================================
// <int> tabbedDialogs.getNumPages <tabbedDialogSpec>
Value*
tabbedDialogs_getNumPages_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.getNumPages, 1, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
		return Integer::intern(dialog->GetNbPages());
	return &undefined;
}

// ============================================================================
// <#(<int>,<int>)> tabbedDialogs.getPageID <tabbedDialogSpec> <tabPageSpec>
Value*
tabbedDialogs_getPageID_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.getPageID, 2, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
	{
		ITabPage *page = Get_Dialog_Page(dialog, arg_list[1]);
		if (page)
		{
			one_typed_value_local(Array* result);
			vl.result = new Array (2);
			Class_ID cid = page->GetPageID();
			vl.result->append(Integer::intern(cid.PartA()));
			vl.result->append(Integer::intern(cid.PartB()));
			return_value(vl.result);
		}
	}
	return &undefined;
}

// ============================================================================
// <#(<int>,<int>)> tabbedDialogs.getCurrentPage <tabbedDialogSpec>
Value*
tabbedDialogs_getCurrentPage_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.getCurrentPage, 1, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
	{
		ITabPage *page = dialog->GetPage(dialog->CurrentPage());
		if (page)
		{
			one_typed_value_local(Array* result);
			vl.result = new Array (2);
			Class_ID cid = page->GetPageID();
			vl.result->append(Integer::intern(cid.PartA()));
			vl.result->append(Integer::intern(cid.PartB()));
			return_value(vl.result);
		}
	}
	return &undefined;
}

// ============================================================================
// tabbedDialogs.setCurrentPage <tabbedDialogSpec> <tabPageSpec>
Value*
tabbedDialogs_setCurrentPage_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.setCurrentPage, 2, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
	{
		ITabPage *page = Get_Dialog_Page(dialog, arg_list[1]);
		if (page)
			dialog->Switch(page->GetPageID());
	}
	return &ok;
}

// ============================================================================
// tabbedDialogs.invalidatePage <tabbedDialogSpec> <tabPageSpec>
Value*
tabbedDialogs_invalidatePage_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.invalidatePage, 2, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
	{
		ITabPage *page = Get_Dialog_Page(dialog, arg_list[1]);
		if (page)
			page->Invalidate();
	}
	return &ok;
}

// ============================================================================
// <string> tabbedDialogs.getPageTitle <tabbedDialogSpec> <tabPageSpec>
Value*
tabbedDialogs_getPageTitle_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.getPageTitle, 2, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
	{
		ITabPage *page = Get_Dialog_Page(dialog, arg_list[1]);
		if (page)
			return new String (MAXScript_interface7->GetTabPageTitle(page));
	}
	return &undefined;
}

// ============================================================================
// <bool> tabbedDialogs.isPage <tabbedDialogSpec> <tabPageSpec>
Value*
tabbedDialogs_isPage_cf(Value** arg_list, int count)
{
	check_arg_count(tabbedDialogs.isPage, 2, count);
	ITabDialogManager* manager = GetTabDialogManager();
	ITabbedDialog *dialog = manager->GetTabbedDialog(Value_to_Dialog_ClassID(arg_list[0]));
	if (dialog)
	{
		ITabPage *page = Get_Dialog_Page(dialog, arg_list[1]);
		return (page) ? &true_value : &false_value;
	}
	return &undefined;
}


