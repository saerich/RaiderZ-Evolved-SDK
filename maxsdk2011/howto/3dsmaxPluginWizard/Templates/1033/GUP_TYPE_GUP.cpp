//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Appwizard generated plugin
// AUTHOR: 
//***************************************************************************/

#include "[!output PROJECT_NAME].h"

#define [!output CLASS_NAME]_CLASS_ID	Class_ID([!output CLASSID1], [!output CLASSID2])


class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:

		static HWND hParams;

		// GUP Methods
		DWORD		Start			( );
		void		Stop			( );
		DWORD_PTR	Control			( DWORD parameter );
		
		// Loading/Saving
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		//Constructor/Destructor
		[!output CLASS_NAME]();
		~[!output CLASS_NAME]();		
};


[!output TEMPLATESTRING_CLASSDESC]

[!if PARAM_MAPS != 0]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif] 

[!output CLASS_NAME]::[!output CLASS_NAME]()
{

}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}

// Activate and Stay Resident
DWORD [!output CLASS_NAME]::Start( ) {
	
	#pragma message(TODO("Do plugin initialization here"))
	
	#pragma message(TODO("Return if you want remain loaded or not"))
	return GUPRESULT_KEEP;
}

void [!output CLASS_NAME]::Stop( ) {
	#pragma message(TODO("Do plugin un-initialization here"))
}

DWORD_PTR [!output CLASS_NAME]::Control( DWORD parameter ) {
	return 0;
}

IOResult [!output CLASS_NAME]::Save(ISave *isave)
{
	return IO_OK;
}

IOResult [!output CLASS_NAME]::Load(ILoad *iload)
{
	return IO_OK;
}

