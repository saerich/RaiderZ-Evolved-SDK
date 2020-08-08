/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: GUAL

#ifndef KyPdg_PdgInputTerrainTypeVolumeBlobBuilder_H
#define KyPdg_PdgInputTerrainTypeVolumeBlobBuilder_H


#include <kypathdata/blob/baseblobbuilder.h>
#include <kypdg/pdginput/pdginputterraintypevolume.h>


namespace Kaim
{


class KynapseTerrainTypeVolume;
class ClientTerrainTypeVolume;

class PdgInputTerrainTypeVolumeBlobBuilder : public BaseBlobBuilder<PdgInputTerrainTypeVolume>
{
public:
	PdgInputTerrainTypeVolumeBlobBuilder(const KynapseTerrainTypeVolume& volume) : m_volume(&volume) {}

private:
	virtual void DoBuild();

	const KynapseTerrainTypeVolume* m_volume;
};

// Same but for ClientInput volumes
class ClientInputTerrainTypeVolumeBlobBuilder : public BaseBlobBuilder<ClientInputTerrainTypeVolume>
{
public:
	ClientInputTerrainTypeVolumeBlobBuilder(const ClientTerrainTypeVolume& volume) : m_volume(&volume) {}

private:
	virtual void DoBuild();

	const ClientTerrainTypeVolume* m_volume;
};
} //namespace Kaim

#endif
