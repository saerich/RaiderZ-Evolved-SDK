using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;



namespace ComLib.Scheduling.Tasks
{  
    /// <summary>
    /// Attribute used to define a widget.
    /// </summary>
    public class TaskAttribute : ExtensionAttribute
    {
        /// <summary>
        /// The start time of the task
        /// </summary>
        public DateTime StartTime { get; set; }


        /// <summary>
        /// End time of the task
        /// </summary>
        public DateTime EndTime { get; set; }


        /// <summary>
        /// Interval in seconds of the task
        /// </summary>
        public int Interval { get; set; }


        /// <summary>
        /// Whether or not this is on-demand.
        /// </summary>
        public bool IsOnDemand { get; set; }


        /// <summary>
        /// Maximum number of times this can run.
        /// </summary>
        public int MaxIterations { get; set; }


        /// <summary>
        /// Whether or not this can repeat.
        /// </summary>
        public bool Repeat { get; set; }
    }
}
