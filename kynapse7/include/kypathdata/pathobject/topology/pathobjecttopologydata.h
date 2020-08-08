/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObjectTopologyData_H
#define KyPathData_PathObjectTopologyData_H


#include "kypathdata/blob/blobarray.h"
#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/topology/connexionspaces/pathobjectconnexionspacedata.h"
#include "kypathdata/pathobject/topology/connexionlinks/pathobjectconnexionlinkdata.h"
#include "kypathdata/pathobject/topology/controlspaces/pathobjectcontrolspacedata.h"
#include <assert.h>

namespace Kaim
{

/*
Serializable structure (Blob) containing all topological data of a pathObject, namely:
-PathObject Connexion spaces: defines spaces where the PathObject must be connected to the PointGraphCell. 
   eg: Connexion space 1 on one side of a door, connexion space 2 on the other side.
-PathObject Connexion links: defines the links between connexion spaces.
   eg: For a lift: link1 = [ [ConnexionSpaceFloor1, ConnexionSpaceFloor2], bidirectional ]
                   link2 = [ [ConnexionSpaceFloor1, ConnexionSpaceFloor3], bidirectional ]
                   link3 = [ [ConnexionSpaceFloor2, ConnexionSpaceFloor3], bidirectional ]
-PathObject Control spaces: areas covered by the PathObject
   eg: For a door, the door volume + a flag forbidding other graph edges to go intersect this volume.
*/
class PathObjectTopologyData
{
public:
	static const char* GetBlobType() { return "PathObjectTopologyData"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	PathObjectTopologyData() {}

	/*! Returns the number of connexion spaces. */
	KyUInt32 GetConnexionSpaceCount() const { return m_connexionSpaces.m_count; }

	/*!
	Gets the connexion space at index idx.
	\param idx Input idx.
	\return the connexion space (read-only)
	*/
	const PathObjectConnexionSpaceData& GetConnexionSpace(const PathObjectConnexionSpaceIdx idx) const
	{
		assert(idx < m_connexionSpaces.m_count);
		return m_connexionSpaces.Values()[idx];
	}

	/*! Returns the number of connexion links. */
	KyUInt32 GetConnexionLinkCount() const { return m_connexionLinks.m_count; }

	/*!
	Gets the connexion link at index idx.
	\param idx Input idx.
	\param connexionLink Output connexion link if valid.
	\return KY_ERROR if invalid input number, else KY_SUCCESS
	*/
	const PathObjectConnexionLinkData& GetConnexionLink(const PathObjectConnexionLinkIdx idx) const
	{
		assert(idx < m_connexionLinks.m_count);
		return m_connexionLinks.Values()[idx];
	}

	/*! Returns the number of control spaces. */
	KyUInt32 GetControlSpaceCount() const { return m_controlSpaces.m_count; }

	/*!
	Gets the control space at index idx.
	\param idx Input idx.
	\param controlSpace Output control space if valid.
	\return KY_ERROR if invalid input number, else KY_SUCCESS
	*/
	const PathObjectControlSpaceData& GetControlSpace(const PathObjectControlSpaceIdx idx) const
	{
		assert(idx < m_controlSpaces.m_count);
		return m_controlSpaces.Values()[idx];
	}

	
public:
	BlobArray<PathObjectConnexionSpaceData> m_connexionSpaces;  //!< Connexion spaces.
	BlobArray<PathObjectConnexionLinkData> m_connexionLinks;    //!< Connexion links.
	BlobArray<PathObjectControlSpaceData> m_controlSpaces;      //!< Control Spaces.

private:
	PathObjectTopologyData& operator=(const PathObjectTopologyData& pathObjectData);
	PathObjectTopologyData(const PathObjectTopologyData& pathObjectData);
};

/*
Implements the endianness swap.
*/
inline void SwapEndianness(Endianness::Target e, PathObjectTopologyData& self)
{
	SwapEndianness(e, self.m_connexionSpaces);
	SwapEndianness(e, self.m_connexionLinks);
	SwapEndianness(e, self.m_controlSpaces);
}

}

#endif // #ifndef KyPathData_PathObjectTopologyData_H
