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
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Collections;
using System.Text.RegularExpressions;
using System.Reflection;

namespace ComLib.Arguments
{
    /// <summary>
    /// Class providing utility methods for parsing a string or collection of arguments.
    /// </summary>
    public partial class Args
    {
        /// <summary>
        /// Parse the line into <see cref="Args"/> object.
        /// </summary>
        /// <param name="text">The line of text containing arguments to parse.</param>
        /// <returns></returns>
        public static BoolMessageItem<Args> Parse(string text)
        {
            return TryCatch(() => Parse(LexArgs.ParseList(text).ToArray()));
        }


        /// <summary>
        /// Parses the line into <see cref="Args"/> object using the supplied prefix and separator.
        /// </summary>
        /// <param name="text">The line of text containing arguments to parse.</param>
        /// <param name="prefix">Prefix used for named arguments. E.g. "-" as in "-env:prod"</param>
        /// <param name="separator">Separator used between name and value of named arguments. E.g. ":" as in "-env:prod"</param>
        /// <returns></returns>
        public static BoolMessageItem<Args> Parse(string text, string prefix, string separator)
        {
            return TryCatch(() => Parse(LexArgs.ParseList(text).ToArray(), prefix, separator));
        }


        /// <summary>
        /// Parses the arguments and checks for named arguments and non-named arguments.
        /// </summary>
        /// <param name="text">The line of text containing arguments to parse.</param>
        /// <param name="prefix">Prefix used for named arguments. E.g. "-" as in "-env:prod"</param>
        /// <param name="separator">Separator used between name and value of named arguments. E.g. ":" as in "-env:prod"</param>
        /// <param name="argReciever">The object to apply the argument values to. This must have ArgAttributes on it's properties.</param>
        /// <returns></returns>
        public static BoolMessageItem<Args> Parse(string text, string prefix, string separator, object argReciever)
        {
            return TryCatch(() => Parse(LexArgs.ParseList(text).ToArray(), prefix, separator, argReciever));
        }
    }   
}
