using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib.Macros
{
    /// <summary>
    /// Attributes.
    /// </summary>
    [AttributeUsage( AttributeTargets.Class, AllowMultiple = true)]
    public class MacroParameter : Attribute
    {
        /// <summary>
        /// Name of the argument.
        /// </summary>
        public string Name = string.Empty;


        /// <summary>
        /// Describes the argument name.
        /// </summary>
        public string Description = string.Empty;


        /// <summary>
        /// Default value.
        /// </summary>
        public object DefaultValue = null;


        /// <summary>
        /// Whether or not the parameter is required.
        /// </summary>
        public bool IsRequired;


        /// <summary>
        /// Date type of the argument.
        /// </summary>
        public Type DataType;


        /// <summary>
        /// Example value.
        /// </summary>
        public string Example = string.Empty;


        /// <summary>
        /// Example of mutliple various values.
        /// </summary>
        public string ExampleMultiple = string.Empty;


        /// <summary>
        /// Allow initialize via named property initializers.
        /// </summary>
        public MacroParameter() { }
    }
}
