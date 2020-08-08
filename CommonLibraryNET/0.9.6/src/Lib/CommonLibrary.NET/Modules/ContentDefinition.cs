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
    public class ViewDefinition
    {
        /// <summary>
        /// The name of this view.( This can be a webpage name. ).
        /// e.g. "Home.aspx"
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// List of modules that are associated with this view.
        /// </summary>
        public IList<Module> Modules { get; set; }
    }
}
