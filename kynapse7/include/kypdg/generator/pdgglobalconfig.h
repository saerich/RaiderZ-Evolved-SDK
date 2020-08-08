/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypdg */

// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_PdgGlobalConfig_H
#define KyPdg_PdgGlobalConfig_H


#include <kypathdata/basesystem/coordsystem.h>

#include <kypdg/generator/pdgadvancedparameters.h>
#include <kypdg/fly/generator/pdgflyglobalconfig.h>
#include <kypdg/pdginput/inputtriangle.h>


namespace Kaim
{

/*! The PdgGlobalConfig class contains configuration parameters that control the characteristics of the PathData 
	created by the Generator. You can create an instance of this class, set up its data members as desired, and 
	pass it in a call to Generator::SetGlobalConfig().
	\ingroup kypdg */
class PdgGlobalConfig
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! Enumerates the possibilities for considering back-facing triangles to be walkable. */
	enum BackFaceTrianglesWalkable { BACKFACE_TRIANGLES_ARE_NON_WALKABLE = 0, /*!< Indicates that back-facing triangles should not be considered walkable. */
									 BACKFACE_TRIANGLES_ARE_WALKABLE = 1 /*!< Indicates that back-facing triangles should be considered walkable. */
									 };
									 /*! Enumerates the possibilities for generating a NavMesh that accurately reflects the altitude of the terrain mesh. For details,
		see #m_doUseAltitudeTolerance. */
	enum GenerateVerticallyAccurateNavMesh
	{
		VERTICAL_ACCURACY_DISABLED = 0, /*!< Instructs the Generator not to adjust the altitude of the PathData. */
		VERTICAL_ACCURACY_ENABLED = 1 /*!< Instructs the Generator to perform a post-processing step to adjust the altitude of the PathData to match the altitude of the terrain mesh more accurately. */
	};

	// 2: Add PdgGraphAdvancedParameters
	enum { Version = 2 };

public:
	/*! \label_constructor */
	PdgGlobalConfig() { Clear(); }

	/*! \label_clear Restores default values for all settings. */
	void Clear()
	{
		m_entityHeight = 2.0f;
		m_entityRadius = 0.32f;
		m_stepMax = 0.59f;
		m_slopeMax = 46.0f;

		m_backFaceTrianglesWalkable = BACKFACE_TRIANGLES_ARE_WALKABLE;
		m_rasterPrecision = 0.2f;
		m_cellSize = 20.0f;
		m_graphDensity = m_cellSize;

		m_advancedParameters.Clear();
		m_pdgFlyGlobalConfig.Clear();
		m_defaultTerrainType = s_allWalkable_terrainType;

		m_altitudeTolerance = 0.5f;
		m_doUseAltitudeTolerance = VERTICAL_ACCURACY_ENABLED;
	}

	/*! Writes a record of the parameter settings in this object to the specified file stream.
		Called internally by the Generator. */
	void DumpParameters(KyFilePtr outFile);

	/*! Write accessor for #m_backFaceTrianglesWalkable. */
	void SetBackFaceTrianglesWalkable(BackFaceTrianglesWalkable backFaceTrianglesWalkable) { m_backFaceTrianglesWalkable = backFaceTrianglesWalkable; }

	/*! Read accessor for #m_backFaceTrianglesWalkable. */
	BackFaceTrianglesWalkable GetBackFaceTrianglesWalkable() const { return (BackFaceTrianglesWalkable)m_backFaceTrianglesWalkable; }

	/*! For internal use. */
	KyResult ReadBlob(const BlobFieldArray& blobFieldArray);

	/*! For internal use. */
	void BuildBlob(BlobHandler<BlobFieldArray>& blobFieldArrayHandler);

	/*! Read accessor for #m_generateFly. */
	bool IsFlyGenerationActivated() const { return m_pdgFlyGlobalConfig.m_generateFly != 0; }

	/*! Indicates whether or not the Generator will create one-way Graph edges that lead down vertical jumps that
		the character is not able to climb. */
	bool IsJumpCandidateGenerationActivated() const { return m_advancedParameters.m_jumpDownHeight > 0; }

	/*! Write accessor for #m_graphDensity. */
	void SetGraphDensity(KyFloat32 density);

	/*! Write accessor for the parameters stored in PdgGraphAdvancedParameters. 
		\pp Calling this method automatically sets PdgGraphAdvancedParameters::m_useGraphDensityParameter to 0, which 
		disables the #m_graphDensity parameter in favor of the values that you specify here. */
	void SetGraphDensity(KyFloat32 distEdgeMax, KyFloat32 coverage, KyFloat32 maxDistanceBetweenCellBoundaryVertices);

	/*! Retrieves the maximum length of any edge in the Graph.
		When PdgGraphAdvancedParameters::m_useGraphDensityParameter is 0, this method returns PdgGraphAdvancedParameters::m_distEdgeMax.
		When PdgGraphAdvancedParameters::m_useGraphDensityParameter is 1, this method returns #m_graphDensity. */
	KyFloat32 GetDistEdgeMax() const;

	/*! Retrieves the maximum distance that may exist between any boundary in the NavMesh and any vertex in the Graph.
		When PdgGraphAdvancedParameters::m_useGraphDensityParameter is 0, this method returns PdgGraphAdvancedParameters::m_coverageDistance.
		When PdgGraphAdvancedParameters::m_useGraphDensityParameter is 1, this method returns #m_graphDensity * 0.5. */
	KyFloat32 GetCoverageDistance() const;

	/*! Retrieves the maximum distance that may exist between vertices along the boundary between two adjacent \NavCells.
		When PdgGraphAdvancedParameters::m_useGraphDensityParameter is 0, this method returns PdgGraphAdvancedParameters::m_maxDistanceBetweenCellBoundaryVertices.
		When PdgGraphAdvancedParameters::m_useGraphDensityParameter is 1, this method returns #m_graphDensity. */
	KyFloat32 GetMaxDistanceBetweenCellBoundaryVertex() const;

	/*! Returns #KY_TRUE if the specified PdgGlobalConfig contains the same values as this object. */
	bool operator==(const PdgGlobalConfig& other) const;

	/*! Returns #KY_TRUE if the specified PdgGlobalConfig contains at least one value that is different from this object. */
	bool operator!=(const PdgGlobalConfig& other) const { return !operator==(other); }

private:
	void AddMapping(BlobFieldsMapping& mapping, KyUInt32& version);
public:
	/*! The height of the character that will use the PathData at runtime.
		\units			meters
		\defaultvalue	\c 2.0f */
	KyFloat32 m_entityHeight; // in meters

	/*! The radius (or half-width) of the character that will use the PathData at runtime.
		\units			meters
		\defaultvalue	\c 0.32f */
	KyFloat32 m_entityRadius; // in meters

	/*! The maximum difference in altitude that the character that will use the PathData at runtime can traverse in its normal movement. 
		\units			meters
		\defaultvalue	\c 0.59f */
	KyFloat32 m_stepMax;      // in meters

	/*! The maximum slope that the character that will use the PathData at runtime can traverse in its normal movement. Any input
		triangle with a slope greater than this value is automatically tagged with the #s_nonWalkable_terrainType terrain type, and no
		PathData will be generated for that triangle.
		\units 			degrees
		\defaultvalue	\c 46.0f */
	KyFloat32 m_slopeMax;     // in degrees

	/*! Specifies the default terrain type that will be applied to all input triangles that do not otherwise have a terrain
		type set explicitly.
		\defaultvalue	#s_allWalkable_terrainType */
	TerrainTypeMask m_defaultTerrainType;

	/*! Determines whether or not back-face triangles (i.e. triangles whose points go in counter-clockwise order when seen
		from above) will be considered walkable and have PathData generated for them.
		\acceptedvalues	Any value from the #BackFaceTrianglesWalkable enumeration.
		\defaultvalue	#BACKFACE_TRIANGLES_ARE_WALKABLE */
	KyUInt32 m_backFaceTrianglesWalkable;

	/*! Determines the width and length of each pixel used to rasterize the input triangles.
		\units			meters
		\defaultvalue	\c 0.2f */
	KyFloat32 m_rasterPrecision; // aka pixelSize

	/*! Determines the width and length of each cell in the grid used to partition the NavMesh and the Graph internally.
		\units			meters
		\defaultvalue	\c 20.0f */
	KyFloat32 m_cellSize;        // cell side size in meter of the NavMesh grid partition

	/*! Determines the density of the final Graph created by the Generator. 
		\pp Decreasing the value of this parameter may result in a denser Graph with more vertices and edges. This can be useful to 
		provide more different potential destination points for your \GraphTraversals and agents to select, or in order for the 
		PathFinder to generate smoother initial paths through the Graph. However, the denser you make the Graph, the longer your 
		path calculations and \GraphTraversals will take to complete, as they likely will explore more candidate vertices.
		\pp Increasing the value of this parameter may produce a less dense Graph, up to a point.
		\pp The default value should suffice in most cases.
		\pp See also :USERGUIDE:"Changing the Graph Density".
		\acceptedvalues	Any positive value
		\units          meters
		\defaultvalue   20.0f */
	KyFloat32 m_graphDensity;

	/*! Determines the maximum difference in altitude that may exist between the NavMesh and the original terrain mesh.
		\pp The way this parameter is interpreted depends on whether or not the Generator is configured to
		generate vertically accurate PathData: i.e. on the value of m_doUseAltitudeTolerance.
		-	When vertically accurate NavMesh generation is enabled (the default), this parameter determines the maximum vertical
			distance that the NavMesh triangles may be from the original raster at any point.
		-	When vertically accurate NavMesh generation is disabled, this parameter is only taken into account at the edges
			of NavMesh triangles that compose the borders between \NavCells and \NavFloors. It has no effect on the PathData
			generated in the interior of the cells themselves.
		\pp Lower values typically result in a NavMesh with smaller triangles that is more faithful to the original 
		rasterization. Larger values simplify the NavMesh more aggressively, and typically result in a NavMesh with fewer,
		larger triangles. In most cases, the default value should be sufficient unless you need an accurate NavMesh for your
		own purposes outside of \SDKName. See also :USERGUIDE:"Increasing Altitude Precision".
		\units 			meters
		\defaultvalue	\c 0.5f */
	KyFloat32 m_altitudeTolerance; //in m

	/*! Determines whether or not the Generator performs a post-processing step to ensure that altitude of the PathData accurately
		reflects the altitude of the terrain mesh.
		-	Without this post-processing step, the altitude of the PathData is guaranteed to match the altitude of the terrain mesh
			only at the boundaries between adjacent cells. Within each cell, the altitude of the PathData is not guaranteed to match
			the terrain mesh with any degree of precision. 
		-	Carrying out the post-processing step increases the vertical accuracy of the final NavMesh, but may increase the size of 
			the final PathData and the time required to generate it.
		\pp See also :USERGUIDE:"Increasing Altitude Precision".
		\acceptedvalues	Any value from the GenerateVerticallyAccurateNavMesh enumeration. 
		\defaultvalue	#VERTICAL_ACCURACY_ENABLED */
	KyUInt32 m_doUseAltitudeTolerance;
	/*! Contains more advanced configuration parameters. The default values of the parameters offered by PdgAdvancedParameters
		should suffice for most projects. */
	PdgAdvancedParameters m_advancedParameters;

	/*! Contains configuration parameters used to generate PathData for flying characters. */
	PdgFlyGlobalConfig m_pdgFlyGlobalConfig;
};

} // namespace Kaim

#endif
