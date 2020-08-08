/*===========================================================================*\
 |  Ishani's stuff for MAX Script R3
 |
 |  FILE:	ui-timer.cpp
 |			UI Control : Timer 
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) KINETIX 1998
 |			All Rights Reserved.
 |
 |  HIST:	Started 7-7-98
 | 
\*===========================================================================*/

/*=========================[ TIMER EVENT CONTROL ]===========================*\
 |
 | use:
 |    Timer [name] [title] <interval:1000> <active:true> <ticks:0>
 |
 |
 | SETTINGS:
 |		interval			--> 		time between tick events, in milliseconds
 |		active	 			-->			timer is ticking or not. when false, no events
 |										will be fired off
 |		ticks				-->			number of tick events that have been fired
 |
 | EVENTS:
 |		tick				-->			called when timer clicks. space between ticks is
 |										determined by the 'interval' property
 |
\*===========================================================================*/


//#include "pch.h"
#include "MAXScrpt.h"
#include "MAXObj.h"
#include "Numbers.h"
#include "rollouts.h"

#include "resource.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "i3.h"
#include "MXSAgni.h"

#include "defextfn.h"
#	include "namedefs.h"

#include "definsfn.h"

/* -------------------- ish3 : Timer  ------------------- */


visible_class_instance (ish3_Timer, s_timer)

void
ish3_Timer::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
	caption = caption->eval();

	// We'll hang onto these...
	parent_rollout = ro;
	control_ID = next_id();

	Value* tmp;
	// Get and update defaults (1 second delay, active)
	tmp = control_param(interval);
	if(tmp==&unsupplied) interval = 1000;
	else
		interval = tmp->to_int();

	tmp = control_param(active);
	if(tmp==&unsupplied) active = TRUE;
	else
		active = tmp->to_bool();

	tmp = control_param(ticks);
	if(tmp==&unsupplied) nTicks = 0;
	else
		nTicks = tmp->to_int();


	// Install timer control to the parent rollout
	if (active) TID = (UINT)SetTimer( parent, (UINT)control_ID, interval, (TIMERPROC)NULL);

}

BOOL
ish3_Timer::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_DESTROY:
			// Rollout page goin' down - kill the timer instance
			if(TID!=0) KillTimer( GetDlgItem(parent_rollout->page, control_ID), TID );
			TID=0;
			break;

		case WM_TIMER:
			// Timer has ticked - pass on an event to MXS
			UINT wTimerID = wParam;
			if( wTimerID == TID )
			{
				nTicks += 1;
//				init_thread_locals();
//				push_alloc_frame();
//				one_value_local(arg);
//				vl.arg = Integer::intern(nTicks);
//				call_event_handler(parent_rollout, n_tick, &vl.arg, 1);
				call_event_handler(parent_rollout, n_tick, NULL, 0);
//				pop_value_locals();
//				pop_alloc_frame();
			}
			break;
	}
	return TRUE;
}

Value*
ish3_Timer::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];
	if (prop == n_interval)
		return Integer::intern((int)interval) ;
	else if (prop == n_active)
		return active ? &true_value : &false_value;
	else if (prop == n_tick)
		return get_wrapped_event_handler(n_tick);
	else if (prop == n_ticks)
		return Integer::intern((int)nTicks) ;
	else
		return RolloutControl::get_property(arg_list, count);
}

Value*
ish3_Timer::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];
	
	if (prop == n_interval)
	{
		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			interval = val->to_int();
			if(interval<1) interval=1;

			// Only act on the timer if its enabled
			// Otherwise we're just setting the interval
			if(active)
			{
				if(TID!=0) KillTimer( GetDlgItem(parent_rollout->page, control_ID), TID );
				TID = (UINT)SetTimer( parent_rollout->page, (UINT)control_ID, interval, (TIMERPROC)NULL);
			}
		}
	}
	else if (prop == n_active)
	{
		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			active = val->to_bool();

			// Start or remove a timer using the enabled flag as a cue
			if(active)
			{
				if(TID!=0) KillTimer( GetDlgItem(parent_rollout->page, control_ID), TID );
				TID = (UINT)SetTimer( parent_rollout->page, (UINT)control_ID, interval, (TIMERPROC)NULL);
			}
			else
			{
				if(TID!=0) KillTimer( GetDlgItem(parent_rollout->page, control_ID), TID );
				TID=0;
			}
		}

	}
	else if (prop == n_ticks)
	{
		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			nTicks = val->to_int();
		}
	}
	else if (prop == n_text || prop == n_caption) // not displayed
	{
		TCHAR *text = val->to_string(); // will throw error if not convertable
		caption = val->get_heap_ptr();
	}
	else 
		return RolloutControl::set_property(arg_list, count);
	return val;
}

void
ish3_Timer::set_enable()
{
}
