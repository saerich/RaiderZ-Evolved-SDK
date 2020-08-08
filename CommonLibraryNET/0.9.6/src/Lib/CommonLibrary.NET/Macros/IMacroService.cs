using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;

using ComLib.Reflection;


namespace ComLib.Macros
{
    /// <summary>
    /// Information Service
    /// </summary>
    public interface IMacroService : IExtensionService<MacroAttribute, IMacro>
    {
        /// <summary>
        /// Builds the content for the specified tag.
        /// </summary>
        /// <param name="tag"></param>
        /// <returns></returns>
        string BuildContent(Tag tag);


        /// <summary>
        /// Builds content by replacing custom tags.
        /// </summary>
        /// <param name="content"></param>
        /// <returns></returns>
        string BuildContent(string content);
    }
}
