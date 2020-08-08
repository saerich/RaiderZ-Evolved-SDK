/**********************************************************************
 *<
	FILE:			PFOperatorMaterial.cpp

	DESCRIPTION:	Base class for Material Operators -
					MaterialStatic, MaterialDynamic and MaterialFrequency

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 2008-06-22

	*>	Copyright 2008 Autodesk, Inc.  All rights reserved.

	Use of this software is subject to the terms of the Autodesk license
	agreement provided at the time of installation or download, or which
	otherwise accompanies this software in either electronic or hard copy form.  

 **********************************************************************/


#include "max.h"
#include "stdmat.h"
#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorMaterial.h"

#include "IParticleObjectExt.h"
#include "IChannelContainer.h"
#include "IParticleChannel.h"
#include "PFMessages.h"
#include "IPViewManager.h"
#include "IParticleChannelMaterialIndex.h"

namespace PFActions {

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorMaterial							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFOperatorMaterial::PFOperatorMaterial()
	:	m_material(NULL)
	,	m_updateMtlInProgress(false)
{
	_dadMgr() = new PFOperatorMaterialDADMgr(this);
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorMaterial								 |
//+--------------------------------------------------------------------------+
PFOperatorMaterial::~PFOperatorMaterial()
{
	if (dadMgr()) 
		delete _dadMgr();
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterial::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterial::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PFOperatorMaterial::GetReference(int i)
{
	switch (i)
	{
	case kMaterial_reference_pblock:	
		return (RefTargetHandle)pblock();
	case kMaterial_reference_material:	
		return (RefTargetHandle)material();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterial::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case kMaterial_reference_pblock:	
		_pblock() = (IParamBlock2*)rtarg;	
		break;
	case kMaterial_reference_material:	
		_material() = (Mtl *)rtarg;	
		if (updateFromRealMtl()) 
		{
			NotifyDependents(FOREVER, PART_MTL, kPFMSG_UpdateMaterial, NOTIFY_ALL, TRUE);
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
		}
		break;
	}
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
const ParticleChannelMask& PFOperatorMaterial::ChannelsUsed(const Interval& time) const
{
								// read channels				// write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time, PCU_MtlIndex);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
int	PFOperatorMaterial::GetRand()
{
	return pblock()->GetInt(randParamID(), 0);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterial::SetRand(int seed)
{
	pblock()->SetValue(randParamID(), 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFOperatorMaterial::IsMaterialHolder() const
{
	return (pblock()->GetInt(assignMtlParamID(), 0) != 0);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
Mtl* PFOperatorMaterial::GetMaterial()
{
	return _material();
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFOperatorMaterial::SetMaterial(Mtl* mtl)
{
	bool wasUpdateInProgress = updateMtlInProgress();
	if (!wasUpdateInProgress)
	{
		_updateMtlInProgress() = true;
		pblock()->SetValue(mtlParamID(), 0, mtl);
	}

	bool res(true);
	if (!verifyMtlsMXSSync())
	{
		if (mtl == NULL) {
			res = DeleteReference(kMaterial_reference_material) == REF_SUCCEED;
		} else {
			res = ReplaceReference(kMaterial_reference_material, mtl) == REF_SUCCEED;
		}
	}

	if (!wasUpdateInProgress)
		_updateMtlInProgress() = false;
	return res;
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorMaterial							 |
//+--------------------------------------------------------------------------+
bool PFOperatorMaterial::isSelfCreator(IObject* pCont, IChannelContainer* chCont, 
									   INode* actionNode)
{
	// it may happen that the materialIndex channel was created by another Material operator
	// in this case the channel has to be re-initialized
	IObject* mtlIDChannel = chCont->GetChannel(PARTICLECHANNELMTLINDEXR_INTERFACE);
	if (mtlIDChannel != NULL) 
	{
		IParticleChannel* iChannel = GetParticleChannelInterface(mtlIDChannel);
		if (iChannel != NULL) 
		{
			INode* creatorNode = iChannel->GetCreatorAction();
			if (creatorNode != actionNode) 
			{
				if (GetPFObject(creatorNode->GetObjectRef()) != (Object*)this) 
				{
					// the creator of the channel is another operator
					// hence the data need to be re-initialized
					iChannel->SetCreatorAction(actionNode);
					addCTWO( pCont );
				}
				else
				{
					// the creator of the channel is the same operator
					// with a different instance node. The material indices
					// were already calculated - nothing to do further
					return false;
				}
			}
		}
	}
	return hasCTWO( pCont );
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorMaterial							 |
//+--------------------------------------------------------------------------+
bool PFOperatorMaterial::verifyMtlsMXSSync()
{
	if (pblock() == NULL) 
		return true;

	Mtl* pblockMtl = pblock()->GetMtl(mtlParamID(), 0);
	Mtl* refMtl = _material();
	if ((refMtl == NULL) || (pblockMtl == NULL))
		return (refMtl == NULL) && (pblockMtl == NULL);

	return (refMtl == pblockMtl);
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorMaterial							 |
//+--------------------------------------------------------------------------+
bool PFOperatorMaterial::updateFromRealMtl()
{
	if (!pblock()) 
		return false;
	if (theHold.RestoreOrRedoing()) 
		return true;
	if (updateMtlInProgress()) 
		return false;
	if (verifyMtlsMXSSync()) 
		return false;

	_updateMtlInProgress() = true;
	if ((pblock()->GetMtl(mtlParamID()) == NULL) && (numSubMtlsParamID() >= 0))
	{
		int numSubs = _material()->NumSubMtls();
		pblock()->SetValue(numSubMtlsParamID(), 0, numSubs);
	}
	pblock()->SetValue(mtlParamID(), 0, _material());
	_updateMtlInProgress() = false;
	return true;
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorMaterial							 |
//+--------------------------------------------------------------------------+
bool PFOperatorMaterial::updateFromMXSMtl()
{
	if (!pblock()) 
		return false;
	if (theHold.RestoreOrRedoing()) 
		return true;
	if (updateMtlInProgress()) 
		return false;
	if (verifyMtlsMXSSync()) 
		return false;

	_updateMtlInProgress() = true;
	SetMaterial(pblock()->GetMtl(mtlParamID(), 0));	
	_updateMtlInProgress() = false;
	return true;
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorMaterial							 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterial::resetCTWOs()
{
	_containersToWorkOn().clear();
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorMaterial							 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterial::addCTWO( IObject* pCont )
{
	_containersToWorkOn().insert( pCont );
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorMaterial							 |
//+--------------------------------------------------------------------------+
bool PFOperatorMaterial::hasCTWO( IObject* pCont )
{
	return ( _containersToWorkOn().find( pCont ) != _containersToWorkOn().end() );
}

//+--------------------------------------------------------------------------+
//|							Drag-And-Drop Manager							 |
//+--------------------------------------------------------------------------+
BOOL PFOperatorMaterialDADMgr::OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew)
{
	if (hfrom == hto) 
		return FALSE;
	return (type==MATERIAL_CLASS_ID) ? TRUE : FALSE;
}

ReferenceTarget *PFOperatorMaterialDADMgr::GetInstance(HWND hwnd, POINT p, SClass_ID type)
{
	if (op() == NULL) 
		return NULL;

	return _op()->GetMaterial();
}

void PFOperatorMaterialDADMgr::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr* srcMgr, BOOL bSrcClone)
{
	Mtl *mtl = (Mtl *)dropThis;

	if (op() && mtl) 
	{
		theHold.Begin();
		_op()->SetMaterial(mtl);
		theHold.Accept(GetString(IDS_PARAMETERCHANGE));
	}
}

} // end of namespace PFActions