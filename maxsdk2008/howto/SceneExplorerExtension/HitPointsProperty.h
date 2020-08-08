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

#ifndef HIT_POINTS_PROPERTY_H
#define HIT_POINTS_PROPERTY_H

#pragma unmanaged
#include "nativeInclude.h"

#pragma managed
// Declare references to .NET assemblies.  Notice that it doesn't matter if
// an assembly is originally written in C++/CLI or C#, we use it same way.  
// The only difference from an external point of view is that C++/CLI may be
// aware of native types.

// SceneExplorer.dll is a 3ds Max .NET assembly written in C++/CLI.
#using <SceneExplorer.dll>
// ExplorerFramework is a 3ds Max .NET assembly written in C#.
#using <ExplorerFramework.dll>
// Microsoft .NET Framework assembly.
#using <System.dll>

namespace SceneExplorerExtension
{

// This is a managed class.  Note the 'ref' keyword.
public ref class HitPointsProperty : SceneExplorer::INodePropertyDescriptor<int>
{
public:
	// Factory method, builds the property with descriptors
	static System::ComponentModel::PropertyDescriptor^ Create();

	// Registers this property with the Scene Explorer's registry
	static void RegisterProperty();

	~HitPointsProperty();

protected:
	// implement the accessor operations
	virtual System::Object^ DoINodeGetValue(INode *node) override;
	virtual void DoINodeSetValue(INode* node, int value) override;

private:
	// Client code should use the factory method instead.
	HitPointsProperty();

	// This would be more efficient as a static constant, but we're trying to 
	// avoid the crash on DLL unload issue.
	// For a managed class to have a native type as a data member, the native 
	// data member must be a reference or a pointer.
	MSTR* mUserPropertyKey;
};

}


#endif