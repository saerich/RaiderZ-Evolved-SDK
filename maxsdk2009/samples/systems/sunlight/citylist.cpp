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

#pragma managed
#include "citylist.h"

CityList* CityList::sInstance = nullptr;

CityList& CityList::GetInstance()
{
	if(nullptr == sInstance)
	{
		sInstance = new CityList();
	}

	return *sInstance;
}

void CityList::Destroy()
{
	delete sInstance;
	sInstance = nullptr;
}

CityList::CityList()
	: mpDfltCity(NULL), mpCityList(NULL), mpCityNames(NULL), 
      mNumCities(0), mCityNameSize(0)
{
}

CityList::~CityList()
{
    delete   mpDfltCity;
	delete[] mpCityList;
	delete[] mpCityNames;
}

void CityList::moreNames(UINT size)
{
	if (size > mCityNameSize) {
		TCHAR* p = static_cast<TCHAR*>(realloc(mpCityNames, size + 1024));
		if (p == NULL)
			throw std::bad_alloc();
		mpCityNames = p;
	}
}

int CityList::byname(const void* p1, const void* p2)
{
	const TCHAR* e1 = static_cast<const Entry*>(p1)->name;
	const TCHAR* e2 = static_cast<const Entry*>(p2)->name;
	if ((e1[0] == '+') || (e1[0] == '*'))
		++e1;
	if ((e2[0] == '+') || (e2[0] == '*'))
		++e2;
	return _tcscmp(e1, e2);
}

bool CityList::parseLine(FILE* fp, Entry& entry, UINT& namePos)
{
	TCHAR buf[512];

	if (fgets(buf, sizeof(buf) / sizeof(buf[0]) - 1, fp) != NULL) {
		TCHAR* p;
		TCHAR* first = NULL;
		TCHAR* last = NULL;

		for (p = buf; p[0] != '\0' && p[0] != ';'; p += _tclen(p)) {
			if (p[0] == ' ' || p[0] == '\t' || p[0] == '\r' || p[0] == '\n') {
				// Found white space.
				if (last == NULL)
					last = p;
			}
			else {
				if (first == NULL)
					first = p;
				last = NULL;
			}
		}

		// Strip trailing spaces if any, and any comments.
		if (last != NULL)
			last[0] = '\0';
		else
			p[0] = '\0';

		if (first != NULL) {
			int latitude, longitude, name;
			if (_stscanf(first, "%d %d %n", &latitude, &longitude, &name) == 2
					&& first[name] != '\0') 
            {

			
			entry.latitude = float(latitude) / 10000.0f;
			entry.longitude = float(longitude) / 10000.0f; // mz dec 2007, no longer true, we fixed this hopefullypfbreton 13 04 2006: the rest of the max code assumes that negative longitude represents EST, but the Acad data provides us with positive values for EST so I flip the value here
				
				entry.nameOff = namePos;
				UINT nameEnd = static_cast<UINT>(namePos + _tcslen(first + name) + 1);	// SR DCAST64: Downcast to 4G limit.
				moreNames(nameEnd);
				_tcscpy(mpCityNames + namePos, first + name);
				namePos = nameEnd;

				return true;
			}
		}
	}
	return false;
}

void CityList::initializeList()
{
	mNumCities = 0;
    delete mpDfltCity;
    mpDfltCity = NULL;
	delete[] mpCityList;
	mpCityList = NULL;
	delete[] mpCityNames;
	mpCityNames = NULL;
	mCityNameSize = 0;

	Interface* ip = GetCOREInterface();
	TSTR cityFile = ip->GetDir(APP_PLUGCFG_DIR);
	cityFile += "\\sitename.txt";

	// Open the city file.
	FILE* fp = fopen(cityFile.data(), "r");
	if (fp == NULL)
		return;			// No file, return with no cities

	// First count the cities in the file.
	UINT count = 0;
	UINT nameSize = 0;
	{
		Entry temp;
		while (!feof(fp)) {
			UINT namePos = 0;
			if (parseLine(fp, temp, namePos)) {
				++count;
				nameSize += namePos;
			}
		}
	}

	if (count <= 0)
		return;		// No Cities

	mpCityList = new Entry[count];
	mCityNameSize = nameSize;
	mpCityNames = static_cast<TCHAR*>(realloc(mpCityNames, mCityNameSize * sizeof(TCHAR)));
	UINT namePos = 0;

	fseek(fp, 0L, SEEK_SET);
   UINT i;
	for (i = 0; i < count && !feof(fp); ) {
		i += parseLine(fp, mpCityList[i], namePos);
	}
	fclose(fp);

	count = i;
	for (i = 0; i < count; ++i)
    {
		mpCityList[i].name = mpCityNames + mpCityList[i].nameOff;

        // should we initialise the default city?
        if ((mpDfltCity == NULL) && (mpCityList[i].name[0] == '*'))
        {
            mpDfltCity = new Entry;
            mpDfltCity->latitude    = mpCityList[i].latitude;
            mpDfltCity->longitude   = mpCityList[i].longitude;
            mpDfltCity->name        = mpCityList[i].name;
            mpDfltCity->nameOff     = mpCityList[i].nameOff;
        }
    }

	if (count > 0) {
		qsort(mpCityList, count, sizeof(mpCityList[0]), byname);
		mNumCities = count;
	}
}

