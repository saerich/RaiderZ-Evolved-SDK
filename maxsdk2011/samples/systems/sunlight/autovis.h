//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
/*===========================================================================*\
	FILE: autovis.h

	DESCRIPTION: Code from Autovision.

	HISTORY: Adapted by John Hutchinson 10/08/97 
			

	Copyright (c) 1996, All Rights Reserved.
 \*==========================================================================*/
#pragma once
#include "NativeInclude.h"
#include "suntypes.h"

#pragma managed(push, on)

#define TWO_PI 6.283185307
#define FTWO_PI 6.283185307f
#define EPOCH 1721424.5f
#define SECS_PER_DAY 86400

#define fixangle(x)  ((x) - 360.0 * (floor ((x) / 360.0)))
#define angle(x,y) (atan((x)/(y))+((y)<=0.0?PI:(x)<=0.0?2*PI:0.0))
#define sqr(x) ((x)*(x))
#define ER2AU (6378160.0/149600.0e6) /* A.U per earth radii */

///////////////////////////////////////////////////////////////////
// Sun locator code from AutoVision
//////////////////////////////////////////////////////////////////
inline double dtr(double dang){return TWO_PI * dang/360.0;}
inline double rtd(double rang){return 360.0 * rang/TWO_PI;}
inline float fdtr(float dang){return FTWO_PI * dang/360.0f;}
inline float frtd(float rang){return 360.0f * rang/FTWO_PI;}

double kepler(double m, double ecc);
double gregorian2julian(unsigned short month, unsigned short day, unsigned short year);
void altaz(double *az, double *alt, double *s_time, double sunra,
	double sundec, double longitude, double latitude,
	double jdate, double jtime);
void rotate(double pt[3], double matrix[3][3]);
double distance(double pt1[3], double pt2[3]);
void precession(double p[3][3], double jdate);
void ecliptic(double planet[3], double v_planet[3], double jdate);
void barycentric(double earth[3], double v_earth[3], double jdate);
void topocentric(double earth[3], double v_earth[3], double lon,
	double lat, double jdate, double jtime);
void nutation(double *ra, double *dec, double jdate);
int sunloc(void);
void julian2gregorian(double jdate, unsigned short* month, unsigned short* day, unsigned short* year);
BOOL isleap( long year);
/* Date and time of creation September 23, 1991 */

interpJulianStruct fusetime(SYSTEMTIME& tstruct);
double date_to_days(SYSTEMTIME& t);
double time_to_days(SYSTEMTIME& t);
void fracturetime(interpJulianStruct time, SYSTEMTIME& t);
void splittime( double fod, SYSTEMTIME& t);
int time_to_secs(SYSTEMTIME& t);
uTimevect secs_to_hms( int secs);
uTimevect decday_to_hms( double fod);
uTimevect addvect(uTimevect v1, uTimevect v2);

void sunLocator(double latitude, double longitude, long month, long day,
	long year, long hour, long mins, long sec, int zone,
	double *altitude, double *azimuth, double *solar_time);
	
int GetMDays(int leap, int month);


#pragma managed(pop)
