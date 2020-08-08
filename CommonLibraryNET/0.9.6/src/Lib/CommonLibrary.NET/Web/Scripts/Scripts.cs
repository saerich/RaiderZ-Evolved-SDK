using System;
using System.Collections;
using System.Linq;
using System.Text;


namespace ComLib.Web.ScriptsSupport
{
    /// <summary>
    /// Script collection.
    /// </summary>
    public class Scripts
    {
        private static ScriptService _service = new ScriptService();


        /// <summary>
        /// Gets the scripts place holder for the specified location.
        /// </summary>
        /// <param name="location"></param>
        /// <returns></returns>
        public static void AddLocation(string location, bool useHttp = true)
        {
            _service.AddLocation(location, useHttp);
        }


        /// <summary>
        /// Gets the scripts place holder for the specified location.
        /// </summary>
        /// <param name="location"></param>
        /// <returns></returns>
        public static ScriptsHolder For(string location)
        {
            return _service.For(location);
        }


        /// <summary>
        /// Add a javascript to the specified location.
        /// </summary>
        /// <param name="location"></param>
        /// <param name="name"></param>
        /// <param name="url"></param>
        /// <param name="dependsOn"></param>
        /// <param name="version"></param>
        public static void AddJavascript(string name, string url, string location = "footer", string dependsOn = "", string version = "")
        {
            _service.AddJavascript(name, url, location, dependsOn, version);
        }


        /// <summary>
        /// Adds a css to the specified location.
        /// </summary>
        /// <param name="location"></param>
        /// <param name="name"></param>
        /// <param name="url"></param>
        /// <param name="dependsOn"></param>
        /// <param name="version"></param>
        public static void AddCss(string name, string url, string location = "footer", string dependsOn = "", string version = "")
        {
            _service.AddCss(name, url, location, dependsOn, version);
        }


        /// <summary>
        /// Gets the current scripts as html tags.
        /// </summary>
        /// <param name="location"></param>
        /// <returns></returns>
        public static string ToHtml(string location = "footer")
        {
            return _service.For(location).ToHtml();
        }
    }
}
