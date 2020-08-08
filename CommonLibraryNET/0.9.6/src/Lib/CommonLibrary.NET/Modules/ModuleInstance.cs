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




namespace ComLib.Modules
{
    /// <summary>
    /// Class to store the definition information for a module.
    /// </summary>
    public class ModuleInstance : ModuleBase, IModuleInstance
    {
        /// <summary>
        /// Name of this module.
        /// </summary>
        public string Name { get; set; }


        /// <summary>
        /// The description of this module.
        /// </summary>
        public string Description { get; set; }


        /// <summary>
        /// Location of the source component that displays
        /// the configuration/settings for this module.
        /// </summary>
        public bool IsCachable { get; set; }


        /// <summary>
        /// How the item is ordered on the page.
        /// </summary>
        public int SortOrder { get; set; }


        /// <summary>
        /// The name of the panel/pane that will host this control.
        /// </summary>
        public string ContainerName { get; set; }


        /// <summary>
        /// The names of the roles that can edit this control.
        /// </summary>
        public string Roles { get; set; }
    }
}
