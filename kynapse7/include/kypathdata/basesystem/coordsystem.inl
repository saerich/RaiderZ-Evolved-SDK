/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




// primary contact: GUAL - secondary contact: MAMU
/*! \file
	\ingroup kypathdata_basesystem */

namespace Kaim
{


namespace CoordSystem_Internal
{
	inline void SetAxis(CoordSystem::ClientAxis axis, KyInt32& axisIdx, KyFloat32& axisSign)
	{
		axisIdx = axis / 2;
		axisSign = (axis % 2) ? -1.0f : 1.0f;
	}

	inline CoordSystem::ClientAxis GetAxis(KyInt32 axisIdx, KyFloat32 axisSign)
	{
		if (axisSign > 0.0f) //axisSign is 1.f or -1.f
			return (CoordSystem::ClientAxis)(axisIdx*2);
		else
			return (CoordSystem::ClientAxis)((axisIdx*2) + 1);
	}

	inline KyInt32 NearestInt(KyFloat32 x)
	{
#if defined (KY_XBOX360) || defined (KY_PS3) || defined(KY_LINUX_ANY) || defined(__GCC__)
		// JAPA - avoid aliasing issues.  We still have a float->int here which is not ideal.
		return (KyInt32)( x + Kaim::Fsel(x, 0.5f, -0.5f) );
#else
		const KyFloat32 _0_5 = 0.5f;
		KyInt32 offset = (0x80000000 & (KyUInt32&)x) | (KyUInt32&)_0_5;
		return (KyInt32)(x + (KyFloat32&)offset);
#endif
	}
}


void CoordSystem::Setup(
	KyFloat32 oneMeterInClientUnits,
	ClientAxis clientAxisForKynapseX,
	ClientAxis clientAxisForKynapseY,
	ClientAxis clientAxisForKynapseZ,
	KyFloat32 integerPrecision)
{
	SetOneMeterInClientUnits(oneMeterInClientUnits);
	SetIntegerPrecision(integerPrecision);

	SetClientAxisForKynapseX(clientAxisForKynapseX);
	SetClientAxisForKynapseY(clientAxisForKynapseY);
	SetClientAxisForKynapseZ(clientAxisForKynapseZ);

	UpdateClientCoordMax();
}


void CoordSystem::Setup(const CoordSystemConfig& config)
{
	Setup(
		config.m_oneMeterInClientUnits,
		(ClientAxis)config.m_clientAxisForKynapseX,
		(ClientAxis)config.m_clientAxisForKynapseY,
		(ClientAxis)config.m_clientAxisForKynapseZ,
		config.m_integerPrecision);
}


void CoordSystem::SetOneMeterInClientUnits(KyFloat32 oneMeterInClientUnits)
{
	m_oneMeter = oneMeterInClientUnits;
	m_inv_oneMeter = 1.0f / oneMeterInClientUnits;
}

void CoordSystem::SetClientAxisForKynapseX(ClientAxis clientAxisForKynapseX) { CoordSystem_Internal::SetAxis(clientAxisForKynapseX, m_kynapseX_idx, m_kynapseX_sign); }
void CoordSystem::SetClientAxisForKynapseY(ClientAxis clientAxisForKynapseY) { CoordSystem_Internal::SetAxis(clientAxisForKynapseY, m_kynapseY_idx, m_kynapseY_sign); }
void CoordSystem::SetClientAxisForKynapseZ(ClientAxis clientAxisForKynapseZ) { CoordSystem_Internal::SetAxis(clientAxisForKynapseZ, m_kynapseZ_idx, m_kynapseZ_sign); }

CoordSystem::ClientAxis CoordSystem::GetClientAxisForKynapseX() { return CoordSystem_Internal::GetAxis(m_kynapseX_idx, m_kynapseX_sign); }
CoordSystem::ClientAxis CoordSystem::GetClientAxisForKynapseY() { return CoordSystem_Internal::GetAxis(m_kynapseY_idx, m_kynapseY_sign); }
CoordSystem::ClientAxis CoordSystem::GetClientAxisForKynapseZ() { return CoordSystem_Internal::GetAxis(m_kynapseZ_idx, m_kynapseZ_sign); }

void CoordSystem::SetIntegerPrecision(KyFloat32 precision)
{
	m_integerPrecision = precision;
	m_inv_integerPrecision = 1.0f / precision;
}

void CoordSystem::UpdateClientCoordMax()
{
	m_clientCoordMax = IntToClient_Dist(KyInt32MAXVAL - 1); // inferior to 2**31;
}

// ------------------------------------ Client <-> Kynapse ------------------------------------
KyFloat32 CoordSystem::ClientToKynapse_Dist(KyFloat32 clientDist) const { return clientDist * m_inv_oneMeter; }
KyFloat32 CoordSystem::KynapseToClient_Dist(KyFloat32 kynapseDist) const { return kynapseDist * m_oneMeter; }

KyFloat32 CoordSystem::ClientToKynapse_SquareDist(KyFloat32 clientSquareDist) const { return clientSquareDist * m_inv_oneMeter * m_inv_oneMeter; }
KyFloat32 CoordSystem::KynapseToClient_SquareDist(KyFloat32 kynapseSquareDist) const { return kynapseSquareDist * m_oneMeter * m_oneMeter; }

void CoordSystem::ClientToKynapse_Pos(const Vec3f& clientPos, Vec3f& kynapsePos) const
{
	kynapsePos.x = ClientToKynapse_Dist(m_kynapseX_sign * clientPos[m_kynapseX_idx]);
	kynapsePos.y = ClientToKynapse_Dist(m_kynapseY_sign * clientPos[m_kynapseY_idx]);
	kynapsePos.z = ClientToKynapse_Dist(m_kynapseZ_sign * clientPos[m_kynapseZ_idx]);
}
void CoordSystem::KynapseToClient_Pos(const Vec3f& kynapsePos, Vec3f& clientPos) const
{
	clientPos[m_kynapseX_idx] = KynapseToClient_Dist(m_kynapseX_sign * kynapsePos.x);
	clientPos[m_kynapseY_idx] = KynapseToClient_Dist(m_kynapseY_sign * kynapsePos.y);
	clientPos[m_kynapseZ_idx] = KynapseToClient_Dist(m_kynapseZ_sign * kynapsePos.z);
}

Vec3f CoordSystem::ClientToKynapse_Pos(const Vec3f& clientPos) const
{
	Vec3f kynapsePos;
	ClientToKynapse_Pos(clientPos, kynapsePos);
	return kynapsePos;
}

Vec3f CoordSystem::KynapseToClient_Pos(const Vec3f& kynapsePos) const
{
	Vec3f clientPos;
	KynapseToClient_Pos(kynapsePos, clientPos);
	return clientPos;
}

void CoordSystem::ClientToKynapse_Normal(const Vec3f& clientNormal, Vec3f& kynapseNormal) const
{
	kynapseNormal.x = m_kynapseX_sign * clientNormal[m_kynapseX_idx];
	kynapseNormal.y = m_kynapseY_sign * clientNormal[m_kynapseY_idx];
	kynapseNormal.z = m_kynapseZ_sign * clientNormal[m_kynapseZ_idx];
}
void CoordSystem::KynapseToClient_Normal(const Vec3f& kynapseNormal, Vec3f& clientNormal) const
{
	clientNormal[m_kynapseX_idx] = m_kynapseX_sign * kynapseNormal.x;
	clientNormal[m_kynapseY_idx] = m_kynapseY_sign * kynapseNormal.y;
	clientNormal[m_kynapseZ_idx] = m_kynapseZ_sign * kynapseNormal.z;
}

Vec3f CoordSystem::ClientToKynapse_Normal(const Vec3f& clientNormal) const
{
	Vec3f kynapseNormal;
	ClientToKynapse_Normal(clientNormal, kynapseNormal);
	return kynapseNormal;
}

Vec3f CoordSystem::KynapseToClient_Normal(const Vec3f& kynapseNormal) const
{
	Vec3f clientNormal;
	KynapseToClient_Normal(kynapseNormal, clientNormal);
	return clientNormal;
}

void CoordSystem::ClientToKynapse_Box(const Vec3f& clientMin, const Vec3f& clientMax, Box3f& kyBox)
{
	kyBox.Clear();
	kyBox.ExpandByVec3(ClientToKynapse_Pos(clientMin));
	kyBox.ExpandByVec3(ClientToKynapse_Pos(clientMax));
}

void CoordSystem::KynapseToClient_Box(const Box3f& kyBox, Vec3f& clientMin, Vec3f& clientMax)
{
	Box3f clientBox;
	clientBox.ExpandByVec3(KynapseToClient_Pos(kyBox.m_min));
	clientBox.ExpandByVec3(KynapseToClient_Pos(kyBox.m_max));
	clientMin = clientBox.m_min;
	clientMax = clientBox.m_max;
}


// ------------------------------------ Client <-> Int ------------------------------------
KyInt32 CoordSystem::ClientToInt_Dist(KyFloat32 clientCoord) const { return CoordSystem_Internal::NearestInt(clientCoord * m_inv_oneMeter * m_inv_integerPrecision); }
KyFloat32 CoordSystem::IntToClient_Dist(KyInt32 intCoord) const { return intCoord * m_integerPrecision * m_oneMeter; }

void CoordSystem::ClientToInt_Pos(const Vec3f& clientPos, Vec3i& intPos) const
{
	intPos.x = ClientToInt_Dist((KyInt32)m_kynapseX_sign * clientPos[m_kynapseX_idx]);
	intPos.y = ClientToInt_Dist((KyInt32)m_kynapseY_sign * clientPos[m_kynapseY_idx]);
	intPos.z = ClientToInt_Dist((KyInt32)m_kynapseZ_sign * clientPos[m_kynapseZ_idx]);
}
void CoordSystem::IntToClient_Pos(const Vec3i& intPos, Vec3f& clientPos) const
{
	clientPos[m_kynapseX_idx] = IntToClient_Dist((KyInt32)m_kynapseX_sign * intPos.x);
	clientPos[m_kynapseY_idx] = IntToClient_Dist((KyInt32)m_kynapseY_sign * intPos.y);
	clientPos[m_kynapseZ_idx] = IntToClient_Dist((KyInt32)m_kynapseZ_sign * intPos.z);
}


// ------------------------------------ Kynapse <-> Int ------------------------------------
KyInt32 CoordSystem::KynapseToInt_Dist(KyFloat32 kynapseDist) const { return KynapseToInt_Dist(kynapseDist, m_inv_integerPrecision); }
KyFloat32 CoordSystem::IntToKynapse_Dist(KyInt32 intDist) const { return IntToKynapse_Dist(intDist, m_integerPrecision); }

void CoordSystem::KynapseToInt_Pos(const Vec3f& kynapsePos, Vec3i& intPos) const { return KynapseToInt_Pos(kynapsePos, intPos, m_inv_integerPrecision); }
void CoordSystem::IntToKynapse_Pos(const Vec3i& intPos, Vec3f& kynapsePos) const { return IntToKynapse_Pos(intPos, kynapsePos, m_integerPrecision); }

void CoordSystem::KynapseToInt_Pos(const Vec2f& kynapsePos, Vec2i& intPos) const { return KynapseToInt_Pos(kynapsePos, intPos, m_inv_integerPrecision); }
void CoordSystem::IntToKynapse_Pos(const Vec2i& intPos, Vec2f& kynapsePos) const { return IntToKynapse_Pos(intPos, kynapsePos, m_integerPrecision); }


// ----------------------- Kynapse <-> Int with custom integerPrecision --------------------------
KyInt32 CoordSystem::KynapseToInt_Dist(KyFloat32 kynapseDist, KyFloat32 inv_integerPrecision) { return CoordSystem_Internal::NearestInt(kynapseDist * inv_integerPrecision); }
KyFloat32 CoordSystem::IntToKynapse_Dist(KyInt32 intDist, KyFloat32 integerPrecision) { return intDist * integerPrecision; }

void CoordSystem::KynapseToInt_Pos(const Vec3f& kynapsePos, Vec3i& intPos, KyFloat32 inv_integerPrecision)
{
	intPos.x = KynapseToInt_Dist(kynapsePos.x, inv_integerPrecision);
	intPos.y = KynapseToInt_Dist(kynapsePos.y, inv_integerPrecision);
	intPos.z = KynapseToInt_Dist(kynapsePos.z, inv_integerPrecision);
}
void CoordSystem::IntToKynapse_Pos(const Vec3i& intPos, Vec3f& kynapsePos, KyFloat32 integerPrecision)
{
	kynapsePos.x = IntToKynapse_Dist(intPos.x, integerPrecision);
	kynapsePos.y = IntToKynapse_Dist(intPos.y, integerPrecision);
	kynapsePos.z = IntToKynapse_Dist(intPos.z, integerPrecision);
}

void CoordSystem::KynapseToInt_Pos(const Vec2f& kynapsePos, Vec2i& intPos, KyFloat32 inv_integerPrecision)
{
	intPos.x = KynapseToInt_Dist(kynapsePos.x, inv_integerPrecision);
	intPos.y = KynapseToInt_Dist(kynapsePos.y, inv_integerPrecision);
}
void CoordSystem::IntToKynapse_Pos(const Vec2i& intPos, Vec2f& kynapsePos, KyFloat32 integerPrecision   )
{
	kynapsePos.x = IntToKynapse_Dist(intPos.x, integerPrecision);
	kynapsePos.y = IntToKynapse_Dist(intPos.y, integerPrecision);
}


void CoordSystem::Convert_Pos(Coordinates coordinates_in, const Vec3f& pos_in, Coordinates coordinates_out, Vec3f& pos_out)
{
	KyUInt32 case_id = (KyUInt32)coordinates_in + (KyUInt32)coordinates_out * 2;
	switch (case_id)
	{
		//                                                      in      => out
		case 0: pos_out = pos_in;                     break; // CLIENT  => CLIENT
		case 1: KynapseToClient_Pos(pos_in, pos_out); break; // KYNAPSE => CLIENT
		case 2: ClientToKynapse_Pos(pos_in, pos_out); break; // CLIENT  => KYNAPSE
		case 3: pos_out = pos_in;                     break; // KYNAPSE => KYNAPSE
	}
}


bool CoordSystem::IsInsideClientCoordBox(const Vec3f& clientPos) const
{
	if (clientPos.x < m_clientCoordMax && clientPos.x > -m_clientCoordMax &&
		clientPos.y < m_clientCoordMax && clientPos.y > -m_clientCoordMax &&
		clientPos.z < m_clientCoordMax && clientPos.z > -m_clientCoordMax)
		return true;
	return false;
}


KyFloat32 CoordSystem::GetMaxEdgeLengthOfTriangle(const Vec3f& A, const Vec3f& B, const Vec3f& C) const
{
	const Vec3f AB = B - A;
	const Vec3f BC = C - B;
	const Vec3f CA = A - C;
	return Kaim::Max(AB.GetLength(), Kaim::Max(BC.GetLength(), CA.GetLength()));
}

bool CoordSystem::IsClientTriangleTooBig(const Vec3f& A, const Vec3f& B, const Vec3f& C) const
{
	const KyFloat32 maxLength = IntToClient_Dist(KyInt32(sqrtf(KyFloat32(KyInt32MAXVAL))) - 1);
	return GetMaxEdgeLengthOfTriangle(A, B, C) >= maxLength;
}


bool CoordSystem::operator==(const CoordSystem& other) const
{
	return m_kynapseX_idx == other.m_kynapseX_idx && m_kynapseX_sign == other.m_kynapseX_sign
		&& m_kynapseY_idx == other.m_kynapseY_idx && m_kynapseY_sign == other.m_kynapseY_sign
		&& m_kynapseZ_idx == other.m_kynapseZ_idx && m_kynapseZ_sign == other.m_kynapseZ_sign
		&& m_inv_integerPrecision == other.m_inv_integerPrecision
		&& m_integerPrecision == other.m_integerPrecision
		&& m_clientCoordMax == other.m_clientCoordMax
		&& m_oneMeter == other.m_oneMeter;
}

bool CoordSystem::operator!=(const CoordSystem& other) const
{
	return !operator==(other);
}


}

