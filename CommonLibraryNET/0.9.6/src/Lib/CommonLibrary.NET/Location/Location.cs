using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using ComLib.Entities;


namespace ComLib.LocationSupport
{
    public class Location
    {
        private static ILocationService _provider;


        public static void Init(ILocationService service)
        {
            _provider = service;
        }


        public static ILocationService Service
        {
            get { return _provider; }
        }


        #region ILocationService Members

        public static LocationLookUpResult Parse(string text)
        {
            return _provider.Parse(text);
        }


        public static BoolMessageItem<City> CreateCity(string name, string state, string country)
        {
            return _provider.CreateCity(name, state, country);
        }


        public static BoolMessageItem<Country> CreateCountry(string name)
        {
            return _provider.CreateCountry(name);
        }


        public static BoolMessageItem<Country> CreateCountry(string name, string alias, bool isAlias)
        {
            return _provider.CreateCountry(name, alias, isAlias);
        }


        public static BoolMessageItem<State> CreateState(string name, string abbreviation, string country)
        {
            return _provider.CreateState(name, abbreviation, country);
        }


        public static IRepository<City> Cities { get { return _provider.Cities; } }


        public static IRepository<State> States { get { return _provider.States; } }


        public static IRepository<Country> Countries { get { return _provider.Countries; } }


        /// <summary>
        /// Get the city lookup.
        /// </summary>
        public static CityLookUp CitiesLookup { get { return _provider.CitiesLookup; } }


        /// <summary>
        /// Get the city list.
        /// </summary>
        public static IList<City> CitiesList { get { return _provider.CitiesList; } }


        /// <summary>
        /// Get the city lookup.
        /// </summary>
        public static CountryLookUp CountriesLookup { get { return _provider.CountriesLookup; } }


        /// <summary>
        /// Get the country lookup
        /// </summary>        
        public static IList<Country> CountriesList { get { return _provider.CountriesList; } }


        /// <summary>
        /// Get the city lookup.
        /// </summary>
        public static StateLookUp StatesLookup { get { return _provider.StatesLookup; } }


        /// <summary>
        /// Get the states list.
        /// </summary>
        public static IList<State> StatesList { get { return _provider.StatesList; } }


        #endregion


        /// <summary>
        /// Get country with the specified name.
        /// </summary>
        /// <param name="countryName"></param>
        /// <returns></returns>
        public static BoolMessageItem<Country> GetCountry(string countryName)
        {
            return _provider.Country(countryName);
        }   


        /// <summary>
        /// Get the states for the specified country
        /// </summary>
        /// <param name="countryname"></param>
        /// <returns></returns>
        public static BoolMessageItem<IList<State>> StatesFor(string countryname)
        {
            return _provider.StatesFor(countryname);
        }


        /// <summary>
        /// Get the states for the specified country
        /// </summary>
        /// <param name="countryname"></param>
        /// <returns></returns>
        public static BoolMessageItem<IList<City>> CitiesFor(string countryname, string statename)
        {
            return _provider.CitiesFor(countryname, statename);
        }
    }
}
