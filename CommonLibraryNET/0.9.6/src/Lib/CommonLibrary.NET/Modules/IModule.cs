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
    public interface IModule
    {
        /// <summary>
        /// The definition.
        /// </summary>
        IModuleDefinition Definition { get; set; }


        /// <summary>
        /// The specific instance
        /// </summary>
        IModuleInstance Instance { get; set; }


        /// <summary>
        /// The modules settings
        /// </summary>
        IModuleSettings Settings { get; set; }


        /// <summary>
        /// The module id.
        /// </summary>
        string Id { get; }
    }




    /// <summary>
    /// Interface for a UI component representing a module.
    /// </summary>
    public interface IModuleView
    {
        ModuleBehaviour Behaviour { get; set; }

        IModule Module { get; set; }
    }
}
