/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_DynamicPathObject_ControlSpace_H
#define KyPathData_DynamicPathObject_ControlSpace_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/ipathobjectcontrolspace.h"

#include "kypathdata/shapes/axisalignedboxvolume.h"


namespace Kaim
{

/*! Defines a base interface for a PathObject control space, which represents a volume within which
	the PathObject has sole responsibility over the movements of a Bot. It adds virtual methods for 
	retrieving volumes and setting the behavior of the control space to the base IPathObjectConnexionSpace interface.
	\pp This class cannot be used directly. Use one of the derived classes supplied with the \SDKName SDK,
	or write your own implementation.
	\ingroup kypathdata_pathobject */
class DynamicPathObjectControlSpace : public IPathObjectControlSpace
{
public:
	/*! \label_baseconstructor */
	DynamicPathObjectControlSpace();

	/*! \label_virtualdestructor */
	virtual ~DynamicPathObjectControlSpace();

	/*! Retrieves the volume of the connection space. */
	virtual const IVolume* GetVolume() const = 0;
	virtual void Render(KyUInt32 /*renderFlags*/) const { GetVolume()->Render(0, 255, 0); }

	virtual bool CanLockEdges() const;
	virtual bool IsAnObstacle() const;

	/*! Sets up the values returned by the CanLockEdges(), IsAnObstacle() and IsASpaceToStartUsingThePathObject() methods.
		Called transparently by the SetUp() methods of derived classes. */
	void SetUp(bool canLockEdges, bool isAnObstacle, bool isASpaceToStartUsingThePathObject);

	bool IsASpaceToStartUsingThePathObject() const { return m_isASpaceToStartUsingThePathObject; }

private:
	bool m_canLockEdges;
	bool m_isAnObstacle;
	bool m_isASpaceToStartUsingThePathObject;
};

}

#endif // #ifndef KyPathData_DynamicPathObject_ControlSpace_H
