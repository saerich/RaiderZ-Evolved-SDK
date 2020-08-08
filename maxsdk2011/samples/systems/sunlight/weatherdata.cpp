
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
	FILE: weatherdata.cpp

	DESCRIPTION:	Weather Data UI

	CREATED BY: Michael Zyracki

	HISTORY: Created Nov 2007

 *>	Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/

#pragma unmanaged
#include "NativeInclude.h"
#pragma managed

#include "weatherdata.h"
#include "sunlight.h"
#include "ManagedServices\StringConverter.h"

#using <WeatherData.dll>
using namespace WeatherData;
using namespace ManagedServices;



// Read just the header from a weather data file.
WeatherHeaderInfo WeatherFileCache::ReadHeader(const TCHAR* filename)
{
	// uses the stack-allocation syntax for predictable resource clean-up when
	// the variable goes out of scope
	// This is necessary to release the file handle.  If we kept the reader 
	// around as gcnew-allocated object, it will keep the file open until the 
	// garbage collector runs.
	// This is equivalent to the "using" syntactic sugar in C#.
	EnergyPlusWeatherFileReader reader(gcnew System::String(filename));

	WeatherHeader^ header = reader.ReadWeatherHeader();
	return ConvertHeaderToStruct(header);
}

// converts a managed WeatherHeader to a WeatherHeaderInfo struct
WeatherHeaderInfo WeatherFileCache::ConvertHeaderToStruct(WeatherHeader^ header)
{
	StringConverter<TCHAR> converter;

	WeatherHeaderInfo headerStruct = {""};
	headerStruct.mCity = converter.Convert(header->Location->City);
	headerStruct.mRegion = converter.Convert(header->Location->Region);
	headerStruct.mCountry = converter.Convert(header->Location->Country);
	headerStruct.mSource = converter.Convert(header->Location->Source);
	headerStruct.mWMO = header->Location->WMO;
	headerStruct.mLat = header->Location->Latitude;
	headerStruct.mLon = header->Location->Longitude;
	headerStruct.mTimeZone = header->Location->TimeZone;
	headerStruct.mElevationMeters = header->Location->ElevationMeters;
	headerStruct.mLeapYearObserved = header->HolidaysAndDaylightSavings->LeapYearObserved;
	headerStruct.mDaylightSavingsStart.x = header->HolidaysAndDaylightSavings->DaylightSavings->StartDay->Month;
	headerStruct.mDaylightSavingsStart.y = header->HolidaysAndDaylightSavings->DaylightSavings->StartDay->Day;
	headerStruct.mDaylightSavingsEnd.x = header->HolidaysAndDaylightSavings->DaylightSavings->EndDay->Month;
	headerStruct.mDaylightSavingsEnd.y = header->HolidaysAndDaylightSavings->DaylightSavings->EndDay->Day;

	// The number of records per hour is accessible in each data period as 
	// header->DataPeriods[i]->RecordPerHourCount.  Each data period can have its
	// own value for this.
	int count =0;
	WeatherDataPeriod period;
	for each(DataPeriodDescriptor^ entry in header->DataPeriods)
	{
		period.mStartMDY.x = entry->PeriodStartDate->Month;
		period.mStartMDY.y = entry->PeriodStartDate->Day;
		period.mStartMDY.z = -1;
		period.mEndMDY.x = entry->PeriodEndDate->Month;
		period.mEndMDY.y = entry->PeriodEndDate->Day;
		period.mEndMDY.z = -1;
		headerStruct.mWeatherDataPeriods.Append(1,&period);
		if(count==0)
			headerStruct.mNumRecordsPerHour = 	entry->RecordPerHourCount;
		else
		{
			if(headerStruct.mNumRecordsPerHour!=entry->RecordPerHourCount)
				headerStruct.mNumRecordsPerHour = -1.0f;
		}
	}


	return headerStruct;
}

// Read an entire data file and fill out all the structs
int WeatherFileCache::ReadWeatherData(const TCHAR* filename)
{
	Clear();
	mFileName = filename;
	EnergyPlusWeatherFileReader reader(gcnew System::String(filename));
	try
	{
		WeatherDataSet^ weatherData = reader.ReadWeatherDataSet();
		mWeatherHeaderInfo = ConvertHeaderToStruct(weatherData->Header);
		mNumEntries =0;

		int numPeriods = mWeatherHeaderInfo.mWeatherDataPeriods.Count();
		if(numPeriods==0)
			return 0;

		//get current info
		int currentPeriod=0; //which period we are in.
		int endDay = mWeatherHeaderInfo.mWeatherDataPeriods[currentPeriod].mEndMDY.y;
		int endMonth = mWeatherHeaderInfo.mWeatherDataPeriods[currentPeriod].mEndMDY.x;
		DaylightWeatherDataEntries *entries = new DaylightWeatherDataEntries;
		mDataPeriods.Append(1,&entries);

		DaylightWeatherData data;

		bool firstIsTrue = true;
		for each(WeatherEntry^ entry in weatherData->Entries)
		{
			data = ConvertEntryToStruct(entry,firstIsTrue);
			firstIsTrue = false;
			if(numPeriods>1&& ((data.mMonth>endMonth)||(data.mMonth==endMonth&&data.mDay>endDay))
				&&( (currentPeriod+1)<numPeriods))  //make sure we don't go over the number of periods for some reason
			{
				//okay new data period
				entries = new DaylightWeatherDataEntries;
				mDataPeriods.Append(1,&entries);
				++currentPeriod;
				endDay = mWeatherHeaderInfo.mWeatherDataPeriods[currentPeriod].mEndMDY.y;
				endMonth = mWeatherHeaderInfo.mWeatherDataPeriods[currentPeriod].mEndMDY.x;
			}
			entries->push_back(data);
			++mNumEntries;
		}
	}
	catch(...) //got an error
	{
		Clear(); //delete anything that was loaded
		mNumEntries = 0;
		TSTR title(MaxSDK::GetResourceString(IDS_WEATHER_FILE));
		TSTR message(MaxSDK::GetResourceString(IDS_WEATHER_FILE_LOAD_ERROR));
		if (GetCOREInterface()->IsNetServer() || GetCOREInterface()->GetQuietMode()) //don't show dialog
		{
			LogSys *ls = GetCOREInterface()->Log();
    		ls->LogEntry(SYSLOG_ERROR, DISPLAY_DIALOG, title.data(), message.data());
		}
		else
		{
			MessageBox(GetCOREInterface()->GetMAXHWnd(),
				message.data(),
				title.data(), 
				MB_OK |  MB_ICONWARNING | MB_APPLMODAL);
					
		}
	}


	return mNumEntries;
}


void WeatherFileCache::Clear()
{	mWeatherHeaderInfo.mCity = TSTR("");
	mWeatherHeaderInfo.mCountry = TSTR("");
	mWeatherHeaderInfo.mWeatherDataPeriods.ZeroCount();
	mFileName = TSTR("");
	for(int i=0;i<mDataPeriods.Count();++i)
	{
		if(mDataPeriods[i])
		{
			mDataPeriods[i]->clear();
			delete mDataPeriods[i];
		}
	}
	mDataPeriods.ZeroCount();
}



void WeatherFileCacheFiltered::Clear()
{
	WeatherFileCache::Clear();
	mDataVector.clear();
}


DaylightWeatherData WeatherFileCache::ConvertEntryToStruct(WeatherEntry^ entry, bool firstEntry)
{
	DaylightWeatherData dataStruct = {0};
	dataStruct.mYear = entry->Year;

	if(dataStruct.mYear==-1)
		dataStruct.mYear = 2007; //just set it to this year.
	dataStruct.mMonth = entry->Month;
	dataStruct.mDay = entry->Day;
	if(firstEntry)
	{	
		if(entry->Hour==0) //only seen this with weather files from CNRC.
			mSubtractHour = false;
		else //assume we subract that's normal.
			mSubtractHour = true;
	}
	if(mSubtractHour)
		dataStruct.mHour = entry->Hour-1;
	else
		dataStruct.mHour = entry->Hour;
	dataStruct.mMinute = entry->Minute;
	if(dataStruct.mMinute>=60)
		dataStruct.mMinute -= 60;
	StringConverter<TCHAR> converter;
	dataStruct.mDataUncertainty = 
		converter.Convert( entry->DataSourceAndUncertaintyFlags->ToString() );

	dataStruct.mDryBulbTemperature = entry->DryBulbTemperature;
	dataStruct.mDewPointTemperature = entry->DewPointTemperature;
	dataStruct.mRelativeHumidity = entry->RelativeHumidity;
	dataStruct.mAtmosphericStationPressure = entry->AtmosphericStationPressure;
	dataStruct.mExtraterrestrialHorizontalRadiation = entry->ExtraterrestrialHorizontalRadiation;
	dataStruct.mExtraterrestrialDirectNormalRadiation = entry->ExtraterrestrialDirectNormalRadiation;
	dataStruct.mHorizontalInfraredRadiationFromSky = entry->HorizontalInfraredRadiationFromSky;
	dataStruct.mGlobalHorizontalRadiation = entry->GlobalHorizontalRadiation;
	dataStruct.mDirectNormalRadiation = entry->DirectNormalRadiation;
	dataStruct.mDiffuseHorizontalRadiation = entry->DiffuseHorizontalRadiation;
	dataStruct.mGlobalHorizontalIlluminance = entry->GlobalHorizontalIlluminance;
	dataStruct.mDirectNormalIlluminance = entry->DirectNormalIlluminance;
	dataStruct.mDiffuseHorizontalIlluminance = entry->DiffuseHorizontalIlluminance;
	dataStruct.mZenithLuminance = entry->ZenithLuminance;

	return dataStruct;
}

int WeatherFileCache::FindDayOfWeek(int year, int month, int day)
{
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
    timeinfo = localtime ( &rawtime );
	timeinfo->tm_year = year - 1900;
    timeinfo->tm_mon = month - 1;
    timeinfo->tm_mday = day;
    mktime( timeinfo);
	return timeinfo->tm_wday;
}

int WeatherFileCacheFiltered::FilterEntries(WeatherUIData &data)
{

	Int3 currentMDY, currentHMS;
	mDataVector.clear();
	if(data.mAnimated==FALSE)
	{
		mNumEntries =0;
		for(int i =0;i<mDataPeriods.Count();++i)
		{
			DaylightWeatherDataEntries::iterator iter = mDataPeriods[i]->begin();
			while(iter!=mDataPeriods[i]->end())
			{
				currentMDY.x = iter->mMonth; currentMDY.y = iter->mDay;  currentMDY.z = iter->mYear;
				currentHMS.x = iter->mHour;  currentHMS.y = iter->mMinute; currentHMS.z = 0;
				if(DateCompare(currentMDY,currentHMS,data.mSpecificTimeMDY,data.mSpecificTimeHMS )==0)
				{
					mDataVector.push_back(*iter);
					++mNumEntries;
					++iter;
				}
				else
				{
					iter = mDataPeriods[i]->erase(iter);
				}
			}
		}
		return mNumEntries;
	}
	else
	{
		mNumEntries = 0;
		for(int i =0;i<mDataPeriods.Count();++i)
		{
			DaylightWeatherDataEntries::iterator iter = mDataPeriods[i]->begin();
			while(iter!=mDataPeriods[i]->end())
			{
				currentMDY.x = iter->mMonth; currentMDY.y = iter->mDay;  currentMDY.z = iter->mYear;
				currentHMS.x = iter->mHour;  currentHMS.y = iter->mMinute; currentHMS.z = 0;
				//compare with start and end dates
				if(DateCompare(currentMDY,currentHMS,data.mAnimStartTimeMDY,data.mAnimStartTimeHMS)<=0
					&&DateCompare(currentMDY,currentHMS,data.mAnimEndTimeMDY,data.mAnimEndTimeHMS)>=0
					&&ValidIter(iter,data.mFilter)
					&&ValidPerFrame(iter, data.mAnimStartTimeMDY,data.mAnimStartTimeHMS,data.mFilter.mFramePer))

				{
					mDataVector.push_back(*iter);
					++mNumEntries;
					++iter;

				}
				else
				{
					iter = mDataPeriods[i]->erase(iter);
				}
			}
		}
		return mNumEntries;
	}
}

DaylightWeatherData& WeatherFileCacheFiltered::GetNthEntry(TimeValue t)
{
	//first we change t into a frame, which is then an index into our array.
	TimeValue start  = GetCOREInterface10()->GetAutoKeyDefaultKeyTime();
	int realCount = 0;

	int index=0; //if t<=start
	if(t>start)
	{
		index = (t-start)/GetTicksPerFrame();
		if(index>=mDataVector.size())
		{
			index = (int)mDataVector.size()-1;
		}
	}
	
	return mDataVector[index];
}

BOOL WeatherFileCache::ValidPerFrame(DaylightWeatherDataEntries::iterator iter,Int3 &MDY,
									 Int3 &HMS, WeatherCacheFilter::FramePer framePer)
{
	//use march 20th, june 21th, sept 22, dec 22
	if(framePer == WeatherCacheFilter::eSeason)
	{
		//first the HMS need to match 
		if(iter->mHour ==HMS.x && iter->mMinute==HMS.y)
		{
			if(iter->mMonth==3&&iter->mDay==20)
				return TRUE;
			else if(iter->mMonth==6&&iter->mDay==21)
				return TRUE;
			else if(iter->mMonth==9&&iter->mDay==22)
				return TRUE;
			else if(iter->mMonth==12&&iter->mDay==22)
				return TRUE;
		}
		return FALSE;
	}
	else if(framePer == WeatherCacheFilter::eDay)
	{
		//just need the HMS need to match 
		if(iter->mHour ==HMS.x && iter->mMinute==HMS.y)
		{
			return TRUE;
		}
		return FALSE;
	}
	else if(framePer == WeatherCacheFilter::eWeek)
	{
		//first the HMS need to match 
		if(iter->mHour ==HMS.x && iter->mMinute==HMS.y)
		{
			int day = FindDayOfWeek(MDY.z, MDY.x,MDY.y);
			//then just the day of week.
			int iterday = FindDayOfWeek(iter->mYear,iter->mMonth, iter->mDay);
			if(day==iterday)
				return TRUE;
			else
				return FALSE;
		}
		return FALSE;
	}
	else if(framePer == WeatherCacheFilter::eMonth)
	{
		//need HMS and the day to match
		if(iter->mHour ==HMS.x && iter->mMinute==HMS.y &&
			iter->mDay == MDY.y)
		{
			return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}

int WeatherFileCache::GetRealNumEntries(WeatherUIData &data,BOOL useRangeAndPerFrame)
{
	if(useRangeAndPerFrame==FALSE)
	{
		if(data.mFilter.mSkipWeekends==FALSE&&data.mFilter.mSkipHours==FALSE)
			return mNumEntries;
		int realCount = 0;
		for(int i =0;i<mDataPeriods.Count();++i)
		{
			DaylightWeatherDataEntries::iterator iter = mDataPeriods[i]->begin();
			while(iter!=mDataPeriods[i]->end())
			{
				if(ValidIter(iter,data.mFilter))
				{
					//check and see if it's valid for the use Per
					++realCount;
				}
				++iter;
			}
		}
		return realCount;
	}
	else
	{
		if(data.mAnimated==FALSE)
		{
			if(mNumEntries>0)
				return 1;
			else
				return 0;
		}
		//now find animated..
		Int3 currentMDY, currentHMS;
		int realCount = 0;
		for(int i =0;i<mDataPeriods.Count();++i)
		{
			DaylightWeatherDataEntries::iterator iter = mDataPeriods[i]->begin();
			while(iter!=mDataPeriods[i]->end())
			{
				currentMDY.x = iter->mMonth; currentMDY.y = iter->mDay;  currentMDY.z = iter->mYear;
				currentHMS.x = iter->mHour;  currentHMS.y = iter->mMinute; currentHMS.z = 0;
				//compare with start and end dates
				if(DateCompare(currentMDY,currentHMS,data.mAnimStartTimeMDY,data.mAnimStartTimeHMS)<=0
					&&DateCompare(currentMDY,currentHMS,data.mAnimEndTimeMDY,data.mAnimEndTimeHMS)>=0
					&&ValidIter(iter,data.mFilter))
				{
					//check and see if it's valid for the perFrame check
					if(data.mFilter.mFramePer== WeatherCacheFilter::ePeriod||ValidPerFrame(iter, data.mAnimStartTimeMDY,data.mAnimStartTimeHMS,data.mFilter.mFramePer))
						++realCount;
				}
				++iter;
			}
		}
		return realCount;
	}
}

//this gets the 'real' entry based upon the skip flags and such.
DaylightWeatherDataEntries::iterator WeatherFileCache::GetFilteredNthEntry(int nth,WeatherCacheFilter &filter)
{
	int realCount = 0;
	DaylightWeatherDataEntries::iterator iter;
	for(int i =0;i<mDataPeriods.Count();++i)
	{
		iter = mDataPeriods[i]->begin();
		while(iter!=mDataPeriods[i]->end())
		{
			if((filter.mSkipWeekends==FALSE&&filter.mSkipHours==FALSE)||ValidIter(iter,filter))
				++realCount;
			if((realCount-1)==nth)
				return iter;
			++iter;
		}
	}
	iter = mDataPeriods[0]->end(); //that's the test
	return iter;
}

int WeatherFileCache::FindNthEntry(Int3 &MDY, Int3 &HMS,WeatherCacheFilter &filter)
{

	int realCount = 0;
	DaylightWeatherDataEntries::iterator iter;
	for(int i =0;i<mDataPeriods.Count();++i)
	{
		iter = mDataPeriods[i]->begin();
		while(iter!=mDataPeriods[i]->end())
		{
			if((filter.mSkipWeekends==FALSE&&filter.mSkipHours==FALSE)||ValidIter(iter,filter))
			{
				if((MDY.x==iter->mMonth)&&(MDY.y == iter->mDay) &&(HMS.x == iter->mHour)&& (HMS.y==iter->mMinute))
					return realCount;
				++realCount;
			}
			++iter;
		}
	}
	return realCount;
}


BOOL WeatherFileCache::ValidIter(DaylightWeatherDataEntries::iterator iter,WeatherCacheFilter &filter)
{

	if(filter.mSkipWeekends)
	{
		int dayOfWeek = FindDayOfWeek(iter->mYear,iter->mMonth,iter->mDay);
		if(dayOfWeek==0||dayOfWeek==6)
			return FALSE;
	}
	if(filter.mSkipHours)
	{
		//first check and see if range.end<range.start if so we only need to check each(most common case)
		if(filter.mSkipHoursEnd <filter.mSkipHoursStart)
		{
			if(iter->mHour>filter.mSkipHoursStart|| iter->mHour<filter.mSkipHoursEnd)
				return FALSE;
		}
		else
		{
			if(iter->mHour>filter.mSkipHoursStart&&iter->mHour<filter.mSkipHoursEnd)
				return FALSE;
		}
	}
	return TRUE;
}



//if next >current returns 1, else -1 if next<current else 0 if the same
int WeatherFileCache::DateCompare(Int3 &currentMDY, Int3 &currentHMS, Int3 &nextMDY, Int3 &nextHMS)
{
	if(nextMDY.x>currentMDY.x)//month
		return 1;
	else if(nextMDY.x <currentMDY.x)
		return -1;
	else
	{
		if(nextMDY.y>currentMDY.y) //day
			return 1;
		else if(nextMDY.y<currentMDY.y)
			return -1;
		else
		{
			if(nextHMS.x>currentHMS.x)  //hour
				return 1;
			else if(nextHMS.x<currentHMS.x)
				return -1;
			else
			{
				if(nextHMS.y>currentHMS.y)  //minute
					return 1;
				else if(nextHMS.y<currentHMS.y)
					return -1;
			}
		}
	}
	return 0; //the same!
}

void WeatherFileCache::CheckIter(DaylightWeatherDataEntries::iterator &iter, int &currentPeriod, int dir, WeatherCacheFilter &filter)
{

	//need to find a valid iter.
	if(dir!=1&&dir!=-1)
		dir =1;
	int count =0;
	while(ValidIter(iter,filter)==FALSE)
	{
		if(dir==1)
		{
			//check the iter and the direction
			++iter;
			if(iter == mDataPeriods[currentPeriod]->end())
			{
				if(currentPeriod==mDataPeriods.Count()-1)
				{
					//go to the first one!
					currentPeriod = 0;
					iter = mDataPeriods[currentPeriod]->begin();
				}
				else
				{
					++currentPeriod;
					iter = mDataPeriods[currentPeriod]->begin();
				}
			}
		}
		else if(dir==-1)
		{
			if(iter == mDataPeriods[currentPeriod]->begin())
			{
				if(currentPeriod==0)
				{
					//go to the last one!
					currentPeriod = mDataPeriods.Count()-1;
					iter = mDataPeriods[currentPeriod]->end();
					--iter;
				}
				else
				{
					--currentPeriod;
					iter = mDataPeriods[currentPeriod]->end();
					--iter;
				}
			}
			else
				--iter;
		}
		if(count++ >= this->mNumEntries) //safety check
			return;
	}
}

//key function that finds iterator that points to that passed in time, based upon 
//the passed in Month Day Year and Hour Minute Secs. 
void WeatherFileCache::GetValidIter(DaylightWeatherDataEntries::iterator &iter, int &currentPeriod, Int3 &MDY, Int3  &HMS,
				  WeatherCacheFilter &filter,int overrideDir)
{
	if(mDataPeriods.Count()<0)
		return;  //do nothing
	//make sure period and iter is valid
	if(currentPeriod<0||currentPeriod>mDataPeriods.Count())
	{
		currentPeriod = 0;
		iter = mDataPeriods[currentPeriod]->begin();
		if(iter==mDataPeriods[currentPeriod]->end())
			return;
	}
	else
	{
		//make sure the iter is valid
		if(iter==mDataPeriods[currentPeriod]->end())
		{
			iter = mDataPeriods[currentPeriod]->begin();
			if(iter==mDataPeriods[currentPeriod]->end())
				return;
		}
	}

	//now find the iter! and set everything up.
	Int3 currentMDY, currentHMS;
	bool notFound = true;
	int dir; //direction we should go in...
	int oldDir = -2;
	int count =0; //safety check
	while(notFound)
	{
		currentMDY.x = iter->mMonth; currentMDY.y = iter->mDay;  currentMDY.z = iter->mYear;
		currentHMS.x = iter->mHour;  currentHMS.y = iter->mMinute; currentHMS.z = 0;
	
		dir = DateCompare(currentMDY,currentHMS,MDY,HMS);
		if(dir>0)
		{
			if(oldDir!=-2&&oldDir!=dir)
			{
				//we switched direction.. end!
				notFound = false;
				CheckIter(iter,currentPeriod,dir,filter);
				MDY.x = iter->mMonth; MDY.y = iter->mDay; MDY.z = iter->mYear;
				HMS.x = iter->mHour;  HMS.y = iter->mMinute;  HMS.z = 0;
			}
			oldDir = dir;
			++iter;
			//check the iter and the direction
			if(iter == mDataPeriods[currentPeriod]->end())
			{
				if(currentPeriod==mDataPeriods.Count()-1)
				{
					--iter; //go back one for now.
					//at end so reverse when we check
					CheckIter(iter,currentPeriod,-1,filter);
					notFound = false;
					MDY.x = iter->mMonth; MDY.y = iter->mDay; MDY.z = iter->mYear;
					HMS.x = iter->mHour;  HMS.y = iter->mMinute;  HMS.z = 0;
				}
				else
				{
					++currentPeriod;
					iter = mDataPeriods[currentPeriod]->begin();
				}
			}
		}
		else if(dir<0)
		{
			if(oldDir!=-2&&oldDir!=dir)
			{
				//we switched direction.. end!
				notFound = false;
				CheckIter(iter,currentPeriod,dir,filter);
				MDY.x = iter->mMonth; MDY.y = iter->mDay; MDY.z = iter->mYear;
				HMS.x = iter->mHour;  HMS.y = iter->mMinute;  HMS.z = 0;
			}
			oldDir = dir;
			//can't do a -- beyond the beginning so need a check her..
			if(iter == mDataPeriods[currentPeriod]->begin())
			{
				--currentPeriod;
				if(currentPeriod<0)
				{
					currentPeriod =0;
					notFound = false;
					//check up from the start. so reverse the direction here.
					CheckIter(iter,currentPeriod,1,filter);
					MDY.x = iter->mMonth; MDY.y = iter->mDay; MDY.z = iter->mYear;
					HMS.x = iter->mHour;  HMS.y = iter->mMinute;  HMS.z = 0;
				}
				else
				{
					iter = mDataPeriods[currentPeriod]->end();
					//go back one to the last one.
					--iter;
				}
			}
			else
				--iter;
		}
		else
		{
			notFound = false;

			if(overrideDir!=0)
				oldDir = overrideDir;
			CheckIter(iter,currentPeriod,oldDir,filter);
			MDY.x = iter->mMonth; MDY.y = iter->mDay; MDY.z = iter->mYear;
			HMS.x = iter->mHour;  HMS.y = iter->mMinute;  HMS.z = 0;
		}
		if(count++>= this->mNumEntries)
			return;

 	}
}

#define SKIPHOURS_CHUNK				0x2100
#define SKIPHOURSSTART_CHUNK		0x2110
#define SKIPHOURSEND_CHUNK			0x2120
#define SKIPWEEKENDS_CHUNK			0x2130
#define FRAMEPER_CHUNK				0x2140
#define ANIMATED_CHUNK				0x2200
#define SPECIFICTIMEMDY_CHUNK		0x2210
#define SPECIFICTIMEHMS_CHUNK		0x2220
#define ANIMSTARTTIMEMDY_CHUNK		0x2230
#define ANIMSTARTTIMEHMS_CHUNK		0x2240
#define ANIMENDTIMEMDY_CHUNK 		0x2250
#define ANIMENDTIMEHMS_CHUNK		0x2260



IOResult WeatherUIData::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case  SKIPHOURS_CHUNK:
				res = iload->Read(&mFilter.mSkipHours,sizeof(mFilter.mSkipHours), &nb);
				break;
			case SKIPHOURSSTART_CHUNK:
				res = iload->Read(&mFilter.mSkipHoursStart, sizeof(mFilter.mSkipHoursStart), &nb);
				break;
			case SKIPHOURSEND_CHUNK:
				res = iload->Read(&mFilter.mSkipHoursEnd, sizeof(mFilter.mSkipHoursEnd), &nb);
				break;
			case SKIPWEEKENDS_CHUNK:
				res = iload->Read(&mFilter.mSkipWeekends, sizeof(mFilter.mSkipWeekends), &nb);
				break;
			case FRAMEPER_CHUNK:
				{
					int frameper;
					res = iload->Read(&frameper, sizeof(int), &nb);
					mFilter.mFramePer = static_cast<WeatherCacheFilter::FramePer>(frameper);
					break;
				}
			case ANIMATED_CHUNK:
				res = iload->Read(&mAnimated, sizeof(mAnimated), &nb);
				break;
			case SPECIFICTIMEMDY_CHUNK:
				res = iload->Read(&mSpecificTimeMDY.x, sizeof(int), &nb);
				res = iload->Read(&mSpecificTimeMDY.y, sizeof(int), &nb);
				res = iload->Read(&mSpecificTimeMDY.z, sizeof(int), &nb);
				break;
			case SPECIFICTIMEHMS_CHUNK:
				res = iload->Read(&mSpecificTimeHMS.x, sizeof(int), &nb);
				res = iload->Read(&mSpecificTimeHMS.y, sizeof(int), &nb);
				res = iload->Read(&mSpecificTimeHMS.z, sizeof(int), &nb);
				break;
			case ANIMSTARTTIMEMDY_CHUNK:
				res = iload->Read(&mAnimStartTimeMDY.x, sizeof(int), &nb);
				res = iload->Read(&mAnimStartTimeMDY.y, sizeof(int), &nb);
				res = iload->Read(&mAnimStartTimeMDY.z, sizeof(int), &nb);
				break;
			case ANIMSTARTTIMEHMS_CHUNK:
				res = iload->Read(&mAnimStartTimeHMS.x, sizeof(int), &nb);
				res = iload->Read(&mAnimStartTimeHMS.y, sizeof(int), &nb);
				res = iload->Read(&mAnimStartTimeHMS.z, sizeof(int), &nb);
				break;
			case ANIMENDTIMEMDY_CHUNK:
				res = iload->Read(&mAnimEndTimeMDY.x, sizeof(int), &nb);
				res = iload->Read(&mAnimEndTimeMDY.y, sizeof(int), &nb);
				res = iload->Read(&mAnimEndTimeMDY.z, sizeof(int), &nb);
				break;
			case ANIMENDTIMEHMS_CHUNK:
				res = iload->Read(&mAnimEndTimeHMS.x, sizeof(int), &nb);
				res = iload->Read(&mAnimEndTimeHMS.y, sizeof(int), &nb);
				res = iload->Read(&mAnimEndTimeHMS.z, sizeof(int), &nb);
				break;

			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	
	
	return IO_OK;
}

IOResult WeatherUIData::Save(ISave *isave)
{
	ULONG nb;
	isave->BeginChunk(SKIPHOURS_CHUNK);
	isave->Write(&mFilter.mSkipHours,sizeof(mFilter.mSkipHours), &nb);
	isave->EndChunk();

	isave->BeginChunk(SKIPHOURSSTART_CHUNK);
	isave->Write(&mFilter.mSkipHoursStart, sizeof(mFilter.mSkipHoursStart), &nb);
	isave->EndChunk();

	isave->BeginChunk(SKIPHOURSEND_CHUNK);
	isave->Write(&mFilter.mSkipHoursEnd, sizeof(mFilter.mSkipHoursEnd), &nb);
	isave->EndChunk();

	isave->BeginChunk(SKIPWEEKENDS_CHUNK);
	isave->Write(&mFilter.mSkipWeekends, sizeof(mFilter.mSkipWeekends), &nb);
	isave->EndChunk();

	isave->BeginChunk(FRAMEPER_CHUNK);
	int frameper = static_cast<int>(mFilter.mFramePer);
	isave->Write(&frameper, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(ANIMATED_CHUNK);
	isave->Write(&mAnimated,sizeof(mAnimated), &nb);
	isave->EndChunk();

	isave->BeginChunk(SPECIFICTIMEMDY_CHUNK);
	isave->Write(&mSpecificTimeMDY.x, sizeof(mSpecificTimeMDY.x), &nb);
	isave->Write(&mSpecificTimeMDY.y, sizeof(mSpecificTimeMDY.y), &nb);
	isave->Write(&mSpecificTimeMDY.z, sizeof(mSpecificTimeMDY.z), &nb);
	isave->EndChunk();

	isave->BeginChunk(SPECIFICTIMEHMS_CHUNK);
	isave->Write(&mSpecificTimeHMS.x, sizeof(mSpecificTimeHMS.x), &nb);
	isave->Write(&mSpecificTimeHMS.y, sizeof(mSpecificTimeHMS.y), &nb);
	isave->Write(&mSpecificTimeHMS.z, sizeof(mSpecificTimeHMS.z), &nb);
	isave->EndChunk();

	isave->BeginChunk(ANIMSTARTTIMEMDY_CHUNK);
	isave->Write(&mAnimStartTimeMDY.x, sizeof(mAnimStartTimeMDY.x), &nb);
	isave->Write(&mAnimStartTimeMDY.y, sizeof(mAnimStartTimeMDY.y), &nb);
	isave->Write(&mAnimStartTimeMDY.z, sizeof(mAnimStartTimeMDY.z), &nb);
	isave->EndChunk();

	isave->BeginChunk(ANIMSTARTTIMEHMS_CHUNK);
	isave->Write(&mAnimStartTimeHMS.x, sizeof(mAnimStartTimeHMS.x), &nb);
	isave->Write(&mAnimStartTimeHMS.y, sizeof(mAnimStartTimeHMS.y), &nb);
	isave->Write(&mAnimStartTimeHMS.z, sizeof(mAnimStartTimeHMS.z), &nb);
	isave->EndChunk();

	isave->BeginChunk(ANIMENDTIMEMDY_CHUNK);
	isave->Write(&mAnimEndTimeMDY.x, sizeof(mAnimEndTimeMDY.x), &nb);
	isave->Write(&mAnimEndTimeMDY.y, sizeof(mAnimEndTimeMDY.y), &nb);
	isave->Write(&mAnimEndTimeMDY.z, sizeof(mAnimEndTimeMDY.z), &nb);
	isave->EndChunk();

	isave->BeginChunk(ANIMENDTIMEHMS_CHUNK);
	isave->Write(&mAnimEndTimeHMS.x, sizeof(mAnimEndTimeHMS.x), &nb);
	isave->Write(&mAnimEndTimeHMS.y, sizeof(mAnimEndTimeHMS.y), &nb);
	isave->Write(&mAnimEndTimeHMS.z, sizeof(mAnimEndTimeHMS.z), &nb);
	isave->EndChunk();

	return IO_OK;
}




	

