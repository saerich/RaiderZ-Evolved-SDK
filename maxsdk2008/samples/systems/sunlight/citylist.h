
/****************
	This default city can be modified with a simple mod to the sitename.txt file 
    installed to the users disk.
	
	The city name has two potential modifiers
	 + indicates that the city is a major site and is used to find the nearest city 
       in the location dialog
	 * indicates the city is a major site, and is also the default city
	
	If a default city is not found, San Fransisco, CA is always used.
	If more than one default city is found, the first one in the file wins.
	
	This comment (added to the sitename.txt file) describes the process again
	
	;    18-End     Place name.  If the first character of the place
	;                 name is "+", this is a "major site" which is a
	;                 candidate place for the "Nearest city" search.
    ;
    ;                 Otherwise the place simply appears in the list
	;                 of site names available for explicit selection.
	;                 If the first character of the place name is "*",
	;                 it is the default location city and is also a 
	;                 a major site. 
    ;                 If no default location is found, San Fransico, 
    ;                 CA becomes the default and if more  than one is 
    ;                 found the first in the list becomes the default.
	
	Note that in the Japanese locale, Tokyo is no longer set as the default city 
    if using a Japanese code page. It is expected that the Japanese localisation 
    will modify sitename.txt to change the '*' before San Fransisco to a '+' 
    and to modify the '+' before Tokyo, Japan to a '*'
	
	Also in order that a valid geographic map gets selected we take the default city 
    and search through the known map files in order until a map is found containing 
    the longitude and latitude of that default city.
	Note that if a city lies within multiple maps, it might not show the map the user 
    would have chosen for that city, but at least the correct city is highlighted on 
    the map - for example with Toronto below...
	
	Note that the map order is such that we have the appropriate search 
    order, for example we should search Subcontinental Asia before Asia
	
	Tested the following city's, and got the correct maps - except for Toronto as explained above
	 Nairobi, Kenya => Africa
	 Tokyo, Japan   => Asia
	 Sydney, Australia => Australia
	 New Delhi, India => India
	 Toronto, Canada => North America
	 Paris, France
	 Lima, Peru
	 San Francisco, CA => North America

	***********************/
	
class CityList {
public:
	struct Entry {
		float latitude;
		float longitude;
		union {
			char* name;
			UINT  nameOff;
		};
	};

	CityList();
	~CityList();

	UINT Count() { return mNumCities; }
	Entry& operator[](UINT i) { return mpCityList[i]; }
	Entry* operator+(UINT i) { return mpCityList + i; }
    
    Entry* GetDefaultCity()  { return mpDfltCity; }

	void init() { if (mpCityList == NULL) initializeList(); }

private:
	void initializeList();
	void moreNames(UINT size);
	static int byname(const void* p1, const void* p2);
	bool parseLine(FILE* fp, Entry& entry, UINT& namePos);

	Entry*      mpDfltCity;
	Entry*		mpCityList;
    TCHAR*		mpCityNames;
	UINT		mNumCities;
	UINT		mCityNameSize;
};

extern CityList cityList;
