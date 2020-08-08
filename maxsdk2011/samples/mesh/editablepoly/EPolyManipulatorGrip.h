/*==============================================================================
Copyright 2010 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
or which otherwise accompanies this software in either electronic or hard copy form.   
*****************************/
// DESCRIPTION: Interfaces for select and manipulate grip items
// AUTHOR: Michael Zyracki created 2009
//***************************************************************************/

#pragma once
#include "iFnPub.h"


//! \brief Interface ID for the EPolyManipulatorGrip 
//! \see EPolyManipulatorGrip 
#define IEPOLYMANIPGRIP_INTERFACE Interface_ID(0x19ce513c, 0x7b4a5132)

//! Helper method for getting the EPolyManipulatorGrip interface
#define GetEPolyManipulatorGrip()	static_cast<EPolyManipulatorGrip*>(GetCOREInterface(IEPOLYMANIPGRIP_INTERFACE))

//! This class provides functionality for turning on and off various grips that may show up when
//! select and manipulate is turned on in editable poly and the edit poly modifier
class EPolyManipulatorGrip : public FPStaticInterface {

public:
	/*! \defgroup ManipulateGrips  Manipulate Grip Items
		The different manipulate mode grip items that can be toggled on or off.	
	*/
	//@{
	enum ManipulateGrips
	{
		//! Soft Selection Falloff grip.  May be on for all subobject modes.
		eSoftSelFalloff = 0,
		//! Soft Selection Pinch grip.  May be on for all subobject modes.
		eSoftSelPinch,
		//! Soft Selection Bubble grip.  May be on for all subobject modes.
		eSoftSelBubble,
		//! Set Flow grip.  Only in edge subobject mode.
		eSetFlow,
		//! Loop Shift Selection Falloff grip.  Only in edge subobject mode.
		eLoopShift,
		//! Ring Shift Selection Falloff grip.  Only in edge subobject mode.
		eRingShift,
		//! Edge Crease grip.  Only in edge subobject mode and only in editable poly.
		eEdgeCrease,
		//! Edge Weight grip.  Only in edge subobject mode and only in editable poly.
		eEdgeWeight,
		//! Vertex Weight grip.  Only in vertex subobject mode and only available in editable poly.
		eVertexWeight

	};
	//@}

	//! \brief Turns the manipulate grip item on or off. When at least one grip item is on and 
	//! select and manipulate mode is on a grip will show up for each item that's on.
	//! \param[in] on - Whether or not the specified item will be turned on or off
	//! \param[in] item - Which grip item that will be turned on or off.
	virtual void SetManipulateGrip(bool on, ManipulateGrips item) =0;

	//! \brief Get whether or not this manipulator item is on or not. 
	//! \param[in] item - Which grip item we are checking to see if it's on or off.
	//! \return Whether or not the item is on or not.  Note that it doesn't
	//! take into account whether or not the correct subobject mode is also active, it just 
	//! checks to see if it would be on if we were in the right mode.
	virtual bool GetManipulateGrip(ManipulateGrips item) = 0;

};