/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_Dynamic_PathObject_Topology_H
#define KyPathData_Dynamic_PathObject_Topology_H

#include "kypathdata/pathobject/ipathobjecttopology.h"
#include <kypathdata/containers/array.h>

namespace Kaim
{

class IPathObjectConnexionSpace;
class IPathObjectControlSpace;
class IPathObjectConnexionLink;
class PathObjectTopologyData;


/*! Each instance of the DynamicPathObjectTopology class represents the connection spaces at which
	\Bots can enter and leave a PathObject, the links the Bot can follow between those connection spaces,
	and the control space (or spaces) that defines the area within which the PathObject controls the movements of the
	\Bots traversing it.
	\ingroup kypathdata_pathobject */
class DynamicPathObjectTopology : public IPathObjectTopology
{
public:
	/*! \label_constructor */
	DynamicPathObjectTopology();

	/*! \label_virtualdestructor */
	~DynamicPathObjectTopology();

	/*! Clears all connection spaces, connection links and control spaces. */
	void Init();

	/*! Deletes registered connexion links and clear connexion spaces and controlspaces.*/
	void Reset();

	/*! Deletes all registered connection links. */
	void DeleteRegisteredConnexionLinks();

	/*! Deletes all registered connection spaces. */
	void DeleteRegisteredConnexionSpaces();

	/*! Deletes all registered control spaces. */
	void DeleteRegisteredControlSpaces();

	virtual KyUInt32 GetConnexionLinkCount() const;
	virtual IPathObjectConnexionLink* GetConnexionLink(const PathObjectConnexionLinkIdx& idx) const;
	
	virtual KyUInt32 GetConnexionSpaceCount() const;
	virtual IPathObjectConnexionSpace* GetConnexionSpace(const PathObjectConnexionSpaceIdx& idx) const;

	virtual KyUInt32 GetControlSpaceCount() const;
	virtual IPathObjectControlSpace* GetControlSpace(const PathObjectControlSpaceIdx& idx) const;

	/*! Adds a new control space to the topology. The input object is not copied, so you must keep it in memory until
		you have finished using this DynamicPathObjectTopology. */
	virtual PathObjectControlSpaceIdx AddControlSpace(IPathObjectControlSpace* controlSpace);

	/*! Adds a new connection space to the topology. The input object is not copied, so you must keep it in memory until
		you have finished using this DynamicPathObjectTopology. */
	virtual PathObjectConnexionSpaceIdx AddConnexionSpace(IPathObjectConnexionSpace* connexionSpace);

	/*! Adds a new one-way connection link between the two specified connection spaces.
		\param startIdx			The index of the starting connection space for the link. 
		\param endIdx			The index of the ending connection space for the link. */
	PathObjectConnexionLinkIdx AddConnexionLink(const PathObjectConnexionSpaceIdx& startIdx, const PathObjectConnexionSpaceIdx& endIdx);
	
	/*! Adds a new connexion link between the two specified connection spaces, specifying whether the link
		is one-way or two-ways.
		\param startIdx			The index of the starting connection space for the link. 
		\param endIdx			The index of the ending connection space for the link. 
		\param bidirectionnal	\c true if two one-way links should be created to link the connection spaces in both
								directions, or \c false if only one one-way link should be created from the starting
								connection space to the ending connection space. */
	KyResult AddConnexionLink(const PathObjectConnexionSpaceIdx& startIdx, const PathObjectConnexionSpaceIdx& endIdx, const bool bidirectionnal);

	/*! \copydoc IPathObjectTopology::CanDetectAccident()
		This implementation returns the value stored in #m_canDetectAccident. */
	bool CanDetectAccident() const { return m_canDetectAccident; }

public:
	Array<IPathObjectConnexionSpace*> m_connexionSpaces; /*!< The list of connection spaces. Do not modify directly. */
	Array<IPathObjectControlSpace*> m_controlSpaces; /*!< The list of control spaces. Do not modify directly. */
	Array<IPathObjectConnexionLink*> m_connexionLinks; /*!< The list of connection links. Do not modify directly. */
	bool m_canDetectAccident; /*!< Determines the value returned by CanDetectAccident(). */
};

}

#endif // #ifndef KyPathData_Dynamic_PathObject_Topology_H
