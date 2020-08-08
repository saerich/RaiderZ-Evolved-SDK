using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib.BootStrapSupport
{
    public interface IAppContext
    {
        IDictionary<string, object> Bag { get; set; }
    }



    /// <summary>
    /// Simple wrapper for passing context data between tasks.
    /// </summary>
    public class AppContext : IAppContext
    {        
        public AppContext()
        {
            Bag = new Dictionary<string, object>();
        }


        public IDictionary<string, object> Bag { get; set; }
    }
}
