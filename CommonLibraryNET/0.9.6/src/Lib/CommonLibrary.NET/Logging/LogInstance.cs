using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;


namespace ComLib.Logging
{
    /// <summary>
    /// This is used scoping a logger with a particular class.
    /// e.g. Have a logger only for class "BlogPostService".
    /// This is analogous to LogManger.Get(typeof(BlogPostService)) in Log4Net.
    /// </summary>
    /// <example>
    /// ILog logger = Logger.Get<BlogPostService>("default");
    /// logger.Info("testing");
    /// </example>
    public class LogInstance : LogBase, ILog
    {
        private string _loggerName;
        private Type _loggerType;


        /// <summary>
        /// Initialize with reference to the actually logger that does the logging
        /// and the calling type of the logger.
        /// </summary>
        /// <param name="loggerName"></param>
        /// <param name="callingType"></param>
        public LogInstance(string loggerName, Type callingType) : base(callingType.FullName)
        {
            _loggerName = loggerName;
            _loggerType = callingType;
        }


        /// <summary>
        /// Log the event to file.
        /// </summary>
        /// <param name="logEvent"></param>
        public override void Log(LogEvent logEvent)
        {
            logEvent.LogType = _loggerType;
            logEvent.FinalMessage = BuildMessage(logEvent);
            Logger.Get(_loggerName).Log(logEvent);            
        }
    }
}
