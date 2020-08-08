// ============================================================================
// GroupBox.cpp
// Copyright ©1999 Discreet
// Created by Simon Feltman
// ----------------------------------------------------------------------------

#include "MAXScrpt.h"
#include "MAXObj.h"
#include "Numbers.h"
#include "Parser.h"

#include "resource.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "defextfn.h"
#	include "namedefs.h"

#include "definsfn.h"

// ============================================================================
visible_class(GroupBoxControl)

class GroupBoxControl : public RolloutControl
{
private:
	HWND m_hWnd;

public:
	GroupBoxControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
		: RolloutControl(name, caption, keyparms, keyparm_count)  { tag = class_tag(GroupBoxControl); }

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
	{ return new GroupBoxControl(name, caption, keyparms, keyparm_count); }

	         classof_methods(GroupBoxControl, RolloutControl);
	void     collect() { delete this; }
	void     sprin1(CharStream* s) { s->printf(_T("GroupBoxControl:%s"), name->to_string()); }

	void     add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCTSTR  get_control_class() { return SPINNERWINDOWCLASS; }
	void     compute_layout(Rollout *ro, layout_data* pos) { pos->width = 90; pos->height = 30;}
	BOOL     handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	Value*   get_property(Value** arg_list, int count);
	Value*   set_property(Value** arg_list, int count);
	void     set_enable();
};


// ============================================================================
visible_class_instance (GroupBoxControl, "GroupBoxControl")

void GroupBoxControl::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
	caption = caption->eval();

	TCHAR *text = caption->eval()->to_string();
	control_ID = next_id();
	parent_rollout = ro;

	layout_data pos;
	RolloutControl::compute_layout(ro, &pos, current_y);

	m_hWnd = CreateWindow(_T("BUTTON"),
		text,
		BS_GROUPBOX | WS_VISIBLE | WS_CHILD | WS_GROUP | WS_OVERLAPPED ,
		pos.left, pos.top, pos.width, pos.height,
		parent, (HMENU)control_ID, hInstance, NULL);

	SendDlgItemMessage(parent, control_ID, WM_SETFONT, (WPARAM)ro->font, 0L);
}


// ============================================================================
BOOL GroupBoxControl::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}


// ============================================================================
Value* GroupBoxControl::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];

	if(prop == n_width)
	{
		if(parent_rollout && parent_rollout->page)
		{
			HWND hWnd = GetDlgItem(parent_rollout->page, control_ID);
			RECT rect;
			GetWindowRect(hWnd, &rect);
			MapWindowPoints(NULL, parent_rollout->page, (LPPOINT)&rect, 2);
			return Integer::intern(rect.right-rect.left);
		}
		else return &undefined;
	}
	else if(prop == n_height)
	{
		if(parent_rollout && parent_rollout->page)
		{
			HWND hWnd = GetDlgItem(parent_rollout->page, control_ID);
			RECT rect;
			GetWindowRect(hWnd, &rect);
			MapWindowPoints(NULL, parent_rollout->page, (LPPOINT)&rect, 2);
			return Integer::intern(rect.bottom-rect.top);
		}
		else return &undefined;
	}

	return RolloutControl::get_property(arg_list, count);
}


// ============================================================================
Value* GroupBoxControl::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if(prop == n_width)
	{
		if(parent_rollout && parent_rollout->page)
		{
			int width = val->to_int();
			HWND	hwnd = GetDlgItem(parent_rollout->page, control_ID);
			RECT	rect;
			GetWindowRect(hwnd, &rect);
			MapWindowPoints(NULL, parent_rollout->page,	(LPPOINT)&rect, 2);
			SetWindowPos(hwnd, NULL, rect.left, rect.top, width, rect.bottom-rect.top, SWP_NOZORDER);
		}
	}
	else if(prop == n_height)
	{
		if(parent_rollout && parent_rollout->page)
		{
			int height = val->to_int();
			HWND	hwnd = GetDlgItem(parent_rollout->page, control_ID);
			RECT	rect;
			GetWindowRect(hwnd, &rect);
			MapWindowPoints(NULL, parent_rollout->page,	(LPPOINT)&rect, 2);
			SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.right-rect.left, height, SWP_NOZORDER);
		}
	}
	else 
		return RolloutControl::set_property(arg_list, count);

	return val;
}


// ============================================================================
void GroupBoxControl::set_enable()
{
	if (parent_rollout != NULL && parent_rollout->page != NULL)
	{
		HWND ctrl = GetDlgItem(parent_rollout->page, control_ID);
		if(ctrl) 
		{	EnableWindow(ctrl, enabled);
			InvalidateRect(ctrl, NULL, TRUE);	// LAM - defect 302171
		}
	}
}


// ============================================================================
void GroupBoxInit()
{
	install_rollout_control(Name::intern("GroupBox"), GroupBoxControl::create);
}

