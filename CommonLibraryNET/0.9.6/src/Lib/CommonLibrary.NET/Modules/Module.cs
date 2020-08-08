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
    /// The different ways a module can behave and be responsible for itself.
    /// </summary>
    public enum ModuleBehaviour 
    { 
        /// <summary>
        /// In passive mode the module yeilds all it's control to 
        /// a external controller that manages the module.
        /// </summary>
        Passive, 
        

        /// <summary>
        /// As an active controller, the module is responsible for 
        /// managing itself.
        /// </summary>
        ActiveController 
    };



    public class Module : IModule
    {
        /// <summary>
        /// The definition.
        /// </summary>
        public IModuleDefinition Definition { get; set; }


        /// <summary>
        /// The specific instance
        /// </summary>
        public IModuleInstance Instance { get; set; }


        /// <summary>
        /// The modules settings
        /// </summary>
        public IModuleSettings Settings { get; set; }


        /// <summary>
        /// Get the module id.
        /// </summary>
        public string Id { get; set; }


        /// <summary>
        /// Allow default construction.
        /// </summary>
        public Module() { }


        /// <summary>
        /// Initialize using the definition, instance, and settings.
        /// </summary>
        /// <param name="def"></param>
        /// <param name="instance"></param>
        /// <param name="settings"></param>
        public Module(string id, IModuleDefinition def, IModuleInstance instance, IModuleSettings settings)
        {
            Definition = def;
            Instance = instance;
            Settings = settings;
            Id = id;
        }
    }
}
