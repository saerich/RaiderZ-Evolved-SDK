/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypdg */

// primary contact: JODA - secondary contact: GUAL

#ifndef KyPdg_PdgGraphAdvancedParameters_H
#define KyPdg_PdgGraphAdvancedParameters_H

#include <kypathdata/generic/types.h>
#include <kypathdata/blob/blobfieldarray.h>

namespace Kaim
{
/*! The PdgGraphAdvancedParameters class is used by the PdgAdvancedParameters class to maintain a set of configuration parameters
	that control detailed technical aspects of the way \Graphs are generated.
	\pp See also :USERGUIDE:"Changing Graph Density".
	\ingroup kypdg */
class PdgGraphAdvancedParameters
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor */
	PdgGraphAdvancedParameters() :
		m_useGraphDensityParameter(1),
		m_distEdgeMax(20.0f),
		m_coverageDistance(10.0f),
		m_maxDistanceBetweenCellBoundaryVertices(20.0f)
	{
		SetDefaultValues();	
	}

	/*! \label_clear */
	void SetDefaultValues()
	{
		m_useGraphDensityParameter = 1;
		m_distEdgeMax = 20.0f;
		m_coverageDistance = 10.0f;
		m_maxDistanceBetweenCellBoundaryVertices = 20.0f;
	}

	/*! For internal use. */
	void AddMapping(BlobFieldsMapping& mapping)
	{
		mapping.AddUInt32("Graph-UseGraphDensityParameter", m_useGraphDensityParameter);
		mapping.AddFloat32("Graph-DistEdgeMax", m_distEdgeMax);
		mapping.AddFloat32("Graph-CoverageDistance", m_coverageDistance);
		mapping.AddFloat32("Graph-MaxDistanceBetweenCellBoundaryVertices", m_maxDistanceBetweenCellBoundaryVertices);
	}

	/*! Returns #KY_TRUE if the specified PdgGraphAdvancedParameters contains the same values as this object. */
	bool operator == (const PdgGraphAdvancedParameters& rhs) const
	{
		return m_useGraphDensityParameter == rhs.m_useGraphDensityParameter
			&& m_distEdgeMax == rhs.m_distEdgeMax
			&& m_coverageDistance == rhs.m_coverageDistance
			&& m_maxDistanceBetweenCellBoundaryVertices == rhs.m_maxDistanceBetweenCellBoundaryVertices;
	}

	/*! Returns #KY_TRUE if the specified PdgGraphAdvancedParameters contains at least one value that is different from this object. */
	bool operator != (const PdgGraphAdvancedParameters& rhs) const { return !(*this == rhs); }
public:
	/*! Determines which configuration parameters the Generator will use to control the density of the \Graphs it creates.
		\acceptedvalues	-	0: The Generator will use the values of the other members in this object, and will ignore the 
							value of PdgGlobalConfig::m_graphDensity. 
						-	1: The Generator will use the value of PdgGlobalConfig::m_graphDensity, and will ignore the values of the
							other members of this object.
		\defaultvalue	\c 1 */
	KyUInt32 m_useGraphDensityParameter;

	/*! Determines the maximum length of any edge in the final \Graphs created by the Generator.
		\pp Read only when #m_useGraphDensityParameter is set to 0.
		\units			meters
		\defaultvalue	\c 20.0f */
	KyFloat32 m_distEdgeMax;

	/*! Determines the maximum distance that may exist between any border in the NavMesh and any vertex in the Graph. This
		includes the boundary between adjacent \NavCells, adjacent \NavFloors, and the outside boundaries of the NavMesh itself.
		\pp Read only when #m_useGraphDensityParameter is set to 0.
		\units			meters
		\defaultvalue	\c 10.0f */
	KyFloat32 m_coverageDistance;

	/*! Determines the maximum distance that may exist between vertices created on the boundary between two adjacent \NavCells.
		\pp Read only when #m_useGraphDensityParameter is set to 0.
		\units			meters
		\defaultvalue	\c 20.0f */
	KyFloat32 m_maxDistanceBetweenCellBoundaryVertices;
};

} // Kaim

#endif // KyPdg_PdgGraphAdvancedParameters_H
