/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_SeedPoint_H
#define KyPdg_SeedPoint_H

#include "kypathdata/math/vec3f.h"
#include "kypathdata/blob/blobarray.h"
#include "kypathdata/blob/baseblobbuilder.h"
#include "kypathdata/containers/array.h"

namespace Kaim
{

typedef Vec3f SeedPointClientPosition;
// Stores a position in client coordinates
// it is meant to be temporally stored by the consumer before creating the pdginput
class ClientSeedPoint
{
public: 
	ClientSeedPoint() {} //default ctor needed to build blobArray
	ClientSeedPoint(const SeedPointClientPosition& position): m_position(position) {}
	static const char* GetBlobType() { return "SeedPoint"; }
	static KyUInt32 GetBlobVersion() { return 0; }

	SeedPointClientPosition m_position;
};

typedef Array<ClientSeedPoint> DynamicClientSeedPointArray;


class ClientSeedPointArray
{
public:
	static const char* GetBlobType() { return "SeedPointArray"; }
	static KyUInt32 GetBlobVersion() { return 0; }

	BlobArray<ClientSeedPoint> m_seedPoints; 
};

inline void SwapEndianness(Endianness::Target e, ClientSeedPoint& self)
{
	SwapEndianness(e, self.m_position);
}

inline void SwapEndianness(Endianness::Target e, ClientSeedPointArray& self)
{
	SwapEndianness(e, self.m_seedPoints);
}

class ClientSeedPointArrayBlobBuilder : public BaseBlobBuilder<ClientSeedPointArray>
{
public:
	ClientSeedPointArrayBlobBuilder(const DynamicClientSeedPointArray* inputSeedpoints) :
	  m_inputSeedPoints(inputSeedpoints)
	  {}
private:
	virtual void DoBuild()
	{
		ClientSeedPoint* blob = BLOB_ARRAY(m_blob->m_seedPoints, m_inputSeedPoints->GetCount());
		if (IsWriteMode() == true)
		{
			for(KyUInt32 i = 0 ; i < m_inputSeedPoints->GetCount(); ++i)
			{
				blob[i] = (*m_inputSeedPoints)[i];
			}
		}
	}
private:
	const DynamicClientSeedPointArray* m_inputSeedPoints;
};



// use an alias for "kynapse" seed points to avoid confusion between seedpoints in Kynapse coordinates and "client" seedpoints
typedef ClientSeedPoint SeedPoint;
typedef SeedPointClientPosition SeedPointKynapsePosition;
typedef DynamicClientSeedPointArray DynamicKynapseSeedPointArray; // to differentiate seedpoints in Kynapse coord
typedef ClientSeedPointArray SeedPointArray;
typedef ClientSeedPointArrayBlobBuilder SeedPointArrayBlobBuilder;


}


#endif

