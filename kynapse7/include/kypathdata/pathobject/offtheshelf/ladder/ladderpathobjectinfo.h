/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: None
#ifndef KYPATHDATA_LADDER_PATHOBJECT_INFO_H
#define KYPATHDATA_LADDER_PATHOBJECT_INFO_H

#include "kypathdata/pathobject/info/dynamicpathobjectinfo.h"

namespace Kaim
{

/*	
*/
class LadderPathObjectInfo : public DynamicPathObjectInfo
{
public:

	LadderPathObjectInfo();

	LadderPathObjectInfo(
		const PathObjectId& id,
		const char* type,
		const char* databaseName,
		const Vec3f& bottom,
		const Vec3f& top,
		const Vec3f& ladderBack,
		const KyFloat32 ladderRadius,
		KyFloat32 averageEntityHeight) :
	DynamicPathObjectInfo(id, type, databaseName, averageEntityHeight), m_bottom(bottom), m_top(top), m_ladderBack(ladderBack), m_ladderRadius(ladderRadius) {}

	LadderPathObjectInfo(
		const Vec3f& bottom,
		const Vec3f& top,
		const Vec3f& ladderBack,
		const KyFloat32 ladderRadius) :
	DynamicPathObjectInfo(), m_bottom(bottom), m_top(top), m_ladderBack(ladderBack), m_ladderRadius(ladderRadius) {}

	/*!
	Render debugging information accordingly to provided flags.
	*/
	virtual void Render(KyUInt32 renderFlags) const
	{
		DynamicPathObjectInfo::Render(renderFlags);

		Vec3f ladderVector = m_top - m_bottom;
		KyFloat32 ladderLength = ladderVector.GetLength();
		ladderVector.Normalize();
		Vec3f ladderLeft = ladderVector ^ m_ladderBack;
		Draw3DLine(m_bottom + 0.5f * m_ladderRadius * ladderLeft, m_top + 0.5f * m_ladderRadius * ladderLeft, 200, 200, 0);
		Draw3DLine(m_bottom - 0.5f * m_ladderRadius * ladderLeft, m_top - 0.5f * m_ladderRadius * ladderLeft, 200, 200, 0);
		DrawArrow(m_bottom + ladderVector * 0.5f * ladderLength, m_bottom + ladderVector * 0.5f * ladderLength + m_ladderBack, 200, 200, 0);
	}

public:
	Vec3f m_bottom;           /*!<	Bottom position of the ladder.*/
	Vec3f m_top;              /*!<	Top position of the ladder.*/
	Vec3f m_ladderBack;       /*!<	Vector pointing toward the back of the ladder. It is used to know when the bot has 
									to turn around to grab or get down out of the ladder.*/
	KyFloat32 m_ladderRadius; /*!<	Ladder radius. It is used to localize and set the position of the bot on the ladder.*/
};

}

#endif // KYPATHDATA_LADDER_PATHOBJECT_INFO_H
