/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_mesh */

#ifndef OBSTACLE_LAYER_BLOB_BUILDER_H
#define OBSTACLE_LAYER_BLOB_BUILDER_H

#include <kypathdata/blob/baseblobbuilder.h>

namespace Kaim
{

class ObstacleLayer;

/*! A type of blob that serializes the data for a single outline within an ObstacleLayerBlob.
	\ingroup kyruntime_mesh */
class ObstacleLayerRecordBlob
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_blobtype */
	static const char* GetBlobType() { return "ObstacleLayerRecordBlob"; }

	/*! \label_blobversion */
	static KyUInt32 GetBlobVersion() { return 0; }

	/*! The list of Vec3f instances that make up the outline. */
	BlobArray<Vec3f> m_points;
}; 

/*! A type of blob that serializes the data for all outlines contained in a single ObstacleLayer.
	Managed through the ObstacleLayerBlobBuilder class.
	\ingroup kyruntime_mesh	*/
class ObstacleLayerBlob
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_blobtype */
	static const char* GetBlobType() { return "ObstacleLayerBlob"; }

	/*! \label_blobversion */
	static KyUInt32 GetBlobVersion() { return 0; }

	/*! A list of ObstacleLayerRecordBlob instances that represent the outlines in the ObstacleLayer. */
	BlobArray<ObstacleLayerRecordBlob> m_records;
};

/*! A type of blob builder that creates an ObstacleLayerBlob from the contents of a specified ObstacleLayer.
	You can use this class to serialize the outlines in an ObstacleLayer to and from a file on disk.
	\ingroup kyruntime_mesh	*/
class ObstacleLayerBlobBuilder : public BaseBlobBuilder<ObstacleLayerBlob>
{
public:
	/*! \label_constructor
		\param layer			The ObstacleLayer to be serialized by this blob builder. */
	ObstacleLayerBlobBuilder(ObstacleLayer* layer) : m_obstacleLayer(layer) {}
	virtual void DoBuild();

private:
	ObstacleLayer* m_obstacleLayer;
};

/*! \label_swapendianness 
	\ingroup kyruntime_mesh */
inline void SwapEndianness(Endianness::Target e, ObstacleLayerRecordBlob& self)
{
	SwapEndianness(e, self.m_points);
}

/*! \label_swapendianness
	\ingroup kyruntime_mesh */
inline void SwapEndianness(Endianness::Target e, ObstacleLayerBlob& self)
{
	SwapEndianness(e, self.m_records);
}

}

#endif // OBSTACLE_LAYER_BLOB_BUILDER_H
