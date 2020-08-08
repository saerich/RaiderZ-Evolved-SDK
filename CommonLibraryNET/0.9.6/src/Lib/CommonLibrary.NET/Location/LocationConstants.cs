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

namespace ComLib.LocationSupport
{
    /// <summary>
    /// This enumeration matches up with the enumerations used in the stored procedure
    /// for searching up by location.
    /// </summary>
    public enum LocationLookUpType { All = 0, City = 1, CityState = 2, CityCountry = 3, CityStateCountry = 4, State = 5, Country = 6, Zip = 7, None = 8 };



    /// <summary>
    /// Location constants.
    /// </summary>
    public class LocationConstants
    {
        public const string CountryUsa = "USA";
        public const int CountryId_USA = 230;
        public const int ZipCodeLength = 5;

        //Constants for Country, State, Zip if class is online.
        public const int CountryId_NA_Online = -1;
        public const string ZipCode_NA_Online = "00000";
        public const int StateId_NA_Online = -1;
        public const int CityId_NA = -1;
    }

}
