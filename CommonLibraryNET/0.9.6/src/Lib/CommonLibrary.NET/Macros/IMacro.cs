using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;

using ComLib.Reflection;


namespace ComLib.Macros
{
    /// <summary>
    /// Interface for a macro
    /// </summary>
    public interface IMacro
    {
        string Process(Tag tag);
    }
}
