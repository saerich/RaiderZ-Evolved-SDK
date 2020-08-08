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
// AUTHOR: David Cunningham - created Oct 16, 2006
//***************************************************************************/
//

#include "maxCDotNetHostWnd.h"
#include "maxDotNetControl.h"

using namespace MXS_dotNet;

#pragma managed

maxCDotNetHostWnd::maxCDotNetHostWnd(maxDotNetControl* aMaxDotNetControl) :
	mMaxDotNetControl(aMaxDotNetControl)
{
	
}

maxCDotNetHostWnd::~maxCDotNetHostWnd()	{

}

Value* maxCDotNetHostWnd::GetMXSContainer()	{
	return NULL;
}

void maxCDotNetHostWnd::ProcessEvent(System::String ^eventName, System::Object ^delegateArgs)	{
	// do nothing for now
	int i=0;
}


void maxCDotNetHostWnd::DoOnDestroy()	{

}
void maxCDotNetHostWnd::DoRegisterControl()	{
	
}

maxCDotNetHostWnd::inherited::delegate_proxy_type_base^ maxCDotNetHostWnd::GetProxyType()	{
	return m_delegate_map_proxy.get_proxy(this);
}

void maxCDotNetHostWnd::LoadCreateControlParams()	{

}


maxCDotNetHostWnd::delegate_proxy_type::delegate_proxy_type(maxCDotNetHostWnd* pNativeTarget)	:
	inherited::delegate_proxy_type_base(pNativeTarget)
{
	
}

maxCDotNetHostWnd::delegate_proxy_type::~delegate_proxy_type()	{
}



bool maxCDotNetHostWnd::CreateControl(HWND hParent, 
									  int id, 
									  int x, 
									  int y, 
									  int w, 
									  int h, 
									  System::Windows::Forms::Control^ aControl)	
{
	bool result = DoCreateControl(hParent, NULL, id, x, y, w, h, aControl);
	return result;
}



