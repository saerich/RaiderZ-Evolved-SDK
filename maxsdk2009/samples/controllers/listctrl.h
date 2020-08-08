/*****************************************************************************

	FILE: listctrl.h

	DESCRIPTION: Classes and functions pulled out of listctrl.cpp so the layer controller can also use them.
	
	CREATED BY:	Michael Zyracki

	HISTORY:	December 9th, 2005	Creation

 	Copyright (c) 2005, All Rights Reserved.
 *****************************************************************************/

#ifndef __LISTCTRL__H
#define __LISTCTRL__H


#include "decomp.h"
#include "iparamb2.h"
#include "istdplug.h"
#include "notify.h"


typedef Tab<Control*> ControlTab;

class NameList : public Tab<TSTR*> {
	public:
		void Free() {
			for (int i=0; i<Count(); i++) {
				delete (*this)[i];
				(*this)[i] = NULL;
				}
			}
		void Duplicate() {
			for (int i=0; i<Count(); i++) {
				if ((*this)[i]) (*this)[i] = new TSTR(*(*this)[i]);
				}
			}
	};

class ListControl;
class ListDummyEntry : public Control {
	public:
		Control *lc;
		void Init(Control *l);
		Class_ID ClassID() {return Class_ID(DUMMY_CONTROL_CLASS_ID,0);}
		SClass_ID SuperClassID();
		void GetClassName(TSTR& s) {s=_T("");}
		RefResult AutoDelete() {return REF_SUCCEED;}
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message) {return REF_SUCCEED;}
		RefTargetHandle Clone(RemapDir &remap) {BaseClone(this,this,remap); return this;}

		void Copy(Control *from) {}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE) {}

		BOOL CanCopyAnim() {return FALSE;}
		int IsKeyable() {return FALSE;}
	};




#endif __LIMITCTRL__H
