using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib.Benchmarks
{
    /// <summary>
    /// Benchmark service class to provide benchmarking functionality.
    /// </summary>
    public class BenchmarkService
    {
        private Action<BenchmarkResult> _logger;
        private string _name;
        private string _message;
        

        /// <summary>
        /// Default initialization.
        /// </summary>
        public BenchmarkService()
        {
        }


        /// <summary>
        /// Initialized w/ a lamda as a logger.
        /// </summary>
        /// <param name="logger"></param>
        public BenchmarkService(Action<BenchmarkResult> logger)
        {
            _logger = logger;
        }


        /// <summary>
        /// Initialized w/ a lamda as a logger.
        /// </summary>
        /// <param name="name">The name of the action to benchmark</param>
        /// <param name="message">A message associated w/ the action to benchmark</param>
        /// <param name="logger">The callback method for logging purposes.</param>
        public BenchmarkService(string name, string message, Action<BenchmarkResult> logger)
        {
            _name = name;
            _message = message;
            _logger = logger;
        }


        /// <summary>
        /// Run the action.
        /// </summary>
        /// <param name="action">The action to benchmark.</param>
        /// <returns></returns>
        public virtual BenchmarkResult Run(Action action)
        {
            return Report(_name, _message, _logger, action);
        }


        /// <summary>
        /// Run the action.
        /// </summary>
        /// <param name="name">The name of the action to benchmark</param>
        /// <param name="message">A message associated w/ the action to benchmark</param>
        /// <param name="action">The action to benchmark.</param>
        /// <returns></returns>
        public virtual BenchmarkResult Run(string name, string message, Action action)
        {
            return Report(name, message, _logger, action);
        }


        /// <summary>
        /// Run a simple benchmark with the supplied action and call the logger action supplied.
        /// </summary>
        /// <param name="name">The name of the action to benchmark</param>
        /// <param name="message">A message associated w/ the action to benchmark</param>
        /// <param name="logger">The callback method for logging purposes.</param>
        /// <param name="action">The action to benchmark.</param>
        public virtual BenchmarkResult Report(string name, string message, Action<BenchmarkResult> logger, Action action)
        {
            var result = new BenchmarkResult();
            result.Name = name;
            result.TimeStarted = DateTime.Now.TimeOfDay;
            action();
            result.TimeEnded = DateTime.Now.TimeOfDay;
            result.TimeDiff = result.TimeEnded - result.TimeStarted;

            if (logger != null)
                logger(result);
            else if (_logger != null)
                logger(result);
            else
                Console.WriteLine(result);

            return result;
        }
    }
}
