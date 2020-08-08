using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ComLib.Scheduling;

namespace ComLib.Scheduling.Tasks
{
    public interface ITaskService : IExtensionService<TaskAttribute, ITask>
    {
        IList<TaskSummary> GetStatuses();
        void RunAll();
    }
}
