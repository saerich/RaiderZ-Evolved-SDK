#include "SubAnimEnumProc.h"
#include <control.h>

// constructor
SubAnimEnumProc::SubAnimEnumProc(Control* c, int scope)
	: mCtrl(c), 
	  mFound(false)
{
	SetScope(scope);
}

//! callback method for each Animatable in the sub-anim tree. 
int SubAnimEnumProc::proc(Animatable* anim, Animatable* /*client*/, int /*subNum*/)
{
	Animatable* temp = dynamic_cast<Animatable*>( mCtrl ); // cast to avoid a 2440 conversion warning
	if (temp == anim)
	{
		mFound = true;
		// No need to keep going, so just bail out of here.
		return ANIM_ENUM_ABORT;
	}

	return ANIM_ENUM_PROCEED;
}

//! Get's whether the control was found or not.
bool SubAnimEnumProc::FoundControl()
{
	return mFound;
}
