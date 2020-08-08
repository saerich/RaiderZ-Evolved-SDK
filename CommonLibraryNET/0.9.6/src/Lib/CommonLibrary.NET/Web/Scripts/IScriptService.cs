using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;

using ComLib.Reflection;


namespace ComLib.Web.ScriptsSupport
{
    /// <summary>
    /// Information Service
    /// </summary>
    public interface IScriptService
    {
        /// <summary>
        /// Add a scripts holder for the specified location.
        /// </summary>
        /// <param name="location"></param>
        /// <param name="useHttp"></param>
        void AddLocation(string location, bool useHttp = true);


        /// <summary>
        /// Get the scripts for the name.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        ScriptsHolder For(string name);


        /// <summary>
        /// Add a javascript to the specified location.
        /// </summary>
        /// <param name="location"></param>
        /// <param name="name"></param>
        /// <param name="url"></param>
        /// <param name="dependsOn"></param>
        /// <param name="version"></param>
        void AddJavascript(string name, string url, string location = "footer", string dependsOn = "", string version = "");


        /// <summary>
        /// Adds a css to the specified location.
        /// </summary>
        /// <param name="location"></param>
        /// <param name="name"></param>
        /// <param name="url"></param>
        /// <param name="dependsOn"></param>
        /// <param name="version"></param>
        void AddCss(string name, string url, string location = "footer", string dependsOn = "", string version = "");


        /// <summary>
        /// Gets the current scripts as html tags.
        /// </summary>
        /// <param name="location"></param>
        /// <returns></returns>
        string ToHtml(string location = "footer");
    }
}
