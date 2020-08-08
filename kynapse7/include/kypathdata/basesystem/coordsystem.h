/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*!	\file
	\ingroup kypathdata_basesystem */

// primary contact: GUAL - secondary contact: MAMU
#ifndef KyPathData_CoordSystem_H
#define KyPathData_CoordSystem_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/blob/blobfieldarray.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/math/vec2f.h"
#include "kypathdata/math/vec3i.h"
#include "kypathdata/math/vec2i.h"
#include "kypathdata/math/vec2i.h"
#include "kypathdata/math/box3f.h"


namespace Kaim
{


/*!	Enumerates the types of coordinates understood by the CoordSystem.
	\ingroup kypathdata_basesystem */
enum Coordinates
{
	COORDINATES_CLIENT  = 0, /*!< Identifies the coordinate system of the \gameOrSim engine. */
	COORDINATES_KYNAPSE = 1, /*!< Identifies the \SDKName coordinate system. */
	COORDINATES_FORCE32 = 0xFFFFFFFF
};


/*!	Enumerates the possible axis orientations that can be assigned to each of the Right,
	Front, and Up axes used within \SDKName. Used in calls to CoordSystem::Setup() and CoordSystem::SetAxes().
	\ingroup kypathdata_basesystem */
enum CoordSystemClientAxis
{
	CLIENT_X       = 0, /*!< Represents the positive direction of the X axis. */
	CLIENT_MINUS_X = 1, /*!< Represents the negative direction of the X axis. */
	CLIENT_Y       = 2, /*!< Represents the positive direction of the Y axis. */
	CLIENT_MINUS_Y = 3, /*!< Represents the negative direction of the Y axis. */
	CLIENT_Z       = 4, /*!< Represents the positive direction of the Z axis. */
	CLIENT_MINUS_Z = 5, /*!< Represents the negative direction of the Z axis. */
	CLIENT_FORCE32 = 0xFFFFFFFF
};


/*!	A structure that sets up the mapping between the coordinate system used in the \SDKName Engine and the \gameOrSim engine.
	\ingroup kypathdata_basesystem */
class CoordSystemConfig
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor Sets the data members of this class to default values. */
	CoordSystemConfig()
	{
		m_oneMeterInClientUnits = 1.0f;
		m_clientAxisForKynapseX = (KyUInt32)CLIENT_X;
		m_clientAxisForKynapseY = (KyUInt32)CLIENT_Y;
		m_clientAxisForKynapseZ = (KyUInt32)CLIENT_Z;
		m_integerPrecision = 0.01f; // 1 cm
	}

	/*! \label_init
		\param oneMeterInClientUnits		Sets the value of #m_oneMeterInClientUnits.
		\param clientAxisForKynapseX		Sets the value of #m_clientAxisForKynapseX.
		\param clientAxisForKynapseY		Sets the value of #m_clientAxisForKynapseY.
		\param clientAxisForKynapseZ		Sets the value of #m_clientAxisForKynapseZ.
		\param integerPrecisionInMeters		Sets the value of #m_integerPrecision.
		*/
	void Init(
		KyFloat32 oneMeterInClientUnits,
		KyUInt32 clientAxisForKynapseX,
		KyUInt32 clientAxisForKynapseY,
		KyUInt32 clientAxisForKynapseZ,
		KyFloat32 integerPrecisionInMeters = 0.01f)
	{
		m_oneMeterInClientUnits = oneMeterInClientUnits;
		m_clientAxisForKynapseX = clientAxisForKynapseX;
		m_clientAxisForKynapseY = clientAxisForKynapseY;
		m_clientAxisForKynapseZ = clientAxisForKynapseZ;
		m_integerPrecision = integerPrecisionInMeters;
	}

	/*! For internal use. */
	void AddMapping(BlobFieldsMapping& mapping)
	{
		mapping.AddFloat32("OneMeterInClientUnits", m_oneMeterInClientUnits);
		mapping.AddUInt32("ClientAxisForKynapseX", m_clientAxisForKynapseX);
		mapping.AddUInt32("ClientAxisForKynapseY", m_clientAxisForKynapseY);
		mapping.AddUInt32("ClientAxisForKynapseZ", m_clientAxisForKynapseZ);
		mapping.AddFloat32("IntegerPrecision", m_integerPrecision);
	}

	/*! Returns \c true if all data members in the specified CoordSystemConfig match all data members in this object. */
	bool operator==(const CoordSystemConfig& other) const
	{
		return m_oneMeterInClientUnits == other.m_oneMeterInClientUnits
			&& m_clientAxisForKynapseX == other.m_clientAxisForKynapseX
			&& m_clientAxisForKynapseY == other.m_clientAxisForKynapseY
			&& m_clientAxisForKynapseZ == other.m_clientAxisForKynapseZ
			&& m_integerPrecision      == other.m_integerPrecision;
	}

	/*! Returns \c true if at least one data member in the specified CoordSystemConfig does not match the corresponding
		data member in this object. */
	bool operator!=(const CoordSystemConfig& other) const { return !operator==(other); }

public:
	/*! The number of world units in the \gameOrSim engine that add up to one meter in length. */
	KyFloat32 m_oneMeterInClientUnits;
	/*! The axis in the \gameOrSim engine coordinate system that should be mapped to the X axis within the \SDKName coordinate system. */
	KyUInt32 m_clientAxisForKynapseX;
	/*! The axis in the \gameOrSim engine coordinate system that should be mapped to the Y axis within the \SDKName coordinate system. */
	KyUInt32 m_clientAxisForKynapseY;
	/*! The axis in the \gameOrSim engine coordinate system that should be mapped to the Z axis within the \SDKName coordinate system. */
	KyUInt32 m_clientAxisForKynapseZ;
	/*! The length in meters of each integer unit used by the PathData generation system. */
	KyFloat32 m_integerPrecision;
};


/*!	\label_swapendianness
	\ingroup kypathdata_basesystem */
inline void SwapEndianness(Endianness::Target e, CoordSystemConfig& self)
{
	SwapEndianness(e, self.m_oneMeterInClientUnits);
	SwapEndianness(e, self.m_clientAxisForKynapseX);
	SwapEndianness(e, self.m_clientAxisForKynapseY);
	SwapEndianness(e, self.m_clientAxisForKynapseZ);
	SwapEndianness(e, self.m_integerPrecision);
}


/*!	This class maintains a mapping between the system of coordinate axes used internally within the
	\gameOrSim engine, the system used within the \SDKName engine at runtime, and the system used by
	the \SDKName PathData generation tools. This mapping is used to convert coordinates and distances
	automatically back and forth between the three systems.
	\pp This class represents the mapping between global spaces by storing internally:
	-	Which axis in the \gameOrSim engine relates to each of the X (Right), Y (Front) and Z (Up) axes within \SDKName.
	-	Whether or not the direction of each of those axes must be reversed (or negated).
	-	The number of units in the \gameOrSim engine world that add up to one meter.
	\pp Internally, all \SDKName components use a global space in which the X, Y and Z axes represent
	the Right, Up and Front axes respectively. In the runtime coordinate system, all distances are expressed
	and stored internally in meters using floating-point numbers. In the PathData generation coordinate system, all
	distances are expressed and stored internally in meters using integers.
	\pp When you open the \SDKName Engine, an instance of the CoordSystem class is automatically created.
	You can access this instance by calling GetBaseSystem().GetCoordSystem(). In your initialization
	code, you must retrieve this instance, and set it up with the correct mapping to the coordinate
	system used in your \gameOrSim engine by calling Setup().
	\pp To convert values between different coordinate systems, you can use the class methods of this object
	directly, or use the conversion macros defined in the coordsystemmacros.inl file.
	\pp For more information, see :USERGUIDE:"The \SDKName Coordinate System" and :USERGUIDE:"Integrating \SDKName".
	\ingroup kypathdata_basesystem */
class CoordSystem
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:

	/*! \label_blobtype */
	static const char* GetBlobType() { return "CoordSystem"; }
	//version history:
	/*! \label_blobversion */
	// 0: initial version
	static KyUInt32 GetBlobVersion() { return 0; }

	/*!	A type that represents the possible axis orientations that can be assigned to each of the Right,
		Front, and Up axes used within \SDKName. Used in calls to CoordSystem::Setup() and CoordSystem::SetAxes(). */
	typedef CoordSystemClientAxis ClientAxis;
	/*!	Represents the positive direction of the X axis in the \gameOrSim engine. */
	static const ClientAxis CLIENT_X        = Kaim::CLIENT_X;
	/*!	Represents the negative direction of the X axis in the \gameOrSim engine. */
	static const ClientAxis CLIENT_MINUS_X  = Kaim::CLIENT_MINUS_X;
	/*!	Represents the positive direction of the Y axis in the \gameOrSim engine. */
	static const ClientAxis CLIENT_Y        = Kaim::CLIENT_Y;
	/*!	Represents the negative direction of the X axis in the \gameOrSim engine. */
	static const ClientAxis CLIENT_MINUS_Y  = Kaim::CLIENT_MINUS_Y;
	/*!	Represents the positive direction of the Z axis in the \gameOrSim engine. */
	static const ClientAxis CLIENT_Z        = Kaim::CLIENT_Z;
	/*!	Represents the negative direction of the Z axis in the \gameOrSim engine. */
	static const ClientAxis CLIENT_MINUS_Z  = Kaim::CLIENT_MINUS_Z;
	/*!	For internal use. */
	static const ClientAxis CLIENT_FORCE32  = Kaim::CLIENT_FORCE32;


public:
	/*!	\label_constructor For internal use. */
	CoordSystem() { Setup(1.0f, CLIENT_X, CLIENT_Y, CLIENT_Z); }

	/*!	Sets the mapping between the coordinate system used in the \SDKName Engine and the \gameOrSim engine.
		\param oneMeterInClientUnits		The number of world units in the \gameOrSim engine that add up to one meter in length.
		\param clientAxisForKynapseX		The axis in the \gameOrSim engine coordinate system that should be mapped to the X axis within the \SDKName coordinate system.
		\param clientAxisForKynapseY		The axis in the \gameOrSim engine coordinate system that should be mapped to the Y axis within the \SDKName coordinate system.
		\param clientAxisForKynapseZ		The axis in the \gameOrSim engine coordinate system that should be mapped to the Z axis within the \SDKName coordinate system.
		\param integerPrecision				The length in meters of each integer unit used by the PathData generation system. */
	inline void Setup(
		KyFloat32 oneMeterInClientUnits,
		ClientAxis clientAxisForKynapseX,
		ClientAxis clientAxisForKynapseY,
		ClientAxis clientAxisForKynapseZ,
		KyFloat32 integerPrecision = 0.01f);

	/*! Sets the mapping between the coordinate system used in the \SDKName Engine and the \gameOrSim engine using the
		values set in the specified CoordSystemConfig object. */
	inline void Setup(const CoordSystemConfig& config);

	/*!	Sets the number of world units in the \gameOrSim engine that add up to one meter in length. */
	inline void SetOneMeterInClientUnits(KyFloat32 oneMeter);

	/*!	Sets the axis in the \gameOrSim engine coordinate system that should be mapped to the X axis within the \SDKName coordinate system. */
	inline void SetClientAxisForKynapseX(ClientAxis clientAxisForKynapseX);

	/*!	Sets the axis in the \gameOrSim engine coordinate system that should be mapped to the Y axis within the \SDKName coordinate system. */
	inline void SetClientAxisForKynapseY(ClientAxis clientAxisForKynapseY);

	/*!	Sets the axis in the \gameOrSim engine coordinate system that should be mapped to the Z axis within the \SDKName coordinate system. */
	inline void SetClientAxisForKynapseZ(ClientAxis clientAxisForKynapseZ);

	/*!	Sets the length in meters of each integer unit used by the PathData generation system. */
	inline void SetIntegerPrecision(KyFloat32 precision);

	/*!	Updates the maximum coordinate that the PathData generation system can accept from the \gameOrSim engine, given the current #m_integerPrecision
		and #m_oneMeter settings. */
	inline void UpdateClientCoordMax();

	/*!	Retrieves the axis in the \gameOrSim engine coordinate system that is mapped to the X axis within the \SDKName coordinate system. */
	inline ClientAxis GetClientAxisForKynapseX();

	/*!	Retrieves the axis in the \gameOrSim engine coordinate system that is mapped to the Y axis within the \SDKName coordinate system. */
	inline ClientAxis GetClientAxisForKynapseY();

	/*!	Retrieves the axis in the \gameOrSim engine coordinate system that is mapped to the Z axis within the \SDKName coordinate system. */
	inline ClientAxis GetClientAxisForKynapseZ();

public:
	// ------------------ Client <-> Kynapse ------------------
	/*!	Converts a distance measurement from the \gameOrSim engine coordinate system to the \SDKName coordinate system.
		\param clientDist			The distance to convert, expressed in \gameOrSim engine units.
		\return The converted distance, expressed in meters. */
	inline KyFloat32 ClientToKynapse_Dist(KyFloat32 clientDist) const;  // Dist

	/*!	Converts a distance measurement from the \SDKName coordinate system to the \gameOrSim engine coordinate system.
		\param kynapseDist			The distance to convert, expressed in meters.
		\return The converted distance, expressed in \gameOrSim engine units. */
	inline KyFloat32 KynapseToClient_Dist(KyFloat32 kynapseDist) const; // Dist

	/*!	Converts a squared distance measurement from the \gameOrSim engine coordinate system to the \SDKName coordinate system.
		\param clientSquareDist		The squared distance to convert, expressed in \gameOrSim engine units.
		\return The converted squared distance, expressed in meters. */
	inline KyFloat32 ClientToKynapse_SquareDist(KyFloat32 clientSquareDist) const;  // SquareDist

	/*!	Converts a squared distance measurement from the \SDKName coordinate system to the \gameOrSim engine coordinate system.
		\param kynapseSquareDist		The squared distance to convert, expressed in meters.
		\return The converted squared distance, expressed in \gameOrSim engine units. */
	inline KyFloat32 KynapseToClient_SquareDist(KyFloat32 kynapseSquareDist) const; // SquareDist

	/*!	Converts a position from the \gameOrSim engine coordinate system to the \SDKName coordinate system.
		\param clientPos				The position to convert, expressed in the \gameOrSim engine coordinate system.
		\param[out] kynapsePos			Used to store the converted position, expressed in the \SDKName coordinate system. */
	inline void ClientToKynapse_Pos(const Vec3f& clientPos, Vec3f& kynapsePos) const; // Pos

	/*!	Converts a position from the \SDKName engine coordinate system to the \gameOrSim coordinate system.
		\param kynapsePos				The position to convert, expressed in the \SDKName coordinate system.
		\param[out] clientPos			Used to store the converted position, expressed in the \gameOrSim engine coordinate system. */
	inline void KynapseToClient_Pos(const Vec3f& kynapsePos, Vec3f& clientPos) const; // Pos

	/*!	Converts a position from the \gameOrSim engine coordinate system to the \SDKName coordinate system.
		\param clientPos				The position to convert, expressed in the \gameOrSim engine coordinate system.
		\return The converted position, expressed in the \SDKName coordinate system. */
	inline Vec3f ClientToKynapse_Pos(const Vec3f& clientPos) const;                   // Pos

	/*!	Converts a position from the \SDKName coordinate system to the \gameOrSim engine coordinate system.
		\param kynapsePos				The position to convert, expressed in the \SDKName coordinate system.
		\return The converted position, expressed in the \gameOrSim engine coordinate system. */
	inline Vec3f KynapseToClient_Pos(const Vec3f& kynapsePos) const;                  // Pos

	/*!	Converts a normalized vector from the \gameOrSim engine coordinate system to the \SDKName coordinate system.
		\param clientNormal				The normal to convert, expressed in the \gameOrSim engine coordinate system.
		\param[out] kynapseNormal		Used to store the converted normal, expressed in the \SDKName coordinate system. */
	inline void ClientToKynapse_Normal(const Vec3f& clientNormal, Vec3f& kynapseNormal) const; // Normal

	/*!	Converts a normalized vector from the \SDKName coordinate system to the \gameOrSim engine coordinate system.
		\param clientNormal				The normal to convert, expressed in the \SDKName coordinate system.
		\param[out] kynapseNormal		Used to store the converted normal, expressed in the \gameOrSim engine coordinate system. */
	inline void KynapseToClient_Normal(const Vec3f& kynapseNormal, Vec3f& clientNormal) const; // Normal

	/*!	Converts a normalized vector from the \gameOrSim engine coordinate system to the \SDKName coordinate system.
		\param clientNormal				The normal to convert, expressed in the \gameOrSim engine coordinate system.
		\return The converted normal, expressed in the \SDKName coordinate system. */
	inline Vec3f ClientToKynapse_Normal(const Vec3f& clientNormal) const;                      // Normal

	/*!	Converts a normalized vector from the \SDKName coordinate system to the \gameOrSim engine coordinate system.
		\param kynapseNormal			The normal to convert, expressed in the \SDKName coordinate system.
		\return The converted normal, expressed in the \gameOrSim engine coordinate system. */
	inline Vec3f KynapseToClient_Normal(const Vec3f& kynapseNormal) const;                     // Normal

	/*!	Converts a bounding box from the \gameOrSim engine coordinate system to the \SDKName coordinate system.
		\param clientMin				The minima of the bounding box to convert, expressed in the \gameOrSim engine coordinate system.
		\param clientMax				The maxima of the bounding box to convert, expressed in the \gameOrSim engine coordinate system.
		\param[out] kyBox				Used to store the converted bounding box, expressed in the \SDKName coordinate system. */
	inline void ClientToKynapse_Box(const Vec3f& clientMin, const Vec3f& clientMax, Box3f& kyBox); // Box

	/*!	Converts a bounding box from the \SDKName coordinate system to the \gameOrSim engine coordinate system.
		\param kyBox					The bounding box to convert, expressed in the \SDKName coordinate system.
		\param[out] clientMin			The minima of the bounding box, expressed in the \gameOrSim engine coordinate system.
		\param[out] clientMax			The maxima of the bounding box, expressed in the \gameOrSim engine coordinate system. */
	inline void KynapseToClient_Box(const Box3f& kyBox, Vec3f& clientMin, Vec3f& clientMax);       // Box

	// ------------------ Client <-> Int ------------------
	/*!	Converts a distance measurement from the \gameOrSim engine coordinate system to an integer, as used in the
		\SDKName PathData generation coordinate system.
		\param clientDist				The distance to convert, expressed in \gameOrSim engine units.
		\return The converted integer distance. */
	inline KyInt32 ClientToInt_Dist(KyFloat32 clientDist) const;              // Dist

	/*!	Converts a distance measurement from an integer, as used in the \SDKName PathData generation coordinate system, to
		the \gameOrSim engine coordinate system.
		\param intDist					The distance to convert, expressed as an integer.
		\return The converted distance, in \gameOrSim engine units. */
	inline KyFloat32 IntToClient_Dist(KyInt32 intDist) const;                 // Dist

	/*!	Converts a position from the \gameOrSim engine coordinate system to the \SDKName PathData generation coordinate system.
		\param clientPos				The position to convert, expressed in the \gameOrSim engine coordinate system.
		\param[out] intPos				The converted position, expressed in the \SDKName PathData generation coordinate system. */
	inline void ClientToInt_Pos(const Vec3f& clientPos, Vec3i& intPos) const; // Vec3

	/*!	Converts a position from the \SDKName PathData generation coordinate system to the \gameOrSim engine coordinate system.
		\param intPos					The position to convert, expressed in the \SDKName PathData generation coordinate system.
		\param[out] clientPos			The converted position, expressed in the \gameOrSim engine coordinate system. */
	inline void IntToClient_Pos(const Vec3i& intPos, Vec3f& clientPos) const; // Vec3

	// ------------------ Kynapse <-> Int ------------------
	/*!	Converts a distance measurement from the \SDKName runtime coordinate system to an integer, as used in the
		\SDKName PathData generation coordinate system.
		\param kynapseDist				The distance to convert, expressed in meters.
		\return The converted integer distance. */
	inline KyInt32 KynapseToInt_Dist(KyFloat32 kynapseDist) const;            // Dist

	/*!	Converts a distance measurement from an integer, as used in the \SDKName PathData generation coordinate system, to
		the \SDKName runtime coordinate system.
		\param intDist					The distance to convert, expressed as an integer.
		\return The converted distance, in meters. */
	inline KyFloat32 IntToKynapse_Dist(KyInt32 intDist) const;                // Dist

	/*!	Converts a position from the \SDKName runtime coordinate system to the \SDKName PathData generation coordinate system.
		\param meterPos					The position to convert, expressed in the \SDKName runtime coordinate system.
		\param[out] intPos				The converted position, expressed in the \SDKName PathData generation coordinate system. */
	inline void KynapseToInt_Pos(const Vec3f& meterPos, Vec3i& intPos    ) const; // Vec3

	/*!	Converts a position from the \SDKName PathData generation coordinate system to the \SDKName runtime coordinate system.
		\param intPos					The position to convert, expressed in the \SDKName PathData generation coordinate system.
		\param[out] kynapsePos			The converted position, expressed in the \SDKName runtime coordinate system. */
	inline void IntToKynapse_Pos(const Vec3i& intPos  , Vec3f& kynapsePos) const; // Vec3

	/*!	Converts a 2D position from the \SDKName runtime coordinate system to the \SDKName PathData generation coordinate system.
		\param kynapsePos				The 2D position to convert, expressed in the \SDKName runtime coordinate system.
		\param[out] intPos				The converted position, expressed in the \SDKName PathData generation coordinate system. */
	inline void KynapseToInt_Pos(const Vec2f& kynapsePos, Vec2i& intPos    ) const; // Vec2

	/*!	Converts a 2D position from the \SDKName PathData generation coordinate system to the \SDKName runtime coordinate system.
		\param intPos					The 2D position to convert, expressed in the \SDKName PathData generation coordinate system.
		\param[out] kynapsePos			The converted position, expressed in the \SDKName runtime coordinate system. */
	inline void IntToKynapse_Pos(const Vec2i& intPos  , Vec2f& kynapsePos) const; // Vec2

	// ----------------------- Kynapse <-> Int with custom integerPrecision --------------------------
	/*!	Converts a distance measurement from the \SDKName runtime coordinate system to an integer (as used in the
		\SDKName PathData generation coordinate system), with a custom integer precision value.
		\param kynapseDist				The distance to convert, expressed in meters.
		\param inv_integerPrecision		One divided by the custom integer precision value.
		\return The converted integer distance. */
	static inline KyInt32 KynapseToInt_Dist(KyFloat32 kynapseDist, KyFloat32 inv_integerPrecision);  // Dist

	/*!	Converts a distance measurement from an integer (as used in the \SDKName PathData generation coordinate system) to
		the \SDKName runtime coordinate system, with a custom integer precision value.
		\param intDist					The distance to convert, expressed as an integer.
		\param integerPrecision			The custom integer precision value.
		\return The converted distance, in meters. */
	static inline KyFloat32 IntToKynapse_Dist(KyInt32 intDist, KyFloat32 integerPrecision);          // Dist

	/*!	Converts a position from the \SDKName runtime coordinate system to the \SDKName PathData generation coordinate system,
		using a custom integer precision value.
		\param kynapsePos				The position to convert, expressed in the \SDKName runtime coordinate system.
		\param[out] intPos				The converted position, expressed in the \SDKName PathData generation coordinate system.
		\param inv_integerPrecision		One divided by the custom integer precision value. */
	static inline void KynapseToInt_Pos(const Vec3f& kynapsePos, Vec3i& intPos    , KyFloat32 inv_integerPrecision); // Vec3

	/*!	Converts a position from the \SDKName PathData generation coordinate system to the \SDKName runtime coordinate system,
		using a custom integer precision value.
		\param intPos					The position to convert, expressed in the \SDKName PathData generation coordinate system.
		\param[out] kynapsePos			The converted position, expressed in the \SDKName runtime coordinate system.
		\param integerPrecision			The custom integer precision value. */
	static inline void IntToKynapse_Pos(const Vec3i& intPos    , Vec3f& kynapsePos, KyFloat32 integerPrecision    ); // Vec3

	/*!	Converts a 2D position from the \SDKName runtime coordinate system to the \SDKName PathData generation coordinate system,
		using a custom integer precision value.
		\param kynapsePos				The 2D position to convert, expressed in the \SDKName runtime coordinate system.
		\param[out] intPos				The converted position, expressed in the \SDKName PathData generation coordinate system.
		\param inv_integerPrecision		One divided by the custom integer precision value. */
	static inline void KynapseToInt_Pos(const Vec2f& kynapsePos, Vec2i& intPos    , KyFloat32 inv_integerPrecision); // Vec2

	/*!	Converts a 2D position from the \SDKName PathData generation coordinate system to the \SDKName runtime coordinate system,
		using a custom integer precision value.
		\param intPos					The 2D position to convert, expressed in the \SDKName PathData generation coordinate system.
		\param[out] kynapsePos			The converted position, expressed in the \SDKName runtime coordinate system.
		\param integerPrecision			The custom integer precision value. */
	static inline void IntToKynapse_Pos(const Vec2i& intPos    , Vec2f& kynapsePos, KyFloat32 integerPrecision    ); // Vec2

	// ------------------ other ------------------
	/*!	Converts a position between two specified coordinate systems.
		\param coordinates_in			The coordinate system of the position to be converted.
		\param pos_in					The position to be converted.
		\param coordinates_out			The final coordinate system of the converted position.
		\param[out] pos_out				The converted position. */
	inline void Convert_Pos(Coordinates coordinates_in, const Vec3f& pos_in, Coordinates coordinates_out, Vec3f& pos_out);

	/*!	Indicates whether or not the specified position is within the valid range of coordinates for the PathData generation system,
		given the current #m_integerPrecision and #m_oneMeter settings. */
	inline bool IsInsideClientCoordBox(const Vec3f& clientPos) const;

	
	/*!	Indicates whether or not the specified triangle (in client coordinates) is too big for the PathData generation system,
		given the current #m_precision and #m_oneMeter settings. */
	inline bool IsClientTriangleTooBig(const Vec3f& A, const Vec3f& B, const Vec3f& C) const;

	/*!	Returns the max edge length among all of the 3 edges of triangle ABC 
		max([|AB|;|BC|;|CA|]) */
	inline KyFloat32 GetMaxEdgeLengthOfTriangle(const Vec3f& A, const Vec3f& B, const Vec3f& C) const;


	/*!	Indicates whether or not the two CoordSystem instances contain the same data members. */
	inline bool operator==(const CoordSystem& other) const;

	/*!	Indicates whether or not the two CoordSystem instances contain at least one different data member. */
	inline bool operator!=(const CoordSystem& other) const;

public:
	// Client axis <=> Kynapse axis
	// kynapse_x = m_kynapseX_sign * clientPos[m_kynapseX_idx];
	// clientPos[m_kynapseX_idx] = m_kynapseX_sign * kynapse_x;
	KyInt32 m_kynapseX_idx; /*!< Identifies the index within each Vec3f that contains \gameOrSim engine coordinates that is mapped to the X axis in the \SDKName coordinate system. */
	KyFloat32 m_kynapseX_sign; /*!< Indicates whether coordinates on the \SDKName X axis need to be negated when converting between the \SDKName and \gameOrSim engine coordinate systems. */

	KyInt32 m_kynapseY_idx; /*!< Identifies the index within each Vec3f that contains \gameOrSim engine coordinates that is mapped to the Y axis in the \SDKName coordinate system. */
	KyFloat32 m_kynapseY_sign; /*!< Indicates whether coordinates on the \SDKName Y axis need to be negated when converting between the \SDKName and \gameOrSim engine coordinate systems. */

	KyInt32 m_kynapseZ_idx; /*!< Identifies the index within each Vec3f that contains \gameOrSim engine coordinates that is mapped to the Z axis in the \SDKName coordinate system. */
	KyFloat32 m_kynapseZ_sign; /*!< Indicates whether coordinates on the \SDKName Z axis need to be negated when converting between the \SDKName and \gameOrSim engine coordinate systems. */

	// client dist <=> kynapse dist
	KyFloat32 m_oneMeter; /*!< The number of world units in the \gameOrSim engine that add up to one meter in length. */
	KyFloat32 m_inv_oneMeter; /*!< One divided by the #m_oneMeter value. */

	// kynapse dist <=> integer dist
	KyFloat32 m_inv_integerPrecision; /*!< One divided by the #m_integerPrecision value. */
	KyFloat32 m_integerPrecision; /*!< The length in meters of each integer unit used by the PathData generation system. */

	// client coord max
	KyFloat32 m_clientCoordMax; /*!< The maximum coordinate that the PathData generation system can accept from the \gameOrSim engine, given the current #m_integerPrecision and #m_oneMeter settings. */
};

/*!	\label_swapendianness
	\ingroup kypathdata_basesystem */
inline void SwapEndianness(Endianness::Target e, CoordSystem& self)
{
	SwapEndianness(e, self.m_kynapseX_idx);
	SwapEndianness(e, self.m_kynapseX_sign);

	SwapEndianness(e, self.m_kynapseY_idx);
	SwapEndianness(e, self.m_kynapseY_sign);

	SwapEndianness(e, self.m_kynapseZ_idx);
	SwapEndianness(e, self.m_kynapseZ_sign);

	SwapEndianness(e, self.m_oneMeter);
	SwapEndianness(e, self.m_inv_oneMeter);

	SwapEndianness(e, self.m_inv_integerPrecision);
	SwapEndianness(e, self.m_integerPrecision);

	SwapEndianness(e, self.m_clientCoordMax);
}


}

#include <kypathdata/basesystem/coordsystem.inl>



#endif

