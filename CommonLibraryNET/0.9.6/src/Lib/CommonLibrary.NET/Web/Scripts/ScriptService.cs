using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;


namespace ComLib.Web.ScriptsSupport
{

    /// <summary>
    /// Information Service
    /// </summary>
    public class ScriptService : IScriptService
    {
        private bool _useHttp = true;
        private IDictionary<string, ScriptsHolder> _holders = new Dictionary<string, ScriptsHolder>();


        /// <summary>
        /// Add a new scripts holder for the specified location.
        /// </summary>
        /// <param name="location"></param>
        public void AddLocation(string location, bool useHttp = true)
        {
            _holders[location] = new ScriptsHolder(useHttp);
        }


        /// <summary>
        /// Get the scriptsholder for the specified location.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public ScriptsHolder For(string location)
        {
            return _holders[location];
        }


        /// <summary>
        /// Add a javascript to the specified location.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="url"></param>
        /// <param name="location"></param>
        /// <param name="dependsOn"></param>
        /// <param name="version"></param>
        public void AddJavascript(string name, string url, string location = "footer", string dependsOn = "", string version = "")
        {
            _holders[location].AddJavascript(name, url, dependsOn, version);
        }


        /// <summary>
        /// Adds a css to the specified location.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="url"></param>
        /// <param name="location"></param>
        /// <param name="dependsOn"></param>
        /// <param name="version"></param>
        public void AddCss(string name, string url, string location = "footer", string dependsOn = "", string version = "")
        {
            _holders[location].AddCss(name, url, dependsOn, version);
        }


        /// <summary>
        /// Gets the current scripts as html tags.
        /// </summary>
        /// <param name="location"></param>
        /// <returns></returns>
        public string ToHtml(string location = "footer")
        {
            return _holders[location].ToHtml();
        }
    }
}