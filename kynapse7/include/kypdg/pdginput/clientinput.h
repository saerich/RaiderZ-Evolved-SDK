
/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_ClientInput_H
#define KyPdg_ClientInput_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/containers/array.h"

#include "kypdg/pdginput/seedpoint.h"
#include "kypdg/pdginput/clientinputtriangle.h"
#include "kypdg/generator/pdgglobalconfig.h"
#include "kypdg/pdginput/terraintypevolume.h"
#include "kypathdata/graph/inputgraphvertex.h"

namespace Kaim
{

class ClientInputLoader;
class PdgSystem;

class ClientInput
{
	KY_DEFINE_NEW_DELETE_OPERATORS;
public:
	/*!	Constructor to only read a PdgInput.*/
	ClientInput() : m_sys(KY_NULL) {}

	/*!	---Internal--- Constructor used during PathData generation.*/
	ClientInput(PdgSystem* sys) : m_sys(sys) {}
	
	~ClientInput() { Clear(); }

	KyResult Save(const char* fullFileName);

	/*!	Load uses PdgFile if a PdgSystem has been specified in constructor.
		Otherwise it loads from specified filename.*/
	KyResult Load(const char* fullFileName);

	void Clear();

private: 
	/* ! Called by previous method, whatever the case */
	KyResult Load(KyFilePtr& file, ClientInputLoader& loader);

public:
	PdgSystem* m_sys;
	PdgGlobalConfig m_globalConfig;
	CoordSystem m_coordSystem;

	DynamicClientSeedPointArray m_seedPoints;
	DynamicInputVertexArray m_inputGraphVertices;
	DynamicClientInputTriangleArray m_triangles;
	DynamicClientTerrainTypeVolumeArray m_volumes;

private:
	ClientInput& operator=(const ClientInput& other);
	ClientInput(const ClientInput& other);
};



}


#endif

