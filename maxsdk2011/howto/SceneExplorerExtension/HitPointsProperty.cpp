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
// AUTHOR: Nicolas Desjardins
// DATE: 2007-07-24
//***************************************************************************/

#include "hitPointsProperty.h"

#using <MaxExplorerBindings.dll>

using namespace SceneExplorer;

namespace SceneExplorerExtension
{

HitPointsProperty::HitPointsProperty() :
	INodePropertyDescriptor("Hit Points"),
	mUserPropertyKey(new MSTR("HitPoints"))
{ }

HitPointsProperty::~HitPointsProperty()
{ 
	delete mUserPropertyKey;
	mUserPropertyKey = nullptr;
}

System::Object^ HitPointsProperty::DoINodeGetValue(INode *node)
{
	int hitPoints = -1;
	if( node->GetUserPropInt(*mUserPropertyKey, hitPoints) )
	{
		return hitPoints;
	}
	else
	{
		return nullptr;
	}
}


void HitPointsProperty::DoINodeSetValue(INode* node, int value)
{
	node->SetUserPropInt(*mUserPropertyKey, value);
}

System::ComponentModel::PropertyDescriptor^ HitPointsProperty::Create()
{
	// create a hit points property decorated with the UndoablePropertyDecorator
	// to support undo and redo for edits in the scene explorer.
	return MaxExplorerBindings::UndoablePropertyDecorator::Create( gcnew HitPointsProperty() );
}

void HitPointsProperty::RegisterProperty()
{
	// Create a HitPointsProperty and register it with the Scene Explorer's 
	// property registry.  It will be available from the Column Chooser.
	NodePropertyRegistry::GetInstance()->AddProperty(Create());
}


}