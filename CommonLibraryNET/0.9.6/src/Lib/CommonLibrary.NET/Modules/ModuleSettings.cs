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
using ComLib.Configuration;


namespace ComLib.Modules
{
    /// <summary>
    /// Base class used for module settings.
    /// </summary>
    public class ModuleSettings : ModuleBase, IModuleSettings
    {

        public ModuleSettings()
        {
            AppId = string.Empty;
            ModuleId = string.Empty;
            InstanceId = string.Empty;           
        }
    }


    public class ModuleBase : ConfigSource
    {
        /// <summary>
        /// Flag that indicates whether or not to load the settings from the data store.
        /// </summary>
        public string InitAction { get; set; }


        /// <summary>
        /// The app id associated with these settings.
        /// This can be either a global application id or a userid to enable
        /// associating settings for a specific user.
        /// </summary>
        public string AppId { get; set; }


        /// <summary>
        /// The module associated with these settings.
        /// </summary>
        public string ModuleId { get; set; }


        /// <summary>
        /// The id of the instance of the module.
        /// </summary>
        public string InstanceId { get; set; }


        /// <summary>
        /// Initalizes the settings.
        /// </summary>
        public void Init()
        {
        }


        /// <summary>
        /// Get the string value for the specified key.
        /// </summary>
        /// <param name="key"></param>
        /// <returns></returns>
        public IConfigSource Settings { get; set; }
    }
}
