/*===========================================================================*\
 |  Ishani's stuff for MAX Script R3
 |
 |  FILE:	i3.h
 |			Header file for my stuff
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) KINETIX 1998
 |			All Rights Reserved.
 |
 |  HIST:	Started 7-7-98
 | 
\*===========================================================================*/

// String Definitions
#define s_timer				_T("Timer")
#define s_timer_out			_T("Timer:%s")
//-----


// Window Classes
#define TIMER_WINDOWCLASS	_T("TIMER_WINDOWCLASS")
//-----


/*========================[ TIMER EVENT CONTROL ]===========================*\
\*===========================================================================*/

class ish3_Timer;

visible_class (ish3_Timer)

class ish3_Timer : public RolloutControl
{
public:
	UINT TID;
	int interval;
	BOOL active;
	int nTicks;

				// Constructor
				ish3_Timer(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: RolloutControl(name, caption, keyparms, keyparm_count)  
				{ 
					tag = class_tag(ish3_Timer);
					TID=0;
				}

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new ish3_Timer (name, caption, keyparms, keyparm_count); }

				classof_methods (ish3_Timer, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(s_timer_out, name->to_string()); }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCTSTR		get_control_class() { return TIMER_WINDOWCLASS; }
	void		compute_layout(Rollout *ro, layout_data* pos) { }
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
};
