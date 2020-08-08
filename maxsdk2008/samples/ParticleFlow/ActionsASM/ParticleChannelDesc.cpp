/**********************************************************************
 *<
	FILE:			ParticleChannelDesc.cpp

	DESCRIPTION:	ParticleChannel-generic Class Descriptor (definition)
					ParticleChannel-specific Class Descriptors (definition)
					It's not a part of the SDK
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 10-16-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "StdAfx.h"
#include "resource.h"

#include "ParticleChannelDesc.h"

#include "PartAsm_GlobalVariables.h"
#include "PartAsm_GlobalFunctions.h"
//#include "PFChannels_SysUtil.h"
#include "PFClassIDs.h"
#include "ParticleChannelMult.h"

namespace PartASM {

// generic Particle Channel Class Descriptor
const TCHAR* ParticleChannelDesc::ClassName() 
{
	return _T(""); // doesn't appear as a button anywhere
}

SClass_ID ParticleChannelDesc::SuperClassID() 
{
	return REF_TARGET_CLASS_ID;
}

Class_ID ParticleChannelDesc::SubClassID()
{
	return ParticleChannelSubClassID;
}

const TCHAR* ParticleChannelDesc::Category() 
{
	return _T(""); // doesn't appear as a button anywhere
}

//----------------------------------------------------//
// specific Particle Channel Class Descriptors		  //
//----------------------------------------------------//

// ParticleChannelMult Class Descriptor
void* ParticleChannelMultDesc::Create(BOOL loading) 
{
	return new ParticleChannelMult();
}

Class_ID ParticleChannelMultDesc::ClassID()
{
	return ParticleChannelMult_Class_ID;
}

const TCHAR* ParticleChannelMultDesc::InternalName()
{
	return _T( "ParticleChannelMult" );
}




} // end of namespace PFChannels