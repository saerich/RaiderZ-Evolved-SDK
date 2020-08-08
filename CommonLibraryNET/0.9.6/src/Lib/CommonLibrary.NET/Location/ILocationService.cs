/*
 * Author: Kishore Reddy
 * Url: http://commonlibrarynet.codeplex.com/
 * Title: CommonLibrary.NET
 * Copyright: � 2009 Kishore Reddy
 * License: LGPL License
 * LicenseUrl: http://commonlibrarynet.codeplex.com/license
 * Description: A C# based .NET 3.5 Open-Source collection of reusable components.
 * Usage: Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using System.Collections.Generic;
using System.Text;

using ComLib.Entities;


namespace ComLib.LocationSupport
{
    /// <summary>
    /// Location service to parse locatiion data.
    /// </summary>
    public interface ILocationService
    {
        /// <summary>
        /// Cities repository
        /// </summary>
        IRepository<City> Cities { get; set; }


        /// <summary>
        /// States repository.
        /// </summary>
        IRepository<State> States { get; set; }


        /// <summary>
        /// Countries repository.
        /// </summary>
        IRepository<Country> Countries { get; set; }


        /// <summary>
        /// Get the cities lookup
        /// </summary>
        CityLookUp CitiesLookup { get; }
        
        
        /// <summary>
        /// Get the city list.
        /// </summary>
        IList<City> CitiesList { get; }


        /// <summary>
        /// Get the city lookup.
        /// </summary>
        CountryLookUp CountriesLookup { get; }


        /// <summary>
        /// Get the country lookup.
        /// </summary>
        IList<Country> CountriesList { get; }


        /// <summary>
        /// Get the city lookup.
        /// </summary>
        StateLookUp StatesLookup { get; }


        /// <summary>
        /// Get the states list.
        /// </summary>
        IList<State> StatesList { get; }


        /// <summary>
        /// Get the location settings.
        /// </summary>
        LocationSettings Settings { get; }


        /// <summary>
        /// Parses string for either the zip or city/state.
        /// e.g.
        ///     City:  "Bronx", "Stamford"
        ///     State: "NY", "NJ", "California"
        ///     CityState: "Queens,New York"
        ///     Country: "USA"
        /// </summary>
        /// <param name="text"></param>
        /// <returns></returns>
        LocationLookUpResult Parse(string text);


        BoolMessageItem<City> CreateCity(string name, string state, string country);


        BoolMessageItem<Country> CreateCountry(string name);


        BoolMessageItem<Country> CreateCountry(string name, string alias, bool isAlias);


        BoolMessageItem<State> CreateState(string name, string abbreviation, string country);


        BoolMessageItem<Country> Country(string name);


        BoolMessageItem<IList<State>> StatesFor(string countryname);


        BoolMessageItem<IList<City>> CitiesFor(string countryname, string states);
    }
}
