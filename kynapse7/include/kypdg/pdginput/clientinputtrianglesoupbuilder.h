/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	

/*! \file
	\ingroup kypdg */

// primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_ClientInputTriangleSoupBuilder_H
#define KyPdg_ClientInputTriangleSoupBuilder_H

#include "kypdg/pdginput/clientinputtrianglesoup.h"

namespace Kaim
{

class ClientInputTriangleSoupBlobBuilder : public BaseBlobBuilder<ClientInputTriangleSoup>
{
public:
	ClientInputTriangleSoupBlobBuilder(const DynamicClientInputTriangleArray* inputTriangles) :
	  m_inputTriangles(inputTriangles)
	  {}
private:
	virtual void DoBuild()
	{
		ClientInputTriangle* blob = BLOB_ARRAY(m_blob->m_triangles, m_inputTriangles->GetCount());
		if (IsWriteMode() == true)
		{
			for(KyUInt32 i = 0 ; i < m_inputTriangles->GetCount(); ++i)
			{
				blob[i] = (*m_inputTriangles)[i];
			}
		}
	}
private:
	const DynamicClientInputTriangleArray* m_inputTriangles;
};

}


#endif

