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
// AUTHOR: David Cunningham - created Oct 27, 2006
//***************************************************************************/
//

#pragma once

#include "maxDotNetControl.h"

#pragma managed

namespace MXS_dotNet	{

	public class maxDotNetButton : public maxDotNetControl	{
	private:
		maxDotNetButton();
	
	protected:

		virtual void InitControl();

	public:
		maxDotNetButton(HWND aHandle, int id);
		virtual ~maxDotNetButton();
	};


}