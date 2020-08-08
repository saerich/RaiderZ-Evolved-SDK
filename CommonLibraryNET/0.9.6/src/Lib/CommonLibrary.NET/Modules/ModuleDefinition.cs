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
    public class ModuleDefinition : ModuleBase, IModuleDefinition
    {
        /// <summary>
        /// Location of the source component that displays this module.
        /// </summary>
        public string SourceView { get; set; }


        /// <summary>
        /// Location of the source component that displays
        /// the configuration/settings for this module.
        /// </summary>
        public string SourceEdit { get; set; }


        /// <summary>
        /// Flag indicating whether or not this module is editable.
        /// </summary>
        public bool IsEditable { get; set; }


        /// <summary>
        /// Name of this module.
        /// </summary>
        public string Name { get; set; }


        /// <summary>
        /// The description of this module.
        /// </summary>
        public string Description { get; set; }


        /// <summary>
        /// Allow default constructor.
        /// </summary>
        public ModuleDefinition() { }


        /// <summary>
        /// Initalize the module
        /// </summary>
        /// <param name="name"></param>
        /// <param name="description"></param>
        /// <param name="appid"></param>
        /// <param name="modId"></param>
        /// <param name="instanceId"></param>
        /// <param name="sourceView"></param>
        /// <param name="sourceEdit"></param>
        /// <param name="isEditable"></param>
        public ModuleDefinition(string name, string description, string appid, string modId, string instanceId, string sourceView, string sourceEdit, bool isEditable)
        {
            Name = name;
            Description = description;
            AppId = appid;
            ModuleId = modId;
            InstanceId = instanceId;
            SourceView = sourceView;
            SourceEdit = sourceEdit;
            IsEditable = isEditable;
        }
    }
}
