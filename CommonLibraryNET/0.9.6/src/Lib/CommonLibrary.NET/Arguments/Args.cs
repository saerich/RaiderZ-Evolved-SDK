
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
using System.Linq;
using System.Text;
using System.Collections;
using System.Text.RegularExpressions;
using System.Reflection;
using System.Collections.ObjectModel;


namespace ComLib.Arguments
{
   /// <summary>
    /// Class to parse arguments and store both named and positional arguments for easy lookup.
    /// </summary>
    /// <example>
    /// <para>
    ///     
    /// </para>
    /// </example>
    public partial class Args
    {
        #region Private Data
        /// <summary>
        /// Index position of meta request.
        /// e.g. -help, -pause, -version
        /// </summary>
        private int _metaIndex = 0;
        #endregion


        #region Public Properties
        /// <summary>
        /// The argument schema containing what options are available.
        /// </summary>
        public ArgsSchema Schema { get; private set; }


        /// <summary>
        /// Named args prefix used. "-"
        /// e.g. -env:Production
        /// </summary>
        public string Prefix { get; set; }


        /// <summary>
        /// Named args key / value separator used. ":"
        /// e.g. -env:Production
        /// </summary>
        public string Separator { get; set; }


        /// <summary>
        /// <para>
        /// Collection of named arguments.
        /// e.g. If "-config:prod.xml -date:T-1 MyApplicationId" is supplied to command line.
        /// 
        /// Named["config"] = "prod.xml"
        /// Named["date"] = "T-1"
        /// </para>
        /// </summary>
        public Dictionary<string, string> Named = new Dictionary<string, string>();


        /// <summary>
        /// <para>
        /// Collection of un-named arguments supplied to command line.
        /// e.g. If "-config:prod.xml -date:T-1 MyApplicationId" is supplied to command line.
        /// 
        /// Positional[0] = "MyApplicationId"
        /// </para>
        /// </summary>
        public List<string> Positional = new List<string>();


        /// <summary>
        /// The original/raw arguments that were supplied.
        /// </summary>
        public string[] Raw = null;
        #endregion


        #region Constructors
        public Args()
            : this("-", ":")
        {
        }


        /// <summary>
        /// Default construction.
        /// </summary>
        public Args(string[] args) 
            : this(args, null)
        {            
        }


        /// <summary>
        /// Initialize only the prefix / separator.
        /// </summary>
        public Args(string prefix, string separator)            
        {
            Prefix = prefix;
            Separator = separator;
            Schema = new ArgsSchema(this);
        }


        /// <summary>
        /// Default construction.
        /// </summary>
        public Args(string[] args, string prefix, string separator)
            : this(args, prefix, separator, null, null, null)
        {
        }


        /// <summary>
        /// Default construction.
        /// </summary>
        public Args(string[] args, List<ArgAttribute> supported) 
            : this(args, "-", ":", supported, null, null)
        {
        }


        /// <summary>
        /// Default construction.
        /// </summary>
        public Args(string[] args, string prefix, string separator, List<ArgAttribute> supported)
            : this(args, prefix, separator, supported, null, null)
        {
        }


        /// <summary>
        /// Initialize arguments.
        /// </summary>
        /// <param name="args">Raw arguments from command line.</param>
        /// <param name="supported">Supported named/positional argument definitions.</param>
        /// <param name="named">Named arguments</param>
        /// <param name="positional">Positional arguments.</param>
        public Args(string[] args, List<ArgAttribute> supported, Dictionary<string, string> named, List<string> positional)
            : this(args, "-", ":", supported, named, positional)
        {
        }


        /// <summary>
        /// Initialize arguments.
        /// </summary>
        /// <param name="args">Raw arguments from command line.</param>
        /// <param name="prefix">Prefix used for named arguments. "-".</param>
        /// <param name="keyValueSeparator">Separator used for named arguments key/values. ":".</param>
        /// <param name="supported">Supported named/positional argument definitions.</param>
        /// <param name="named">Named arguments</param>
        /// <param name="positional">Positional arguments.</param>
        public Args(string[] args, string prefix, string keyValueSeparator, List<ArgAttribute> supported, Dictionary<string, string> named, List<string> positional)
        {            
            Raw = args;            
            Prefix = prefix;
            Separator = keyValueSeparator;
            Schema = new ArgsSchema(this);
            if (supported != null) Schema.Items = supported; 
            if (named != null) Named = named;
            if(positional != null) Positional = positional;            
        }
        #endregion


        #region Parse methods
        /// <summary>
        /// Parse the raw arguments using the internal schema.
        /// </summary>
        /// <param name="rawArgs"></param>
        /// <returns></returns>
        public BoolMessage DoParse(string[] rawArgs)
        {
            var result =Parse(rawArgs, Prefix, Separator, Schema.Items);

            // Transfer the Positional and Named back over to this instance.
            if (result.Success)
            {
                this.Positional = result.Item.Positional;
                this.Named = result.Item.Named;
            }
            this.Raw = rawArgs;
            return result;
        }
        #endregion


        #region Convenience methods
        /// <summary>
        /// Whether or not the named argument exists.
        /// </summary>
        /// <param name="namedArg"></param>
        /// <returns></returns>
        public bool Contains(string namedArg)
        {
            return Named != null && Named.ContainsKey(namedArg);
        }


        /// <summary>
        /// Get the named argument specified by <paramref name="argName"/>
        /// throws an exception if the named argument is not present.
        /// </summary>
        /// <param name="indexPosition">Index position of the argument.</param>
        /// <returns></returns>
        public T Get<T>(string argName)
        {
            if (!Named.ContainsKey(argName))
                throw new ArgumentException("There is no named argument : " + argName);

            string val = Named[argName];
            if (string.IsNullOrEmpty(val))
                throw new ArgumentException("The named argument : " + argName + " is empty");

            T converted = Converter.ConvertTo<T>(val);
            return converted;
        }


        /// <summary>
        /// Get the named argument specified by <paramref name="argName"/>
        /// if it exists, returns <paramref name="defaultValue"/> otherwise.
        /// </summary>
        /// <param name="argName">Name of the named argument.</param>
        /// <param name="defaultValue">Default value to return if named arg does not exist.</param>
        /// <returns></returns>
        public T Get<T>(string argName, T defaultValue)
        {
            if (!Named.ContainsKey(argName))
                return defaultValue;

            string val = Named[argName];
            if (string.IsNullOrEmpty(val))
                return defaultValue;

            T converted = Converter.ConvertTo<T>(val);
            return converted;
        }


        /// <summary>
        /// Get the positional argument specified by <paramref name="indexPosition"/>
        /// throws an exception if the positional argument is not present.
        /// </summary>
        /// <param name="indexPosition">Index position of the argument.</param>
        /// <returns></returns>
        public T Get<T>(int indexPosition)
        {
            if (Positional == null || Positional.Count == 0)
                throw new ArgumentException("There are 0 positional arguments");

            string val = Positional[indexPosition];
            T converted = Converter.ConvertTo<T>(val);
            return converted;
        }


        /// <summary>
        /// Get the positional argument specified by <paramref name="indexPosition"/>
        /// if it exists, returns <paramref name="defaultValue"/> otherwise.
        /// </summary>
        /// <param name="argName">Name of the named argument.</param>
        /// <param name="defaultValue">Default value to return if named arg does not exist.</param>
        /// <returns></returns>
        public T Get<T>(int indexPosition, T defaultValue)
        {
            if(Positional == null || Positional.Count <= indexPosition)
                return defaultValue;

            string val = Positional[indexPosition];
            if (string.IsNullOrEmpty(val))
                return defaultValue;

            T converted = Converter.ConvertTo<T>(val);
            return converted;
        }
        #endregion


        #region Checks
        /// <summary>
        /// True if there are 0 arguments.
        /// </summary>
        public bool IsEmpty
        {
            get { return Raw == null || Raw.Length == 0; }
        }


        /// <summary>
        /// Returns true if there is only 1 argument with value:  --Version -Version /Version
        /// </summary>
        /// <param name="args"></param>
        /// <returns></returns>
        public bool IsVersion
        {
            get { return IsPositionalArg(_metaIndex, "version"); }
        }


        /// <summary>
        /// Returns true if there is only 1 argument with value: --pause -pause /pause
        /// </summary>
        /// <param name="args"></param>
        /// <returns></returns>
        public bool IsPause
        {
            get { return IsPositionalArg(_metaIndex, "pause"); }
        }


        /// <summary>
        /// Returns true if there is only 1 argument with value: --help -help --help -help /? -? ?
        /// </summary>
        /// <param name="args"></param>
        /// <returns></returns>
        public bool IsHelp
        {
            get { return IsPositionalArg(_metaIndex, "help", "?", "/?", "-?"); }
        }


        /// <summary>
        /// Returns true if there is only 1 argument with value: --About -About /About
        /// </summary>
        public bool IsInfo
        {
            get { return IsPositionalArg(_metaIndex, "about"); }
        }
        #endregion


        #region Usage
        /// <summary>
        /// Show the usage of the arguments on the console.
        /// </summary>
        public void ShowUsage(string appName)
        {
            string usage = GetUsage(appName);
            Console.WriteLine(usage);
        }


        /// <summary>
        /// Get a string representing the usage of the arguments.
        /// </summary>
        /// <returns></returns>
        public string GetUsage(string appName)
        {
            if (Schema.IsEmpty) return "Argument definitions are not present.";

            string usage = ArgsUsage.Build(appName, Schema.Items, Prefix, Separator);
            return usage;
        }
        #endregion


        /// <summary>
        /// Set the index position of the argument indicating a specific meta query.
        /// Such as "-help", "-version", "-about".
        /// e.g. if 0, this indicates that the argument "-help" should be expected at position 0
        /// in the raw arguments.
        /// </summary>
        /// <param name="ndx"></param>
        public void SetMetaIndex(int ndx)
        {
            if(ndx >= 0) _metaIndex = ndx;
        }



        /// <summary>
        /// Checks if the first positional arg in the raw arguments is equal to 
        /// what's provided.
        /// </summary>
        /// <param name="valToCheck"></param>
        /// <returns></returns>
        private bool IsPositionalArg(int metaIndex, string valToCheck, params string[] additionalValues)
        {
            string[] args = Raw;
            valToCheck = valToCheck.ToLower().Trim();

            // Check for help
            if (args != null && args.Length >= 1)
            {
                string val = args[metaIndex].ToLower();
                if (val == Prefix + valToCheck || val == "-" + valToCheck  || val == "--" + valToCheck || val == "/" + valToCheck || val == valToCheck)
                    return true;

                if (additionalValues != null && additionalValues.Length > 0)
                {
                    foreach (string additionalVal in additionalValues)
                        if (additionalVal == val)
                            return true;
                }
            }
            return false;
        }
    }
}
