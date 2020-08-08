// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "DayMasterClassDesc.h"
#include "SunMasterCreateMode.h"
#include "sunclass.h"
#include "sunlight.h"

#pragma managed
namespace 
{

	void ReplaceChar( TSTR& str, TCHAR searchChar, TCHAR replaceChar )
	{
		for( int i=0; i < str.Length(); i++ )
		{
			if( str[i] == searchChar ) str[i] = replaceChar;
		}
	}

	// Checks if an appropriate exposure control is currently applied, for this light type.
	// If not, the user is prompted with the option to automatically apply an approriate exposure control.
	// This function calls the underlying function implemented in MaxScript.
	// TO DO: Remove duplicated function in photometric Free Lights and Target Lights.
	void ValidateExposureControl(ClassDesc* classDesc)
	{
		TSTR className = classDesc->ClassName();
		ReplaceChar( className, _T(' '), _T('_') ); // Repalce spaces with underscores

		// Set up the command to execute in MaxScript
		TSTR execString;
		execString.printf( _T("ValidateExposureControlForLight %s"), className );

		// Execute the command in MaxScript
		FPValue retval;
		BOOL quietErrors = TRUE;
		ExecuteMAXScriptScript( execString, quietErrors, &retval );
	}

}

int DayMasterClassDesc::IsPublic() 
{ 
	return 1; 
}

void * DayMasterClassDesc::Create(BOOL) 
{ 
	return new SunMaster(true); 
}

// This method returns the name of the class.  This name appears 
// in the button for the plug-in in the MAX user interface.
const TCHAR * DayMasterClassDesc::ClassName() 
{
	return MaxSDK::GetResourceString(IDS_DAY_CLASS);
}

SClass_ID DayMasterClassDesc::SuperClassID() 
{ 
	return SYSTEM_CLASS_ID; 
} 

Class_ID DayMasterClassDesc::ClassID() 
{ 
	return DAYLIGHT_CLASS_ID; 
}

const TCHAR* DayMasterClassDesc::Category() 
{ 
	return _T("");  
}

// This is the method of the class descriptor that actually begins the 
// creation process in the viewports.
int DayMasterClassDesc::BeginCreate(Interface *i)
{
	ValidateExposureControl(this);

	// Save the interface pointer passed in.  This is used to call 
	// methods provided by MAX itself.
	IObjCreate *iob = i->GetIObjCreate();

	SunMasterCreateMode::GetInstance()->Begin( iob, this, true );
	// Set the current command mode to the SunMasterCreateMode.
	iob->PushCommandMode( SunMasterCreateMode::GetInstance() );

	return TRUE;
}

// This is the method of the class descriptor that terminates the 
// creation process.
int DayMasterClassDesc::EndCreate(Interface *i)
{
	SunMasterCreateMode::GetInstance()->End();
	// Remove the command mode from the command stack.
	i->RemoveMode( SunMasterCreateMode::GetInstance() );
	return TRUE;
}
