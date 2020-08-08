//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// AUTHOR: Larry.Minton / David Cunningham - created Oct 2, 2006
//***********

#pragma once

#include "stdafx.h"

// 
#pragma unmanaged
#include "MAXScrpt.h"
#include "Numbers.h"
#include "parser.h"
#include "funcs.h"
#include "thunks.h"
#include "hashtab.h"
#include "macrorec.h"

#pragma managed
namespace MXS_dotNet	{

	// Note that this functionality should probably be added to CStr/WStr at some point
	struct MNETStr
	{
		static MSTR ToMSTR(System::String^);
		static MSTR ToMSTR(System::Exception^, bool InnerException = true);
	};

	/* -------------------- event_delegate_pair  ------------------- */
	// 

	public ref class event_delegate_pair
	{
	public:
		System::String^ m_pEventName;
		System::Delegate^ m_pEventDelegate;
		event_delegate_pair(System::String^ pEventName, System::Delegate^ pEventDelegate)
		{
			m_pEventName = pEventName;
			m_pEventDelegate = pEventDelegate;
		}
	};
}