/*
 * Author: Kishore Reddy
 * Url: http://commonlibrarynet.codeplex.com/
 * Title: CommonLibrary.NET
 * Copyright: � 2009 Kishore Reddy
 * License: LGPL License
 * LicenseUrl: http://commonlibrarynet.codeplex.com/license
 * Description: A C# based .NET 3.5 Open-Source collection of reusable components.
 * Usage: Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Text;
using System.Threading;


namespace ComLib.Scheduling
{
    public static class Scheduler 
    {
        private static IScheduler _provider = new SchedulerService();
        

        /// <summary>
        /// Schedules the specified task via a delegate.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="trigger"></param>
        /// <param name="start"></param>
        public static void Schedule(string name, Trigger trigger, bool start, Action methodToExecute)
        {
            _provider.Schedule(name, trigger, start, methodToExecute, null);
        }


        /// <summary>
        /// Schedules the specified task via a delegate.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="trigger"></param>
        /// <param name="methodToExecute"></param>
        public static void Schedule(string name, Trigger trigger, Action methodToExecute)
        {
            _provider.Schedule(name, trigger, true, methodToExecute, null);
        }


        /// <summary>
        /// Schedules the specified task via a lamda and also get notification when it completes.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="trigger"></param>
        /// <param name="methodToExecute"></param>
        /// <param name="onCompletedAction"></param>
        public static void Schedule(string name, Trigger trigger, Action methodToExecute, Action<Task> onCompletedAction)
        {
            _provider.Schedule(name, trigger, true, methodToExecute, onCompletedAction);
        }


        /// <summary>
        /// Is started.
        /// </summary>
        public static bool IsStarted
        {
            get { return _provider.IsStarted; }
        }


        /// <summary>
        /// Is shut down.
        /// </summary>
        public static bool IsShutDown
        {
            get { return _provider.IsShutDown; }
        }


        /// <summary>
        /// Pauses the task
        /// </summary>
        /// <param name="name"></param>
        public static void Pause(string name)
        {
            _provider.Pause(name);
        }


        /// <summary>
        /// Run the task.
        /// </summary>
        /// <param name="name"></param>
        public static void Run(string name)
        {
            _provider.Run(name);
        }


        /// <summary>
        /// Resumes the task
        /// </summary>
        /// <param name="name"></param>
        public static void Resume(string name)
        {
            _provider.Resume(name);
        }


        /// <summary>
        /// Delete task
        /// </summary>
        /// <param name="name"></param>
        public static void Delete(string name)
        {
            _provider.Delete(name);
        }


        /// <summary>
        /// Gets all the active tasks in the schedule.
        /// BUG: Currently does not return the task name that are associated 
        /// with the group name.
        /// </summary>
        /// <returns></returns>
        public static string[] GetNames()
        {
            return _provider.GetNames();
        }


        /// <summary>
        /// Pause all tasks
        /// </summary>
        public static void PauseAll()
        {
            _provider.PauseAll();
        }


        /// <summary>
        /// Resume all tasks
        /// </summary>
        public static void ResumeAll()
        {
            _provider.ResumeAll();
        }


        /// <summary>
        /// return a list of all the statues of all tasks.
        /// </summary>
        /// <returns></returns>
        public static IList<TaskSummary> GetStatuses()
        {
            return _provider.GetStatus();
        }


        /// <summary>
        /// Get the status of the task specified by name.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static TaskSummary GetStatus(string name)
        {
            return _provider.GetStatus(name);
        }


        /// <summary>
        /// Shuts down the scheduler.
        /// </summary>
        public static void ShutDown()
        {
            _provider.ShutDown();
        }
    }
}