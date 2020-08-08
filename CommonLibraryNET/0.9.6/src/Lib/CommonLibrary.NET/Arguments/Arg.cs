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


namespace ComLib.Arguments
{
    /// <summary>
    /// Attribute to apply to properties of an object which can 
    /// recieve the argument values supplied to a program.
    /// This also describes named arguments.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property)]
    public class ArgAttribute : Attribute
    {
        /// <summary>
        /// Name of the argument.
        /// </summary>
        public string Name = string.Empty;


        /// <summary>
        /// Short alias to represent the name.
        /// </summary>
        public string Alias = string.Empty;


        /// <summary>
        /// Describes the argument name.
        /// </summary>
        public string Description = string.Empty;


        /// <summary>
        /// Default value.
        /// </summary>
        public object DefaultValue = null;


        /// <summary>
        /// Date type of the argument.
        /// </summary>
        public Type DataType;


        /// <summary>
        /// Indicates if argument is case sensitive.
        /// </summary>
        public bool IsCaseSensitive = false;


        /// <summary>
        /// Indicates if is required.
        /// </summary>
        public bool IsRequired = true;


        /// <summary>
        /// The index position of any un-named args.
        /// As in index 0 = a, 1 = b in "-config:Prod a b c"
        /// where a, b, c are unnamed arguments and -config:Prod is a named argument.
        /// </summary>
        public int IndexPosition = 0;


        /// <summary>
        /// Flag to indicate if this value should be interpreted.
        /// </summary>
        public bool Interpret = false;


        /// <summary>
        /// Whether or not this is used only for development.
        /// </summary>
        public bool IsUsedOnlyForDevelopment = false;


        /// <summary>
        /// Example value.
        /// </summary>
        public string Example = string.Empty;


        /// <summary>
        /// Example of mutliple various values.
        /// </summary>
        public string ExampleMultiple = string.Empty;


        /// <summary>
        /// Used to group various arguments.
        /// e.g. The tag can be used to separate base/derived argument defintions.
        /// </summary>
        public string Tag = string.Empty;


        /// <summary>
        /// Allow initialize via named property initializers.
        /// </summary>
        public ArgAttribute() { }


        /// <summary>
        /// Initialize using description.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="description"></param>
        /// <param name="dataType"></param>
        /// <param name="isRequired"></param>
        /// <param name="isCaseSensitive"></param>
        /// <param name="defaultValue"></param>
        public ArgAttribute(string name, string description, Type dataType, bool isRequired, object defaultValue, string example,
            bool interpret, bool isCaseSensitive, bool onlyForDevelopment)
            : this(name, description, dataType, isRequired, defaultValue, example, example, interpret, isCaseSensitive, onlyForDevelopment)
        {
        }


        /// <summary>
        /// Initialize using description.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="description"></param>
        /// <param name="dataType"></param>
        /// <param name="isRequired"></param>
        /// <param name="isCaseSensitive"></param>
        /// <param name="defaultValue"></param>
        public ArgAttribute(string name, string description, Type dataType, bool isRequired, object defaultValue, string example, string exampleMultiple,
            bool interpret, bool isCaseSensitive, bool onlyForDevelopment)
        {
            Init(name, description, dataType, isRequired, isCaseSensitive, defaultValue, onlyForDevelopment, example, exampleMultiple);
            Interpret = interpret;
        }


        /// <summary>
        /// Initialize using description.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="description"></param>
        /// <param name="dataType"></param>
        /// <param name="isRequired"></param>
        /// <param name="isCaseSensitive"></param>
        /// <param name="defaultValue"></param>
        public ArgAttribute(string name, string description, Type dataType, bool isRequired, object defaultValue, string example)
        {
            Init(name, description, dataType, isRequired, false, defaultValue, false, example, example);
            Interpret = false;
        }


        /// <summary>
        /// Initialize using description.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="description"></param>
        /// <param name="dataType"></param>
        /// <param name="isRequired"></param>
        /// <param name="isCaseSensitive"></param>
        /// <param name="defaultValue"></param>
        public ArgAttribute(string name, string description, Type dataType, bool isRequired, object defaultValue, string example, string exampleMultiple)
        {
            Init(name, description, dataType, isRequired, false, defaultValue, false, example, exampleMultiple);
            Interpret = false;
        }


        /// <summary>
        /// Initialize using description.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="description"></param>
        /// <param name="dataType"></param>
        /// <param name="isRequired"></param>
        /// <param name="isCaseSensitive"></param>
        /// <param name="defaultValue"></param>
        public ArgAttribute(int indexPosition, string description, Type dataType, bool isRequired, bool isCaseSensitive, object defaultValue,
            bool onlyForDevelopment, string example)
        {
            Init(string.Empty, description, dataType, isRequired, isCaseSensitive, defaultValue, onlyForDevelopment, example, example);
            IndexPosition = indexPosition;
        }

        
        /// <summary>
        /// Initialize using description.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="description"></param>
        /// <param name="dataType"></param>
        /// <param name="isRequired"></param>
        /// <param name="isCaseSensitive"></param>
        /// <param name="defaultValue"></param>
        public ArgAttribute(int indexPosition, string description, Type dataType, bool isRequired, object defaultValue, string example)
            : this(indexPosition, description, dataType, isRequired, defaultValue, example, example)
        {
        }


        /// <summary>
        /// Initialize using description.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="description"></param>
        /// <param name="dataType"></param>
        /// <param name="isRequired"></param>
        /// <param name="isCaseSensitive"></param>
        /// <param name="defaultValue"></param>
        public ArgAttribute(int indexPosition, string description, Type dataType, bool isRequired, object defaultValue, string example, string exampleMultiple)
        {
            Init(string.Empty, description, dataType, isRequired, false, defaultValue, false, example, exampleMultiple);
            IndexPosition = indexPosition;
            Interpret = false;
        }


        /// <summary>
        /// Initialize using description.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="description"></param>
        /// <param name="dataType"></param>
        /// <param name="isRequired"></param>
        /// <param name="isCaseSensitive"></param>
        /// <param name="defaultValue"></param>
        public ArgAttribute(int indexPosition, string description, Type dataType, bool isRequired, bool interpret, object defaultValue, string example, string exampleMultiple)
        {
            Init(string.Empty, description, dataType, isRequired, false, defaultValue, false, example, exampleMultiple);
            IndexPosition = indexPosition;
            Interpret = interpret;
        }


        /// <summary>
        /// Initialize all the properties.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="description"></param>
        /// <param name="dataType"></param>
        /// <param name="isRequired"></param>
        /// <param name="isCaseSensitive"></param>
        /// <param name="defaultValue"></param>
        /// <param name="onlyForDevelopment"></param>
        /// <param name="example"></param>
        public void Init(string name, string description, Type dataType, bool isRequired, bool isCaseSensitive, object defaultValue,
            bool onlyForDevelopment, string example, string exampleMultiple)
        {
            Name = name;
            Description = description;
            DataType = dataType;
            IsRequired = isRequired;
            IsCaseSensitive = isCaseSensitive;
            DefaultValue = defaultValue;
            IsUsedOnlyForDevelopment = onlyForDevelopment;
            Example = example;
            ExampleMultiple = exampleMultiple;
        }


        public bool IsNamed
        {
            get { return !string.IsNullOrEmpty(Name); }
        }


        /// <summary>
        /// Build the usage for this argument.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            string val = "";
            string startId = "";

            if (!string.IsNullOrEmpty(Name))
                startId = "-" + Name + " : ";
            else
                startId = "- index[" + IndexPosition + "] : ";

            val += string.IsNullOrEmpty(Description) ? string.Empty : Description + ", ";
            val += IsRequired ? "Required" : "Optional";
            val += ", " + DataType.Name + ", ";
            val += IsCaseSensitive ? "Case Sensitive" : "Not CaseSensitive";
            val += DefaultValue != null ? ", default to : " + DefaultValue : string.Empty;
            val = startId + val;
            return val;
        }
    }
}
