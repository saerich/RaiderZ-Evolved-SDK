/**********************************************************************
 *<
	FILE:			PFOperatorMaterial.h

	DESCRIPTION:	Base class for Material Operators -
					MaterialStatic, MaterialDynamic and MaterialFrequency

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 2008-06-22

	*>	Copyright 2008 Autodesk, Inc.  All rights reserved.

	Use of this software is subject to the terms of the Autodesk license
	agreement provided at the time of installation or download, or which
	otherwise accompanies this software in either electronic or hard copy form.  

 **********************************************************************/

#pragma once

#include <set>

#include "IPFOperator.h"
#include "IPFActionState.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

// forward declare
class IChannelContainer;

namespace PFActions {

// reference IDs
enum {	kMaterial_reference_pblock,
		kMaterial_reference_material,
		kMaterial_reference_num = 2 
};

// forward declare
class PFOperatorMaterialDADMgr;

class PFOperatorMaterial:	public PFSimpleOperator 
{
public:
	PFOperatorMaterial(void);
	virtual ~PFOperatorMaterial();

	// From Animatable
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	// From ReferenceMaker
	int NumRefs() { return kMaterial_reference_num; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const Interval ActivityInterval() const { return FOREVER; }

	bool	SupportRand() const	{ return true; }
	int		GetRand();
	void	SetRand(int seed);

	bool	IsMaterialHolder() const;
	Mtl*	GetMaterial();
	bool	SetMaterial(Mtl* mtl);

protected:
	virtual int assignMtlParamID(void) const = 0;
	virtual int mtlParamID(void) const = 0;
	virtual int numSubMtlsParamID(void) const = 0;
	virtual int randParamID(void) const = 0;

	bool isSelfCreator(IObject* pCont, IChannelContainer* chCont, INode* actionNode);
	bool verifyMtlsMXSSync();
	virtual bool updateFromRealMtl();
	bool updateFromMXSMtl();

	void resetCTWOs();
	void addCTWO( IObject* pCont );
	bool hasCTWO( IObject* pCont );

	// const access to class members
	const PFOperatorMaterialDADMgr*	dadMgr()			const	{ return m_dadMgr; }
	TSTR							lastDynamicName()	const	{ return m_lastDynamicName; }
	const Mtl*						material()			const	{ return m_material; }
	bool							updateMtlInProgress() const { return m_updateMtlInProgress; }

	// access to class members
	PFOperatorMaterialDADMgr*&	_dadMgr()				{ return m_dadMgr; }
	TSTR&						_lastDynamicName()		{ return m_lastDynamicName; }
	Mtl*&						_material()				{ return m_material; }
	std::set<IObject*>&			_containersToWorkOn()	{ return m_containersToWorkOn; }
	bool&						_updateMtlInProgress()	{ return m_updateMtlInProgress; }

protected:
	PFOperatorMaterialDADMgr*	m_dadMgr;	// material DAD manager
	Mtl* m_material;			// material
	TSTR m_lastDynamicName;
	std::set<IObject*> m_containersToWorkOn;
	bool m_updateMtlInProgress;
};


// drag-and-drop manager
class PFOperatorMaterialDADMgr : public DADMgr
{
public:
	PFOperatorMaterialDADMgr(PFOperatorMaterial* op) : m_operator(op) {}

	// called on the draggee to see what if anything can be dragged from this x,y
	SClass_ID GetDragType(HWND hwnd, POINT p) { return MATERIAL_CLASS_ID; }
	// called on potential dropee to see if can drop type at this x,y
	BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew);
	int SlotOwner() { return OWNER_SCENE; }  
	ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
	void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr* srcMgr = NULL, BOOL bSrcClone = FALSE);
	BOOL AutoTooltip() { return TRUE; }

private:
	PFOperatorMaterialDADMgr() : m_operator(NULL) {}

	const PFOperatorMaterial*	op() const	{ return m_operator; }
	PFOperatorMaterial*&		_op()		{ return m_operator; }

	PFOperatorMaterial*			m_operator;
};


} // end of namespace PFActions
