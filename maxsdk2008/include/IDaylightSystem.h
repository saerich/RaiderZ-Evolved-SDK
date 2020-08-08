//*****************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//*****************************************************************************/
// DESCRIPTION: Interface for creating and modifying daylight systems programatically.
// AUTHOR: Chloe Mignot - created April 27 2007
//*****************************************************************************/


#ifndef DAYLIGHTFACTORY_H
#define DAYLIGHTFACTORY_H

#include "ifnpub.h"

class INode;
class Point3;
class LightObject;
class IDaylightSystem;

// --- Interface IDs
#define DAYLIGHTSYSTEM_FACTORY_INTERFACE Interface_ID(0x5c665bc7, 0x61b44a30)

/*! \brief Creates Daylight systems 
* Use this interface to create a daylight system in the scene without the UI.
* Note that:
*	The type of sun and sky component created depends on the current Market Defaults.
*/
class IDaylightSystemFactory : public FPStaticInterface
{
public:

	/*! \brief Creates a daylight system and adds it to the scene, at a default position of (0,0,0)
	* \param [in,out] pDaylight returns the component of the daylight system which can be used to access
	* the sun and sky. See IDaylightSystem.
	* \return A pointer on the Daylight assembly node of the daylight system.
	*/
	virtual INode* Create(
		IDaylightSystem*& pDaylight
		) = 0;

	/*! \brief Retrieves the single instance of the Daylight system factory
	*/
	static IDaylightSystemFactory* GetInstance()	{
		return static_cast<IDaylightSystemFactory*>(GetCOREInterface(DAYLIGHTSYSTEM_FACTORY_INTERFACE));
	}
};

// --- Interface IDs
#define DAYLIGHT_SYSTEM_INTERFACE Interface_ID(0x20033f9f, 0x6a65470)

/*! \brief Use this interface to access the sun and sky components of a daylight 
* system created by IDaylightSystemFactory.
* See Also: class IDaylightSystemFactory
*/
class IDaylightSystem : public FPMixinInterface 
{
public:
	/*! \brief Returns the sun component of the daylight system.
	* It can be directional light, IES sun or MR sun depending on the market defaults.
	* \return pointer on the sun.
	*/
	virtual LightObject* GetSun() const = 0;

	/*! \brief Returns the sky component of the daylight system.
	* It can be standard sky, IES sky or MR sky depending on the market defaults.
	* \return pointer on the sky.
	*/
	virtual LightObject* GetSky() const = 0;

	/*! \brief Sets the position of the compass of the daylight system.
	* \param [in] position Origin position of the system
	*/
	virtual void SetPosition(const Point3& position) = 0;

	/*! \brief Returns the position of the compass of the daylight system.
	* \return position of the compass of the daylight system.
	*/
	virtual Point3 GetPosition() const = 0;

	/*! \brief Sets the orbital scale of the system.
	* \param [in] orbScale  distance of the sun (the directional light) from the compass rose (orbital scale in the UI)
	* Note that the orbital scale does not have an effect on the sun's precision or how it's rendered.
	*/
	virtual void  SetOrbitalScale(float orbScale) = 0;

	/*! \brief Returns the orbital scale of the system.
	* \return orbital scale value.
	* Note that the orbital scale does not have an effect on the sun's precision or how it's rendered.
	*/
	virtual float GetOrbitalScale() const = 0;

	/*! \brief Sets the value of the north direction.
	* \param [in] angle rotational direction of the compass rose in the scene (north direction in the UI)
	* The angle is in degree. The value is expected to be >=0.
	*/
	virtual void SetNorthDirection(float angle) = 0;

	/*! \brief Returns the value of the north direction.
	* \return the value of the north direction.
	* The angle is in degree. The value is expected to be >=0.
	*/
	virtual float GetNorthDirection() const = 0;

	/*! \brief Sets the compass diameter of this daylight.
	* \param [in] compassDiameter this parameter controls the diameter of the compass rose.
	* Note that the compass diameter does not have an effect on the sun's precision or how it's rendered.
	* The value is expected to be >=0.
	*/
	virtual void SetCompassDiameter(float compassDiameter) = 0;

	/*! \brief Returns the compass diameter of this daylight system.
	* Note that the compass diameter does not have an effect on the sun's precision or how it's rendered.
	* \return compass diameter.
	*/
	virtual float GetCompassDiameter() const = 0;

	/*! \brief Sets the current time associated with this daylight system.
	* \param [in] time time of day. The expected format is : (hour,minute,second)
	*/
	virtual void SetTimeOfDay(const Point3& time) = 0;

	/*! \brief Returns the current time associated with this daylight system.
	* \return current time of the daylight system. 
	* The format is : (hour,minute,second).
	*/
	virtual Point3 GetTimeOfDay() const = 0;

	/*! \brief Sets the current date associated to this daylight system.
	* \param [in] date date of the year. The expected format is : (month,day,year)
	*/
	virtual void SetDate(const Point3& date) = 0;

	/*! \brief Returns the current date associated to this daylight system.
	* \return the current date associated to this daylight system. 
	* The format is (month,day,year).
	*/
	virtual Point3 GetDate() const = 0;

	/*! \brief Set the latitude and longitude of this daylight system, note that
	* the time zone, city, azimuth and altitude are automatically calculated.
	* \param [in] latitude latitude of the location (used to determine city)
	* \param [in] longitude longitude of the location (used to determine city)
	*/
	virtual void SetLatLong(float latitude, float longitude) = 0;

	/*! \brief Returns latitude value of this daylight system.
	* \return Latitude value of this daylight system.
	*/
	virtual float GetLatitude() const = 0;


	/*! \brief Longitude value of this daylight system.
	* \return Longitude value of this daylight system.
	*/
	virtual float GetLongitude() const = 0;

	/*! \brief Set the daylight saving time parameter.
	* \param [in] isDaylightSavingTime When on, calculates daylight savings by adjusting azimuth and altitude 
	* during the summer months. 
	*/
	virtual void SetDaylightSavingTime(BOOL isDaylightSavingTime) = 0;

	/*! \brief Returns the daylight saving time parameter.
	* \return isDaylightSavingTime When on, calculates daylight savings by adjusting azimuth and altitude 
	* during the summer months. 
	*/
	virtual BOOL GetDaylightSavingTime() const = 0;
};


#endif //DAYLIGHTFACTORY_H