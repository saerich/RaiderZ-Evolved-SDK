/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObjectConnexionLinkVisualRepresentation_H
#define KyPathData_PathObjectConnexionLinkVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"
#include "kypathdata/pathobject/pathobjectdatatypes.h"

namespace Kaim
{

class IPathObjectTopology;

/* Implements the visual representation of the pathObject ConnexionLink. */
class PathObjectConnexionLinkVisualRepresentation : public VisualRepresentation
{
public:
	PathObjectConnexionLinkVisualRepresentation() : m_idx(KyUInt32MAXVAL) {}

	void Init(IPathObjectTopology* poTopology, PathObjectConnexionLinkIdx idx)
	{
		m_representedData = poTopology;
		m_idx = idx;
	}

	virtual ~PathObjectConnexionLinkVisualRepresentation() {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	PathObjectConnexionLinkIdx m_idx;
};


}

#endif // #KyPathData_PathObjectConnexionLinkVisualRepresentation_H
