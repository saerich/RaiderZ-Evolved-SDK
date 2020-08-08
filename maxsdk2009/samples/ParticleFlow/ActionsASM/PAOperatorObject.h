/**********************************************************************
 *<
	FILE:			PAOperatorObject.h

	DESCRIPTION:	SimpleSpin Operator header
					Operator to effect speed unto particles

	CREATED BY:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef _PAOPERATOROBJECT_H_
#define _PAOPERATOROBJECT_H_

#include "IPFOperator.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

#include <map>

namespace PartASM {

class PAObjectValidatorClass : public PBValidator
{
	public:
	PAObjectValidatorClass() { action=NULL; paramID=-1; }
	BOOL Validate(PB2Value &v);

	ReferenceMaker *action;
	int paramID; // parameter id in the paramblock for the list of the included items
				// list of all items that were already included is used to avoid duplicate items
};

class PAOperatorObjectLocalData
{
public:
	~PAOperatorObjectLocalData() { FreeAll(); }
	void FreeAll();
	void InitNodeGeometry(int num);
	int NodeGeometryCount() const;
	void SetNodeGeometry(int i, PFNodeGeometry* nodeGeom);
	// get nodeGeometry by index
	PFNodeGeometry* GetNodeGeometry(int i);
	// get random nodeGeometry according to probability weight
	PFNodeGeometry* GetNodeGeometry(TimeValue time, float randValue);
	// get nodeGeometry by INode passed
	PFNodeGeometry* GetNodeGeometry(INode* node);
	// get nodeGeometry with the closest surface
	PFNodeGeometry* GetClosestNodeSurface(TimeValue time, const Point3& toPoint,
									Point3& worldLocation, Point3& localLocation, int& faceIndex, int& nodeIndex);
	// get nodeGeometry with the closest vertex
	PFNodeGeometry* GetClosestNodeVertex(TimeValue time, const Point3& toPoint,
									Point3& worldLocation, int& vertexIndex, int& nodeIndex);
	// get nodeGeometry with the closest pivot
	PFNodeGeometry* GetClosestNodePivot(TimeValue time, const Point3& toPoint, int& nodeIndex);

protected:
	// const access to class members
	PFNodeGeometry* geomData(int i) const { return m_geomData[i]; }
	const Tab<PFNodeGeometry*>& geomData() const { return m_geomData; }

	// access to class members
	PFNodeGeometry*& _geomData(int i) const { return m_geomData[i]; }
	Tab<PFNodeGeometry*>& _geomData() { return m_geomData; }

private:
	Tab<PFNodeGeometry*> m_geomData;
};

class PAOperatorObject:	public PFSimpleOperator,
						public HitByNameDlgCallback 
{
public:
	PAOperatorObject();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	// From ReferenceMaker
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	// From ReferenceTarget
	RefTargetHandle Clone(RemapDir &remap);

	// From BaseObject
	TCHAR *GetObjectName();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// From IPFAction interface
	bool Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);
	bool Release(IObject* pCont);
	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const Interval ActivityInterval() const { return FOREVER; }

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

	bool	SupportRand() const { return false; }

	// From HitByNameDlgCallback
	TCHAR *dialogTitle()	{ return GetString(IDS_SELECTOBJECTS); }
	TCHAR *buttonText()		{ return GetString(IDS_SELECT); }
	BOOL useFilter()		{ return TRUE; }
	int filter(INode *node);
	BOOL useProc()			{ return TRUE; }
	void proc(INodeTab &nodeTab);

protected:
		int getNumObjects();
		// update objectsMaxscript info on basis of targets info
		bool updateFromRealObjects();
		// update objects info on basis of objectsMaxscript info
		bool updateFromMXSObjects();
		// update object names in UI and dynamic names
		bool updateObjectNames(int pblockID);

		// const access to class members
		int				numObjects()const	{ return m_numObjects; }
		const Tab<INode*>& objectsToShow()	const	{ return m_objectsToShow; }
		INode* objectToShow(int i)			const	{ return m_objectsToShow[i]; }

		// access to class members
		std::map<IObject*, PAOperatorObjectLocalData>& _localData()	{ return m_localData; }
		PAOperatorObjectLocalData& _localData(IObject* pCont) { return m_localData[pCont]; }
		int&			_numObjects()		{ return m_numObjects; }
		Tab<INode*>&	_objectsToShow()	{ return m_objectsToShow; }
		INode*&			_objectToShow(int i){ return m_objectsToShow[i]; }

private:
	static PAObjectValidatorClass validator;
	static bool validatorInitiated;
	std::map<IObject*, PAOperatorObjectLocalData>	m_localData;
	int m_numObjects;
	Tab<INode*> m_objectsToShow;
};

} // end of namespace PartASM

#endif // _PAOPERATOROBJECT_H_