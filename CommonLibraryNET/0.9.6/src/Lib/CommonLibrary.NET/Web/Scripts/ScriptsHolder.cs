using System;
using System.Collections;
using System.Linq;
using System.Text;
using System.Collections.Specialized;


namespace ComLib.Web.ScriptsSupport
{
    /// <summary>
    /// Script collection.
    /// </summary>
    public class ScriptsHolder
    {
        private bool _useHttp = true;
        private IDictionary _scripts = new OrderedDictionary();


        /// <summary>
        /// Default initialization
        /// </summary>
        public ScriptsHolder() : this(true)
        {   
        }


        /// <summary>
        /// Initialize flag to use http context or in-memory.
        /// </summary>
        /// <param name="useHttp"></param>
        public ScriptsHolder(bool useHttp)
        {
            _useHttp = useHttp;
        }


        /// <summary>
        /// Adds the javascript to the scripts.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="url"></param>
        /// <param name="dependsOn"></param>
        /// <param name="version"></param>
        public void AddJavascript(string name, string url, string dependsOn = "", string version = "")
        {
            var script = new Script(name, url, dependsOn, version);
            string format = "<script src=\"{0}\" type=\"text/javascript\"></script>";
            script.Tag = string.Format(format, url);
            var scripts = GetScripts();
            scripts[name] = script;
        }


        /// <summary>
        /// Adds the css to the scripts collection.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="url"></param>
        /// <param name="dependsOn"></param>
        /// <param name="version"></param>
        public void AddCss(string name, string url, string dependsOn = "", string version = "")
        {
            var script = new Script(name, url, dependsOn, version);
            string format = "<link href=\"{0}\" rel=\"stylesheet\" type=\"text/css\" />";
            script.Tag = string.Format(format, url);
            var scripts = GetScripts();
            scripts[name] = script;
        }


        /// <summary>
        /// Html representation of all the scripts, javascript followed by css.
        /// </summary>
        /// <returns></returns>
        public string ToHtml()
        {
            var scripts = GetScripts();
            if (scripts == null || scripts.Count == 0) return string.Empty;
            var buffer = new StringBuilder();
            foreach (DictionaryEntry pair in scripts)
            {
                var script = pair.Value as Script;
                buffer.AppendLine(script.Tag);
            }
            string html = buffer.ToString();
            return html;
        }


        private IDictionary GetScripts()
        {
            if (!_useHttp) return _scripts;

            var scriptsMap = System.Web.HttpContext.Current.Items["scripts"] as IDictionary;
            if (scriptsMap == null)
            {
                scriptsMap = new OrderedDictionary();
                System.Web.HttpContext.Current.Items["scripts"] = scriptsMap;
            }
            return scriptsMap;
        }
    }
}
