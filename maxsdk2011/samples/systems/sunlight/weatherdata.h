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
/**********************************************************************
 *<
	FILE: weatherdata.h

	DESCRIPTION:	Weather Data UI and various strtucs

	CREATED BY: Michael Zyracki

	HISTORY: Created Nov 2007

 *>	Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/


#ifndef _WEATHERDATA_H
#define _WEATHERDATA_H

#pragma once
#pragma managed(push, off)
#include "NativeInclude.h"
#include <list>
#include <vector>
#pragma managed(pop)

#pragma managed(push, on)

#using <WeatherData.dll>
using namespace WeatherData;

/*
	Following are cache values coming out from the WeatheData reader. We do this for multiple reasons.
	1)  The WeatherFile reader expects a weather file as input and we need to make sure that we correctly close the file
	handle so we need to allocate it on the stack and then deallocate it.  The current interface doesn't allow us to
	open and close files easily
	2) We don't want all of the information that's in the weather. Saving subsections will save memory.
	3) We may need special data structures, instead of just a single list to point at each period and to have a special
	iterator.
*/
class Int3
{
public:
	Int3(): x(0),y(0),z(0){};
	Int3(int a, int b, int c): x(a),y(b),z(c){};
	int x,y,z;
};

struct WeatherDataPeriod
{
	Int3 mStartMDY; //year will be blank usually
	Int3 mEndMDY; //year will be blank usually
};
struct WeatherHeaderInfo
{
	TSTR mCity;
	TSTR mRegion;
	TSTR mCountry;
	TSTR mSource;
	int mWMO;
	float mLat;
	float mLon;
	float mTimeZone;
	float mElevationMeters;
	bool  mLeapYearObserved;
	Tab<WeatherDataPeriod> mWeatherDataPeriods;
	float mNumRecordsPerHour; //will be -1 if they are all different
	Int3 mDaylightSavingsStart;
	Int3 mDaylightSavingsEnd;
};

struct MDY
{
	int month;
	int day;
	int year;
};


struct DaylightWeatherData
{
	int mYear;
	int mMonth;
	int mDay;
	int mHour;
	int mMinute;
	TSTR mDataUncertainty;
	float mDryBulbTemperature; //0,1
	float mDewPointTemperature;//2,3
	float mRelativeHumidity; //4,5
	float mAtmosphericStationPressure; //6,7
	float mExtraterrestrialHorizontalRadiation;
	float mExtraterrestrialDirectNormalRadiation;
	float mHorizontalInfraredRadiationFromSky; //8.9
	float mGlobalHorizontalRadiation;  //10,11
	float mDirectNormalRadiation;  //12,13
	float mDiffuseHorizontalRadiation;  //14,15
	float mGlobalHorizontalIlluminance;  //16,17
	float mDirectNormalIlluminance;     //18,19
	float mDiffuseHorizontalIlluminance; //20,21
	float mZenithLuminance;   //22,23

};

typedef std::list<DaylightWeatherData> DaylightWeatherDataEntries;
typedef std::vector<DaylightWeatherData> DaylightWeatherDataVector;


//the values of these paramters may effect the cache of the filter.
struct WeatherCacheFilter
{
	//this enum and string id table need to be kept in sync. eEnd is always the last one.
	enum FramePer{ePeriod =0,eDay,eWeek,eMonth,eSeason,eEnd};
	BOOL mSkipHours;
	int mSkipHoursStart;
	int mSkipHoursEnd;
	BOOL mSkipWeekends;
	FramePer mFramePer;
	WeatherCacheFilter() :mSkipHours(FALSE),mSkipHoursStart(18), mSkipHoursEnd(8), mSkipWeekends(FALSE),mFramePer(ePeriod){}
};

//This struct contains all of the information that can get passed into and out of the weather file dialog.
//This gets filled out as input into the dialog and then filled out on exit as the dialog operates.
struct WeatherUIData
{
	

	TSTR mFileName; //this is the full pathed file name, as a string.
	BOOL mAnimated; //if TRUE then anmated with start and end, otherwise single date.
	Int3 mSpecificTimeMDY;
	Int3 mSpecificTimeHMS;
	Int3 mAnimStartTimeMDY;
	Int3 mAnimStartTimeHMS;
	Int3 mAnimEndTimeMDY;
	Int3 mAnimEndTimeHMS;
	WeatherCacheFilter mFilter;
	bool mResetTimes;
	WeatherUIData(): mAnimated(FALSE),mSpecificTimeMDY(1,1,2007),mSpecificTimeHMS(12,0,0),
		mAnimStartTimeMDY(1,1,2007),mAnimStartTimeHMS(12,0,0),
		mAnimEndTimeMDY(12,31,2007),mAnimEndTimeHMS(12,0,0), mResetTimes(true)
		{};
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

};


class WeatherFileCache
{
public:
	WeatherFileCache():mNumEntries(0),mSubtractHour(true){Clear();}

	//virtual functions
	virtual ~WeatherFileCache(){Clear();}
	virtual void Clear();//also deltes

	//non virtual functions 

	WeatherHeaderInfo ReadHeader(const TCHAR* filename);
	int ReadWeatherData(const TCHAR* filename); //returns the number of entries read

	TSTR mFileName;
	WeatherHeaderInfo mWeatherHeaderInfo;
	Tab<DaylightWeatherDataEntries*> mDataPeriods;
	int mNumEntries;


	BOOL WeatherFileCache::ValidPerFrame(DaylightWeatherDataEntries::iterator iter,Int3 &MDY,
									 Int3 &HMS, WeatherCacheFilter::FramePer framePer);

	//used mainly by UI
	int GetRealNumEntries(WeatherUIData &data, BOOL useRangeAndPerFrame);
	//key function that finds iterator that points to that passed in time, based upon 
	//the passed in Month Day Year and Hour Minute Secs.  Note that 
	void GetValidIter(DaylightWeatherDataEntries::iterator &iter, int &currentPeriod, Int3 &MDY, Int3  &HMS,
		WeatherCacheFilter &filter,int overrideDir =0); //0 means don't override, dir is 0 match, 1 pos, -1, negative.

	void CheckIter(DaylightWeatherDataEntries::iterator &iter,int &currentPeriod, int dir,WeatherCacheFilter &filter);
	BOOL ValidIter(DaylightWeatherDataEntries::iterator iter,WeatherCacheFilter &filter);
	
	int FindDayOfWeek(int year, int month, int day);

	//this gets the 'real' entry based upon the skip flags and such.
	DaylightWeatherDataEntries::iterator GetFilteredNthEntry(int nth,WeatherCacheFilter &filter);
	int FindNthEntry(Int3 &MDY, Int3 &HMS,WeatherCacheFilter &filter);
	//if next >current returns 1, else -1 if next<current else 0 if the same
	int DateCompare(Int3 &currentMDY, Int3 &currentHMS, Int3 &nextMDY, Int3 &nextHMS);

protected:
	WeatherHeaderInfo ConvertHeaderToStruct(WeatherHeader^ header);
	DaylightWeatherData ConvertEntryToStruct(WeatherEntry^ entry, bool firstConversion);

	bool mSubtractHour; //some files start at hour 1, some start at hour 0. We expect hour 0 to be the first hour.
					   //so we check this.

};


class WeatherFileCacheFiltered : public WeatherFileCache
{
public:

	WeatherFileCacheFiltered(): WeatherFileCache(){};
	~WeatherFileCacheFiltered(){Clear();}
	void Clear();

	DaylightWeatherDataVector mDataVector;


	//filter data to to the vector
	int FilterEntries(WeatherUIData &data);
	//returns the nth based upon the time value.
	DaylightWeatherData & GetNthEntry(TimeValue t);


};


#pragma managed(pop)
#endif

