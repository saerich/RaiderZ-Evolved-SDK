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
// AUTHOR: Larry.Minton - created Feb 6, 2009
//***************************************************************************/
//

#include "stdafx.h"

namespace MXS_dotNet
{

// Note: this class isn't used by this project, but is exposed in the MXS_dotNet namespace for use by scripts

/* -------------------- ListViewItemComparer  ------------------- */
// 
#pragma managed

	// Implements the manual sorting of items by columns.
	public ref class ListViewItemComparer: public System::Collections::IComparer
	{
	private:
		int col;

	public:
		ListViewItemComparer()
		{
			col = 0;
		}

		ListViewItemComparer( int column )
		{
			col = column;
		}

		virtual int Compare( Object^ x, Object^ y )
		{
			using namespace System::Windows::Forms;
			return System::String::Compare( 
				(dynamic_cast<ListViewItem^>(x))->SubItems[ col ]->Text,
				(dynamic_cast<ListViewItem^>(y))->SubItems[ col ]->Text );
		}
	};

};
