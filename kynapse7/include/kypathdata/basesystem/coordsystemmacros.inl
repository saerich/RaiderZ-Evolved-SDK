/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_basesystem */

// primary contact: GUAL - secondary contact: NOBODY

// ------------------- client => kynapse -------------------
/*!	Converts a distance measurement from the \gameOrSim engine coordinate system to the \SDKName coordinate system.
	\param in_clientDist			The distance to convert, expressed in \gameOrSim engine units.
	\return A Vec3f that contains the converted distance, expressed in meters.
	\ingroup kypathdata_basesystem */
#define CLIENT2KY_DIST(in_clientDist)                         Kaim::GetCoordSystem().ClientToKynapse_Dist(in_clientDist)

/*!	Converts a position from the coordinate system used in the \gameOrSim engine to the \SDKName coordinate system.  
	\param in_clientPos				The position to convert, expressed in the \gameOrSim engine coordinate system.
	\return A Vec3f that contains the converted position, expressed in the \SDKName coordinate system.
	\ingroup kypathdata_basesystem */
#define CLIENT2KY_POS(in_clientPos)                           Kaim::GetCoordSystem().ClientToKynapse_Pos((const Kaim::Vec3f&)in_clientPos)

/*!	Converts a normalized vector from the coordinate system used in the \gameOrSim engine to the \SDKName coordinate system.  
	\param in_clientNormal			The normal to convert, expressed in the \gameOrSim engine coordinate system.
	\return A Vec3f that contains the converted normal, expressed in the \SDKName coordinate system.
	\ingroup kypathdata_basesystem */
#define CLIENT2KY_NORMAL(in_clientNormal)                     Kaim::GetCoordSystem().ClientToKynapse_Normal((const Kaim::Vec3f&)in_clientNormal)

/*!	Converts an axis-oriented bounding box from the coordinate system used in the \gameOrSim engine to the \SDKName coordinate system.  
	\param in_clientMin				The minima of the bounding box in the \gameOrSim engine coordinate system.
	\param in_clientMax				The maxima of the bounding box in the \gameOrSim engine coordinate system.
	\param[out] out_kyBox			A Box3f object that expresses the bounding box in the \SDKName coordinate system.
	\ingroup kypathdata_basesystem */
#define CLIENT2KY_AABB(in_clientMin, in_clientMax, out_kyBox) Kaim::GetCoordSystem().ClientToKynapse_Box((const Kaim::Vec3f&)in_clientMin, (const Kaim::Vec3f&)in_clientMax, out_kyBox)

// ------------------- kynapse => client -------------------
/*!	Converts a distance measurement from the \SDKName coordinate system to the \gameOrSim engine coordinate system.
	\param in_kynapseDist			The distance to convert, expressed in meters.
	\return A Vec3f that contains the converted distance, expressed in \gameOrSim engine units.
	\ingroup kypathdata_basesystem */
#define KY2CLIENT_DIST(in_kynapseDist)                              Kaim::GetCoordSystem().KynapseToClient_Dist(in_kynapseDist)

/*!	Converts a position from the \SDKName coordinate system to the \gameOrSim engine coordinate system.
	\param in_kynapsePos			The position to convert, expressed in the \SDKName coordinate system.
	\param[out] out_clientPos		Stores the calculated position, expressed in the \SDKName coordinate system.
	\return A Vec3f that contains the converted position, expressed in the \gameOrSim engine coordinate system.
	\ingroup kypathdata_basesystem */
#define KY2CLIENT_POS(in_kynapsePos, out_clientPos)                 Kaim::GetCoordSystem().KynapseToClient_Pos(in_kynapsePos, (Kaim::Vec3f&)out_clientPos)

/*!	Converts a normalized vector from the \SDKName coordinate system to the \gameOrSim engine coordinate system.
	\param in_kynapseNormal			The normal to convert, expressed in the \SDKName coordinate system.
	\param[out] out_clientNormal	Stores the calculated normal, expressed in the \SDKName coordinate system.
	\return A Vec3f that contains the converted normal, expressed in the \gameOrSim engine coordinate system.
	\ingroup kypathdata_basesystem */
#define KY2CLIENT_NORMAL(in_kynapseNormal, out_clientNormal)        Kaim::GetCoordSystem().KynapseToClient_Normal(in_kynapseNormal, (Kaim::Vec3f&)out_clientNormal)

/*!	Converts an axis-oriented bounding box from the \SDKName coordinate system to the \gameOrSim engine coordinate system.
	\param in_kynapseBox			A Box3f object that expresses the bounding box in the \SDKName coordinate system.
	\param[out] out_clientMin		The minima of the bounding box in the \gameOrSim engine coordinate system.
	\param[out] out_clientMax		The maxima of the bounding box in the \gameOrSim engine coordinate system.
	\ingroup kypathdata_basesystem */
#define KY2CLIENT_AABB(in_kynapseBox, out_clientMin, out_clientMax) Kaim::GetCoordSystem().KynapseToClient_Box(in_kynapseBox, (Kaim::Vec3f&)out_clientMin, (Kaim::Vec3f&)out_clientMax)

