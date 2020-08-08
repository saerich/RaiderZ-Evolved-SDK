/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	

/*! \file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_DynamicPathObjectInfo_H
#define KyPathData_DynamicPathObjectInfo_H

#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/ipathobjectinfo.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/generic/kyguid.h"
#include "kypathdata/basesystem/coordsystem.h"

namespace Kaim
{
class PathObjectInfoData;

/*! Each instance of the DynamicPathObjectInfo class represents the metadata about a PathObject, such as its
	name, type, and the Database that it will be connected to.
	\ingroup kypathdata_pathobject */
class DynamicPathObjectInfo : public IPathObjectInfo
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	DynamicPathObjectInfo();

	/*! \label_constructor
		\param databaseName				The name of the Database this PathObject will be connected to.
		\param m_averageEntityHeight	The height of the characters that use the PathData in \c databaseName. */
	DynamicPathObjectInfo(const char* databaseName, KyFloat32 m_averageEntityHeight);

	/*! \label_constructor
		\param id						A unique ID for the PathObject. Used only to generate a file name if you dump the PathObject to a file on disk.
		\param type						The type of the PathObject. Used only to generate a file name if you dump the PathObject to a file on disk.
		\param databaseName				The name of the Database this PathObject will be connected to.
		\param m_averageEntityHeight	The height of the characters that use the PathData in \c databaseName. */
	DynamicPathObjectInfo(const PathObjectId& id, const char* type, const char* databaseName, KyFloat32 m_averageEntityHeight);

	/*! \label_constructor For internal use when creating a PathObject from a file. */
	DynamicPathObjectInfo(const PathObjectInfoData& data);


	/*! \label_virtualdestructor */
	virtual ~DynamicPathObjectInfo();

	void Init();

	virtual PathObjectId GetId() const;
	virtual const char* GetType() const;
	virtual const char* GetDataBaseName() const;
	virtual KyFloat32 GetAverageEntityHeight() const;

	/*! \label_reinit
		\param id						A unique ID for the PathObject. Used only to generate a file name if you dump the PathObject to a file on disk.
		\param type						The type of the PathObject. Used only to generate a file name if you dump the PathObject to a file on disk.
		\param databaseName				The name of the Database this PathObject will be connected to.
		\param averageEntityHeight		The height of the characters that use the PathData in \c databaseName. */
	void SetUp(const PathObjectId& id, const char* type, const char* databaseName, KyFloat32 averageEntityHeight);

	/*! Sets a unique ID for the PathObject, used only to generate a file name if you dump the PathObject to a file on disk. */
	void SetId(const PathObjectId& poId);

	/*! Sets the type of the PathObject, used only to generate a file name if you dump the PathObject to a file on disk. */
	void SetType(const char* typeName);

	/*! Sets the name of the Database this PathObject will be connected to. */
	void SetDataBaseName(const char* databaseName);

	/*! Sets the height of the characters that use the PathData in \c databaseName. */
	void SetAverageEntityHeight(KyFloat32 height);

	virtual void Render(KyUInt32 /*renderFlags*/) const {}

public:
	PathObjectId m_id; /*!< Stores the ID set for the PathObject. */

	static const KyUInt32 TYPE_MAX_CHAR = 256;
	char m_type[TYPE_MAX_CHAR]; /*!< Stores the ID set for the PathObject. */
	static const KyUInt32 DATABASE_NAME_MAX_CHAR = 256;
	char m_databaseName[DATABASE_NAME_MAX_CHAR];  /*!< Stores the name of the Database set for the PathObject. */
	KyFloat32 m_averageEntityHeight; /*!< Stores the height of the characters that use the PathData in #m_databaseName. */
};

}

#endif // #ifndef KyPathData_DynamicPathObjectInfo_H
