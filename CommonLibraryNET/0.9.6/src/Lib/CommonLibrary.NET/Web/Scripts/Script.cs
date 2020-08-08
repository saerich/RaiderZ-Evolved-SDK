using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib.Web.ScriptsSupport
{
    /// <summary>
    /// Script object
    /// </summary>
    public class Script
    {
        /// <summary>
        /// Default
        /// </summary>
        public Script() { }


        /// <summary>
        /// Initalize 
        /// </summary>
        public Script(string name, string url, string dependsOn, string version)
        {
            Name = name;
            Url = url;
            DependsOn = dependsOn;
            Version = version;
        }


        /// <summary>
        /// Name of the script.
        /// </summary>
        public string Name;


        /// <summary>
        /// Url of the script.
        /// </summary>
        public string Url;


        /// <summary>
        /// Version of the script.
        /// </summary>
        public string Version;


        /// <summary>
        /// The other scripts this depends on.
        /// </summary>
        public string DependsOn;


        /// <summary>
        /// The html tag.
        /// </summary>
        public string Tag;
    }
}
