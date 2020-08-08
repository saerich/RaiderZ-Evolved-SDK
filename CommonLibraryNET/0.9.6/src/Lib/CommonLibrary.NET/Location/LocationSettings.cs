using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib.LocationSupport
{
    public class LocationSettings
    {
        public LocationSettings()
        {
            CacheCityList = false;
            CacheCityLookup = false;
            CacheCountryList = false;
            CacheCountryLookup = false;
            CacheStateList = false;
            CacheStateLookup = false;
            CacheTimeForCitiesInSeconds = 300;
            CacheTimeForCountriesInSeconds = 300;
            CacheTimeForStatesInSeconds = 300;
        }


        public bool CacheCountryList { get; set; }
        public bool CacheStateList { get; set; }
        public bool CacheCityList { get; set; }
        public bool CacheCountryLookup { get; set; }
        public bool CacheStateLookup { get; set; }
        public bool CacheCityLookup { get; set; }

        public int CacheTimeForCountriesInSeconds { get; set; }
        public int CacheTimeForStatesInSeconds { get; set; }
        public int CacheTimeForCitiesInSeconds { get; set; }
    }
}
