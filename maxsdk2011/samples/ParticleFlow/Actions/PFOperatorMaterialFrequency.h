/**********************************************************************
 *<
	FILE:			PFOperatorMaterialFrequency.h

	DESCRIPTION:	MaterialFrequency Operator header
					Operator to assign mark shape to particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

	*>	Copyright 2008 Autodesk, Inc.  All rights reserved.

	Use of this software is subject to the terms of the Autodesk license
	agreement provided at the time of installation or download, or which
	otherwise accompanies this software in either electronic or hard copy form.  

 **********************************************************************/

#ifndef _PFOPERATORMATERIALFREQUENCY_H_
#define _PFOPERATORMATERIALFREQUENCY_H_

#include "IPFOperator.h"
#include "IPFActionState.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"
#include "PFOperatorMaterial.h"

namespace PFActions {

// operator state
class PFOperatorMaterialFrequencyState:	public IObject, 
										public IPFActionState
{		
public:
	// from IObject interface
	int NumInterfaces() const { return 1; }
	BaseInterface* GetInterfaceAt(int index) const { return ((index == 0) ? (IPFActionState*)this : NULL); }
	BaseInterface* GetInterface(Interface_ID id);
	void DeleteIObject();

	// From IPFActionState
	Class_ID GetClassID();
	ULONG GetActionHandle() const { return actionHandle(); }
	void SetActionHandle(ULONG handle) { _actionHandle() = handle; }
	IOResult Save(ISave* isave) const;
	IOResult Load(ILoad* iload);

protected:
	friend class PFOperatorMaterialFrequency;

		// const access to class members
		ULONG		actionHandle()	const	{ return m_actionHandle; }		
		const RandGenerator* randGen()	const	{ return &m_randGen; }
		BOOL		activeCTWO()	const	{ return m_activeCTWO; }

		// access to class members
		ULONG&			_actionHandle()		{ return m_actionHandle; }
		RandGenerator*	_randGen()			{ return &m_randGen; }
		BOOL&			_activeCTWO()		{ return m_activeCTWO; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGen;
		BOOL m_activeCTWO;
};

class PFOperatorMaterialFrequency:	public PFOperatorMaterial 
{
public:
	PFOperatorMaterialFrequency();
	~PFOperatorMaterialFrequency();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();

	// From ReferenceMaker
	RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message);

	// From ReferenceTarget
	RefTargetHandle Clone(RemapDir &remap);

	// From BaseObject
	TCHAR *GetObjectName();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// From IPFAction interface
	bool Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);
	bool Release(IObject* pCont);

	IObject* GetCurrentState(IObject* pContainer);
	void SetCurrentState(IObject* actionState, IObject* pContainer);

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

protected:
	bool updateFromRealMtl();

	// from PFOperatorMaterial
	int assignMtlParamID(void) const;
	int mtlParamID(void) const;
	int numSubMtlsParamID(void) const;
	int randParamID(void) const;

private:
	// const access to class members
	int		lastNumSubs()	const	{ return m_lastNumSubs; }

	// access to class members
	int&	_lastNumSubs()	{ return m_lastNumSubs; }

private:
	int		m_lastNumSubs;
};

} // end of namespace PFActions

#endif // _PFOPERATORMATERIALFREQUENCY_H_