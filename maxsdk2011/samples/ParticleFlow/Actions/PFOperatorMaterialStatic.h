/**********************************************************************
 *<
	FILE:			PFOperatorMaterialStatic.h

	DESCRIPTION:	MaterialStatic Operator header
					Operator to assign mark shape to particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

	*>	Copyright 2008 Autodesk, Inc.  All rights reserved.

	Use of this software is subject to the terms of the Autodesk license
	agreement provided at the time of installation or download, or which
	otherwise accompanies this software in either electronic or hard copy form.  

 **********************************************************************/

#ifndef _PFOPERATORMATERIALSTATIC_H_
#define _PFOPERATORMATERIALSTATIC_H_

#include <map>

#include "IPFOperator.h"
#include "IPFActionState.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"
#include "PFOperatorMaterial.h"

namespace PFActions {

// operator state
class PFOperatorMaterialStaticState:	public IObject, 
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
	friend class PFOperatorMaterialStatic;

		// const access to class members
		ULONG		actionHandle()	const	{ return m_actionHandle; }		
		const RandGenerator* randGen()	const	{ return &m_randGen; }
		float cycleOffset()				const	{ return m_cycleOffset; }
		TimeValue offsetTime()			const	{ return m_offsetTime; }
		BOOL		activeCTWO()	const	{ return m_activeCTWO; }

		// access to class members
		ULONG&			_actionHandle()		{ return m_actionHandle; }
		RandGenerator*	_randGen()			{ return &m_randGen; }
		float&			_cycleOffset()		{ return m_cycleOffset; }
		TimeValue&		_offsetTime()		{ return m_offsetTime; }
		BOOL&			_activeCTWO()		{ return m_activeCTWO; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGen;		
		float m_cycleOffset;		
		TimeValue m_offsetTime;
		BOOL m_activeCTWO;
};

class PFOperatorMaterialStatic:	public PFOperatorMaterial
{
public:
	PFOperatorMaterialStatic();
	~PFOperatorMaterialStatic();

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
	// from PFOperatorMaterial
	int assignMtlParamID(void) const;
	int mtlParamID(void) const;
	int numSubMtlsParamID(void) const;
	int randParamID(void) const;

private:
	// access to class members
	float&						_cycleOffset(IObject* pCont)	{ return m_cycleOffset[pCont]; }
	std::map<IObject*, float>&	_cycleOffset()					{ return m_cycleOffset; }
	TimeValue&					_offsetTime(IObject* pCont)		{ return m_offsetTime[pCont]; }
	std::map<IObject*, TimeValue>& _offsetTime()				{ return m_offsetTime; }
	Tab<int>&					_randMtlIndex(IObject* pCont)	{ return m_randMtlIndex[pCont]; }
	std::map<IObject*, Tab<int> >& _randMtlIndex()				{ return m_randMtlIndex; }

protected:
	std::map<IObject*, float> m_cycleOffset; // cycle offset for a current particle
	std::map<IObject*, TimeValue> m_offsetTime; // timing for the cycle offset
	std::map<IObject*, Tab<int> > m_randMtlIndex; 
};

} // end of namespace PFActions

#endif // _PFOPERATORMATERIALSTATIC_H_