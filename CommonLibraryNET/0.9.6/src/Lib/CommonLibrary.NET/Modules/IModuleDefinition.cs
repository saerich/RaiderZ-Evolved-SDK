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
namespace ComLib.Modules
{
    /// <summary>
    /// ModuleDefinition.
    /// </summary>
    public interface IModuleDefinition : IModuleSettings
    {
        /// <summary>
        /// Name of this module.
        /// </summary>
        string Name { get; set; }


        /// <summary>
        /// The description of this module.
        /// </summary>
        string Description { get; set; }
        
        
        /// <summary>
        /// Location of the source component that displays this module.
        /// </summary>
        string SourceView { get; set; }


        /// <summary>
        /// Location of the source component that displays
        /// the configuration/settings for this module.
        /// </summary>
        string SourceEdit { get; set; }


        /// <summary>
        /// Flag indicating whether or not this module is editable.
        /// </summary>
        bool IsEditable { get; set; }        
    }
}
