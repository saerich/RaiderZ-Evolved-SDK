using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;

using ComLib;
using ComLib.Extensions;
using ComLib.Reflection;
using ComLib.Scheduling;


namespace ComLib.Scheduling.Tasks
{
    
    /// <summary>
    /// Tasks Service
    /// </summary>
    public class TaskService : ExtensionService<TaskAttribute, ITask>, ITaskService
    {
        /// <summary>
        /// Gets the statuses of all the tasks in the system.
        /// </summary>
        /// <returns></returns>
        public IList<TaskSummary> GetStatuses()
        {
            var summaries = Scheduler.GetStatuses();
            foreach (var summary in summaries)
            {
                if (_lookup.ContainsKey(summary.Name))
                {
                    var attrib = _lookup[summary.Name].Attribute as TaskAttribute;                
                    summary.Group = attrib.Group;
                    summary.Description = attrib.Description;
                }
            }
            return summaries;
        }


        /// <summary>
        /// Runs all the extension tasks.
        /// </summary>
        public void RunAll()
        {
            foreach (var entry in this._lookup)
            {
                var metadata = entry.Value.Attribute as TaskAttribute;
                var instance = Create(metadata.Name);
                var trigger = new Trigger()
                {
                    StartTime = metadata.StartTime, 
                    MaxIterations = metadata.MaxIterations,
                    Interval = (int)metadata.Interval.Seconds().TotalMilliseconds,
                    Repeat = metadata.Repeat,
                    EndTime = metadata.EndTime,
                    IsOnDemand = metadata.IsOnDemand
                };
                if (metadata.IsOnDemand)
                    trigger.OnDemand();
                else if (metadata.MaxIterations > 0)
                    trigger.MaxRuns(metadata.MaxIterations);
                
                Scheduler.Schedule(metadata.Name, trigger, true, () => instance.Process());
            }
        }
    }
}
