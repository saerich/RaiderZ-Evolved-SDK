/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_DynamicPathObject_ConnexionLink_H
#define KyPathData_DynamicPathObject_ConnexionLink_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/ipathobjectconnexionlink.h"

namespace Kaim
{
class PathObjectConnexionLinkData;

/*!
DynamicPathObjectConnexionLink
*/
class DynamicPathObjectConnexionLink : public IPathObjectConnexionLink
{
public:
	DynamicPathObjectConnexionLink();
	DynamicPathObjectConnexionLink(const PathObjectConnexionLinkData& data);
	virtual ~DynamicPathObjectConnexionLink();

	virtual const PathObjectConnexionSpaceIdx& GetStartConnexionSpaceIdx() const;
	virtual const PathObjectConnexionSpaceIdx& GetEndConnexionSpaceIdx() const;
	
public:
	PathObjectConnexionSpaceIdx m_startConnexionSpaceIdx;   //!< ID of the start connexion space.
	PathObjectConnexionSpaceIdx m_endConnexionSpaceIdx;     //!< ID of the end connexion space.
};

}

#endif // #ifndef KyPathData_DynamicPathObject_ConnexionLink_H
