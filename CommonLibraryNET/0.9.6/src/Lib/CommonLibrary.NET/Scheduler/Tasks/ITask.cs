using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;



namespace ComLib.Scheduling.Tasks
{
    /// <summary>
    /// Interface for a macro
    /// </summary>
    public interface ITask
    {
        void Process();
    }
}
