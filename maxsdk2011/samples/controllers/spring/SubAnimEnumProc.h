#pragma once

#include <AnimEnum.h>
// forward declarations
class Control;

/*! Enumerator to go down a Animatable Tree to find a control pointer */
class SubAnimEnumProc : public AnimEnum
{
protected:
	// The controller you want to find
	Control* mCtrl;
	bool mFound;
public:
	//! Constructor
	SubAnimEnumProc(Control* c, int scope);
	//! Called for each Animatable in the sub-anim tree. 
	int proc(Animatable* anim, Animatable* client, int subNum);
	//! Get's whether the control was found or not. Call this after the 
	//! enumeration is complete
	bool FoundControl();
};