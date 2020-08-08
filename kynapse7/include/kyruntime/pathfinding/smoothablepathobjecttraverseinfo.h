/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




// primary contact: LAPA - secondary contact: NOBODY
#ifndef KyRuntime_SmoothablePathObjectTraverseInfo_H
#define KyRuntime_SmoothablePathObjectTraverseInfo_H


namespace Kaim
{

class SmoothablePathObjectTraverseInfo
{
	KY_DEFINE_NEW_DELETE_OPERATORS;

public:
	enum Status
	{
		Unset = 0,
		Approach,
		Traverse,
		DUMMY_ = KYFORCEENUMSIZEINT
	};

	SmoothablePathObjectTraverseInfo():
		m_pathObject(KY_NULL),
		m_status(Unset)
	{
		m_entryPoint.SetInfo(m_entryVertexSafePtr);
		m_exitPoint.SetInfo(m_exitVertexSafePtr);
	}

	SmoothablePathObjectTraverseInfo(IPathObject* pathObject, const GraphVertexSafePtr& entryVertexPtr, const GraphVertexSafePtr& exitVertexPtr):
		m_pathObject(KY_NULL),
		m_status(Unset)
	{
		m_entryPoint.SetInfo(m_entryVertexSafePtr);
		m_exitPoint.SetInfo(m_exitVertexSafePtr);

		SetPathObject(pathObject, entryVertexPtr, exitVertexPtr);
	}

	SmoothablePathObjectTraverseInfo(const SmoothablePathObjectTraverseInfo& other):
		m_pathObject(KY_NULL),
		m_status(Unset)
	{
		*this = other;
	}

	~SmoothablePathObjectTraverseInfo() { Clear(); }

	void Clear()
	{
		m_entryVertexSafePtr.Invalidate();
		m_exitVertexSafePtr.Invalidate();
		m_entryPoint.SetPosition(Vec3f::Zero());
		m_exitPoint.SetPosition(Vec3f::Zero());
		m_pathObject = KY_NULL;
		m_status = Unset;
	}

	SmoothablePathObjectTraverseInfo& operator=(const SmoothablePathObjectTraverseInfo& other)
	{
		// We indirectly setup the internals here to be sure of the PointInfos.

		SetPathObject(other.m_pathObject, other.GetEntryVertexSafePtr(), other.GetExitVertexSafePtr());
		m_entryPoint.SetPosition(other.m_entryPoint.GetPosition());
		m_exitPoint.SetPosition(other.m_exitPoint.GetPosition());
		m_status = other.m_status;

		return *this;
	}

	void SetPathObject(IPathObject* pathObject, const GraphVertexSafePtr& entryVertexSafePtr, const GraphVertexSafePtr& exitVertexSafePtr)
	{
		m_pathObject = pathObject;
		m_entryVertexSafePtr = entryVertexSafePtr;
		m_exitVertexSafePtr = exitVertexSafePtr;

		// Note: We don't check vertexSafePtrs here, since they are coming from
		// the Pathfinder, which manage the pathdata validity.

		const Vec3f* entryPosition = entryVertexSafePtr.GetPosition();
		m_entryPoint.SetPosition(entryPosition ? *entryPosition : Vec3f::Zero());

		const Vec3f* exitPosition = exitVertexSafePtr.GetPosition();
		m_exitPoint.SetPosition(exitPosition ? *exitPosition : Vec3f::Zero());

		m_status = Approach;
	}

	void SetEntryPosition(const Vec3f& entryPosition) { m_entryPoint.SetPosition(entryPosition); }
	void SetExitPosition(const Vec3f& exitPosition) { m_exitPoint.SetPosition(exitPosition); }

	bool IsSet() { return (m_status != Unset); }
	bool IsApproaching() { return (m_status == Approach); }
	bool IsTraversing() { return (m_status == Traverse); }
	void StartTraverse()
	{
		if (m_status == Approach)
			m_status = Traverse;
	}

	IPathObject* GetPathObject() const { return m_pathObject; }
	const GraphVertexSafePtr& GetEntryVertexSafePtr() const { return m_entryVertexSafePtr.GetVertexSafePtr(); }
	const GraphVertexSafePtr& GetExitVertexSafePtr() const { return m_exitVertexSafePtr.GetVertexSafePtr(); }

	const PointWrapper& GetEntryPoint()
	{
		m_entryPoint.SetInfo(m_entryVertexSafePtr);
		return m_entryPoint;
	}

	const PointWrapper& GetExitPoint()
	{
		m_exitPoint.SetInfo(m_exitVertexSafePtr);
		return m_exitPoint;
	}

	const Vec3f& GetEntryPosition() { return m_entryPoint.GetPosition(); }
	const Vec3f& GetExitPosition() { return m_exitPoint.GetPosition(); }


private:
	PointInfo_VertexSafePtr m_entryVertexSafePtr;
	PointInfo_VertexSafePtr m_exitVertexSafePtr;
	PointWrapper m_entryPoint;
	PointWrapper m_exitPoint;
	IPathObject* m_pathObject;
	Status m_status;
};

} // namespace Kaim

#endif // KyRuntime_SmoothablePathObjectTraverseInfo_H
