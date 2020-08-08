using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib
{
    public interface IExtensionAttribute
    {
        string Name { get; set; }
        string DisplayName { get; set; }
        string Description { get; set; }
        string Author { get; set; }
        string Email { get; set; }
        string Url { get; set; }
        string Version { get; set; }
        string DeclaringType { get; set; }
        string DeclaringAssembly { get; set; }
        Type DeclaringDataType { get; set; }
    }


    public class ExtensionAttribute : Attribute, IExtensionAttribute
    {
        /// <summary>
        /// Name of the widget.
        /// </summary>
        public string Name { get; set; }


        /// <summary>
        /// Group
        /// </summary>
        public string Group { get; set; }


        /// <summary>
        /// Name to display on labels.
        /// </summary>
        public string DisplayName { get; set; }


        /// <summary>
        /// Description for the widget
        /// </summary>
        public string Description { get; set; }


        /// <summary>
        /// Author of the widget.
        /// </summary>
        public string Author { get; set; }


        /// <summary>
        /// Email of the author.
        /// </summary>
        public string Email { get; set; }


        /// <summary>
        /// Version of the widget.
        /// </summary>
        public string Version { get; set; }


        /// <summary>
        /// Used to sort model in list of models.
        /// </summary>
        public int SortIndex { get; set; }


        /// <summary>
        /// Url for reference / documentation.
        /// </summary>
        public string Url { get; set; }


        /// <summary>
        /// The name of the class representing the widget.
        /// </summary>
        public string DeclaringType { get; set; }


        /// <summary>
        /// The name of the class representing the widget.
        /// </summary>
        public Type DeclaringDataType { get; set; }


        /// <summary>
        /// The name of the assembly containing the widget.
        /// </summary>
        public string DeclaringAssembly { get; set; }

        
        /// <summary>
        /// Roles that can access this information.
        /// </summary>
        public string Roles { get; set; }


        /// <summary>
        /// Whether or not this extension is reusable.
        /// </summary>
        public bool IsReusable { get; set; }
    }
}
