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
using System.Collections.Specialized;
using System.Collections.ObjectModel;
using System.Collections;
using System.Text;
using System.Linq;
using ComLib;

namespace ComLib.Collections
{

    /// <summary>
    /// Collection utility methods.
    /// </summary>
    public class CollectionUtils
    {
        /// <summary>
        /// Determines whether the specified enumerable collection is empty.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="items">Collection to test</param>
        /// <returns>
        /// 	<c>true</c> if the specified collection is empty; otherwise, <c>false</c>.
        /// </returns>
        [Obsolete("Method moved to Utilities.EnumerableExtensions.IsNullOrEmpty()")]
        public static bool IsEmpty<T>(IEnumerable<T> items)
        {
            return items.IsNullOrEmpty();
        }
    }
}
